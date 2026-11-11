"""Freeze verified binding candidates. No installation, game launch or upload."""
import argparse
import datetime
import hashlib
import json
import shutil
import subprocess
import zipfile
from pathlib import Path

p=argparse.ArgumentParser()
p.add_argument('--muonium',required=True,type=Path)
a=p.parse_args()
root=Path(__file__).resolve().parents[1]
workspace=root.parent
muonium=a.muonium.resolve()
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest().upper()
def write(path,text):
    path.parent.mkdir(parents=True,exist_ok=True)
    path.write_text(text,encoding='utf-8')
def copy(source,dest):
    assert source.is_file() and not source.is_symlink(),source
    dest.parent.mkdir(parents=True,exist_ok=True)
    shutil.copy2(source,dest)
def archive(source,dest,prefix=True):
    with zipfile.ZipFile(dest,'x',compression=zipfile.ZIP_DEFLATED) as z:
        for f in sorted(source.rglob('*')):
            if f.is_file():z.write(f,(source.name+'/' if prefix else '')+f.relative_to(source).as_posix())
def sums(directory):
    files=[f for f in sorted(directory.rglob('*')) if f.is_file() and f.name!='SHA256SUMS.txt']
    write(directory/'SHA256SUMS.txt',''.join(digest(f)+'  '+f.relative_to(directory).as_posix()+'\n' for f in files))

subprocess.run(['py','-3',str(root/'tools/verify_workspace_source.py')],check=True,cwd=root)
report=json.loads((root/'artifacts/verification.json').read_text(encoding='utf-8-sig'))
assert report['Success'] and report['Version']=='0.6.0-test5-leftalt'
assert {x['Mode'] for x in report['Modes']}=={'debug','release'}
assert all(x['Suites']==22 for x in report['Modes'])
dll=root/'build/windows/x64/release/ModMenuBedrock.dll'
assert digest(dll)==next(x['RebuiltDLLSHA256'] for x in report['Modes'] if x['Mode']=='release')
consumer=json.loads((root/'artifacts/binding-consumer-verification.json').read_text(encoding='utf-8'))
assert len(consumer)>=80 and all(x['exit']==0 for x in consumer)
assert any('PlayerTests.exe' in x['command'][0] for x in consumer)
sdk=(root/'integration/sdk/ModMenuBinding.h').read_bytes()

stamp=datetime.datetime.now().strftime('%Y%m%d-%H%M%S')
out=root/'releases'/('ModMenuBedrock-0.6.0-test5-leftalt-'+stamp)
out.mkdir(parents=True,exist_ok=False)
payload=out/'BindingCandidates'
main=payload/'ModMenuBedrock'
copy(dll,main/dll.name)
for name in ['manifest.json','mod.json','LICENSE','THIRD_PARTY_NOTICES.txt','config.schema.json','docs/keybindings-testing.md']:
    copy(root/name,main/name)
for f in (root/'integration').rglob('*'):
    if f.is_file():copy(f,main/'integration'/f.relative_to(root/'integration'))

projects=[('AutoClick',workspace/'AutoClick',False),('AutoJumpReset',workspace/'AutoJumpReset/v2',True),
          ('ContainerPreview',workspace/'ContainerPreview',False),('FakeSneak',workspace/'FakeSneak',False),
          ('FullBright',workspace/'fullbright/v26.52',False),('ToggleSprint',workspace/'ToggleSprint',False),
          ('MuoniumPlayerBedrock',muonium,False)]
records=[{'plugin':'ModMenuBedrock','version':report['Version'],'bytes':dll.stat().st_size,'sha256':digest(dll)}]
for name,project,has_package in projects:
    assert (project/'include/sdk/ModMenuBinding.h').read_bytes()==sdk,name
    meta=project/'package' if has_package else project
    descriptor=json.loads((meta/'modmenu.settings.json').read_text(encoding='utf-8'))
    manifest=json.loads((meta/'manifest.json').read_text(encoding='utf-8-sig'))
    assert descriptor['schemaVersion']==5 and manifest['version'] in descriptor['plugin']['versions']
    assert descriptor['plugin']['id']==name and descriptor['plugin']['entry']==manifest['entry']
    binary=project/'build/windows/x64/release'/manifest['entry']
    dest=payload/'plugin-updates'/name
    copy(binary,dest/manifest['entry'])
    for f in ['manifest.json','mod.json','modmenu.settings.json','config.schema.json']:
        if (meta/f).is_file():copy(meta/f,dest/f)
    for f in ['LICENSE','THIRD_PARTY_NOTICES.txt','NOTICE']:
        if (project/f).is_file():copy(project/f,dest/f)
    records.append({'plugin':name,'version':manifest['version'],'bytes':binary.stat().st_size,'sha256':digest(binary)})
    sums(dest)

copy(root/'docs/keybindings-testing.md',payload/'TESTING.md')
write(payload/'DLL-MANIFEST.json',json.dumps(records,indent=2)+'\n')
write(payload/'VALIDATION.json',json.dumps({'version':report['Version'],'menu':report,
      'consumerOfflineCommands':len(consumer),'gameplayAcceptance':'pending',
      'muoniumBase':'frozen 0.6.0-test4 sources; only binding files applied',
      'containerPreviewLocal51Launcher':'unreviewed image; fail-closed refusal verified'},indent=2)+'\n')
write(payload/'README.txt','Keyboard binding test candidates / 快捷鍵測試候選版\n\n'
      'Read TESTING.md first. Install ModMenu manually, then only the plugin updates you want to test.\n'
      'Keep existing JSON settings and runtime files. No automatic installation or GitHub upload.\n'
      '先閱讀 TESTING.md；手動更新 ModMenu，再選擇要測試的插件。保留原有 JSON 與執行檔案。\n'
      'BedrockToolNative test5 pins the older ModMenu hash; its Workspace needs a separate companion update.\n')
sums(main)
sums(payload)
mainzip=out/'ModMenuBedrock-0.6.0-test5-leftalt-win-x64.zip'
archive(main,mainzip)
combined=out/'ModMenu-and-plugin-bindings-testpack-win-x64.zip'
archive(payload,combined)
subprocess.run(['py','-3',str(root/'tools/verify_artifacts.py'),'--dll',str(main/dll.name),'--workspace-api',
                '--zip',str(mainzip),'--payload',str(main)],check=True,cwd=root)
with zipfile.ZipFile(combined) as z:
    expected={payload.name+'/'+f.relative_to(payload).as_posix():f for f in payload.rglob('*') if f.is_file()}
    assert set(z.namelist())==set(expected)
    for name,f in expected.items():assert hashlib.sha256(z.read(name)).hexdigest().upper()==digest(f),name

names=subprocess.check_output(['git','ls-files','--cached','--others','--exclude-standard','-z'],cwd=root).decode('utf-8').split('\0')
sourcezip=out/'ModMenuBedrock-0.6.0-test5-leftalt-source.zip'
with zipfile.ZipFile(sourcezip,'x',compression=zipfile.ZIP_DEFLATED) as z:
    for name in sorted(set(names)-{''}):z.write(root/name,name)
for f in [mainzip,combined,sourcezip]:write(Path(str(f)+'.sha256'),digest(f)+'  '+f.name+'\n')
copy(root/'artifacts/verification.json',out/'verification.json')
write(out/'DLL-MANIFEST.json',json.dumps(records,indent=2)+'\n')
write(root/'artifacts/bindings-latest-package.json',json.dumps({'directory':str(out),'main':str(mainzip),
      'combined':str(combined),'source':str(sourcezip),'dlls':records},indent=2)+'\n')
print('Frozen binding candidates: '+str(out))
print(json.dumps(records,indent=2))
