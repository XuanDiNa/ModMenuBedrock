// Exercise the production dependency gate with the actual release DLL.
// The private copy runs only in this unsupported test host and cannot hook a game.
#include "../src/NativeHost.cpp"
#include <filesystem>
#include <fstream>
#include <iostream>
int wmain(int argc,wchar_t** argv){
    if(argc!=3)return 2;
    auto mode=std::wstring_view(argv[2]);bool accepted=mode==L"accepted";
    if(!accepted && mode!=L"tampered" && mode!=L"hash-tampered")return 2;
    auto folder=std::filesystem::temp_directory_path()/
        (L"ModMenu-MouseAdmission-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64()));
    std::filesystem::create_directory(folder);
    auto copy=folder/L"MouseTweaksBedrock.dll";
    std::filesystem::copy_file(argv[1],copy);
    // The frozen input is read-only. Only its disposable test copy may mutate.
    auto attributes=GetFileAttributesW(copy.c_str());
    if(attributes==INVALID_FILE_ATTRIBUTES || !SetFileAttributesW(copy.c_str(),attributes&~FILE_ATTRIBUTE_READONLY)){
        std::cerr<<"Cannot make private test copy writable\n";return 2;
    }
    if(mode==L"tampered"){std::ofstream f(copy,std::ios::binary|std::ios::app);f.put('\x5A');if(!f)return 2;}
    if(mode==L"hash-tampered"){
        // Change only the copied PE timestamp, retaining length and executable code.
        std::fstream f(copy,std::ios::binary|std::ios::in|std::ios::out);
        std::uint32_t pe{};f.seekg(0x3C);f.read(reinterpret_cast<char*>(&pe),4);
        f.seekg(pe+8);char stamp{};f.get(stamp);f.seekp(pe+8);f.put(stamp^1);if(!f)return 2;
    }
    auto module=LoadLibraryW(copy.c_str());if(!module){std::cerr<<"LoadLibrary failed "<<GetLastError()<<'\n';return 2;}
    auto status=reinterpret_cast<int(*)()>(GetProcAddress(module,"MTB_GetStatus"));
    if(!status)return 2;
    for(unsigned i=0;i<500 && status()==0;++i)Sleep(10);
    int checks{},failures{};
    auto check=[&](bool ok,char const* name){++checks;if(!ok){++failures;std::cerr<<"FAIL "<<name<<'\n';}};
    check(status()==1,"actual MouseTweaks refuses unsupported test host");
    menu::NativeProfile p;menu::SharedState state;
    // Only stage the immutable definition-host identity. InstallNative must exit
    // at the real dependency gate before inspecting or installing any game hook.
    menu::minhookInitialized=true;menu::definitionReady.store(true);menu::profile=&p;menu::shared=&state;
    std::string reason;auto result=menu::InstallNative(p,state,true,reason);
    check(result==menu::InstallResult::Conflict,"non-Ready dependency cannot activate ModMenu");
    auto expected=accepted?"MouseTweaks did not reach Ready":"Unrecognized MouseTweaks image/status contract";
    check(reason==expected,"exact DLL admission reaches the correct production gate");
    check(!menu::ready.load() && !menu::MMB_PointerOriginal,"no input hooks installed on refusal");
    check(menu::Owned(reinterpret_cast<std::uintptr_t>(status),module),"status export belongs to loaded MouseTweaks module");
    check(!menu::Owned(reinterpret_cast<std::uintptr_t>(status),GetModuleHandleW(nullptr)),"wrong export owner is rejected");
    if(reason!=expected)std::cerr<<"Actual gate: "<<reason<<'\n';
    std::cout<<checks<<" actual MouseTweaks admission checks / "<<failures<<" failures; isolated copy: "<<folder.string()<<'\n';
    // MouseTweaks pins its live worker module; process exit releases this copy.
    return failures?1:0;
}
