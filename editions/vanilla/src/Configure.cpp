#include "ModMenu/Configure.h"
#include "ModMenu/Slider.h"
#include <algorithm>
namespace menu {
std::vector<std::string> SettingCategories(SettingsDraft const& draft){
 std::vector<std::string> result{"All"};
 for(auto const& f:draft.fields)if(std::find(result.begin(),result.end(),f.category)==result.end())result.push_back(f.category);
 return result;
}
std::vector<int> VisibleSettings(Menu const& m){
 auto categories=SettingCategories(m.draft);std::vector<int> result;
 int c=std::clamp(m.settingCategory,0,int(categories.size())-1);
 for(int i=0;i<int(m.draft.fields.size());++i)if(!c || m.draft.fields[i].category==categories[c])result.push_back(i);
 return result;
}
ConfigureLayout ArrangeSettings(Menu const& m,Frame const& f){
 auto l=Arrange(m.config,f);ConfigureLayout out;
 out.previous={8,32,20,20};out.next={f.width-128,32,20,20};out.category={32,32,f.width-164,20};out.resetAll={f.width-104,32,96,20};
 auto fields=VisibleSettings(m);
 for(int row=0;row<l.fieldVisible;++row){
  int at=m.fieldScroll+row;if(at<0 || at>=int(fields.size()))break;
  Rect r{l.fields.x+2,l.fields.y+row*l.row,l.fields.w-4,l.row-1};
  Rect value{r.x+r.w*.42f,r.y+1,r.w*.58f-47,r.h-2};
  float inner=std::max(1.f,value.w-38);Rect rail,number;
  if(inner>=160){rail={value.x+19,value.y+4,inner*.56f,value.h-8};number={rail.x+rail.w+4,value.y,inner-rail.w-4,value.h};}
  else{rail={value.x+19,value.y,inner,9};number={value.x+19,value.y+11,inner,value.h-11};}
  out.rows.push_back({fields[at],r,value,{r.x+r.w-43,r.y+1,41,r.h-2},
    {value.x,value.y,18,value.h},{value.x+value.w-18,value.y,18,value.h},rail,number});
 }
 out.help={8,l.fields.y+l.fields.h+3,f.width-16,f.height<200?14.f:26.f};
 out.status={8,out.help.y+out.help.h+2,f.width-16,18};
 if(m.choiceField>=0 && m.choiceField<int(m.draft.fields.size())){
  Rect anchor{f.width*.42f,58,f.width*.58f-66,22};for(auto const& row:out.rows)if(row.index==m.choiceField)anchor=row.value;
  auto count=m.draft.fields[m.choiceField].options.size();float h=float(count)*out.optionHeight;
  out.popup={anchor.x,std::clamp(anchor.y+anchor.h,28.f,std::max(28.f,f.height-8-h)),std::max(40.f,anchor.w),h};
 }return out;
}
void Menu::CancelSlider(){
 if(sliderField_<0)return;
 if(sliderField_<int(draft.fields.size()) && draft.fields[sliderField_].type==SettingType::Slider){draft.fields[sliderField_].value=sliderStart_;settingsChanged=sliderChangedBefore_;}
 sliderField_=-1;action_=0;
}
void Menu::MoveSlider(float x){
 if(sliderField_>=0 && sliderField_<int(draft.fields.size()) && sliderRail_.w>6)
  settingsChanged=PositionSlider(draft.fields[sliderField_],double(x-sliderRail_.x-3)/double(sliderRail_.w-6)) || settingsChanged;
}
void Menu::ClearSettingControl(){CancelSlider();captureField=editField=choiceField=-1;bindingCapture_.Cancel();bindingTarget_=-3;numberText.clear();numberError.clear();numberReplace=false;textEditorActive=false;++textGeneration;clipboardPaste=clipboardCopy=false;}
bool Menu::CommitNumber(){
 if(editField<0)return true;
 if(editField>=int(draft.fields.size())){ClearSettingControl();return false;}
 auto& field=draft.fields[editField];auto old=field.value;
 if(!(field.type==SettingType::Text?SetText(field,numberText):SetInteger(field,numberText))){numberError=SettingInputError(field);return false;}
 settingsChanged=settingsChanged || old!=field.value;editField=-1;numberText.clear();numberError.clear();numberReplace=false;return true;
}
void Menu::ConfigureText(std::string_view text){
 if(settingsBusy || editField<0 || editField>=int(draft.fields.size()) || text.empty())return;
 auto const& field=draft.fields[editField];auto limit=SettingInputLimit(field);if(text.size()>limit)return;
 if(field.external&&field.type==SettingType::Text){if(!textEditorActive){textEditor.Begin(numberText,limit,numberReplace);textEditorActive=true;++textGeneration;}textEditor.Insert(text);numberText=textEditor.value;numberError=textEditor.error;numberReplace=false;return;}
 for(unsigned char c:text)if(field.type==SettingType::Text?(c<32 || c>=127):
  !((c>='0' && c<='9') || (field.type==SettingType::Slider && (c=='-' || (field.slider.decimal && c=='.')))))return;
 auto next=numberReplace?std::string(text):numberText+std::string(text);
 if(next.size()>limit)return;numberText=std::move(next);numberReplace=false;numberError.clear();
}
bool Menu::ConfigureKey(unsigned key,Modifiers mods,bool repeat){
 if(settingsBusy || repeat)return true;
 if(sliderField_>=0){if(key==27)CancelSlider();return true;}
 if(captureField>=0 && captureField<int(draft.fields.size())){
  auto& field=draft.fields[captureField];auto old=field.value;
  if(SetKey(field,key)){captureField=-1;settingsChanged=settingsChanged || old!=field.value;}
  else if(key==27)captureField=-1; // External fields without NONE retain their declared contract.
  return true;
 }
 if(editField>=0){
  auto const&field=draft.fields[editField];if(field.external&&field.type==SettingType::Text){
   if(!textEditorActive){textEditor.Begin(numberText,SettingInputLimit(field),numberReplace);textEditorActive=true;++textGeneration;}
   if(key==27){ClearSettingControl();return true;}if(key==13||key==9){CommitNumber();textEditorActive=false;++textGeneration;return true;}
   if(mods.control){if(key=='A')textEditor.SelectAll();if(key=='V'){clipboardPaste=true;clipboardGeneration=textGeneration;}if(key=='C'){clipboardCopy=true;clipboardGeneration=textGeneration;clipboardText=textEditor.Selection();}return true;}if(mods.alt)return true;
   if(key==37||key==39)textEditor.Move(key==37?-1:1,mods.shift);else if(key==35||key==36)textEditor.Edge(key==35,mods.shift);else if(key==8||key==46)textEditor.Erase(key==8);else textEditor.Insert(AsciiKey(key,mods.shift,mods.caps));numberText=textEditor.value;numberError=textEditor.error;numberReplace=false;return true;
  }
  if(key==27){ClearSettingControl();return true;}
  if(key==13 || key==9){CommitNumber();return true;}
  if(mods.control || mods.alt)return true;
  if(key==8){if(!numberText.empty())numberText.pop_back();numberReplace=false;numberError.clear();}
  else if(key==46){numberText.clear();numberReplace=false;numberError.clear();}
  else ConfigureText(AsciiKey(key,mods.shift,mods.caps));
  return true;
 }
 if(choiceField>=0 && choiceField<int(draft.fields.size())){
  auto& field=draft.fields[choiceField];
  if(key==27){choiceField=-1;return true;}
  if(key==38 || key==37)choiceSelected=std::max(0,choiceSelected-1);
  if(key==40 || key==39)choiceSelected=std::min(int(field.options.size())-1,choiceSelected+1);
  if(key==13 || key==32){auto old=field.value;field.value=choiceSelected;settingsChanged=settingsChanged || old!=field.value;choiceField=-1;}
  return true;
 }
 auto shortcutMask=BindingModifiers(mods,key);
 if(key==27 || (key==config.openKey && shortcutMask==config.openModifiers)){
  configuring=false;draft={};settingsChanged=false;settingsRequest=Request::None;ClearSettingControl();++settingsGeneration;return true;
 }
 if(!draft.valid || draft.fields.empty() || mods.control || mods.alt)return true;
 auto visible=VisibleSettings(*this);if(visible.empty())return true;
 auto at=std::find(visible.begin(),visible.end(),fieldSelected);
 int pos=at==visible.end()?0:int(at-visible.begin());
 if(key==40)pos=std::min(int(visible.size())-1,pos+1);
 if(key==38)pos=std::max(0,pos-1);
 if(key==9)pos=(pos+(mods.shift?int(visible.size())-1:1))%int(visible.size());
 fieldSelected=visible[pos];
 if(pos<fieldScroll)fieldScroll=pos;
 // Tick clamps scroll on the next callback; current viewport is known in Core.
 auto& field=draft.fields[fieldSelected];
 if(key==37 || key==39){
  if(field.type==SettingType::Integer || field.type==SettingType::Choice || field.type==SettingType::Slider)settingsChanged=AdjustSetting(field,(key==37?-1:1)*(mods.shift?10:1)) || settingsChanged;
  else if(auto b=std::get_if<bool>(&field.value)){*b=!*b;settingsChanged=true;}
 }else if((key==36 || key==35) && field.type==SettingType::Slider){settingsChanged=PositionSlider(field,key==36?0.:1.) || settingsChanged;
 }else if(key==13 || key==32){
  if(field.type==SettingType::Key || field.type==SettingType::Binding){captureField=fieldSelected;if(field.type==SettingType::Binding)BeginBindingCapture(captureField);}
  else if(field.type==SettingType::Integer || field.type==SettingType::Text || field.type==SettingType::Slider){editField=fieldSelected;numberText=SettingText(field);numberReplace=true;numberError.clear();}
  else if(field.type==SettingType::Choice){choiceField=fieldSelected;choiceSelected=std::get<int>(field.value);}
  else if(auto b=std::get_if<bool>(&field.value)){*b=!*b;settingsChanged=true;}
 }return true;
}
bool Menu::ConfigurePress(Event const& e,Frame const& f){
 if(settingsBusy)return true;
 auto l=Arrange(config,f);auto c=ArrangeSettings(*this,f);
 if(choiceField>=0){
  if(c.popup.Contains(e.x,e.y)){row_=int((e.y-c.popup.y)/c.optionHeight);action_=17;}else action_=18;
  return true;
 }
 if(l.cancel.Contains(e.x,e.y))action_=11;
 else if(l.apply.Contains(e.x,e.y) && draft.valid && (settingsChanged || editField>=0))action_=12;
 else if(draft.valid){
  if(c.previous.Contains(e.x,e.y))action_=14;
  else if(c.next.Contains(e.x,e.y))action_=15;
  else if(c.resetAll.Contains(e.x,e.y))action_=16;
  else for(auto const& r:c.rows)if(r.bounds.Contains(e.x,e.y)){
   row_=r.index;action_=r.reset.Contains(e.x,e.y)?20:13;
   auto& field=draft.fields[r.index];
   if(field.type==SettingType::Integer || field.type==SettingType::Slider){if(r.minus.Contains(e.x,e.y))action_=21;else if(r.plus.Contains(e.x,e.y))action_=22;}
   if(field.type==SettingType::Slider && r.rail.Contains(e.x,e.y)){
    action_=0;if(!CommitNumber())return true;ClearSettingControl();fieldSelected=r.index;
    sliderField_=r.index;sliderStart_=std::get<SliderValue>(field.value);sliderRail_=r.rail;sliderWidth_=f.width;sliderHeight_=f.height;sliderChangedBefore_=settingsChanged;MoveSlider(e.x);
   }
   break;
  }
 }return true;
}
void Menu::ConfigureClick(int action,int row,Event const& e,Frame const& f){
 if(!configuring || settingsBusy)return;
 auto l=Arrange(config,f);auto c=ArrangeSettings(*this,f);
 if(action==11 && l.cancel.Contains(e.x,e.y)){
  configuring=false;draft={};settingsRequest=Request::None;settingsChanged=false;ClearSettingControl();++settingsGeneration;return;
 }
 if(action==12 && l.apply.Contains(e.x,e.y) && draft.valid){
  if(!CommitNumber())return;
  if(settingsChanged){settingsRequest=Request::Apply;settingsBusy=true;ClearSettingControl();}return;
 }
 if(!draft.valid)return;
 if(action==18){choiceField=-1;return;}
 if(action==17){
  if(choiceField>=0 && choiceField<int(draft.fields.size()) && c.popup.Contains(e.x,e.y) && row==int((e.y-c.popup.y)/c.optionHeight)){
   auto& field=draft.fields[choiceField];if(row>=0 && row<int(field.options.size())){auto old=field.value;field.value=row;settingsChanged=settingsChanged || old!=field.value;}
  }choiceField=-1;return;
 }
 if((action==14 && c.previous.Contains(e.x,e.y)) || (action==15 && c.next.Contains(e.x,e.y))){
  ClearSettingControl();auto count=int(SettingCategories(draft).size());settingCategory=(settingCategory+(action==14?count-1:1))%count;
  fieldScroll=0;auto visible=VisibleSettings(*this);if(!visible.empty())fieldSelected=visible.front();return;
 }
 if(action==16 && c.resetAll.Contains(e.x,e.y)){ClearSettingControl();settingsChanged=ResetSettings(draft) || settingsChanged;return;}
 for(auto const& r:c.rows)if(r.index==row && r.bounds.Contains(e.x,e.y)){
  if(action==20 && r.reset.Contains(e.x,e.y)){ClearSettingControl();fieldSelected=row;settingsChanged=ResetSetting(draft.fields[row]) || settingsChanged;return;}
  if(action!=13 && action!=21 && action!=22)return;
  if(action==21 && !r.minus.Contains(e.x,e.y))return;
  if(action==22 && !r.plus.Contains(e.x,e.y))return;
  if(editField>=0 && editField!=row && !CommitNumber())return;
  fieldSelected=row;auto& field=draft.fields[row];
  if(action==21 || action==22){if(!CommitNumber())return;settingsChanged=AdjustSetting(field,action==21?-1:1) || settingsChanged;}
  else if(field.type==SettingType::Key || field.type==SettingType::Binding){ClearSettingControl();captureField=row;if(field.type==SettingType::Binding)BeginBindingCapture(row);}
  else if(field.type==SettingType::Choice){ClearSettingControl();choiceField=row;choiceSelected=std::get<int>(field.value);}
  else if(field.type==SettingType::Integer || field.type==SettingType::Text || (field.type==SettingType::Slider && r.number.Contains(e.x,e.y))){if(editField!=row){ClearSettingControl();editField=row;numberText=SettingText(field);numberReplace=true;}}
  else if(auto b=std::get_if<bool>(&field.value)){ClearSettingControl();*b=!*b;settingsChanged=true;}
  return;
 }
}
}
