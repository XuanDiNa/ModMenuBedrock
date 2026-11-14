"""Editor contracts only; exact tokens and transactions are tested by C++."""
import copy
import json
from pathlib import Path
from jsonschema import Draft202012Validator

root = Path(__file__).resolve().parents[1] / 'integration'
v1 = Draft202012Validator(json.loads((root / 'modmenu.settings.schema.json').read_text(encoding='utf-8')))
schema = json.loads((root / 'modmenu.settings.v2.schema.json').read_text(encoding='utf-8'))
Draft202012Validator.check_schema(schema)
v2 = Draft202012Validator(schema)
checks = 0
for file in sorted(root.glob('v2-examples/*/modmenu.settings.json')):
    sample = json.loads(file.read_text(encoding='utf-8'))
    v2.validate(sample)
    assert not v1.is_valid(sample), 'v1 editor must refuse a v2 description'
    checks += 2
for file in sorted(root.glob('examples/*/modmenu.settings.json')):
    sample = json.loads(file.read_text(encoding='utf-8'))
    v1.validate(sample)
    sample['schemaVersion'] = 2
    v2.validate(sample)
    checks += 2
sample = json.loads((root / 'v2-examples/ExamplePlugin/modmenu.settings.json').read_text(encoding='utf-8'))
for field_index, property_name, value in [
    (0, 'precision', 2), (0, 'min', -64.5), (0, 'numberType', 'float'),
    (1, 'precision', 0), (1, 'precision', 7), (1, 'step', 0),
    (1, 'step', -1), (1, 'default', '1.0'), (1, 'default', 2147483648),
    (1, 'keys', ['NONE']), (1, 'options', ['a', 'b']),
]:
    candidate = copy.deepcopy(sample)
    candidate['fields'][field_index][property_name] = value
    assert not v2.is_valid(candidate), f'invalid slider declaration accepted: {property_name}={value}'
    checks += 1
for key in ['numberType', 'min', 'max', 'step', 'precision']:
    candidate = copy.deepcopy(sample)
    del candidate['fields'][1][key]
    assert not v2.is_valid(candidate), f'missing slider member accepted: {key}'
    checks += 1
print(f'{checks} slider editor-schema checks / 0 failures')
