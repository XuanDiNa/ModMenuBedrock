#pragma once
#include <string>
#include <string_view>
namespace menu {
std::string_view DefinitionNamespace(std::string_view path);
bool InsertModsDefinition(std::string const& source,std::string& output,std::string_view expectedNamespace={});
inline constexpr char ModsEventName[]="button.mmb_open_mods";
inline constexpr unsigned ModsEventId=[] {unsigned h=0x811c9dc5;for(char c:ModsEventName){if(!c)break;h=(h^static_cast<unsigned char>(c))*0x1000193;}return h;}();
}
