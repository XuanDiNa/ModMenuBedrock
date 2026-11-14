// Isolated loader double built against the exact official SDK. Never deployed.
#define LL_EXPORT
#include "ll/api/Versions.h"
#include "ll/api/mod/NativeMod.h"
#include "ll/api/memory/Memory.h"
#include "mc/deps/core/memory/IMemoryAllocator.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <malloc.h>
#include <atomic>
namespace {
std::atomic<unsigned> allocations{},releases{};
bool client=true;unsigned patch=7,enableRegistrations{},disableRegistrations{},unloadRegistrations{};
class Allocator final:public Bedrock::Memory::IMemoryAllocator {
 void* _allocate(uint64 n) override{++allocations;return HeapAlloc(GetProcessHeap(),0,n?n:1);}
 void _release(gsl::not_null<void*> p) override{++releases;HeapFree(GetProcessHeap(),0,p.get());}
 void* _alignedAllocate(uint64 n,uint64 align) override{++allocations;return _aligned_malloc(n?n:1,align);}
 void _alignedRelease(gsl::not_null<void*> p) override{++releases;_aligned_free(p.get());}
 uint64 _getUsableSize(gsl::not_null<void*> p,bool) override{return HeapSize(GetProcessHeap(),0,p.get());}
 void* _realloc(gsl::not_null<void*> p,uint64 n) override{return HeapReAlloc(GetProcessHeap(),0,p.get(),n);}
 void* _alignedRealloc(gsl::not_null<void*> p,uint64 n,uint64 align) override{return _aligned_realloc(p.get(),n,align);}
};
}
namespace ll {
data::Version getLoaderVersion(){return data::Version{26,60,static_cast<uint16>(patch)};}
bool isClient(){return client;}
namespace memory {
Bedrock::Memory::IMemoryAllocator& getDefaultAllocator(){static Allocator a;return a;}
[[noreturn]] void throwMemoryException(std::size_t){throw std::bad_alloc{};}
}
namespace mod {
struct Mod::Impl {CallbackFn enable,disable;};
struct NativeMod::Impl {};
Mod::Mod(Manifest):mImpl(std::make_unique<Impl>()){}
Mod::~Mod()=default;
NativeMod::NativeMod(Manifest manifest,sys_utils::HandleT):Mod(std::move(manifest)),mImpl(std::make_unique<Impl>()){}
NativeMod::~NativeMod()=default;
void Mod::onEnable(CallbackFn cb) noexcept{++enableRegistrations;mImpl->enable=std::move(cb);}
void Mod::onDisable(CallbackFn cb) noexcept{++disableRegistrations;mImpl->disable=std::move(cb);}
void Mod::onUnload(CallbackFn) noexcept{++unloadRegistrations;}
}
}
extern "C" __declspec(dllexport) void* FixtureMod(){static ll::mod::NativeMod mod{ll::mod::Manifest{}};return &mod;}
extern "C" __declspec(dllexport) void FixtureEnvironment(unsigned mode){client=mode!=2;patch=mode==1?6:7;}
extern "C" __declspec(dllexport) unsigned FixtureCount(unsigned i){switch(i){case 0:return allocations.load();case 1:return releases.load();case 2:return enableRegistrations;case 3:return disableRegistrations;default:return unloadRegistrations;}}
#define LL_MEMORY_OPERATORS
#include "ll/api/memory/MemoryOperators.h"
