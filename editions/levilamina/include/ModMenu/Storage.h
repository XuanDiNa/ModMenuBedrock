#pragma once
#include "Core.h"
#include <filesystem>
#include <functional>
namespace menu {
bool ValidUtf8(std::string const&);
bool LoadConfig(std::filesystem::path const&,Config&);
bool SaveConfig(std::filesystem::path const&,Config const&);
std::vector<Entry> Discover(std::filesystem::path const& mods,std::function<bool(std::filesystem::path const&)> const& loaded={});
std::vector<Entry> LoadedCatalog(std::filesystem::path const& mods,std::function<bool(std::filesystem::path const&)> const& loaded);
}
