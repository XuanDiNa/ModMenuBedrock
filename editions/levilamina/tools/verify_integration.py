"""Validate editor schema/examples; authoritative runtime tests live in C++."""
import copy, json
from pathlib import Path
from jsonschema import Draft202012Validator

root = Path(__file__).resolve().parents[1] / 'integration'
schema = json.loads((root / 'modmenu.settings.schema.json').read_text(encoding='utf-8'))
Draft202012Validator.check_schema(schema)
validator = Draft202012Validator(schema)
examples = sorted(root.glob('examples/*/modmenu.settings.json'))
if len(examples) != 3:
    raise RuntimeError('Expected three allowlisted integration examples')
for example in examples:
    validator.validate(json.loads(example.read_text(encoding='utf-8')))

sample = json.loads(examples[1].read_text(encoding='utf-8'))
cases = []
for member in ('schemaVersion', 'plugin', 'config', 'fields'):
    case = copy.deepcopy(sample); del case[member]; cases.append(case)
case = copy.deepcopy(sample); case['schemaVersion'] = 2; cases.append(case)
case = copy.deepcopy(sample); case['script'] = 'anything'; cases.append(case)
case = copy.deepcopy(sample); case['fields'][0]['default'] = 'true'; cases.append(case)
case = copy.deepcopy(sample); case['fields'][0]['label'] = 'bad\nlabel'; cases.append(case)
case = copy.deepcopy(sample); case['fields'][0]['label'] = 'bad\x7flabel'; cases.append(case)
case = copy.deepcopy(sample); case['fields'][0]['label'] = 'bad§alabel'; cases.append(case)
case = copy.deepcopy(sample); case['fields'][1]['min'] = -1; cases.append(case)
case = copy.deepcopy(sample); case['fields'][2]['options'] = list('abcdefg'); cases.append(case)
case = copy.deepcopy(sample); case['fields'][3]['keys'] = ['ESCAPE']; cases.append(case)
for case in cases:
    if validator.is_valid(case):
        raise RuntimeError('Editor schema unexpectedly accepts invalid author fixture')
print(f'{len(examples)} editor examples and {len(cases)} invalid-description refusals pass')
