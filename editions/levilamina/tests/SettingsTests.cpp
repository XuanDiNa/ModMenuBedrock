#include "ModMenu/Settings.h"
#include "json.hpp"
#include <fstream>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <crtdbg.h>
namespace fs=std::filesystem;
int checks{},failures{};
void TransactionChecks(void(*)(bool,char const*));
void ExternalSettingsChecks(void(*)(bool,char const*));
void AdapterChecks(fs::path const&,void(*)(bool,char const*));
void Check(bool b,char const* n){++checks;if(!b){++failures;std::cerr<<"FAIL "<<n<<'\n';}}
std::string Read(fs::path const& p){std::ifstream s(p,std::ios::binary);return {std::istreambuf_iterator<char>(s),{}};}
int main(){using namespace menu;
 _CrtSetReportMode(_CRT_ASSERT,_CRTDBG_MODE_FILE);_CrtSetReportFile(_CRT_ASSERT,_CRTDBG_FILE_STDERR);
 auto root=fs::temp_directory_path()/(L"ModMenuSettingsTests-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));fs::create_directory(root);
 struct Cleanup{fs::path p;~Cleanup(){std::error_code ec;fs::remove_all(p,ec);}} cleanup{root};
 auto setup=[&](std::string const& id,std::string const& file,std::string const& text){fs::create_directory(root/id);std::ofstream(root/id/"manifest.json")<<"{\"entry\":\""<<id<<".dll\"}";std::ofstream(root/id/file,std::ios::binary)<<text;};
 setup("AutoRestock","config.json",R"({"enabled":true,"preRestockThreshold":17})");
 auto path=root/"AutoRestock/config.json";auto before=Read(path);auto d=LoadSettings(root,"AutoRestock");
 Check(d.valid && d.fields.size()==2 && Read(path)==before,"loading draft is read-only and source-backed");
 if(!d.valid || d.fields.size()!=2)return 1;
 Check(SettingsSchema("AutoRestock").version==1 && d.fields[1].defaultValue==SettingValue{6} &&
       !d.fields[1].description.empty(),"description retains real default rather than loaded threshold");
 auto originalDraft=d;
 Check(ResetSetting(d.fields[1]) && std::get<int>(d.fields[1].value)==6 && Read(path)==before,"single reset changes draft only");
 Check(!SetInteger(d.fields[1],"99999999999999999") && !SetInteger(d.fields[1],"1.5") &&
       !SetInteger(d.fields[1],"65") && std::get<int>(d.fields[1].value)==6,"numeric overflow syntax and bounds refuse without mutation");
 Check(SetInteger(d.fields[1],"16") && std::get<int>(d.fields[1].value)==16,"direct integer edit validates complete input");
 Check(AdjustSetting(d.fields[1],100000) && std::get<int>(d.fields[1].value)==64,"large adjustment clamps without overflow");
 d=originalDraft;
 Check(!HasSettings("FastBlockBreak") && !LoadSettings(root,"../AutoRestock").valid,"unsupported plugin and traversal refused");
 d.fields[1].value=65;auto r=ApplySettings(root,d);Check(!r.saved && Read(path)==before,"out of range refuses without touching source");
 d.fields[1].value=16;r=ApplySettings(root,d);Check(r.saved,"Apply saves valid draft");
 Check(nlohmann::json::parse(Read(path))["preRestockThreshold"]==16,"actual typed setting persisted");
 bool backup=false;for(auto const& p:fs::directory_iterator(path.parent_path()))if(p.path().extension()==L".bak")backup|=Read(p.path())==before;
 Check(backup,"exact original backup kept");
 auto stale=LoadSettings(root,"AutoRestock");std::ofstream(path)<<R"({"enabled":false,"preRestockThreshold":10})";auto external=Read(path);
 stale.fields[0].value=true;Check(!ApplySettings(root,stale).saved && Read(path)==external,"external edit conflicts rather than overwrites");
 std::ofstream(path)<<R"({"enabled":true,"enabled":false})";Check(!LoadSettings(root,"AutoRestock").valid,"duplicate fields rejected");
 std::ofstream(path)<<R"({"enabled":"true"})";Check(!LoadSettings(root,"AutoRestock").valid,"wrong source type rejected");
 setup("ContainerPreview","config.json",R"({"enabled":true,"holdKey":"LEFT_ALT","future":{"a":[1,"物品",false]}})");
 d=LoadSettings(root,"ContainerPreview");Check(d.valid && d.fields.size()==2,"preview accepts native key alias");
 Check(SetKey(d.fields[1],0xA5) && std::get<std::string>(d.fields[1].value)=="RALT","key capture serializes accepted native key");
 Check(SetKey(d.fields[1],27) && SettingText(d.fields[1])=="NONE","Escape clears preview key explicitly");d.fields[0].value=false;r=ApplySettings(root,d);
 auto j=nlohmann::json::parse(Read(root/"ContainerPreview/config.json"));
 Check(r.saved && j["future"]["a"][1]=="物品" && j["future"]["a"][2]==false,"unknown nested fields preserved on Apply");
 auto resetPreview=LoadSettings(root,"ContainerPreview");auto resetBefore=Read(root/"ContainerPreview/config.json");ResetSettings(resetPreview);
 Check(Read(root/"ContainerPreview/config.json")==resetBefore,"reset all does not write before explicit Apply");
 auto resetResult=ApplySettings(root,resetPreview);auto resetJson=nlohmann::json::parse(Read(root/"ContainerPreview/config.json"));
 Check(resetResult.saved && resetJson["enabled"]==true && resetJson["holdKey"]=="LALT" && resetJson["future"]["a"][1]=="物品","explicit Apply of reset uses source defaults and preserves unknown data");
 d=LoadSettings(root,"ContainerPreview");d.fields[1].value=std::string("F25");Check(!ApplySettings(root,d).saved,"invalid programmatic key refused");
 d=LoadSettings(root,"ContainerPreview");d.fields.push_back({"/future","Injected",SettingType::Boolean,true});Check(!ApplySettings(root,d).saved,"unexposed field cannot be injected by draft");
 setup("MouseTweaksBedrock","MouseTweaksBedrock.json",R"({"enabled":true,"wheelScrollDirection":3,"wheelSearchOrder":1,"scrollItemScaling":0})");
 d=LoadSettings(root,"MouseTweaksBedrock");Check(d.valid && d.fields.size()==9,"MouseTweaks real fields and defaults loaded");
 if(!d.valid || d.fields.size()!=9)return 1;
 Check(d.fields[2].category=="Dragging" && d.fields[6].category=="Mouse wheel" &&
       d.fields[1].category=="Diagnostics","one shared description groups actual MouseTweaks fields");
 auto resetDraft=d;resetDraft.fields[0].value=false;resetDraft.fields[6].value=0;
 ResetSettings(resetDraft);Check(std::get<bool>(resetDraft.fields[0].value) && std::get<int>(resetDraft.fields[6].value)==1,
       "reset all restores actual source defaults not the last saved values");
 // The earlier wrong-type refusal deliberately left this fixture invalid.
 std::ofstream(path,std::ios::binary)<<before;
 auto forged=LoadSettings(root,"AutoRestock");
 Check(forged.valid && forged.fields.size()==2,"range-forgery regression starts from a valid loaded draft");
 if(!forged.valid || forged.fields.size()!=2)return 1;
 forged.fields[1].maximum=1000;forged.fields[1].value=999;
 Check(!ApplySettings(root,forged).saved,"UI metadata cannot widen storage validation range");
 for(auto& field:d.fields)if(field.path=="/wheelScrollDirection")field.value=4;
 Check(!ApplySettings(root,d).saved,"enum beyond real parser range refused");
 setup("FakeSneak","config.json",R"({"enabled":false,"toggleKey":"NONE","showNotifications":true})");
 d=LoadSettings(root,"FakeSneak");Check(d.valid && SetKey(d.fields[1],0x2D),"FakeSneak supports native named key");
 fs::rename(root/"FakeSneak/manifest.json",root/"FakeSneak/manifest.json.close");Check(LoadSettings(root,"FakeSneak").valid,"disabled installed plugin settings remain editable without loading it");
 d=LoadSettings(root,"ContainerPreview");auto h=CreateFileW((root/"ContainerPreview/config.json").c_str(),GENERIC_READ,0,nullptr,OPEN_EXISTING,0,nullptr);
 Check(!ApplySettings(root,d).saved,"locked target fails closed");if(h!=INVALID_HANDLE_VALUE)CloseHandle(h);
 std::error_code ec;fs::create_directory_symlink(root/"AutoRestock",root/"linked",ec);
 Check(!LoadSettings(root/"linked","AutoRestock").valid,"linked or absent root cannot redirect settings IO");
 auto previewPath=root/"ContainerPreview/config.json";d=LoadSettings(root,"ContainerPreview");auto previewBefore=Read(previewPath);
 SetFileAttributesW(previewPath.c_str(),FILE_ATTRIBUTE_READONLY);Check(!ApplySettings(root,d).saved && Read(previewPath)==previewBefore,"read-only original is preserved");SetFileAttributesW(previewPath.c_str(),FILE_ATTRIBUTE_NORMAL);
 fs::create_hard_link(previewPath,root/"hardlink.json",ec);if(!ec)Check(!LoadSettings(root,"ContainerPreview").valid,"multiply linked config refused");fs::remove(root/"hardlink.json",ec);
 fs::create_directory(root/"other");std::ofstream(root/"other/config.json")<<R"({"enabled":true})";
 fs::create_directory_symlink(root/"other",root/"redirected",ec);if(!ec){
  fs::rename(root/"ContainerPreview",root/"originalPreview");fs::rename(root/"redirected",root/"ContainerPreview");
  Check(!LoadSettings(root,"ContainerPreview").valid,"actual plugin-directory reparse point refused");
  fs::remove(root/"ContainerPreview");fs::rename(root/"originalPreview",root/"ContainerPreview");
 }
 setup("AutoClick","AutoClick.json",R"({"cps":17,"requiredSlot":1,"requireLmbHold":true,"enabled":false})");
 d=LoadSettings(root,"AutoClick");Check(d.valid && d.fields.size()==11,"AutoClick exposes verified effective settings");
 setup("AutoJumpReset","AutoJumpReset.json",R"({"delayStepMs":2,"overlay":{"enabled":false,"durationMs":1200,"alpha":200},"delayMs":"12"})");
 d=LoadSettings(root,"AutoJumpReset");Check(d.valid && d.fields.size()==17,"AutoJumpReset expanded settings match source");
 if(d.valid){d.fields[1].value=true;r=ApplySettings(root,d);auto result=nlohmann::json::parse(Read(root/"AutoJumpReset/AutoJumpReset.json"));Check(r.saved && result["overlay"]["enabled"]==true && result["delayMs"]=="12","nested Apply preserves unrelated delay specification");}
 std::ofstream(root/"ContainerPreview/config.json")<<std::string(4097,' ');Check(!LoadSettings(root,"ContainerPreview").valid,"plugin-specific source size cap respected");
 std::ofstream(root/"ContainerPreview/config.json",std::ios::binary)<<std::string("{\"holdKey\":\"\xC0\x80\"}");Check(!LoadSettings(root,"ContainerPreview").valid,"malformed UTF-8 config is refused");
 std::ofstream(root/"ContainerPreview/config.json")<<R"({"enabled":true})";
 std::ofstream(root/"ContainerPreview/manifest.json")<<R"({"entry":"Unrelated.dll"})";Check(!LoadSettings(root,"ContainerPreview").valid,"matching folder alone cannot authorize unrelated plugin settings");
 std::ofstream(root/"ContainerPreview/manifest.json")<<R"({"entry":"ContainerPreview.dll"})";
 std::string deep="0";for(int n=0;n<40;++n)deep="{\"nested\":"+deep+"}";
 std::ofstream(root/"ContainerPreview/config.json")<<deep;Check(!LoadSettings(root,"ContainerPreview").valid,"excessive config depth rejected");
 std::ofstream(root/"AutoJumpReset/AutoJumpReset.json")<<R"({"overlay":"unsafe parent"})";Check(!LoadSettings(root,"AutoJumpReset").valid,"nested field cannot silently overwrite non-object parent");
 TransactionChecks(Check);
 ExternalSettingsChecks(Check);
 AdapterChecks(root,Check);
 std::cout<<checks<<" checks / "<<failures<<" failures\n";return failures?1:0;
}
