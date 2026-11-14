#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include "../integration/sdk/modmenu_workspace_v1.h"
int wmain(int argc,wchar_t**argv){if(argc!=2)return 2;auto lib=LoadLibraryW(argv[1]);if(!lib)return 2;
 for(auto name:{"MMB_WorkspaceRegisterV1","MMB_WorkspacePublishV1","MMB_WorkspacePollV1","MMB_WorkspaceUnregisterV1"})if(!GetProcAddress(lib,name)){std::cerr<<"FAIL missing public workspace API: "<<name<<'\n';return 1;}
 auto publish=reinterpret_cast<MMB_WorkspacePublishV1Fn>(GetProcAddress(lib,"MMB_WorkspacePublishV1"));
 if(publish(1,"{}",2)!=-2){std::cerr<<"FAIL workspace must stay inactive in unsupported native host\n";return 1;}
 std::cout<<"Public workspace exports present; unsupported native host refuses publication\n";return 0;
}
