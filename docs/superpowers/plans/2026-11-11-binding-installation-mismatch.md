# Binding installation mismatch follow-up

The user reports all plugins cannot use two-key combinations. Read-only local
attribution confirms ModMenu test5-leftalt is installed, while consumers retain
legacy DLLs and key descriptors. Those controls finish on a single key; they
cannot safely be changed to binding controls without matching consumers.
Installed Muonium is newer 0.7.0-test1, so the older 0.6.x candidate must not
replace it. Preserve the exact frozen game/native/source boundaries.

- [x] Attribute installed DLLs, manifests, descriptions and recent menu log.
- [x] Rebase only binding parsing/trigger changes onto frozen player 0.7.0-test1
  source in isolation; test config/capture/reload and retain native bytes.
- [x] Verify a flat mods bundle through actual ModMenu Load/Apply/capture paths
  for each updated consumer; check legacy entries remain single-key.
- [x] Freeze one versioned complete 26.52 update ZIP with no user configs,
  exact hashes/sizes, read-only installation checker and concise instructions.
  Reuse the already-verified ModMenu DLL; do not install, launch or publish.

Results: 898 package checks cover fourteen activation fields and seven modifier
masks. Player Debug/Release builds and their existing suites pass; the exact
native/Java source base is preserved. The read-only installation checker passes
the matching payload and rejects the installed legacy consumers, missing
sidecars, a same-size corrupt DLL, and duplicate loader copies. Archive entries,
sizes and hashes are verified. Game acceptance remains pending the user's test.
