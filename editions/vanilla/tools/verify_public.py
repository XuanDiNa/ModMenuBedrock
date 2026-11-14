"""Audit only tracked publication files; never reads or executes game files."""
import hashlib
import json
import re
import subprocess
from pathlib import Path
from urllib.parse import unquote

root = Path(__file__).resolve().parents[1]


def require(condition, message):
    if not condition:
        raise RuntimeError(message)


tracked = subprocess.check_output(
    ['git', '-C', str(root), 'ls-files', '-z']
).decode('utf-8').split('\0')
tracked = [name for name in tracked if name]
require(bool(tracked), 'Stage the explicit public allowlist before auditing')
bad_extension = {'.exe', '.dll', '.pdb', '.zip', '.log', '.i64', '.id0', '.id1', '.id2', '.nam', '.til', '.pyc'}
private_path = re.compile(r'(?i)(?:[a-z]:[\\/](?:Users|LeviLauncher_Construct|bedrock_Tool|Stroll)|/[U]sers/|/[h]ome/[^/]+/)')
secret = re.compile(r'(?:gh[pousr]_[A-Za-z0-9]{30,}|github_pat_[A-Za-z0-9_]{40,}|-----BEGIN (?:RSA |OPENSSH |EC )?PRIVATE KEY-----)')
allowed_roots = {'src', 'include', 'tests', 'tools', 'integration', 'vendor', 'docs'}
allowed_top = {'.gitignore', '.gitattributes', 'LICENSE', 'README.md', 'BUILDING.md', 'TESTING.md', 'THIRD_PARTY_NOTICES.txt', 'xmake.lua', 'manifest.json', 'mod.json', 'config.json', 'config.schema.json'}

for name in tracked:
    path = root / name
    require(path.suffix.lower() not in bad_extension, f'Forbidden publication artifact: {name}')
    parts = path.relative_to(root).parts
    require(parts[0] in allowed_roots if len(parts) > 1 else name in allowed_top, f'Unexpected publication path: {name}')
    require(not path.is_symlink(), f'Symlink refused: {name}')
    text = path.read_text(encoding='utf-8-sig')
    require(not private_path.search(text), f'Private absolute path in {name}')
    require(not secret.search(text), f'Credential-shaped content in {name}')
    require(not re.search(r'^(?:<<<<<<<|>>>>>>>)(?: |$)', text, re.M), f'Conflict marker in {name}')
    if path.suffix == '.md':
        for target in re.findall(r'\]\(([^\s)]+)(?:\s+"[^"]*")?\)', text):
            if re.match(r'^(?:https?://|mailto:|#)', target):
                continue
            relative = unquote(target.split('#', 1)[0])
            require((path.parent / relative).is_file(), f'Broken documentation link: {name} -> {target}')

rows = (root / 'docs/runtime-source-sha256.txt').read_text(encoding='ascii').splitlines()
require(len(rows) == 27, 'Expected all 27 accepted runtime files')
covered = set()
for row in rows:
    digest, name = row.split('  ', 1)
    require(name not in covered and name in tracked, f'Unexpected runtime manifest member: {name}')
    require(hashlib.sha256((root / name).read_bytes()).hexdigest().upper() == digest, f'Runtime source changed: {name}')
    covered.add(name)
runtime = {name for name in tracked if name.startswith(('src/', 'include/'))}
require(covered == runtime, 'Runtime hash manifest coverage differs from source tree')
for name in ('manifest.json', 'mod.json'):
    require(json.loads((root / name).read_text(encoding='utf-8'))['version'] == '0.5.0-test1', 'Version metadata mismatch')
require((root / 'LICENSE').read_text().startswith('MIT License\n'), 'MIT license missing')
print(f'Public audit passed: {len(tracked)} tracked files; 27 exact runtime sources; no private paths, binaries, credentials or broken local Markdown links.')
