#include "ModMenu/Core.h"
#include "ModMenu/Configure.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace menu;namespace fs=std::filesystem;using Json=nlohmann::json;
unsigned checks{};
void Check(bool ok,char const* message){++checks;if(!ok)throw std::runtime_error(message);}
std::string Read(fs::path const& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void Write(fs::path const& p,std::string const& s){std::ofstream(p,std::ios::binary)<<s;}
void Click(Menu& m,Frame const& f,Rect r){auto x=r.x+r.w/2,y=r.y+r.h/2;m.Input({Kind::Down,x,y,1},f);m.Input({Kind::Up,x,y,1},f);}
int main(int argc,char** argv){try{
 Check(argc==2,"usage: BindingBundleTests <flat mods bundle>");fs::path bundle=argv[1];
 auto root=fs::temp_directory_path()/("BindingBundle-"+std::to_string(GetCurrentProcessId()));fs::create_directory(root);
 struct Cleanup{fs::path root;~Cleanup(){std::error_code ec;fs::remove_all(root,ec);}}cleanup{root};
 Frame frame{854,480,true,true,false,MenuScene::Pause};unsigned fields{};
 for(auto id:{"AutoClick","AutoJumpReset","ContainerPreview","FakeSneak","FullBright","ToggleSprint","MuoniumPlayerBedrock"}){
  auto path=bundle/id;Check(fs::is_regular_file(path/"modmenu.settings.json"),"matching consumer description must be present");
  auto manifest=Json::parse(Read(path/"manifest.json"));auto desc=Json::parse(Read(path/"modmenu.settings.json"));
  Check(desc["schemaVersion"]==5,"consumer explicitly opts into chord contract v5");
  auto def=ParseSettingsDescription(desc.dump(),id,manifest.at("entry"),manifest.at("version"));
  auto dir=root/id;fs::create_directory(dir);Write(dir/"manifest.json",manifest.dump());Write(dir/"modmenu.settings.json",desc.dump());
  Json original={{"_binding_test_sentinel",7}};for(auto const& field:desc["fields"])original[Json::json_pointer(field["path"])]=field["default"];
  for(int i=0;i<int(def.fields.size());++i){
   if(def.fields[i].type!=SettingType::Binding)continue;++fields;
   for(unsigned mask=1;mask<=7;++mask){
    Write(dir/def.file,original.dump());Menu m;m.nativeEntry=true;modmenu::binding::Snapshot keys{};m.PollKeyboard(keys,frame);m.OpenNative(frame);m.configuring=true;m.draft=LoadSettings(root,id);
    Check(m.draft.valid,"actual bundle settings load");m.fieldSelected=i;m.fieldScroll=i;m.Tick(frame);
    auto layout=ArrangeSettings(m,frame);auto row=std::find_if(layout.rows.begin(),layout.rows.end(),[&](auto const& r){return r.index==i;});
    Check(row!=layout.rows.end(),"binding control visible after scroll");Click(m,frame,row->value);
    Check(m.captureField==i,"mouse click enters chord capture");
    keys[162]=(mask&1)!=0;keys[160]=(mask&2)!=0;keys[164]=(mask&4)!=0;m.PollKeyboard(keys,frame);
    Check(m.captureField==i,"modifier press waits for a primary key");keys[135]=true;m.PollKeyboard(keys,frame);
    auto expected=modmenu::binding::Token({135,mask});Check(m.captureField==-1&&SettingText(m.draft.fields[i])==expected,"complete chord captured through actual UI");
    keys={};m.PollKeyboard(keys,frame);Click(m,frame,Arrange(m.config,frame).apply);Check(m.settingsBusy&&m.settingsRequest==Menu::Request::Apply,"UI queues Apply");
    auto saved=ApplySettings(root,m.draft);Check(saved.saved,"real transactional Apply succeeds");m.CompleteSettings(m.settingsGeneration,saved);
    auto disk=Json::parse(Read(dir/def.file));Check(disk[Json::json_pointer(def.fields[i].path)]==expected&&disk["_binding_test_sentinel"]==7,"full chord and unknown values persist");
    auto again=LoadSettings(root,id);Check(again.valid&&SettingText(again.fields[i])==expected,"reopened actual bundle retains chord");
   }
  }
  std::cout<<id<<": actual descriptions/click capture/Apply/reload passed\n";
 }
 Check(fields==14,"all fourteen user activation bindings covered; emitted jump key excluded");
 std::cout<<"Binding bundle: "<<checks<<" checks, "<<fields<<" fields, 0 failures\n";return 0;
}catch(std::exception const& e){std::cerr<<"FAIL after "<<checks<<": "<<e.what()<<'\n';return 1;}}
