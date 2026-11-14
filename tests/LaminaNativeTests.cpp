#include "ModMenu/NativeProfile.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <iostream>
#include <vector>
unsigned checks{},failures{};
void Check(bool b,char const*n){++checks;if(!b){++failures;std::cerr<<"FAIL "<<n<<'\n';}}
int wmain(int argc,wchar_t**argv){
 using namespace menu;
 for(auto b:{GameBuild::V45,GameBuild::V51,GameBuild::V52})Check(!LayoutFor(b)&&!ResolveNativeRva(b,0x48560C0)&&NativeGuards(b).empty(),"LL edition refuses legacy native authority");
 Check(!AcceptedIdentity({0x8664,0x6AB54E37,0x12C01000,0x128CD801}),"LL edition rejects old game identity");
 Check(LayoutFor(GameBuild::V60)&&ResolveNativeRva(GameBuild::V60,0x48560C0)==0x4CCE060&&NativeGuards(GameBuild::V60).size()==35,"60 exact native mapping and complete entry guards");
 if(argc<2||argc>3){std::cerr<<"Pass the exact 1.26.60.05 instance directory and optional private launcher-order fixture directory\n";return 2;}
 for(auto name:{L"Minecraft.Windows.exe.orig",L"Minecraft.Windows.exe.bak",L"Minecraft.Windows.exe"}){
  auto path=std::wstring(argv[1])+L"\\"+name;auto file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);LARGE_INTEGER size{};
  if(file==INVALID_HANDLE_VALUE||!GetFileSizeEx(file,&size)){Check(false,"exact image readable");if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);continue;}
  auto map=CreateFileMappingW(file,nullptr,PAGE_WRITECOPY,0,0,nullptr);auto data=map?static_cast<std::byte*>(MapViewOfFile(map,FILE_MAP_COPY,0,0,0)):nullptr;
  Check(data&&VerifyFile({data,std::size_t(size.QuadPart)}),"exact reviewed image admitted");
  if(data){
   auto bytes=std::span(data,std::size_t(size.QuadPart));auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(data+reinterpret_cast<IMAGE_DOS_HEADER*>(data)->e_lfanew);
   auto subsystem=nt->OptionalHeader.Subsystem;
   for(WORD candidate:{WORD(2),WORD(3),WORD(1)}){nt->OptionalHeader.Subsystem=candidate;Check(VerifyFile(bytes)==(candidate==2||(candidate==3&&std::wstring(name)==L"Minecraft.Windows.exe")),"only exact reviewed GUI and console hashes admitted");}
   nt->OptionalHeader.Subsystem=subsystem;
   data[size.QuadPart-1]^=std::byte{1};Check(!VerifyFile(bytes),"non-code mutation refused");data[size.QuadPart-1]^=std::byte{1};
   auto sec=IMAGE_FIRST_SECTION(nt);data[sec->PointerToRawData]^=std::byte{1};Check(!VerifyFile(bytes),"code mutation refused");data[sec->PointerToRawData]^=std::byte{1};
   std::vector<std::byte> header(data,data+8192);Check(VerifyLoadedHeaders(bytes,header,0x180000000),"matching loaded headers accepted");header[256]^=std::byte{1};Check(!VerifyLoadedHeaders(bytes,header,0x180000000),"loaded-header mutation refused");
   Check(VerifyFile(bytes),"private COW mutations never modify original image");UnmapViewOfFile(data);
  }
  if(map)CloseHandle(map);CloseHandle(file);
 }
 if(argc==3){
  Check(AcceptedIdentity({0x8664,0x6AD79EE4,0x13847000,0x134DC051}),"independently reproduced PreLoader-first identity can select 60 descriptor");
  for(auto name:{L"player-preloader-first-console.bin",L"player-preloader-first-gui.bin"}){
   auto path=std::wstring(argv[2])+L"\\"+name;auto file=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);LARGE_INTEGER size{};
   if(file==INVALID_HANDLE_VALUE||!GetFileSizeEx(file,&size)){Check(false,"private loader-order image readable");if(file!=INVALID_HANDLE_VALUE)CloseHandle(file);continue;}
   auto map=CreateFileMappingW(file,nullptr,PAGE_WRITECOPY,0,0,nullptr);auto data=map?static_cast<std::byte*>(MapViewOfFile(map,FILE_MAP_COPY,0,0,0)):nullptr;
   Check(data&&VerifyFile({data,std::size_t(size.QuadPart)}),"exact PreLoader-first whole-file identity admitted");
   if(data){
    auto bytes=std::span(data,std::size_t(size.QuadPart));auto nt=reinterpret_cast<IMAGE_NT_HEADERS64*>(data+reinterpret_cast<IMAGE_DOS_HEADER*>(data)->e_lfanew);
    auto subsystem=nt->OptionalHeader.Subsystem;auto checksum=nt->OptionalHeader.CheckSum;
    for(WORD candidate:{WORD(2),WORD(3),WORD(1)}){nt->OptionalHeader.Subsystem=candidate;Check(VerifyFile(bytes)==(candidate==2||candidate==3),"PreLoader-first admits exact GUI/console pair but no normalized arbitrary subsystem");}
    nt->OptionalHeader.Subsystem=subsystem;
    nt->OptionalHeader.CheckSum^=1;Check(!VerifyFile(bytes),"PreLoader-first unknown checksum refused");nt->OptionalHeader.CheckSum=checksum;
    data[size.QuadPart-1]^=std::byte{1};Check(!VerifyFile(bytes),"PreLoader-first launcher payload mutation refused despite matching game code");data[size.QuadPart-1]^=std::byte{1};
    auto sec=IMAGE_FIRST_SECTION(nt);data[sec->PointerToRawData]^=std::byte{1};Check(!VerifyFile(bytes),"PreLoader-first game code mutation refused");data[sec->PointerToRawData]^=std::byte{1};
    Check(!VerifyFile(bytes.first(bytes.size()-1)),"PreLoader-first truncation refused");
    std::vector<std::byte> header(data,data+8192);Check(VerifyLoadedHeaders(bytes,header,0x180000000),"PreLoader-first loaded-header equality preserved");
    auto live=reinterpret_cast<IMAGE_NT_HEADERS64*>(header.data()+reinterpret_cast<IMAGE_DOS_HEADER*>(header.data())->e_lfanew);live->OptionalHeader.ImageBase=0x180000000;
    Check(VerifyLoadedHeaders(bytes,header,0x180000000),"PreLoader-first loaded ASLR header accepted");live->OptionalHeader.CheckSum^=1;
    Check(!VerifyLoadedHeaders(bytes,header,0x180000000),"PreLoader-first changed loaded header refused");
    Check(VerifyFile(bytes),"PreLoader-first restored COW view admitted without changing source");UnmapViewOfFile(data);
   }
   if(map)CloseHandle(map);CloseHandle(file);
  }
 }
 std::cout<<checks<<" LL native checks / "<<failures<<" failures\n";return failures?1:0;
}
