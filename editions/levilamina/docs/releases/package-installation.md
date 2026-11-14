# Installation / 安裝

## English

This release contains ModMenuBedrock only. One DLL supports the exact builds
1.26.45.01 / 1.26.51.01 / 1.26.52.03 on Windows x64 / LeviLauncher.

1. Close Minecraft and back up ModMenuBedrock outside the active mods folder.
2. Merge the archive's ModMenuBedrock folder into the existing installation.
   Keep config.json, logs and unrelated files. Do not load two copies of the DLL.
3. Preserve enabled/disabled state: keep only the launcher-managed manifest.json
   or manifest.json.close, not both.
4. Start through LeviLauncher. Use Mods in the main/pause menu, or F10 unless
   you have changed or cleared that shortcut.

Click a binding field, hold Ctrl/Shift/Left Alt, then press the main key. Apply
saves, Cancel discards, and Esc during capture selects Unbound. Standalone
modifiers are confirmed on release. Mouse buttons cannot be bound. Other plugins
must support the saved binding format themselves.

The ZIP contains no default config.json or other plugin update. Restore your
backup to roll back. See [testing](../../TESTING.md).

## 繁體中文

本次只包含 ModMenuBedrock。Windows x64／LeviLauncher 的三個精確版本
1.26.45.01／1.26.51.01／1.26.52.03 共用一份 DLL。

1. 完全退出 Minecraft，將 ModMenuBedrock 備份到載入中的 mods 以外。
2. 將壓縮包的 ModMenuBedrock 合併進既有安裝，保留 config.json、日誌及其他檔案，
   不要載入兩份 DLL。
3. 維持啟用／停用狀態，只保留啟動器管理的 manifest.json 或 manifest.json.close。
4. 從 LeviLauncher 啟動，在主／暫停選單點 Mods；未改動或解除快捷鍵時可用 F10。

組合鍵先點欄位、按住 Ctrl／Shift／左 Alt，再按主鍵；Apply 儲存、Cancel 捨棄，
綁定期間 Esc 設為無綁定。單獨修飾鍵放開確認，滑鼠按鍵不可綁定。
其他插件需要自行支援儲存的綁鍵格式。

本包不含預設 config.json 或其他插件更新，需要回退時還原備份。詳見[測試](../../TESTING.md)。
