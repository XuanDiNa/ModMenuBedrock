// Exercise the exact shipping assembly at its observer/previous-owner boundary.
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <array>
#include <cstdint>
#include <iostream>
namespace {
bool consume{},throwCpp{},throwSeh{};unsigned observed{},forwarded{};
std::array<std::uintptr_t,3> arguments{};std::uintptr_t caller{};
std::uintptr_t Previous(void* a,void const* b,char c){++forwarded;arguments={reinterpret_cast<std::uintptr_t>(a),reinterpret_cast<std::uintptr_t>(b),static_cast<unsigned char>(c)};return 0xFA112233445566A5ULL;}
}
extern "C" {
    std::uintptr_t PointerHook(void*,void const*,char) noexcept(false);
    decltype(&Previous) MMB_PointerOriginal=Previous;
    bool MMB_PointerObserver(void*,void const*,char,std::uintptr_t originalCaller) noexcept(false){
        ++observed;caller=originalCaller;
        if(throwCpp)throw 73;
        if(throwSeh)RaiseException(0xE0420001,0,0,nullptr);
        return consume;
    }
}
bool ObserveSeh(){__try{PointerHook(nullptr,nullptr,0);return false;}__except(GetExceptionCode()==0xE0420001?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH){return true;}}
int main(){
    int checks{},failures{};auto check=[&](bool ok,char const* name){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
    auto result=PointerHook(reinterpret_cast<void*>(std::uintptr_t{0x1234}),reinterpret_cast<void const*>(std::uintptr_t{0xABCD}),static_cast<char>(-91));
    check(result==0xFA112233445566A5ULL && forwarded==1 && observed==1,"unhandled bridge preserves the previous owner's full result exactly once");
    check(arguments==std::array<std::uintptr_t,3>{0x1234,0xABCD,0xA5} && caller,"observer cannot clobber the three forwarded argument registers");
    consume=true;check(PointerHook(nullptr,nullptr,0)==1 && forwarded==1 && observed==2,"consumed menu pointer returns true without entering previous hook");
    consume=false;for(unsigned i=0;i<10000;++i)PointerHook(nullptr,nullptr,0);
    check(forwarded==10001 && observed==10002,"repeated tail forwarding keeps stack and call accounting balanced");
    throwCpp=true;bool caught=false;try{PointerHook(nullptr,nullptr,0);}catch(int value){caught=value==73;}
    check(caught && forwarded==10001,"Windows x64 unwind crosses bridge for observer C++ exception");throwCpp=false;
    throwSeh=true;check(ObserveSeh() && forwarded==10001,"Windows SEH unwinds the described bridge frame");throwSeh=false;
    check(PointerHook(nullptr,nullptr,0)==0xFA112233445566A5ULL && forwarded==10002,"normal forwarding remains balanced after exceptional unwind");
    std::cout<<checks<<" pointer-bridge ABI checks / "<<failures<<" failures\n";return failures?1:0;
}
