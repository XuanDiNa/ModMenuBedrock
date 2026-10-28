#include "ModMenu/Storage.h"
#include "json.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <algorithm>
#include <cmath>
namespace menu {
namespace {
namespace fs=std::filesystem;using Json=nlohmann::json;
bool Safe(fs::path path){
    if(!path.is_absolute())path=fs::absolute(path);
    for(auto p=path;!p.empty();){auto a=GetFileAttributesW(p.c_str());
        if(a==INVALID_FILE_ATTRIBUTES || (a&FILE_ATTRIBUTE_REPARSE_POINT))return false;
        auto next=p.parent_path();if(next==p)break;p=next;
    }return true;
}
Json Read(fs::path const& path){
    if(!Safe(path))return Json();
    auto h=CreateFileW(path.c_str(),GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,FILE_FLAG_OPEN_REPARSE_POINT,nullptr);
    if(h==INVALID_HANDLE_VALUE)return Json();
    BY_HANDLE_FILE_INFORMATION info{};LARGE_INTEGER size{};std::string s;DWORD used{};
    bool ok=GetFileInformationByHandle(h,&info) && !(info.dwFileAttributes&(FILE_ATTRIBUTE_DIRECTORY|FILE_ATTRIBUTE_REPARSE_POINT)) && GetFileSizeEx(h,&size) && size.QuadPart>0 && size.QuadPart<=65536;
    if(ok){s.resize(static_cast<std::size_t>(size.QuadPart));ok=ReadFile(h,s.data(),DWORD(s.size()),&used,nullptr) && used==s.size();}CloseHandle(h);
    if(!ok || !ValidUtf8(s))return Json();
    auto j=Json::parse(s,nullptr,false);return j.is_object()?j:Json();
}
std::string String(Json const& j,char const* key,std::string fallback={},std::size_t limit=2048){
    if(!j.contains(key) || !j[key].is_string())return fallback;
    auto s=j[key].get<std::string>();
    if(s.size()>limit || !ValidUtf8(s))return fallback;
    for(auto& c:s)if(static_cast<unsigned char>(c)<32)c=' ';
    // Formatting codes in arbitrary manifests must not affect adjacent labels.
    for(std::size_t p=0;(p=s.find("\xc2\xa7",p))!=std::string::npos;)s.replace(p,2,"?");
    return s;
}
bool ConfigValid(Config const& c){return (!c.openKey || UsableKey(c.openKey)) &&
    c.button.anchor>=0 && c.button.anchor<=3 && std::isfinite(c.button.x) && std::isfinite(c.button.y) &&
    std::abs(c.button.x)<=32768 && std::abs(c.button.y)<=32768 && std::isfinite(c.button.scale) && c.button.scale>=.5f && c.button.scale<=2.f;}
}
bool ValidUtf8(std::string const& s){
    for(std::size_t p=0;p<s.size();){auto c=static_cast<unsigned char>(s[p++]);if(c<128)continue;
        unsigned cp{},n{},min{};
        if(c>=0xc2 && c<=0xdf){cp=c&31;n=1;min=0x80;}
        else if(c>=0xe0 && c<=0xef){cp=c&15;n=2;min=0x800;}
        else if(c>=0xf0 && c<=0xf4){cp=c&7;n=3;min=0x10000;}else return false;
        if(n>s.size()-p)return false;
        while(n--){c=static_cast<unsigned char>(s[p++]);if((c&0xc0)!=0x80)return false;cp=(cp<<6)|(c&63);}
        if(cp<min || cp>0x10ffff || (cp>=0xd800 && cp<=0xdfff))return false;
    }return true;
}
bool LoadConfig(fs::path const& path,Config& out){
    try{auto j=Read(path);if(!j.is_object())return false;Config c;
        if(j.contains("openKey")){if(!j["openKey"].is_number_unsigned() && !j["openKey"].is_number_integer())return false;auto k=j["openKey"].get<std::int64_t>();if(k<0 || k>=255)return false;c.openKey=unsigned(k);}
        if(j.contains("button")){auto const& b=j["button"];if(!b.is_object())return false;
            if(b.contains("anchor")){if(!b["anchor"].is_number_integer())return false;c.button.anchor=b["anchor"].get<int>();}
            if(b.contains("x"))c.button.x=b["x"].get<float>();if(b.contains("y"))c.button.y=b["y"].get<float>();if(b.contains("scale"))c.button.scale=b["scale"].get<float>();
        }if(!ConfigValid(c))return false;out=c;return true;
    }catch(...){return false;}
}
bool SaveConfig(fs::path const& path,Config const& c){
    try{if(!ConfigValid(c) || !Safe(path.parent_path()))return false;
        if(fs::exists(path) && !Safe(path))return false;
        Json j={{"openKey",c.openKey},{"button",{{"anchor",c.button.anchor},{"x",c.button.x},{"y",c.button.y},{"scale",c.button.scale}}}};auto bytes=j.dump(2)+"\n";
        auto tmp=path;tmp+=L".tmp-"+std::to_wstring(GetCurrentProcessId())+L"-"+std::to_wstring(GetTickCount64());
        auto h=CreateFileW(tmp.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_WRITE_THROUGH,nullptr);if(h==INVALID_HANDLE_VALUE)return false;
        DWORD used{};bool ok=WriteFile(h,bytes.data(),DWORD(bytes.size()),&used,nullptr) && used==bytes.size() && FlushFileBuffers(h);CloseHandle(h);
        if(ok)ok=Safe(path.parent_path()) && (!fs::exists(path) || Safe(path)) && MoveFileExW(tmp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);
        if(!ok)DeleteFileW(tmp.c_str());return ok;
    }catch(...){return false;}
}
std::vector<Entry> LoadedCatalog(fs::path const& mods,std::function<bool(fs::path const&)> const& loaded){
    auto entries=Discover(mods,loaded);
    std::erase_if(entries,[](auto const& entry){return !entry.loaded;});return entries;
}
std::vector<Entry> Discover(fs::path const& mods,std::function<bool(fs::path const&)> const& loaded){
    std::vector<Entry> result;
    try{if(!Safe(mods))return result;std::size_t scanned{};
        for(auto const& dir:fs::directory_iterator(mods)){
            if(++scanned>512 || result.size()>=256)break;
            if(!dir.is_directory() || !Safe(dir.path()))continue;
            auto manifest=dir.path()/"manifest.json";bool disabled=false;
            if(!fs::exists(manifest)){
                manifest=dir.path()/"manifest.json.close";disabled=true;
                if(!fs::exists(manifest))manifest=dir.path()/"manifest.json.disabled";
            }
            auto j=Read(manifest);if(!j.is_object())continue;
            auto entry=String(j,"entry",{},240);fs::path dll(std::u8string(entry.begin(),entry.end()));
            if(entry.empty() || dll.is_absolute() || dll.has_parent_path() || dll.extension()!=L".dll" || entry.find(':')!=std::string::npos)continue;
            disabled=disabled || (j.contains("enabled") && j["enabled"].is_boolean() && !j["enabled"].get<bool>());
            auto raw=dir.path().filename().u8string();std::string id(raw.begin(),raw.end());
            Entry e{id,String(j,"name",id,160),String(j,"version","Unknown",80),String(j,"author",{},160),String(j,"description"),"Installed (not loaded)",String(j,"website",{},512)};
            auto meta=Read(dir.path()/"mod.json");if(meta.is_object()){
                e.name=String(meta,"name",e.name,160);e.version=String(meta,"version",e.version,80);e.author=String(meta,"author",e.author,160);e.description=String(meta,"description",e.description);e.website=String(meta,"website",e.website,512);
            }
            auto full=dir.path()/dll;
            bool safe=fs::is_regular_file(full) && Safe(full);
            e.loaded=safe && loaded && loaded(full);
            if(e.loaded)e.status="DLL loaded (Ready unknown)";
            else if(disabled)e.status="Disabled manifest";
            else if(!safe)e.status="Entry missing/unsafe";
            result.push_back(std::move(e));
        }
        std::sort(result.begin(),result.end(),[](auto const& a,auto const& b){return a.name<b.name;});
    }catch(...){/* Retain only already validated entries. */}return result;
}
}
