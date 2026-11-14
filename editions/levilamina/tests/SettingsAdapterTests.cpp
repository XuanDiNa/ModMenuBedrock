#include "ModMenu/Settings.h"
#include "json.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
namespace {
namespace fs=std::filesystem;using namespace menu;using Json=nlohmann::json;
std::string Read(fs::path const& path){std::ifstream file(path,std::ios::binary);return {std::istreambuf_iterator<char>(file),{}};}
SettingField* Field(SettingsDraft& draft,char const* path){for(auto& f:draft.fields)if(f.path==path)return &f;return nullptr;}
}
// Catches missing adapters, incompatible value encodings and unsafe widening.
// Uses the real filesystem transaction code; never loads either plugin DLL.
void AdapterChecks(fs::path const& root,void(*check)(bool,char const*)){
 auto click=root/"AutoClick/AutoClick.json",jump=root/"AutoJumpReset/AutoJumpReset.json";
 std::string clickBytes=R"({"cps":14,"intervalMs":60,"stabilityMode":true,"disableWhenCursorVisible":true,"wheelInvert":false,"uiProtectionMode":1,"keySlotMap":"1=7,2=8,3=9,E=2,R=1,Z=3","uiBlockKeys":"none","requireLmbHold":false,"requiredSlot":1,"keys":{"toggle":"F8"},"enabled":false,"jitter":0,"slotMode":"input","configVersion":2,"future":{"keep":[1,false]}})";
 std::string jumpBytes=R"({"delayMs":"120-150","delayStepMs":1,"autoUnhookAfterScan":true,"overlay":{"enabled":false,"durationMs":1400,"alpha":230},"keys":{"toggle":"J","debug":"F7","scan":"F8","hooks":"F9","reloadConfig":"F5","bind":"F4","overlayToggle":"0x24","jump":"SPACE","delayDown":"[","delayUp":"]","stepDown":"-","stepUp":"+"},"future":true})";
 std::ofstream(click,std::ios::binary)<<clickBytes;std::ofstream(jump,std::ios::binary)<<jumpBytes;
 auto c=LoadSettings(root,"AutoClick"),j=LoadSettings(root,"AutoJumpReset");
 check(c.valid && c.fields.size()==11,"AutoClick exposes all eleven effective user settings");
 check(j.valid && j.fields.size()==17,"AutoJumpReset exposes actual delay and eleven parsed keybinds");
 check(Read(click)==clickBytes && Read(jump)==jumpBytes,"expanded settings discovery and draft load remain read-only");
 check(!Field(c,"/enabled") && !Field(c,"/jitter") && !Field(c,"/slotCheck") && !Field(c,"/slotMode"),"ignored or forced AutoClick fields are not editable");
 check(!Field(j,"/keys/overlayToggle"),"unparsed JSON overlay toggle is not advertised as working");
 auto interval=Field(c,"/intervalMs"),toggle=Field(c,"/keys/toggle"),map=Field(c,"/keySlotMap"),block=Field(c,"/uiBlockKeys");
 auto delay=Field(j,"/delayMs"),down=Field(j,"/keys/delayDown"),step=Field(j,"/keys/stepUp");
 check(interval && toggle && map && block && delay && down && step,"missing settings are discoverable by their real JSON paths");
 if(!interval || !toggle || !map || !block || !delay || !down || !step)return;
 interval->value=75;check(SetKey(*toggle,'1') && SettingText(*toggle)=="0x31","AutoClick digit capture uses VK encoding not numeric control code");
 map->value=std::string("R=1,E=2,Z=3,1=7");block->value=std::string("MOUSE5,TAB");
 auto saved=ApplySettings(root,c);auto cj=Json::parse(Read(click));
 check(saved.saved && cj["intervalMs"]==75 && cj["keys"]["toggle"]=="0x31" && cj["keySlotMap"]=="R=1,E=2,Z=3,1=7" && cj["uiBlockKeys"]=="MOUSE5,TAB","expanded AutoClick values persist using existing parser formats");
 check(cj["enabled"]==false && cj["jitter"]==0 && cj["slotMode"]=="input" && cj["configVersion"]==2 && cj["future"]["keep"][1]==false,"Apply preserves forced and unknown AutoClick configuration");
 delay->value=std::string("134");check(SetKey(*down,0xDB) && SettingText(*down)=="0xDB" && SetKey(*step,0xBB),"AutoJumpReset OEM keys serialize as native VK values");
 saved=ApplySettings(root,j);auto jj=Json::parse(Read(jump));
 check(saved.saved && jj["delayMs"]=="134" && jj["keys"]["delayDown"]=="0xDB" && jj["keys"]["stepUp"]=="0xBB","delay and punctuation key edits persist");
 check(jj["keys"]["overlayToggle"]=="0x24" && jj["future"]==true,"unparsed and unknown AutoJumpReset fields are preserved");
 auto bad=LoadSettings(root,"AutoJumpReset");auto original=Read(jump);
 for(auto text:{"", "-1", "5001", "1-5001", "1-2-3", "134junk", "1.5", "1\n2"}){
  Field(bad,"/delayMs")->value=std::string(text);check(!ApplySettings(root,bad).saved && Read(jump)==original,"invalid delay text cannot reach disk");
 }
 bad=LoadSettings(root,"AutoClick");original=Read(click);
 for(auto text:{"R=0","R=10","UNKNOWN=1","R=1,R=2","R=1,garbage","R=1\nE=2"}){
  Field(bad,"/keySlotMap")->value=std::string(text);check(!ApplySettings(root,bad).saved && Read(click)==original,"invalid or ambiguous slot map cannot reach disk");
 }
 bad=LoadSettings(root,"AutoClick");Field(bad,"/uiBlockKeys")->value=std::string("UNKNOWN");
 check(!ApplySettings(root,bad).saved && Read(click)==original,"unrecognized UI block key is rejected rather than ignored");
 bad=LoadSettings(root,"AutoClick");Field(bad,"/keys/toggle")->value=std::string("HOME");
 check(!ApplySettings(root,bad).saved,"AutoClick cannot save an unsupported named key alias");
 bad=LoadSettings(root,"AutoClick");auto key=Field(bad,"/keys/toggle");key->value=std::string("F25");key->options.push_back("F25");
 check(!ApplySettings(root,bad).saved,"draft metadata cannot authorize an invalid native key");
 bad=LoadSettings(root,"AutoClick");Field(bad,"/keySlotMap")->value=std::string("NUMPAD1=6,R=1");
 saved=ApplySettings(root,bad);cj=Json::parse(Read(click));
 check(saved.saved && cj["keySlotMap"]=="NUM1=6,R=1","storage normalizes edited map aliases before they reach the native parser");
 // The native AutoJumpReset parser accepts number-valued keys and delay.
 std::ofstream(jump)<<R"({"delayMs":134,"keys":{"toggle":117,"delayDown":"0xDB"},"future":1})";
 j=LoadSettings(root,"AutoJumpReset");check(j.valid,"native numeric key and numeric delay aliases load");
 if(j.valid){Field(j,"/overlay/enabled")->value=true;saved=ApplySettings(root,j);jj=Json::parse(Read(jump));
  check(saved.saved && jj["delayMs"]==134 && jj["keys"]["toggle"]==117 && !jj["keys"].contains("scan"),"unedited native numeric forms and absent keys are not rewritten");}
 std::ofstream(jump)<<R"({"delayMs":"12","keys":{"toggle":"0x00"}})";
 check(LoadSettings(root,"AutoJumpReset").valid,"supported zero key in AutoJumpReset loads as unbound");
}
