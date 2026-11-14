// Compile the real adapters into a separate test EXE. No test API is exported
// by the shipping DLL and no Minecraft process is loaded or launched.
#include "../src/NativeHost.cpp"
#include <iostream>
#include <fstream>
#include <json.hpp>
namespace {
int destructorCalls{},pointerCalls{},downCalls{},upCalls{},rawCalls{},textCalls{},wheelCalls{};
unsigned clipboardReads{};
std::string ReadTestClipboard(){++clipboardReads;return "物品";}
unsigned uiObserved{};
unsigned parseCalls{};char const* parseBytes{};std::size_t parseLength{};
std::array<std::uintptr_t,3> parseForwarded{};
unsigned char Parse(void* reader,char const* bytes,std::size_t length,void* json,char comments){++parseCalls;parseBytes=bytes;parseLength=length;parseForwarded={reinterpret_cast<std::uintptr_t>(reader),reinterpret_cast<std::uintptr_t>(json),static_cast<unsigned char>(comments)};return 0xAB;}
unsigned controllerCalls{};void* controllerForwarded{};void const* eventForwarded{};bool scopedButton{};
unsigned Controller(void* controller,void const* event){++controllerCalls;controllerForwarded=controller;eventForwarded=event;return 0x12345678;}
void ScopedButton(void* scene,unsigned,char){scopedButton=menu::eventScene==scene;}
void FaultButton(void*,unsigned,char){RaiseException(0xE0420001,0,0,nullptr);}
bool GuardForwardFault(){__try{menu::ForwardButton(reinterpret_cast<void*>(1),0,0,true);return false;}__except(EXCEPTION_EXECUTE_HANDLER){return true;}}
std::array<std::uintptr_t,6> upstreamForwarded{};
void UiSubmit(void* screen){uiObserved=static_cast<unsigned char*>(screen)[125];}
void UiSubmit60(void* screen){uiObserved=static_cast<unsigned char*>(screen)[117];}
void UiFault(void*){RaiseException(0xE0420001,0,0,nullptr);}
void* D(void* p){++destructorCalls;return p;}
std::uintptr_t P(void*,void const*,char){++pointerCalls;return false;}
void BDown(void*,unsigned,char){++downCalls;}void BUp(void*,unsigned,char){++upCalls;}
void T(void*,void const*,char){++textCalls;}void W(void*,unsigned char,float,float,char){++wheelCalls;}
std::uint64_t UR(void* a,void const* k,void const* t,void const* s,void const* r,void* c){
 ++rawCalls;upstreamForwarded={reinterpret_cast<std::uintptr_t>(a),reinterpret_cast<std::uintptr_t>(k),reinterpret_cast<std::uintptr_t>(t),reinterpret_cast<std::uintptr_t>(s),reinterpret_cast<std::uintptr_t>(r),reinterpret_cast<std::uintptr_t>(c)};return 0xCAFEBABE12345678ULL;
}
}
int main(int argc,char** argv){
 menu::SharedState state;menu::NativeProfile p;menu::profile=&p;menu::shared=&state;
 menu::originalDtor=D;menu::MMB_PointerOriginal=P;menu::originalDown=BDown;menu::originalUp=BUp;menu::originalUpstreamRaw=UR;menu::originalText=T;menu::originalWheel=W;
 unsigned key=121;unsigned char type=4,keyState=1,remap=0;
 menu::NativeText text;std::memcpy(text.data,"A",2);text.length=1;
 for(bool active:{false,true}){
  menu::ready.store(active);state.menu.open=true;
  menu::DtorHook(nullptr);menu::PointerHook(nullptr,nullptr,0);menu::DownHook(nullptr,0x588FCBB0,0);menu::UpHook(nullptr,0x588FCBB0,0);
  menu::UpstreamRawHook(nullptr,&key,&type,&keyState,&remap,nullptr);menu::TextHook(nullptr,&text,0);menu::WheelHook(nullptr,2,0,1,0);
 }
 if(destructorCalls!=2 || pointerCalls!=2 || downCalls!=2 || upCalls!=2 || rawCalls!=2 || textCalls!=2 || wheelCalls!=2 || state.menu.open){std::cerr<<"Native original-call/pass-through contract failed\n";return 1;}
 int checks{},failures{};
 auto check=[&](bool ok,char const* name){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
 {
  // Exact exported I18nImpl/shared-holder/Localization layout, owned fixture.
  alignas(8) std::array<std::byte,392> i18n{};
  alignas(8) std::array<std::byte,24> holder{};
  alignas(8) std::array<std::byte,120> language{};SRWLOCK languageLock=SRWLOCK_INIT;
  auto put=[](auto& bytes,std::size_t off,auto v){std::memcpy(bytes.data()+off,&v,sizeof v);};
  put(i18n,32,holder.data());put(holder,0,&languageLock);put(holder,8,language.data());
  std::memcpy(language.data()+40,"zh_TW",6);put(language,56,std::size_t(5));put(language,64,std::size_t(15));
  menu::Locale locale=menu::Locale::English;
  check(menu::ReadLocaleSnapshot(i18n.data(),locale) && locale==menu::Locale::TraditionalChinese,"current language reader follows selected language holder");
  char heap[]="zh_CN";put(language,40,heap);put(language,64,std::size_t(31));
  check(menu::ReadLocaleSnapshot(i18n.data(),locale) && locale==menu::Locale::SimplifiedChinese,"current language snapshot supports bounded native heap strings");
  put(language,64,std::size_t(4));check(!menu::ReadLocaleSnapshot(i18n.data(),locale),"invalid native language capacity refuses");
  put(language,64,std::size_t(31));put(language,40,reinterpret_cast<char*>(1));
  check(!menu::ReadLocaleSnapshot(i18n.data(),locale),"unreadable language pointer refuses without native fault");
  check(TryAcquireSRWLockExclusive(&languageLock)!=0,"language lock is released after unreadable payload");ReleaseSRWLockExclusive(&languageLock);
  put(language,40,heap);AcquireSRWLockExclusive(&languageLock);
  check(!menu::ReadLocaleSnapshot(i18n.data(),locale),"contended language read does not stall the UI thread");ReleaseSRWLockExclusive(&languageLock);
  auto outer=reinterpret_cast<PSRWLOCK>(i18n.data()+24);
  check(TryAcquireSRWLockExclusive(outer)!=0,"outer read lock is released after inner lock contention");
  check(!menu::ReadLocaleSnapshot(i18n.data(),locale),"busy outer language lock refuses without recursively waiting");ReleaseSRWLockExclusive(outer);
  std::memcpy(heap,"en_GB",6);check(menu::ReadLocaleSnapshot(i18n.data(),locale) && locale==menu::Locale::English,"live language change can return from Chinese to English");
  std::memcpy(heap,"zh_CN",6);
  put(i18n,32,static_cast<void*>(nullptr));put(i18n,8,language.data());
  check(menu::ReadLocaleSnapshot(i18n.data(),locale) && locale==menu::Locale::SimplifiedChinese,"native fallback localization is used only when no current holder exists");
  put(language,56,std::size_t(4096));check(!menu::ReadLocaleSnapshot(i18n.data(),locale),"unbounded language length is refused");
  check(!menu::ReadLocaleSnapshot(nullptr,locale),"absent locale snapshot fails without game access");
 }
 std::array<unsigned char,192> uiScreen{};
 check(menu::WithNativeUiMode(uiScreen.data(),UiSubmit,uiScreen.data()) && uiObserved==1,"late overlay is submitted in native UI mode");
 check(uiScreen[125]==0,"native UI mode is restored after late overlay");
 uiScreen[125]=1;uiObserved=0;
 check(menu::WithNativeUiMode(uiScreen.data(),UiSubmit,uiScreen.data()) && uiObserved==1 && uiScreen[125]==1,"already active native UI mode is preserved");
 uiScreen[125]=0;
 check(!menu::WithNativeUiMode(uiScreen.data(),UiFault,nullptr) && uiScreen[125]==0,"native UI mode is restored on a native submission fault");
 check(!menu::WithNativeUiMode(nullptr,UiSubmit,nullptr),"missing screen refuses before submission");
 check(menu::WithNativeUiMode(uiScreen.data(),UiSubmit60,uiScreen.data(),117) && uiObserved==1 && uiScreen[117]==0 && uiScreen[125]==0,"60 native UI mode uses 117 and restores without touching 125");
 uiScreen[117]=1;
 check(menu::WithNativeUiMode(uiScreen.data(),UiSubmit60,uiScreen.data(),117) && uiScreen[117]==1,"60 preexisting UI mode survives submission");uiScreen[117]=0;
 check(!menu::WithNativeUiMode(uiScreen.data(),UiFault,nullptr,117) && uiScreen[117]==0,"60 UI mode restores on native fault");
 uiScreen[117]=2;
 check(!menu::WithNativeUiMode(uiScreen.data(),UiSubmit60,uiScreen.data(),117),"60 invalid flag refuses before writing");uiScreen[117]=0;
 check(!menu::WithNativeUiMode(uiScreen.data(),UiSubmit,nullptr,116),"unreviewed UI mode offset refuses");
 {
  std::array<std::byte,432> records{};
  auto put60=[](auto& b,std::size_t off,auto v){std::memcpy(b.data()+off,&v,sizeof v);};
  std::array<std::uintptr_t,3> v{reinterpret_cast<std::uintptr_t>(records.data()),reinterpret_cast<std::uintptr_t>(records.data()+216),reinterpret_cast<std::uintptr_t>(records.data()+432)};
  std::string fake=R"({"namespace":"start","main_buttons_panel":{}})";
  put60(records,0,fake.c_str());put60(records,16,fake.size());put60(records,24,fake.size());
  check(menu::AuditedDefinition60(v.data()).empty(),"60 arbitrary start namespace cannot grant definition authority");
  check(menu::AuditedDefinition60(reinterpret_cast<void const*>(1)).empty(),"60 unreadable vector fails closed");
  if(argc==2){
   std::ifstream input(argv[1]);nlohmann::json fixtures;input>>fixtures;
   for(auto name:{"start_screen.json","pause_screen.json"}){
    auto source=fixtures.at("1.26.60.05").at(name).get<std::string>();
    auto expected=std::string(name)=="start_screen.json"?"start":"pause";
    put60(records,0,source.c_str());put60(records,16,source.size());put60(records,24,source.size());
    auto before=records;
    check(menu::AuditedDefinition60(v.data())==expected,"60 exact native record and payload select audited menu");
    check(records==before,"60 native inspection does not mutate borrowed records");
    source.back()^=1;check(menu::AuditedDefinition60(v.data()).empty(),"60 modified payload refuses");source.back()^=1;
    v[1]=v[0]+432;check(menu::AuditedDefinition60(v.data()).empty(),"60 layered menu cannot authorize automatic insertion");
    v[1]=v[0]+215;check(menu::AuditedDefinition60(v.data()).empty(),"60 truncated record refuses");v[1]=v[0]+216;
    put60(records,24,std::size_t(1));check(menu::AuditedDefinition60(v.data()).empty(),"60 invalid native string capacity refuses");
    put60(records,24,source.size());put60(records,0,reinterpret_cast<char const*>(1));
    check(menu::AuditedDefinition60(v.data()).empty(),"60 unreadable payload refuses");
   }
  }
  v[1]=v[0]-1;check(menu::AuditedDefinition60(v.data()).empty(),"60 reversed vector refuses");
  v[1]=v[0]+216;v[2]=v[0]+216*257;check(menu::AuditedDefinition60(v.data()).empty(),"60 oversized capacity refuses");
 }
 auto priorRaw=state.rawEvents.load();
 menu::originalUpstreamRaw=UR;
 auto upstreamResult=menu::UpstreamRawHook(nullptr,&key,&type,&keyState,&remap,nullptr);
 check(upstreamResult==0xCAFEBABE12345678ULL && rawCalls==3,"unhandled upstream raw event preserves original result exactly once");
 check(upstreamForwarded==std::array<std::uintptr_t,6>{0,reinterpret_cast<std::uintptr_t>(&key),reinterpret_cast<std::uintptr_t>(&type),reinterpret_cast<std::uintptr_t>(&keyState),reinterpret_cast<std::uintptr_t>(&remap),0} && key==121 && type==4 && keyState==1 && remap==0,"upstream forwarding preserves all six native arguments and packet bytes");
 check(state.rawEvents.load()==priorRaw+1 && state.lastRawKey.load()==121,"upstream raw receipt is observed even without a scene recipient");
 priorRaw=state.rawEvents.load();
 menu::UpstreamRawHook(nullptr,nullptr,&type,&keyState,&remap,nullptr);
 check(rawCalls==4 && state.rawEvents.load()==priorRaw && state.faults.load()==0,"unreadable upstream packet forwards without fabricating a key");
 check(state.pointerEvents.load()==1,"active pointer receipt counted even when not consumed");
 check(state.downEvents.load()==1 && state.upEvents.load()==1,"active mapped receipt counted even on invalid scene");
 check(state.textEvents.load()==1 && state.wheelEvents.load()==1,"active text/wheel receipt counted independently of consumption");
 check(state.lastMappedId.load()==0x588FCBB0,"last native mapped ID retained as a scalar");
 check(state.lastRawKey.load()==121 && state.lastRawType.load()==4 && state.lastRawState.load()==1,"raw key tuple retained independently of eligibility");
 check(state.inputs.load()==0 && state.faults.load()==0,"passive receipt diagnostics do not consume input or fault");
 // Callback-local fixtures mirror the complete bounded fields the passive
 // reader uses. They are not native game objects and never grant permission.
 std::array<std::byte,320> context{},scene{};
 std::array<std::byte,1176> view{};std::array<std::byte,24> holder{};
 std::array<std::byte,64> meta{};std::array<std::byte,192> screen{};
 std::array<std::byte,160> gui{};unsigned char valid=1;
 auto put=[](auto& bytes,std::size_t offset,auto value){std::memcpy(bytes.data()+offset,&value,sizeof value);};
 put(scene,64,view.data());put(view,80,holder.data());put(holder,8,meta.data());
 std::memcpy(meta.data()+32,"start_screen",13);put(meta,48,std::size_t(12));put(meta,56,std::size_t(15));
 auto const& nameLayout=*menu::LayoutFor(menu::GameBuild::V52);
 check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Main,"real bounded metadata reader recognizes SSO main name");
 std::memcpy(meta.data()+32,"pause_screen",13);
 check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Pause,"real bounded metadata reader recognizes SSO pause name");
 char heapName[]="pause_screen";put(meta,32,heapName);put(meta,56,std::size_t(31));
 check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Pause,"heap-owned pause metadata follows native string representation");
 put(meta,56,std::size_t(11));check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Unknown,"invalid native capacity refuses before borrowing name");
 put(meta,32,reinterpret_cast<char*>(1));put(meta,56,std::size_t(31));check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Unknown,"unreadable native name fails closed");
 std::memcpy(meta.data()+32,"hud_screen__",13);put(meta,56,std::size_t(15));
 check(menu::ReadMenuName(view.data(),nameLayout)==menu::MenuScene::Unknown && menu::NamedMenu(scene.data())==menu::MenuScene::Unknown,"name fixture alone cannot bypass exact native scene/controller gates");
 {
  std::array<std::byte,1176> oldView{};
  auto const& oldLayout=*menu::LayoutFor(menu::GameBuild::V45);
  put(oldView,72,holder.data());std::memcpy(meta.data()+32,"start_screen",13);
  check(menu::ReadMenuName(oldView.data(),oldLayout)==menu::MenuScene::Main,"45 main name follows independently audited owner at 72");
  check(menu::ReadMenuName(oldView.data(),nameLayout)==menu::MenuScene::Unknown,"52 name layout cannot reinterpret a 45 view");
  put(oldView,72,static_cast<void*>(nullptr));put(oldView,80,holder.data());
  check(menu::ReadMenuName(oldView.data(),oldLayout)==menu::MenuScene::Unknown,"45 never falls back to newer name-owner offset");
  put(oldView,80,static_cast<void*>(nullptr));put(oldView,72,holder.data());
  char oldName[]="pause_screen";put(meta,32,oldName);put(meta,56,std::size_t(31));
  check(menu::ReadMenuName(oldView.data(),oldLayout)==menu::MenuScene::Pause,"45 heap-owned pause name uses bounded native string");
  put(meta,56,std::size_t(11));check(menu::ReadMenuName(oldView.data(),oldLayout)==menu::MenuScene::Unknown,"45 invalid name capacity also fails closed");
  put(meta,56,std::size_t(15));
 }
 std::memcpy(meta.data()+32,"start_screen",13);
 put(context,16,screen.data());put(context,56,1.f);put(context,200,(unsigned char)0);
 put(screen,152,&valid);put(screen,168,gui.data());put(gui,80,854.f);put(gui,84,469.f);
 menu::Frame frame{854,469,true,true};
 menu::ObserveRender(context.data(),scene.data(),frame);
 check(state.renderReceiptCount==1 && std::strcmp(state.renderReceipts[0].scene.data(),"start_screen")==0,"passive snapshot owns bounded scene name");
 check(state.renderReceipts[0].nativeWidth==854 && state.renderReceipts[0].nativeHeight==469 && state.renderReceipts[0].contextAlpha==1,"passive snapshot reads native viewport and alpha");
 std::memcpy(meta.data()+32,"other_screen",13); // Snapshot must not retain a borrowed char pointer.
 check(std::strcmp(state.renderReceipts[0].scene.data(),"start_screen")==0,"snapshot is independent of native source mutation");
 std::memcpy(meta.data()+32,"start_screen",13);menu::ObserveRender(context.data(),scene.data(),frame);
 check(state.renderReceiptCount==1,"identical frame snapshots deduplicate");
 frame.foreground=false;menu::ObserveRender(context.data(),scene.data(),frame);
 check(state.renderReceiptCount==2 && !state.renderReceipts[1].foreground,"focus variants remain distinguishable");
 meta[32]=std::byte{1};menu::ObserveRender(context.data(),scene.data(),frame);
 check(state.renderReceiptCount==2,"nonprintable names are refused");
 meta[32]=std::byte{'s'};put(meta,48,std::size_t(64));menu::ObserveRender(context.data(),scene.data(),frame);
 check(state.renderReceiptCount==2,"oversized name is refused before read");
 put(meta,48,std::size_t(12));frame.eligible=false;frame.observerOnly=false;menu::ObserveRender(nullptr,nullptr,frame);
 check(state.renderReceiptCount==2 && state.inputs.load()==0 && state.faults.load()==0,"unknown frames produce no snapshot or authority");
 state.renderReceiptCount=12;frame.eligible=true;menu::ObserveRender(nullptr,nullptr,frame);
 check(state.renderReceiptCount==12,"snapshot capacity stops native reads");
 menu::originalJsonParse=Parse;
 std::string definition=R"({"namespace":"start","main_button@common.button":{},"stacked_row":{"type":"panel"},"main_buttons_panel":{"controls":[{"stacked_rows":{"type":"stack_panel","controls":[{"play@start.play_button_stack":{}},{"settings@start.stacked_row":{}}]}}]}})";
 {
  std::array<std::byte,48*9> layers{};
  std::array<std::uintptr_t,3> values{reinterpret_cast<std::uintptr_t>(layers.data()),reinterpret_cast<std::uintptr_t>(layers.data()+96),reinterpret_cast<std::uintptr_t>(layers.data()+layers.size())};
  auto layer=[&](unsigned index,std::string const& text){auto off=index*48;
   put(layers,off,unsigned(7+index));put(layers,off+8,text.c_str());put(layers,off+24,text.size());put(layers,off+32,std::max<std::size_t>(16,text.size()));};
  layer(0,definition);put(layers,48+32,std::size_t(15));
  auto originalLayers=layers;auto originalValues=values;
  auto probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.validVector && probe.layers==2 && probe.inspected==2,"diagnostic observes two resource layers without authorizing them");
  check(probe.items[0].state==menu::LayerProbeState::Compatible && probe.items[0].bytes==definition.size() && probe.items[0].tag==7,"diagnostic recognizes the existing supported base layout");
  check(probe.items[1].state==menu::LayerProbeState::Empty,"diagnostic distinguishes an empty later layer");
  check(layers==originalLayers && values==originalValues && parseCalls==0 && menu::definitionLifetime==nullptr,"diagnostic never mutates native records, parses natively or creates a transform scope");
  std::string unsupported=R"({"namespace":"start","private_marker_do_not_log":1})";
  layer(1,unsupported);probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[1].state==menu::LayerProbeState::Unsupported,"diagnostic distinguishes a populated unsupported override");
  layer(1,definition);probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[1].state==menu::LayerProbeState::Compatible && probe.items[1].sameAsBase,"diagnostic recognizes byte-identical populated layers");
  probe=menu::ProbeDefinition(values.data(),"pause");
  check(probe.pause && probe.items[0].state==menu::LayerProbeState::Unsupported,"diagnostic binds layout recognition to the actual resource namespace");
  put(layers,48+8,reinterpret_cast<char const*>(1));put(layers,48+24,std::size_t(20));put(layers,48+32,std::size_t(31));
  probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[1].state==menu::LayerProbeState::Unreadable,"unreadable diagnostic payload refuses without a native exception");
  put(layers,48+24,std::size_t(32));
  probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[1].state==menu::LayerProbeState::InvalidString,"invalid native string metadata is rejected before dereferencing");
  put(layers,48+8,std::uint64_t(0x7D7B));put(layers,48+24,std::size_t(2));put(layers,48+32,std::size_t(15));
  probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[1].state==menu::LayerProbeState::Unsupported,"bounded inline native strings are observed without treating bytes as a pointer");
  values[1]=values[0]+48*9;probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.layers==9 && probe.inspected==8,"diagnostic limits per-load inspection to eight records");
  values[1]=values[0]+47;check(!menu::ProbeDefinition(values.data(),"start").validVector,"partial native records are rejected");
  values[1]=values[0]-1;check(!menu::ProbeDefinition(values.data(),"start").validVector,"reversed native vector is rejected");
  values[1]=values[0]+48*257;values[2]=values[1];check(!menu::ProbeDefinition(values.data(),"start").validVector,"excessive native vector count is rejected before reading");
  values[1]=values[0]+96;values[2]=values[0]+48;check(!menu::ProbeDefinition(values.data(),"start").validVector,"capacity below end is rejected");
  check(!menu::ProbeDefinition(nullptr,"start").validVector && !menu::ProbeDefinition(values.data(),"unknown").validVector,"absent or unrelated resources produce no payload observation");
  values[1]=values[2]=values[0]+48*3;
  std::string large=definition+std::string(900*1024,' ');layer(0,large);layer(1,large);layer(2,large);
  probe=menu::ProbeDefinition(values.data(),"start");
  check(probe.items[0].state==menu::LayerProbeState::Compatible && probe.items[1].sameAsBase && probe.items[2].state==menu::LayerProbeState::Budget,"total diagnostic payload reads stop at two MiB per load");
  values[1]=values[2]=values[0]+96;layer(0,definition);layer(1,unsupported);
  for(unsigned n=0;n<20;++n)menu::ObserveDefinition(values.data(),"start");
  check(state.definitionProbeClaims.load()==8 && state.definitionProbeReady[7].load(),"diagnostic publication has a fixed eight-load session limit");
  auto reportText=menu::DescribeDefinitionProbe(state.definitionProbes[0]);
  check(reportText.find("multiple-layers-refused")!=std::string::npos && reportText.find("compatible-layout")!=std::string::npos && reportText.find("unsupported-layout")!=std::string::npos && reportText.find("private_marker")==std::string::npos && reportText.find(definition)==std::string::npos,"worker report contains structural metadata and refusal reason, never resource text");
  check(state.faults.load()==0 && state.definitionsPatched.load()==0 && state.definitionScopes.load()==0 && !state.menu.open,"passive diagnostic cannot activate UI, transform or fault authority");
 }
 auto parseResult=menu::ParseDefinition(0x416FC0,&key,definition.c_str(),definition.size(),&type,0);
 check(parseResult==0xAB && parseCalls==1 && parseBytes==definition.data(),"unscoped native JSON calls preserve source and original return exactly once");
 {
  menu::DefinitionLifetime lifetime("start");
  parseResult=menu::ParseDefinition(0x416FC0,&key,definition.c_str(),definition.size(),&type,0);
  check(parseResult==0xAB && parseCalls==2 && !lifetime.bytes.empty() && parseBytes==lifetime.bytes.data(),"audited definition scope supplies transformed owned text to the original parser once");
  check(std::string(parseBytes,parseLength).find("button.mmb_open_mods")!=std::string::npos && parseForwarded==std::array<std::uintptr_t,3>{reinterpret_cast<std::uintptr_t>(&key),reinterpret_cast<std::uintptr_t>(&type),0},"parser output reader and flags stay native while replacement survives parse return");
  {menu::DefinitionLifetime nested({});menu::ParseDefinition(0x416FC0,nullptr,definition.c_str(),definition.size(),nullptr,0);check(parseBytes==definition.data(),"nested unrelated resource load never borrows parent replacement authority");}
  check(menu::definitionLifetime==&lifetime,"nested definition scope restores previous lifetime");
  menu::ParseDefinition(0x417691,nullptr,definition.c_str(),definition.size(),nullptr,0);check(parseBytes==definition.data(),"secondary or unrelated parser callsite remains unchanged");
  std::string pause=R"({"namespace":"pause","pause_button_template@common_buttons.light_text_button":{"size":["100%",28]},"settings_button@pause.pause_button_template":{"$pressed_button_name":"button.menu_settings"},"menu_background@pause.transparent_background":{"controls":[{"button_panel":{"type":"stack_panel","controls":[{"return_to_game_button@pause.return_to_game_button":{}},{"settings_button@pause.settings_button":{}},{"settings@pause.vertical_padding":{}},{"quit_button@pause.quit_button":{}}]}}]}})";
  menu::ParseDefinition(0x416FC0,nullptr,pause.c_str(),pause.size(),nullptr,0);
  check(parseBytes==pause.data(),"start resource scope cannot transform a pause namespace payload");
  {menu::DefinitionLifetime scopedPause("pause");menu::ParseDefinition(0x416FC0,nullptr,pause.c_str(),pause.size(),nullptr,0);check(parseBytes==scopedPause.bytes.data() && !scopedPause.bytes.empty(),"pause resource owns its transformed text until its callback completes");
   menu::ParseDefinition(0x416FC0,nullptr,definition.c_str(),definition.size(),nullptr,0);check(parseBytes==definition.data(),"pause resource scope cannot transform a start namespace payload");}
 }
 check(menu::definitionLifetime==nullptr,"definition lifetime ends only after native callback returns");
 menu::originalControllerEvent=Controller;
 check(menu::ControllerEventHook(&key,&type)==0x12345678 && controllerCalls==1 && controllerForwarded==&key && eventForwarded==&type,"unscoped controller dispatch preserves original arguments and result once");
 menu::eventScene=scene.data();
 check(menu::ControllerEventHook(&key,&type)==0x12345678 && controllerCalls==2,"scoped unknown controller still forwards original exactly once");
 menu::eventScene=nullptr;
 menu::originalDown=ScopedButton;menu::ForwardButton(&key,0,0,true);
 check(scopedButton && menu::eventScene==nullptr,"forwarded mapped event has callback-local scene authority only");
 menu::originalDown=FaultButton;
 check(GuardForwardFault() && menu::eventScene==nullptr,"scene scope restored even if native mapped dispatch faults");
 std::string installReason;
 check(menu::InstallNative(p,state,false,installReason)==menu::InstallResult::Failure && menu::profile==&p && menu::shared==&state,"input hooks require the already published definition host");
 menu::SharedState otherState;menu::minhookInitialized=true;menu::definitionReady.store(true);
 check(menu::InstallNative(p,otherState,false,installReason)==menu::InstallResult::Failure && menu::profile==&p && menu::shared==&state,"active definition callbacks cannot be rebound to another state");
 menu::minhookInitialized=false;menu::definitionReady.store(false);
 {
  menu::Frame pauseFrame{640,360,true,true,false,menu::MenuScene::Pause};menu::Menu search;search.nativeEntry=true;
  search.OpenNative(pauseFrame);search.searchFocus=true;
  menu::Event paste{menu::Kind::Key,0,0,'V',1,{}, {false,true,false,false}};
  search.Input(paste,pauseFrame);menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);
  check(search.query=="物品" && clipboardReads==1,"real paste adapter reads only after explicit Ctrl V and inserts UTF8 into focused search");
  menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);search.Input(paste,pauseFrame);menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);
  check(search.query=="物品" && clipboardReads==1,"duplicate raw receipts and repeated drains cannot read or paste twice");
  search.Input({menu::Kind::Key,0,0,'V',0},pauseFrame);search.Input(paste,pauseFrame);
  auto observer=pauseFrame;observer.eligible=false;observer.observerOnly=true;menu::PasteIfRequested(search,observer,ReadTestClipboard);
  menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);
  check(search.query=="物品物品" && clipboardReads==2,"observer adapter cannot consume top search paste ownership");
  search.Input({menu::Kind::Key,0,0,'V',0},pauseFrame);search.Input(paste,pauseFrame);search.searchFocus=false;
  menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);
  check(search.query=="物品物品" && clipboardReads==2,"lost search focus refuses clipboard read and drops the pending paste");
  search.searchFocus=true;search.Input({menu::Kind::Key,0,0,'V',0},pauseFrame);search.Input(paste,pauseFrame);
  auto background=pauseFrame;background.foreground=false;menu::PasteIfRequested(search,background,ReadTestClipboard);
  menu::PasteIfRequested(search,pauseFrame,ReadTestClipboard);
  check(search.open && !search.searchFocus && search.query=="物品物品" && clipboardReads==2,"background adapter retains page but cancels search without clipboard read on refocus");
  menu::Menu numbers;numbers.nativeEntry=true;numbers.OpenNative(pauseFrame);numbers.configuring=true;numbers.draft.valid=true;
  numbers.draft.fields=menu::SettingsSchema("AutoRestock").fields;numbers.fieldSelected=1;
  numbers.Input({menu::Kind::Key,0,0,13,1},pauseFrame);numbers.Input(paste,pauseFrame);
  menu::PasteIfRequested(numbers,pauseFrame,ReadTestClipboard);
  check(numbers.editField==1 && numbers.numberText=="6" && clipboardReads==2,"numeric Ctrl V cannot request clipboard access through actual native paste adapter");
 }
 menu::DeactivateNative();
 menu::ready.store(true);menu::definitionReady.store(true); // Simulate a late installer publication after disable.
 check(!menu::UiActive() && !menu::DefinitionsActive(),"LL disable remains a permanent barrier against late worker publication");
 std::cout<<checks<<" real native-adapter refusal/receipt/UI-scope checks / "<<failures<<" failures\n";return failures?1:0;
}
