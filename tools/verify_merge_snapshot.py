"""Audit the frozen two-edition merge, not gameplay or future runtime changes."""
import argparse
import json
from pathlib import Path
import re
import subprocess
from urllib.parse import unquote


ROOT = Path(__file__).resolve().parents[1]
SNAPSHOTS = {
    'vanilla': ('4c23aa29f7f75c91c0b6064ec18065e150bf98ab',
                '0.6.0-test5-leftalt', 'ModMenuBedrock.dll'),
    'levilamina': ('9679aa91246396ba10198eb8599f1ba33d73a306',
                   '0.1.0-test2-ll26.60', 'ModMenuBedrockLL.dll'),
}


def git(*args):
    return subprocess.check_output(['git', '-C', str(ROOT), *args])


def require(value, reason):
    if not value:
        raise RuntimeError(reason)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--tree', default='HEAD', help='Commit or staged tree ID')
    args = parser.parse_args()
    tree = git('rev-parse', '--verify', args.tree + '^{tree}').decode().strip()
    files = git('ls-tree', '-r', '--name-only', '-z', tree).decode('utf-8').strip('\0').split('\0')
    rows = []
    for edition, (commit, version, entry) in SNAPSHOTS.items():
        prefix = 'editions/' + edition
        expected = git('rev-parse', commit + '^{tree}').decode().strip()
        actual = git('rev-parse', tree + ':' + prefix).decode().strip()
        require(actual == expected, 'Snapshot changed: ' + edition)
        for name in ['manifest.json', 'mod.json']:
            data = json.loads(git('show', tree + ':' + prefix + '/' + name))
            require(data['version'] == version, 'Version changed: ' + edition)
            if name == 'manifest.json':
                require(data['entry'] == entry, 'DLL identity changed: ' + edition)
        count = sum(name.startswith(prefix + '/') for name in files)
        rows.append({'edition': edition, 'originalCommit': commit,
                     'tree': actual, 'files': count, 'completeSnapshotMatches': True})

    forbidden = {'.dll', '.exe', '.bin', '.zip', '.pdb', '.log', '.i64', '.pyc'}
    require(all(Path(name).suffix.lower() not in forbidden for name in files),
            'Unexpected binary, private fixture or build output')
    for name in ['README.md', 'docs/edition-layout.md', 'docs/releases/2026-11-14-cumulative.md']:
        text = git('show', tree + ':' + name).decode('utf-8-sig')
        for target in re.findall(r'\]\(([^\s)]+)\)', text):
            if re.match(r'^(?:https?://|mailto:|#)', target):
                continue
            relative = unquote(target.split('#', 1)[0])
            require(relative and not relative.startswith('/'), 'Invalid shared-doc link')
            destination = (ROOT / name).parent / relative
            require(destination.resolve().is_relative_to(ROOT), 'Link escapes repository')
            require(destination.relative_to(ROOT).as_posix() in files,
                    'Broken shared-doc link: ' + name + ' -> ' + target)
    require(not any(name.startswith(('src/', 'include/')) or name == 'xmake.lua'
                    for name in files), 'Combined runtime at repository root')
    print(json.dumps({'success': True, 'snapshots': rows, 'sharedDocumentationLinks': 'pass',
                      'runtimeChanges': False, 'gameplay': 'not run'}, indent=2))


if __name__ == '__main__':
    main()
