# Left Alt binding extension

The user confirmed Left Alt plus another key and authorized public binding and existing consumer parsing/trigger changes through a testable handoff. Execute inline. Preserve frozen test4, installed files and unrelated work.

Design: SDK contract 2 adds bit 4 (LeftAlt) and canonical CTRL+SHIFT+LALT+KEY order. Add explicit settings v5; frozen v4 remains Ctrl/Shift only, including readable CTRL+LALT primary-key bindings. LeftAlt is side-specific; RightAlt/AltGr never satisfies it. LeftAlt-only capture waits for release. Exact matching excludes the primary modifier itself. Existing foreground/scene/focus/release barriers remain. Raw input appends read-only LeftAlt metadata from the existing Windows sampler, no new hooks or native addresses. Audit that exact diff.

- [x] Add failing parser, capture, lifecycle, schema compatibility and real-menu tests.
- [x] Implement SDK, opt-in v5 and menu capture/input metadata.
- [x] Update seven scoped consumers and focused config/trigger tests.
- [x] Run Debug/Release suites, schema/native boundaries and consumer checks; review inline.
- [x] Freeze test5-leftalt DLLs, SDK/source and bilingual manual test steps with hashes/sizes. No installation or publication.
