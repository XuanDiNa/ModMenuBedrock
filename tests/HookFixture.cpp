#include <MinHook.h>
#include <atomic>
#include <cstdint>
#include <intrin.h>
namespace {using Fn=int(*)(int);Fn original{};std::atomic<int> calls{};int Hook(int n){++calls;return original(n)+10;}}
extern "C" __declspec(dllexport) int FixtureInstall(void* target){if(MH_Initialize()!=MH_OK || MH_CreateHook(target,reinterpret_cast<void*>(Hook),reinterpret_cast<void**>(&original))!=MH_OK || MH_EnableHook(target)!=MH_OK)return 2;return 3;}
extern "C" __declspec(dllexport) int FixtureCalls(){return calls.load();}
namespace {
using Pointer=std::uintptr_t(*)(void*,void const*,char);
Pointer originalPointer{};std::atomic<std::uintptr_t> pointerCaller{};std::atomic<unsigned> pointerCalls{};
std::uintptr_t PointerHook(void* a,void const* b,char c){pointerCaller.store(reinterpret_cast<std::uintptr_t>(_ReturnAddress()));++pointerCalls;return originalPointer(a,b,c);}
}
extern "C" __declspec(dllexport) int FixtureInstallPointer(void* target){if(MH_Initialize()!=MH_OK || MH_CreateHook(target,reinterpret_cast<void*>(PointerHook),reinterpret_cast<void**>(&originalPointer))!=MH_OK || MH_EnableHook(target)!=MH_OK)return 2;return 3;}
extern "C" __declspec(dllexport) std::uintptr_t FixturePointerCaller(){return pointerCaller.load();}
extern "C" __declspec(dllexport) unsigned FixturePointerCalls(){return pointerCalls.load();}
