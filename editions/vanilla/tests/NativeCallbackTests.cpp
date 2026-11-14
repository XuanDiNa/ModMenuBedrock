// Compile the real adapters into a separate test EXE. No test API is exported
// by the shipping DLL and no Minecraft process is loaded or launched.
#include "../src/NativeHost.cpp"
#include <iostream>
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
void UiFault(void*){RaiseException(0xE0420001,0,0,nullptr);}
void* D(void* p){++destructorCalls;return p;}
std::uintptr_t P(void*,void const*,char){++pointerCalls;return false;}
void BDown(void*,unsigned,char){++downCalls;}void BUp(void*,unsigned,char){++upCalls;}
void T(void*,void const*,char){++textCalls;}void W(void*,unsigned char,float,float,char){++wheelCalls;}
std::uint64_t UR(void* a,void const* k,void const* t,void const* s,void const* r,void* c){
 ++rawCalls;upstreamForwarded={reinterpret_cast<std::uintptr_t>(a),reinterpret_cast<std::uintptr_t>(k),reinterpret_cast<std::uintptr_t>(t),reinterpret_cast<std::uintptr_t>(s),reinterpret_cast<std::uintptr_t>(r),reinterpret_cast<std::uintptr_t>(c)};return 0xCAFEBABE12345678ULL;
}
}
int main(){
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
 std::cout<<checks<<" real native-adapter refusal/receipt/UI-scope checks / "<<failures<<" failures\n";return failures?1:0;
}
