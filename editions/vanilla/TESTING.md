# Paired 26.51 native compatibility test / 配套測試

MouseTweaksBedrock 0.3.3-test1-vanilla51
ModMenuBedrock 0.6.0-test10-mouse51

Offline verification passed. Manual gameplay acceptance is pending.
MouseTweaks DLL SHA-256: 77425954246FCD765070911FAF6135C7E60A4A5E4946FC84C5A1FDC88C461431
ModMenu DLL SHA-256: 2039530A342CA2E44D8F7A3AC7DB85BB66546E2BBB720EF9CE14392B13005482

## Install / 安裝

Close Minecraft normally and back up both existing plugin folders. Extract both
update ZIPs to the selected instance's mods folder. Keep existing JSON settings
and logs; neither update archive contains a default configuration or game files.
Use one loaded copy of each DLL. No LeviLamina reinstallation is required.

If MouseTweaks was disabled as manifest.json.close, keep only one active
manifest.json after updating. Do not leave duplicate loadable manifests or
change either game EXE.

正常退出遊戲並備份兩個插件資料夾，把兩個更新包解壓至同一遊戲實例的 mods。
保留自己的 JSON 設定與日誌。若 MouseTweaks 是 manifest.json.close，先恢復一份
啟用的 manifest.json，再套用更新；避免重複描述檔及 DLL。無須重裝 LeviLamina。

## Focused test / 人工測試

1. Start 26.51 without LeviLamina. Confirm MouseTweaks NATIVE READY and ModMenu
   native input Ready in this launch's logs.
2. Check main-menu and pause-menu Mods buttons, configured shortcut and Escape.
3. In a container, Shift-left drag across mixed item types. Check left collection,
   right placement and wheel transfer. Check normal input after closing the UI.
4. Exit normally and repeat a second launch. Include console disabled if you use
   that launcher option. If a problem occurs, retain both logs from that run.
5. MouseTweaks supports exactly 26.45 / 26.51 / 26.52; regression-test the other
   two versions as needed for this newly built DLL. ModMenu also supports 26.60,
   but this update does not add 26.60 support to MouseTweaks.

先確認兩個插件 Ready，再檢查主／暫停選單、快捷鍵、Shift＋左鍵混合搬運、
左鍵收集、右鍵放置、滾輪及關閉介面後的正常輸入。完全退出後第二次啟動，
有使用主控台選項時包含一次關閉主控台。出錯請保留這次兩份新日誌。
本次 DLL 尚待遊戲內驗收；先前版本的人工結果不能替代此次測試。

The inspected 26.51 session reached Ready in both plugins and recorded a Mods
open event with zero ModMenu faults. This is startup/menu evidence, not completion
of the full container gesture and second-launch acceptance steps above.
