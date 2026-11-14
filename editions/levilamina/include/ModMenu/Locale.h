#pragma once
#include "Settings.h"
#include <string>
#include <string_view>
namespace menu {
enum class Locale {English,TraditionalChinese,SimplifiedChinese};
Locale LocaleFromCode(std::string_view) noexcept;
std::string Tr(Locale,std::string_view);
std::string DisplaySetting(SettingField const&,Locale);
std::string DisplayCaption(SettingTranslation const&,std::string_view,Locale,bool external);
std::string DisplayCategory(SettingsDraft const&,std::string_view,Locale);
std::string DisplayOption(SettingField const&,std::size_t,Locale);
std::string DisplayMessage(std::string_view,Locale);
float TextWidth(std::string_view) noexcept;
}
