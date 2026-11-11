#include "ModMenu/Core.h"
#include "ModMenu/Storage.h"
#include "ModMenu/Configure.h"
#include <algorithm>
#include <cmath>
namespace menu {
WorkspaceSnapshot const* Menu::WorkspaceFor(std::string const&id)const{for(auto const&s:workspaces)if(s.definition&&s.definition->id==id)return &s;return nullptr;}
void Menu::CompleteClipboard(std::uint64_t generation,std::string const&text){constexpr auto domain=std::uint64_t(1)<<63;constexpr auto empty="Clipboard is empty, unreadable or exceeds limits.";if((generation&domain)&&workspaceOpen&&!workspace.editing.empty()&&(generation&~domain)==workspace.editGeneration){if(text.empty()){workspace.notice=empty;return;}workspace.editor.Insert(text);workspace.notice=workspace.editor.error;}else if(!(generation&domain)&&configuring&&textEditorActive&&generation==textGeneration){if(text.empty()){numberError=empty;return;}textEditor.Insert(text);numberText=textEditor.value;numberError=textEditor.error;}}
MenuScene ClassifyMenuScene(std::string_view name){
    if(name=="start_screen")return MenuScene::Main;
    if(name=="pause_screen")return MenuScene::Pause;
    return MenuScene::Unknown;
}
bool Menu::OpenNative(Frame const& f){
    Tick(f);if(!nativeEntry || f.observerOnly || !f.eligible || !f.foreground || f.width<240 || f.height<160)return false;
    open=true;settings=editing=keyCapture=searchFocus=false;
    pasteRequested_=false;searchRepeatKey_=0;searchRepeatAt_=0;return true;
}
void Menu::CompleteSettings(unsigned long long generation,SettingsDraft response){
    if(!configuring || generation!=settingsGeneration || response.id!=draft.id)return;
    settingsChanged=settingsChanged && !response.saved;
    draft=std::move(response);settingsBusy=false;settingsRequest=Request::None;ClearSettingControl();
}
std::string AsciiKey(unsigned key,bool shift,bool caps){
    if(key>='A' && key<='Z')return std::string(1,char((shift!=caps)?key:key+32));
    if(key>='0' && key<='9')return std::string(1,shift?")!@#$%^&*("[key-'0']:char(key));
    if(key>=96 && key<=105)return std::string(1,char('0'+key-96));
    switch(key){
    case 32:return " ";case 186:return shift?":":";";case 187:return shift?"+":"=";
    case 188:return shift?"<":",";case 189:return shift?"_":"-";case 190:return shift?">":".";
    case 191:return shift?"?":"/";case 192:return shift?"~":"`";
    case 219:return shift?"{":"[";case 220:return shift?"|":std::string(1,char(92));
    case 221:return shift?"}":"]";case 222:return std::string(1,shift?char(34):char(39));
    default:return {};
    }
}
namespace {
float Finite(float v,float fallback){return std::isfinite(v)?v:fallback;}
std::string Fold(std::string s){for(auto& c:s)if(c>='A'&&c<='Z')c=char(c+'a'-'A');return s;}
void Backspace(std::string& s){if(s.empty())return;auto i=s.size()-1;while(i && (static_cast<unsigned char>(s[i])&0xc0)==0x80)--i;s.resize(i);}
}
bool Rect::Contains(float px,float py) const {return px>=x && py>=y && px<x+w && py<y+h;}
Rect Placement::Bounds(float width,float height) const {
    width=std::max(0.f,Finite(width,0));height=std::max(0.f,Finite(height,0));
    auto s=std::clamp(Finite(scale,1),.5f,2.f);
    float w=std::min(width,80*s),h=std::min(height,24*s);
    float px=Finite(x,12),py=Finite(y,12);
    if(anchor==1 || anchor==3)px=width-w-px;
    if(anchor==2 || anchor==3)py=height-h-py;
    return {std::clamp(px,0.f,width-w),std::clamp(py,0.f,height-h),w,h};
}
Layout Arrange(Config const& c,Frame const& f) {
    Layout l;l.button=c.button.Bounds(f.width,f.height);
    float pad=8,w=std::max(0.f,f.width-pad*2);
    float gap=12,left=std::max(0.f,(w-gap)*.5f),right=std::max(0.f,w-left-gap);
    l.search={pad,28,left,20};l.count={pad,52,left,10};
    l.list={pad,68,left,std::max(0.f,f.height-108)};
    l.details={pad+left+gap,28,right,std::max(0.f,f.height-68)};
    float footerWidth=std::min(160.f,std::max(0.f,(w-8)/2));
    l.done={f.width/2+4,f.height-pad-24,footerWidth,24};
    l.settings={f.width/2-4-footerWidth,f.height-pad-24,footerWidth,24};
    float step=std::min(30.f,std::max(20.f,(f.height-88)/4));
    l.back=l.done;l.edit={pad+8,pad+38,150,20};l.reset={pad+8,pad+38+step,150,20};
    l.key={pad+8,pad+38+step*2,150,20};l.scale={pad+8,pad+38+step*3,150,20};
    l.configure={l.details.x+4,l.details.y+46,std::max(0.f,l.details.w-8),20};
    l.description={l.details.x,l.configure.y+l.configure.h+5,l.details.w,std::max(0.f,l.details.y+l.details.h-l.configure.y-l.configure.h-5)};
    l.apply={f.width-pad-152,f.height-pad-24,72,24};l.cancel={f.width-pad-72,f.height-pad-24,72,24};
    l.fields={pad,58,w,std::max(24.f,f.height-144)};
    l.row=24;l.visible=std::max(1,int(l.list.h/l.catalogRow));l.fieldVisible=std::max(1,int(l.fields.h/l.row));return l;
}
void Menu::SuspendInput() {
    if(focusSuspended_)return;
    focusSuspended_=awaitingKeyboard_=true;
    for(unsigned k=0;k<held_.size();++k)releaseKeys_[k]=releaseKeys_[k] || held_[k] || capturedKeys_[k];
    releaseButtons_|=capturedButtons_;
    if(!workspace.pressed.empty())releaseButtons_|=1u<<1;
    if(editing){editing=false;open=settings=true;}
    keyCapture=searchFocus=pressed_=dragging_=false;action_=0;
    workspace.SuspendInput();ClearSettingControl();
    clipboardPaste=clipboardCopy=pasteRequested_=false;searchRepeatKey_=0;searchRepeatAt_=0;
    shortcutPrimed_=keyboardPrimed_=false;previousKeys_.fill(false);
    held_.fill(false);capturedKeys_.fill(false);capturedButtons_=0;
    mappedCount_=0;mappedOverflow_=false;
}
void Menu::Tick(Frame const& f) {
    if(!keyCapture && !(configuring && captureField>=0)){bindingCapture_.Cancel();bindingTarget_=-3;}
    if(f.observerOnly && f.foreground)return;
    bool knownOrigin=origin_==MenuScene::Main || origin_==MenuScene::Pause;
    // A verified background sibling carries no viewport/category. It cannot
    // invalidate (or resume) the independently verified top menu's navigation.
    if(f.observerOnly && !f.foreground && knownOrigin){SuspendInput();return;}
    if(sliderField_>=0 && (f.width!=sliderWidth_ || f.height!=sliderHeight_))CancelSlider();
    bool sceneChanged=f.eligible && f.scene!=MenuScene::Unknown && origin_!=MenuScene::Unknown && origin_!=f.scene;
    if(sceneChanged || !f.eligible || (knownOrigin && f.scene==MenuScene::Unknown) ||
       f.width<240 || f.height<160 || (!f.foreground && !knownOrigin)){
        open=settings=editing=keyCapture=searchFocus=pressed_=dragging_=false;
        bindingCapture_.Cancel();bindingTarget_=-3;
        workspaceOpen=false;workspace.Clear();clipboardPaste=clipboardCopy=false;
        shortcutPrimed_=false;
        keyboardPrimed_=false;previousKeys_.fill(false);
        pasteRequested_=false;searchRepeatKey_=0;searchRepeatAt_=0;
        mappedCount_=0;mappedOverflow_=false;
        held_.fill(false);capturedKeys_.fill(false);capturedButtons_=0;action_=0;
        focusSuspended_=awaitingKeyboard_=false;releaseKeys_.fill(false);releaseButtons_=0;
        if(configuring){configuring=settingsBusy=settingsChanged=false;settingsRequest=Request::None;draft={};++settingsGeneration;ClearSettingControl();}
    }else if(!f.foreground){
        SuspendInput();return;
    }else if(focusSuspended_){
        focusSuspended_=false;
        // Cached host-owned snapshots only; no provider callbacks or work
        // cancellation on the render/input thread. A new session is a new UI.
        if(workspaceOpen){
            auto s=workspace.snapshot.definition?WorkspaceFor(workspace.snapshot.definition->id):nullptr;
            if(!s || s->session!=workspace.snapshot.session){workspaceOpen=false;workspace.Clear();}
            else workspace.Accept(*s);
        }
    }
    // An owned category, never a native scene pointer. Native eligibility is
    // independently established for every callback; this only cancels stale UI.
    origin_=f.eligible && f.foreground?f.scene:MenuScene::Unknown;
    auto n=static_cast<int>(Filtered().size());auto l=Arrange(config,f);
    selected=std::clamp(selected,0,std::max(0,n-1));scroll=std::clamp(scroll,0,std::max(0,n-l.visible));
    settingCategory=std::clamp(settingCategory,0,int(SettingCategories(draft).size())-1);
    fieldSelected=std::clamp(fieldSelected,0,std::max(0,int(draft.fields.size())-1));
    fieldScroll=std::clamp(fieldScroll,0,std::max(0,int(VisibleSettings(*this).size())-l.fieldVisible));
}
bool Menu::PollShortcut(bool down,Frame const& f,Modifiers mods) {
    Tick(f);
    if(f.observerOnly && f.foreground)return false;
    if(!f.eligible || !f.foreground || f.width<240 || f.height<160 ||
       (f.scene!=MenuScene::Main && f.scene!=MenuScene::Pause) || !UsableKey(config.openKey)){
        shortcutPrimed_=false;return false;
    }
    // Samples never stand in for real typing or keybind capture. Reprime after
    // either capture so the same held key cannot accidentally close the page.
    if(keyCapture || (configuring && captureField>=0)){
        shortcutPrimed_=false;return false;
    }
    auto key=config.openKey;
    if(!down)releaseKeys_[key]=false;
    if(!shortcutPrimed_ || shortcutKey_!=key || shortcutModifiers_!=config.openModifiers){
        shortcutPrimed_=true;shortcutKey_=key;shortcutModifiers_=config.openModifiers;
        // Entering an eligible scene/refocusing while already held is not a
        // new shortcut press. A released sample still clears any raw capture.
        if(down){held_[key]=true;return false;}
    }
    // Native raw receipt and the local sample share this held/capture state.
    // Either order handles one physical press once; releases cannot stick.
    if(down==held_[key])return false;
    auto consumed=Input({Kind::Key,0,0,int(key),down?1:0,{},mods},f);
    return down && consumed;
}
bool Menu::PollKeyboard(std::array<bool,256> const& keys,Frame const& f,bool caps,std::uint64_t now) {
    Tick(f);
    if(f.observerOnly && f.foreground)return false;
    if(!f.eligible || !f.foreground || f.width<240 || f.height<160 ||
       (f.scene!=MenuScene::Main && f.scene!=MenuScene::Pause)){
        keyboardPrimed_=false;shortcutPrimed_=false;return false;
    }
    Modifiers mods{keys[16] || keys[0xA0] || keys[0xA1],keys[17] || keys[0xA2] || keys[0xA3],keys[18] || keys[0xA4] || keys[0xA5],caps,keys[0xA4]};
    if(!keyboardPrimed_){
        keyboardPrimed_=true;previousKeys_=keys;
        // Refocus/scene entry is not a new press, including keys held before
        // capture. Raw receipts and samples use the same repeat bookkeeping.
        for(unsigned k=8;k<255;++k){held_[k]=keys[k];if(awaitingKeyboard_)releaseKeys_[k]=keys[k];}
        awaitingKeyboard_=false;
        return PollShortcut(config.openKey<keys.size() && keys[config.openKey],f,mods);
    }
    auto previous=previousKeys_;previousKeys_=keys;
    // Releases must run even after Escape closes our modal or capture ends.
    for(unsigned k=8;k<255;++k)if(!keys[k] && (previous[k] || capturedKeys_[k]))
        Input({Kind::Key,0,0,int(k),0,{},mods},f);
    bool capture=keyCapture || (configuring && captureField>=0);
    bool escape=keys[27] && !previous[27];
    bool capturedPress=false;
    if(escape && (open || editing) && !held_[27]){
        Input({Kind::Key,0,0,27,1,{},mods},f);capturedPress=true;
    }else if(capture){
        // Escape wins over simultaneous candidates. Generic modifiers are
        // aliases, not bindable keys; their left/right versions are distinct.
        for(unsigned k=8;k<255 && !escape;++k){
            if(k==16 || k==17 || k==18 || k==27 || !keys[k] || previous[k] || held_[k])continue;
            Input({Kind::Key,0,0,int(k),1,{},mods},f);
            if(!keyCapture && !(configuring && captureField>=0))break;
        }
    }
    // Do not reinterpret a binding/cancel press as the open shortcut in the
    // same frame. Prime its current state so only a later fresh press acts.
    if(capture || capturedPress){
        searchRepeatKey_=0;searchRepeatAt_=0;
        shortcutPrimed_=true;shortcutKey_=config.openKey;shortcutModifiers_=config.openModifiers;
        if(config.openKey<keys.size() && keys[config.openKey])held_[config.openKey]=true;
        return false;
    }
    if(configuring || workspaceOpen){
        for(unsigned k=8;k<255 && (configuring || workspaceOpen);++k){
            if(k==27 || k==config.openKey || !keys[k] || previous[k] || held_[k])continue;
            if(k==8 || k==9 || k==13 || k==32 || k==35 || k==36 || k==37 || k==38 || k==39 || k==40 || k==46 || !AsciiKey(k,mods.shift,mods.caps).empty())
                Input({Kind::Key,0,0,int(k),1,{},mods},f);
        }
    }
    if(!SearchActive() || !searchRepeatKey_ || !keys[searchRepeatKey_] || mods.control || mods.alt){searchRepeatKey_=0;searchRepeatAt_=0;}
    if(SearchActive()){
        for(unsigned k=8;k<255;++k){
            bool printable=!AsciiKey(k,mods.shift,mods.caps).empty();
            if(!keys[k] || previous[k] || (k!=8 && k!=9 && k!=13 && !printable))continue;
            Input({Kind::Key,0,0,int(k),1,{},mods},f);
            if(!SearchActive()){searchRepeatKey_=0;searchRepeatAt_=0;break;}
            if(now && !mods.control && !mods.alt && (k==8 || printable)){
                searchRepeatKey_=k;searchRepeatAt_=now+400;
            }
        }
        if(SearchActive() && searchRepeatKey_ && now && now>=searchRepeatAt_){
            SearchKey(searchRepeatKey_,mods);searchRepeatAt_=now+50;
        }
    }
    return PollShortcut(config.openKey<keys.size() && keys[config.openKey],f,mods);
}
void Menu::BeginBindingCapture(int target){
    auto sample=previousKeys_;
    for(unsigned k=0;k<sample.size();++k)sample[k]=sample[k]||held_[k];
    bindingTarget_=target;bindingCapture_.Begin(sample,target>=0?draft.fields[target].bindingModifiers:modmenu::binding::AllModifiers);
}
bool Menu::CaptureBindingKey(unsigned key,bool down,Modifiers mods,std::array<bool,256> const& before){
    int target=keyCapture?-2:(configuring&&captureField>=0&&captureField<int(draft.fields.size())&&draft.fields[captureField].type==SettingType::Binding?captureField:-3);
    if(target==-3){bindingCapture_.Cancel();bindingTarget_=-3;return false;}
    if(settingsBusy)return true;
    if(target!=bindingTarget_||!bindingCapture_.Active()){bindingTarget_=target;bindingCapture_.Begin(before,target>=0?draft.fields[target].bindingModifiers:modmenu::binding::AllModifiers);}
    auto sample=held_;sample[16]=mods.shift;sample[17]=mods.control;sample[164]=mods.leftAlt;
    auto result=bindingCapture_.Event(key,down,sample);
    if(result){
        if(target==-2){config.openKey=result->key;config.openModifiers=result->modifiers;keyCapture=false;dirty=true;}
        else{auto& field=draft.fields[target];auto old=field.value;if(!SetBinding(field,*result)){bindingCapture_.Begin(sample,field.bindingModifiers);return true;}settingsChanged=settingsChanged||old!=field.value;captureField=-1;}
        bindingTarget_=-3;
    }
    return true;
}
std::string Menu::BindingCaptureCaption()const{
    auto mask=bindingCapture_.PendingModifiers();
    std::string prefix=(mask&modmenu::binding::Ctrl?"Ctrl+":"");
    if(mask&modmenu::binding::Shift)prefix+="Shift+";
    if(mask&modmenu::binding::LeftAlt)prefix+="LAlt+";
    if(!prefix.empty())return prefix+"...";
    return Tr(locale,bindingCapture_.NeedsPrimary()?"Press a primary key (Esc unbinds)":"Press key (Esc unbinds)");
}
bool Menu::TakePasteRequest(Frame const& f){
    Tick(f);
    if(f.observerOnly && f.foreground)return false;
    bool requested=pasteRequested_;pasteRequested_=false;
    return requested && f.eligible && f.foreground && f.width>=240 && f.height>=160 &&
        (f.scene==MenuScene::Main || f.scene==MenuScene::Pause) && SearchActive();
}
void Menu::AppendSearch(std::string const& value){
    if(value.size()>256 || !ValidUtf8(value))return;
    std::string clean;clean.reserve(value.size());
    for(unsigned char c:value)if(c>=32 && c!=127)clean+=char(c);
    if(clean.size()>256 || query.size()>256-clean.size())return;
    query+=clean;selected=scroll=0;
}
void Menu::SearchKey(unsigned key,Modifiers mods){
    if(mods.alt)return;
    if(mods.control){if(key=='V')pasteRequested_=true;return;}
    if(key==8){Backspace(query);selected=scroll=0;}
    else if(key==13 || key==9){searchFocus=false;pasteRequested_=false;searchRepeatKey_=0;searchRepeatAt_=0;}
    else AppendSearch(AsciiKey(key,mods.shift,mods.caps));
}
bool Menu::CaptureMapped(unsigned action,bool down,Frame const& f) {
    Tick(f);
    if(f.observerOnly || !f.eligible || !f.foreground || f.width<240 || f.height<160 ||
       (f.scene!=MenuScene::Main && f.scene!=MenuScene::Pause))return false;
    auto end=mappedKeys_.begin()+mappedCount_;
    auto found=std::find(mappedKeys_.begin(),end,action);
    bool owned=found!=end,blocked=BlocksMapped();
    if(!down){
        if(owned){*found=mappedKeys_[--mappedCount_];}
        return owned || blocked || mappedOverflow_;
    }
    if(owned || mappedOverflow_)return true;
    if(!blocked)return false;
    if(mappedCount_<mappedKeys_.size())mappedKeys_[mappedCount_++]=action;
    else mappedOverflow_=true; // Bounded fail-closed until scene/focus reset.
    return true;
}
bool Menu::Input(Event const& e,Frame const& f) {
    Tick(f);if(f.observerOnly || !f.eligible || !f.foreground || f.width<240 || f.height<160)return false;
    auto l=Arrange(config,f);
    if(e.kind==Kind::Key){
        if(e.code<0 || e.code>255)return open;
        auto k=static_cast<unsigned>(e.code);
        // Raw input may precede the first physical sample after refocus.
        // Only the separately primed shortcut has its own sample boundary.
        if(e.value && (releaseKeys_[k] || (awaitingKeyboard_ && (k!=config.openKey || !shortcutPrimed_)))){
            releaseKeys_[k]=held_[k]=true;return open || editing;
        }
        if(!e.value)releaseKeys_[k]=false;
        auto before=held_;bool repeat=held_[k];held_[k]=e.value!=0;
        if((!e.value || !repeat) && CaptureBindingKey(k,e.value!=0,e.modifiers,before)){
            if(e.value)capturedKeys_[k]=true;else capturedKeys_[k]=false;return true;
        }
        if(!e.value){bool consumed=capturedKeys_[k];capturedKeys_[k]=false;return consumed || open || editing;}
        auto mask=BindingModifiers(e.modifiers,k);
        bool shortcut=config.openKey && k==config.openKey && mask==config.openModifiers;
        bool consume=open || editing || shortcut;
        if(!consume)return false;
        capturedKeys_[k]=true;
        if(workspaceOpen){workspace.Input(e,f,repeat);if(workspace.close){workspaceOpen=false;workspace.Clear();}return true;}
        if(configuring){
            auto consumed=ConfigureKey(k,e.modifiers,repeat);
            auto visible=VisibleSettings(*this);auto at=std::find(visible.begin(),visible.end(),fieldSelected);
            if(at!=visible.end()){int pos=int(at-visible.begin());if(pos>=fieldScroll+l.fieldVisible)fieldScroll=pos-l.fieldVisible+1;}
            return consumed;
        }
        if(keyCapture){
            if(repeat)return true;
            if(k==27){config.openKey=0;dirty=true;keyCapture=false;}
            else if(UsableKey(k)){config.openKey=k;dirty=true;keyCapture=false;}
            return true;
        }
        if(repeat && (k==config.openKey || k==27))return true;
        if(k==27){if(editing){editing=false;open=settings=true;}else if(settings){settings=false;}else open=false;searchFocus=false;return true;}
        if(SearchActive() && (k!=config.openKey || k==8 || k==9 || k==13 || !AsciiKey(k,false,false).empty())){
            if(!repeat)SearchKey(k,e.modifiers);return true;
        }
        if(shortcut){open=!open;editing=settings=searchFocus=false;return true;}
        if(open && !settings){
            auto n=int(Filtered().size());
            if(k==38)selected=std::max(0,selected-1);
            if(k==40)selected=std::min(std::max(0,n-1),selected+1);
            if(k==36)selected=0;
            if(k==35)selected=std::max(0,n-1);
            if(k==9)searchFocus=true;
            if(selected<scroll)scroll=selected;
            if(selected>=scroll+l.visible)scroll=selected-l.visible+1;
        }
        return true;
    }
    unsigned bit=e.code>0 && e.code<8?1u<<e.code:0;
    if((e.kind==Kind::Down || e.kind==Kind::Up) && (releaseButtons_&bit)){
        if(e.kind==Kind::Up)releaseButtons_&=~bit;
        return open || editing;
    }
    if(e.kind==Kind::Down && (open || editing))capturedButtons_|=bit;
    if(e.kind==Kind::Up && workspaceOpen)capturedButtons_&=~bit;
    // NativeHost mapped Down/Up use this pointer even while Workspace owns input.
    if(workspaceOpen){if(e.kind==Kind::Move){pointerX=e.x;pointerY=e.y;}workspace.Input(e,f,false);if(workspace.close){workspaceOpen=false;workspace.Clear();}return true;}
    if(e.kind==Kind::Text){if(configuring)ConfigureText(e.text);else if(SearchActive())AppendSearch(e.text);return open;}
    if(e.kind==Kind::Move){
        pointerX=e.x;pointerY=e.y;
        if(sliderField_>=0){MoveSlider(e.x);return true;}
        if(dragging_){config.button.anchor=0;config.button.x=e.x-dragX_;config.button.y=e.y-dragY_;auto b=config.button.Bounds(f.width,f.height);config.button.x=b.x;config.button.y=b.y;dirty=true;}
        return open || editing || dragging_;
    }
    if(e.kind==Kind::Wheel){if(sliderField_>=0)return true;if(configuring){if(choiceField<0 && editField<0)fieldScroll=std::clamp(fieldScroll-std::clamp(e.value,-10000,10000),0,std::max(0,int(VisibleSettings(*this).size())-l.fieldVisible));}
        else if(open && !settings){auto n=int(Filtered().size());auto steps=std::clamp(e.value,-10000,10000);scroll=std::clamp(scroll-steps,0,std::max(0,n-l.visible));}return open || editing;}
    if(e.kind==Kind::Down){
        bool consume=open || editing || (!nativeEntry && l.button.Contains(e.x,e.y) && e.code==1) || dragging_;
        if(!consume)return false;
        capturedButtons_|=bit;
        if(sliderField_>=0)return true;
        if(e.code!=1)return true;
        pressed_=true;action_=0;
        if(editing){if(l.button.Contains(e.x,e.y)){dragging_=true;dragX_=e.x-l.button.x;dragY_=e.y-l.button.y;}return true;}
        if(!open){action_=1;return true;}
        if(configuring){
            return ConfigurePress(e,f);
        }else if(settings){
            if(l.back.Contains(e.x,e.y))action_=3;
            else if(!nativeEntry && l.edit.Contains(e.x,e.y))action_=4;
            else if(!nativeEntry && l.reset.Contains(e.x,e.y))action_=5;
            else if(l.key.Contains(e.x,e.y))action_=6;
            else if(!nativeEntry && l.scale.Contains(e.x,e.y))action_=7;
        }else{
            pasteRequested_=false;searchRepeatKey_=0;searchRepeatAt_=0;
            searchFocus=l.search.Contains(e.x,e.y);
            if(l.done.Contains(e.x,e.y))action_=2;
            else if(l.settings.Contains(e.x,e.y))action_=8;
            else if(l.configure.Contains(e.x,e.y)){action_=10;auto filtered=Filtered();if(selected>=0&&selected<int(filtered.size())){auto const&entry=entries[filtered[selected]];if(WorkspaceFor(entry.id)&&(!Configurable(entry)||e.x>=l.configure.x+(l.configure.w+4)/2))action_=30;}}
            else if(l.list.Contains(e.x,e.y)){row_=scroll+int((e.y-l.list.y)/l.catalogRow);action_=9;}
        }return true;
    }
    if(e.kind==Kind::Up){
        bool consume=(capturedButtons_&bit)!=0 || open;capturedButtons_&=~bit;
        if(e.code!=1 || !pressed_)return consume;
        pressed_=false;
        if(sliderField_>=0){MoveSlider(e.x);sliderField_=-1;action_=0;return true;}
        if(dragging_){dragging_=false;return true;}
        if(configuring){ConfigureClick(action_,row_,e,f);action_=0;return consume;}
        switch(action_){
        case 30:{auto filtered=Filtered();if(selected>=0&&selected<int(filtered.size())&&l.configure.Contains(e.x,e.y)){auto const&entry=entries[filtered[selected]];if(!Configurable(entry)||e.x>=l.configure.x+(l.configure.w+4)/2)if(auto s=WorkspaceFor(entry.id)){workspace.Open(*s);workspaceOpen=true;searchFocus=false;}}break;}
        case 1:if(l.button.Contains(e.x,e.y))open=true;break;
        case 2:if(l.done.Contains(e.x,e.y)){open=false;searchFocus=false;}break;
        case 3:if(l.back.Contains(e.x,e.y)){settings=false;keyCapture=false;}break;
        case 4:if(l.edit.Contains(e.x,e.y)){open=false;editing=true;}break;
        case 5:if(l.reset.Contains(e.x,e.y)){config.button={};dirty=true;}break;
        case 6:if(l.key.Contains(e.x,e.y)){keyCapture=true;BeginBindingCapture(-2);}break;
        case 7:if(l.scale.Contains(e.x,e.y)){config.button.scale=config.button.scale>=2?.5f:config.button.scale+.25f;dirty=true;}break;
        case 8:if(l.settings.Contains(e.x,e.y)){settings=true;searchFocus=false;}break;
        case 9:if(l.list.Contains(e.x,e.y) && row_==scroll+int((e.y-l.list.y)/l.catalogRow) && row_<scroll+l.visible && row_<int(Filtered().size()))selected=row_;break;
        case 10:if(l.configure.Contains(e.x,e.y)){
            auto filtered=Filtered();if(selected>=0 && selected<int(filtered.size())){
                auto const& entry=entries[filtered[selected]];
                if(Configurable(entry)){draft={};draft.id=entry.id;configuring=settingsBusy=true;settingsChanged=searchFocus=false;
                    fieldSelected=fieldScroll=settingCategory=0;ClearSettingControl();settingsRequest=Request::Load;++settingsGeneration;}
            }}break;
        default:break;
        }action_=0;return consume;
    }return open;
}
std::vector<int> Menu::Filtered() const {
    std::vector<int> result;auto q=Fold(query);
    for(std::size_t i=0;i<entries.size();++i){auto const& e=entries[i];
        if(q.empty() || Fold(e.name+" "+e.id+" "+e.description).find(q)!=std::string::npos)result.push_back(int(i));
    }return result;
}
}
