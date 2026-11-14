"""Run offline UI/native/real-DLL loader suites; never launches Minecraft."""
import argparse,json,subprocess,sys
from pathlib import Path
p=argparse.ArgumentParser()
p.add_argument('--ui-bin',type=Path,required=True)
p.add_argument('--native-bin',type=Path,required=True)
p.add_argument('--fixture',type=Path,required=True)
p.add_argument('--images',type=Path,required=True)
p.add_argument('--menu-fixtures',type=Path)
p.add_argument('--launcher-fixtures',type=Path,help='Private independently reproduced PreLoader-first images')
p.add_argument('--report',type=Path,required=True)
a=p.parse_args();root=Path(__file__).resolve().parents[1]
b=a.ui_bin.resolve();ll=a.native_bin.resolve();fixture=a.fixture.resolve();images=a.images.resolve()
cases=[]
for name in ['Core','Storage','View','NativeCallback','Settings','Isolated','AdapterUi','Slider','TextEditor','Workspace','WorkspaceUi','WorkspacePointer','Focus','Binding']:
    args=[str(b/f'ModMenu{name}Tests.exe')]
    if name=='NativeCallback' and a.menu_fixtures:args.append(str(a.menu_fixtures.resolve()))
    cases.append((name,args))
cases += [('legacy-fixtures',[str(b/'ModMenuNativeTests.exe'),str(images)]),('LL-native',[str(b/'ModMenuLaminaNativeTests.exe'),str(images/'1.26.60.05')]+([str(a.launcher_fixtures.resolve())] if a.launcher_fixtures else [])),('LL-lifecycle',[str(ll/'ModMenuLaminaLifecycleTests.exe')]),('definition',[str(b/'ModMenuDefinitionTests.exe')]+([str(a.menu_fixtures.resolve())] if a.menu_fixtures else [])),('hooks',[str(b/'ModMenuHookTests.exe'),str(b/'ModMenuHookFixture.dll')]),('pointer-chain',[str(b/'ModMenuPointerChainTests.exe'),str(b/'ModMenuHookFixture.dll')]),('pointer-bridge',[str(b/'ModMenuPointerBridgeTests.exe')]),('Workspace-C-ABI',[str(b/'ModMenuWorkspaceAbiHost.exe'),str(b/'ModMenuWorkspaceAbiFixture.dll'),str(b/'ModMenuWorkspaceExample.dll')])]
for mode in ['missing','0','1','2','3']:
    cmd=[str(ll/'ModMenuLaminaLoaderTests.exe'),str(fixture),str(ll/'ModMenuBedrockLL.dll'),mode]
    if mode=='3':cmd.append(str(b/'ModMenuHookFixture.dll'))
    cases.append(('LL-loader-'+mode,cmd))
cases.append(('actual-DLL-PE',[sys.executable,str(root/'tools/verify_artifacts.py'),'--dll',str(ll/'ModMenuBedrockLL.dll'),'--workspace-api','--lamina']))
results=[]
for name,cmd in cases:
    result=subprocess.run(cmd,capture_output=True,text=True,encoding='utf-8',errors='replace',timeout=120)
    output=result.stdout+result.stderr
    results.append({'suite':name,'exit':result.returncode,'output':output})
    print(name, 'PASS' if result.returncode==0 else 'FAIL',output[-500:] if result.returncode==0 else output,flush=True)
report={'success':all(x['exit']==0 for x in results),'suites':len(results),'gameplay':'not run','private_menu_fixtures':bool(a.menu_fixtures),'launcher_order_fixtures':bool(a.launcher_fixtures),'results':results}
a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding='utf-8')
raise SystemExit(not report['success'])
