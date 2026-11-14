# Independent LeviLamina client edition

Approved scope: ModMenu only, separate from the vanilla edition. Follow the official LeviLamina native-mod template and v26.60.7 API for Minecraft 1.26.60.05 Windows x64.

Use `native`/`client` manifest, `LL_REGISTER_MOD`, official xmake link rule and shared allocator. Startup belongs to `load`, not DllMain; client mods load before packs and enable at `_InitComplete`. Install definition hooks during load so main-menu definitions are observed. No unload callback: live native hooks and registered Workspace callbacks require process lifetime. Disable input/render and stop worker on LL disable; never unload a live trampoline.

Use independent `ModMenuBedrockLL` DLL/folder/config/log. Refuse concurrent vanilla ModMenu. Keep Configure v1-v5, binding SDK, Workspace V1 and resource-pack button API. Existing providers with hardcoded DLL names or exact binary pins need their own explicit update; do not bypass their guards or edit those projects.

Only exact 26.60 profiles are authorities. Admit individually reviewed GUI and console loader image hashes, retain full file/text/loaded-header/entry validation and all foreground, top-scene, controller and hook-owner guards. Unknown modifications fail closed.

Build Debug and Release with official SDK. Check actual exports/imports, allocator and lifecycle in isolated hosts, original UI regressions, exact game images and package contents. Do not launch/inject/install Minecraft or upload. Gameplay remains for user acceptance after testable delivery.
