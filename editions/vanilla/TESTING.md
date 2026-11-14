# Testing / 測試

## English

On 2026-11-11 the maintainer reported the latest ModMenu compatibility package
working on **1.26.45.01 and 1.26.51.01**. Its DLL SHA-256 is
`FD314F0EFC6397D1229B2F18A62DCEC5B8C404829E3A13A28B14F137C4BF2F74`.
This is basic manual acceptance, not a recorded per-step benchmark or exhaustive
acceptance of every environment. The exact 1.26.52.03 native profile remains
supported and was included in the offline checks.

Debug and Release each passed 22 suites, including 94 exact original/launcher
image checks, 431 binding checks, pointer/focus/Workspace and real C ABI tests.
Schema/parser agreement covered 3,525 cases. Private vanilla-menu fixtures were
unavailable and skipped; synthetic definitions were tested. No test launched Minecraft.

For focused regression checks:

1. Check Mods / F10 on the main and pause menus, including a normal cold restart.
2. Check pointer clicks, scrolling, Apply/Cancel and Escape navigation.
3. In ModMenu's own shortcut field, try Ctrl+K, Shift+K and Left Alt+K. Apply,
   reopen, check persistence and test the selected shortcut from the main/pause
   menu. Restore your preferred shortcut afterward.
4. Switch briefly to another app and return. Confirm the permitted page remains
   and a held key or stale click does not activate anything.

Plugins interpret and hot reload their own settings; their chord actions are
outside this ModMenu-only acceptance. Long sessions, every GUI scale, OreUI,
all providers and arbitrary custom menus are not exhaustively verified.
Do not bypass INACTIVE refusals; report the version and log reason.
See [build checks](BUILDING.md) and [installation](docs/releases/package-installation.md).

## 繁體中文

2026-11-11 維護者回報最新 ModMenu 相容套件在 **1.26.45.01、1.26.51.01** 使用正常。
DLL SHA-256 為 `FD314F0EFC6397D1229B2F18A62DCEC5B8C404829E3A13A28B14F137C4BF2F74`。
這是基本人工驗收，不自行推定每一步均有個別紀錄，也不是效能數據或全部環境的完整驗證。
1.26.52.03 的精確原生配置保留支援，亦已納入本輪離線檢查。

Debug／Release 每模式各通過 22 組測試，包括 94 項原始／啟動器映像、431 項綁鍵、
滑鼠、焦點、Workspace 與實際跨 DLL C ABI；Schema／解析器對照有 3,525 個案例。
私有原版選單資料無法取得，已略過，合成選單測試正常執行；自動測試沒有啟動遊戲。

安裝後可做精簡回歸：

1. 主／暫停選單 Mods、F10，並正常退出後冷啟動一次。
2. 滑鼠、捲動、Apply／Cancel 及 Escape 返回。
3. ModMenu 自己的快捷鍵依次試 Ctrl+K、Shift+K、左 Alt+K，Apply 後重開確認保存，
   在主／暫停選單測試選定組合鍵，之後恢復偏好的快捷鍵。
4. 暫切其他程式再回來，確認允許的頁面保留，按住的鍵及舊點擊不會造成誤觸。

各插件自行解析及熱更新設定，其組合鍵功能不屬本輪 ModMenu 驗收。
長時間使用、所有 GUI 比例、OreUI、全部提供者及任意改版選單尚未全面驗證。
遇到 INACTIVE 應回報版本及日誌原因，不要取消保護。
詳見[建置檢查](BUILDING.md)及[安裝](docs/releases/package-installation.md)。
