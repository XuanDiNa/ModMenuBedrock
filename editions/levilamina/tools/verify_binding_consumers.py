"""Offline consumer build/config/native tests. Never installs or starts Minecraft."""
import argparse
import hashlib
import json
import subprocess
from pathlib import Path
from jsonschema import Draft202012Validator

parser = argparse.ArgumentParser()
parser.add_argument('--workspace', type=Path, default=Path(__file__).resolve().parents[2])
parser.add_argument('--muonium', required=True, type=Path)
parser.add_argument('--images', required=True, type=Path)
parser.add_argument('--only', default='')
args = parser.parse_args()
args.workspace=args.workspace.resolve()
args.muonium=args.muonium.resolve()
args.images=args.images.resolve()
menu = Path(__file__).resolve().parents[1]
report_path=menu/'artifacts/binding-consumer-verification.json'
log = json.loads(report_path.read_text(encoding='utf-8')) if args.only and report_path.exists() else []

def run(argv, cwd):
    result = subprocess.run([str(a) for a in argv], cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    output = result.stdout.decode('utf-8', errors='replace')
    entry = {'command': [str(a) for a in argv], 'cwd': str(cwd), 'exit': result.returncode, 'output': output}
    log.append(entry)
    print(output[-5000:], end='', flush=True)
    (menu/'artifacts/binding-consumer-verification.json').write_text(json.dumps(log, ensure_ascii=True, indent=2), encoding='utf-8')
    if result.returncode:
        raise RuntimeError(f'Failed: {argv}')

projects = [
    ('AutoClick', ['AutoClickTests', 'InputThreadTests', 'ConfigReloadTests', 'SlotInputTests', 'SlotSessionTests']),
    ('AutoJumpReset/v2', ['ConfigTests', 'ConfigStoreTests', 'PolicyTests', 'BurstTests', 'ScriptSourceTests', 'ScriptJumpTests', 'NotificationRouteTests', 'DiagnosticsTests', 'NativeDiagnosticsTests', 'StartupTests', 'CallerDiagnosticsTests', 'SourceModeTests', 'DispatchLeafTests', 'ReceiveTests', 'DamagePacketTests', 'PacketFileTests', 'MotionMatcherTests', 'ReceiveLeafTests', 'NoticeTests', 'ToastTests', 'MenuContractTests']),
    ('ContainerPreview', ['ContainerPreview'+n+'Tests' for n in ['Core','Compatibility','EntityUse','HeldUse','RemoteNative','Remote','Hook','Render','World','Bridge','Container','Native','Diagnostic']]),
    ('FakeSneak', ['FakeSneakTests','FakeSneakQuietReleaseTests','FakeSneakTraceTests','FakeSneakNativeTests']),
    ('fullbright/v26.52', ['FullBright'+n+'Tests' for n in ['Core','ConfigFile','ModMenu','Native','Smoke']]),
    ('ToggleSprint', ['ToggleSprint'+n+'Tests' for n in ['KeyToken','Config','ConfigFile','ModMenu','Policy','NativeInput','Smoke']]),
]
sdk = (menu/'integration/sdk/ModMenuBinding.h').read_bytes()
schema = Draft202012Validator(json.loads((menu/'integration/modmenu.settings.v5.schema.json').read_text(encoding='utf-8')))
for name, tests in projects:
    if args.only and args.only!=name:continue
    root = args.workspace/name
    print(f'--- {name} ---', flush=True)
    assert (root/'include/sdk/ModMenuBinding.h').read_bytes() == sdk, name
    run(['xmake','build','-P',root,'-y','-j','2'],root)
    if name == 'AutoJumpReset/v2':
        run(['xmake','build','-P',root,'-y','-j','2','MenuContractTests'],root)
    for target in tests:
        command=[root/'build/windows/x64/release'/f'{target}.exe']
        if target=='FakeSneakNativeTests':command.append(args.images/'1.26.52.03/Minecraft.Windows.exe.orig')
        run(command, root)
    sidecar = root/('package' if name=='AutoJumpReset/v2' else '')/'modmenu.settings.json'
    schema.validate(json.loads(sidecar.read_text(encoding='utf-8')))
    if name=='ContainerPreview':
        for target in ['Container','Diagnostic','RemoteNative','HeldUse','EntityUse']:
            run([root/'build/windows/x64/release'/f'ContainerPreview{target}Tests.exe','51'],root)
        for version in ['1.26.45.01','1.26.51.01','1.26.52.03']:
            for file in ['Minecraft.Windows.exe.orig','Minecraft.Windows.exe']:
                image=args.images/version/file
                approved={'008408E6F544D7987F9CDE4E34C8E853690A65AFD97F8498DA2CD6ED24C4793B','4143182F113D774062809039154248E7AF7578A81C92E54323F7D53361F8EFA1','4A92BFA3CE2428B40EE517B7C1125D9F6F79274382DE03846351992663B2E2E4','0B66101DC873434E853A898329CB85CEE0DE52D0AE91AC796EFA0FE05B61522C','258EC563FD37B3EFCF67C2C5207AA0DF6AB8E20DC09ACF2394380D697CC24DAC','1ECE1D37E7762F6B32C20DB355A7F0B199A85FC6A60C59CF693922C999A16AE7'}
                digest=hashlib.sha256(image.read_bytes()).hexdigest().upper()
                command=[root/'build/windows/x64/release/ContainerPreviewProfileTests.exe',image]
                if digest not in approved:
                    print(f'Unreviewed launcher image {version}: {digest}; testing fail-closed rejection, not new compatibility.',flush=True)
                    command.append('reject')
                run(command,root)
    if name in ('fullbright/v26.52','ToggleSprint'):
        prefix = 'FullBright' if name.startswith('fullbright') else 'ToggleSprint'
        run([root/'build/windows/x64/release'/f'{prefix}ProfileTests.exe',args.images/'1.26.52.03/Minecraft.Windows.exe.orig',args.images/'1.26.52.03/Minecraft.Windows.exe'],root)
    if name=='FakeSneak':
        run([root/'build/windows/x64/release/FakeSneakProfileTests.exe',args.images/'1.26.52.03/Minecraft.Windows.exe.orig'],root)
        run([root/'build/windows/x64/release/FakeSneakUnsupportedHostTest.exe',root/'build/windows/x64/release/FakeSneak.dll'],root)

if not args.only or args.only=='MuoniumPlayerBedrock':
    root=args.muonium
    assert (root/'include/sdk/ModMenuBinding.h').read_bytes()==sdk
    schema.validate(json.loads((root/'modmenu.settings.json').read_text(encoding='utf-8')))
    for target in ['PlayerTests','JvmPolicyTests']:
        run(['xmake','build','-P',root,'-y','-j','2',target],root)
        run([root/'build/windows/x64/release'/f'{target}.exe'],root)
print(f'PASS {len(log)} consumer offline commands; no Minecraft process was started.')
