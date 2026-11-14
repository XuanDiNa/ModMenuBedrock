# Edition layout / 版本目錄

Both distributions live on `main`, with independent source and build roots:

```text
README.md                    # shared download and documentation entry
editions/vanilla/             # ModMenuBedrock, LeviLauncher native route
editions/levilamina/          # ModMenuBedrockLL, official LL native client route
tools/verify_merge_snapshot.py
```

Build from the selected edition directory using its own BUILDING.md and
tools/verify.ps1. There is deliberately no combined runtime target at repository
root. Configurations, dependency setup and output directories stay scoped to
that edition. The root directory is a documentation entry point.

| Snapshot | Original commit | Release |
| --- | --- | --- |
| `editions/vanilla` | `4c23aa29f7f75c91c0b6064ec18065e150bf98ab` | `v0.6.0-test5-leftalt`, with the subsequent public documentation index |
| `editions/levilamina` | `9679aa91246396ba10198eb8599f1ba33d73a306` | `v0.1.0-test2-ll26.60` |

The merge retains both parent histories. Each subtree is identical to its
original complete Git tree, including runtime sources, build files and editor
contracts. No DLL is rebuilt or repackaged. Existing tag-based source archives
keep their original root layout; new checkouts use the edition directories.
Edition READMEs and historic audit documents are preserved as snapshots and may
refer to the former standalone branches or tag layouts. Use this root README
for the current repository paths.

Run the merge-only integrity audit with a normal full Git checkout:

```powershell
py -3 tools/verify_merge_snapshot.py
```

It compares the two subtrees with their recorded source commits and verifies
versions, independent entry DLLs, local links in the shared docs and publication
exclusions. Before committing, pass `--tree <git-write-tree-id>` to check the
staged tree. This is a frozen merge audit; future deliberate source changes
will require a new baseline rather than silently passing this historic check.
It does not run Minecraft or establish new gameplay acceptance.

## 繁體中文

兩個版本已在同一個 `main`，各自以 `editions/vanilla`、`editions/levilamina`
作為原始碼與編譯根目錄。根目錄提供統一首頁，編譯時請進入所選版本並依該版本的
BUILDING.md 與驗證腳本操作。相依、設定與輸出仍各自獨立。

這次合併保留雙方 Git 歷史；每個子目錄與表列原提交的完整目錄樹完全相同。
沒有重新編譯 DLL，也沒有替換或重打包既有 Release。原標籤下載的原始碼維持
當時的根目錄形式，新版倉庫則使用版本子目錄。

版本內部的 README 與歷史稽核文件保持原快照，因此可能仍提及先前分支或舊標籤
目錄；目前的倉庫入口以根目錄 README 為準。

上方命令只驗證這次搬移與合併，確認兩份完整 Git 樹、版本、獨立入口及共用文件
連結。未來若刻意修改原始碼，需要新的基準，不能把本次歷史檢查當成新版本驗收。
本次不啟動遊戲，也不新增人工實機通過的聲明。
