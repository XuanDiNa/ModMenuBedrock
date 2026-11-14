# Release verification / 發佈驗證

## English

**Version:** 0.5.0-test1, Windows x64, prerelease.

**Exact builds:** 1.26.45.01 / 1.26.51.01 / 1.26.52.03.

The maintainer reported normal basic use on all three builds on 2026-10-29.
The Release DLL is the retained, manually tested binary:

```text
SHA256 243C3F73B21F2EFF939116D7395E2CBEC39792F62C9DCDAD64DE36A59105322B
Size   836608 bytes
```

All 27 runtime files in `src/` and `include/` match the source hashes recorded
by that binary's development verification. The public xmake configuration is
self-contained; two test entry points accept optional local paths instead of
depending on a private workspace. No runtime source or native guard was changed
for publication. The complete public runtime source hash list is in
[runtime-source-sha256.txt](runtime-source-sha256.txt).

The public gate clean-builds Debug and Release, runs 14 suites per mode, validates
both settings-description schemas, and checks the DLL's AMD64 PE, exports and
imports. The publication run additionally uses six local read-only game images
and six local vanilla-menu fixtures. Game files and fixtures are not uploaded.
The internal development gate had two additional cross-project parser suites;
their companion-source excerpts are not included in the standalone public tree.

A public-source rebuild is verification evidence, not an automatically accepted
replacement DLL. Its hash is recorded locally by `tools/verify.ps1`. Releases
retain the hash above. The public ZIP is repackaged with MIT licensing and current
bilingual documentation; its DLL is unchanged. ZIP/payload checksums and downloaded
GitHub assets are checked separately. No live install is changed by publication.

Unknown images, altered entries and incompatible shared-hook owners fail closed.
The three version profiles remain explicit, with independently reviewed layouts;
no runtime scan or guessed address delta was added. See
[compatibility](compatibility-0.4.1-test1.md).

## 繁體中文

本次為 **0.5.0-test1 預發佈版**，維護者於 2026-10-29 回報三個精確版本基本使用正常。
Release 保留上方雜湊及大小的原始實測 DLL，不以公開原始碼的新編譯檔取代。

27 個執行期原始檔均符合實測檔的建置來源紀錄。只調整公開建置配置及兩個測試入口的
可攜性，未改功能程式或原生保護。公開驗證會全新編譯 Debug／Release、每模式執行
14 組測試、檢查 Schema、PE／匯出／匯入；發佈時另使用本機六個只讀遊戲映像及
六份選單測試資料，不上傳這些資料。內部另外兩組跨插件解析器測試不含在公開倉庫。

公開 ZIP 更新了 MIT 授權與中英文文件，DLL 保持不變。重新編譯僅是離線驗證，
不能視為已測替代品；打包、校驗碼與 GitHub 下載另行核對，發佈不變更現有安裝。
三版本精確配置、入口與共用 Hook 檢查全部保留，沒有新增掃描、猜測偏移或取消保護。
