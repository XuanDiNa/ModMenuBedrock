"""Freeze only this independent mod's allowlisted files; never install or launch it."""
import argparse
import hashlib
import json
import shutil
import subprocess
import sys
import zipfile
from datetime import datetime
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
VERSION = "0.1.0-test2-ll26.60"
NAME = "ModMenuBedrockLL"
TOP = [".gitattributes", ".gitignore", "BUILDING.md", "README.md", "TESTING.md",
       "LICENSE", "THIRD_PARTY_NOTICES.txt", "LL-PROVENANCE.json", "xmake.lua",
       "manifest.json", "mod.json", "config.json", "config.schema.json"]
SOURCE_DIRS = ["src", "include", "tests", "tools", "integration", "vendor", "docs", "third_party"]
BAD = {".exe", ".dll", ".pdb", ".ilk", ".i64", ".id0", ".id1", ".id2",
       ".nam", ".til", ".zip", ".log", ".pyc"}


def require(ok, message):
    if not ok:
        raise RuntimeError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def describe(path):
    return {"name": path.name, "size": path.stat().st_size, "sha256": digest(path)}


def copy(source, target):
    require(source.is_file() and not source.is_symlink(), f"Unsafe source {source}")
    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(source, target)


def sums(root):
    files = sorted(p for p in root.rglob("*") if p.is_file())
    (root / "SHA256SUMS.txt").write_text(
        "".join(f"{digest(p)}  {p.relative_to(root).as_posix()}\n" for p in files), encoding="utf-8")


def archive(path, members):
    require(not path.exists(), f"Refuse overwrite: {path}")
    expected = {}
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED, compresslevel=9) as z:
        for file, name in sorted(members, key=lambda x: x[1]):
            require(name not in expected and ".." not in Path(name).parts, "Unsafe/duplicate archive member")
            expected[name] = digest(file)
            z.write(file, name)
    with zipfile.ZipFile(path) as z:
        require(len(z.namelist()) == len(expected) and set(z.namelist()) == set(expected), "Archive manifest differs")
        for name, sha in expected.items():
            require(hashlib.sha256(z.read(name)).hexdigest().upper() == sha, f"Archive mismatch: {name}")
    return len(expected)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", type=Path)
    args = parser.parse_args()
    manifest = json.loads((ROOT / "manifest.json").read_text())
    require(manifest == {"name": NAME, "entry": NAME + ".dll", "version": VERSION,
                         "type": "native", "platform": "client"}, "Unexpected native manifest")
    require(json.loads((ROOT / "mod.json").read_text())["version"] == VERSION, "Display version differs")
    dll = ROOT / "build/windows/x64/release" / (NAME + ".dll")
    verified = {}
    for mode in ["debug", "release"]:
        report = json.loads((ROOT / f"artifacts/verification-{mode}.json").read_text())
        require(report["success"] and report["suites"] == 28 and report["private_menu_fixtures"] and report.get("launcher_order_fixtures"), "Full verification including launcher-order images required")
        require(all(r["exit"] == 0 for r in report["results"]), "Failed suite")
        pe = json.loads(next(r["output"] for r in report["results"] if r["suite"] == "actual-DLL-PE"))["dll"]
        current = ROOT / f"build/windows/x64/{mode}" / (NAME + ".dll")
        require(pe["sha256"] == digest(current) and pe["size"] == current.stat().st_size, "Verified DLL changed")
        verified[mode] = {"suites": 28, "sha256": pe["sha256"], "size": pe["size"]}
    release_root = (ROOT / "releases").resolve()
    out = (args.output or release_root / f"{NAME}-{VERSION}-{datetime.now():%Y%m%d-%H%M%S}").resolve()
    require(out.parent == release_root and not out.exists(), "Output must be a new direct release directory")
    out.mkdir(parents=True)
    stage = out / "runtime"
    payload = stage / "mods" / NAME
    runtime = {
        "build/windows/x64/release/ModMenuBedrockLL.dll": NAME + ".dll",
        "manifest.json": "manifest.json", "mod.json": "mod.json",
        "config.json": "config.json", "config.schema.json": "config.schema.json",
        "README.md": "README.md", "TESTING.md": "TESTING.md", "LICENSE": "LICENSE",
        "THIRD_PARTY_NOTICES.txt": "THIRD_PARTY_NOTICES.txt",
        "vendor/MinHook-LICENSE.txt": "licenses/MinHook-LICENSE.txt",
        "vendor/json/LICENSE.MIT": "licenses/nlohmann-json-LICENSE.MIT",
        "third_party/LeviLamina/COPYING": "licenses/LeviLamina/COPYING",
        "third_party/LeviLamina/COPYING.LESSER": "licenses/LeviLamina/COPYING.LESSER",
        "docs/levilamina-provider-migration.md": "docs/levilamina-provider-migration.md",
    }
    for source, target in runtime.items():
        copy(ROOT / source, payload / target)
    sums(payload)
    require({p.relative_to(payload).as_posix() for p in payload.rglob("*") if p.is_file()}
            == set(runtime.values()) | {"SHA256SUMS.txt"}, "Runtime allowlist differs")
    runtime_zip = out / f"{NAME}-{VERSION}-windows-x64.zip"
    runtime_count = archive(runtime_zip, [(p, p.relative_to(stage).as_posix()) for p in payload.rglob("*") if p.is_file()])

    source_files = [ROOT / name for name in TOP]
    for directory in SOURCE_DIRS:
        for file in (ROOT / directory).rglob("*"):
            if not file.is_file() or "__pycache__" in file.parts:
                continue
            require(not file.is_symlink() and file.suffix.lower() not in BAD, f"Forbidden source artifact: {file}")
            require(file.stat().st_size <= 8 * 1024 * 1024, f"Unexpected source size: {file}")
            source_files.append(file)
    source_zip = out / f"{NAME}-{VERSION}-source.zip"
    source_count = archive(source_zip, [(p, NAME + "-source/" + p.relative_to(ROOT).as_posix()) for p in source_files])
    sdk_files = [p for p in source_files if p.is_relative_to(ROOT / "integration")]
    sdk_files += [ROOT / n for n in ["LICENSE", "THIRD_PARTY_NOTICES.txt", "docs/levilamina-provider-migration.md"]]
    sdk_zip = out / f"{NAME}-{VERSION}-integration-sdk.zip"
    sdk_count = archive(sdk_zip, [(p, NAME + "-sdk/" + p.relative_to(ROOT).as_posix()) for p in sdk_files])
    audit = out / "local-audit"
    audit.mkdir()
    for mode in ["debug", "release"]:
        copy(ROOT / f"artifacts/verification-{mode}.json", audit / f"verification-{mode}.json")
    pe_check = subprocess.run([sys.executable, str(ROOT / "tools/verify_artifacts.py"),
                               "--dll", str(payload / (NAME + ".dll")), "--workspace-api", "--lamina"],
                              capture_output=True, text=True, check=True)
    report = {"edition": "LeviLamina native client", "version": VERSION,
              "minecraft": "1.26.60.05", "levilamina": "26.60.7",
              "installed": False, "published": False, "manualGameplay": "pending",
              "dll": describe(payload / (NAME + ".dll")), "verification": verified,
              "archiveMembers": {"runtime": runtime_count, "source": source_count, "sdk": sdk_count},
              "packages": [describe(p) for p in [runtime_zip, source_zip, sdk_zip]],
              "pe": json.loads(pe_check.stdout)["dll"]}
    (out / "ARTIFACTS.json").write_text(json.dumps(report, indent=2) + "\n", encoding="utf-8")
    (out / "SHA256SUMS.txt").write_text("".join(f"{digest(p)}  {p.name}\n" for p in [runtime_zip, source_zip, sdk_zip]), encoding="utf-8")
    print(json.dumps({"release": str(out), **report}, indent=2))


if __name__ == "__main__":
    main()
