// A real generic provider. No networking, authentication, scripts or filesystem work.
#include "../sdk/modmenu_workspace_v1.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <string>
namespace {
using J=nlohmann::json;
J Caption(char const*en,char const*tw,char const*cn){return {{"en_US",en},{"zh_TW",tw},{"zh_CN",cn}};}
DWORD WINAPI Worker(void*){
 HMODULE pin{};GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Worker),&pin);
 for(unsigned tries=0;tries<600;++tries){
 auto menu=GetModuleHandleW(L"ModMenuBedrock.dll");if(!menu){Sleep(200);continue;}
 auto status=reinterpret_cast<int(*)()>(GetProcAddress(menu,"MMB_GetStatus"));if(!status||status()!=3){Sleep(200);continue;}
 auto reg=reinterpret_cast<MMB_WorkspaceRegisterV1Fn>(GetProcAddress(menu,"MMB_WorkspaceRegisterV1"));auto publish=reinterpret_cast<MMB_WorkspacePublishV1Fn>(GetProcAddress(menu,"MMB_WorkspacePublishV1"));auto poll=reinterpret_cast<MMB_WorkspacePollV1Fn>(GetProcAddress(menu,"MMB_WorkspacePollV1"));if(!reg||!publish||!poll)return 0;
 J controls=J::array({
 {{"id","name"},{"type","text"},{"label",Caption("Job name","工作名稱","工作名称")},{"minBytes",0},{"maxBytes",128}},
 {{"id","start"},{"type","action"},{"label",Caption("Start","開始","开始")}},
 {{"id","stop"},{"type","action"},{"label",Caption("Stop","停止","停止")}},
 {{"id","reset"},{"type","action"},{"label",Caption("Reset results","清除結果","清除结果")},{"confirmation",Caption("Clear example results?","清除示範結果？","清除示例结果？")}},
 {{"id","refresh"},{"type","action"},{"label",Caption("Refresh","重新整理","刷新")}},
 {{"id","progress"},{"type","progress"},{"label",Caption("Progress","進度","进度")}},
 {{"id","state"},{"type","status"},{"label",Caption("State","狀態","状态")}}
 });
 J pages=J::array({{{"id","jobs"},{"label",Caption("Jobs","工作","工作")},{"controls",controls}},
 {{"id","results"},{"label",Caption("Results / image","結果／圖像","结果／图像")},{"controls",J::array({
 {{"id","results"},{"type","list"},{"label",Caption("Results","結果","结果")},{"selectAction","select"},{"emptyText",Caption("No completed jobs","尚無完成的工作","尚无完成的工作")}},
 {{"id","preview"},{"type","image"},{"label",Caption("Bounded pixel preview","有界像素預覽","有界像素预览")}}
 })}}});
 auto declaration=J({{"schemaVersion",1},{"plugin",{{"id","ModMenuWorkspaceExample"},{"entry","ModMenuWorkspaceExample.dll"},{"versions",{"1.0.0"}}}},{"pages",pages}}).dump();
 std::uint64_t session{};if(reg(reinterpret_cast<void const*>(&Worker),declaration.data(),static_cast<std::uint32_t>(declaration.size()),&session)!=0)return 0;
 bool running=false,changed=true;unsigned progress=0,finished=0;std::uint64_t revision=0;std::string name="Example job",detail="Idle";J results=J::array(),acks=J::array();
 for(;;){
 if(changed){J pixels=J::array();for(unsigned y=0;y<24;++y)for(unsigned x=0;x<32;++x)pixels.push_back(((x/4+y/4+finished)%2)?0x5C9B47FFu:0x386A9FFFu);
 J state={{"protocolVersion",1},{"revision",++revision},{"controls",{
 {"name",{{"value",name},{"enabled",!running}}},{"start",{{"enabled",!running},{"reason","Stop the active job first."}}},
 {"stop",{{"enabled",running},{"reason","No active job."}}},{"reset",{{"enabled",!running}}},{"refresh",{{"enabled",true}}},
 {"progress",{{"value",progress},{"total",20}}},{"state",{{"text",detail}}},{"results",{{"rows",results}}},
 {"preview",{{"width",32},{"height",24},{"pixels",pixels}}}}},{"results",acks}};
 auto raw=state.dump();if(publish(session,raw.data(),static_cast<std::uint32_t>(raw.size()))!=0)return 0;changed=false;}
 char buffer[32768]{};std::uint32_t required{};auto rc=poll(session,buffer,sizeof buffer,&required);if(rc<0)return 0;
 if(rc==0&&required){auto cmd=J::parse(buffer,nullptr,false);if(!cmd.is_object())return 0;auto action=cmd.value("action",std::string{});bool success=true;
 if(action=="start"&&!running){name=cmd.at("values").value("name",std::string{});running=true;progress=0;detail="Running: "+name;}
 else if(action=="stop"&&running){running=false;detail="Stopped";}
 else if(action=="reset"&&!running){results=J::array();detail="Results cleared";}
 else if(action=="refresh")detail="Refreshed by explicit user action";
 else if(action=="select")detail="Selected result: "+cmd.value("rowId",std::string{});
 else success=false;
 acks=J::array({{{"requestId",cmd.at("requestId")},{"state",success?"succeeded":"failed"},{"message",success?"Example action completed":"Example action unavailable"}}});changed=true;
 }
 Sleep(250);if(running){if(++progress>=20){running=false;++finished;detail="Completed: "+name;if(results.size()>=32)results.erase(results.begin());results.push_back({{"id","job"+std::to_string(finished)},{"label",name.empty()?"Untitled job":name},{"text","Completed"}});}changed=true;}
 }
 }return 0;
}
}
BOOL APIENTRY DllMain(HMODULE,DWORD reason,LPVOID){if(reason==DLL_PROCESS_ATTACH){auto h=CreateThread(nullptr,0,Worker,nullptr,0,nullptr);if(h)CloseHandle(h);}return TRUE;}
