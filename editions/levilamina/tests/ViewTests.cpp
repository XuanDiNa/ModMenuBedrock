#include "ModMenu/View.h"
#include <iostream>
#include "ModMenu/Configure.h"
#include "ModMenu/Storage.h"
#include <algorithm>
int checks{},failures{};void Check(bool b,char const* n){++checks;if(!b){++failures;std::cerr<<"FAIL "<<n<<'\n';}}
bool Bounded(std::vector<menu::Paint> const& paints,menu::Frame const& frame){
 bool bounded=true;for(auto const& p:paints){
  bool valid=p.rect.x>=0 && p.rect.y>=0 && p.rect.w>=0 && p.rect.h>=0 && p.rect.x+p.rect.w<=frame.width+.01f && p.rect.y+p.rect.h<=frame.height+.01f;
  if(!valid)std::cerr<<"Out-of-bounds paint at "<<frame.width<<'x'<<frame.height<<": "<<p.rect.x<<','<<p.rect.y<<','<<p.rect.w<<','<<p.rect.h<<" text="<<p.text<<'\n';
  bounded&=valid;
 }return bounded;
}
int main(){using namespace menu;Menu m;Frame f{640,360,true,true};
 Check(ClassifyEntry(true,false,0)==Chain::Canonical,"canonical input entry accepted");
 Check(ClassifyEntry(false,true,0)==Chain::Wait,"known owner not ready waits");
 Check(ClassifyEntry(false,true,3)==Chain::KnownReady,"formal Ready permits audited chain");
 Check(ClassifyEntry(false,true,2)==Chain::Refuse && ClassifyEntry(false,false,3)==Chain::Refuse,"unknown/error hook owner refused");
 auto v=BuildView(m,f);Check(!v.empty(),"entry button painted");m.open=true;
 {
  Menu background;background.open=true;background.settings=true;
  auto paints=BuildView(background,{640,360,true,false});
  bool mods=false,modal=false;for(auto const& paint:paints){mods|=paint.text=="Mods";modal|=paint.text.find("ModMenuBedrock / Settings")!=std::string::npos;}
  Check(mods && !modal,"unfocused verified main menu retains only the visible Mods entry");
  Check(BuildView(background,{640,360,false,false}).empty(),"unfocused unknown scene still paints nothing");
 }
 m.entries={{"a","物品管理器","1.0","Author","A long description of the installed plugin.","Installed","https://example.test"}};
 v=BuildView(m,f);bool name{},settings{};for(auto const& p:v){name|=p.text=="物品管理器";settings|=p.text=="Settings";
  Check(p.rect.x>=0 && p.rect.y>=0 && p.rect.w>=0 && p.rect.h>=0 && p.rect.x+p.rect.w<=f.width+.01f && p.rect.y+p.rect.h<=f.height+.01f,"paint stays in viewport");
 }Check(name && settings,"catalog and controls painted");
 m.settings=true;v=BuildView(m,f);bool key{};for(auto const& p:v)key|=p.text.find("Open key")!=std::string::npos;Check(key,"own settings painted");
 Check(BuildView(m,{640,360,false,true}).empty(),"unknown scene paints nothing");
 Check(BuildView(m,{100,80,true,true}).empty(),"tiny viewport paints nothing");
 for(auto small:{Frame{240,160,true,true},Frame{320,180,true,true}}){
  auto p=BuildView(m,small);bool bounded=Bounded(p,small);
  Check(bounded,"settings stays inside small GUI viewport");
 }
 m.settings=false;m.configuring=true;m.draft.id="AutoRestock";m.draft.valid=true;
 m.draft.fields={{"/enabled","Enabled",SettingType::Boolean,true},{"/preRestockThreshold","Restock below",SettingType::Integer,17,1,64}};
 v=BuildView(m,f);bool apply=false,cancel=false,enabled=false;
 for(auto const& p:v){apply|=p.text=="Apply";cancel|=p.text=="Cancel";enabled|=p.text=="Enabled";}
 Check(apply && cancel && enabled,"Configure paints typed fields and explicit Apply Cancel");
 for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},Frame{854,480,true,true}}){
  m.draft.message="Saved with backup. Live application is not confirmed; disabled DLLs need restart.";
  auto paints=BuildView(m,viewport);bool bounded=Bounded(paints,viewport);
  Check(bounded,"Configure fields status and actions stay inside supported GUI viewports");
 }
 Menu native;native.nativeEntry=true;Check(BuildView(native,f).empty(),"native entry is not imitated by a floating rectangle");
 native.open=native.settings=true;auto own=BuildView(native,f);bool move=false,openKey=false;for(auto const& p:own){move|=p.text=="Move Mods button";openKey|=p.text.find("Open key:")!=std::string::npos;}
 Check(!move && openKey,"native entry keeps shortcut settings but removes irrelevant position controls");
 {
  Menu catalog;catalog.open=true;catalog.entries={{"AutoRestock","AutoRestock","0.3.1","XuanDiNa","Automatically restock matching items.","Ready","https://example.test"},
      {"FakeSneak","FakeSneak","0.2","XuanDiNa","Stay safe near block edges.","Disabled manifest",""}};
  auto layout=Arrange(catalog.config,f);auto draw=BuildView(catalog,f);
  bool transparent=false,count=false,summary=false,outline=false,fullSelection=false;
  for(auto const& p:draw){
   if(p.text.empty() && p.rect.x==0 && p.rect.y==0 && p.rect.w==640 && p.rect.h==360)transparent|=p.color.a<.8f;
   count|=p.text=="Showing 2 plugins";
   summary|=p.text.find("Automatically restock")!=std::string::npos && p.rect.x<320 && p.rect.y>=layout.list.y;
   if(p.text.empty() && p.rect.x>=layout.list.x && p.rect.x<layout.list.x+layout.list.w && p.rect.y>=layout.list.y && p.rect.y<layout.list.y+32){
    outline|=p.rect.w>layout.list.w-12 && p.rect.h<=1.1f && p.color.r>.6f;
    fullSelection|=p.rect.w>layout.list.w-12 && p.rect.h>15 && p.color.g>p.color.r+.1f;
   }
  }
  Check(transparent,"catalog dims rather than replaces the underlying native menu background");
  Check(count && summary,"reference-style catalog exposes filtered count and short row description");
  Check(outline && !fullSelection,"selected catalog row uses a thin neutral outline instead of a green slab");
  Check(layout.list.w>=290 && layout.list.w<=315 && layout.configure.y<layout.details.y+80,"balanced split places Configure beside selected plugin header not at bottom");
  catalog.query="edge";draw=BuildView(catalog,f);bool one=false;for(auto const& p:draw)one|=p.text=="Showing 1 of 2 plugins";
  Check(one,"count reflects actual search filtering instead of installed total only");
  catalog.entries.resize(40);catalog.scroll=10;draw=BuildView(catalog,f);bool thumb=false;
  for(auto const& p:draw)thumb|=p.text.empty() && p.rect.x>=layout.list.x+layout.list.w-5 && p.rect.w<=5 && p.rect.h>5 && p.rect.h<layout.list.h && p.color.r>.4f;
  Check(thumb,"long catalog has a visible proportional scroll thumb");
  catalog.query.clear();catalog.entries.resize(1);catalog.scroll=0;
  catalog.entries[0].description="First paragraph.\nSecond paragraph.";
  draw=BuildView(catalog,f);bool first=false,second=false,newline=false;
  for(auto const& p:draw){
   newline|=p.text.find('\n')!=std::string::npos;
   if(p.rect.x>=layout.description.x){first|=p.text=="First paragraph.";second|=p.text=="Second paragraph.";}
  }
  Check(first && second && !newline,"description paragraphs stay separate paint lines instead of escaping single-line bounds");
  catalog.entries[0].description="Small words wrap at a word boundary without dividing ordinarywords across rows.";
  draw=BuildView(catalog,f);bool wordBreak=false;
  for(auto const& p:draw)if(p.rect.x>=layout.description.x && p.rect.y>=layout.description.y)wordBreak|=p.text=="Small words wrap at a word boundary without dividing";
  Check(wordBreak,"detail description wraps at ordinary word boundaries");
  for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},Frame{640,360,true,true},Frame{854,480,true,true}}){
   auto l=Arrange(catalog.config,viewport);auto paints=BuildView(catalog,viewport);bool bounds=true;
   for(auto const& p:paints)bounds&=p.rect.x>=0 && p.rect.y>=0 && p.rect.x+p.rect.w<=viewport.width+.01f && p.rect.y+p.rect.h<=viewport.height+.01f;
   Check(bounds && l.search.y+l.search.h<=l.list.y && l.list.y+l.list.h<=l.done.y && l.configure.y+l.configure.h<=l.details.y+l.details.h,"reference catalog remains bounded and actions do not overlap on supported viewports");
  }
 }
 {
  Menu page;page.open=page.configuring=true;page.draft.id="MouseTweaksBedrock";page.draft.valid=true;page.draft.fields=SettingsSchema(page.draft.id).fields;
  auto categories=SettingCategories(page.draft);page.settingCategory=int(std::find(categories.begin(),categories.end(),"Mouse wheel")-categories.begin());page.fieldSelected=7;
  auto paints=BuildView(page,f);bool group=false,resetAll=false,reset=false,help=false;
  for(auto const& p:paints){group|=p.text=="Mouse wheel";resetAll|=p.text=="Reset all";reset|=p.text=="Reset";help|=p.text.find("Choose normal")!=std::string::npos;}
  Check(group && reset && resetAll && help,"shared Configure renders category reset and selected-field help");
  bool defaults=false;for(auto const& p:paints)defaults|=p.text.find("Default: Normal")!=std::string::npos;
  Check(defaults,"selected field always exposes its default separately from long help text");
  page.choiceField=7;page.choiceSelected=2;paints=BuildView(page,f);bool option=false;auto popup=ArrangeSettings(page,f).popup;
  for(auto const& p:paints)if(p.rect.y>=popup.y && p.rect.y<popup.y+popup.h)option|=p.text=="Position-aware inverted";
  Check(option,"dropdown paints every choice at the popup hitbox instead of cycling silently");
  page.draft.fields=SettingsSchema("AutoRestock").fields;page.settingCategory=0;page.choiceField=-1;page.editField=1;page.numberText="999";page.numberError="Enter a whole number from 1 to 64.";
  page.fieldSelected=1;bool compactDefault=false;
  for(auto const& p:BuildView(page,{320,180,true,true}))compactDefault|=p.text.find("Default: 6")!=std::string::npos;
  Check(compactDefault,"compact viewport prioritizes numeric bounds and real default over long description");
  paints=BuildView(page,f);bool buffer=false,error=false;for(auto const& p:paints){buffer|=p.text=="999_";error|=p.text.find("Enter a whole number")!=std::string::npos;}
  Check(buffer && error,"active numeric buffer and validation error are visible without committing invalid number");
  for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},Frame{640,360,true,true},Frame{854,480,true,true}}){
   auto layout=ArrangeSettings(page,viewport);auto base=Arrange(page.config,viewport);
   Check(layout.help.y+layout.help.h<=layout.status.y && layout.status.y+layout.status.h<=base.apply.y,"help status and action controls never overlap even in compact Configure");
   auto draw=BuildView(page,viewport);bool bounded=Bounded(draw,viewport);
   Check(bounded,"typed Configure paints stay inside each supported viewport");
  }
 }
 {
  Check(LocaleFromCode("zh_TW")==Locale::TraditionalChinese && LocaleFromCode("ZH-tw")==Locale::TraditionalChinese,"game locale accepts underscore hyphen and ASCII case");
  Check(LocaleFromCode("zh_CN")==Locale::SimplifiedChinese && LocaleFromCode("en_GB")==Locale::English && LocaleFromCode("fr_FR")==Locale::English,"unsupported game languages fall back to English");
  Check(LocaleFromCode("zh_TWgarbage")==Locale::English && LocaleFromCode("")==Locale::English,"partial or missing language code does not guess Chinese");
  Check(TextWidth("模組")==20 && TextWidth("Mods")==24 && TextWidth("A模")==16,"CJK layout uses codepoints not UTF8 byte count");
  Menu page;page.open=true;page.nativeEntry=true;page.locale=Locale::TraditionalChinese;
  page.entries={{"AutoRestock","AutoRestock","0.3.1","Author","Original plugin description","DLL loaded (Ready unknown)",""}};
  auto has=[](auto const& paints,std::string_view word){for(auto const& p:paints)if(p.text==word)return true;return false;};
  auto paints=BuildView(page,f);
  Check(has(paints,"模組") && has(paints,"設定") && has(paints,"完成"),"Traditional Chinese catalog uses game locale for actions and title");
  Check(has(paints,"顯示 1 個插件") && has(paints,"Original plugin description"),"localized count preserves arbitrary plugin metadata");
  page.locale=Locale::SimplifiedChinese;paints=BuildView(page,f);
  Check(has(paints,"模组") && has(paints,"设置") && has(paints,"完成"),"locale change affects next paint without reopening or changing config");
  page.configuring=true;page.draft.id="FakeSneak";page.draft.valid=true;page.draft.fields=SettingsSchema("FakeSneak").fields;
  auto original=page.draft.fields;paints=BuildView(page,f);
  Check(has(paints,"启用") && has(paints,"应用") && has(paints,"取消"),"Configure localizes typed settings and save actions");
  Check(page.draft.fields[1].path=="/toggleKey" && std::get<std::string>(page.draft.fields[1].value)=="F5" && page.draft.fields[0].value==original[0].value,"translations cannot alter JSON paths key tokens or feature values");
  auto choice=SettingsSchema("MouseTweaksBedrock").fields[7];choice.value=2;
  Check(DisplaySetting(choice,Locale::TraditionalChinese)=="依格子位置" && std::get<int>(choice.value)==2,"choice caption translates while stored index is unchanged");
  Check(DisplaySetting(page.draft.fields[1],Locale::TraditionalChinese)=="F5","key tokens remain canonical");
  Check(DisplayMessage("Config changed externally. Cancel and reopen Configure.",Locale::TraditionalChinese).find("設定")!=std::string::npos,"external-edit error is localized at presentation boundary");
  for(auto locale:{Locale::TraditionalChinese,Locale::SimplifiedChinese})for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},f,Frame{854,480,true,true}}){
   page.locale=locale;auto draw=BuildView(page,viewport);
   bool valid=true;for(auto const& p:draw)valid&=ValidUtf8(p.text);
   Check(valid && Bounded(draw,viewport),"localized Configure has bounded rectangles and complete UTF8");
  }
 }
 {
  Menu page;page.nativeEntry=true;page.open=true;Entry entry{"ExternalPlugin","ExternalPlugin","1"};entry.settingsAvailable=true;page.entries={entry};
  auto has=[](auto const& paints,std::string_view word){for(auto const& p:paints)if(p.text==word)return true;return false;};
  Check(has(BuildView(page,f),"Configure"),"unknown plugin uses worker-published Configure support without view filesystem reads");
  page.entries[0].id="AutoRestock";page.entries[0].settingsAvailable=false;page.entries[0].settingsReason="Description plugin version mismatch";
  auto paints=BuildView(page,f);Check(has(paints,"No settings available") && has(paints,"Description plugin version mismatch"),"invalid description disables built-in Configure and explains refusal");
  page.configuring=true;page.draft.id="ExternalPlugin";page.draft.valid=true;
  SettingField field{"/mode","Author mode",SettingType::Choice,1,0,1,{"A","B"}};field.external=true;
  field.labelText={"Author mode","作者模式","作者模式"};field.category="Author group";field.categoryText={"Author group","作者群組","作者分组"};
  field.description="Custom help";field.descriptionText={"Custom help","自訂說明","自定义说明"};field.optionText={{"A","甲","甲"},{"B","乙","乙"}};
  page.draft.fields={field};page.settingCategory=1;page.fieldSelected=0;
  for(auto locale:{Locale::TraditionalChinese,Locale::SimplifiedChinese}){
   page.locale=locale;paints=BuildView(page,f);
   Check(has(paints,"作者模式") && has(paints,"乙 v") && has(paints,locale==Locale::TraditionalChinese?"自訂說明":"自定义说明"),"external labels choice captions and selected help follow supplied locale");
   Check(has(paints,locale==Locale::TraditionalChinese?"作者群組":"作者分组"),"external category translation does not change filter identity");
   Check(page.draft.fields[0].path=="/mode" && page.draft.fields[0].value==SettingValue{1},"external translation preserves path and saved choice index");
  }
  field.label="Done";field.labelText={"Done",{},{} };page.draft.fields={field};page.locale=Locale::TraditionalChinese;
  Check(has(BuildView(page,f),"Done"),"external missing translation falls back to author English not a coincidental built-in dictionary key");
  page.draft.fields[0].options={"A","B","C","D","E","F"};page.draft.fields[0].optionText.clear();page.choiceField=0;
  for(auto viewport:{Frame{240,160,true,true},Frame{320,180,true,true},f})Check(Bounded(BuildView(page,viewport),viewport),"external six-choice popup fits existing compact shared controls");
 }
 std::cout<<checks<<" checks / "<<failures<<" failures\n";return failures?1:0;
}
