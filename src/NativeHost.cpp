#include "ModMenu/NativeHost.h"
#include "ModMenu/Storage.h"
#include "ModMenu/View.h"
#include "ModMenu/Definition.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <bcrypt.h>
#include <MinHook.h>
#include <intrin.h>
#include <array>
#include <algorithm>
#include <cstring>
#include <cmath>
namespace menu {
namespace {
NativeProfile const* profile{};SharedState* shared{};std::atomic<bool> ready{false};
using Dtor=void*(*)(void*);Dtor originalDtor{};
using Pointer=std::uintptr_t(*)(void*,void const*,char);
}
extern "C" {
    Pointer MMB_PointerOriginal{};
    std::uintptr_t PointerHook(void*,void const*,char);
}
namespace {
using Button=void(*)(void*,unsigned,char);Button originalDown{},originalUp{};
using UpstreamRaw=std::uint64_t(*)(void*,void const*,void const*,void const*,void const*,void*);UpstreamRaw originalUpstreamRaw{};
using Text=void(*)(void*,void const*,char);Text originalText{};
using Wheel=void(*)(void*,unsigned char,float,float,char);Wheel originalWheel{};
using DefinitionLoad=void(*)(void*,void const*);DefinitionLoad originalDefinitionLoad{};
using JsonParse=unsigned char(*)(void*,char const*,std::size_t,void*,char);JsonParse originalJsonParse{};
using ControllerEvent=unsigned(*)(void*,void const*);ControllerEvent originalControllerEvent{};
std::atomic<bool> definitionReady{false};bool minhookInitialized{};
thread_local void* eventScene{};
struct DefinitionLifetime {
    std::string bytes;std::string_view target;bool allowed;DefinitionLifetime* previous;
    explicit DefinitionLifetime(std::string_view);
    ~DefinitionLifetime();
};
thread_local DefinitionLifetime* definitionLifetime{};
DefinitionLifetime::DefinitionLifetime(std::string_view name):target(name),allowed(!name.empty()),previous(definitionLifetime){definitionLifetime=this;}
DefinitionLifetime::~DefinitionLifetime(){definitionLifetime=previous;}
unsigned char ParseDefinition(std::uintptr_t caller,void* reader,char const* bytes,std::size_t length,void* json,char comments);
struct NonOwner {void* valid{};void* control{};void* object{};};
struct LTRB {float left{},right{},top{},bottom{};};
struct NativeText {char data[16]{};std::size_t length{},capacity{15};};
struct Measure {float scale{1},padding{};bool shadow{true},colors{},hyphen{};};
struct Caret {int position{-1};bool render{};};
static_assert(sizeof(NativeText)==32 && sizeof(Measure)==12 && sizeof(Caret)==8);
bool ReadBytes(void const* p,void* out,std::size_t n) noexcept{__try{if(!p)return false;std::memcpy(out,p,n);return true;}__except(EXCEPTION_EXECUTE_HANDLER){return false;}}
template<class T>T Read(void const* p,std::size_t offset=0){T out{};if(p)ReadBytes(static_cast<std::byte const*>(p)+offset,&out,sizeof out);return out;}
bool ReadLanguageCode(void* language,Locale& out) noexcept{
    // Localization::language code: constructor 45DD40, consumer 45AF30.
    auto length=Read<std::size_t>(language,56),capacity=Read<std::size_t>(language,64);
    if(!language || length!=5 || capacity<length || capacity>4095)return false;
    auto chars=capacity<16?static_cast<char*>(language)+40:Read<char*>(language,40);
    char code[6]{};if(!ReadBytes(chars,code,sizeof code) || code[5])return false;
    for(unsigned i:{0u,1u,3u,4u})if(!((code[i]>='A' && code[i]<='Z') || (code[i]>='a' && code[i]<='z')))return false;
    if(code[2]!='_' && code[2]!='-')return false;
    out=LocaleFromCode({code,5});return true;
}
bool ReadLocaleLocked(void* singleton,Locale& out) noexcept{
    // Same outer/inner lock order as 45AF30. Try-locks never block rendering.
    // Holding both read locks keeps holder and payload alive without refcount
    // ownership, native allocation, pointer caching or a new native call.
    auto outer=reinterpret_cast<PSRWLOCK>(static_cast<std::byte*>(singleton)+24);
    if(!TryAcquireSRWLockShared(outer))return false;
    bool ok=false;
    __try{
        auto holder=Read<void*>(singleton,32);
        if(holder){
            auto inner=Read<PSRWLOCK>(holder);
            if(inner && inner!=outer && TryAcquireSRWLockShared(inner)){
                __try{ok=ReadLanguageCode(Read<void*>(holder,8),out);}
                __finally{ReleaseSRWLockShared(inner);}
            }
        }else ok=ReadLanguageCode(Read<void*>(singleton,8),out);
    }__finally{ReleaseSRWLockShared(outer);}
    return ok;
}
bool ReadLocaleSnapshot(void* singleton,Locale& out) noexcept{
    if(!singleton)return false;
    __try{return ReadLocaleLocked(singleton,out);}__except(EXCEPTION_EXECUTE_HANDLER){return false;}
}
Locale CurrentLocale() noexcept{
    if(!profile || !profile->Base())return Locale::English;
    auto singleton=reinterpret_cast<void*>(profile->Address(0x11D2B570));
    if(!profile->Table(singleton,0xE75E4A0))return Locale::English;
    // Exact singleton getter, selection read/update and string-layout evidence.
    for(auto rva:{0x44E6A0u,0x44E750u,0x45AE40u,0x45AF30u,0x4572D0u,0x45DD40u}){
        // 45 independently establishes the read layout through its getter,
        // constructor and locked consumer. It never calls an update helper.
        if(rva==0x4572D0 && profile->Build()==GameBuild::V45)continue;
        if(!profile->Entry(profile->Address(rva)))return Locale::English;
    }
    Locale result=Locale::English;ReadLocaleSnapshot(singleton,result);return result;
}
unsigned char ParseDefinition(std::uintptr_t caller,void* reader,char const* bytes,std::size_t length,void* json,char comments){
    auto scope=definitionLifetime;char const* input=bytes;auto count=length;
    if(scope && scope->allowed && caller==profile->Address(0x416FC0) && length && length<=1024*1024){
        try{
            std::string source(length,'\0');
            if(ReadBytes(bytes,source.data(),length) && InsertModsDefinition(source,scope->bytes,scope->target)){
                input=scope->bytes.c_str();count=scope->bytes.size();shared->definitionsPatched.fetch_add(1);
            }else shared->definitionsRefused.fetch_add(1);
        }catch(...){shared->definitionsRefused.fetch_add(1);}
    }
    // The Json reader retains begin/end pointers. scope->bytes is owned by
    // DefinitionLoadHook, and outlives the reader's destruction in 416F40.
    return originalJsonParse(reader,input,count,json,comments);
}
unsigned char JsonParseHook(void* reader,char const* bytes,std::size_t count,void* json,char comments){
    return ParseDefinition(reinterpret_cast<std::uintptr_t>(_ReturnAddress()),reader,bytes,count,json,comments);
}
std::string_view AuditedDefinition(void* callback,void const* values){
    if(!definitionReady.load(std::memory_order_acquire) || !profile->Table(callback,0xE75C408))return {};
    auto name=static_cast<std::byte*>(callback)+8;
    auto length=Read<std::size_t>(name,16),capacity=Read<std::size_t>(name,24);
    // Both audited paths are exactly 20 bytes. Namespace is bound to the
    // path, not inferred from an arbitrary resource's JSON payload.
    std::array<char,20> actual{};
    if(length!=actual.size() || capacity<length || capacity>4095)return {};
    auto chars=capacity<16?reinterpret_cast<char const*>(name):Read<char const*>(name);
    if(!ReadBytes(chars,actual.data(),length))return {};
    auto target=DefinitionNamespace({actual.data(),actual.size()});if(target.empty())return {};
    auto begin=Read<std::uintptr_t>(values),end=Read<std::uintptr_t>(values,8),limit=Read<std::uintptr_t>(values,16);
    shared->definitionCandidates.fetch_add(1);
    shared->definitionLayers.store(begin && end>=begin && (end-begin)%48==0 && (end-begin)/48<=256?unsigned((end-begin)/48):0);
    // Multiple resource layers may contain modifications/overrides. This
    // initial native transform refuses those rather than reinterpreting them.
    return begin && end>begin && end-begin==48 && limit>=end && limit-begin<=48*256?target:std::string_view{};
}
void DefinitionLoadHook(void* callback,void const* values){
    DefinitionLifetime scope(AuditedDefinition(callback,values));
    if(scope.allowed)shared->definitionScopes.fetch_add(1);
    originalDefinitionLoad(callback,values);
}
bool Foreground(){DWORD pid{};auto w=GetForegroundWindow();if(w)GetWindowThreadProcessId(w,&pid);return pid==GetCurrentProcessId();}
bool Valid(NonOwner const& v){return v.object && v.valid && Read<unsigned char>(v.valid)!=0;}
void Release(NonOwner& v){profile->Function<void(*)(void*)>(0x7E380)(&v);v={};}
void* Top(void* stack,bool fallback=false){
    auto valid=Read<void*>(stack,8);if(!valid || !Read<unsigned char>(valid))return nullptr;
    auto const& layout=profile->Layout();auto fn=profile->Virtual(stack,fallback?layout.gameTop:layout.stackTop);return fn?reinterpret_cast<void*(*)(void*)>(fn)(stack):nullptr;
}
// Mirrors the two independently exported 5DC6430 branches. Never call its
// shared AutoClick entry; all virtual destinations must match disk and code.
void* TopFor(void* client){
    auto const& layout=profile->Layout();
    if(auto top=Top(Read<void*>(client,layout.clientStack)))return top;
    auto game=Read<void*>(client,layout.clientGame);auto fn=profile->Virtual(game,layout.gameStack);if(!fn)return nullptr;
    NonOwner stack{};reinterpret_cast<void*(*)(void*,NonOwner*)>(fn)(game,&stack);
    auto top=Valid(stack)?Top(stack.object,true):nullptr;Release(stack);return top;
}
MenuScene ReadMenuName(void* view,NativeLayout const& layout){
    // Exact raw-name ownership/layout from 484F4A0; bounded SSO/heap read.
    auto holder=Read<void*>(view,layout.viewNameOwner),meta=Read<void*>(holder,8);if(!meta)return MenuScene::Unknown;
    auto length=Read<std::size_t>(meta,48),capacity=Read<std::size_t>(meta,56);
    if(length!=12 || capacity<length || capacity>4095)return MenuScene::Unknown;
    auto chars=capacity<16?static_cast<std::byte*>(meta)+32:Read<std::byte*>(meta,32);char name[12]{};
    return ReadBytes(chars,name,12)?ClassifyMenuScene({name,12}):MenuScene::Unknown;
}
std::uint32_t ControllerTable(MenuScene kind){
    return kind==MenuScene::Main?0xE900070:kind==MenuScene::Pause?0xE804440:0;
}
MenuScene NamedMenu(void* scene){
    if(!profile->Table(scene,0xE8E9080))return MenuScene::Unknown;
    auto const& layout=profile->Layout();
    auto view=Read<void*>(scene,64);if(!view || Read<unsigned char>(view,layout.transitionA) || Read<unsigned char>(view,layout.transitionB))return MenuScene::Unknown;
    auto kind=ReadMenuName(view,layout);auto table=ControllerTable(kind);
    return table && profile->Table(Read<void*>(view,layout.viewController),table)?kind:MenuScene::Unknown;
}
Frame SceneFrame(void* scene){
    Frame f;f.foreground=Foreground();shared->lastGate.store(1);
    if(!profile->Table(scene,0xE8E9080)){shared->unknownScenes.fetch_add(1);return f;}
    auto validity=Read<void*>(scene,40),client=Read<void*>(scene,56);
    shared->lastGate.store(2);if(!validity || !Read<unsigned char>(validity) || !profile->Table(client,0xE9731B0))return f;
    shared->lastGate.store(3);auto top=TopFor(client);
    if(top!=scene){
        // The UI renders and dispatches to several sibling scenes in a frame.
        // They must pass through, not erase the top menu's modal/captures.
        // This is not a cached permission: ownership/name are checked now.
        auto topValidity=NamedMenu(top)!=MenuScene::Unknown?Read<void*>(top,40):nullptr;
        if(topValidity && Read<unsigned char>(topValidity) && Read<void*>(top,56)==client){f.observerOnly=true;shared->siblingCallbacks.fetch_add(1);shared->lastGate.store(7);}
        return f;
    }
    shared->lastGate.store(1);f.scene=NamedMenu(scene);if(f.scene==MenuScene::Unknown){shared->unknownScenes.fetch_add(1);return f;}
    shared->lastGate.store(4);auto fn=profile->Virtual(client,profile->Layout().guiGetter);if(!fn)return f;NonOwner gui{};
    reinterpret_cast<void*(*)(void*,NonOwner*)>(fn)(client,&gui);
    if(Valid(gui)){f.width=Read<float>(gui.object,80);f.height=Read<float>(gui.object,84);f.eligible=std::isfinite(f.width) && std::isfinite(f.height) && f.width>0 && f.width<32768 && f.height>0 && f.height<32768;}
    Release(gui);if(f.eligible){shared->eligible.fetch_add(1);shared->lastGate.store(f.width>=240 && f.height>=160?6:5);shared->width.store(f.width);shared->height.store(f.height);}return f;
}
Frame SafeFrame(void* scene) noexcept{__try{return SceneFrame(scene);}__except(EXCEPTION_EXECUTE_HANDLER){shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);return {};}}
bool Coordinates(void* scene,void const* event,float& x,float& y){
    auto client=Read<void*>(scene,56);auto getter=profile->Virtual(client,profile->Layout().guiGetter);if(!getter)return false;
    NonOwner gui{};reinterpret_cast<void*(*)(void*,NonOwner*)>(getter)(client,&gui);bool ok=false;
    if(Valid(gui)){
        auto app=Read<void*>(gui.object,152);auto fn=profile->Virtual(app,profile->Layout().appOrigin);
        if(fn){auto origin=reinterpret_cast<void*(*)(void*)>(fn)(app);auto scale=Read<float>(gui.object,96);
            x=(Read<short>(event,4)-Read<float>(origin,8))*scale;y=(Read<short>(event,6)-Read<float>(origin,12))*scale;
            ok=origin && std::isfinite(scale) && scale>0 && scale<64 && std::isfinite(x) && std::isfinite(y);
        }
    }Release(gui);return ok;
}
std::string Clipboard();
void PasteIfRequested(Menu& m,Frame const& f,std::string(*read)()=Clipboard){
    if(m.TakePasteRequest(f))m.Input({Kind::Text,0,0,0,0,read()},f);
}
bool NativeInput(Event const& e,void* scene){
    if(!ready.load(std::memory_order_acquire))return false;
    try{auto f=SafeFrame(scene);std::lock_guard lock(shared->mutex);auto input=e;
        if(input.kind==Kind::Key && f.eligible && f.foreground && !f.observerOnly){
            input.modifiers={(GetAsyncKeyState(VK_SHIFT)&0x8000)!=0,(GetAsyncKeyState(VK_CONTROL)&0x8000)!=0,
                (GetAsyncKeyState(VK_MENU)&0x8000)!=0,(GetKeyState(VK_CAPITAL)&1)!=0};
        }
        auto consumed=shared->menu.Input(input,f);PasteIfRequested(shared->menu,f);
        if(consumed)shared->inputs.fetch_add(1);return consumed;}
    catch(...){shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);return false;}
}
std::string Clipboard(){
    std::string result;if(!OpenClipboard(nullptr))return result;auto h=GetClipboardData(CF_UNICODETEXT);
    if(h && GlobalSize(h)<=8192){auto p=static_cast<wchar_t const*>(GlobalLock(h));if(p){auto bound=GlobalSize(h)/sizeof(wchar_t);std::size_t n=0;while(n<bound && p[n])++n;
        if(n<bound && n<=256){int count=WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,p,int(n),nullptr,0,nullptr,nullptr);if(count>0 && count<=256){result.resize(count);WideCharToMultiByte(CP_UTF8,WC_ERR_INVALID_CHARS,p,int(n),result.data(),count,nullptr,nullptr);}}
        GlobalUnlock(h);
    }}CloseClipboard();return result;
}
bool TextValue(void const* value,std::string& out){
    auto n=Read<std::size_t>(value,16),cap=Read<std::size_t>(value,24);if(n>256 || cap<n || cap>65536)return false;
    auto p=cap<16?static_cast<char const*>(value):Read<char const*>(value);out.resize(n);return ReadBytes(p,out.data(),n) && ValidUtf8(out);
}
bool HandleRaw(void* scene,int key,char type,char state){
    bool consumed=false;
    if(ready.load()){shared->rawEvents.fetch_add(1);shared->lastRawKey.store(key);shared->lastRawType.store(type);shared->lastRawState.store(state);if(type==4)shared->keyEvents.fetch_add(1);}
    if(type==4 && (state==0 || state==1)){
        consumed=NativeInput({Kind::Key,0,0,key,state},scene);
    }else if(ready.load()){
        auto f=SafeFrame(scene);std::lock_guard lock(shared->mutex);shared->menu.Tick(f);auto& m=shared->menu;
        consumed=f.eligible && f.foreground && (m.BlocksMapped() || (!m.nativeEntry && type==1 && Arrange(m.config,f).button.Contains(m.pointerX,m.pointerY)));
    }
    return consumed;
}
void* RawScene(void* client) noexcept{
    __try{return profile->Table(client,0xE9731B0)?TopFor(client):nullptr;}
    __except(EXCEPTION_EXECUTE_HANDLER){shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);return nullptr;}
}
std::uint64_t UpstreamRawHook(void* adapter,void const* key,void const* type,void const* state,void const* remap,void* client){
    // 42A5E40 registers this six-argument raw listener. 42E1630 obtains
    // client virtual+1808 before forwarding to scene virtual+192; that
    // recipient need not exist for a normal start-screen key event.
    // This adapter forwards only actual native events. Local physical menu
    // sampling in Render is independent and never calls a native input method.
    unsigned k{};unsigned char t{},s{},r{};
    if(ready.load() && ReadBytes(key,&k,4) && ReadBytes(type,&t,1) && ReadBytes(state,&s,1) && ReadBytes(remap,&r,1)){
        if(HandleRaw(RawScene(client),int(k),char(t),char(s)))return 0;
    }
    return originalUpstreamRaw(adapter,key,type,state,remap,client);
}
}
extern "C" bool MMB_PointerObserver(void* scene,void const* event,char,std::uintptr_t caller) noexcept{
    float x{},y{};bool consumed=false;
    try{if(ready.load()){
        shared->pointerEvents.fetch_add(1);
        if(caller==profile->Address(0x5D9430F))shared->primaryPointerEvents.fetch_add(1);
        auto f=SafeFrame(scene);
        if(f.eligible && f.foreground && Coordinates(scene,event,x,y))consumed=NativeInput({Kind::Move,x,y},scene);
        else{std::lock_guard lock(shared->mutex);shared->menu.Tick(f);}
    }}catch(...){shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);return false;}
    // PointerHook's assembly entry handles consumption or a tail jump. Calling
    // the previous owner here would replace the game's audited return address.
    return consumed;
}
namespace {
void ForwardButton(void* scene,unsigned id,char focus,bool down){
    auto previous=eventScene;eventScene=scene;
    __try{(down?originalDown:originalUp)(scene,id,focus);}
    __finally{eventScene=previous;}
}
unsigned ControllerEventHook(void* controller,void const* event){
    if(ready.load(std::memory_order_acquire) && eventScene){
        shared->scopedControllerEvents.fetch_add(1);auto table=Read<std::uintptr_t>(controller);
        shared->lastControllerTable.store(table>=profile->Base() && table-profile->Base()<0x12C02000?unsigned(table-profile->Base()):0);
    }
    if(ready.load(std::memory_order_acquire) && eventScene &&
       (profile->Table(controller,0xE900070) || profile->Table(controller,0xE804440))){
        std::array<unsigned char,18> bytes{};
        if(ReadBytes(event,bytes.data(),bytes.size())){
            unsigned tag{},id{};std::memcpy(&tag,bytes.data(),4);std::memcpy(&id,bytes.data()+8,4);
            if(tag==1 && id==ModsEventId && bytes[16]<=2 && bytes[17]<=2){
                shared->modsEvents.fetch_add(1);
                auto f=SafeFrame(eventScene);auto view=Read<void*>(eventScene,64);
                if(f.eligible && f.foreground && Read<void*>(view,profile->Layout().viewController)==controller){
                    std::lock_guard lock(shared->mutex);auto& m=shared->menu;
                    if(bytes[17]==1 && bytes[16]!=1 && !m.open && m.OpenNative(f)){shared->nativeOpens.fetch_add(1);return 40;}
                    if(m.nativeEntry)return 32;
                }
            }
        }
    }
    return originalControllerEvent(controller,event);
}
void ButtonEvent(void* scene,unsigned id,char focus,bool down){
    // Read the exact current UI logical cursor. The outer pointer adapter
    // updates it normally when not modal; while modal our own value is used.
    auto f=ready.load()?SafeFrame(scene):Frame{};bool consumed=false;
    if(ready.load()){
        (down?shared->downEvents:shared->upEvents).fetch_add(1);shared->lastMappedId.store(id);
        std::lock_guard lock(shared->mutex);auto& m=shared->menu;m.Tick(f);
        int button=id==0xB447F8E7 || id==0x588FCBB0?1:id==0x2A5C0ECC?2:0;
        // Unknown mapped actions must not activate vanilla controls under a modal.
        if(button)consumed=m.Input({down?Kind::Down:Kind::Up,m.pointerX,m.pointerY,button},f);
        else consumed=m.CaptureMapped(id,down,f);
        if(consumed)shared->inputs.fetch_add(1);
    }
    if(!consumed)ForwardButton(scene,id,focus,down);
}
void DownHook(void* s,unsigned id,char f){ButtonEvent(s,id,f,true);}
void UpHook(void* s,unsigned id,char f){ButtonEvent(s,id,f,false);}
void TextHook(void* scene,void const* text,char focus){std::string value;
    if(ready.load())shared->textEvents.fetch_add(1);
    bool valid=TextValue(text,value);
    // Raw keys own ASCII typing, even without a native TextEdit component.
    // Do not duplicate a secondary native text event for the same key.
    if(valid && std::all_of(value.begin(),value.end(),[](unsigned char c){return c<128;}))value.clear();
    bool consumed=valid && NativeInput({Kind::Text,0,0,0,0,value},scene);
    if(!consumed)originalText(scene,text,focus);
}
void WheelHook(void* scene,unsigned char id,float x,float y,char focus){
    if(ready.load())shared->wheelEvents.fetch_add(1);
    int amount=y>0?1:y<0?-1:x>0?1:x<0?-1:0;
    if(!NativeInput({Kind::Wheel,0,0,0,amount},scene))originalWheel(scene,id,x,y,focus);
}
void NativeString(void* context,Paint const& paint){
    NativeText text;Measure measure;measure.scale=paint.scale;Caret caret;auto const& value=paint.text;
    void* allocated=nullptr;
    if(value.size()>15){text.capacity=value.size()|std::size_t(15);allocated=profile->Function<void*(*)(std::size_t)>(0x26E810)(text.capacity+1);if(!allocated)return;std::memcpy(text.data,&allocated,sizeof allocated);}
    auto data=allocated?static_cast<char*>(allocated):text.data;std::memcpy(data,value.data(),value.size());data[value.size()]=0;text.length=value.size();
    LTRB rect{paint.rect.x,paint.rect.x+paint.rect.w,paint.rect.y,paint.rect.y+paint.rect.h};
    __try{profile->Function<void(*)(void*,LTRB const*,NativeText*,Color const*,float,int,Measure const*,Caret const*)>(0x662FED0)(context,&rect,&text,&paint.color,paint.color.a,0,&measure,&caret);}
    __finally{if(text.capacity>=16)profile->Function<void(*)(void*)>(0x26E860)(allocated);}
}
// 4830560 enables screen+125; 463F2B0 disables it before our destructor
// callback. Native late UI draws (4352910 / 47A3100) save/set/restore it.
// This scope never changes projection matrices or leaves a flag enabled.
bool WithNativeUiMode(void* screen,void(*submit)(void*),void* user) noexcept{
    if(!screen || !submit)return false;
    unsigned char previous{};auto flag=static_cast<unsigned char*>(screen)+125;
    if(!ReadBytes(flag,&previous,1) || previous>1)return false;
    bool ok=false;
    __try{
        __try{*flag=1;submit(user);ok=true;}
        __finally{*flag=previous;}
    }__except(EXCEPTION_EXECUTE_HANDLER){ok=false;}
    return ok;
}
struct PaintWork {void* context;std::vector<Paint> const* paints;};
void SubmitPaints(void* user){
    auto& work=*static_cast<PaintWork*>(user);auto context=work.context;
    for(auto const& paint:*work.paints){
        if(paint.text.empty()){LTRB rect{paint.rect.x,paint.rect.x+paint.rect.w,paint.rect.y,paint.rect.y+paint.rect.h};profile->Function<void(*)(void*,LTRB const*,Color const*,float)>(0x6636EE0)(context,&rect,&paint.color,paint.color.a);}
        else NativeString(context,paint);
    }
    profile->Function<void(*)(void*,float,std::uint64_t)>(0x6630150)(context,0.f,0);
}
bool DrawNative(void* context,std::vector<Paint> const& paints) noexcept{
    std::array<std::byte,17> saved{};bool clipping=false,ok=false;
    __try{
        if(!profile->Table(context,0xE9958B0) || !ReadBytes(static_cast<std::byte*>(context)+296,saved.data(),17))return false;
        __try{
            profile->Function<void(*)(void*)>(0x6637B00)(context);clipping=true;profile->Function<void(*)(void*)>(0x6637AF0)(context);
            PaintWork work{context,&paints};ok=WithNativeUiMode(Read<void*>(context,16),SubmitPaints,&work);
        }__finally{
            if(clipping){profile->Function<void(*)(void*)>(0x6637B90)(context);std::memcpy(static_cast<std::byte*>(context)+296,saved.data(),17);}
        }
    }__except(EXCEPTION_EXECUTE_HANDLER){ok=false;}
    return ok;
}
// Passive, bounded snapshots only. No pointers or borrowed strings survive
// this callback, and these observations confer no draw/input permission.
void ObserveRender(void* context,void* scene,Frame const& f){
    if((!f.eligible && !f.observerOnly) || shared->renderReceiptCount>=shared->renderReceipts.size())return;
    RenderReceipt receipt;receipt.foreground=f.foreground;receipt.top=f.eligible;
    auto view=Read<void*>(scene,64),holder=Read<void*>(view,profile->Layout().viewNameOwner),meta=Read<void*>(holder,8);
    auto n=Read<std::size_t>(meta,48),capacity=Read<std::size_t>(meta,56);
    if(!meta || n==0 || n>=receipt.scene.size() || capacity<n || capacity>4095)return;
    auto chars=capacity<16?static_cast<std::byte*>(meta)+32:Read<std::byte*>(meta,32);
    if(!ReadBytes(chars,receipt.scene.data(),n))return;
    for(std::size_t i=0;i<n;++i)if(static_cast<unsigned char>(receipt.scene[i])<32 || static_cast<unsigned char>(receipt.scene[i])>126)return;
    for(unsigned i=0;i<shared->renderReceiptCount;++i){auto const& old=shared->renderReceipts[i];
        if(old.scene==receipt.scene && old.foreground==receipt.foreground && old.top==receipt.top)return;
    }
    auto rva=[](std::uintptr_t p){return p>=profile->Base() && p-profile->Base()<0x12C02000?std::uint32_t(p-profile->Base()):0u;};
    auto renderer=Read<void*>(scene,32);receipt.rendererTable=rva(Read<std::uintptr_t>(renderer));
    receipt.beginMethod=rva(profile->Virtual(renderer,8));receipt.endMethod=rva(profile->Virtual(renderer,16));
    auto screen=Read<void*>(context,16),validity=Read<void*>(screen,152),gui=Read<void*>(screen,168);
    if(validity && Read<unsigned char>(validity) && gui){receipt.nativeWidth=Read<float>(gui,80);receipt.nativeHeight=Read<float>(gui,84);}
    receipt.contextAlpha=Read<float>(context,56);receipt.depth=Read<unsigned char>(context,200);receipt.clip=Read<unsigned char>(screen,120);
    auto gpu=Read<void*>(screen,16),stencil=Read<void*>(gpu,264);receipt.stencil=Read<unsigned short>(stencil);
    auto state=Read<void*>(screen,32),uniform=Read<void*>(state,336),alpha=Read<void*>(uniform,64);receipt.shaderAlpha=Read<float>(alpha);
    shared->renderReceipts[shared->renderReceiptCount++]=receipt;
}
void Render(void* context){
    if(!ready.load() || !profile->Table(context,0xE9958B0))return;
    auto scene=Read<void*>(context,288);auto f=SafeFrame(scene);std::vector<Paint> paints;
    auto locale=f.eligible && !f.observerOnly?CurrentLocale():Locale::English;
    if(f.eligible)(f.foreground?shared->foregroundRenders:shared->backgroundRenders).fetch_add(1);
    {std::lock_guard lock(shared->mutex);auto& m=shared->menu;
        if(f.eligible && !f.observerOnly)m.locale=locale;
        // Physical edges act only on plugin-owned UI in the independently
        // validated top menu. No native/OS input is sent or fabricated.
        std::array<bool,256> keys{};
        if(f.eligible && f.foreground && !f.observerOnly && f.width>=240 && f.height>=160 &&
           (f.scene==MenuScene::Main || f.scene==MenuScene::Pause))
            for(unsigned k=8;k<255;++k)keys[k]=(GetAsyncKeyState(int(k))&0x8000)!=0;
        bool down=m.config.openKey<keys.size() && keys[m.config.openKey];
        bool caps=f.eligible && f.foreground && !f.observerOnly && (GetKeyState(VK_CAPITAL)&1)!=0;
        if(m.PollKeyboard(keys,f,caps,GetTickCount64()))shared->sampledShortcutPresses.fetch_add(1);
        PasteIfRequested(m,f);
        ObserveRender(context,scene,f);
        if(down)shared->physicalKeyFrames.fetch_add(1);
        if(f.eligible && !m.open && !m.editing && !m.Captured()){
            auto view=Read<void*>(scene,64);auto x=Read<float>(view,profile->Layout().cursorX),y=Read<float>(view,profile->Layout().cursorY);
            if(std::isfinite(x) && std::isfinite(y)){m.pointerX=x;m.pointerY=y;}
        }paints=BuildView(m,f);
    }
    if(paints.empty())return;
    shared->paintCommands.fetch_add(paints.size());
    if(DrawNative(context,paints))shared->draws.fetch_add(1);
    else{shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);}
}
void* DtorHook(void* context){
    if(reinterpret_cast<std::uintptr_t>(_ReturnAddress())==profile->Address(0x4857852)){shared->contexts.fetch_add(1);try{Render(context);}catch(...){shared->faults.fetch_add(1);ready.store(false);shared->status.store(4);}}
    return originalDtor(context); // Exactly once, including every fail-closed branch.
}
bool HashFile(HMODULE module,char const* expected){
    wchar_t path[32768]{};auto n=GetModuleFileNameW(module,path,32768);if(!n || n>=32768)return false;
    auto h=CreateFileW(path,GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);if(h==INVALID_HANDLE_VALUE)return false;
    LARGE_INTEGER length{};bool ok=GetFileSizeEx(h,&length) && length.QuadPart==158208;std::vector<unsigned char> bytes(ok?std::size_t(length.QuadPart):0);DWORD used{};
    if(ok)ok=ReadFile(h,bytes.data(),DWORD(bytes.size()),&used,nullptr) && used==bytes.size();CloseHandle(h);
    if(!ok)return false;BCRYPT_ALG_HANDLE alg{};BCRYPT_HASH_HANDLE hash{};DWORD size{};
    ok=BCryptOpenAlgorithmProvider(&alg,BCRYPT_SHA256_ALGORITHM,nullptr,0)>=0;
    if(ok)ok=BCryptGetProperty(alg,BCRYPT_OBJECT_LENGTH,reinterpret_cast<PUCHAR>(&size),4,&used,0)>=0;
    std::vector<unsigned char> object(size);std::array<unsigned char,32> digest{};
    if(ok)ok=BCryptCreateHash(alg,&hash,object.data(),size,nullptr,0,0)>=0;
    if(ok)ok=BCryptHashData(hash,bytes.data(),ULONG(bytes.size()),0)>=0 && BCryptFinishHash(hash,digest.data(),32,0)>=0;
    if(hash)BCryptDestroyHash(hash);if(alg)BCryptCloseAlgorithmProvider(alg,0);
    constexpr char hex[]="0123456789ABCDEF";for(unsigned i=0;ok && i<32;++i)ok=hex[digest[i]>>4]==expected[i*2] && hex[digest[i]&15]==expected[i*2+1];return ok;
}
bool Owned(std::uintptr_t address,HMODULE module){HMODULE owner{};return GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(address),&owner) && owner==module;}
bool KnownJump(std::uintptr_t address,HMODULE module){
    std::array<unsigned char,16> bytes{};if(!ReadBytes(reinterpret_cast<void*>(address),bytes.data(),bytes.size()) || bytes[0]!=0xe9)return false;
    std::int32_t delta{};std::memcpy(&delta,bytes.data()+1,4);auto target=address+5+delta;
    if(Owned(target,module))return true;
    if(!ReadBytes(reinterpret_cast<void*>(target),bytes.data(),bytes.size()) || bytes[0]!=0xff || bytes[1]!=0x25)return false;
    std::memcpy(&delta,bytes.data()+2,4);auto actual=Read<std::uintptr_t>(reinterpret_cast<void*>(target+6+delta));return Owned(actual,module);
}
}
void DeactivateNative() noexcept{ready.store(false,std::memory_order_release);definitionReady.store(false,std::memory_order_release);}
bool InstallDefinitions(NativeProfile const& p,SharedState& state,std::string& reason){
    std::array<std::uint32_t,2> rvas{0x416F40,0xE033870};
    for(auto r:rvas)if(!p.Entry(p.Address(r))){reason="UI definition entry changed; no hooks installed";return false;}
    if(MH_Initialize()!=MH_OK){reason="Independent definition-hook initialization failed";return false;}
    minhookInitialized=true;profile=&p;shared=&state;
    std::array<void*,2> hooks{reinterpret_cast<void*>(DefinitionLoadHook),reinterpret_cast<void*>(JsonParseHook)};
    std::array<void**,2> originals{reinterpret_cast<void**>(&originalDefinitionLoad),reinterpret_cast<void**>(&originalJsonParse)};
    for(std::size_t i=0;i<rvas.size();++i){auto target=reinterpret_cast<void*>(p.Address(rvas[i]));
        if(MH_CreateHook(target,hooks[i],originals[i])!=MH_OK || MH_QueueEnableHook(target)!=MH_OK){MH_Uninitialize();minhookInitialized=false;reason="Definition hook creation failed; none enabled";return false;}}
    if(MH_ApplyQueued()!=MH_OK){for(auto r:rvas)MH_DisableHook(reinterpret_cast<void*>(p.Address(r)));reason="Definition hook activation failed; inactive trampolines retained";return false;}
    definitionReady.store(true,std::memory_order_release);reason="Guarded native definition hooks active before dependency wait";return true;
}
InstallResult InstallNative(NativeProfile const& p,SharedState& state,bool expectMouseTweaks,std::string& reason){
    // Definition callbacks may already run on other threads. Their published
    // host pointers stay immutable; input installation must reuse that host.
    if(!minhookInitialized || !definitionReady.load(std::memory_order_acquire) || profile!=&p || shared!=&state){
        reason="Definition host unavailable or mismatched; input hooks refused";return InstallResult::Failure;
    }
    auto mtb=GetModuleHandleW(L"MouseTweaksBedrock.dll");
    if(expectMouseTweaks && !mtb){reason="Waiting for installed MouseTweaksBedrock to load";return InstallResult::Wait;}
    int mtbStatus=-1;bool known=false;
    if(mtb){
        known=HashFile(mtb,"2F289102D36111AA975671F794D10F2C3C6ACE7B1423466674010FD12C7F4577");
        auto status=reinterpret_cast<int(*)()>(GetProcAddress(mtb,"MTB_GetStatus"));
        if(!known || !status || !Owned(reinterpret_cast<std::uintptr_t>(status),mtb)){reason="Unrecognized MouseTweaks image/status contract";return InstallResult::Conflict;}
        mtbStatus=status();if(mtbStatus==0){reason="Waiting for formal MouseTweaks Ready";return InstallResult::Wait;}
        if(mtbStatus!=3){reason="MouseTweaks did not reach Ready";return InstallResult::Conflict;}
        HMODULE pin{};if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(status),&pin)){reason="Cannot pin audited hook owner";return InstallResult::Failure;}
    }
    std::array<std::uint32_t,8> rvas{0x662FBE0,0x48560C0,0x48560A0,0x48560B0,0x42E1630,0x485A220,0x48587D0,0x4819AF0};
    for(std::size_t i=0;i<rvas.size();++i){auto address=p.Address(rvas[i]);bool canonical=p.Entry(address),isShared=i>=1 && i<=3;
        auto decision=ClassifyEntry(canonical,isShared && known && KnownJump(address,mtb),mtbStatus);
        if(decision!=Chain::Canonical && decision!=Chain::KnownReady){reason="Input/render entry conflict at RVA "+std::to_string(rvas[i]);return InstallResult::Conflict;}
    }
    std::array<void*,8> detours{reinterpret_cast<void*>(DtorHook),reinterpret_cast<void*>(PointerHook),reinterpret_cast<void*>(DownHook),reinterpret_cast<void*>(UpHook),reinterpret_cast<void*>(UpstreamRawHook),reinterpret_cast<void*>(TextHook),reinterpret_cast<void*>(WheelHook),reinterpret_cast<void*>(ControllerEventHook)};
    std::array<void**,8> originals{reinterpret_cast<void**>(&originalDtor),reinterpret_cast<void**>(&MMB_PointerOriginal),reinterpret_cast<void**>(&originalDown),reinterpret_cast<void**>(&originalUp),reinterpret_cast<void**>(&originalUpstreamRaw),reinterpret_cast<void**>(&originalText),reinterpret_cast<void**>(&originalWheel),reinterpret_cast<void**>(&originalControllerEvent)};
    for(std::size_t i=0;i<rvas.size();++i){auto target=reinterpret_cast<void*>(p.Address(rvas[i]));if(MH_CreateHook(target,detours[i],originals[i])!=MH_OK || MH_QueueEnableHook(target)!=MH_OK){DeactivateNative();reason="Input/render hook creation failed; queued hooks inactive, trampolines retained";return InstallResult::Failure;}}
    if(MH_ApplyQueued()!=MH_OK){for(auto r:rvas)MH_DisableHook(reinterpret_cast<void*>(p.Address(r)));reason="Hook activation failed; trampolines retained";return InstallResult::Failure;}
    ready.store(true,std::memory_order_release);reason="Native main/pause menu render/input Ready";return InstallResult::Ready;
}
}
