# Matched keyboard binding update

## English

This is an update for the existing Minecraft **1.26.52.03** installation. The
installed ModMenu already has the new binding UI, but the installed consumers
still have older DLLs and single-key descriptions. Update each DLL, manifest
and settings description together so capture and gameplay understand the same
Ctrl / Shift / Left Alt combination.

The bundle has a flat `mods` directory and contains no user configuration,
runtime, Java program or account data. ModMenu is the unchanged
`0.6.0-test5-leftalt` DLL. Muonium is `0.7.0-test2-leftalt`, a binding-only update
based on the exact installed `0.7.0-test1` source release; keep its existing
runtime and player files. Native guards and existing game support are retained.
This bundle does not add support for other game versions.

1. Exit Minecraft. Back up the eight existing plugin directories **outside the
   active mods directory**.
2. Merge the bundle's `mods` directory into the `1.26.52.03` version directory.
   Replace the matching DLLs, manifests and `modmenu.settings.json` files. Keep
   existing JSON settings, profiles, runtime and all other player files. Do not
   delete and replace whole plugin directories.
3. In PowerShell, run the read-only checker from the extracted bundle:
   `powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\CHECK-INSTALL.ps1 -ModsPath 'your version directory\mods'`.
   It checks DLL hashes/sizes, exact metadata and v5 descriptions. Expect
   `PASS: all 8 matching DLL/manifest/description sets verified`.
4. Restart Minecraft. In a plugin's Configure, click its activation key field,
   hold **Ctrl**, press **K**, release both, then Apply. Reopen to confirm
   `CTRL+K` remains. Repeat with **Shift+K** and **Left Alt+K**. In that
   plugin's allowed gameplay context, check that the full combination works
   and K alone does not. Test one plugin at a time to avoid shared shortcuts.

All seven combinations of Ctrl, Shift and Left Alt are supported. Hold the
modifier(s) before pressing the primary key. Escape during capture selects
Unbound; mouse buttons cannot be bound. The emitted jump key in AutoJumpReset
remains a single output key. ContainerPreview remains a hold action and retains
its original sneak/Shift guard for new background opens; use Ctrl+K or Left
Alt+K for that check. Windows-reserved shortcuts may be consumed by the OS.

The installed BedrockToolNative test5 accepts an exact older ModMenu test3 hash
and will reject this test5 DLL. Its companion admission update is outside this
bundle. Offline verification is complete; actual gameplay remains a manual test.

For rollback, exit the game and restore the backed-up DLL, manifest, description
and JSON together. Older runtimes cannot interpret newly saved combinations.

## 繁體中文

這是既有 **Minecraft 1.26.52.03** 安裝的配套更新。已安裝的 ModMenu 支援組合鍵，
但其他插件仍是舊 DLL 與單鍵描述檔；需要將 DLL、manifest 與設定描述檔一起更新，
讓綁定介面與實際觸發使用同一套 Ctrl／Shift／左 Alt 組合規格。

壓縮包直接提供 `mods` 目錄，不包含使用者設定、執行環境、Java 程式或帳戶資料。
ModMenu 沿用相同的 `0.6.0-test5-leftalt` DLL。Muonium 為 `0.7.0-test2-leftalt`，
以目前安裝的 `0.7.0-test1` 原始碼發行包為基礎，只更新快捷鍵；保留現有 runtime
與播放器檔案。原生保護及既有版本支援不變；此包不新增其他遊戲版本支援。

1. 完全退出遊戲，把目前八個插件目錄備份到 **mods 以外**。
2. 將此包的 `mods` 合併到 `1.26.52.03` 版本目錄，替換對應 DLL、manifest
   與 `modmenu.settings.json`。保留原有設定 JSON、profile、runtime 及播放器檔案，
   不要刪除整個插件目錄再放入。
3. 在解壓目錄執行上述 PowerShell 檢查指令，把 `-ModsPath` 改成實際的 mods 路徑。
   工具只讀取 DLL 與公開中繼資料；顯示八組全部 `PASS` 才代表配套一致。
4. 重啟遊戲，進入插件 Configure，點快捷鍵欄位，先按住 Ctrl，再按 K，
   放開兩鍵後套用；重開確認保留 `CTRL+K`。再分別測 Shift＋K、左 Alt＋K，
   並在插件允許的遊戲情境測試完整組合有效、單按 K 無效。逐一測插件，避免共用鍵衝突。

支援 Ctrl／Shift／左 Alt 的七種修飾鍵組合，先按修飾鍵再按主鍵。擷取時按 Esc
設為無綁定，滑鼠鍵不可綁。AutoJumpReset 的輸出跳躍鍵仍是單鍵。容器預覽維持
按住生效，新增背景開箱仍保留原有潛行／Shift 保護；該項請用 Ctrl＋K 或左 Alt＋K。
Windows 保留的系統快捷鍵可能由系統攔截。

目前安裝的 BedrockToolNative test5 只接受固定的 ModMenu test3 雜湊，會拒絕此
test5 DLL；本包未變更 companion 檢查。離線驗證已完成，遊戲內效果仍待人工驗收。

回退時先退出遊戲，一起還原備份的 DLL、manifest、描述檔與 JSON；舊版不能解析
新版儲存的組合鍵。
