// Real ModMenu adapter plus a separate, independently linked MinHook owner.
// Does not load Minecraft or alter an installed plugin.
#include "../src/NativeHost.cpp"
#include <iostream>
namespace {
using TestPointer=std::uintptr_t(*)(void*,void const*,char);
volatile unsigned nativeCalls{};
std::array<std::uintptr_t,3> nativeArgs{};
__declspec(noinline) std::uintptr_t Target(void* a,void const* b,char c){
    ++nativeCalls;nativeArgs={reinterpret_cast<std::uintptr_t>(a),reinterpret_cast<std::uintptr_t>(b),static_cast<unsigned char>(c)};
    return 0xA1B2C3D4E5F60788ULL;
}
TestPointer volatile invoke=Target;
__declspec(noinline) std::uintptr_t Invoke(){
    volatile auto result=invoke(reinterpret_cast<void*>(std::uintptr_t{0x12345678}),reinterpret_cast<void const*>(std::uintptr_t{0x87654321}),static_cast<char>(-91));
    return result;
}
}
int wmain(int argc,wchar_t** argv){
    if(argc!=2)return 2;
    auto dll=LoadLibraryW(argv[1]);if(!dll)return 2;
    auto install=reinterpret_cast<int(*)(void*)>(GetProcAddress(dll,"FixtureInstallPointer"));
    auto caller=reinterpret_cast<std::uintptr_t(*)()>(GetProcAddress(dll,"FixturePointerCaller"));
    auto calls=reinterpret_cast<unsigned(*)()>(GetProcAddress(dll,"FixturePointerCalls"));
    if(!install || !caller || !calls || install(reinterpret_cast<void*>(Target))!=3)return 2;
    int checks{},failures{};
    auto check=[&](bool ok,char const* name){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
    auto result=Invoke();auto originalCaller=caller();
    check(result==0xA1B2C3D4E5F60788ULL && originalCaller && calls()==1 && nativeCalls==1,"baseline real pointer hook forwards once");
    check(menu::KnownJump(reinterpret_cast<std::uintptr_t>(Target),dll),"actual shared entry resolves to independent hook owner");
    check(!menu::KnownJump(reinterpret_cast<std::uintptr_t>(Target),GetModuleHandleW(nullptr)),"shared entry rejects a wrong jump owner");
    check(menu::ClassifyEntry(false,true,3)==menu::Chain::KnownReady,"verified owned jump needs formal Ready");
    check(menu::ClassifyEntry(false,true,0)==menu::Chain::Wait,"initializing owned jump waits");
    for(int status:{1,2,4,-1})check(menu::ClassifyEntry(false,true,status)==menu::Chain::Refuse,"non-Ready owned jump refuses");
    check(menu::ClassifyEntry(false,false,3)==menu::Chain::Refuse,"Ready alone never admits unknown jump owner");
    if(MH_Initialize()!=MH_OK || MH_CreateHook(reinterpret_cast<void*>(Target),reinterpret_cast<void*>(menu::PointerHook),reinterpret_cast<void**>(&menu::MMB_PointerOriginal))!=MH_OK || MH_EnableHook(reinterpret_cast<void*>(Target))!=MH_OK)return 2;
    menu::SharedState state;menu::NativeProfile profile;menu::profile=&profile;menu::shared=&state;
    for(bool active:{false,true}){
        menu::ready.store(active);result=Invoke();
        check(caller()==originalCaller,"ModMenu preserves exact original caller for underlying Mouse Tweaks-style gate");
        check(result==0xA1B2C3D4E5F60788ULL,"unhandled pointer keeps full native return value");
        check(nativeArgs==std::array<std::uintptr_t,3>{0x12345678,0x87654321,0xA5},"unhandled pointer keeps all three native arguments");
    }
    check(calls()==3 && nativeCalls==3,"ModMenu forwards exactly once per unhandled event");
    if(MH_DisableHook(reinterpret_cast<void*>(Target))!=MH_OK)return 2;
    result=Invoke();check(caller()==originalCaller && result==0xA1B2C3D4E5F60788ULL && calls()==4 && nativeCalls==4,"disabling outer hook restores independent previous owner");
    std::cout<<checks<<" real pointer-chain checks / "<<failures<<" failures\n";
    return failures?1:0;
}
