#include "ModMenu/LaminaRuntime.h"
#include "ll/api/mod/RegisterHelper.h"

namespace {
class ModMenuLamina {
public:
    static ModMenuLamina& getInstance(){static ModMenuLamina self;return self;}
    bool load(){return menu::LoadLaminaRuntime();}
    bool enable(){return menu::EnableLaminaRuntime();}
    bool disable(){return menu::DisableLaminaRuntime();}
    // No unload callback: engine trampolines and Workspace providers are pinned.
};
}
LL_REGISTER_MOD(ModMenuLamina,ModMenuLamina::getInstance());
