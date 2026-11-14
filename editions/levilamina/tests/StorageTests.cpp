#include "ModMenu/Storage.h"
#include <fstream>
#include <iostream>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
int checks{},failures{};
void Check(bool b,char const* n){++checks;if(!b){++failures;std::cerr<<"FAIL "<<n<<'\n';}}
int main(){
 using namespace menu;namespace fs=std::filesystem;
 auto root=fs::temp_directory_path()/(L"ModMenuTests-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));fs::create_directory(root);
 struct Cleanup{fs::path p;~Cleanup(){std::error_code ec;fs::remove_all(p,ec);}}cleanup{root};
 Check(ValidUtf8("物品") && !ValidUtf8(std::string("\xc0\x80",2)) && !ValidUtf8(std::string("\xed\xa0\x80",3)),"strict UTF8");
 Config c;c.openKey=65;c.button={3,17,24,1.25f};
 Check(SaveConfig(root/"config.json",c),"atomic initial save");Config d;
 Check(LoadConfig(root/"config.json",d) && d.openKey==65 && d.button.anchor==3 && d.button.scale==1.25f,"config roundtrip");
 std::ofstream(root/"config.json")<<"{broken";d.openKey=88;
 Check(!LoadConfig(root/"config.json",d) && d.openKey==88,"invalid preserves destination");
 std::ofstream(root/"config.json")<<R"({"openKey":0,"button":{"scale":500}})";
 Check(!LoadConfig(root/"config.json",d),"unsafe key/scale rejected");
 std::ofstream(root/"config.json")<<R"({"openKey":27})";Check(!LoadConfig(root/"config.json",d),"Escape cannot be a recovery hotkey");
 fs::create_directory(root/"mods");fs::create_directory(root/"mods/a");fs::create_directory(root/"mods/b");fs::create_directory(root/"mods/c");fs::create_directory(root/"mods/d");
 std::ofstream(root/"mods/a/manifest.json")<<R"({"name":"Alpha","entry":"a.dll","version":"1.0"})";
 std::ofstream(root/"mods/a/mod.json")<<R"({"name":"物品","description":"metadata","author":"Someone"})";
 std::ofstream(root/"mods/a/a.dll")<<"inert fixture";
 std::ofstream(root/"mods/b/manifest.json")<<R"({"name":"Disabled","entry":"b.dll","enabled":false})";
 std::ofstream(root/"mods/c/manifest.json")<<R"({"name":"Escape","entry":"../a/a.dll"})";
 std::ofstream(root/"mods/d/manifest.json")<<"broken";
 auto list=Discover(root/"mods",[](auto const&){return true;});
 Check(list.size()==2,"bounded discovery rejects traversal and malformed manifests");
 bool utf{},disabled{},loaded{};for(auto const& e:list){utf|=e.name=="物品";disabled|=e.status=="Disabled manifest";loaded|=e.status=="DLL loaded (Ready unknown)";}
 Check(utf && disabled && loaded,"UTF8 metadata and honest disabled/loaded status");
 auto outside=root/"outside";fs::create_directory(outside);std::error_code ec;fs::create_directory_symlink(outside,root/"mods/link",ec);
 if(!ec)Check(Discover(root/"mods").size()==2,"reparse folder skipped");
 Check(!SaveConfig(root/"mods/link/config.json",c),"linked or missing parent write refused");
 Check(!fs::exists(root/"config.json.tmp"),"temporary file not leaked");
 fs::create_directory(root/"mods/closed");std::ofstream(root/"mods/closed/manifest.json.close")<<R"({"name":"Closed","entry":"closed.dll"})";
 auto closed=Discover(root/"mods");bool foundClosed=false;for(auto const& e:closed)foundClosed|=e.id=="closed" && e.status=="Disabled manifest";
 Check(foundClosed,"LeviLauncher close suffix is discovered as disabled");
 // Installed dependency discovery and runtime-visible entries are different.
 fs::create_directory(root/"mods/unloaded");
 std::ofstream(root/"mods/unloaded/manifest.json")<<R"({"name":"Not running","entry":"unloaded.dll"})";
 std::ofstream(root/"mods/unloaded/unloaded.dll")<<"inert fixture";
 std::ofstream(root/"mods/closed/closed.dll")<<"inert fixture";
 std::ofstream(root/"mods/closed/config.json")<<R"({"enabled":false})";
 auto predicate=[](fs::path const& p){return p.filename()=="a.dll" || p.filename()=="closed.dll";};
 auto installed=Discover(root/"mods",predicate);auto visible=LoadedCatalog(root/"mods",predicate);
 Check(installed.size()==4,"installed dependency discovery retains unloaded entries");
 Check(visible.size()==2,"runtime catalog excludes unloaded installed DLLs");
 bool loadedClosed=false;for(auto const& e:visible)loadedClosed|=e.id=="closed";
 Check(loadedClosed,"loaded DLL remains visible despite closed manifest and disabled feature config");
 Check(LoadedCatalog(root/"mods",{}).empty(),"missing process evidence cannot mark installed DLLs loaded");
 std::cout<<checks<<" checks / "<<failures<<" failures\n";return failures?1:0;
}
