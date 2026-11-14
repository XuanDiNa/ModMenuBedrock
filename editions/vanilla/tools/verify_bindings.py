"""Validate public binding schemas against the executable SDK parser."""
import json
import hashlib
import subprocess
import sys
from pathlib import Path
from jsonschema import Draft202012Validator

root = Path(__file__).resolve().parents[1]
exe = Path(sys.argv[1]) if len(sys.argv) > 1 else root / 'build/windows/x64/release/ModMenuBindingTests.exe'
cases = json.loads(subprocess.check_output([str(exe), '--schema-cases'], text=True))
for version in (4,5):
    schema = json.loads((root / f'integration/modmenu.settings.v{version}.schema.json').read_text(encoding='utf-8'))
    Draft202012Validator.check_schema(schema)
    validator = Draft202012Validator(schema['$defs']['binding'])
    for case in cases:
        assert validator.is_valid(case['value']) == case['v4' if version==4 else 'valid'], (version,case)
assert hashlib.sha256((root/'integration/modmenu.settings.v4.schema.json').read_bytes()).hexdigest().upper() == '6FD56523AC30D4DB392003A1064095CE37A7A5231BB89905727B9696022F8B84', 'Frozen v4 schema changed'
for name in ['modmenu.settings.schema.json', 'modmenu.settings.v2.schema.json']:
    original = subprocess.check_output(['git', 'show', '861e98a:integration/' + name], cwd=root)
    assert (root / 'integration' / name).read_bytes() == original, name
assert hashlib.sha256((root/'integration/modmenu.settings.v3.schema.json').read_bytes()).hexdigest().upper() == '219092E48995700CECA61653750F9CB7463FC1BDD65D1FAC68B642F68F624146', 'Frozen test3 v3 schema changed'
print(f'Binding schema/parser agreement passed: {len(cases)} cases; v1/v2/v3/v4 unchanged.')
