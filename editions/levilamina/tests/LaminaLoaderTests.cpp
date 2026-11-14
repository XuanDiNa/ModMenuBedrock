#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <string>
#include <filesystem>
int wmain(int argc,wchar_t**argv){
 if(argc<4)return 2;
 if(std::wstring(argv[3])==L"missing"){
    auto dll=LoadLibraryW(argv[2]);auto error=GetLastError();bool ok=!dll&&error==ERROR_MOD_NOT_FOUND;
    std::cout<<"missing LL dependency "<<(ok?"PASS":"FAIL")<<" error="<<error<<'\n';return ok?0:1;
 }
 auto fixture=LoadLibraryW(argv[1]);if(!fixture){std::cerr<<"fixture load "<<GetLastError()<<'\n';return 1;}
 auto environment=reinterpret_cast<void(*)(unsigned)>(GetProcAddress(fixture,"FixtureEnvironment"));
 auto count=reinterpret_cast<unsigned(*)(unsigned)>(GetProcAddress(fixture,"FixtureCount"));
 auto mod=reinterpret_cast<void*(*)()>(GetProcAddress(fixture,"FixtureMod"));
 if(!environment||!count||!mod)return 1;
 unsigned mode=unsigned(std::stoi(argv[3]));environment(mode);
 if(mode==3){
    if(argc!=5)return 2;
    auto dir=std::filesystem::temp_directory_path()/(L"MMB-duplicate-fixture-"+std::to_wstring(GetCurrentProcessId()));
    std::filesystem::create_directories(dir);auto alias=dir/L"ModMenuBedrock.dll";
    if(!CopyFileW(argv[4],alias.c_str(),TRUE)||!LoadLibraryW(alias.c_str()))return 1;
 }
 auto dll=LoadLibraryW(argv[2]);if(!dll){std::cerr<<"plugin load "<<GetLastError()<<'\n';return 1;}
 auto status=reinterpret_cast<int(*)()>(GetProcAddress(dll,"MMB_GetStatus"));
 auto version=reinterpret_cast<char const*(*)()>(GetProcAddress(dll,"MMB_GetVersion"));
 auto load=reinterpret_cast<bool(*)(void*)>(GetProcAddress(dll,"ll_mod_load"));
 bool ok=status&&version&&load&&GetProcAddress(dll,"ll_memory_operator_overrided");
 ok=ok&&status()==0;Sleep(100);ok=ok&&status()==0; // LoadLibrary must not launch the UI worker.
 if(ok){ok=!load(mod())&&status()==1;ok=ok&&count(0)>0&&count(2)==1&&count(3)==1&&count(4)==0;}
 for(auto symbol:{"MMB_WorkspaceRegisterV1","MMB_WorkspacePublishV1","MMB_WorkspacePollV1","MMB_WorkspaceUnregisterV1"})ok=ok&&GetProcAddress(dll,symbol);
 auto publish=reinterpret_cast<int(*)(unsigned long long,char const*,unsigned)>(GetProcAddress(dll,"MMB_WorkspacePublishV1"));
 ok=ok&&publish&&publish(1,"{}",2)==-2;
 std::cout<<"official native registration / allocator / inert DllMain / refused host mode="<<mode<<" "<<(ok?"PASS":"FAIL")<<'\n';
 // Pinned shipping DLL intentionally stays loaded. Process exit cleans fixture.
 return ok?0:1;
}
