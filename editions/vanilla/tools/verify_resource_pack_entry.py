"""Validate the public entry contract and authored pack; never run Minecraft."""
import argparse
import copy
import json
import re
import uuid
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
EVENT = 'button.modmenu.open'
MARKER = 'mmb_resource_pack_entry_v1'
SCENES = {'start_screen.json': ('start', 'start_screen_content'),
          'pause_screen.json': ('pause', 'pause_screen_content')}
MAPPINGS = [
    {'from_button_id': 'button.menu_select', 'to_button_id': '$pressed_button_name', 'mapping_type': 'pressed'},
    {'from_button_id': 'button.menu_ok', 'to_button_id': '$pressed_button_name', 'mapping_type': 'focused'}]
checks = 0


def require(ok, message):
    global checks
    checks += 1
    if not ok:
        raise ValueError(message)


def unique(pairs):
    result = {}
    for name, value in pairs:
        if name in result:
            raise ValueError('Duplicate JSON member')
        result[name] = value
    return result


def load(path):
    require(path.is_file() and not path.is_symlink() and path.stat().st_size <= 65536, 'Unsafe or oversized pack file')
    return json.loads(path.read_text(encoding='utf-8-sig'), object_pairs_hook=unique)


def template_valid(doc):
    require(set(doc) == {'namespace', 'button@common_buttons.light_text_button'} and doc['namespace'] == 'modmenu_entry_v1', 'Template identity changed')
    button = doc['button@common_buttons.light_text_button']
    require(button['$pressed_button_name'] == EVENT, 'Template does not send public event')
    require(button['button_mappings'] == MAPPINGS, 'Button must use local pressed/focused mappings')
    require(button['$button_text|default'] == 'modmenu.entry.open', 'Template localization key changed')
    require(button['size'] == [96, 24], 'Example dimensions changed')


def scene_valid(doc, filename):
    namespace, target = SCENES[filename]
    require(doc['namespace'] == namespace and set(doc) == {'namespace', MARKER, target}, 'Example must only override its own allowed menu')
    require(doc[MARKER] == {'type': 'panel', 'visible': False}, 'Ownership marker must be an unused hidden control')
    panel = doc[target]
    require(set(panel) == {'modifications'} and len(panel['modifications']) == 1, 'Do not replace original content or its bindings')
    mod = panel['modifications'][0]
    require(set(mod) == {'array_name', 'operation', 'value'} and mod['array_name'] == 'controls' and mod['operation'] == 'insert_back', 'Example must append a local control')
    require(len(mod['value']) == 1 and set(mod['value'][0]) == {'modmenu_entry@modmenu_entry_v1.button'}, 'Expected exactly one public entry instance')
    instance = mod['value'][0]['modmenu_entry@modmenu_entry_v1.button']
    require(set(instance) == {'anchor_from', 'anchor_to', 'offset', 'layer', '$focus_id'}, 'Example instance cannot override event or inherit unknown behavior')
    require(instance['anchor_from'] == instance['anchor_to'] == 'top_left' and instance['offset'] == [8, 8] and instance['layer'] == 100, 'Example placement changed')
    require(instance['$focus_id'] == 'modmenu_resource_pack_' + namespace, 'Each menu needs its own unique focus ID')


def refuses(fn, value, message):
    try:
        fn(value)
    except (ValueError, KeyError, TypeError):
        return
    raise AssertionError('Mutation was accepted: ' + message)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--pack-dir', type=Path)
    parser.add_argument('--fixtures', type=Path)
    parser.add_argument('--report', type=Path)
    args = parser.parse_args()
    base = ROOT / 'integration/resource-pack-entry'
    pack = args.pack_dir or base / 'example-pack'
    contract = load(base / 'entry.v1.json')
    require(contract == {
        'apiVersion': 1, 'minimumModMenuVersion': '0.6.0-test7-resourcepack',
        'event': EVENT, 'legacyEvent': 'button.mmb_open_mods', 'ownershipControl': MARKER,
        'uiTechnology': 'bedrock-json-ui', 'allowedScenes': ['start_screen', 'pause_screen'],
        'template': 'modmenu_entry_v1.button', 'templateFile': 'ui/modmenu_entry_v1.json',
        'renderOwner': 'resource-pack', 'openOwner': 'ModMenuBedrock', 'availabilityBinding': None}, 'Public v1 contract changed')
    header = (ROOT / 'include/ModMenu/Definition.h').read_text(encoding='utf-8')
    for value in (EVENT, contract['legacyEvent'], MARKER):
        require('"' + value + '"' in header, 'Runtime contract constants differ from SDK')
    require('ResourcePackEntryApiVersion=1;' in header, 'Runtime API version differs')
    require(set(p.relative_to(pack).as_posix() for p in pack.rglob('*') if p.is_file()) == {
        'manifest.json', 'ui/_ui_defs.json', 'ui/modmenu_entry_v1.json', 'ui/start_screen.json',
        'ui/pause_screen.json', 'texts/languages.json', 'texts/en_US.lang', 'texts/zh_TW.lang',
        'texts/zh_CN.lang'}, 'Example contains unexpected files or omits an asset')
    manifest = load(pack / 'manifest.json')
    require(manifest['format_version'] == 2 and manifest['header']['version'] == [1, 0, 0], 'Resource manifest version differs')
    require(len(manifest['modules']) == 1 and manifest['modules'][0]['type'] == 'resources', 'Example must not install scripts or behavior')
    require(uuid.UUID(manifest['header']['uuid']) != uuid.UUID(manifest['modules'][0]['uuid']), 'Pack/module identities collide')
    require(manifest['header']['min_engine_version'] == [1, 26, 40], 'Example engine floor differs')
    require(load(pack / 'ui/_ui_defs.json') == {'ui_defs': [contract['templateFile']]}, 'Only the new template is registered; existing menu paths are already native')
    template = load(pack / contract['templateFile'])
    template_valid(template)
    bad = copy.deepcopy(template); bad['button@common_buttons.light_text_button']['$pressed_button_name'] = 'button.menu_quit'
    refuses(template_valid, bad, 'wrong action')
    bad = copy.deepcopy(template); bad['button@common_buttons.light_text_button']['button_mappings'][1]['mapping_type'] = 'global'
    refuses(template_valid, bad, 'global confirm hijack')
    scenes = {}
    for filename in SCENES:
        doc = load(pack / 'ui' / filename);scene_valid(doc, filename);scenes[filename] = doc
        bad = copy.deepcopy(doc);bad.pop(MARKER)
        refuses(lambda x: scene_valid(x, filename), bad, 'missing ownership declaration')
        bad = copy.deepcopy(doc);bad['namespace'] = 'settings'
        refuses(lambda x: scene_valid(x, filename), bad, 'different scene')
        bad = copy.deepcopy(doc);bad[SCENES[filename][1]]['modifications'][0]['value'] *= 2
        refuses(lambda x: scene_valid(x, filename), bad, 'duplicate instance')
    require(load(pack / 'texts/languages.json') == ['en_US', 'zh_TW', 'zh_CN'], 'Example languages differ')
    for lang, text in [('en_US', 'Mods'), ('zh_TW', '模組'), ('zh_CN', '模组')]:
        require((pack / ('texts/' + lang + '.lang')).read_text(encoding='utf-8').strip() == 'modmenu.entry.open=' + text, 'Missing or inconsistent localization')
    fixture_count = 0
    if args.fixtures:
        versions = json.loads(args.fixtures.read_text(encoding='utf-8-sig'))
        for version in ['1.26.45.01', '1.26.51.01', '1.26.52.03', '1.26.60.05']:
            for filename, (namespace, target) in SCENES.items():
                raw = versions[version][filename]
                # Preserve quoted URLs/escapes while removing native JSON comments.
                raw = re.sub(r'("(?:\\.|[^"\\])*")|//[^\r\n]*|/\*[\s\S]*?\*/', lambda m: m.group(1) or '', raw)
                vanilla = json.loads(raw, object_pairs_hook=unique)
                require(vanilla['namespace'] == namespace and vanilla[target]['type'] == 'panel', 'Exact menu does not have the authored content target')
                require(isinstance(vanilla[target]['controls'], list), 'Exact content target is not appendable')
                existing = vanilla[target]['controls']
                new = scenes[filename][target]['modifications'][0]['value']
                merged = copy.deepcopy(existing) + copy.deepcopy(new)
                require(merged[:-1] == existing and len(merged) == len(existing) + 1, 'Authored append must preserve every existing control')
                require(not any('modmenu_entry@modmenu_entry_v1.button' in x for x in existing), 'Native fixture unexpectedly already contains example entry')
                fixture_count += 1
    report = {'apiVersion': 1, 'checks': checks, 'privateMenuFixtures': fixture_count,
              'event': EVENT, 'engineMergeAndGameplayAcceptance': 'pending manual test'}
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
