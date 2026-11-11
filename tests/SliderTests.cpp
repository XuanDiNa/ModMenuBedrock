#include "ModMenu/Settings.h"
#include "ModMenu/Core.h"
#include "ModMenu/Configure.h"
#include "ModMenu/View.h"
#include "ModMenu/Slider.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
using namespace menu;
namespace {
int checks{},failures{};
void Check(bool ok,char const* label){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<label<<'\n';}}
std::string Description(std::string field,unsigned version=2){return "{\"schemaVersion\":"+std::to_string(version)+R"(,"plugin":{"id":"ExamplePlugin","entry":"ExamplePlugin.dll","versions":["1.0.0"]},"config":{"file":"settings.json","reload":"hot"},"fields":[)"+field+"]}";}
std::string integer=R"({"path":"/height","type":"slider","numberType":"integer","min":-64,"max":320,"step":1,"default":192,"label":"Height"})";
std::string decimal=R"({"path":"/scale","type":"slider","numberType":"decimal","precision":2,"min":0.5,"max":2,"step":0.05,"default":1,"unit":"x","label":"Scale"})";
SettingsDefinition Parse(std::string const& raw){return ParseSettingsDescription(raw,"ExamplePlugin","ExamplePlugin.dll","1.0.0");}
void Run(char const* name,std::function<void()> test){try{test();}catch(std::exception const& e){Check(false,name);std::cerr<<"  "<<e.what()<<'\n';}}
void Refuse(std::string const& bytes,char const* name){bool refused=false;try{(void)Parse(bytes);}catch(...){refused=true;}Check(refused,name);}
namespace fs=std::filesystem;using Json=nlohmann::json;
void Write(fs::path const& p,std::string const& s){std::ofstream(p,std::ios::binary)<<s;}
std::string Read(fs::path const& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
void NumericTests(){
 Run("published v2 sidecars",[]{for(auto id:{"ExamplePlugin","AutoRestock"}){auto bytes=Read(fs::path("integration/v2-examples")/id/"modmenu.settings.json");auto j=Json::parse(bytes);Check(!ParseSettingsDescription(bytes,id,j["plugin"]["entry"],j["plugin"]["versions"][0]).fields.empty(),"published v2 author fixture accepted by runtime");}});
 Run("signed integer arithmetic",[]{auto field=Parse(Description(integer)).fields[0];
  Check(SettingText(field)=="192","integer default formats correctly");
  Check(SetInteger(field,"-64") && SettingText(field)=="-64","signed editor reaches lower bound");
  Check(!SetInteger(field,"-65") && !SetInteger(field,"321") && !SetInteger(field,"1.0"),"integer editor rejects out-of-range and fractional syntax");
  Check(AdjustSetting(field,10) && SettingText(field)=="-54","integer keyboard steps exact");
  Check(ResetSetting(field) && SettingText(field)=="192","slider reset restores typed default");
 });
 Run("decimal grid arithmetic",[]{auto field=Parse(Description(decimal)).fields[0];
  Check(SetInteger(field,"1.25") && SettingText(field)=="1.25","decimal text is accepted");
  Check(!SetInteger(field,"1.26") && !SetInteger(field,"1.25001") && !SetInteger(field,"NaN") && !SetInteger(field,"1e0"),"decimal editor rejects off-grid excess-precision and exponent input");
  Check(SetInteger(field,"1.2500") && SettingText(field)=="1.25","trailing decimal zeroes are harmless");
  for(int i=0;i<30;++i){AdjustSetting(field,1);AdjustSetting(field,-1);}
  Check(SettingText(field)=="1.25","repeated decimal adjustment does not drift");
  Check(AdjustSetting(field,2147483647) && SettingText(field)=="2","large positive adjustment clamps without overflow");
  Check(AdjustSetting(field,(-2147483647-1)) && SettingText(field)=="0.5","large negative adjustment clamps without overflow");
 });
 Refuse(Description(integer,1),"v1 refuses slider type");
 for(auto pair:{std::pair{"step","0"}, {"step","-1"},{"step","5"},{"min","320"},{"max","-65"},{"default","321"},{"default","1.5"},{"numberType","\"float\""},{"precision","2"}}){
  auto f=Json::parse(integer);f[pair.first]=Json::parse(pair.second);Refuse(Description(f.dump()),"invalid integer slider declaration refused");
 }
 for(auto pair:{std::pair{"precision","0"},{"precision","7"},{"step","0.03"},{"default","1.26"},{"default","0.499999"},{"max","2.001"},{"step","0.00001"}}){
  auto f=Json::parse(decimal);f[pair.first]=Json::parse(pair.second);Refuse(Description(f.dump()),"invalid decimal slider declaration refused");
 }
 {auto f=Json::parse(decimal);f.erase("precision");Refuse(Description(f.dump()),"decimal precision required");}
 {auto f=Json::parse(integer);f.erase("step");Refuse(Description(f.dump()),"slider step required");}
 Run("six place signed JSON numeric tokens",[]{
  auto f=R"({"path":"/scale","type":"slider","numberType":"decimal","precision":6,"min":-1e-6,"max":1e-6,"step":1e-6,"default":-0.000000,"label":"Scale"})";
  auto field=Parse(Description(f)).fields[0];Check(SettingText(field)=="0","negative zero normalized");
  Check(AdjustSetting(field,-1) && SettingText(field)=="-0.000001","exponent tokens parse exactly at precision six");
 });
 {auto raw=decimal;auto at=raw.find("\"default\":1");raw.replace(at,11,"\"default\":1.00000000000000000000000000001");Refuse(Description(raw),"raw excess precision cannot round into a valid default");}
 for(auto literal:{"2147483648","-2147483649","1e999","\"1\""}){auto f=Json::parse(integer);auto raw=f.dump();auto at=raw.find("\"default\":192");raw.replace(at,13,std::string("\"default\":")+literal);Refuse(Description(raw),"numeric overflow and wrong type refused");}
 Run("full signed range",[]{auto f=Json::parse(integer);f["min"]=-2147483647-1;f["max"]=2147483647;f["default"]=0;auto field=Parse(Description(f.dump())).fields[0];Check(SetInteger(field,"-2147483648") && SettingText(field)=="-2147483648","signed int32 minimum reachable");Check(SetInteger(field,"2147483647"),"signed int32 maximum reachable");});
 Run("large decimal grid and exact ties",[]{
  auto raw=R"({"path":"/scale","type":"slider","numberType":"decimal","precision":6,"min":-2147483648,"max":2147483647,"step":0.000001,"default":0,"label":"Scale"})";
  auto field=Parse(Description(raw)).fields[0];PositionSlider(field,0.);Check(SettingText(field)=="-2147483648","largest scaled grid lower endpoint exact");PositionSlider(field,1.);Check(SettingText(field)=="2147483647","largest scaled grid upper endpoint exact");
  SetInteger(field,"2147483646.999999");StepSlider(field,1);Check(SettingText(field)=="2147483647","microstep at large magnitude does not lose precision");
  auto small=Parse(Description(decimal)).fields[0];PositionSlider(small,.25);Check(SettingText(small)=="0.9","half-step tie chooses larger value");auto before=small.value;PositionSlider(small,std::numeric_limits<double>::quiet_NaN());Check(small.value==before,"nonfinite pointer ratio leaves value unchanged");
 });
 Run("v2 keeps legacy integers unchanged",[]{auto f=Json::parse(integer);f["type"]="integer";f.erase("numberType");f["min"]=1;f["max"]=64;f["step"]=2;f["default"]=8;auto field=Parse(Description(f.dump())).fields[0];Check(SettingText(field)=="8" && SetInteger(field,"10"),"legacy integer off-grid editing is not tightened by v2");f["min"]=-1;Refuse(Description(f.dump()),"v2 legacy integer remains nonnegative");});
}
void StorageTests(){
 auto root=fs::temp_directory_path()/("ModMenuSlider-"+std::to_string(GetCurrentProcessId())+"-"+std::to_string(GetTickCount64()));fs::create_directory(root);
 struct Cleanup{fs::path path;~Cleanup(){std::error_code ec;fs::remove_all(path,ec);}} cleanup{root};
 auto dir=root/"ExamplePlugin";fs::create_directory(dir);auto config=dir/"settings.json",sidecar=dir/"modmenu.settings.json";
 auto description=Description(integer+","+decimal);Write(dir/"manifest.json",R"({"entry":"ExamplePlugin.dll","version":"1.0.0"})");Write(sidecar,description);
 std::string original=R"({"height":-10,"scale":1.25,"future":{"text":"物品","array":[1,false]}})";Write(config,original);
 auto draft=LoadSettings(root,"ExamplePlugin");Check(draft.valid && draft.fields.size()==2,"real config loads both slider numeric types");
 if(!draft.valid)return;
 Check(SetInteger(draft.fields[0],"-64") && SetInteger(draft.fields[1],"1.3") && Read(config)==original,"numeric draft editing never writes disk");
 auto saved=ApplySettings(root,draft);auto result=Json::parse(Read(config));
 Check(saved.saved && result["height"].is_number_integer() && result["height"]==-64 && result["scale"].is_number() && result["scale"]==1.3,"Apply writes real JSON numbers not strings");
 Check(Read(config).find("1.3")!=std::string::npos && Read(config).find("1.3000000")==std::string::npos && result["future"]["text"]=="物品","canonical decimal and unknown data survive save");
 bool backup=false;for(auto const& file:fs::directory_iterator(dir))if(file.path().extension()==".bak")backup|=Read(file.path())==original;Check(backup,"exact original backup retained");
 draft=LoadSettings(root,"ExamplePlugin");Check(draft.valid && SettingText(draft.fields[1])=="1.3","saved decimal reloads on same grid");
 auto before=Read(config);draft.fields[1].value=std::string("forged");Check(!ApplySettings(root,draft).saved && Read(config)==before,"wrong variant cannot bypass slider validation");
 draft=LoadSettings(root,"ExamplePlugin");SetInteger(draft.fields[1],"1.5");Write(sidecar,description+" ");Check(!ApplySettings(root,draft).saved && Read(config)==before,"stale description refuses slider Apply");Write(sidecar,description);
 for(auto value:{"1.26","1.2500000000000000000000001","3","\"1.25\"","null"}){Write(config,std::string("{\"height\":0,\"scale\":")+value+"}");Check(!LoadSettings(root,"ExamplePlugin").valid,"invalid stored decimal is never silently snapped");}
 Write(config,R"({"height":0,"scale":1.25e0})");Check(LoadSettings(root,"ExamplePlugin").valid,"exact decimal exponent config loads");
 Write(config,original);draft=LoadSettings(root,"ExamplePlugin");draft.fields[1].slider.precision=3;draft.fields[1].slider.step=1;draft.fields[1].value=SliderValue{126};Check(!ApplySettings(root,draft).saved && Read(config)==original,"forged slider precision or step cannot widen authoritative grid");
 auto escaped=Json::parse(decimal);escaped["path"]="/group/a~1b~0c";Write(sidecar,Description(escaped.dump()));Write(config,R"({"group":{"a/b~c":1.25},"array":[2,{"x":true}]})");draft=LoadSettings(root,"ExamplePlugin");
 Check(draft.valid && SetInteger(draft.fields[0],"1.5"),"escaped object pointer finds exact numeric token");if(draft.valid){saved=ApplySettings(root,draft);auto j=Json::parse(Read(config));Check(saved.saved && j["group"]["a/b~c"]==1.5 && j["array"][1]["x"]==true,"fixed numeric serializer escapes keys and preserves arrays");}
}
Frame frame{854,480,true,true,false,MenuScene::Pause};
void Key(Menu& m,unsigned k,Modifiers mods={}){m.Input({Kind::Key,0,0,int(k),1,{},mods},frame);m.Input({Kind::Key,0,0,int(k),0,{},mods},frame);}
void Mouse(Menu& m,Kind kind,float x,float y){m.Input({kind,x,y,1},frame);}
void Click(Menu& m,Rect const& r){Mouse(m,Kind::Down,r.x+r.w/2,r.y+r.h/2);Mouse(m,Kind::Up,r.x+r.w/2,r.y+r.h/2);}
Menu Page(){Menu m;m.nativeEntry=m.open=m.configuring=true;m.draft.valid=true;m.draft.fields=Parse(Description(decimal+","+integer)).fields;return m;}
void UiTests(){
 Run("rail geometry and drag lifecycle",[]{auto m=Page();auto row=ArrangeSettings(m,frame).rows[0];auto rail=row.rail;
  Check(rail.w>10 && rail.h>0 && row.number.w>0,"slider has distinct rail and number targets");
  Mouse(m,Kind::Down,rail.x,rail.y+rail.h/2);Check(SettingText(m.draft.fields[0])=="0.5" && m.Captured(),"left press on track selects minimum and captures");
  Mouse(m,Kind::Move,rail.x+rail.w/2,rail.y+rail.h/2);Check(SettingText(m.draft.fields[0])=="1.25","middle drag quantizes decimal grid");
  auto apply=Arrange(m.config,frame).apply;Mouse(m,Kind::Up,apply.x+apply.w,apply.y+1);
  Check(!m.Captured() && m.settingsRequest==Menu::Request::None && m.configuring,"release over Apply stops capture without saving");
  Mouse(m,Kind::Down,rail.x,rail.y+rail.h/2);Mouse(m,Kind::Move,frame.width+100,20);Mouse(m,Kind::Up,frame.width+100,20);
  Check(SettingText(m.draft.fields[0])=="2" && !m.Captured(),"outside release clamps to maximum and ends drag");
 });
 Run("escape restores drag but not earlier edits",[]{auto m=Page();SetInteger(m.draft.fields[1],"-10");m.settingsChanged=true;auto rail=ArrangeSettings(m,frame).rows[0].rail;
  Mouse(m,Kind::Down,rail.x,rail.y+2);Key(m,27);Check(m.configuring && SettingText(m.draft.fields[0])=="1" && SettingText(m.draft.fields[1])=="-10" && m.settingsChanged,"Escape restores pre-drag value and retains earlier draft edits");
  auto cancel=Arrange(m.config,frame).cancel;Mouse(m,Kind::Up,cancel.x+1,cancel.y+1);Check(m.configuring && !m.Captured(),"canceled drag release cannot trigger Cancel");
  Key(m,27);Check(!m.configuring && m.open,"next Escape exits Configure only");
 });
 Run("drag interruption safety",[]{
  for(int mode=0;mode<4;++mode){auto m=Page();auto rail=ArrangeSettings(m,frame).rows[0].rail;Mouse(m,Kind::Down,rail.x,rail.y+2);auto changed=frame;
   if(mode==0)changed.width=640;if(mode==1)changed.foreground=false;if(mode==2)changed.eligible=false;if(mode==3)changed.scene=MenuScene::Main;
   m.Tick(changed);
   if(mode==0){Check(m.configuring && SettingText(m.draft.fields[0])=="1" && !m.settingsChanged,"resize cancels draft drag and restores change state");m.Input({Kind::Up,20,20,1},changed);Check(!m.Captured(),"resize leaves no stuck release");}
   else Check(!m.configuring && !m.Captured(),"focus scene or eligibility loss clears active drag");
  }
 });
 Run("input isolation",[]{auto m=Page();auto rail=ArrangeSettings(m,frame).rows[0].rail;Mouse(m,Kind::Down,rail.x,rail.y+2);
  m.Input({Kind::Wheel,rail.x,rail.y,0,-100},frame);Mouse(m,Kind::Down,200,200);Key(m,39);
  Check(m.fieldScroll==0 && SettingText(m.draft.fields[0])=="0.5","wheel duplicate press and keyboard cannot retarget active drag");
  Mouse(m,Kind::Move,rail.x+rail.w,rail.y+2);Mouse(m,Kind::Up,rail.x+rail.w,rail.y+2);Check(SettingText(m.draft.fields[0])=="2","drag continues after duplicate press");
  m=Page();m.Input({Kind::Down,rail.x,rail.y+2,2},frame);m.Input({Kind::Up,rail.x,rail.y+2,2},frame);Check(SettingText(m.draft.fields[0])=="1","right mouse never changes slider");
  m.settingsBusy=true;Mouse(m,Kind::Down,rail.x,rail.y+2);Mouse(m,Kind::Up,rail.x,rail.y+2);Check(SettingText(m.draft.fields[0])=="1","busy slider cannot be edited");
 });
 Run("keyboard text and reset",[]{auto m=Page();Key(m,39);Check(SettingText(m.draft.fields[0])=="1.05","Right moves one decimal step");
  Key(m,37,{true});Check(SettingText(m.draft.fields[0])=="0.55","Shift Left moves ten decimal steps");Key(m,35);Check(SettingText(m.draft.fields[0])=="2","End selects maximum");Key(m,36);Check(SettingText(m.draft.fields[0])=="0.5","Home selects minimum");
  Click(m,ArrangeSettings(m,frame).rows[0].number);m.Input({Kind::Text,0,0,0,0,"1.26"},frame);Click(m,Arrange(m.config,frame).apply);
  Check(m.editField==0 && !m.numberError.empty() && !m.settingsBusy && SettingText(m.draft.fields[0])=="0.5","invalid direct text blocks Apply");
  Key(m,27);Check(m.configuring && m.editField<0,"Escape cancels only numeric edit");Key(m,13);m.Input({Kind::Text,0,0,0,0,"1.25"},frame);Key(m,13);Check(SettingText(m.draft.fields[0])=="1.25" && m.editField<0,"Enter commits decimal text");
  Click(m,ArrangeSettings(m,frame).rows[0].reset);Check(SettingText(m.draft.fields[0])=="1","Reset restores slider default");
  m.fieldSelected=1;Key(m,13);m.Input({Kind::Text,0,0,0,0,"-64"},frame);Key(m,13);Check(SettingText(m.draft.fields[1])=="-64","signed typed text works through real Configure");
 });
 Run("paint geometry and locales",[]{for(auto locale:{Locale::English,Locale::TraditionalChinese,Locale::SimplifiedChinese})for(auto f:{Frame{240,160,true,true},Frame{320,180,true,true},Frame{640,360,true,true},frame}){
  auto m=Page();m.locale=locale;m.Tick(f);auto paints=BuildView(m,f);bool bounded=!paints.empty(),value=false;
  for(auto const& p:paints){bounded&=std::isfinite(p.rect.x) && p.rect.w>=0 && p.rect.h>=0 && p.rect.x>=0 && p.rect.y>=0 && p.rect.x+p.rect.w<=f.width+.01f && p.rect.y+p.rect.h<=f.height+.01f;value|=p.text.find("1")!=std::string::npos;}
  Check(bounded && value,"slider paints stay finite and bounded across locales and sizes");
  auto r=ArrangeSettings(m,f).rows[0];Check(r.rail.w>0 && r.number.w>0 && r.rail.x+r.rail.w<=r.value.x+r.value.w,"compact rail and numeric editor remain inside value region");
  m.editField=0;m.numberText="-2147483648.123456";bool caret=false;for(auto const& p:BuildView(m,f))caret|=p.text.ends_with('_');Check(caret,"numeric editor caret stays visible at compact widths");
 }});
}
}
int main(){
 try{Check(Parse(Description(integer)).fields.size()==1,"valid v2 signed slider accepted");}
 catch(std::exception const& e){Check(false,"valid v2 signed slider accepted");std::cerr<<"  "<<e.what()<<'\n';}
 NumericTests();StorageTests();UiTests();
 std::cout<<checks<<" slider checks / "<<failures<<" failures\n";return failures?1:0;
}
