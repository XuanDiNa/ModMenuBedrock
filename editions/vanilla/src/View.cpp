#include "ModMenu/View.h"
#include "ModMenu/Configure.h"
#include "ModMenu/Slider.h"
#include <algorithm>
#include <cmath>
namespace menu {
namespace {
std::vector<std::string> Lines(std::string const& s,float width,int max){
    std::vector<std::string> lines;std::string line;float used{};
    auto measure=[](std::string const& value){return TextWidth(value);};
    for(std::size_t p=0;p<s.size() && int(lines.size())<max;){auto c=static_cast<unsigned char>(s[p]);
        if(c=='\r'){++p;continue;}
        if(c=='\n'){lines.push_back(line);line.clear();used=0;++p;continue;}
        std::size_t n=c<128?1:c<224?2:c<240?3:4;n=std::min(n,s.size()-p);float w=n>=3?10.f:6.f;
        if(used+w>width && !line.empty()){
            auto split=line.find_last_of(' ');
            if(max>1 && split!=std::string::npos && split>0){
                lines.push_back(line.substr(0,split));line.erase(0,split+1);used=measure(line);
            }else{lines.push_back(line);line.clear();used=0;}
            if(int(lines.size())>=max)break;
        }
        if(c==' ' && line.empty()){++p;continue;}
        line.append(s,p,n);used+=w;p+=n;
    }if(!line.empty() && int(lines.size())<max)lines.push_back(line);return lines;
}
}
std::vector<Paint> BuildView(Menu const& m,Frame const& f){
    if(m.open&&m.workspaceOpen&&f.eligible&&f.foreground&&!f.observerOnly&&f.width>=240&&f.height>=160)return m.workspace.View(f,m.locale);
    std::vector<Paint> out;if(f.observerOnly || !f.eligible || f.width<240 || f.height<160)return out;
    Color bg{0,0,0,.38f},panel{0,0,0,.62f},button{.32f,.32f,.32f,1},accent{.66f,.69f,.72f,1},white{1,1,1,1},muted{.66f,.66f,.66f,1};
    auto t=[&](std::string_view s){return Tr(m.locale,s);};
    auto setting=[&](SettingField const& field){return DisplaySetting(field,m.locale);};
    auto fill=[&](Rect r,Color c){r.x=std::clamp(r.x,0.f,f.width);r.y=std::clamp(r.y,0.f,f.height);r.w=std::clamp(r.w,0.f,f.width-r.x);r.h=std::clamp(r.h,0.f,f.height-r.y);if(r.w>0 && r.h>0)out.push_back({r,c,{}});};
    auto text=[&](Rect r,std::string const& s,Color c=Color{.95f,.96f,.97f,1},float scale=1.f){
        if(r.w<=0 || r.h<=0)return;auto lines=Lines(s,r.w/scale,1);if(!lines.empty())out.push_back({r,c,lines[0],scale});
    };
    auto border=[&](Rect r,Color color){
        fill({r.x,r.y,r.w,1},color);fill({r.x,r.y+r.h-1,r.w,1},color);
        fill({r.x,r.y,1,r.h},color);fill({r.x+r.w-1,r.y,1,r.h},color);
    };
    auto control=[&](Rect r,std::string const& s,bool enabled=true){
        bool hover=enabled && f.foreground && r.Contains(m.pointerX,m.pointerY);
        fill(r,{.03f,.03f,.03f,1});fill({r.x+1,r.y+1,r.w-2,r.h-2},enabled?(hover?Color{.42f,.42f,.42f,1}:button):Color{.17f,.17f,.17f,1});
        fill({r.x+1,r.y+1,r.w-2,1},enabled?Color{.56f,.56f,.56f,1}:Color{.28f,.28f,.28f,1});
        fill({r.x+1,r.y+r.h-3,r.w-2,2},{.19f,.19f,.19f,1});
        if(hover)border(r,{.9f,.9f,.9f,1});
        float scale=std::min(.9f,std::max(.55f,(r.w-10)/std::max(1.f,TextWidth(s))));
        float offset=std::max(5.f,(r.w-TextWidth(s)*scale)/2);
        text({r.x+offset,r.y+(r.h-9*scale)/2,std::max(0.f,r.w-offset-3),9*scale},s,enabled?white:muted,scale);
    };
    auto l=Arrange(m.config,f);
    if(!m.open || !f.foreground){if(m.nativeEntry)return out;auto s=std::clamp(m.config.button.scale,.5f,2.f);fill(l.button,button);
        text({l.button.x+5*s,l.button.y+(l.button.h-9*s)/2,l.button.w-10*s,9*s},t("Mods"),white,s);
        if(m.editing && f.foreground)text({8,3,f.width-16,10},t("Drag Mods; Escape returns to Settings"),white,.8f);return out;
    }
    fill({0,0,f.width,f.height},bg);
    std::string title=m.configuring?t("Configure")+" / "+m.draft.id:m.settings?"ModMenuBedrock / "+t("Settings"):t("Mods");
    float titleArea=m.configuring || m.settings?f.width-16:l.list.w;
    float titleWidth=std::min(titleArea,TextWidth(title));
    text({8+(titleArea-titleWidth)/2,8,titleWidth,12},title);
    if(m.configuring){
        auto c=ArrangeSettings(m,f);auto categories=SettingCategories(m.draft);
        control(c.previous,"<",!m.settingsBusy);control(c.next,">",!m.settingsBusy);
        text({c.category.x,c.category.y+6,c.category.w,10},DisplayCategory(m.draft,categories[std::clamp(m.settingCategory,0,int(categories.size())-1)],m.locale),white,.8f);
        control(c.resetAll,t("Reset all"),m.draft.valid && !m.settingsBusy);
        fill(l.fields,panel);
        for(auto const& row:c.rows){
            auto const& field=m.draft.fields[row.index];auto r=row.bounds;
            if(row.index==m.fieldSelected){fill(r,{.10f,.10f,.10f,.65f});border(r,{.5f,.5f,.5f,1});}
            text({r.x+5,r.y+4,r.w*.4f-6,10},DisplayCaption(field.labelText,field.label,m.locale,field.external),white,.78f);
            text({r.x+5,r.y+15,r.w*.4f-6,7},DisplayCategory(m.draft,field.category,m.locale)+(field.value!=field.defaultValue?" *":""),muted,.6f);
            if(field.type==SettingType::Slider){
                control(row.minus,"-",!m.settingsBusy);control(row.plus,"+",!m.settingsBusy);
                auto rail=row.rail;fill({rail.x+2,rail.y+rail.h/2-1,std::max(0.f,rail.w-4),2},{.3f,.3f,.3f,1});
                float x=rail.x+3+float(SliderRatio(field))*std::max(0.f,rail.w-6);
                fill({x-2,rail.y+1,4,std::max(0.f,rail.h-2)},m.settingsBusy?muted:white);
                auto box=row.number;fill(box,{.03f,.03f,.03f,1});border(box,m.editField==row.index?white:Color{.4f,.4f,.4f,1});
                auto value=m.editField==row.index?m.numberText+"_":setting(field)+(field.unit.empty()?"":" "+field.unit);
                int count=std::max(1,int((box.w-4)/(6*.65f)));
                if(m.editField==row.index && int(value.size())>count)value=value.substr(value.size()-count);
                text({box.x+2,box.y+std::max(1.f,(box.h-7)/2),std::max(0.f,box.w-4),7},value,white,.65f);
            }else if(field.type==SettingType::Integer){
                control(row.minus,"-",!m.settingsBusy);control(row.plus,"+",!m.settingsBusy);
                Rect box{row.value.x+19,row.value.y,row.value.w-38,row.value.h};
                fill(box,{.03f,.03f,.03f,1});border(box,m.editField==row.index?white:Color{.4f,.4f,.4f,1});
                auto value=m.editField==row.index?m.numberText+"_":setting(field);
                text({box.x+3,box.y+7,box.w-6,9},value,white,.8f);
            }else if(field.type==SettingType::Text){
                fill(row.value,{.03f,.03f,.03f,1});border(row.value,m.editField==row.index?white:Color{.4f,.4f,.4f,1});
                auto value=m.editField==row.index?(field.external&&m.textEditorActive?m.textEditor.Visible(std::max(1,int((row.value.w-6)/6))):m.numberText+"_"):setting(field);
                // Keep the caret visible while editing long mappings; text() also clips.
                int count=std::max(1,int((row.value.w-6)/(6*.8f)));
                if(m.editField==row.index && !field.external && int(value.size())>count)value=value.substr(value.size()-count);
                text({row.value.x+3,row.value.y+7,row.value.w-6,9},value,white,.8f);
            }else if(field.type==SettingType::Boolean){
                fill(row.value,{.06f,.06f,.06f,1});border(row.value,{.4f,.4f,.4f,1});
                bool on=std::get<bool>(field.value);Rect check{row.value.x+5,row.value.y+5,11,11};border(check,muted);
                if(on){fill({check.x+3,check.y+5,2,3},white);fill({check.x+5,check.y+3,3,2},white);}
                text({row.value.x+23,row.value.y+7,row.value.w-26,10},setting(field),on?white:muted,.8f);
            }else{
                auto copy=field;bool canUnbind=(field.type==SettingType::Key || field.type==SettingType::Binding) && SetKey(copy,27);
                auto value=row.index==m.captureField?(field.type==SettingType::Binding?m.BindingCaptureCaption():t(canUnbind?"Press key (Esc unbinds)":"Press key (Esc cancels)")):setting(field);
                if(field.type==SettingType::Choice)value+=" v";
                control(row.value,value,!m.settingsBusy);
            }
            control(row.reset,t("Reset"),!m.settingsBusy && field.value!=field.defaultValue);
        }
        auto visible=VisibleSettings(m);
        if(int(visible.size())>l.fieldVisible){
            float thumb=l.fields.h*float(l.fieldVisible)/float(visible.size());
            float offset=(l.fields.h-thumb)*float(m.fieldScroll)/float(int(visible.size())-l.fieldVisible);
            fill({l.fields.x+l.fields.w-2,l.fields.y+offset,2,thumb},muted);
        }
        if(m.fieldSelected>=0 && m.fieldSelected<int(m.draft.fields.size())){
            auto const& field=m.draft.fields[m.fieldSelected];auto defaults=field;defaults.value=field.defaultValue;
            std::string metadata=t("Default: ")+setting(defaults)+".";
            if(field.type==SettingType::Integer)metadata+=t(" Range: ")+std::to_string(field.minimum)+"-"+std::to_string(field.maximum)+(field.unit.empty()?"":" "+field.unit)+t("; step ")+std::to_string(field.step)+".";
            if(field.type==SettingType::Slider){auto const& s=field.slider;metadata+=t(" Range: ")+FormatFixed(s.minimum,s.precision)+".."+FormatFixed(s.maximum,s.precision)+(field.unit.empty()?"":" "+field.unit)+t("; step ")+FormatFixed(s.step,s.precision)+".";}
            float y=c.help.y;int lines=int(c.help.h/9);
            for(auto const& line:Lines(DisplayCaption(field.descriptionText,field.description,m.locale,field.external),c.help.w/.7f,std::max(0,lines-1))){text({c.help.x,y,c.help.w,9},line,muted,.7f);y+=9;}
            text({c.help.x,y,c.help.w,9},metadata,muted,.7f);
        }
        std::string message=m.settingsBusy?t("Working..."):!m.numberError.empty()?DisplayMessage(m.numberError,m.locale):
            (m.editField>=0?t(m.draft.fields[m.editField].type==SettingType::Text?"Type text; Enter confirms / Escape cancels.":"Type a number; Enter confirms / Escape cancels."):DisplayMessage(m.draft.message,m.locale));
        if(m.draft.message.empty() && !m.settingsBusy && m.numberError.empty() && m.editField<0)message=t("Edit draft only. Apply saves; Cancel discards.");
        float y=c.status.y;for(auto const& line:Lines(message,c.status.w/.65f,2)){text({c.status.x,y,c.status.w,8},line,m.numberError.empty()?muted:Color{1,.5f,.4f,1},.65f);y+=8;}
        text({8,l.apply.y+8,std::max(0.f,l.apply.x-16),9},t(m.settingsChanged || m.editField>=0?"Unsaved changes":"No pending changes"),muted,.65f);
        control(l.apply,t("Apply"),m.draft.valid && (m.settingsChanged || m.editField>=0) && !m.settingsBusy);
        control(l.cancel,t("Cancel"),!m.settingsBusy);
        if(m.choiceField>=0 && m.choiceField<int(m.draft.fields.size())){
            fill({c.popup.x-1,c.popup.y-1,c.popup.w+2,c.popup.h+2},{.02f,.02f,.02f,1});
            auto const& field=m.draft.fields[m.choiceField];
            for(int i=0;i<int(field.options.size());++i){
                Rect r{c.popup.x,c.popup.y+i*c.optionHeight,c.popup.w,c.optionHeight};
                bool hovered=r.Contains(m.pointerX,m.pointerY);
                fill(r,i==m.choiceSelected || hovered?Color{.34f,.34f,.34f,1}:Color{.10f,.10f,.10f,1});
                if(i==m.choiceSelected)border(r,accent);
                auto option=DisplayOption(field,std::size_t(i),m.locale);
                text({r.x+4,r.y+6,r.w-8,10},option,white,std::min(.8f,std::max(.5f,(r.w-8)/std::max(1.f,TextWidth(option)))));
            }
        }return out;
    }
    if(m.settings){
        if(!m.nativeEntry){control(l.edit,t("Move Mods button"));control(l.reset,t("Reset position"));control(l.scale,t("Button scale: ")+std::to_string(m.config.button.scale).substr(0,4));}
        control(l.key,m.keyCapture?m.BindingCaptureCaption():t("Open key: ")+(m.config.openKey?modmenu::binding::Token({m.config.openKey,m.config.openModifiers}):t("Unbound")));control(l.back,t("Back"));
        float x=180,w=std::max(0.f,f.width-x-12);
        text({x,55,w,12},t("Saved to this plugin's config.json"),muted,.8f);
        text({x,75,w,12},t("F10 is the default, not a reserved key."),muted,.8f);
        text({x,95,w,12},t("Reset config.json if a key is unreachable."),muted,.8f);
        if(f.height>=220)text({8,f.height-52,f.width-16,12},t("Only this menu's settings are changed."),muted,.8f);return out;
    }
    fill(l.search,{0,0,0,.95f});border(l.search,m.searchFocus?white:Color{.55f,.55f,.55f,1});
    text({l.search.x+4,l.search.y+6,l.search.w-8,12},m.query.empty()?t("Search mods..."):m.query,m.query.empty()?muted:white,.85f);
    auto filtered=m.Filtered();std::string count="Showing "+std::to_string(filtered.size());
    if(filtered.size()!=m.entries.size())count+=" of "+std::to_string(m.entries.size());count+=" plugins";
    if(m.locale!=Locale::English){count=(m.locale==Locale::TraditionalChinese?"顯示 ":"显示 ")+std::to_string(filtered.size());
        if(filtered.size()!=m.entries.size())count+=" / "+std::to_string(m.entries.size());count+=m.locale==Locale::TraditionalChinese?" 個插件":" 个插件";}
    text(l.count,count,muted,.75f);
    text({l.details.x+4,8,l.details.w-8,10},t("Native client plugins"),muted,.7f);
    fill(l.list,panel);border(l.list,{.3f,.3f,.3f,.85f});
    auto tile=[&](Rect r,Entry const& e){
        unsigned hash=2166136261u;for(unsigned char c:e.id){hash^=c;hash*=16777619u;}
        fill(r,{.16f+float(hash&31)/150,.16f+float((hash>>5)&31)/150,.16f+float((hash>>10)&31)/150,1});
        border(r,{.45f,.45f,.45f,1});
        text({r.x+std::max(2.f,(r.w-6)/2),r.y+(r.h-9)/2,r.w-4,10},e.name.empty()?"?":Lines(e.name,10,1).front(),white,.95f);
    };
    for(int i=0;i<l.visible && m.scroll+i<int(filtered.size());++i){auto index=m.scroll+i;auto const& e=m.entries[filtered[index]];
        Rect row{l.list.x+2,l.list.y+i*l.catalogRow+2,l.list.w-8,l.catalogRow-3};
        if(index==m.selected){fill(row,{0,0,0,.55f});border(row,{.72f,.72f,.72f,1});}
        else if(row.Contains(m.pointerX,m.pointerY))fill(row,{.16f,.16f,.16f,.45f});
        tile({row.x+3,row.y+3,23,23},e);
        text({row.x+30,row.y+2,std::max(0.f,row.w-34),10},e.name,white,.82f);
        text({row.x+30,row.y+12,std::max(0.f,row.w-34),8},e.description.empty()?t("No description provided."):e.description,muted,.68f);
        text({row.x+30,row.y+21,std::max(0.f,row.w-34),7},t(e.status)+(Configurable(e)?" / "+t("Configurable"):""),muted,.56f);
    }
    if(int(filtered.size())>l.visible){
        float track=l.list.h-4,thumb=std::max(8.f,track*float(l.visible)/float(filtered.size()));
        float offset=(track-thumb)*float(m.scroll)/float(int(filtered.size())-l.visible);
        fill({l.list.x+l.list.w-5,l.list.y+2,4,track},{0,0,0,.7f});
        fill({l.list.x+l.list.w-5,l.list.y+2+offset,4,thumb},{.60f,.60f,.60f,1});
    }
    if(filtered.empty())text({l.list.x+5,l.list.y+8,l.list.w-10,12},t("No matching plugins"),muted,.8f);
    else{
        auto const& e=m.entries[filtered[std::clamp(m.selected,0,int(filtered.size())-1)]];
        tile({l.details.x+4,l.details.y+1,32,32},e);
        float hx=l.details.x+42,hw=std::max(0.f,l.details.w-46);
        text({hx,l.details.y+1,hw,11},e.name,white,.9f);
        text({hx,l.details.y+13,hw,9},e.version.empty()?t("Version unknown"):e.version,muted,.75f);
        text({hx,l.details.y+23,hw,9},e.author.empty()?t("Author not provided"):(m.locale==Locale::English?"By ":m.locale==Locale::TraditionalChinese?"作者：":"作者：")+e.author,muted,.7f);
        text({l.details.x+4,l.details.y+35,l.details.w-8,8},t(e.status),muted,.65f);
        auto configure=l.configure;
        if(m.WorkspaceFor(e.id)){Rect work=configure;if(Configurable(e)){configure.w=(configure.w-4)/2;work.x=configure.x+configure.w+4;work.w=configure.w;}control(work,t("Workspace"));}
        if(Configurable(e)||!m.WorkspaceFor(e.id))control(configure,t(Configurable(e)?"Configure":"No settings available"),Configurable(e));
        fill(l.description,panel);border(l.description,{.25f,.25f,.25f,.9f});
        float x=l.description.x+6,y=l.description.y+6,w=std::max(0.f,l.description.w-12);
        auto line=[&](std::string const& s,Color color,float scale=.8f){if(y+10<=l.description.y+l.description.h-4){text({x,y,w,10},s,color,scale);y+=11;}};
        if(!Configurable(e) && !e.settingsReason.empty())for(auto const& s:Lines(DisplayMessage(e.settingsReason,m.locale),w/.7f,2))line(s,muted,.7f);
        for(auto const& s:Lines(e.description.empty()?t("No description provided."):e.description,w/.8f,8))line(s,white);
        if(!e.website.empty()){y+=7;line(t("Website"),muted,.7f);for(auto const& s:Lines(e.website,w/.7f,3))line(s,{.55f,.65f,1,1},.7f);}
    }
    control(l.settings,t("Settings"));control(l.done,t("Done"));
    return out;
}
Chain ClassifyEntry(bool canonical,bool knownOwner,int status){
    if(canonical)return Chain::Canonical;
    if(!knownOwner)return Chain::Refuse;
    if(status==0)return Chain::Wait;
    return status==3?Chain::KnownReady:Chain::Refuse;
}
}
