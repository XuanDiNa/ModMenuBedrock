# Building / 編譯

## English

Requires Windows x64, Visual Studio 2022 C++ tools with the Windows SDK and MASM,
[xmake](https://xmake.io/), and Python 3 for verification. xmake obtains the pinned
MinHook 1.3.4 dependency; nlohmann/json 3.11.3 is vendored with its license.
No LeviLamina SDK, Minecraft SDK, IDA installation or sibling repository is needed.

```powershell
git clone https://github.com/XuanDiNa/ModMenuBedrock.git
cd ModMenuBedrock
xmake f -P . -y -m release
xmake build -P . -y -j 2 ModMenuBedrock
```

Output: `build/windows/x64/release/ModMenuBedrock.dll`.

For a fresh Debug/Release build and all 14 public suites:

```powershell
py -3 -m pip install -r tools/requirements.txt
powershell -NoProfile -ExecutionPolicy Bypass -File tools/verify.ps1
```

This runs synthetic/native-policy, settings/storage, UI, slider, hook-chain,
pointer-bridge and unsupported-host tests plus the v1/v2 schema checks. It never
starts Minecraft. Results go to the ignored `artifacts/verification.json`.

Private image tests can be included with `-GameImagesRoot <local-directory>`.
That directory must contain one subfolder per exact game version, each with
`Minecraft.Windows.exe.orig` and `Minecraft.Windows.exe`. Tests read the files and
use private copy-on-write mappings; they never write the executables. Optional
`-MenuFixtures <local-json-file>` adds exact vanilla-menu fixtures extracted by
the developer from a lawful local install. Without those arguments the tests
explicitly report those checks as skipped. No game files or extracted fixtures
are included here.

The private development gate also has two cross-project companion-parser suites.
Those are not part of this standalone checkout; their companion source excerpts
are not redistributed. Existing public UI/key/settings tests remain included.

Runtime `src/` and `include/` match the accepted DLL's source snapshot. The public
build layout and two test entry points were made portable. Build environment,
paths and toolchain can change binary hashes: a clean rebuild is not the
maintainer-tested Release artifact. See [provenance](docs/verification.md).

## 繁體中文

需要 Windows x64、Visual Studio 2022 C++ 工具（含 Windows SDK 與 MASM）、
[xmake](https://xmake.io/)；驗證另需 Python 3。xmake 會取得固定版本 MinHook 1.3.4，
專案已附 nlohmann/json 3.11.3 及授權。不需要 LeviLamina／Minecraft SDK、IDA
或其他相鄰專案。

上方第一組命令先取得原始碼再編譯 Release，DLL 位於 `build/windows/x64/release/`。
第二組命令會執行完整 Debug／Release 編譯、每模式 14 組公開測試與 v1／v2 Schema
驗證，不啟動遊戲；結果放在 Git 忽略的 `artifacts/verification.json`。

可選 `-GameImagesRoot` 指定本機遊戲目錄，內含各精確版本資料夾及原始／啟動器 EXE；
只讀取並使用私有寫入時複製映射，不會改寫 EXE。`-MenuFixtures` 可指定開發者自行從
合法本機安裝提取的原版選單測試資料。不提供參數時會明確顯示略過相應檢查；
公開倉庫不包含遊戲檔或提取資料。

內部另有兩組跨專案解析器測試，並未將其他插件的原始碼片段一併公開；本倉庫保留
ModMenu 本身的介面、按鍵及設定測試。27 個執行期原始檔保持與已測快照一致，
僅公開建置配置與兩個測試入口改為可攜形式。重新編譯的雜湊可能不同，不代表已通過
人工遊戲測試；Release 使用的仍是維護者實測檔案，詳見[來源紀錄](docs/verification.md)。
