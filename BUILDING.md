# Build the independent LL edition

Requires Windows x64, Visual Studio C++/Windows SDK, LLVM clang-cl, xmake 3.0+ and Python 3.

The default xmake path follows the official mod template: LeviLamina 26.60.7 client package, LeviBuildScript 0.6.1 link rule, MD runtime, LL_REGISTER_MOD and real shared allocator overrides.

```powershell
xmake f -y -m release --target_type=client
xmake build -y ModMenuBedrockLL
```

If clang-cl does not inherit the Windows SDK link paths, use `tools/with_toolchain.ps1 -LlvmBin <LLVM-bin> build -y ModMenuBedrockLL`; it imports VS toolchain variables only into its own process.

## Official binary/header SDK alternative

Rebuilding the complete LL runtime is unnecessary for this plugin. The delivered candidate uses an import library generated from the **unmodified official client release DLL** and the exact official v26.60.7 headers. It does not replace the installed runtime. No ABI declarations are handwritten.

Clone https://github.com/LiteLDev/LeviLamina at v26.60.7 (commit dacca851fe5f8b97e292496f702db3a73b118f39), download `levilamina-v26.60.7-client-release-windows-x64.zip` from that official release, then run:

```powershell
py tools/prepare_official_sdk.py --source <checkout> --runtime-zip <official-release.zip> --lib-exe <MSVC-x64-lib.exe> --output <sdk-directory>
xmake f -y -m release --target_type=client --ll_sdk_root=<absolute-sdk-directory>
xmake build -y ModMenuBedrockLL
```

The script verifies source commit/header cleanliness and exact official DLL hash, copies common/client headers, generates a typed PE export import library, and records provenance. The same package versions as the official SDK are used. Public dependencies are fetched by xmake. Nothing is installed into Minecraft.

Run `tools/verify.ps1` for independent LL checks; pass the private Instances root for exact-image checks. Never package game EXEs, vanilla payload fixtures, installed settings, credentials, logs, IDA databases, toolchains, or the dependency SDK with the mod.
