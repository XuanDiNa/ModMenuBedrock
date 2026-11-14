# Edition layout / 版本目錄

Both distributions live on `main`, with independent source and build roots:

```text
README.md                    # shared download and documentation entry
editions/vanilla/             # ModMenuBedrock, LeviLauncher native route
editions/levilamina/          # ModMenuBedrockLL, official LL native client route
tools/verify_merge_snapshot.py
tools/verify_release_snapshot.py
```

Build from the selected edition directory using its own BUILDING.md and
tools/verify.ps1. There is deliberately no combined runtime target at repository
root. Configurations, dependency setup and output directories stay scoped to
that edition. The root directory is a documentation entry point.

| Snapshot | Original commit | Release |
| --- | --- | --- |
| `editions/vanilla` | [test11 source manifest](../docs/vanilla-test11-snapshot.json) | `v0.6.0-test11`; fixes the withdrawn test10 release |
| `editions/levilamina` | `9679aa91246396ba10198eb8599f1ba33d73a306` | `v0.1.0-test2-ll26.60` |

Both parent histories remain. Native test11 adds the exact 26.60 checksum-order images and publishes a new DLL;
LL remains byte-identical to its
recorded complete source commit. Old tags retain their original layouts; the test10 Release is withdrawn. Historical audit documents describe their own frozen versions.

Run the current integrity audit with a normal full Git checkout:

```powershell
py -3 tools/verify_release_snapshot.py
```

It compares the native subtree with its published tree manifest, checks LL
against its source commit and verifies independent entry DLLs, links and
publication exclusions. Before committing, use `--tree <git-write-tree-id>`.
Future deliberate source changes require a reviewed new baseline. The original
merge-only audit is preserved: `py -3 tools/verify_merge_snapshot.py --tree 28727e0`.
Neither command runs Minecraft or establishes gameplay acceptance.

## 繁體中文

兩個版本已在同一個 `main`，各自以 `editions/vanilla`、`editions/levilamina`
作為原始碼與編譯根目錄。根目錄提供統一首頁，編譯時請進入所選版本並依該版本的
BUILDING.md 與驗證腳本操作。相依、設定與輸出仍各自獨立。

保留雙方 Git 歷史；本次更新原版 test11 子目錄並提供新 DLL，LL 子目錄
仍與表列提交完全相同。撤下 test10 Release，舊標籤保留，新版倉庫使用版本子目錄。

歷史稽核文件只描述其當時快照；目前倉庫入口以根目錄 README 為準。

上方命令確認新原版來源樹、未更動的 LL 樹、版本、獨立入口及公開檔案。未來
修改原始碼仍需審核新基準。原合併稽核以明確的舊提交執行，不啟動遊戲。
