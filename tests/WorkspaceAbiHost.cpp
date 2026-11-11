#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "../integration/sdk/modmenu_workspace_v1.h"
int wmain(int argc,wchar_t**argv){if(argc!=3)return 2;namespace fs=std::filesystem;
 auto root=fs::temp_directory_path()/(L"ModMenuWorkspaceAbi-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));fs::create_directories(root/L"ModMenuBedrock");fs::create_directory(root/L"ModMenuWorkspaceExample");
 auto menu=root/L"ModMenuBedrock"/L"ModMenuBedrock.dll",provider=root/L"ModMenuWorkspaceExample"/L"ModMenuWorkspaceExample.dll";fs::copy_file(argv[1],menu);fs::copy_file(argv[2],provider);std::ofstream(root/L"ModMenuWorkspaceExample"/L"manifest.json")<<R"({"entry":"ModMenuWorkspaceExample.dll","version":"1.0.0"})";
 auto lib=LoadLibraryW(menu.c_str());if(!lib)return 3;auto state=reinterpret_cast<int(*)()>(GetProcAddress(lib,"MMB_TestProviderState"));auto start=reinterpret_cast<int(*)()>(GetProcAddress(lib,"MMB_TestStart"));
 auto reg=reinterpret_cast<MMB_WorkspaceRegisterV1Fn>(GetProcAddress(lib,"MMB_WorkspaceRegisterV1"));auto publish=reinterpret_cast<MMB_WorkspacePublishV1Fn>(GetProcAddress(lib,"MMB_WorkspacePublishV1"));auto poll=reinterpret_cast<MMB_WorkspacePollV1Fn>(GetProcAddress(lib,"MMB_WorkspacePollV1"));auto unreg=reinterpret_cast<MMB_WorkspaceUnregisterV1Fn>(GetProcAddress(lib,"MMB_WorkspaceUnregisterV1"));if(!reg||!publish||!poll||!unreg)return 4;
 std::uint64_t token=17;std::uint32_t required=17;
 if(reg(nullptr,"{}",2,&token)!=-1||token!=0||reg(reinterpret_cast<void const*>(&wmain),"{}",2,&token)!=-3||token!=0||reg(nullptr,"{}",2,nullptr)!=-1||publish(999,"{}",2)!=-3||publish(999,nullptr,2)!=-1||poll(999,nullptr,0,&required)!=-3||required!=0||poll(999,nullptr,1,&required)!=-1||unreg(999)!=-3){std::cerr<<"FAIL public ABI argument/owner/stale-session checks\n";return 1;}
 if(!state||!start||!LoadLibraryW(provider.c_str()))return 4;
 for(unsigned i=0;i<80&&state()==0;++i)Sleep(50);if(state()!=1){std::cerr<<"FAIL real provider registration/publication\n";return 1;}
 Sleep(300);if(state()!=1){std::cerr<<"FAIL publication initiated a job\n";return 1;}if(start()!=1)return 1;
 for(unsigned i=0;i<30&&state()!=2;++i)Sleep(50);if(state()!=2){std::cerr<<"FAIL C mailbox worker command round trip\n";return 1;}
 for(unsigned i=0;i<130&&state()!=3;++i)Sleep(50);if(state()!=3){std::cerr<<"FAIL provider job did not finish\n";return 1;}
 std::cout<<"PASS real C ABI registration, copy snapshots, explicit command, worker progress/completion; no game/network\n";
 // Both modules are intentionally pinned. Temp fixture files remain until process
 // exit; no attempt to unload live providers or recursively erase locked modules.
 std::wcout<<L"Offline fixture: "<<root.c_str()<<L'\n';return 0;
}
