# ModMenu 0.6.0-test10 / 原版路線更新

Windows x64 / LeviLauncher native DLL / MIT.
Minecraft **1.26.45.01 / 1.26.51.01 / 1.26.52.03 / 1.26.60.05**.
No LeviLamina API or server plugin is required. The LL edition remains separate.
Public release name: **ModMenu 0.6.0-test10**. The frozen DLL retains its internal
build identifier `0.6.0-test10-mouse51` so its tested identity remains traceable.

This prerelease includes all native-edition work since public test5:

| Version | Changes |
| --- | --- |
| test6-layer-diag1 | Bounded resource-layer diagnostics for missing entries. This did not establish that enabling the game console fixes layered menus. |
| test7-resourcepack | Public `button.modmenu.open` event, legacy alias, ownership contract, bilingual integration guide and optional example pack. Pack authors control entry appearance, label and placement. |
| test8-mc26.60 | Independently reviewed 26.60 native profile and scene/render/input adapters, retaining 26.45/51/52. Automatic entry insertion on 26.60 requires the exact single-layer vanilla menu; modified/layered packs integrate the public event. |
| test9-vanilla51 | Exact 26.51 images after removing PreLoader/LeviLamina. All eight original game sections, headers and original entry match the previously reviewed 51 sample. Also pins the reviewed 26.60 launcher console form. Unknown images remain refused. |
| test10-mouse51 | Exact size/hash admission for MouseTweaks 0.3.3-test1-vanilla51, while retaining 0.3.2-test1. Ready status, export/jump ownership and pinned module lifetime remain mandatory. Adds real companion admission/mutation regression tests. |

Settings v1-v5, Workspace V1 and resource-pack entry v1 stay compatible. This
update changes no other plugin's settings/trigger logic and does not update the
independent LL distribution. A Workspace provider pinning another ModMenu DLL
needs its own compatibility review.

## Install and test

Exit Minecraft and back up the plugin folders. Use the native `update` ZIP and
keep existing `config.json` and logs. Enable only one ModMenu edition and one
copy of each DLL. The full ZIP adds default settings for first installations.

On 26.51 without LeviLamina, an older MouseTweaks may refuse the game image,
which also prevents safe ModMenu shared-hook initialization. Use the paired
**MouseTweaks 0.3.3-test1-vanilla51**, DLL SHA-256
`77425954246FCD765070911FAF6135C7E60A4A5E4946FC84C5A1FDC88C461431`.
This release supplies its exact reviewed update archive as a companion asset;
the MouseTweaks repository is not changed by this publication. Temporarily
disable an incompatible companion to test ModMenu alone. Do not reinstall
LeviLamina to work around image admission.

The inspected 26.51 session reached MouseTweaks `NATIVE READY`, ModMenu status 3
and a Mods open event with zero ModMenu faults. This establishes startup/menu
evidence only. Check the main and pause menus, configured shortcut, Escape,
container drag/collection/placement/wheel operations and a second complete
launch. The new MouseTweaks supports 26.45/51/52, **not 26.60**.

Debug and Release each passed **24 offline ModMenu suites**, four-version
private image/menu checks, both exact MouseTweaks DLLs and mutation refusals.
Public-source verification is rerun from the edition checkout; a rebuilt DLL
is not substituted for the frozen packaged DLL. Full gesture/restart and new
build cross-version gameplay acceptance remain pending.

ModMenu DLL: **987,648 bytes**, SHA-256
`2039530A342CA2E44D8F7A3AC7DB85BB66546E2BBB720EF9CE14392B13005482`.

## 繁體中文

本次公開原版路線 **ModMenu 0.6.0-test10**（DLL 內部標記為 `0.6.0-test10-mouse51`），包含上表 test6 到 test10 累積內容：
材質包入口、26.60 原生適配、移除 LeviLamina 後的 26.51 精確映像支援，以及
MouseTweaks 0.3.3-test1-vanilla51 的精確配對。保留完整映像、程式碼、載入標頭、
入口、掛鉤擁有者、Ready、場景與前景檢查，不修改遊戲 EXE。

完全退出遊戲並備份後更新；已有安裝選 `update` 包，保留設定與日誌。首次安裝可用
完整包。只啟用一個 ModMenu 分支，避免重複 DLL。26.51 若搭配因映像拒絕而未就緒
的舊 MouseTweaks，也會阻止 ModMenu 安全串接，請使用上述精確配套新版；本次
Release 附有其已核驗更新包，但不修改 MouseTweaks 倉庫。無需重裝 LeviLamina。

本機 26.51 已記錄兩者 Ready 與 Mods 開啟，ModMenu faults 為 0；這仍不是完整
操作驗收。請確認主／暫停選單、快捷鍵、Escape、各項容器操作及完全退出後再次
啟動。新版 MouseTweaks 支援 26.45／26.51／26.52，沒有新增 26.60 支援。

Debug／Release 各 24 組 ModMenu 離線測試及配對／負向檢查通過。發佈保留已交付的
精確 DLL，公開原始碼重新驗證；完整拖曳、重啟及新編譯 DLL 的跨版本人工驗收仍待完成。
獨立 LL 分支及其附件保持不變。
