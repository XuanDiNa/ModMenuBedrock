"""Candidate source/privacy audit; the frozen 0.5.0 audit stays unchanged."""
import hashlib
import json
import re
import subprocess
from pathlib import Path
from urllib.parse import unquote

root = Path(__file__).resolve().parents[1]
names = subprocess.check_output(
    ['git', 'ls-files', '--cached', '--others', '--exclude-standard', '-z'], cwd=root
).decode('utf-8').split('\0')
names = sorted({n for n in names if n})
allowed_roots = {'src', 'include', 'tests', 'tools', 'integration', 'vendor', 'docs'}
allowed_top = {'.gitignore', '.gitattributes', 'LICENSE', 'README.md', 'BUILDING.md',
               'TESTING.md', 'THIRD_PARTY_NOTICES.txt', 'xmake.lua', 'manifest.json',
               'mod.json', 'config.json', 'config.schema.json'}
forbidden = {'.exe', '.dll', '.pdb', '.zip', '.log', '.i64', '.id0', '.id1', '.id2',
             '.nam', '.til', '.pyc'}
private = re.compile(r'(?i)(?:[a-z]:[\\/](?:Users|LeviLauncher_Construct|bedrock_Tool|Stroll)|/[U]sers/|/[h]ome/[^/]+/)')
secret = re.compile(r'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----)')
digests = []
for name in names:
    path = root / name
    parts = Path(name).parts
    assert (parts[0] in allowed_roots if len(parts) > 1 else name in allowed_top), name
    assert path.suffix.lower() not in forbidden and not path.is_symlink(), name
    text = path.read_text(encoding='utf-8-sig')
    assert not private.search(text), f'Private absolute path: {name}'
    assert not secret.search(text), f'Credential-shaped content: {name}'
    assert not re.search(r'^(?:<<<<<<<|>>>>>>>)(?: |$)', text, re.M), name
    if path.suffix == '.md':
        for target in re.findall(r'\]\(([^\s)]+)(?:\s+"[^"]*")?\)', text):
            if re.match(r'^(?:https?://|mailto:|#)', target):
                continue
            assert (path.parent / unquote(target.split('#', 1)[0])).is_file(), (name, target)
    digests.append(hashlib.sha256(path.read_bytes()).hexdigest() + '  ' + name)
for name in ['tools/verify_public.py', 'docs/runtime-source-sha256.txt', 'docs/verification.md']:
    original = subprocess.check_output(['git', 'show', '861e98a:'+name], cwd=root)
    assert (root/name).read_text(encoding='utf-8') == original.decode('utf-8').replace('\r\n', '\n'), f'Historic audit changed: {name}'
assert (root/'LICENSE').read_text().startswith('MIT License\n')
for name in ['manifest.json', 'mod.json']:
    assert json.loads((root/name).read_text())['version'] == '0.6.0-test8-mc26.60'
print(f'Candidate public source audit passed: {len(names)} allowlisted files; no private paths, binaries, credential patterns or broken Markdown links.')
print('Current source tree SHA256: ' + hashlib.sha256('\n'.join(digests).encode()).hexdigest().upper())
