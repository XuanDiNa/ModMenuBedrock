#include "ModMenu/Workspace.h"
#include "ModMenu/Storage.h"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <filesystem>
#include <fstream>
extern "C" int MMB_GetStatus();
namespace {
bool SafePath(std::filesystem::path p){if(!p.is_absolute())return false;for(;;){auto flags=GetFileAttributesW(p.c_str());if(flags==INVALID_FILE_ATTRIBUTES||flags&FILE_ATTRIBUTE_REPARSE_POINT)return false;auto next=p.parent_path();if(next==p)break;p=next;}return true;}
bool CopyInput(char const*p,std::uint32_t n,std::size_t limit,std::string&out){if(!p||!n||n>limit)return false;out.assign(p,n);return true;}
}
extern "C" __declspec(dllexport) std::int32_t MMB_WorkspaceRegisterV1(void const*owner,char const*raw,std::uint32_t bytes,std::uint64_t*out){
 if(!out)return -1;*out=0;if(MMB_GetStatus()!=3)return -2;
 try{std::string input;if(!CopyInput(raw,bytes,65536,input)||!owner)return -1;HMODULE module{};
 if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(owner),&module)||module==GetModuleHandleW(nullptr))return -3;
 wchar_t filename[32768]{};auto length=GetModuleFileNameW(module,filename,32768);if(!length||length>=32768)return -3;std::filesystem::path path(filename);auto dir=path.parent_path();
 HMODULE self{};if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,reinterpret_cast<LPCWSTR>(&MMB_WorkspaceRegisterV1),&self))return -3;
 wchar_t ownPath[32768]{};auto ownLength=GetModuleFileNameW(self,ownPath,32768);if(!ownLength||ownLength>=32768||!std::filesystem::equivalent(dir.parent_path(),std::filesystem::path(ownPath).parent_path().parent_path()))return -3;
 if(!SafePath(path)||!SafePath(dir/L"manifest.json"))return -3;auto size=std::filesystem::file_size(dir/L"manifest.json");if(!size||size>65536)return -3;std::ifstream file(dir/L"manifest.json",std::ios::binary);std::string manifest(static_cast<std::size_t>(size),'\0');if(!file.read(manifest.data(),manifest.size()))return -3;
 auto j=menu::StrictJson(manifest,65536);auto entry=j.at("entry").get<std::string>(),version=j.at("version").get<std::string>();auto id=dir.filename().string();if(path.filename().string()!=entry||j.value("enabled",true)==false)return -3;
 auto definition=menu::ParseWorkspace(input,id,entry,version);HMODULE pinned{};if(!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_PIN,reinterpret_cast<LPCWSTR>(owner),&pinned)||pinned!=module)return -3;
 *out=menu::Workspaces().Register(std::move(definition));return *out?0:-3;
 }catch(...){return -3;}
}
extern "C" __declspec(dllexport) std::int32_t MMB_WorkspacePublishV1(std::uint64_t session,char const*raw,std::uint32_t bytes){if(MMB_GetStatus()!=3)return -2;try{std::string value;if(!CopyInput(raw,bytes,262144,value))return -1;return menu::Workspaces().Publish(session,value)?0:-3;}catch(...){return -3;}}
extern "C" __declspec(dllexport) std::int32_t MMB_WorkspacePollV1(std::uint64_t session,char*out,std::uint32_t capacity,std::uint32_t*required){if(!required||capacity>32768||(!out&&capacity))return -1;*required=0;if(MMB_GetStatus()!=3)return -2;try{if(!menu::Workspaces().Poll(session,out,capacity,*required))return -3;return *required>capacity?1:0;}catch(...){return -3;}}
extern "C" __declspec(dllexport) std::int32_t MMB_WorkspaceUnregisterV1(std::uint64_t session){try{return menu::Workspaces().Unregister(session)?0:-3;}catch(...){return -3;}}
