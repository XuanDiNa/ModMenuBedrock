#include "ModMenu/NativeHost.h"
#include "ModMenu/Storage.h"
#include "ModMenu/Clipboard.h"
#include "ModMenu/LaminaLifecycle.h"
#include "ModMenu/LaminaRuntime.h"
#include "ll/api/Versions.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <algorithm>
namespace {
HINSTANCE module{};menu::SharedState state;menu::NativeProfile profile;
std::atomic<bool> stopRequested{};HANDLE stopEvent{},workerThread{};menu::LaminaLifecycle lifecycle;
std::filesystem::path Directory(){wchar_t path[32768]{};auto n=GetModuleFileNameW(module,path,32768);return n && n<32768?std::filesystem::path(path).parent_path():std::filesystem::path{};}
bool Loaded(std::filesystem::path const& path){
    auto dll=GetModuleHandleW(path.c_str());if(!dll)return false;wchar_t actual[32768]{};
    auto n=GetModuleFileNameW(dll,actual,32768);if(!n || n>=32768)return false;
    std::error_code ec;return std::filesystem::equivalent(path,std::filesystem::path(actual),ec) && !ec;
}
DWORD WINAPI Worker(void*){
    HMODULE pin{};if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&Worker),&pin)){state.status.store(4);return 0;}
    try{
        auto dir=Directory();if(dir.empty()){state.status.store(4);return 0;}
        std::ofstream log(dir/L"ModMenuBedrockLL.log",std::ios::app);
        auto write=[&](std::string const& s){log<<GetTickCount64()<<' '<<s<<'\n';log.flush();};
        write("ModMenuBedrockLL 0.1.0-test2-ll26.60; pid="+std::to_string(GetCurrentProcessId())+"; exact 26.60 profile / resource-pack entry API v1 / bounded definition authority / no payload logging; x64; official LeviLamina 26.60.7 native lifecycle");
        auto configPath=dir/L"config.json";menu::Config config;
        bool existing=std::filesystem::exists(configPath);
        if(!menu::LoadConfig(configPath,config)){
            if(existing)write("Invalid config retained; using safe defaults for this session");
            else if(!menu::SaveConfig(configPath,config))write("Cannot create config; in-memory defaults only");
        }
        {std::lock_guard lock(state.mutex);state.menu.config=config;}
        auto mods=dir.parent_path();auto list=menu::Discover(mods,Loaded);
        bool expect=false;for(auto const& entry:list)if(entry.id=="MouseTweaksBedrock" && entry.status!="Disabled manifest" && entry.status!="Entry missing/unsafe")expect=true;
        std::string previous;menu::InstallResult result;
        auto waitStart=GetTickCount64();
        do{
            if(stopRequested.load())return 0;
            std::string reason;result=menu::InstallNative(profile,state,expect,reason);
            if(reason!=previous){write(reason);previous=reason;}
            if(result==menu::InstallResult::Wait){
                if(GetTickCount64()-waitStart>120000){menu::DeactivateNative();state.status.store(2);write("INACTIVE: dependency Ready timeout; definition callbacks pass through");return 0;}WaitForSingleObject(stopEvent,100);
            }
        }while(result==menu::InstallResult::Wait);
        if(result!=menu::InstallResult::Ready){menu::DeactivateNative();state.status.store(2);return 0;}
        state.status.store(3,std::memory_order_release);
        auto lastCatalog=GetTickCount64()-2000,lastLog=GetTickCount64();unsigned long long priorFaults{};unsigned loggedReceipts{};
        std::array<bool,8> loggedDefinitionProbes{};
        while(!stopRequested.load(std::memory_order_acquire)){
            auto now=GetTickCount64();
            if(GetModuleHandleW(L"ModMenuBedrock.dll")){menu::DeactivateNative();state.status.store(2);write("INACTIVE: vanilla ModMenu loaded concurrently; disable one edition and restart");return 0;}
            for(unsigned i=0;i<loggedDefinitionProbes.size();++i){
                if(!loggedDefinitionProbes[i] && state.definitionProbeReady[i].load(std::memory_order_acquire)){
                    write(menu::DescribeDefinitionProbe(state.definitionProbes[i]));loggedDefinitionProbes[i]=true;
                }
            }
            auto snapshots=menu::Workspaces().Snapshots();
            {std::lock_guard lock(state.mutex);auto&m=state.menu;m.workspaces=std::move(snapshots);if(m.workspaceOpen){auto s=m.WorkspaceFor(m.workspace.snapshot.definition->id);if(s&&s->session==m.workspace.snapshot.session)m.workspace.Accept(*s);else{m.workspaceOpen=false;m.workspace.Clear();}}}
            bool paste=false,copy=false;std::uint64_t editGeneration{};std::string copied;
            {std::lock_guard lock(state.mutex);auto&m=state.menu;
             if(m.workspaceOpen){paste=m.workspace.paste;copy=m.workspace.copy;editGeneration=m.workspace.editGeneration|(std::uint64_t(1)<<63);copied=m.workspace.editor.Selection();m.workspace.paste=m.workspace.copy=false;}
             else{paste=m.clipboardPaste;copy=m.clipboardCopy;editGeneration=m.clipboardGeneration;copied=m.clipboardText;m.clipboardPaste=m.clipboardCopy=false;}}
            if(copy)menu::WriteEditorClipboard(copied);
            if(paste){auto text=menu::ReadEditorClipboard();std::lock_guard lock(state.mutex);state.menu.CompleteClipboard(editGeneration,text);}
            if(now-lastCatalog>=2000){auto fresh=menu::LoadedCatalog(mods,Loaded);
                for(auto& e:fresh){auto support=menu::InspectSettings(mods,e.id);e.settingsAvailable=support.available;e.settingsReason=std::move(support.reason);}
                for(auto& e:fresh)if(e.id==dir.filename().string())e.status=state.status.load()==3?"Ready":"Inactive";
                {std::lock_guard lock(state.mutex);state.menu.entries=std::move(fresh);}lastCatalog=now;
            }
            bool save=false,menuOpen=false;menu::Config changed;
            menu::Menu::Request request=menu::Menu::Request::None;menu::SettingsDraft settings;unsigned long long generation{};
            {std::lock_guard lock(state.mutex);auto& m=state.menu;request=m.settingsRequest;
                if(request!=menu::Menu::Request::None){settings=m.draft;generation=m.settingsGeneration;m.settingsRequest=menu::Menu::Request::None;}}
            if(request!=menu::Menu::Request::None){
                auto response=request==menu::Menu::Request::Load?menu::LoadSettings(mods,settings.id):menu::ApplySettings(mods,settings);
                write(std::string(request==menu::Menu::Request::Load?"Configure load: ":"Configure Apply: ")+settings.id+"; "+response.message);
                std::lock_guard lock(state.mutex);state.menu.CompleteSettings(generation,std::move(response));
            }
            {std::lock_guard lock(state.mutex);menuOpen=state.menu.open;if(state.menu.dirty && !state.menu.Captured()){changed=state.menu.config;state.menu.dirty=false;save=true;}}
            std::array<menu::RenderReceipt,12> receipts{};unsigned receiptCount{};
            {std::lock_guard lock(state.mutex);receiptCount=state.renderReceiptCount;std::copy_n(state.renderReceipts.begin(),receiptCount,receipts.begin());}
            while(loggedReceipts<receiptCount){auto const& r=receipts[loggedReceipts++];
                write("render-sample scene="+std::string(r.scene.data())+" top="+std::to_string(r.top)+" foreground="+std::to_string(r.foreground)+
                    " renderer-table-rva="+std::to_string(r.rendererTable)+" begin-rva="+std::to_string(r.beginMethod)+" end-rva="+std::to_string(r.endMethod)+
                    " native-viewport="+std::to_string(r.nativeWidth)+"x"+std::to_string(r.nativeHeight)+" context-alpha="+std::to_string(r.contextAlpha)+
                    " shader-alpha="+std::to_string(r.shaderAlpha)+" stencil="+std::to_string(r.stencil)+" depth="+std::to_string(r.depth)+" clip="+std::to_string(r.clip));
            }
            if(save){if(menu::SaveConfig(configPath,changed))write("Own configuration saved");else write("Own configuration save failed; previous file retained");}
            auto faults=state.faults.load();
            if(now-lastLog>=5000 || faults!=priorFaults){
                write("status="+std::to_string(state.status.load())+" contexts="+std::to_string(state.contexts.load())+" draws="+std::to_string(state.draws.load())+" inputs="+std::to_string(state.inputs.load())+" eligible="+std::to_string(state.eligible.load())+" other-scenes="+std::to_string(state.unknownScenes.load())+" gate="+std::to_string(state.lastGate.load())+" viewport="+std::to_string(state.width.load())+"x"+std::to_string(state.height.load())+" faults="+std::to_string(faults)+" fg-render="+std::to_string(state.foregroundRenders.load())+" bg-render="+std::to_string(state.backgroundRenders.load())+" siblings="+std::to_string(state.siblingCallbacks.load())+" raw="+std::to_string(state.rawEvents.load())+" keyboard="+std::to_string(state.keyEvents.load())+" paints="+std::to_string(state.paintCommands.load())+" open="+std::to_string(menuOpen)+
                    " pointer="+std::to_string(state.pointerEvents.load())+" primary-pointer="+std::to_string(state.primaryPointerEvents.load())+
                    " mapped-down="+std::to_string(state.downEvents.load())+" mapped-up="+std::to_string(state.upEvents.load())+" mapped-id="+std::to_string(state.lastMappedId.load())+
                    " text="+std::to_string(state.textEvents.load())+" wheel="+std::to_string(state.wheelEvents.load())+" physical-openkey-frames="+std::to_string(state.physicalKeyFrames.load())+
                    " sampled-shortcut-presses="+std::to_string(state.sampledShortcutPresses.load())+
                    " raw-key="+std::to_string(state.lastRawKey.load())+" raw-type="+std::to_string(state.lastRawType.load())+" raw-state="+std::to_string(state.lastRawState.load()));lastLog=now;priorFaults=faults;
                write("native-entry candidates="+std::to_string(state.definitionCandidates.load())+" layers="+std::to_string(state.definitionLayers.load())+" scopes="+std::to_string(state.definitionScopes.load())+" patched="+std::to_string(state.definitionsPatched.load())+" refused="+std::to_string(state.definitionsRefused.load())+" opens="+std::to_string(state.nativeOpens.load())+" controller-events="+std::to_string(state.scopedControllerEvents.load())+" mods-events="+std::to_string(state.modsEvents.load())+" controller-table-rva="+std::to_string(state.lastControllerTable.load()));
            }WaitForSingleObject(stopEvent,250);
        }
    }catch(...){menu::DeactivateNative();state.status.store(4);OutputDebugStringA("ModMenuBedrock worker stopped after an exception\n");}
    return 0;
}
}
extern "C" __declspec(dllexport) int MMB_GetStatus(){return menu::LaminaVisibleStatus(state.status.load(std::memory_order_acquire),stopRequested.load(std::memory_order_acquire));}
extern "C" __declspec(dllexport) char const* MMB_GetVersion(){return "0.1.0-test2-ll26.60";}
namespace {
struct LaminaOperations {
 bool Admit(){
    if(GetModuleHandleW(L"ModMenuBedrock.dll")){state.status.store(1);OutputDebugStringA("ModMenuBedrockLL refused: disable vanilla edition first\n");return false;}
    auto v=ll::getLoaderVersion();
    if(!ll::isClient()||v.major!=26||v.minor!=60||v.patch!=7){state.status.store(1);return false;}
    HMODULE owner{};
    if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(&MMB_GetVersion),&owner)){state.status.store(4);return false;}
    module=owner;return true;
 }
 bool Prepare(){
    auto dir=Directory();if(dir.empty()){state.status.store(4);return false;}
    std::ofstream log(dir/L"ModMenuBedrockLL.log",std::ios::app);
    auto write=[&](std::string const& s){log<<GetTickCount64()<<' '<<s<<'\n';log.flush();};
    write("ModMenuBedrockLL 0.1.0-test2-ll26.60; official native load; pid="+std::to_string(GetCurrentProcessId()));
        menu::ProfileDiagnostic diagnostic;
        if(!profile.Initialize(GetModuleHandleW(nullptr),&diagnostic)){state.status.store(1);write("INACTIVE: "+menu::DescribeProfileDiagnostic(diagnostic));return false;}
        write(std::string("Verified native profile: ")+profile.Version());
        {std::lock_guard lock(state.mutex);state.menu.nativeEntry=true;}
        std::string definitionReason;
        if(!menu::InstallDefinitions(profile,state,definitionReason)){state.status.store(2);write("INACTIVE: "+definitionReason);return false;}
        write(definitionReason);

    state.status.store(2,std::memory_order_release);return true;
 }
 bool Start(){
    if(GetModuleHandleW(L"ModMenuBedrock.dll")){state.status.store(1);return false;}
    stopEvent=CreateEventW(nullptr,TRUE,FALSE,nullptr);
    if(!stopEvent){state.status.store(4);return false;}
    workerThread=CreateThread(nullptr,0,Worker,nullptr,0,nullptr);
    if(!workerThread){CloseHandle(stopEvent);stopEvent=nullptr;state.status.store(4);return false;}
    return true;
 }
 void Stop(){
    stopRequested.store(true,std::memory_order_release);menu::DeactivateNative();
    {std::lock_guard lock(state.mutex);state.menu={};}
    if(stopEvent)SetEvent(stopEvent);
    if(workerThread){
        if(WaitForSingleObject(workerThread,5000)==WAIT_OBJECT_0){CloseHandle(workerThread);workerThread=nullptr;CloseHandle(stopEvent);stopEvent=nullptr;}
    }
    if(state.status.load()!=1&&state.status.load()!=4)state.status.store(2,std::memory_order_release);
 }
} operations;
}
namespace menu {
bool LoadLaminaRuntime(){try{return lifecycle.Load(operations);}catch(...){operations.Stop();state.status.store(4);return false;}}
bool EnableLaminaRuntime(){try{return lifecycle.Enable(operations);}catch(...){operations.Stop();state.status.store(4);return false;}}
bool DisableLaminaRuntime(){try{return lifecycle.Disable(operations);}catch(...){state.status.store(4);return false;}}
}
