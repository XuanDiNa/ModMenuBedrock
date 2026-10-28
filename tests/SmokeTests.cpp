#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;auto dll=LoadLibraryW(argv[1]);if(!dll){std::cerr<<"LoadLibrary failed "<<GetLastError()<<'\n';return 1;}
 auto status=reinterpret_cast<int(*)()>(GetProcAddress(dll,"MMB_GetStatus"));auto version=reinterpret_cast<char const*(*)()>(GetProcAddress(dll,"MMB_GetVersion"));
 if(!status || !version)return 1;int code=0;for(unsigned n=0;n<200 && (code=status())==0;++n)Sleep(25);
 std::cout<<"version="<<version()<<" unsupported-host-status="<<code<<'\n';return code==1?0:1;
}
