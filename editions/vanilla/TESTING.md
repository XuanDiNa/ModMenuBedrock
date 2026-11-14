# ModMenu 0.6.0-test11 — Manual test / 人工測試

Windows x64 / LeviLauncher native DLL.
Minecraft 1.26.45.01 / 1.26.51.01 / 1.26.52.03 / 1.26.60.05.
Offline checks do not establish gameplay acceptance for this newly built DLL.

## Install / 安裝

Exit Minecraft normally and back up the existing ModMenuBedrock folder.
Extract the update ZIP into the selected instance's mods directory. It contains
no config.json: retain existing settings and logs. Enable one ModMenu edition
and one DLL copy. Do not edit the game EXE or reinstall LeviLamina.
The separate LeviLamina edition is not changed by this update.

正常退出遊戲並備份 ModMenuBedrock；更新包解壓到所選實例的 mods。
保留設定與日誌，只載入一份 DLL。更新包不含 config.json。
這是原版路線更新，獨立 LeviLamina 版沒有變動。

## Focused 26.60 test / 本次重點

1. Launch 1.26.60.05 with test11. Confirm this launch's ModMenu log reports
   0.6.0-test11 and no INACTIVE: disk-identity.
2. In the main menu check the Mods entry and the configured shortcut (F10 by
   default). Open Configure, then return with Escape/Done.
3. Enter a world and test Mods in the pause menu. Return to normal play and
   confirm mouse and keyboard input. Settings, inventory and HUD must not open Mods.
4. Exit fully and repeat a second launch. If using the launcher console option,
   check the other console setting too; do not manually alter PE fields.
5. If your resource pack changes the menu, it must supply button.modmenu.open;
   automatic 26.60 insertion remains limited to exact single-layer vanilla menus.

先測 26.60 本次新 DLL：日誌版本、主選單入口、F10、Configure、返回、暫停選單，
再完全退出重啟一次。使用啟動器控制台選項時，可切換該選項驗證另一種啟動；
無需手動修改 EXE。修改選單的材質包須接入 button.modmenu.open，這次沒有放寬
自動注入條件。若出錯，保留這輪 ModMenu 日誌、遊戲版本與控制台選項狀態。

## Companion scope / 配套範圍

The optional MouseTweaks 0.3.3-test1-vanilla51 asset supports 26.45/51/52 only,
not 26.60. On 26.51, it is the exact reviewed companion for shared hooks;
older incompatible loaded DLLs may prevent safe initialization. Test ModMenu
alone when a companion does not support the selected game version.

選用 MouseTweaks 配套不支援 26.60。26.51 可沿用已核實的配套新版。
不支援該遊戲版本的其他 DLL 請先停用，以單獨驗收 ModMenu。

DLL: **988,160 bytes**; SHA-256
`17CB9D3505B6B17942A2975CD7E5F670A277B4B42D61E40042F2C3AA781DAF6E`.
