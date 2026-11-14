#include <array>
#include <cstdlib>
#include <iostream>
#include <fstream>
#if __has_include("../integration/sdk/ModMenuBinding.h")
#include "../integration/sdk/ModMenuBinding.h"
#include "ModMenu/Core.h"
#include "ModMenu/Storage.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
namespace b=modmenu::binding;
static unsigned checks{};
static void Check(bool ok,char const* text){++checks;if(!ok){std::cerr<<"FAIL: "<<text<<'\n';std::exit(1);}}
int main(int argc,char** argv){
 if(argc==2&&std::string_view(argv[1])=="--schema-cases"){
  nlohmann::json cases=nlohmann::json::array();
  auto add=[&](std::string const& value){auto parsed=b::Parse(value);cases.push_back({{"value",value},{"valid",parsed&&b::Token(*parsed)==value},{"v4",parsed&&b::Token(*parsed)==value&&parsed->modifiers<4}});};
  for(unsigned k=0;k<256;++k)for(unsigned mask=0;mask<8;++mask){
   std::string prefix=(mask&b::Ctrl?"CTRL+":"");if(mask&b::Shift)prefix+="SHIFT+";if(mask&4)prefix+="LALT+";
   add(prefix+b::HexToken(k));auto token=b::Token({k,mask});if(!token.empty())add(token);
  }
  for(auto s:{"NONE","CTRL+NONE","SHIFT+CTRL+K","ctrl+k","CTRL+LCTRL","ESC","LBUTTON","CTRL+","+K","K+L","CTRL+CTRL+K","ALT+K","","VK_FF"})add(s);
  std::cout<<cases.dump();return 0;
 }
 // Left Alt extension: red first, then run against the shared implementation.
 for(auto value:{"LALT+K","CTRL+LALT+K","SHIFT+LALT+K","CTRL+SHIFT+LALT+K"}){
  auto alt=b::Parse(value);Check(alt.has_value(),"Left Alt chord parses");Check(b::Token(*alt)==value,"Left Alt canonical round trip");
  b::Snapshot physical{};b::Tracker t;t.Update(*alt,physical,true);
  physical[165]=true;physical[162]=(alt->modifiers&1)!=0;physical[160]=(alt->modifiers&2)!=0;physical['K']=true;
  Check(!t.Update(*alt,physical,true).held,"Right Alt never satisfies Left Alt chord");
  physical['K']=false;t.Update(*alt,physical,true);physical[165]=false;physical[164]=true;physical['K']=true;
  Check(t.Update(*alt,physical,true).pressed,"Left Alt chord triggers on primary edge");
  Check(!t.Update(*alt,physical,true).pressed,"Left Alt held never repeats");
  physical[164]=false;Check(!t.Update(*alt,physical,true).held,"release Left Alt ends hold");
  physical[164]=true;Check(!t.Update(*alt,physical,true).held,"late Left Alt cannot resume hold");
  t.Update(*alt,physical,false);Check(!t.Update(*alt,physical,true).held,"refocus with Alt chord held blocked");
  b::Capture c;physical={};c.Begin(physical);physical[164]=true;Check(!c.Event(164,true,physical),"Left Alt waits for primary");
  physical[162]=(alt->modifiers&1)!=0;physical[160]=(alt->modifiers&2)!=0;physical['K']=true;
  auto got=c.Event('K',true,physical);Check(got&&*got==*alt,"capture full Left Alt chord");
 }
 {b::Capture c;b::Snapshot s{};c.Begin(s);s[164]=true;c.Event(164,true,s);s[164]=false;auto got=c.Event(164,false,s);Check(got&&b::Token(*got)=="LALT","standalone Left Alt remains bindable on release");}
 Check(b::Parse("CTRL+LALT").has_value(),"old Ctrl plus Left Alt primary remains readable");
 for(auto value:{"LALT+NONE","LALT+LALT","LALT+CTRL","LALT+ALT","RALT+K","LALT+CTRL+K","LALT+LALT+K"})Check(!b::Parse(value),"malformed Left Alt binding refused");
 for(auto s:{"NONE","A","0","F24","BACKSPACE","TAB","ENTER","OEM_PLUS","NUMPAD0","LWIN","BROWSER_BACK","VOLUME_UP","LCTRL","CTRL+K","SHIFT+K","CTRL+SHIFT+K"}){
  auto v=b::Parse(s);Check(v.has_value(),s);Check(b::Token(*v)==s,"canonical round trip");
 }
 for(auto s:{"ESC","ESCAPE","LBUTTON","MOUSE4","VK_01","VK_1B","VK_FF","VK_07","VK_3A","CTRL+NONE","CTRL+CTRL+K","SHIFT+CTRL+K","ALT+K","CTRL+LCTRL","CTRL+","+K","K+L","ctrl+k"})Check(!b::Parse(s),s);
 Check(b::Parse("VK_E9")->key==0xE9,"assigned fallback accepted");
 for(unsigned k=0;k<256;++k){if(b::Allowed(k)){auto t=b::Token({k,0});auto p=b::Parse(t);Check(p&&p->key==k,"every allowed code round trips");}else Check(!b::Parse(b::HexToken(k)),"forbidden codes cannot use fallback");}
 b::Snapshot keys{};b::Tracker tracker;auto chord=*b::Parse("CTRL+K");
 Check(!tracker.Update(chord,keys,true).pressed,"released sample primes");
 keys['K']=true;Check(!tracker.Update(chord,keys,true).pressed,"bare primary does not match chord");
 keys[0xA2]=true;auto late=tracker.Update(chord,keys,true);Check(!late.pressed&&!late.held,"late modifier does not fabricate press or hold");
 keys['K']=false;tracker.Update(chord,keys,true);keys['K']=true;
 Check(tracker.Update(chord,keys,true).pressed,"modifier before fresh primary triggers");
 Check(!tracker.Update(chord,keys,true).pressed,"held key never repeats");
 keys[0xA0]=true;Check(!tracker.Update(chord,keys,true).held,"extra modifier prevents exact match");
 tracker.Update(chord,keys,false);keys[0xA0]=false;
 Check(!tracker.Update(chord,keys,true).held,"refocus with primary held is blocked");
 keys['K']=false;tracker.Update(chord,keys,true);keys['K']=true;Check(tracker.Update(chord,keys,true).pressed,"fresh press after refocus works");
 Check(!tracker.Update(*b::Parse("K"),keys,true).pressed,"config change with held key is blocked");
 b::Capture capture;keys={};capture.Begin(keys);keys[0xA2]=true;Check(!capture.Event(0xA2,true,keys),"Ctrl waits for primary");
 keys['K']=true;auto captured=capture.Event('K',true,keys);Check(captured&&b::Token(*captured)=="CTRL+K","capture chord");
 keys={};capture.Begin(keys);keys[0xA0]=true;capture.Event(0xA0,true,keys);keys[0xA0]=false;
 captured=capture.Event(0xA0,false,keys);Check(captured&&b::Token(*captured)=="LSHIFT","standalone modifier captured on release");
 keys={};capture.Begin(keys);keys[8]=true;captured=capture.Event(8,true,keys);Check(captured&&captured->key==8,"Backspace is bindable");
 keys={};capture.Begin(keys);Check(!capture.Event(1,true,keys),"mouse ignored");captured=capture.Event(27,true,keys);Check(captured&&captured->key==0,"Esc unbinds");
 keys={};keys['K']=true;capture.Begin(keys);Check(!capture.Event('K',true,keys),"preheld key blocked");keys['K']=false;capture.Event('K',false,keys);keys['K']=true;Check(capture.Event('K',true,keys).has_value(),"fresh capture after release");
 using namespace menu;
 auto description=R"({"schemaVersion":4,"plugin":{"id":"Example","entry":"Example.dll","versions":["1"]},"config":{"file":"config.json","reload":"hot"},"fields":[{"path":"/key","type":"binding","default":"CTRL+K","label":"Key"}]})";
 try{auto def=ParseSettingsDescription(description,"Example","Example.dll","1");Check(def.fields.size()==1&&ValidSettingValue(def.fields[0]),"v4 binding is opt-in");}catch(...){Check(false,"v4 binding description accepted");}
 auto v5=nlohmann::json::parse(description);v5["schemaVersion"]=5;v5["fields"][0]["default"]="LALT+K";
 SettingsDefinition altDef;
 try{altDef=ParseSettingsDescription(v5.dump(),"Example","Example.dll","1");Check(ValidSettingValue(altDef.fields[0]),"v5 Left Alt default accepted");}catch(...){Check(false,"v5 opt-in description accepted");}
 {auto old=nlohmann::json::parse(description);old["fields"][0]["default"]="LALT+K";bool refused=false;try{ParseSettingsDescription(old.dump(),"Example","Example.dll","1");}catch(...){refused=true;}Check(refused,"v4 cannot opt into Left Alt chords");}
 {auto old=ParseSettingsDescription(description,"Example","Example.dll","1");Check(!SetBinding(old.fields[0],{'K',4}),"v4 SetBinding refuses new modifier");old.fields[0].value="LALT+K";Check(!ValidSettingValue(old.fields[0]),"v4 config rejects Left Alt chord");}
 for(bool rawFirst:{false,true}){
  Frame af{640,360,true,true,false,MenuScene::Pause};Menu am;b::Snapshot ak{};am.PollKeyboard(ak,af);am.OpenNative(af);am.configuring=true;am.draft.valid=true;am.draft.fields=altDef.fields;am.captureField=0;
  ak[164]=true;if(rawFirst)am.Input({Kind::Key,0,0,164,1,{},Modifiers{false,false,true,false,true}},af);am.PollKeyboard(ak,af);
  Check(am.captureField==0,"real field Left Alt waits for primary");Check(am.BindingCaptureCaption()=="LAlt+...","real field shows Left Alt pending");
  ak['K']=true;if(rawFirst)am.Input({Kind::Key,0,0,'K',1,{},Modifiers{false,false,true,false,true}},af);am.PollKeyboard(ak,af);
  Check(am.captureField==-1&&SettingText(am.draft.fields[0])=="LALT+K","real field captures Left Alt once");
  am.configuring=false;am.settings=true;ak={};am.PollKeyboard(ak,af);am.keyCapture=true;ak[164]=true;am.PollKeyboard(ak,af);ak['K']=true;am.PollKeyboard(ak,af);
  Check(!am.keyCapture&&am.config.openKey=='K'&&am.config.openModifiers==4,"menu open binding accepts Left Alt chord");
  am.settings=false;am.open=false;ak={};am.PollKeyboard(ak,af);ak[165]=true;am.PollKeyboard(ak,af);ak['K']=true;
  if(rawFirst)am.Input({Kind::Key,0,0,'K',1,{},Modifiers{false,false,true,false,false}},af);am.PollKeyboard(ak,af);Check(!am.open,"Right Alt cannot open Left Alt menu chord");
  ak={};am.PollKeyboard(ak,af);ak[164]=true;am.PollKeyboard(ak,af);ak['K']=true;if(rawFirst)am.Input({Kind::Key,0,0,'K',1,{},Modifiers{false,false,true,false,true}},af);am.PollKeyboard(ak,af);
  Check(am.open,"raw and sampled Left Alt open menu once");
 }
 Frame frame{640,360,true,true,false,MenuScene::Pause};Menu menu;keys={};menu.PollKeyboard(keys,frame);menu.OpenNative(frame);menu.settings=true;menu.keyCapture=true;
 keys[0xA2]=true;menu.PollKeyboard(keys,frame);Check(menu.keyCapture,"menu capture waits for Ctrl");
 Check(menu.BindingCaptureCaption()=="Ctrl+...","menu displays pending modifier");
 keys['K']=true;menu.PollKeyboard(keys,frame);Check(!menu.keyCapture&&menu.config.openKey=='K',"menu captures chord primary");
 keys={};menu.PollKeyboard(keys,frame);menu.open=false;keys['K']=true;menu.PollKeyboard(keys,frame);Check(!menu.open,"bare K cannot open chord menu");
 keys['K']=false;menu.PollKeyboard(keys,frame);keys[0xA2]=true;menu.PollKeyboard(keys,frame);keys['K']=true;menu.PollKeyboard(keys,frame);Check(menu.open,"Ctrl K opens menu on fresh primary");
 Check(menu.config.openModifiers==b::Ctrl,"menu stores required modifiers");
 keys={};menu.PollKeyboard(keys,frame);menu.configuring=true;menu.settings=false;menu.draft.valid=true;
 auto def=ParseSettingsDescription(description,"Example","Example.dll","1");menu.draft.fields=def.fields;menu.captureField=0;
 keys[8]=true;menu.PollKeyboard(keys,frame);Check(menu.captureField==-1&&SettingText(menu.draft.fields[0])=="BACKSPACE","real binding field accepts Backspace");
 keys={};menu.PollKeyboard(keys,frame);menu.captureField=0;keys[0xA0]=true;menu.PollKeyboard(keys,frame);keys['K']=true;menu.PollKeyboard(keys,frame);
 Check(menu.captureField==-1&&SettingText(menu.draft.fields[0])=="SHIFT+K","real binding field captures chord");
 keys={};menu.PollKeyboard(keys,frame);menu.captureField=0;keys[27]=true;menu.PollKeyboard(keys,frame);Check(menu.configuring&&menu.captureField==-1&&SettingText(menu.draft.fields[0])=="NONE","Esc only clears binding and stays Configure");
 keys={};menu.PollKeyboard(keys,frame);menu.captureField=0;keys[0xA2]=true;menu.PollKeyboard(keys,frame);auto background=frame;background.foreground=false;menu.PollKeyboard(keys,background);menu.PollKeyboard(keys,frame);
 Check(menu.configuring&&menu.captureField==-1&&SettingText(menu.draft.fields[0])=="NONE","focus loss cancels pending chord without applying");
 keys={};menu.PollKeyboard(keys,frame);menu.captureField=0;
 keys[0xA2]=true;menu.PollKeyboard(keys,frame);keys[0xA2]=false;menu.PollKeyboard(keys,frame);
 Check(menu.captureField==-1&&SettingText(menu.draft.fields[0])=="LCTRL","standalone modifier completes on sampled release");
 keys={};menu.PollKeyboard(keys,frame);menu.captureField=0;
 keys[0xA2]=true;menu.Input({Kind::Key,0,0,0xA2,1,{},Modifiers{false,true}},frame);menu.PollKeyboard(keys,frame);
 keys['K']=true;menu.Input({Kind::Key,0,0,'K',1,{},Modifiers{false,true}},frame);menu.PollKeyboard(keys,frame);
 Check(menu.captureField==-1&&SettingText(menu.draft.fields[0])=="CTRL+K"&&menu.configuring,"raw then sampled chord captures once");
 keys={};menu.PollKeyboard(keys,frame);menu.Input({Kind::Key,0,0,'K',1},frame);
 Check(menu.configuring,"bare primary of menu chord does not close Configure");
 menu.Input({Kind::Key,0,0,'K',0},frame);menu.Input({Kind::Key,0,0,'K',1,{},Modifiers{false,true}},frame);
 Check(!menu.configuring&&menu.open,"complete menu chord returns Configure to Mods");
 namespace fs=std::filesystem;auto root=fs::temp_directory_path()/("ModMenuBinding-"+std::to_string(GetCurrentProcessId()));fs::create_directories(root/"Example");
 struct Cleanup{fs::path root;~Cleanup(){std::error_code ec;fs::remove_all(root,ec);}}cleanup{root};
 auto write=[](auto const& p,auto const& v){std::ofstream(p,std::ios::binary)<<v;};
 write(root/"Example/manifest.json",R"({"entry":"Example.dll","version":"1"})");write(root/"Example/modmenu.settings.json",description);write(root/"Example/config.json",R"({"key":"CTRL+K","other":{"keep":7}})");
 auto draft=LoadSettings(root,"Example");Check(draft.valid,"v4 config loads through actual settings boundary");SetBinding(draft.fields[0],{'K',b::Ctrl|b::Shift});auto saved=ApplySettings(root,draft);Check(saved.saved,"v4 Apply saves");
 auto loaded=LoadSettings(root,"Example");Check(loaded.valid&&SettingText(loaded.fields[0])=="CTRL+SHIFT+K"&&loaded.original.find("keep")!=std::string::npos,"Apply reload preserves binding and unknown config");
 auto legacy=nlohmann::json::parse(description);legacy["fields"][0]["legacyNumericStrings"]=true;legacy["fields"][0]["default"]="7";
 write(root/"Example/modmenu.settings.json",legacy.dump());
 for(auto raw:{R"({"key":56,"keep":7})",R"({"key":"56","keep":7})"}){
  write(root/"Example/config.json",raw);auto old=LoadSettings(root,"Example");
  Check(old.valid&&SettingKeyCaption(old.fields[0])=="8","legacy numeric JSON and numeric string keep VK meaning");
  Check(ApplySettings(root,old).saved,"unedited legacy value saves without reinterpretation");
  auto preserved=nlohmann::json::parse(std::ifstream(root/"Example/config.json"));
  Check(preserved["key"]==nlohmann::json::parse(raw)["key"],"unedited legacy number or string representation remains unchanged");
  old=LoadSettings(root,"Example");ResetSetting(old.fields[0]);Check(SettingText(old.fields[0])=="VK_37","canonical digit default uses unambiguous legacy encoding");
  Check(ApplySettings(root,old).saved,"legacy digit reset saves");
  auto parsed=nlohmann::json::parse(std::ifstream(root/"Example/config.json"));
  Check(parsed["key"]=="VK_37"&&parsed["keep"]==7,"legacy digit edit preserves unrelated values");
 }
 write(root/"Example/modmenu.settings.json",v5.dump());write(root/"Example/config.json",R"({"key":"LALT+K","keep":7})");
 auto altDraft=LoadSettings(root,"Example");Check(altDraft.valid,"v5 config load");Check(SetBinding(altDraft.fields[0],{'K',7}),"v5 all modifiers edit");Check(ApplySettings(root,altDraft).saved,"v5 Apply");
 Check(SettingText(LoadSettings(root,"Example").fields[0])=="CTRL+SHIFT+LALT+K","v5 all modifiers persisted");
 Config altCfg;altCfg.openKey='K';altCfg.openModifiers=7;Check(SaveConfig(root/"alt.json",altCfg),"own Left Alt chord saves");Config altRecovered;Check(LoadConfig(root/"alt.json",altRecovered)&&altRecovered.openModifiers==7,"own Left Alt chord persists");
 Config cfg;cfg.openKey='K';cfg.openModifiers=3;Check(SaveConfig(root/"own.json",cfg),"own chord saves");Config recovered;Check(LoadConfig(root/"own.json",recovered)&&recovered.openModifiers==3&&recovered.openKey=='K',"own chord persists");
 for(auto value:{"CTRL+NONE","VK_01","ESC","SHIFT+CTRL+K"}){auto j=nlohmann::json::parse(description);j["fields"][0]["default"]=value;bool refused=false;try{ParseSettingsDescription(j.dump(),"Example","Example.dll","1");}catch(...){refused=true;}Check(refused,"malformed or forbidden binding default refuses");}
 std::cout<<"Binding tests: "<<checks<<" passed\n";
}
#else
int main(){std::cerr<<"FAIL: public keyboard binding SDK is missing\n";return 1;}
#endif
