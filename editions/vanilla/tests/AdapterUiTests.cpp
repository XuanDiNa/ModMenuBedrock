#include "ModMenu/Core.h"
#include "ModMenu/View.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
namespace fs=std::filesystem;using namespace menu;using Json=nlohmann::json;
namespace {
int checks{},failures{};
void Check(bool b,char const* message){++checks;if(!b){++failures;std::cerr<<"FAIL "<<message<<'\n';}}
std::string Read(fs::path const& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
int Index(SettingsDraft const& d,char const* path){for(int i=0;i<int(d.fields.size());++i)if(d.fields[i].path==path)return i;return -1;}
Frame frame{854,480,true,true,false,MenuScene::Pause};
void Key(Menu& m,unsigned k){m.Input({Kind::Key,0,0,int(k),1},frame);m.Input({Kind::Key,0,0,int(k),0},frame);}
void Click(Menu& m,Rect r){auto x=r.x+r.w/2,y=r.y+r.h/2;m.Input({Kind::Down,x,y,1},frame);m.Input({Kind::Up,x,y,1},frame);}
void Edit(Menu& m,int i,std::string const& text){m.fieldSelected=i;Key(m,13);m.Input({Kind::Text,0,0,0,0,text},frame);}
Menu Open(fs::path const& root,char const* id){Menu m;m.nativeEntry=true;m.open=m.configuring=true;m.draft=LoadSettings(root,id);return m;}
}
int main(){
 auto root=fs::temp_directory_path()/(L"ModMenuAdapterUi-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));fs::create_directory(root);
 struct Cleanup{fs::path p;~Cleanup(){std::error_code ec;fs::remove_all(p,ec);}}cleanup{root};
 for(auto id:{"AutoJumpReset","AutoClick"}){fs::create_directory(root/id);std::ofstream(root/id/"manifest.json")<<"{\"entry\":\""<<id<<".dll\"}";}
 auto jump=root/"AutoJumpReset/AutoJumpReset.json",click=root/"AutoClick/AutoClick.json";
 std::ofstream(jump)<<R"({"delayMs":"12","keys":{"toggle":"J","delayDown":"["},"sentinel":true})";
 std::ofstream(click)<<R"({"cps":14,"keys":{"toggle":"F8"},"keySlotMap":"R=1,E=2","sentinel":true})";
 auto m=Open(root,"AutoJumpReset");auto delay=Index(m.draft,"/delayMs"),bind=Index(m.draft,"/keys/toggle");
 Check(m.draft.valid && delay>=0 && bind>=0,"new delay and key controls exist in a loaded draft");
 if(delay<0 || bind<0){std::cout<<checks<<" adapter UI checks / "<<failures<<" failures\n";return 1;}
 auto original=Read(jump);Edit(m,delay,"150-120");
 Check(m.editField==delay && m.numberText=="150-120" && Read(jump)==original,"text range input replaces existing draft only");
 Key(m,13);Check(m.editField==-1 && SettingText(m.draft.fields[delay])=="120-150" && m.settingsChanged,"Enter validates and canonicalizes native delay range");
 Click(m,Arrange(m.config,frame).cancel);Check(!m.configuring && Read(jump)==original,"Cancel discards a text edit without file changes");
 m=Open(root,"AutoJumpReset");Edit(m,delay,"9999");Click(m,Arrange(m.config,frame).apply);
 Check(m.editField==delay && !m.numberError.empty() && !m.settingsBusy && Read(jump)==original,"invalid active text blocks Apply without leaving editor");
 Key(m,27);Check(m.configuring && m.editField==-1 && SettingText(m.draft.fields[delay])=="12","first Escape cancels only active text input");
 Key(m,27);Check(m.open && !m.configuring,"second Escape returns to Mods, not the game HUD");
 m=Open(root,"AutoJumpReset");Edit(m,delay,"134");Click(m,Arrange(m.config,frame).apply);
 Check(m.settingsRequest==Menu::Request::Apply && m.settingsBusy,"Apply commits active text and queues a storage request");
 auto saved=ApplySettings(root,m.draft);m.CompleteSettings(m.settingsGeneration,saved);
 Check(saved.saved && Json::parse(Read(jump))["delayMs"]=="134" && Json::parse(Read(jump))["sentinel"]==true,"real settings worker transaction persists fixed delay");
 m=Open(root,"AutoJumpReset");m.fieldSelected=bind;Key(m,13);Key(m,'1');
 Check(m.captureField==-1 && SettingText(m.draft.fields[bind])=="0x31","key capture stores digit's true virtual key");
 Check(DisplaySetting(m.draft.fields[bind],Locale::English)=="1","encoded native key is displayed as a readable digit");
 m=Open(root,"AutoClick");auto map=Index(m.draft,"/keySlotMap"),toggle=Index(m.draft,"/keys/toggle");
 Check(map>=0 && toggle>=0,"AutoClick mapping and key controls exist");
 if(map>=0 && toggle>=0){
  Edit(m,map,"r=1, e=2, 1=7");Key(m,13);
  Check(SettingText(m.draft.fields[map])=="R=1,E=2,1=7","slot map entry normalizes safe supported syntax");
  auto draft=m.draft;Edit(m,map,"R=0");Key(m,13);
  Check(m.editField==map && !m.numberError.empty() && m.draft.fields[map].value==draft.fields[map].value,"invalid map does not replace prior valid draft");
  Key(m,27);m.fieldSelected=toggle;Key(m,13);Key(m,27);
  Check(m.configuring && m.captureField==-1 && SettingText(m.draft.fields[toggle])=="0x00","Escape clears native hotkey without leaving Configure");
 }
 // Exercise actual view generation for long text, scrolled rows and all locales.
 for(auto id:{"AutoClick","AutoJumpReset"})for(auto locale:{Locale::English,Locale::TraditionalChinese,Locale::SimplifiedChinese})for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},Frame{640,360,true,true},Frame{854,480,true,true}}){
  auto page=Open(root,id);page.locale=locale;auto field=Index(page.draft,id==std::string("AutoClick")?"/keySlotMap":"/delayMs");
  page.fieldSelected=page.editField=field;page.fieldScroll=field;page.numberText=id==std::string("AutoClick")?"R=1,E=2,Z=3,1=7,2=8,3=9,MOUSE5=4":"120-150";page.Tick(viewport);
  auto paints=BuildView(page,viewport);bool bounded=!paints.empty(),caret=false;
  for(auto const& p:paints){bounded&=p.rect.x>=0 && p.rect.y>=0 && p.rect.w>=0 && p.rect.h>=0 && p.rect.x+p.rect.w<=viewport.width+.01f && p.rect.y+p.rect.h<=viewport.height+.01f;caret|=p.text.ends_with('_');}
  Check(bounded && caret,"new text controls remain visible and bounded at every supported locale and viewport");
 }
 for(auto id:{"AutoClick","AutoJumpReset"})for(auto locale:{Locale::TraditionalChinese,Locale::SimplifiedChinese}){
  auto schema=SettingsSchema(id);for(auto const& field:schema.fields){
   Check(Tr(locale,field.label)!=field.label && Tr(locale,field.category)!=field.category && Tr(locale,field.description)!=field.description,"all built-in expanded fields have localized labels categories and help");
  }
 }
 std::cout<<checks<<" adapter UI checks / "<<failures<<" failures\n";return failures?1:0;
}
