// Offline test DLL only. Never linked into or shipped as the real menu.
#include "ModMenu/Workspace.h"
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstring>
extern "C" __declspec(dllexport) int MMB_GetStatus(){return 3;}
extern "C" __declspec(dllexport) int MMB_TestProviderState(){auto s=menu::Workspaces().Snapshots();if(s.empty()||!s[0].state)return 0;auto const&v=s[0].state->at("controls").at("state").at("text");if(v=="Idle")return 1;if(v.is_string()&&v.get<std::string>().starts_with("Running:"))return 2;if(v.is_string()&&v.get<std::string>().starts_with("Completed:"))return 3;return 4;}
extern "C" __declspec(dllexport) int MMB_TestStart(){auto s=menu::Workspaces().Snapshots();if(s.empty()||!s[0].state)return 0;std::string error;return menu::Workspaces().Submit(s[0].session,"jobs","start",s[0].state->at("revision").get<std::uint64_t>(),{{"name","ABI test"}},"",error)?1:0;}
