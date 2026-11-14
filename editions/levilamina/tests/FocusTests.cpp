#include "ModMenu/View.h"
#include "ModMenu/Configure.h"
#include <iostream>

namespace {
using namespace menu;
int checks{}, failures{};
void Check(bool ok, char const* name) {
    ++checks;
    if (!ok) { ++failures; std::cerr << "FAIL " << name << '\n'; }
}
Frame const mainFrame{854,469,true,true,false,MenuScene::Main};
Frame const pauseFrame{854,469,true,true,false,MenuScene::Pause};
Frame Background(Frame f) { f.foreground=false; return f; }
struct WorkspaceCase {
    Menu m; std::uint64_t session{};
    explicit WorkspaceCase(Frame f=mainFrame) {
        auto d=ParseWorkspace(R"({"schemaVersion":1,"plugin":{"id":"FocusCase","entry":"FocusCase.dll","versions":["1"]},"pages":[{"id":"jobs","label":"Jobs","controls":[{"id":"status","type":"status","label":"State"},{"id":"start","type":"action","label":"Start"},{"id":"name","type":"text","label":"Name","minBytes":0,"maxBytes":64},{"id":"erase","type":"action","label":"Erase","confirmation":"Really erase?"}]},{"id":"results","label":"Results","controls":[{"id":"list","type":"list","label":"Results","selectAction":"choose"}]}]})","FocusCase","FocusCase.dll","1");
        session=Workspaces().Register(std::move(d));
        Publish(1);
        m.nativeEntry=true;m.OpenNative(f);
        for(auto const&s:m.workspaces)if(s.session==session)m.workspace.Open(s);
        m.workspaceOpen=true;
    }
    void Publish(unsigned revision) {
        WorkspaceJson rows=WorkspaceJson::array();
        for(int i=0;i<16;++i)rows.push_back({{"id","row"+std::to_string(i)},{"label","Result"}});
        Workspaces().Publish(session,WorkspaceJson{{"protocolVersion",1},{"revision",revision},{"controls",{{"status",{{"text","working"}}},{"start",{{"enabled",true}}},{"name",{{"value","draft"}}},{"erase",{{"enabled",true}}},{"list",{{"rows",rows}}}}}}.dump());
        m.workspaces=Workspaces().Snapshots();
        if(m.workspaceOpen)for(auto const&s:m.workspaces)if(s.session==session)m.workspace.Accept(s);
    }
    ~WorkspaceCase(){Workspaces().Unregister(session);}
};
bool Empty(std::uint64_t s){std::uint32_t n{};return Workspaces().Poll(s,nullptr,0,n)&&n==0;}
void Click(Menu&m,float x,float y,Frame f=mainFrame){m.Input({Kind::Move,x,y},f);m.Input({Kind::Down,m.pointerX,m.pointerY,1},f);m.Input({Kind::Up,m.pointerX,m.pointerY,1},f);}
}
int main(){
    for(auto f:{mainFrame,pauseFrame}){
        WorkspaceCase c(f);auto&m=c.m;m.workspace.page=1;m.workspace.scroll=4;m.workspace.focus=2;
        m.Tick(Background(f));
        Check(m.open&&m.workspaceOpen&&m.workspace.page==1&&m.workspace.scroll==4,"focus loss retains workspace page and scroll");
        Check(BuildView(m,Background(f)).empty(),"background workspace has no paint authority");
        Check(!m.Input({Kind::Wheel,0,0,0,-1},Background(f))&&m.workspace.scroll==4,"background wheel cannot modify retained page");
        m.Tick({0,0,false,false,true}); // Actual background sibling shape.
        m.Tick({0,0,false,true,true});  // Foreground sibling must not resume it.
        m.Tick(f);
        Check(m.open&&m.workspaceOpen&&m.workspace.page==1&&m.workspace.scroll==4,"same allowed top scene restores after background siblings");
        Check(!BuildView(m,f).empty()&&Empty(c.session),"restored page paints without submitting an action");
    }
    {
        WorkspaceCase c;auto&m=c.m;
        m.Input({Kind::Move,20,93},mainFrame);m.Input({Kind::Down,20,93,1},mainFrame);
        m.Tick(Background(mainFrame));m.Tick(mainFrame);
        m.Input({Kind::Down,20,93,1},mainFrame); // Repeat of a held button.
        m.Input({Kind::Up,20,93,1},mainFrame);
        Check(Empty(c.session),"held mouse press across focus loss cannot submit on return");
        Click(m,20,93);Check(!Empty(c.session),"fresh paired mouse click works after release");
    }
    {
        WorkspaceCase c;auto&m=c.m;
        m.Input({Kind::Down,20,93,1},mainFrame);
        m.Tick({0,0,false,false,true});m.Tick(mainFrame);
        m.Tick(Background(mainFrame));m.Tick(mainFrame);
        m.Input({Kind::Down,20,93,1},mainFrame);m.Input({Kind::Up,20,93,1},mainFrame);
        Check(m.workspaceOpen&&Empty(c.session),"background sibling first and repeated focus cycles retain mouse release barrier");
        Click(m,20,93);Check(!Empty(c.session),"fresh click recovers after repeated focus cycles");
    }
    {
        WorkspaceCase c;auto&m=c.m;
        Click(m,20,149);Check(!m.workspace.confirmation.empty(),"explicit dangerous action opens confirmation");
        m.Tick(Background(mainFrame));m.Tick(mainFrame);
        Check(m.workspaceOpen&&m.workspace.confirmation.empty()&&Empty(c.session),"focus loss cancels confirmation without leaving workspace");
    }
    {
        WorkspaceCase c;auto&m=c.m;Click(m,20,121);
        m.Input({Kind::Key,0,0,'V',1,{}, {false,true,false,false}},mainFrame);
        auto old=m.workspace.editGeneration;
        m.Tick(Background(mainFrame));m.CompleteClipboard((std::uint64_t(1)<<63)|old,"late");m.Tick(mainFrame);
        Check(m.workspaceOpen&&m.workspace.editing.empty()&&!m.workspace.paste&&!m.workspace.copy&&m.workspace.editGeneration>old&&m.workspace.values["name"]=="draft","focus loss cancels text/paste generation but retains committed draft");
    }
    {
        WorkspaceCase c;auto&m=c.m;std::array<bool,256>keys{};m.PollKeyboard(keys,mainFrame);
        m.Tick(Background(mainFrame));keys[13]=keys[121]=true;
        m.Input({Kind::Key,0,0,13,1},mainFrame); // Raw before first physical sample.
        m.PollKeyboard(keys,mainFrame);m.PollKeyboard(keys,mainFrame);
        Check(m.workspaceOpen&&m.open&&Empty(c.session),"held Enter or F10 cannot act before or after refocus sampling");
        keys.fill(false);m.PollKeyboard(keys,mainFrame);m.workspace.focus=1;keys[13]=true;m.PollKeyboard(keys,mainFrame);
        Check(!Empty(c.session),"fresh keyboard activation works after neutral refocus sample");
    }
    {
        WorkspaceCase c;auto&m=c.m;Click(m,20,93);
        m.Tick(Background(mainFrame));
        char command[32768]{};std::uint32_t n{};Workspaces().Poll(c.session,command,sizeof command,n);
        Check(n>0&&WorkspaceJson::parse(command)["action"]=="start","background focus loss retains previously queued work");
        c.Publish(2);m.Tick(mainFrame);
        Check(m.workspaceOpen&&m.workspace.snapshot.state->at("revision")==2&&Empty(c.session),"background provider state updates survive without command replay or Stop");
    }
    for(auto invalid:{Frame{854,469,false,false,false,MenuScene::Unknown},pauseFrame,Frame{239,469,true,false,false,MenuScene::Main},Frame{854,469,true,true,false,MenuScene::Unknown}}){
        WorkspaceCase c;auto&m=c.m;m.Tick(Background(mainFrame));m.Tick(invalid);m.Tick(mainFrame);
        Check(!m.open&&!m.workspaceOpen,"scene transition ineligible world or invalid viewport prevents stale restoration");
    }
    {
        WorkspaceCase c;auto&m=c.m;m.Tick(Background(mainFrame));
        Workspaces().Unregister(c.session);m.workspaces=Workspaces().Snapshots();m.Tick(mainFrame);
        Check(!m.workspaceOpen&&m.open,"removed provider cannot regain its old workspace on refocus");
    }
    {
        WorkspaceCase c;auto&m=c.m;auto def=*m.workspace.snapshot.definition;m.Tick(Background(mainFrame));
        Workspaces().Unregister(c.session);c.session=Workspaces().Register(def);c.Publish(1);m.Tick(mainFrame);
        Check(!m.workspaceOpen&&m.open,"replacement provider with same ID cannot inherit old session page");
    }
    {
        Menu m;m.nativeEntry=true;m.OpenNative(pauseFrame);m.settings=true;m.keyCapture=true;
        m.Tick(Background(pauseFrame));m.Tick(pauseFrame);
        Check(m.open&&m.settings&&!m.keyCapture&&m.config.openKey==121,"Mods settings survive but key capture is cancelled without rebinding");
    }
    {
        Menu m;m.nativeEntry=true;m.OpenNative(pauseFrame);m.configuring=true;m.draft.valid=true;
        m.draft.fields.push_back({"/key","Key",SettingType::Key,std::string("F5"),0,0,{"F5","F6","NONE"}});m.captureField=0;m.settingsChanged=true;
        m.Tick(Background(pauseFrame));m.Tick(pauseFrame);
        Check(m.open&&m.configuring&&m.captureField==-1&&m.draft.fields.size()==1&&m.settingsChanged&&m.settingsRequest==Menu::Request::None,"Configure retains unsaved draft without capture or automatic Apply");
    }
    std::cout<<"focus checks="<<checks<<"; failures="<<failures<<'\n';return failures?1:0;
}
