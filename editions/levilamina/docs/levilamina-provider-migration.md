# LeviLamina edition: integration notes

This independent native client mod uses `ModMenuBedrockLL.dll` and LeviLamina's `native` manager. It is not the vanilla ModMenuBedrock distribution.

Settings descriptions v1-v5, keyboard bindings, Workspace V1 signatures/return codes and resource-pack `button.modmenu.open` are unchanged. Native LL manifests are discovered when their entry DLL is actually loaded. Folder, loaded-module and provider ownership checks remain.

Workspace providers must explicitly resolve the loaded `ModMenuBedrockLL.dll`. Providers pinning a vanilla DLL hash must review and add this separate binary themselves. This package does not modify providers, alias the DLL name, bypass admission or claim unchanged BedrockTool binaries are compatible. The bundled example resolves the LL edition.

Only one ModMenu edition may run. Configuration and logs use `mods/ModMenuBedrockLL/`; no migration or overwrite of vanilla settings occurs.

The runtime manifest does not list LeviLamina as an ordinary mod dependency: its `preload-native` manifest is filtered out by the official native mod registrar. The DLL imports the official runtime and checks `ll::isClient()` and the exact loader version 26.60.7 before preparing hooks. Package-manager dependencies, if published later, belong in the package metadata described by the official template.

`LL_REGISTER_MOD` binds the official lifecycle. Real `MemoryOperators.h` overrides are included once. `load` prepares verified definition hooks before client pack initialization; `enable` starts the worker at `_InitComplete`. `disable` permanently gates processing and signals worker shutdown. Engine trampolines/provider callbacks remain pinned until process exit. No unload callback is registered: restart Minecraft after changing this DLL.

Official references:

- https://github.com/LiteLDev/LeviLamina/blob/v26.60.7/docs/main/contents/developer_guides/tutorials/create_your_first_mod.zh.md
- https://github.com/LiteLDev/levilamina-mod-template
- https://github.com/LiteLDev/LeviLamina/blob/v26.60.7/src/ll/api/mod/RegisterHelper.h
- https://github.com/LiteLDev/LeviLamina/blob/v26.60.7/src/ll/api/memory/MemoryOperators.h
- https://github.com/LiteLDev/LeviLamina/blob/v26.60.7/src-client/ll/core/main_win.cpp
- https://github.com/LiteLDev/LeviLamina/blob/v26.60.7/src/ll/core/mod/ModRegistrar.cpp
