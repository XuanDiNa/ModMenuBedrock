#pragma once
#include "Core.h"
namespace menu {
struct Color {float r{},g{},b{},a{1};};
struct Paint {Rect rect;Color color;std::string text;float scale{1};};
std::vector<Paint> BuildView(Menu const&,Frame const&);
enum class Chain {Canonical,Wait,KnownReady,Refuse};
Chain ClassifyEntry(bool canonical,bool knownOwner,int status);
}
