#pragma once
#include "Core.h"
namespace menu {
struct SettingRow {int index{};Rect bounds,value,reset,minus,plus,rail,number;};
struct ConfigureLayout {
    Rect previous,next,category,resetAll,help,status,popup;
    std::vector<SettingRow> rows;
    float optionHeight{20};
};
std::vector<std::string> SettingCategories(SettingsDraft const&);
std::vector<int> VisibleSettings(Menu const&);
ConfigureLayout ArrangeSettings(Menu const&,Frame const&);
}
