#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <MinHook.h>
#include <iostream>
namespace {volatile int base=5;int vanillaCalls{},ourCalls{};bool suppress{};using Fn=int(*)(int);Fn previous{};
__declspec(noinline) int Target(int n){++vanillaCalls;return n+base;}
int Ours(int n){++ourCalls;if(suppress)return 900;return previous(n)+100;}
}
int wmain(int argc,wchar_t** argv){
 if(argc!=2)return 2;auto dll=LoadLibraryW(argv[1]);if(!dll)return 1;
 auto install=reinterpret_cast<int(*)(void*)>(GetProcAddress(dll,"FixtureInstall"));auto calls=reinterpret_cast<int(*)()>(GetProcAddress(dll,"FixtureCalls"));
 if(!install || !calls || install(reinterpret_cast<void*>(Target))!=3)return 1;
 auto invoke=static_cast<Fn>(Target);if(invoke(3)!=18 || vanillaCalls!=1 || calls()!=1)return 1;
 if(MH_Initialize()!=MH_OK || MH_CreateHook(reinterpret_cast<void*>(Target),reinterpret_cast<void*>(Ours),reinterpret_cast<void**>(&previous))!=MH_OK || MH_EnableHook(reinterpret_cast<void*>(Target))!=MH_OK)return 1;
 if(invoke(3)!=118 || vanillaCalls!=2 || calls()!=2 || ourCalls!=1)return 1;
 suppress=true;if(invoke(3)!=900 || vanillaCalls!=2 || calls()!=2 || ourCalls!=2)return 1;
 suppress=false;if(MH_DisableHook(reinterpret_cast<void*>(Target))!=MH_OK || invoke(3)!=18 || vanillaCalls!=3 || calls()!=3)return 1;
 std::cout<<"4 real hook-chain checks / 0 failures; two independent static MinHook instances\n";return 0;
}
