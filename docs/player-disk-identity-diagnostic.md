# ModMenuLL disk-identity feedback

The supplied ModMenuBedrockLL 0.1.0-test1 log contains three failed loads with
`INACTIVE: disk-identity`. This means the exact on-disk image check rejected the
game before any ModMenu UI hooks were installed. The log does not contain the
observed EXE hash, so the particular difference is not established. Do not add
an unknown hash or weaken the image check to work around this report.

The supplied LeviLamina logs dated November 13 and 14 each contain 4,096 bytes
and end mid-line. Their visible startup sequences list arke only. They do not
identify the same processes as the ModMenu log, establish a ModMenu/arke
conflict, or supply the missing executable identity.

## Player collection / 玩家收集步驟

1. Extract `ModMenuLL-Diagnostics-1.zip` into a writable folder.
2. Keep the affected Minecraft instance running if possible, then double-click
   `Collect-ModMenuLL-Diagnostics.cmd`. No administrator access is required.
3. Select the **actual Minecraft.Windows.exe used by that instance** in the
   file dialog. Alternatively, drag that EXE onto the CMD file. This only selects
   it for reading; it never launches Minecraft.
4. Return the generated `ModMenuLL-report-*.json`. Also provide the **complete**
   LeviLamina log and ModMenuBedrockLL.log from the same launch. Use the files
   directly rather than a truncated text preview. No EXE upload is needed.

1. 將診斷 ZIP 解壓到可寫入的資料夾。
2. 建議保持出問題的遊戲實例正在執行，雙擊 CMD，無需管理員權限。
3. 在選擇視窗中選取該實例實際使用的 `Minecraft.Windows.exe`；也可把 EXE
   拖到 CMD 上。工具只讀取檔案，不會啟動遊戲。
4. 回傳同目錄生成的 `ModMenuLL-report-*.json`，以及同次啟動的完整
   LeviLamina 日誌和 ModMenuBedrockLL.log。請傳送檔案本身，避免文字預覽截斷。

The collector reads full-file SHA-256, bounded PE metadata and section hashes,
the manifests of three known plugin paths, and (when accessible) matching
Minecraft process module identities. Absolute paths are redacted. The report
does not include logs, game content, settings, credentials or memory dumps.
DLL hashes describe their on-disk files; they do not prove unchanged loaded
memory. No networking, installation, injection or guard changes occur.

PowerShell 5.1 or newer is required. If collection fails, return its error text.
