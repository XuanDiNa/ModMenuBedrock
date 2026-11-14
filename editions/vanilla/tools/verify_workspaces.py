"""Additive candidate contract audit. Does not relabel the historic frozen build."""
import json
import subprocess
from pathlib import Path
from jsonschema import Draft202012Validator

root = Path(__file__).resolve().parents[1]
v3 = Draft202012Validator(json.loads((root/'integration/modmenu.settings.v3.schema.json').read_text()))
Draft202012Validator.check_schema(v3.schema)
example = json.loads((root/'integration/workspace-example/modmenu.settings.json').read_text(encoding='utf-8'))
v3.validate(example)
for version, filename in [(1, 'modmenu.settings.schema.json'), (2, 'modmenu.settings.v2.schema.json')]:
    validator = Draft202012Validator(json.loads((root/'integration'/filename).read_text()))
    old = dict(example, schemaVersion=version)
    assert not validator.is_valid(old), 'Old schemas must refuse opt-in text'
    original = subprocess.check_output(['git', 'show', f'861e98a:integration/{filename}'], cwd=root)
    assert (root/'integration'/filename).read_bytes() == original, 'Old editor contract changed'
for name in ['src/NativeGuards.inl', 'src/PointerBridge.asm']:
    original = subprocess.check_output(['git', 'show', '861e98a:'+name], cwd=root)
    actual = (root/name).read_bytes()
    assert actual == original, f'Native guard/hook changed: {name}'
for name in ['manifest.json', 'mod.json']:
    assert json.loads((root/name).read_text())['version'] == '0.6.0-test11'
subprocess.run(['py', '-3', str(root/'tools/verify_native60.py')], check=True, cwd=root)
print('Workspace candidate contract audit passed: unchanged v1/v2, legacy entry guards and pointer bridge; additive audited 60 profile/adapters; original scene/input policy and hook set retained.')
