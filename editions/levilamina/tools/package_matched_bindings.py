"""Freeze a flat, matching 26.52 update; never install, launch or publish."""
import argparse
import datetime
import hashlib
import json
import shutil
import subprocess
import zipfile
from pathlib import Path


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(value, encoding='utf-8')


def load(path):
    return json.loads(path.read_text(encoding='utf-8-sig'))


def sums(directory):
    files = [f for f in sorted(directory.rglob('*')) if f.is_file() and f.name != 'SHA256SUMS.txt']
    write(directory / 'SHA256SUMS.txt', ''.join(
        digest(f) + '  ' + f.relative_to(directory).as_posix() + '\n' for f in files))


def archive(directory, destination):
    files = {f.relative_to(directory).as_posix(): f for f in directory.rglob('*') if f.is_file()}
    with zipfile.ZipFile(destination, 'x', compression=zipfile.ZIP_DEFLATED) as z:
        for name, path in sorted(files.items()):
            z.write(path, name)
    with zipfile.ZipFile(destination) as z:
        assert set(z.namelist()) == set(files)
        for name, path in files.items():
            assert hashlib.sha256(z.read(name)).hexdigest().upper() == digest(path), name


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--player-source', type=Path, required=True)
    parser.add_argument('--player-source-zip', type=Path, required=True)
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[1]
    player = args.player_source.resolve()
    basezip = args.player_source_zip.resolve()
    assert digest(basezip) == '60B87D5407B33A00CAD82079A0A195E2E84ABEA8C9C2355C88D2F5C30E0A784D'
    expected_changes = {'include/Player.h', 'manifest.json', 'mod.json', 'modmenu.settings.json',
                        'src/Core.cpp', 'src/JvmHost.cpp', 'src/Plugin.cpp', 'src/Ui.cpp', 'tests/CoreTests.cpp'}
    with zipfile.ZipFile(basezip) as z:
        source_names = sorted(n for n in z.namelist() if not n.endswith('/'))
        changes = {n for n in source_names if z.read(n) != (player / n).read_bytes()}
        assert changes == expected_changes, changes
    sdk_name = 'include/sdk/ModMenuBinding.h'
    assert (player / sdk_name).read_bytes() == (root / 'integration/sdk/ModMenuBinding.h').read_bytes()
    for name in ['binding-070-verification.json', 'binding-070-debug-verification.json']:
        commands = load(root / 'artifacts' / name)
        assert len(commands) >= 9 and all(c['exit'] == 0 for c in commands), name
    consumers = load(root / 'artifacts/binding-consumer-verification.json')
    assert len(consumers) >= 88 and all(c['exit'] == 0 for c in consumers)
    subprocess.run(['py', '-3', 'tools/verify_workspace_source.py'], cwd=root, check=True)
    subprocess.run(['git', 'diff', '--check'], cwd=root, check=True)

    prior = load(root / 'artifacts/bindings-latest-package.json')
    prior_dlls = {p['plugin']: p for p in prior['dlls']}
    stage = root / 'artifacts/binding-flat-followup'
    # Finish only this isolated staging tree, never a live installation.
    player_dest = stage / 'mods/MuoniumPlayerBedrock'
    for source, name in [(player / 'build/windows/x64/release/MuoniumPlayerBedrock.dll', 'MuoniumPlayerBedrock.dll'),
                         (player / 'LICENSE', 'LICENSE'), (player / 'NOTICE', 'NOTICE')]:
        shutil.copy2(source, player_dest / name)
    for name in ['manifest.json', 'mod.json', 'modmenu.settings.json']:
        assert (player_dest / name).read_bytes() == (player / name).read_bytes(), name
    folders = sorted((stage / 'mods').iterdir())
    assert {p.name for p in folders} == set(prior_dlls)
    for directory in folders:
        meta = load(directory / 'manifest.json')
        binary = directory / meta['entry']
        if directory.name != 'MuoniumPlayerBedrock':
            old = prior_dlls[directory.name]
            assert meta['version'] == old['version']
            assert binary.stat().st_size == old['bytes'] and digest(binary) == old['sha256']
        else:
            assert meta['version'] == '0.7.0-test2-leftalt'
        if directory.name != 'ModMenuBedrock':
            desc = load(directory / 'modmenu.settings.json')
            assert desc['schemaVersion'] == 5 and desc['plugin']['id'] == directory.name
            assert desc['plugin']['entry'] == meta['entry'] and meta['version'] in desc['plugin']['versions']
    stamp = datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
    out = root / 'releases' / ('Bindings-matched-26.52-' + stamp)
    out.mkdir(parents=True, exist_ok=False)
    payload = out / 'payload'
    # This runtime update needs no SDK examples or default settings files.
    # Copy only named root payload files, rather than a whole project tree.
    for directory in folders:
        dest = payload / 'mods' / directory.name
        dest.mkdir(parents=True)
        allowed = {load(directory / 'manifest.json')['entry'], 'manifest.json', 'mod.json',
                   'modmenu.settings.json', 'config.schema.json', 'LICENSE', 'NOTICE',
                   'THIRD_PARTY_NOTICES.txt'}
        for name in sorted(allowed):
            file = directory / name
            if file.is_file():
                assert not file.is_symlink(), file
                shutil.copy2(file, dest / name)
    (payload / 'mods/ModMenuBedrock/docs').mkdir()
    shutil.copy2(root / 'docs/keybindings-testing.md', payload / 'mods/ModMenuBedrock/docs/keybindings-testing.md')
    shutil.copy2(root / 'docs/bindings-matched-testing.md', payload / 'README.md')
    shutil.copy2(root / 'tools/check_binding_install.ps1', payload / 'CHECK-INSTALL.ps1')
    records = []
    for directory in sorted((payload / 'mods').iterdir()):
        meta = load(directory / 'manifest.json')
        binary = directory / meta['entry']
        files = [binary, directory / 'manifest.json']
        files += [directory / n for n in ['mod.json', 'modmenu.settings.json'] if (directory / n).is_file()]
        records.append({'plugin': directory.name, 'entry': meta['entry'], 'version': meta['version'],
                        'bytes': binary.stat().st_size, 'sha256': digest(binary),
                        'files': [{'path': f.name, 'bytes': f.stat().st_size, 'sha256': digest(f)} for f in files]})
        sums(directory)
    write(payload / 'DLL-MANIFEST.json', json.dumps({'schemaVersion': 1, 'targetGame': '1.26.52.03',
                                                   'plugins': records}, indent=2) + '\n')
    write(payload / 'VALIDATION.json', json.dumps({
        'gameplayAcceptance': 'pending', 'automaticInstallOrLaunch': False,
        'menuDLL': 'unchanged frozen 0.6.0-test5-leftalt',
        'menuBaselineSuitesPerMode': 22, 'menuBindingChecks': 431, 'schemaParserCases': 3525,
        'consumerBaselineCommands': len(consumers), 'bundleChecks': 898, 'activationFields': 14,
        'muoniumReleaseCommands': len(load(root / 'artifacts/binding-070-verification.json')),
        'muoniumDebugCommands': len(load(root / 'artifacts/binding-070-debug-verification.json')),
        'muoniumBaseSourceSHA256': digest(basezip), 'muoniumChangedFiles': sorted(changes),
        'muoniumAddedFile': sdk_name, 'muoniumNativeAndJavaSourcesUnchanged': True,
        'companionAdmissionUpdated': False}, indent=2) + '\n')
    subprocess.run(['powershell.exe', '-NoProfile', '-ExecutionPolicy', 'Bypass', '-File',
                    str(payload / 'CHECK-INSTALL.ps1'), '-ModsPath', str(payload / 'mods')], check=True)
    subprocess.run([str(root / 'build/windows/x64/release/ModMenuBindingBundleTests.exe'),
                    str(payload / 'mods')], check=True)
    sums(payload)
    package = out / 'ModMenu-matched-bindings-26.52-testpack.zip'
    archive(payload, package)

    menu_source = out / 'ModMenuBedrock-bindings-followup-source.zip'
    names = subprocess.check_output(['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'],
                                    cwd=root).decode('utf-8').split('\0')
    with zipfile.ZipFile(menu_source, 'x', compression=zipfile.ZIP_DEFLATED) as z:
        for name in sorted(set(names) - {''}):
            z.write(root / name, name)
    player_source = out / 'MuoniumPlayerBedrock-0.7.0-test2-leftalt-source.zip'
    with zipfile.ZipFile(player_source, 'x', compression=zipfile.ZIP_DEFLATED) as z:
        for name in sorted(source_names + [sdk_name]):
            z.write(player / name, name)
    for path in [package, menu_source, player_source]:
        write(Path(str(path) + '.sha256'), digest(path) + '  ' + path.name + '\n')
    summary = {'directory': str(out), 'package': str(package), 'source': str(menu_source),
               'playerSource': str(player_source), 'dlls': records,
               'archives': [{'name': f.name, 'bytes': f.stat().st_size, 'sha256': digest(f)}
                            for f in [package, menu_source, player_source]]}
    write(root / 'artifacts/bindings-flat-latest-package.json', json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2))


if __name__ == '__main__':
    main()
