# UI resource layer diagnostic

Version: **0.6.0-test6-layer-diag1**. This is a diagnostic candidate, not a confirmed fix.

The reported 1.26.52.03 log reaches native Ready, but both menu resources contain two layers. The existing single-layer policy refuses them before inserting Mods. No F10 press was observed in that log. Whether enabling the launcher console changes this condition remains unconfirmed.

This candidate retains all three exact game profiles, hooks, input protections, parser ownership and transform permissions. It records at most eight definition loads, eight layers per load, and reads at most two MiB of resource text per load. Only numeric metadata, format classifications and equality flags are published to the worker log; resource text and pointers are not logged or retained. No game executable or other plugin is changed.

## Test

1. Exit Minecraft normally. Preserve the current ModMenu folder and log. Replace only the supplied DLL, `manifest.json` and `mod.json`; retain your existing `config.json` and other plugins.
2. Start the affected game with **Open game console disabled**, keeping the same plugins and resource packs. Wait ten seconds at the main menu. Record whether the Mods button appears, then press the configured Mods shortcut once (F10 by default) and record whether it opens.
3. If it remains unavailable, exit normally and repeat once with **Open game console enabled**, changing nothing else. The log appends both sessions.
4. Return `ModMenuBedrock.log` and the two observations separately: button / shortcut, console off / on. Also identify any global resource packs that alter the main or pause menu. Do not change packs between these two runs.

The original symptom may remain: the new `definition-probe` records distinguish empty placeholder layers from populated compatible or unsupported overrides. These results determine the next fix. Do not substitute this build for the accepted public release permanently.

## 中文

這是**診斷版，尚未確認修復**。玩家日誌已通過 1.26.52.03 原生驗證，但兩層選單資源被現有單層限制擋下；日誌也未收到 F10 按下紀錄。目前不能斷定與啟動器控制台存在因果關係。

只更新 ModMenu 的 DLL、`manifest.json`、`mod.json`，保留原有設定、日誌及其他插件。先正常退出遊戲並備份原 ModMenu。

1. 保持相同插件與資源包，**不勾選「開啟遊戲控制台」**啟動。在主選單等十秒，分別記錄 Mods 按鈕是否出現、按一次目前設定的快捷鍵（預設 F10）是否開啟。
2. 若仍不能用，正常退出後，只改成**勾選控制台**，再做同一次測試。兩次之間不要更動資源包或其他設定。
3. 回傳追加了兩次啟動紀錄的 `ModMenuBedrock.log`，以及「按鈕／快捷鍵」各自結果；若有修改主選單或暫停選單的全域資源包，也請提供名稱。

新增紀錄只含有界的層數、大小、格式分類及相等標記，不寫入資源內容或記憶體位址。原有版本、場景、前景、輸入與 UI 合併保護均保留。此版可能仍沒有按鈕；用途是取得舊日誌缺少的層資訊，再據此修正。
