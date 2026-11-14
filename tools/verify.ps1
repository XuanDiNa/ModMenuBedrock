[CmdletBinding()]
param(
 [Parameter(Mandatory)][string]$GameImagesRoot,
 [Parameter(Mandatory)][string]$LlvmBin,
 [string]$SdkRoot='', [string]$MenuFixtures='', [string]$LauncherFixtures='',
 [ValidateSet('debug','release')][string[]]$Modes=@('debug','release')
)
$ErrorActionPreference='Stop'
$project=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$GameImagesRoot=(Resolve-Path -LiteralPath $GameImagesRoot).Path
$LlvmBin=(Resolve-Path -LiteralPath $LlvmBin).Path
if($SdkRoot){$SdkRoot=(Resolve-Path -LiteralPath $SdkRoot).Path}
if($MenuFixtures){$MenuFixtures=(Resolve-Path -LiteralPath $MenuFixtures).Path}
if($LauncherFixtures){$LauncherFixtures=(Resolve-Path -LiteralPath $LauncherFixtures).Path}
function Run([string]$Exe,[string[]]$Arguments){& $Exe @Arguments;if($LASTEXITCODE -ne 0){throw "$Exe failed: $LASTEXITCODE"}}
function RunToolchain([string[]]$Arguments){& (Join-Path $PSScriptRoot 'with_toolchain.ps1') -LlvmBin $LlvmBin @Arguments;if($LASTEXITCODE -ne 0){throw "Toolchain command failed: $LASTEXITCODE"}}
Push-Location -LiteralPath $project
try{
 Run 'py' @('-3','tools/generate_regression.py')
 foreach($mode in $Modes){
  $cfg=@('f','-y','-m',$mode,'--ll_sdk=y','--target_type=client')
  if($SdkRoot){$cfg+=('--ll_sdk_root='+$SdkRoot)}
  RunToolchain $cfg
  foreach($target in @('ModMenuBedrockLL','ModMenuLaminaLoaderFixture','ModMenuLaminaLoaderTests','ModMenuLaminaLifecycleTests')){
   RunToolchain @('build','-y','-j','4',$target)
  }
  # xmake may discover the parent project even after Push-Location. Bind both
  # paths explicitly so fresh regression binaries are built where we run them.
  $uiProject=Join-Path $project 'artifacts/regression'
  $uiFile=Join-Path $uiProject 'xmake.lua'
  Run 'xmake' @('f','-P',$uiProject,'-F',$uiFile,'-y','-m',$mode,'--ll_sdk=n')
  Run 'xmake' @('build','-P',$uiProject,'-F',$uiFile,'-y','-j','4')
  $verify=@('-3','tools/verify_lamina.py','--ui-bin',"artifacts/regression/bin/$mode",'--native-bin',"build/windows/x64/$mode",'--fixture',"build/fixture/$mode/LeviLamina.dll",'--images',$GameImagesRoot,'--report',"artifacts/verification-$mode.json")
  if($MenuFixtures){$verify+=@('--menu-fixtures',$MenuFixtures)}
  if($LauncherFixtures){$verify+=@('--launcher-fixtures',$LauncherFixtures)}
  Run 'py' $verify
 }
 foreach($script in @('verify_integration.py','verify_slider_schema.py','verify_workspaces.py')){Run 'py' @('-3',"tools/$script")}
 $pack=@('-3','tools/verify_resource_pack_entry.py');if($MenuFixtures){$pack+=@('--fixtures',$MenuFixtures)}
 Run 'py' $pack
} finally{Pop-Location}
