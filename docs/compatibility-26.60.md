# Minecraft 1.26.60.05 compatibility

Candidate: **0.6.0-test8-mc26.60**. Offline verification and manual gameplay
acceptance are separate. This candidate changes ModMenu only.

The new profile independently pins the complete original and reviewed launcher
image hashes, PE identity, `.text` hash and 35 entry signatures. Native addresses
are constants from exact-image IDA review. There is no runtime scan, arbitrary
header normalization or fallback to another game version. Unknown images and
unrecognized shared hook owners remain inactive.

The 26.45/26.51/26.52 identity, address and guard tables are preserved. The 26.60
adapter accounts for its different scene-stack slots, GUI getters, transition
flags, screen fields and UI render flag. The string draw overload and complete
vtable beginnings were established from constructors and callers. The existing
hook set, top-scene/controller ownership and foreground checks are retained.
One unverified passive pointer counter and stencil observation are omitted on
26.60; neither is an input or rendering permission.

## Menu definitions and resource packs

26.60's definition callback no longer supplies the old resource path. Automatic
button insertion therefore requires its exact callback vtable, a single bounded
216-byte record, an audited vanilla payload SHA-256, and the exact JSON parser
return address. The parser input is checked again before transformation. Modified
or layered definitions cannot acquire automatic insertion authority.

Resource packs continue using **`button.modmenu.open`**, the legacy event alias
and the v1 ownership marker without a contract change. A pack that changes the
menu must provide its own entry; F10 remains available in an eligible native main
or pause menu. See [resource-pack integration](../integration/RESOURCE_PACKS.md).
OreUI, settings, inventory and HUD do not acquire entry permissions.

Private game binaries, IDA databases, decompilation and payload fixtures are
excluded from source and update packages. `tools/verify_native60.py` checks the
reviewed source hashes and projects away the allowed additions to compare the
legacy policies exactly with the frozen baseline.

---

本候選版只更新 ModMenu。26.60 使用獨立核實的完整映像雜湊、程式碼雜湊、35 個入口
簽章及 UI 結構；舊三版本的身分、位址和檢查表保留。前景、頂層場景、控制器及共享
掛鉤擁有者檢查沿用既有政策，不修改遊戲 EXE。

26.60 的原生回呼不再提供舊版資源路徑，因此自動加入按鈕只接受單層、精確雜湊相符
的原版選單內容。修改過選單的材質包須主動提供 `button.modmenu.open` 入口；公用 v1
規格與標記不變。F10 仍限於允許的原生主選單／暫停選單。

離線驗證不等於遊戲內驗收。新 DLL 需人工確認入口、操作、返回與原有插件共存。
