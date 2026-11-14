#include "ModMenu/Core.h"
#include <iostream>
#include <cmath>
#include <algorithm>
#include "ModMenu/Configure.h"
int checks{},failures{};
void Check(bool b,char const* n){++checks;if(!b){++failures;std::cerr<<"FAIL "<<n<<'\n';}}
int main(){
    using namespace menu;Frame f{640,360,true,true};Menu m;
    Check(ClassifyMenuScene("start_screen")==MenuScene::Main && ClassifyMenuScene("pause_screen")==MenuScene::Pause,"only exact main and pause names grant a scene category");
    for(auto name:{"hud_screen","inventory_screen","settings_screen","globalpause_screen","pause_screen_extra","Pause_screen","pause.pause_screen",""})
        Check(ClassifyMenuScene(name)==MenuScene::Unknown,"HUD inventory settings and near-match names are refused");
    Check(Rect{1,2,3,4}.Contains(1,2),"inclusive left/top");
    Check(!Rect{1,2,3,4}.Contains(4,3),"exclusive right");
    Placement p{0,-100,900,1};auto b=p.Bounds(640,360);
    Check(b.x==0 && b.y==336 && b.w==80 && b.h==24,"placement clamp");
    p={3,10,20,1};b=p.Bounds(640,360);Check(b.x==550 && b.y==316,"bottom-right anchor");
    m.entries={{"a","AutoRestock","1","","restocks blocks","Installed",""},{"b","MouseTweaksBedrock","2","","inventory","Installed",""}};
    m.query="RESTOCK";Check(m.Filtered()==std::vector<int>{0},"case insensitive filter");m.query="inventory";Check(m.Filtered()==std::vector<int>{1},"description filter");m.query.clear();
    auto l=Arrange(m.config,f);
    Check(m.Input({Kind::Down,l.button.x+1,l.button.y+1,1},f),"entry captures down");
    Check(!m.open,"entry does not open on down");
    Check(m.Input({Kind::Up,l.button.x+1,l.button.y+1,1},f) && m.open,"entry opens on captured up");
    Check(m.Input({Kind::Down,630,10,1},f),"modal captures outside");
    Check(m.Input({Kind::Up,630,10,1},f),"modal consumes release");
    Check(m.Input({Kind::Key,0,0,27,1},f) && !m.open,"escape closes");
    Check(m.Input({Kind::Key,0,0,27,0},f),"escape release swallowed after close");
    Check(m.Input({Kind::Key,0,0,121,1},f) && m.open,"hotkey opens");
    Check(m.Input({Kind::Key,0,0,121,1},f) && m.open,"held hotkey does not retoggle");
    m.Input({Kind::Key,0,0,121,0},f);
    m.searchFocus=true;m.Input({Kind::Text,0,0,0,0,"物品"},f);Check(m.query=="物品","UTF8 preserved");
    m.Input({Kind::Key,0,0,8,1},f);Check(m.query=="物","UTF8 backspace whole codepoint");
    m.Tick({640,360,false,true});Check(!m.open && !m.Captured(),"scene loss cancels");
    Check(!m.Input({Kind::Key,0,0,121,1},{640,360,false,true}),"unknown scene passthrough");
    m.open=true;m.Tick({640,360,true,false});Check(!m.open,"focus loss closes");
    {
        Menu sibling;sibling.Input({Kind::Key,0,0,121,1},f);
        Frame observer{640,360,false,true,true};sibling.Tick(observer);
        Check(sibling.open,"sibling render must not close the verified top main menu");
        Check(!sibling.Input({Kind::Key,0,0,121,0},observer) && sibling.open,"sibling input passes through without consuming the captured top-scene release");
        Check(sibling.Input({Kind::Key,0,0,121,0},f) && sibling.open,"top-scene hotkey release remains paired after sibling callbacks");
        sibling.Tick({640,360,false,true});Check(!sibling.open,"actual top-scene loss still closes modal after sibling callback");
        Check(!sibling.Input({Kind::Down,13,13,1},{640,360,true,false}),"unfocused visible Mods entry never consumes input");
        sibling.Input({Kind::Key,0,0,121,1},f);
        sibling.Tick({640,360,false,false,true});
        Check(!sibling.open && !sibling.BlocksMapped(),"global focus loss clears captures even when only a sibling callback arrives");
    }
    m.query.clear();m.open=true;m.Input({Kind::Wheel,0,0,0,-10000},f);Check(m.scroll==0,"short list cannot overscroll");
    m.entries.resize(100);m.Input({Kind::Wheel,0,0,0,-10000},f);Check(m.scroll<=100-l.visible,"long list scroll clamp");
    m.settings=true;m.keyCapture=true;m.Input({Kind::Key,0,0,65,1},f);Check(m.config.openKey==65 && m.dirty && !m.keyCapture,"custom hotkey capture");
    Menu drag;drag.editing=true;l=Arrange(drag.config,f);
    Check(drag.Input({Kind::Down,l.button.x+3,l.button.y+3,1},f),"edit drag captures");
    Check(drag.Input({Kind::Move,-900,999,0},f),"drag move consumed");
    drag.Input({Kind::Up,-900,999,1},f);b=drag.config.button.Bounds(f.width,f.height);
    Check(b.x==0 && b.y==336 && drag.dirty && !drag.Captured(),"drag clamp persists");
    Menu clicks;l=Arrange(clicks.config,f);clicks.open=true;
    clicks.Input({Kind::Down,l.done.x+1,l.done.y+1,1},f);
    Check(clicks.Input({Kind::Up,l.done.x+1,l.done.y+1,1},f) && !clicks.open,"done release does not click through");
    clicks.open=true;clicks.Input({Kind::Down,100,100,2},f);clicks.Input({Kind::Key,0,0,27,1},f);
    Check(clicks.Input({Kind::Up,100,100,2},f),"secondary paired release after close");
    Check(!clicks.Input({Kind::Down,l.button.x+1,l.button.y+1,1},{100,100,true,true}),"tiny viewport passes through");
    Menu edit;edit.editing=true;Check(edit.Input({Kind::Down,500,300,1},f),"position edit blocks underlying buttons");
    Check(edit.Input({Kind::Up,500,300,1},f) && !edit.open && edit.editing,"outside edit click does not open vanilla or menu");
    Check(edit.Input({Kind::Move,500,300,0},f),"edit blocks vanilla hover changes");
    Check(AsciiKey('A',false,false)=="a","search typing without vanilla textbox focus");
    Check(AsciiKey('A',true,false)=="A" && AsciiKey('A',true,true)=="a","search shift/caps");
    Check(AsciiKey('1',true,false)=="!" && AsciiKey('1',false,false)=="1","search digits");
    Check(AsciiKey(32,false,false)==" " && AsciiKey(190,false,false)==".","search spaces and punctuation");
    Check(AsciiKey(121,false,false).empty() && AsciiKey(17,false,false).empty(),"control keys never become search text");
    {
        Menu editor;editor.open=true;editor.entries={{"AutoRestock","AutoRestock","1"}};
        auto layout=Arrange(editor.config,f);
        auto click=[&](Rect r){editor.Input({Kind::Down,r.x+2,r.y+2,1},f);editor.Input({Kind::Up,r.x+2,r.y+2,1},f);};
        click(layout.configure);
        Check(editor.configuring && editor.settingsBusy && editor.settingsRequest==Menu::Request::Load,"Configure requests worker load");
        if(!editor.configuring)return 1;
        SettingsDraft draft;draft.id="AutoRestock";draft.valid=true;draft.fields={{"/enabled","Enabled",SettingType::Boolean,true},{"/preRestockThreshold","Restock below",SettingType::Integer,17,1,64}};
        auto generation=editor.settingsGeneration;editor.CompleteSettings(generation,draft);
        Check(!editor.settingsBusy && editor.draft.valid,"matching worker response opens typed draft");
        click({layout.fields.x,layout.fields.y,layout.fields.w,layout.row});
        Check(!std::get<bool>(editor.draft.fields[0].value) && editor.settingsRequest==Menu::Request::None,"editing bool never writes before Apply");
        editor.Input({Kind::Key,0,0,40,1},f);editor.Input({Kind::Key,0,0,39,1},f);
        Check(std::get<int>(editor.draft.fields[1].value)==18,"keyboard edits bounded numeric field");
        click(layout.apply);Check(editor.settingsBusy && editor.settingsRequest==Menu::Request::Apply,"only Apply queues persistence");
        auto failed=editor.draft;failed.message="Config is locked";failed.saved=false;
        editor.CompleteSettings(generation,failed);
        Check(!editor.settingsBusy && editor.settingsChanged && std::get<int>(editor.draft.fields[1].value)==18,"failed Apply retains draft and permits an explicit retry");
        click(layout.apply);failed.saved=true;editor.CompleteSettings(generation,failed);
        Check(!editor.settingsBusy && !editor.settingsChanged,"successful Apply clears pending changes");
        editor.CompleteSettings(generation,draft);click(layout.cancel);
        Check(!editor.configuring && editor.settingsRequest==Menu::Request::None,"Cancel discards draft and pending requests");
        editor.CompleteSettings(generation,draft);Check(!editor.configuring,"late worker response cannot reopen cancelled page");
        click(layout.configure);editor.Tick({640,360,true,false});Check(!editor.configuring && editor.settingsRequest==Menu::Request::None,"focus loss discards settings draft");
    }
    {
        Menu native;native.nativeEntry=true;auto layout=Arrange(native.config,f);
        Check(!native.Input({Kind::Down,layout.button.x+1,layout.button.y+1,1},f) && !native.Input({Kind::Up,layout.button.x+1,layout.button.y+1,1},f) && !native.open,"native mode has no floating hitbox over vanilla controls");
        Check(!native.OpenNative({640,360,false,true}) && !native.open,"native event cannot authorize unknown scene");
        Check(native.OpenNative(f) && native.open,"verified native button event opens catalog");
    }
    {
        Frame main{640,360,true,true,false,MenuScene::Main},pause{640,360,true,true,false,MenuScene::Pause};
        Menu native;native.nativeEntry=true;
        Check(native.OpenNative(main),"main entry opens shared native catalog");
        native.configuring=true;native.settingsRequest=Menu::Request::Load;native.draft.id="AutoRestock";
        auto generation=native.settingsGeneration;
        native.Tick(pause);
        Check(!native.open && !native.configuring && native.settingsRequest==Menu::Request::None && native.settingsGeneration>generation,"direct main-to-pause transition discards modal and unsent settings draft");
        Check(native.OpenNative(pause),"pause entry opens the same native catalog");
        Frame sibling{640,360,false,true,true};native.Tick(sibling);
        Check(native.open,"pause modal survives observer-only sibling rendering");
        Check(native.Input({Kind::Key,0,0,27,1},pause) && !native.open,"Escape closes only the pause-origin modal");
        Check(native.Input({Kind::Key,0,0,27,0},pause),"Escape release cannot leak through to native Resume");
        native.OpenNative(pause);auto layout=Arrange(native.config,pause);
        native.Input({Kind::Down,layout.done.x+1,layout.done.y+1,1},pause);
        Check(native.Input({Kind::Up,layout.done.x+1,layout.done.y+1,1},pause) && !native.open,"Done consumes release while leaving underlying pause scene intact");
        Check(!native.Input({Kind::Key,0,0,121,1},{640,360,false,true}) && !native.open,"HUD or settings rejection cannot open catalog through F10");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu shortcut;shortcut.nativeEntry=true;
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"physical shortcut edge opens pause catalog without any native raw callback");
        for(unsigned i=0;i<20;++i)shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"held physical shortcut never retoggles the catalog");
        shortcut.PollShortcut(false,pause);
        Check(shortcut.open && shortcut.BlocksMapped(),"shortcut release leaves catalog open");
        shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"second physical shortcut edge closes only the catalog");
        shortcut.PollShortcut(false,pause);
        Check(!shortcut.BlocksMapped(),"sampled close and release leave no mapped-input capture behind");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu shortcut;
        shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"shortcut already held on menu entry is not a new press");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"release then press rearms a shortcut held on entry");
        shortcut.PollShortcut(false,pause);
        shortcut.PollShortcut(false,{640,360,false,true});shortcut.PollShortcut(true,{640,360,false,true});
        Check(!shortcut.open,"HUD inventory settings and unknown top scenes cannot open via sampling");
        shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"holding shortcut across gameplay to pause cannot open it");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"new press after returning to pause still works");
        auto background=pause;background.foreground=false;shortcut.PollShortcut(true,background);
        shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"focus loss retains page and held key cannot toggle on refocus");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"fresh shortcut after foreground release closes retained page");
    }
    {
        Frame main{640,360,true,true,false,MenuScene::Main},pause{640,360,true,true,false,MenuScene::Pause};Menu shortcut;
        shortcut.PollShortcut(false,main);shortcut.PollShortcut(true,main);
        Check(shortcut.open,"same shortcut opens exact main scene");
        shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"main to pause transition cancels stale menu without a held-key reopen");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,{640,360,false,true,true});
        Check(!shortcut.open,"observer-only sibling sample grants no shortcut authority");
        shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"observer-only sibling does not swallow top menu press edge");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,{200,100,true,true,false,MenuScene::Pause});
        Check(!shortcut.open,"tiny viewport refuses sampled shortcut and clears modal");
        shortcut.PollShortcut(true,{640,360,true,true,false,MenuScene::Unknown});
        Check(!shortcut.open,"eligible flag without an exact known category cannot authorize sampling");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu shortcut;
        shortcut.PollShortcut(false,pause);
        shortcut.Input({Kind::Key,0,0,121,1},pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"native raw press followed by sampling cannot double-toggle");
        shortcut.Input({Kind::Key,0,0,121,0},pause);shortcut.PollShortcut(false,pause);
        shortcut.PollShortcut(true,pause);shortcut.Input({Kind::Key,0,0,121,1},pause);
        Check(!shortcut.open,"sampling followed by native raw press cannot double-toggle");
        shortcut.PollShortcut(false,pause);shortcut.Input({Kind::Key,0,0,121,0},pause);
        Check(!shortcut.BlocksMapped(),"mixed raw and sampled releases do not leave a stale capture");
        shortcut.config.openKey=120;shortcut.PollShortcut(true,pause);
        Check(!shortcut.open,"configured key change reprimes without treating an already-held key as new");
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open,"sampled shortcut respects configured nondefault open key");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu shortcut;shortcut.nativeEntry=true;
        shortcut.OpenNative(pause);shortcut.configuring=true;shortcut.draft.id="AutoRestock";
        shortcut.settingsChanged=true;auto generation=shortcut.settingsGeneration;
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.open && !shortcut.configuring && !shortcut.settingsChanged && shortcut.settingsGeneration>generation,"sampled shortcut cancels Configure draft back to catalog without Apply");
        shortcut.PollShortcut(false,pause);shortcut.keyCapture=true;
        shortcut.PollShortcut(true,pause);
        Check(shortcut.keyCapture && shortcut.config.openKey==121 && shortcut.open,"physical sample never fabricates a key-capture assignment");
        shortcut.keyCapture=false;shortcut.configuring=true;shortcut.captureField=0;
        shortcut.draft.fields.push_back({"/key","Key",SettingType::Key,std::string("F5"),0,0,{}});
        shortcut.PollShortcut(false,pause);shortcut.PollShortcut(true,pause);
        Check(shortcut.captureField==0 && std::get<std::string>(shortcut.draft.fields[0].value)=="F5","physical sample never assigns a plugin keybind field");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(!keys.open,"physical Escape closes catalog without a native raw callback");
        Check(keys.BlocksMapped(),"Escape closing press keeps native mapped actions blocked until release");
        sample[27]=false;keys.PollKeyboard(sample,pause);
        Check(!keys.BlocksMapped(),"Escape release clears the local keyboard capture");
        keys.OpenNative(pause);keys.settings=true;sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(keys.open && !keys.settings,"Escape from menu settings returns to catalog before closing");
        sample[27]=false;keys.PollKeyboard(sample,pause);keys.keyCapture=true;sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(keys.open && !keys.keyCapture && keys.config.openKey==0,"Escape unbinds own shortcut without closing catalog");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.keyCapture=true;
        sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(!keys.keyCapture && keys.config.openKey==117 && keys.dirty,"physical F6 rebinds menu shortcut without any native raw event");
        Check(keys.open,"captured new shortcut cannot immediately toggle the menu");
        keys.PollKeyboard(sample,pause);Check(keys.open,"holding a newly bound shortcut does not close the menu");
        sample[117]=false;keys.PollKeyboard(sample,pause);sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(!keys.open,"new shortcut works after captured key is released and pressed again");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.configuring=true;keys.draft.id="FakeSneak";keys.draft.valid=true;
        keys.draft.fields.push_back({"/toggleKey","Toggle key",SettingType::Key,std::string("F5"),0,0,{"F5","F6","NONE"}});keys.captureField=0;
        sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(keys.captureField==-1 && keys.settingsChanged && std::get<std::string>(keys.draft.fields[0].value)=="F6","physical key capture changes plugin draft using its actual allowed options");
        Check(keys.settingsRequest==Menu::Request::None && keys.configuring,"capturing a plugin key does not Apply or leave Configure");
        sample[117]=false;keys.PollKeyboard(sample,pause);keys.captureField=0;sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(keys.configuring && keys.captureField==-1,"first Escape cancels only plugin key capture");
        keys.PollKeyboard(sample,pause);Check(keys.configuring,"held Escape cannot also discard Configure on the next frame");
        sample[27]=false;keys.PollKeyboard(sample,pause);sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(keys.open && !keys.configuring && !keys.settingsChanged && keys.settingsRequest==Menu::Request::None,"second Escape discards unsaved Configure without Apply");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.configuring=true;keys.draft.valid=true;
        keys.draft.fields.push_back({"/holdKey","Hold key",SettingType::Key,std::string("LALT"),0,0,{"LALT","RALT"}});keys.captureField=0;
        sample['A']=true;keys.PollKeyboard(sample,pause);
        Check(keys.captureField==0 && !keys.settingsChanged,"unsupported plugin key stays in capture mode without changing draft");
        sample['A']=false;keys.PollKeyboard(sample,pause);sample[0xA5]=true;sample[18]=true;keys.PollKeyboard(sample,pause);
        Check(keys.captureField==-1 && std::get<std::string>(keys.draft.fields[0].value)=="RALT","side-specific Alt key can be captured without binding generic Alt first");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        sample[117]=true;keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.keyCapture=true;
        keys.PollKeyboard(sample,pause);
        Check(keys.keyCapture && keys.config.openKey==121,"key held before capture starts is not accepted as a fresh binding");
        sample[117]=false;keys.PollKeyboard(sample,pause);sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(!keys.keyCapture && keys.config.openKey==117,"preheld binding works after release and fresh press");
        sample[117]=false;keys.PollKeyboard(sample,pause);keys.keyCapture=true;
        auto background=pause;background.foreground=false;sample[118]=true;keys.PollKeyboard(sample,background);
        keys.PollKeyboard(sample,pause);
        Check(keys.open && !keys.keyCapture && keys.config.openKey==117,"background capture is cancelled without rebinding or leaving retained page");
        keys.PollKeyboard(sample,{640,360,false,true});sample[27]=true;keys.PollKeyboard(sample,{640,360,false,true});
        Check(!keys.open && keys.config.openKey==117,"unknown HUD inventory or settings samples cannot act on Escape or binding");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);
        Check(keys.CaptureMapped(0x1175B43,true,pause),"modal consumes and records a real unknown mapped press");
        sample[27]=true;keys.PollKeyboard(sample,pause);sample[27]=false;keys.PollKeyboard(sample,pause);
        Check(keys.CaptureMapped(0x1175B43,false,pause),"mapped release remains consumed after sampled Escape and physical release close modal");
        Check(!keys.CaptureMapped(0x1175B43,true,pause),"later fresh mapped press passes through once modal and release capture end");
        keys.OpenNative(pause);sample[27]=true;keys.PollKeyboard(sample,pause);
        Check(keys.CaptureMapped(0x87654321,true,pause),"mapped press arriving after sampled close is captured by held Escape");
        sample[27]=false;keys.PollKeyboard(sample,pause);
        Check(keys.CaptureMapped(0x87654321,false,pause),"late mapped release cannot reach native Resume after keyboard release");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.CaptureMapped(123,true,pause);
        Check(!keys.CaptureMapped(123,false,{640,360,false,true,true}),"observer sibling cannot consume another scene mapped release");
        sample[27]=true;keys.PollKeyboard(sample,pause);sample[27]=false;keys.PollKeyboard(sample,pause);
        Check(keys.CaptureMapped(123,false,pause),"observer sibling cannot erase top modal release ownership");
        keys.OpenNative(pause);keys.CaptureMapped(321,true,pause);keys.Tick({640,360,false,true});
        Check(!keys.CaptureMapped(321,false,pause),"scene loss discards mapped ownership rather than blocking future menus");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);
        keys.Input({Kind::Key,0,0,117,1},pause);keys.keyCapture=true;
        keys.Input({Kind::Key,0,0,117,1},pause);
        Check(keys.keyCapture && keys.config.openKey==121,"raw repeat of a preheld key cannot bind own shortcut");
        keys.Input({Kind::Key,0,0,117,0},pause);keys.Input({Kind::Key,0,0,117,1},pause);
        Check(!keys.keyCapture && keys.config.openKey==117,"fresh raw press binds own shortcut after release");
        Menu plugin;plugin.nativeEntry=true;plugin.OpenNative(pause);plugin.configuring=true;plugin.draft.valid=true;
        plugin.draft.fields.push_back({"/key","Key",SettingType::Key,std::string("F5"),0,0,{"F5","F7","NONE"}});
        plugin.Input({Kind::Key,0,0,118,1},pause);plugin.captureField=0;
        plugin.Input({Kind::Key,0,0,118,1},pause);
        Check(plugin.configuring && plugin.captureField==0 && !plugin.settingsChanged,"raw repeat of a preheld key cannot bind plugin draft");
        plugin.Input({Kind::Key,0,0,118,0},pause);plugin.Input({Kind::Key,0,0,118,1},pause);
        Check(plugin.captureField==-1 && plugin.settingsChanged && std::get<std::string>(plugin.draft.fields[0].value)=="F7","fresh raw press binds plugin draft after release");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);keys.keyCapture=true;
        sample[27]=sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(!keys.keyCapture && keys.open && keys.config.openKey==0,"Escape unbinds and wins over a simultaneous shortcut candidate");
        sample.fill(false);keys.PollKeyboard(sample,pause);keys.keyCapture=true;
        auto sibling=pause;sibling.observerOnly=true;sibling.eligible=false;
        sample[117]=true;keys.PollKeyboard(sample,sibling);
        Check(keys.keyCapture && keys.config.openKey==0,"observer sample cannot capture a binding");
        keys.PollKeyboard(sample,pause);
        Check(!keys.keyCapture && keys.config.openKey==117,"observer sample does not erase top scene binding edge");
        sample.fill(false);keys.PollKeyboard(sample,pause);keys.configuring=true;keys.settingsBusy=true;keys.draft.valid=true;
        keys.draft.fields.push_back({"/key","Key",SettingType::Key,std::string("F5"),0,0,{"F5","F6","NONE"}});keys.captureField=0;
        sample[117]=true;keys.PollKeyboard(sample,pause);
        Check(keys.captureField==0 && !keys.settingsChanged && std::get<std::string>(keys.draft.fields[0].value)=="F5","busy Configure cannot accept a sampled binding");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu keys;keys.nativeEntry=true;std::array<bool,256> sample{};
        keys.PollKeyboard(sample,pause);keys.OpenNative(pause);
        bool owned=true;for(unsigned id=0;id<257;++id)owned=keys.CaptureMapped(id,true,pause) && owned;
        sample[27]=true;keys.PollKeyboard(sample,pause);sample[27]=false;keys.PollKeyboard(sample,pause);
        Check(owned && keys.CaptureMapped(256,false,pause),"bounded mapped overflow fails closed instead of leaking an unmatched release");
        auto unfocused=pause;unfocused.foreground=false;keys.Tick(unfocused);
        Check(!keys.CaptureMapped(999,true,pause),"focus reset clears bounded mapped overflow");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.entries={{"AutoRestock","AutoRestock","1","","Restock blocks","Ready",""},{"FakeSneak","FakeSneak","1","","Edge walk","Ready",""}};
        search.PollKeyboard(sample,pause);search.OpenNative(pause);auto layout=Arrange(search.config,pause);
        search.Input({Kind::Down,layout.search.x+2,layout.search.y+2,1},pause);search.Input({Kind::Up,layout.search.x+2,layout.search.y+2,1},pause);
        for(unsigned key:{'r','e','s','t','o','c','k'}){sample[key-32]=true;search.PollKeyboard(sample,pause);sample[key-32]=false;search.PollKeyboard(sample,pause);}
        Check(search.query=="restock" && search.Filtered()==std::vector<int>{0},"physical search input filters real entries without native text or raw callbacks");
        sample[8]=true;search.PollKeyboard(sample,pause);sample[8]=false;search.PollKeyboard(sample,pause);
        Check(search.query=="restoc","physical Backspace edits the focused query");
        sample[13]=true;search.PollKeyboard(sample,pause);sample[13]=false;search.PollKeyboard(sample,pause);
        Check(search.open && !search.searchFocus,"physical Enter exits search focus without closing Mods");
        sample['A']=true;search.PollKeyboard(sample,pause);
        Check(search.query=="restoc","typing outside search focus does not change query");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.PollKeyboard(sample,pause);search.OpenNative(pause);search.searchFocus=true;
        sample[0xA0]=sample['A']=true;search.PollKeyboard(sample,pause);sample.fill(false);search.PollKeyboard(sample,pause);
        sample['B']=true;search.PollKeyboard(sample,pause,true);sample.fill(false);search.PollKeyboard(sample,pause);
        sample[16]=sample['C']=true;search.PollKeyboard(sample,pause,true);sample.fill(false);search.PollKeyboard(sample,pause);
        sample[16]=sample['1']=true;search.PollKeyboard(sample,pause);sample.fill(false);search.PollKeyboard(sample,pause);
        sample[32]=true;search.PollKeyboard(sample,pause);sample.fill(false);search.PollKeyboard(sample,pause);
        sample[190]=true;search.PollKeyboard(sample,pause);
        Check(search.query=="ABc! .","sampled search respects side-specific Shift CapsLock digits space and punctuation");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.PollKeyboard(sample,pause);search.OpenNative(pause);search.searchFocus=true;
        sample[0xA2]=sample['V']=true;search.PollKeyboard(sample,pause);
        bool requested=search.TakePasteRequest(pause);
        Check(requested && !search.TakePasteRequest(pause) && search.query.empty(),"Ctrl V requests clipboard once without inserting a literal v");
        if(requested)search.Input({Kind::Text,0,0,0,0,"物品"},pause);
        search.PollKeyboard(sample,pause);
        Check(search.query=="物品" && !search.TakePasteRequest(pause),"held Ctrl V cannot repeat UTF8 paste");
        sample.fill(false);search.PollKeyboard(sample,pause);sample[8]=true;search.PollKeyboard(sample,pause);
        Check(search.query=="物","sampled Backspace removes a whole pasted UTF8 codepoint");
        sample.fill(false);search.PollKeyboard(sample,pause);sample[17]=sample['C']=true;search.PollKeyboard(sample,pause);
        sample.fill(false);search.PollKeyboard(sample,pause);sample[18]=sample['A']=true;search.PollKeyboard(sample,pause);
        Check(search.query=="物","Ctrl and Alt chords do not leak printable search text");
        sample.fill(false);search.PollKeyboard(sample,pause);sample[17]=sample['V']=true;search.PollKeyboard(sample,pause);search.settings=true;
        Check(!search.TakePasteRequest(pause),"queued paste is discarded after search loses its page");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.PollKeyboard(sample,pause);search.OpenNative(pause);search.searchFocus=true;
        search.Input({Kind::Key,0,0,'A',1},pause);sample['A']=true;search.PollKeyboard(sample,pause);
        Check(search.query=="a","raw followed by sampled press inserts exactly one search character");
        sample.fill(false);search.PollKeyboard(sample,pause);sample['B']=true;search.PollKeyboard(sample,pause);search.Input({Kind::Key,0,0,'B',1},pause);
        Check(search.query=="ab","sampled followed by raw press inserts exactly one search character");
        search.Input({Kind::Key,0,0,'B',1},pause);
        Check(search.query=="ab","duplicate raw repeats do not double-type independently of local repeat timer");
        sample.fill(false);search.PollKeyboard(sample,pause);search.config.openKey='C';search.PollKeyboard(sample,pause);
        sample['C']=true;search.PollKeyboard(sample,pause);
        Check(search.open && search.query=="abc","focused search owns printable keys even when one is the configured shortcut");
        sample.fill(false);search.PollKeyboard(sample,pause);search.searchFocus=false;sample['C']=true;search.PollKeyboard(sample,pause);
        Check(!search.open,"same printable shortcut still closes Mods outside search focus");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        sample['A']=true;search.PollKeyboard(sample,pause);search.OpenNative(pause);search.searchFocus=true;search.PollKeyboard(sample,pause);
        Check(search.query.empty(),"key preheld before search focus cannot type");
        sample.fill(false);search.PollKeyboard(sample,pause);sample['A']=true;
        auto sibling=pause;sibling.eligible=false;sibling.observerOnly=true;search.PollKeyboard(sample,sibling);search.PollKeyboard(sample,pause);
        Check(search.query=="a","observer sibling neither types nor erases the top search key edge");
        auto unfocused=pause;unfocused.foreground=false;sample['B']=true;search.PollKeyboard(sample,unfocused);search.PollKeyboard(sample,pause);
        Check(search.open && !search.searchFocus && search.query=="a" && !search.TakePasteRequest(pause),"focus loss retains catalog but cancels text authority and held typing");
        search.Input({Kind::Text,0,0,0,0,"secret"},{640,360,false,true});
        Check(search.query=="a","HUD inventory and unknown scenes cannot accept search text");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.PollKeyboard(sample,pause,false,1000);search.OpenNative(pause);search.searchFocus=true;
        sample['A']=true;search.PollKeyboard(sample,pause,false,1010);search.PollKeyboard(sample,pause,false,1409);
        Check(search.query=="a","held text key waits for the local repeat delay");
        search.PollKeyboard(sample,pause,false,1410);search.PollKeyboard(sample,pause,false,1460);
        Check(search.query=="aaa","focused search repeats on the bounded local timer without raw callback");
        search.PollKeyboard(sample,pause,false,9000);
        Check(search.query=="aaaa","a delayed frame adds at most one repeat instead of a catch-up burst");
        sample.fill(false);search.PollKeyboard(sample,pause,false,9010);sample[8]=true;search.PollKeyboard(sample,pause,false,9020);search.PollKeyboard(sample,pause,false,9420);
        Check(search.query=="aa","held Backspace uses the same bounded repeat timer");
        sample.fill(false);search.PollKeyboard(sample,pause,false,9500);search.PollKeyboard(sample,pause,false,99999);
        Check(search.query=="aa","release cancels text repeat immediately");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;search.OpenNative(pause);search.searchFocus=true;
        search.Input({Kind::Text,0,0,0,0,std::string("\xC0\xAF",2)},pause);
        Check(search.query.empty(),"malformed UTF8 is rejected by the actual query boundary");
        search.Input({Kind::Text,0,0,0,0,std::string("a\n\tb\x7F",5)},pause);
        Check(search.query=="ab","pasted C0 and DEL controls cannot enter single-line search");
        search.query=std::string(254,'x');search.Input({Kind::Text,0,0,0,0,"物"},pause);
        Check(search.query==std::string(254,'x'),"oversized UTF8 append is refused without splitting a codepoint");
        search.Input({Kind::Text,0,0,0,0,"ab"},pause);
        Check(search.query==std::string(254,'x')+"ab","search accepts exactly its 256 byte capacity");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu search;search.nativeEntry=true;std::array<bool,256> sample{};
        search.PollKeyboard(sample,pause,false,1000);search.OpenNative(pause);auto layout=Arrange(search.config,pause);
        auto click=[&](float x,float y){search.Input({Kind::Down,x,y,1},pause);search.Input({Kind::Up,x,y,1},pause);};
        click(layout.search.x+2,layout.search.y+2);sample['A']=true;search.PollKeyboard(sample,pause,false,1010);
        click(layout.list.x+2,layout.list.y+2);click(layout.search.x+2,layout.search.y+2);search.PollKeyboard(sample,pause,false,2000);
        Check(search.query=="a","leaving and refocusing search cancels repeat even between physical samples");
        sample.fill(false);search.PollKeyboard(sample,pause,false,2010);sample[9]=true;search.PollKeyboard(sample,pause,false,2020);
        Check(search.open && !search.searchFocus,"Tab leaves search focus without closing the menu");
        sample.fill(false);search.PollKeyboard(sample,pause);search.settings=true;search.keyCapture=true;sample[17]=sample['V']=true;search.PollKeyboard(sample,pause);
        Check(!search.keyCapture && search.config.openKey=='V' && search.query=="a" && !search.TakePasteRequest(pause),"binding V with Ctrl held cannot read clipboard or edit search");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu editor;editor.nativeEntry=true;editor.OpenNative(pause);
        editor.configuring=true;editor.draft.id="AutoRestock";editor.draft.valid=true;editor.draft.fields=SettingsSchema("AutoRestock").fields;
        auto key=[&](unsigned k){editor.Input({Kind::Key,0,0,int(k),1},pause);editor.Input({Kind::Key,0,0,int(k),0},pause);};
        auto click=[&](Rect r){editor.Input({Kind::Down,r.x+2,r.y+2,1},pause);editor.Input({Kind::Up,r.x+2,r.y+2,1},pause);};
        editor.fieldSelected=1;key(13);key('1');key('6');
        Check(editor.editField==1 && editor.numberText=="16" && std::get<int>(editor.draft.fields[1].value)==6,"numeric typing replaces old display but does not commit a partial value");
        key(13);Check(editor.editField==-1 && std::get<int>(editor.draft.fields[1].value)==16 && editor.settingsChanged,"Enter commits a complete typed integer to draft only");
        key(13);key('9');key('9');click(Arrange(editor.config,pause).apply);
        Check(!editor.settingsBusy && editor.settingsRequest==Menu::Request::None && editor.editField==1 && !editor.numberError.empty(),"invalid active number refuses Apply instead of saving the old value");
        key(27);Check(editor.configuring && editor.editField==-1 && std::get<int>(editor.draft.fields[1].value)==16,"first Escape cancels numeric buffer without discarding page draft");
        click(ArrangeSettings(editor,pause).rows[1].reset);
        Check(std::get<int>(editor.draft.fields[1].value)==6 && editor.settingsRequest==Menu::Request::None,"field Reset restores actual default without any save request");
        key(27);Check(!editor.configuring && editor.open && editor.settingsRequest==Menu::Request::None,"second fresh Escape cancels Configure and returns only to Mods");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu editor;editor.nativeEntry=true;std::array<bool,256> sample{};
        editor.PollKeyboard(sample,pause);editor.OpenNative(pause);editor.configuring=true;editor.draft.id="AutoRestock";
        editor.draft.valid=true;editor.draft.fields=SettingsSchema("AutoRestock").fields;
        auto press=[&](unsigned k){sample[k]=true;editor.PollKeyboard(sample,pause);sample[k]=false;editor.PollKeyboard(sample,pause);};
        press(40);press(13);press('3');press('2');press(13);
        Check(editor.fieldSelected==1 && std::get<int>(editor.draft.fields[1].value)==32,"physical fallback handles Configure navigation and direct numbers without raw callbacks");
        editor.Input({Kind::Key,0,0,39,1},pause);sample[39]=true;editor.PollKeyboard(sample,pause);
        Check(std::get<int>(editor.draft.fields[1].value)==33,"raw then sampled Configure adjustment occurs once per physical press");
        sample[39]=false;editor.PollKeyboard(sample,pause);sample[39]=true;editor.PollKeyboard(sample,pause);editor.Input({Kind::Key,0,0,39,1},pause);
        Check(std::get<int>(editor.draft.fields[1].value)==34,"sampled then raw Configure adjustment also occurs once");
        sample.fill(false);editor.PollKeyboard(sample,pause);press(13);press('9');press(27);
        Check(editor.configuring && editor.editField==-1,"physical Escape cancels number editing before leaving Configure");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu editor;editor.nativeEntry=true;editor.OpenNative(pause);
        editor.configuring=true;editor.draft.id="MouseTweaksBedrock";editor.draft.valid=true;editor.draft.fields=SettingsSchema("MouseTweaksBedrock").fields;
        auto click=[&](Rect r){editor.Input({Kind::Down,r.x+2,r.y+2,1},pause);editor.Input({Kind::Up,r.x+2,r.y+2,1},pause);};
        auto categories=SettingCategories(editor.draft);
        Check(categories.size()==5 && categories[0]=="All","source-backed categories include All and unique groups");
        editor.settingCategory=int(std::find(categories.begin(),categories.end(),"Mouse wheel")-categories.begin());
        Check(VisibleSettings(editor)==std::vector<int>({5,6,7,8}),"category filtering preserves backing field indices");
        auto layout=ArrangeSettings(editor,pause);click(layout.rows[2].value);
        Check(editor.choiceField==7 && !editor.settingsChanged,"choice control opens popup without changing draft");
        layout=ArrangeSettings(editor,pause);click({layout.popup.x,layout.popup.y+layout.optionHeight*3,layout.popup.w,layout.optionHeight});
        Check(editor.choiceField==-1 && std::get<int>(editor.draft.fields[7].value)==3,"popup captured release chooses the clicked option");
        click(ArrangeSettings(editor,pause).rows[2].value);editor.Input({Kind::Key,0,0,27,1},pause);editor.Input({Kind::Key,0,0,27,0},pause);
        Check(editor.configuring && editor.choiceField==-1,"Escape closes choice popup without leaving Configure");
        click(ArrangeSettings(editor,pause).resetAll);
        Check(std::get<int>(editor.draft.fields[7].value)==0 && editor.settingsRequest==Menu::Request::None,"Reset all restores fields outside the active category without saving");
        click(ArrangeSettings(editor,pause).next);
        Check(editor.settingCategory!=int(std::find(categories.begin(),categories.end(),"Mouse wheel")-categories.begin()) && editor.fieldScroll==0,"category controls reset scrolling without altering storage");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu editor;editor.nativeEntry=true;editor.OpenNative(pause);editor.configuring=true;
        editor.draft.id="MouseTweaksBedrock";editor.draft.valid=true;editor.draft.fields=SettingsSchema(editor.draft.id).fields;
        editor.fieldSelected=7;
        auto key=[&](unsigned k){editor.Input({Kind::Key,0,0,int(k),1},pause);editor.Input({Kind::Key,0,0,int(k),0},pause);};
        key(13);auto c=ArrangeSettings(editor,pause);auto apply=Arrange(editor.config,pause).apply;
        editor.Input({Kind::Down,apply.x+2,apply.y+2,1},pause);editor.Input({Kind::Up,apply.x+2,apply.y+2,1},pause);
        Check(editor.choiceField==-1 && editor.settingsRequest==Menu::Request::None,"outside popup click dismisses it without activating underlying Apply");
        key(13);key(40);key(40);key(13);
        Check(std::get<int>(editor.draft.fields[7].value)==2 && editor.choiceField==-1,"choice popup supports keyboard selection and commit");
        editor.Tick({640,360,true,false,false,MenuScene::Pause});
        Check(editor.configuring && editor.choiceField==-1 && editor.editField==-1 && std::get<int>(editor.draft.fields[7].value)==2,"focus loss cancels shared-control state and retains unsent draft");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu editor;editor.nativeEntry=true;editor.OpenNative(pause);editor.configuring=true;
        editor.draft.valid=true;editor.draft.fields=SettingsSchema("AutoRestock").fields;editor.fieldSelected=1;
        auto key=[&](unsigned k){editor.Input({Kind::Key,0,0,int(k),1},pause);editor.Input({Kind::Key,0,0,int(k),0},pause);};
        key(13);key('9');
        editor.Input({Kind::Key,0,0,27,1},pause);editor.Input({Kind::Key,0,0,27,1},pause);
        Check(editor.configuring && editor.editField==-1,"held Escape after cancelling number buffer cannot also close Configure");
        editor.Input({Kind::Key,0,0,27,0},pause);key(13);key(46);key(13);
        Check(editor.editField==1 && !editor.numberError.empty(),"empty numeric editor cannot commit or silently become zero");
        editor.Input({Kind::Text,0,0,0,0,"物品"},pause);
        Check(editor.numberText.empty(),"native nonnumeric text cannot enter an integer draft");
        key('3');key('2');auto apply=Arrange(editor.config,pause).apply;
        editor.Input({Kind::Down,apply.x+2,apply.y+2,1},pause);editor.Input({Kind::Up,apply.x+2,apply.y+2,1},pause);
        Check(editor.settingsBusy && editor.settingsRequest==Menu::Request::Apply && std::get<int>(editor.draft.fields[1].value)==32,"Apply validates active numeric buffer and queues its new value without extra Enter");
        key('9');Check(std::get<int>(editor.draft.fields[1].value)==32,"busy Apply cannot edit the submitted draft");
    }
    {
        Frame pause{640,360,true,true,false,MenuScene::Pause};Menu catalog;catalog.nativeEntry=true;catalog.OpenNative(pause);
        catalog.entries={{"AutoRestock","AutoRestock","1"},{"FakeSneak","FakeSneak","1"},{"ContainerPreview","ContainerPreview","1"}};
        auto layout=Arrange(catalog.config,pause);
        // Screenshot-style rows have room for a tile, name, summary and status.
        float x=layout.list.x+40,y=layout.list.y+60;
        catalog.Input({Kind::Down,x,y,1},pause);catalog.Input({Kind::Up,x,y,1},pause);
        Check(catalog.selected==1,"clicking the second tall catalog row selects its actual displayed entry");
        catalog.Input({Kind::Down,layout.configure.x+2,layout.configure.y+2,1},pause);
        catalog.Input({Kind::Up,layout.configure.x+2,layout.configure.y+2,1},pause);
        Check(catalog.configuring && catalog.draft.id=="FakeSneak" && catalog.settingsRequest==Menu::Request::Load,"header Configure action loads the selected entry and cannot hit another row");
    }
    {
        Frame compact{240,160,true,true,false,MenuScene::Main};Menu catalog;catalog.nativeEntry=true;catalog.OpenNative(compact);
        catalog.entries={{"AutoRestock","AutoRestock","1"},{"FakeSneak","FakeSneak","1"},{"ContainerPreview","ContainerPreview","1"}};
        // Only one complete 32px row fits; the remaining strip is not a row.
        catalog.Input({Kind::Down,48,110,1},compact);catalog.Input({Kind::Up,48,110,1},compact);
        Check(catalog.selected==0,"blank strip below the last fully rendered catalog row cannot select an invisible entry");
    }
    {
        Frame frame{640,360,true,true,false,MenuScene::Main};Menu catalog;catalog.nativeEntry=true;catalog.OpenNative(frame);
        Entry entry{"ExternalPlugin","ExternalPlugin","1.0.0"};entry.settingsAvailable=true;catalog.entries={entry};
        auto r=Arrange(catalog.config,frame).configure;
        catalog.Input({Kind::Down,r.x+2,r.y+2,1},frame);catalog.Input({Kind::Up,r.x+2,r.y+2,1},frame);
        Check(catalog.configuring && catalog.settingsBusy && catalog.settingsRequest==Menu::Request::Load && catalog.draft.id=="ExternalPlugin","worker-published external Configure support queues normal load for an unknown plugin");
        Menu refused;refused.nativeEntry=true;refused.OpenNative(frame);entry.id="AutoRestock";entry.settingsAvailable=false;refused.entries={entry};
        refused.Input({Kind::Down,r.x+2,r.y+2,1},frame);refused.Input({Kind::Up,r.x+2,r.y+2,1},frame);
        Check(!refused.configuring && refused.settingsRequest==Menu::Request::None,"explicit worker refusal overrides built-in adapter Configure input");
    }
    std::cout<<checks<<" checks / "<<failures<<" failures\n";return failures?1:0;
}
