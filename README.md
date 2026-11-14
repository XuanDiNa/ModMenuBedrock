# ModMenu

[English](#english) · [繁體中文](#繁體中文)

## English

A native plugin menu for Minecraft Bedrock on Windows x64, inspired by
[Java Mod Menu](https://github.com/TerraformersMC/ModMenu).
Browse loaded plugins, edit supported settings and open plugin workspaces.

Choose the edition matching your loader. Each has its own source directory,
DLL, settings and installation package. Enable one edition at a time.

| Edition | Supported environment | Download | Source and build guide |
| --- | --- | --- | --- |
| LeviLauncher native, vanilla route | Minecraft 1.26.45.01 / 1.26.51.01 / 1.26.52.03 / 1.26.60.05 | [ModMenu 0.6.0-test10](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.6.0-test10) | [editions/vanilla](editions/vanilla/README.md) · [Build](editions/vanilla/BUILDING.md) |
| LeviLamina native client | Minecraft 1.26.60.05 + LeviLamina 26.60.7 | [0.1.0-test2-ll26.60](https://github.com/XuanDiNa/ModMenuBedrock/releases/tag/v0.1.0-test2-ll26.60) | [editions/levilamina](editions/levilamina/README.md) · [Build](editions/levilamina/BUILDING.md) |

### Features

- Mods entry in supported main and pause menus, with F10 as the default shortcut.
- Search, plugin details, shared Configure controls and Apply/Cancel drafts.
- Optional sliders, UTF-8 text fields and asynchronous plugin workspaces.
- Ctrl / Shift / Left Alt chords and Esc-to-Unbound capture for compatible plugins.
- English, Traditional Chinese and Simplified Chinese, following the game language.
- Resource-pack entry API in both current releases: pack authors can render and position
  their own Mods button using `button.modmenu.open`.
- Exact game-image, native hook ownership, scene and input checks.

For existing installations, use the matching `update` ZIP to retain `config.json`;
first installations can use the full `windows-x64` ZIP. Exit Minecraft before
updating. Each plugin remains responsible for hot reloading settings and
implementing its own bindings. Workspace providers with binary pins need an
explicitly compatible version.

[Vanilla test10 changes](docs/releases/2026-11-14-vanilla-test10.md) cover the
resource-pack API, 26.60 native adapter, 26.51 without PreLoader and admission
for MouseTweaks 0.3.3-test1-vanilla51. A paired native 26.51 session reached Ready
and opened Mods; full gesture and restart acceptance remains pending.
The separate LL test2 sources and assets are unchanged; its affected-player and
music-plugin acceptance is still pending. [Earlier LL changes](docs/releases/2026-11-14-cumulative.md).

Developer entry points: [repository layout and verification](docs/edition-layout.md),
[vanilla integration](editions/vanilla/integration/README.md),
[LL integration](editions/levilamina/integration/README.md), and
[native resource-pack buttons](editions/vanilla/integration/RESOURCE_PACKS.md).
Run build commands from the selected edition directory.

## 繁體中文

參考 [Java Mod Menu](https://github.com/TerraformersMC/ModMenu) 製作的
Windows x64 基岩版原生插件選單，可瀏覽已載入插件、調整支援的設定及開啟工作頁。

依啟動方式選擇上表版本。原版路線位於 `editions/vanilla`，獨立 LeviLamina 版位於
`editions/levilamina`；兩者各自保留原始碼、DLL、設定與安裝包，只啟用其中一個。

### 功能

- 在受支援的主選單與暫停選單提供 Mods 入口，預設快捷鍵為 F10。
- 插件搜尋、詳細資料、共用 Configure 控件及 Apply／Cancel 草稿。
- 選用滑動條、UTF-8 文字欄位與非同步插件工作頁。
- 相容插件支援 Ctrl／Shift／左 Alt 組合鍵，綁定時按 Esc 解除綁定。
- 跟隨遊戲語言提供英文、繁體中文與簡體中文。
- 兩個目前發佈版皆提供材質包入口接口，可透過 `button.modmenu.open` 自訂 Mods 按鈕外觀與位置。
- 精確遊戲映像、原生掛鉤擁有者、場景與輸入檢查。

已有安裝時使用對應版本的 `update` ZIP 保留 `config.json`；首次安裝可用完整 `windows-x64`
ZIP，更新前完全退出 Minecraft。各插件自行負責設定熱更新與快捷鍵觸發，限定
ModMenu 二進位版本的 Workspace 提供者仍需相容版本。

[原版 test10 更新紀錄](docs/releases/2026-11-14-vanilla-test10.md)包含材質包接口、
26.60 原版適配、26.51 不含 PreLoader 的映像與 MouseTweaks 新版配對。
已觀察到配套 26.51 就緒及 Mods 開啟；完整拖曳與重新啟動驗收仍待完成。
獨立 LL test2 的原始碼與附件保持不變，其玩家及音樂插件共同驗收仍待完成。
[先前 LL 更新紀錄](docs/releases/2026-11-14-cumulative.md)。

開發請先選定版本目錄，再依各自的編譯與整合文件操作；詳見上方開發入口及
[倉庫結構說明](docs/edition-layout.md)。

ModMenu sources use the [MIT license](LICENSE). Each edition retains its
dependency notices and licenses.
