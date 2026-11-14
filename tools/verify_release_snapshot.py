"""Audit the published test11 native snapshot and unchanged LL source tree."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import subprocess
from urllib.parse import unquote

ROOT = Path(__file__).resolve().parents[1]
LL_COMMIT = '9679aa91246396ba10198eb8599f1ba33d73a306'


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
    snapshot = json.loads(git('show', tree + ':docs/vanilla-test11-snapshot.json'))
    require(snapshot['version'] == '0.6.0-test11', 'Wrong native release')
    require(snapshot['dllSHA256'] == '17CB9D3505B6B17942A2975CD7E5F670A277B4B42D61E40042F2C3AA781DAF6E',
            'Frozen release DLL identity changed')
    require(git('rev-parse', tree + ':editions/vanilla').decode().strip() == snapshot['sourceTree'],
            'Native source differs from the reviewed publication snapshot')
    require(git('rev-parse', tree + ':editions/levilamina').strip() == git('rev-parse', LL_COMMIT + '^{tree}').strip(),
            'LL snapshot changed')
    for edition, version, entry in [('vanilla', snapshot['version'], 'ModMenuBedrock.dll'),
                                     ('levilamina', '0.1.0-test2-ll26.60', 'ModMenuBedrockLL.dll')]:
        for name in ['manifest.json', 'mod.json']:
            data = json.loads(git('show', tree + ':editions/' + edition + '/' + name))
            require(data['version'] == version, 'Version mismatch: ' + edition)
            if name == 'manifest.json':
                require(data['entry'] == entry, 'DLL identity mismatch: ' + edition)

    forbidden = {'.dll', '.exe', '.bin', '.zip', '.pdb', '.log', '.i64', '.id0', '.id1', '.id2', '.pyc'}
    private = re.compile(r'(?i)(?:[a-z]:[\\/](?:Users|LeviLauncher_Construct|bedrock_Tool|Stroll)|/[U]sers/|/[h]ome/[^/]+/)')
    secret = re.compile(r'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----)')
    texts = git('grep', '-I', '-l', '.', tree).decode('utf-8').splitlines()
    require(bool(texts), 'Empty source snapshot')
    for name in files:
        require(Path(name).suffix.lower() not in forbidden, 'Unexpected publication artifact: ' + name)
        raw = git('show', tree + ':' + name)
        text = raw.decode('utf-8-sig')
        require(not private.search(text), 'Private path in ' + name)
        require(not secret.search(text), 'Credential-shaped content in ' + name)
        require(not re.search(r'^(?:<<<<<<<|>>>>>>>)(?: |$)', text, re.M), 'Conflict marker in ' + name)
        # Unchanged upstream reference excerpts intentionally omit website images.
        upstream = name.startswith('editions/levilamina/docs/upstream-levilamina-26.60.7/')
        if Path(name).suffix == '.md' and not upstream:
            for target in re.findall(r'\]\(([^\s)]+)\)', text):
                if re.match(r'^(?:https?://|mailto:|#)', target):
                    continue
                relative = unquote(target.split('#', 1)[0])
                destination = (ROOT / name).parent / relative
                require(relative and not relative.startswith('/') and destination.resolve().is_relative_to(ROOT),
                        'Unsafe local link in ' + name)
                require(destination.resolve().relative_to(ROOT).as_posix() in files,
                        'Broken local link: ' + name + ' -> ' + target)
    require(not any(name.startswith(('src/', 'include/')) or name == 'xmake.lua' for name in files),
            'Combined runtime at repository root')
    print(json.dumps({'success': True, 'nativeVersion': snapshot['version'],
                      'nativeSourceTree': snapshot['sourceTree'], 'frozenDLLSHA256': snapshot['dllSHA256'],
                      'levilaminaUnchanged': True, 'publicationFiles': len(files),
                      'linksPrivacyExclusions': 'pass', 'gameplay': 'not run'}, indent=2))


if __name__ == '__main__':
    main()
