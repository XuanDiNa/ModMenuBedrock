# ModMenu 0.6.0-test11

Windows x64 / LeviLauncher native DLL / MIT / Prerelease.
Minecraft **1.26.45.01 / 1.26.51.01 / 1.26.52.03 / 1.26.60.05**.

## 26.60 fix

The published test10 could refuse 26.60 with `INACTIVE: disk-identity` if the
launcher captured the console-enabled original before rebuilding imports.
That order recalculates the launcher checksum to `0x134D3CBB`; test10 covered
only `0x134D3CBA`. Read-only reconstruction confirms all eight original section
headers/contents and the original entrypoint are unchanged. Test11 adds the
three exact missing full-image hashes. All prior identities and native guards
remain; arbitrary header, code, data and import changes are still rejected.
No game EXE is modified. [Technical notes](../../editions/vanilla/docs/releases/0.6.0-test11.md).

## Changes since the previous public test5

| Version | Included changes |
| --- | --- |
| test6 | Bounded resource-layer diagnostics for missing menu entries. Console causation was not established. |
| test7 | Resource-pack `button.modmenu.open` API, legacy alias, entry ownership contract, bilingual guide and optional example pack. |
| test8 | Independently reviewed 26.60 native profile and render/input/scene adapters. Automatic insertion requires exact single-layer vanilla menus; modified packs provide their own event entry. |
| test9 | Exact 26.51 images after removing PreLoader/LeviLamina, plus the reviewed 26.60 launcher console form. |
| test10 | Exact size/hash admission for MouseTweaks 0.3.3-test1-vanilla51; retains 0.3.2-test1 and all Ready/export/owner/lifetime checks. |
| test11 | Completes 26.60 console-original and both launcher rebuild orders, with real image and mutation regressions. |

Settings v1–v5, Workspace V1 and resource-pack entry v1 remain compatible.
No other plugin is updated. A provider with an exact ModMenu DLL pin needs its
own explicit compatibility review. The independent LeviLamina distribution is
unchanged. The optional MouseTweaks update supports **26.45/51/52 only**, not 26.60.

## Install and validate

Exit Minecraft and back up the plugin folder. Existing installations use the
`update` ZIP, which retains config.json. First installations use `windows-x64`.
Enable one edition and one DLL copy. No LeviLamina reinstallation is required.

Debug and Release each passed **24 offline suites**, exact four-version image
checks, eight menu fixtures, both MouseTweaks admission fixtures and mutation
refusals. The 26.60 image suite checks original GUI/console and all four
launcher checksum/subsystem forms using private copy-on-write pages. Full-file
reconstruction is additional static evidence, not a gameplay test.

Check 26.60 main/pause menus, F10, Configure/return and a second full launch;
include the alternate launcher console setting if you use it. The new DLL has
**not yet been manually accepted in game**. [Testing guide](../../editions/vanilla/TESTING.md).
The test10 GitHub Release is withdrawn and replaced by this prerelease; its Git
tag and local frozen artifacts are retained for provenance.

## 繁體中文

本版修復 test10 在 26.60 的映像拒絕：啟動器先保存開啟控制台的原始檔、再重建
匯入區段時，校驗值與先前收錄的順序不同。已完整核對八個原始區段及入口，新增
三個精確完整檔案雜湊；不改位址、不放寬安全檢查，也不修改遊戲 EXE。

包含 test6–test10 尚未完整公開的累積功能：材質包入口接口、26.60 原生適配、
移除 LeviLamina 後的 26.51 映像支援，以及 MouseTweaks 新版精確配對。
設定、Workspace 與材質包公用接口保留；限定 ModMenu 二進位的提供者須核實新版。
選用 MouseTweaks 配套只支援 26.45／26.51／26.52，**不支援 26.60**。
獨立 LeviLamina 版及其他插件未更新。

退出遊戲並備份後更新；已有安裝使用 update 包保留設定，首次安裝用完整包。
Debug／Release 各 24 組離線回歸通過，含四版本精確映像及負向變異檢查；
這些結果不能代替遊戲內驗收。請在 26.60 檢查主／暫停選單、F10、設定返回及
完全退出後再啟動；使用控制台選項時也測另一種啟動方式。
剛才的 test10 Release 撤下，由 test11 取代，舊標籤及本機凍結檔保留供追溯。

DLL: **988,160 bytes**; SHA-256
`17CB9D3505B6B17942A2975CD7E5F670A277B4B42D61E40042F2C3AA781DAF6E`.
