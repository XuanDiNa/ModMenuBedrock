#include "ModMenu/WorkspaceUi.h"
#include "ModMenu/Core.h"
#include "ModMenu/View.h"
#include "ModMenu/Locale.h"
#include <algorithm>
namespace menu {
namespace {
using J=WorkspaceJson;
struct Row {std::string id,item;Rect box;};
Rect Back(Frame const&f){return {8,f.height-28,96,20};}
J const* State(WorkspaceUi const&u,std::string const&id){if(!u.snapshot.state)return nullptr;auto const&controls=u.snapshot.state->at("controls");auto it=controls.find(id);return it==controls.end()?nullptr:&*it;}
std::vector<Row> AllRows(WorkspaceUi const&u){std::vector<Row>out;if(!u.snapshot.definition||u.page>=u.snapshot.definition->pages.size())return out;for(auto const&c:u.snapshot.definition->pages[u.page].controls){out.push_back({c.id,{},{}});auto s=State(u,c.id);if(c.type=="list"&&s&&s->contains("rows"))for(auto const&r:s->at("rows"))out.push_back({c.id,r.at("id").get<std::string>(),{}});}return out;}
std::vector<Row> Rows(WorkspaceUi const&u,Frame const&f){auto all=AllRows(u);std::vector<Row>out;float y=60;for(std::size_t i=std::min(std::size_t(std::max(0,u.scroll)),all.size());i<all.size();++i){auto r=all[i];auto c=FindWorkspaceControl(*u.snapshot.definition,r.id);float height=c&&c->type=="image"?std::min(80.f,f.height-116):28.f;if(y+height>f.height-56)break;r.box={8,y,f.width-16,height-2};out.push_back(r);y+=height;}return out;}
std::string Key(Row const&r){return r.id+"/"+r.item;}
std::string Caption(SettingTranslation const&t,Locale locale){return DisplayCaption(t,t.en,locale,true);}
std::string StateCaption(J const&s,char const*key,Locale l){return s.contains(key)?Caption(WorkspaceCaption(s.at(key),1024),l):std::string{};}
bool Enabled(WorkspaceUi const&u,WorkspaceControl const&c,J const*s){auto action=c.type=="list"?c.selectAction:c.id;return s&&s->value("enabled",true)&&!s->value("busy",false)&&std::find(u.snapshot.pending.begin(),u.snapshot.pending.end(),action)==u.snapshot.pending.end();}
}
void WorkspaceUi::Clear(){snapshot={};values=J::object();editing.clear();confirmation.clear();pressed.clear();notice.clear();resultMessage={};lastResult=0;paste=copy=false;close=false;++editGeneration;}
void WorkspaceUi::SuspendInput(){editing.clear();confirmation.clear();pressed.clear();confirmRevision=0;paste=copy=close=false;++editGeneration;}
void WorkspaceUi::Open(WorkspaceSnapshot s){Clear();page=0;scroll=focus=0;Accept(std::move(s));}
void WorkspaceUi::Accept(WorkspaceSnapshot s){if(snapshot.session&&snapshot.session!=s.session)return;
 if(snapshot.state&&s.state&&snapshot.state->at("revision")!=s.state->at("revision"))pressed.clear();
 snapshot=std::move(s);if(!snapshot.definition||!snapshot.state)return;
 for(auto const&p:snapshot.definition->pages)for(auto const&c:p.controls)if((c.type=="text"||c.type=="boolean")&&!values.contains(c.id)){auto state=State(*this,c.id);values[c.id]=state&&state->contains("value")?state->at("value"):(c.type=="boolean"?J(false):J(""));}
 if(snapshot.state->contains("results"))for(auto const&r:snapshot.state->at("results")){auto id=r.at("requestId").get<std::uint64_t>();if(id>lastResult){lastResult=id;resultMessage=r.contains("message")?WorkspaceCaption(r.at("message"),1024):SettingTranslation{r.at("state")=="succeeded"?"Action completed.":"Action failed.",{},{}};notice.clear();}}
}
bool WorkspaceUi::Commit(){if(editing.empty())return true;auto c=FindWorkspaceControl(*snapshot.definition,editing);if(!c||!PlainText(editor.value,c->minBytes,c->maxBytes,c->identifier)){notice="Text does not match the declared format or UTF-8 byte limits.";return false;}values[editing]=editor.value;editing.clear();paste=copy=false;++editGeneration;notice.clear();return true;}
void WorkspaceUi::Activate(std::string const&id,std::string const&item,bool confirmed){if(!snapshot.definition||page>=snapshot.definition->pages.size())return;auto c=FindWorkspaceControl(*snapshot.definition,id);if(!c)return;auto state=State(*this,id);if(!Enabled(*this,*c,state)){notice="Action is unavailable or busy.";return;}
 if(c->type=="text"){if(!Commit())return;editing=id;editor.Begin(values.value(id,std::string{}),c->maxBytes);++editGeneration;return;}
 if(c->type=="boolean"){values[id]=!values.value(id,false);return;}
 if(c->type!="action"&&(c->type!="list"||item.empty()||c->selectAction.empty()))return;
 if(!Commit())return;if(!c->confirmation.en.empty()&&!confirmed){confirmation=id;confirmRevision=snapshot.state->at("revision").get<std::uint64_t>();return;}
 J form=J::object();for(auto const&field:snapshot.definition->pages[page].controls)if(field.type=="text"||field.type=="boolean")form[field.id]=values.at(field.id);
 Workspaces().Submit(snapshot.session,snapshot.definition->pages[page].id,c->type=="list"?c->selectAction:id,snapshot.state->at("revision").get<std::uint64_t>(),form,item,notice);
 if(notice.empty()){resultMessage={};notice="Command submitted; waiting for provider result.";}
}
bool WorkspaceUi::Input(Event const&e,Frame const&f,bool repeat){
 if(inputWidth&&((inputWidth!=f.width)||(inputHeight!=f.height))){pressed.clear();confirmation.clear();}inputWidth=f.width;inputHeight=f.height;
 auto rows=Rows(*this,f);auto key=e.kind==Kind::Key?e.code:0;bool down=e.kind==Kind::Key&&e.value;
 if(e.kind==Kind::Move)return true;
 if(e.kind==Kind::Text){if(!editing.empty()){editor.Insert(e.text);notice=editor.error;}return true;}
 Rect yes{12,f.height-80,f.width/2-18,24},no{f.width/2+6,f.height-80,f.width/2-18,24};
 if(!confirmation.empty()){
  bool accept=down&&!repeat&&key==13;
  if(down&&!repeat&&key==27){confirmation.clear();return true;}
  if(e.kind==Kind::Down&&e.code==1)pressed=yes.Contains(e.x,e.y)?"confirm":no.Contains(e.x,e.y)?"cancel":"";
  if(e.kind==Kind::Up&&e.code==1){accept=pressed=="confirm"&&yes.Contains(e.x,e.y);if(pressed=="cancel"&&no.Contains(e.x,e.y))confirmation.clear();pressed.clear();}
  if(accept){auto id=confirmation;confirmation.clear();if(snapshot.state&&snapshot.state->at("revision")==confirmRevision)Activate(id,{},true);else notice="State changed; activate the action again.";}return true;
 }
 if(!editing.empty()&&down&&!repeat){if(key==27){editing.clear();paste=copy=false;++editGeneration;notice.clear();return true;}if(key==13||key==9){Commit();return true;}if(e.modifiers.control){if(key=='A')editor.SelectAll();if(key=='V')paste=true;if(key=='C')copy=true;return true;}if(e.modifiers.alt)return true;
  if(key==37||key==39)editor.Move(key==37?-1:1,e.modifiers.shift);else if(key==36||key==35)editor.Edge(key==35,e.modifiers.shift);else if(key==8||key==46)editor.Erase(key==8);else editor.Insert(AsciiKey(key,e.modifiers.shift,e.modifiers.caps));notice=editor.error;return true;}
 if(e.kind==Kind::Wheel){scroll=std::clamp(scroll-std::clamp(e.value,-8,8),0,std::max(0,int(AllRows(*this).size())-1));pressed.clear();return true;}
 if(down&&!repeat){if(key==27){close=true;return true;}if(key==9||key==38||key==40){if(!rows.empty())focus=(focus+(key==38||(key==9&&e.modifiers.shift)?int(rows.size())-1:1))%int(rows.size());return true;}if((key==13||key==32)&&!rows.empty()){auto&r=rows[std::clamp(focus,0,int(rows.size())-1)];Activate(r.id,r.item);return true;}}
 if(e.kind==Kind::Down&&e.code==1){pressed.clear();if(Back(f).Contains(e.x,e.y))pressed="back";else if(e.y>=32&&e.y<54){if(e.x<36)pressed="previous";else if(e.x>=f.width-36)pressed="next";}else for(std::size_t i=0;i<rows.size();++i)if(rows[i].box.Contains(e.x,e.y)){pressed=Key(rows[i]);focus=int(i);}return true;}
 if(e.kind==Kind::Up&&e.code==1){auto p=pressed;pressed.clear();if(p=="back"&&Back(f).Contains(e.x,e.y)){close=true;return true;}if(e.y>=32&&e.y<54&&((p=="previous"&&e.x<36)||(p=="next"&&e.x>=f.width-36))){if(!Commit())return true;auto n=snapshot.definition->pages.size();page=(page+(p=="next"?1:n-1))%n;scroll=focus=0;return true;}for(auto const&r:rows)if(p==Key(r)&&r.box.Contains(e.x,e.y)){Activate(r.id,r.item);break;}}
 return true;
}
std::vector<Paint> WorkspaceUi::View(Frame const&f,Locale locale)const{
 std::vector<Paint>out;auto fill=[&](Rect r,Color color){r.x=std::clamp(r.x,0.f,f.width);r.y=std::clamp(r.y,0.f,f.height);r.w=std::clamp(r.w,0.f,f.width-r.x);r.h=std::clamp(r.h,0.f,f.height-r.y);if(r.w>0&&r.h>0)out.push_back({r,color,{}});};
 auto text=[&](Rect r,std::string const&value,Color color=Color{.95f,.95f,.95f,1},float scale=.75f){if(value.empty()||r.w<=0)return;std::string visible;float width=0;for(std::size_t p=0;p<value.size();){auto end=p+1;while(end<value.size()&&(static_cast<unsigned char>(value[end])&192)==128)++end;auto part=value.substr(p,end-p);width+=TextWidth(part)*scale;if(width>r.w)break;visible+=part;p=end;}if(!visible.empty())out.push_back({r,color,visible,scale});};
 fill({0,0,f.width,f.height},{.07f,.07f,.07f,1});auto t=[&](char const*s){return Tr(locale,s);};text({8,9,f.width-16,12},snapshot.definition?snapshot.definition->id+" / "+t("Workspace"):t("Workspace"));if(!snapshot.definition)return out;auto const&pg=snapshot.definition->pages[page];
 fill({8,32,24,20},{.25f,.25f,.25f,1});fill({f.width-32,32,24,20},{.25f,.25f,.25f,1});text({15,38,12,9},"<");text({f.width-25,38,12,9},">");text({40,38,f.width-80,10},Caption(pg.label,locale)+" ("+std::to_string(page+1)+"/"+std::to_string(snapshot.definition->pages.size())+")");
 auto rows=Rows(*this,f);for(std::size_t index=0;index<rows.size();++index){auto const&r=rows[index];auto c=FindWorkspaceControl(*snapshot.definition,r.id);if(!c)continue;auto s=State(*this,r.id);bool enabled=Enabled(*this,*c,s);fill(r.box,index==std::size_t(focus)?Color{.2f,.2f,.2f,1}:Color{.12f,.12f,.12f,1});auto color=enabled?Color{.95f,.95f,.95f,1}:Color{.55f,.55f,.55f,1};auto label=Caption(c->label,locale);std::string value;
 if(!r.item.empty()){if(s&&s->contains("rows"))for(auto const&item:s->at("rows"))if(item.at("id")==r.item){label="  "+Caption(WorkspaceCaption(item.at("label")),locale);value=StateCaption(item,"text",locale);}}
 else if(c->type=="text")value=editing==c->id?editor.Visible(std::max(1,int((r.box.w*.55f)/6))):values.value(c->id,std::string{});
 else if(c->type=="boolean")value=t(values.value(c->id,false)?"On":"Off");else if(c->type=="action")value=s&&s->value("busy",false)?t("Working..."):enabled?">":t("Unavailable");
 else if(c->type=="list"){if(!s||!s->contains("rows")||s->at("rows").empty())value=c->emptyText.en.empty()?t("No results"):Caption(c->emptyText,locale);else value=std::to_string(s->at("rows").size());}
 else if(c->type=="progress"&&s&&s->contains("value")&&s->contains("total")){value=std::to_string(s->at("value").get<std::uint64_t>())+" / "+std::to_string(s->at("total").get<std::uint64_t>());float ratio=float(s->at("value").get<std::uint64_t>())/float(s->at("total").get<std::uint64_t>());fill({r.box.x,r.box.y+r.box.h-3,r.box.w*ratio,2},{.25f,.65f,.35f,1});}else if(s)value=StateCaption(*s,"text",locale);
 if(s&&!enabled){auto reason=StateCaption(*s,"reason",locale);if(!reason.empty())value=reason;}if(s&&s->contains("error"))value=StateCaption(*s,"error",locale);
 text({r.box.x+4,r.box.y+5,r.box.w*.42f-6,10},label,color);text({r.box.x+r.box.w*.42f,r.box.y+5,r.box.w*.58f-5,10},value,color);
 if(c->type=="image"&&s&&s->contains("pixels")){auto w=s->at("width").get<unsigned>(),h=s->at("height").get<unsigned>();float size=std::min((r.box.w-8)/float(w),std::max(1.f,r.box.h-21)/float(h));for(unsigned y=0;y<h;++y)for(unsigned x=0;x<w;){auto pixel=s->at("pixels")[y*w+x].get<std::uint32_t>();auto end=x+1;while(end<w&&s->at("pixels")[y*w+end]==pixel)++end;fill({r.box.x+4+x*size,r.box.y+19+y*size,(end-x)*size,size},{float(pixel>>24)/255,float((pixel>>16)&255)/255,float((pixel>>8)&255)/255,float(pixel&255)/255});x=end;}}
 }
 text({8,f.height-49,f.width-16,10},notice.empty()?(!resultMessage.en.empty()?Tr(locale,Caption(resultMessage,locale)):(snapshot.state?t("Explicit actions only; closing does not stop jobs."):t("Waiting for provider state."))):Tr(locale,notice),{.8f,.8f,.65f,1},.65f);fill(Back(f),{.27f,.27f,.27f,1});text({14,f.height-22,84,10},t("Back"));
 if(!confirmation.empty()){fill({4,55,f.width-8,f.height-90},{.05f,.05f,.05f,1});auto c=FindWorkspaceControl(*snapshot.definition,confirmation);if(c)text({12,64,f.width-24,20},Caption(c->confirmation,locale));fill({12,f.height-80,f.width/2-18,24},{.35f,.2f,.15f,1});fill({f.width/2+6,f.height-80,f.width/2-18,24},{.25f,.25f,.25f,1});text({18,f.height-73,f.width/2-25,10},t("Confirm"));text({f.width/2+12,f.height-73,f.width/2-25,10},t("Cancel"));}
 return out;
}
}
