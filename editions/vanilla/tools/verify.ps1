[CmdletBinding()]
param([string]$GameImagesRoot = '', [string]$MenuFixtures = '', [string]$Reviewed51Image = '', [string[]]$MouseTweaksDLLs=@(), [ValidateSet('debug','release')][string[]]$Modes=@('debug','release'), [switch]$Incremental)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
function Run([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe failed with exit code $LASTEXITCODE" }
}
if ($GameImagesRoot) { $GameImagesRoot = (Resolve-Path -LiteralPath $GameImagesRoot).Path }
if ($MenuFixtures) { $MenuFixtures = (Resolve-Path -LiteralPath $MenuFixtures).Path }
if ($Reviewed51Image) {
    if (!$GameImagesRoot) { throw 'Reviewed51Image also requires GameImagesRoot' }
    $Reviewed51Image = (Resolve-Path -LiteralPath $Reviewed51Image).Path
}
$mouseInputs = @($MouseTweaksDLLs | ForEach-Object {
    $path = (Resolve-Path -LiteralPath $_).Path
    [pscustomobject]@{Path=$path;Size=(Get-Item -LiteralPath $path).Length;SHA256=(Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash}
})
$suites = if ($mouseInputs.Count) { 24 } else { 23 }
Push-Location -LiteralPath $project
try {
    Run 'py' @('-3', 'tools/verify_integration.py')
    Run 'py' @('-3', 'tools/verify_slider_schema.py')
    Run 'py' @('-3', 'tools/verify_workspaces.py')
    $packArgs = @('-3', 'tools/verify_resource_pack_entry.py'); if ($MenuFixtures) { $packArgs += @('--fixtures', $MenuFixtures) }
    Run 'py' $packArgs
    $report = [ordered]@{Version='0.6.0-test10-mouse51';Success=$false;Modes=@();PrivateImageChecks=[bool]$GameImagesRoot;PrivateMenuFixtures=[bool]$MenuFixtures;Reviewed51ImageChecks=[bool]$Reviewed51Image;MouseTweaksImages=$mouseInputs}
    foreach ($mode in $Modes) {
        if ($Incremental) {
            Run 'xmake' @('f', '-P', $project, '-y', '-m', $mode)
            Run 'xmake' @('build', '-P', $project, '-y', '-j', '4')
        } else {
            Run 'xmake' @('f', '-P', $project, '-c', '-y', '-m', $mode)
            Run 'xmake' @('build', '-P', $project, '-r', '-y', '-j', '4')
        }
        $bin = Join-Path $project "build/windows/x64/$mode"
        foreach ($test in @('Core', 'Storage', 'View', 'NativeCallback', 'Settings', 'Isolated', 'AdapterUi', 'Slider', 'TextEditor', 'Workspace', 'WorkspaceUi', 'WorkspacePointer', 'Focus', 'Binding')) {
            $testArgs = @(); if ($test -eq 'NativeCallback' -and $MenuFixtures) { $testArgs = @($MenuFixtures) }
            Run (Join-Path $bin "ModMenu$($test)Tests.exe") $testArgs
        }
        $nativeArgs = @(); if ($GameImagesRoot) { $nativeArgs = @($GameImagesRoot) }
        Run (Join-Path $bin 'ModMenuNativeTests.exe') $nativeArgs
        $variantArgs = @()
        if ($Reviewed51Image) {
            $variantArgs = @($Reviewed51Image, (Join-Path $GameImagesRoot '1.26.51.01/Minecraft.Windows.exe.orig'), (Join-Path $GameImagesRoot '1.26.51.01/Minecraft.Windows.exe'))
        }
        Run (Join-Path $bin 'ModMenuNative51VariantsTests.exe') $variantArgs
        $definitionArgs = @(); if ($MenuFixtures) { $definitionArgs = @($MenuFixtures) }
        Run (Join-Path $bin 'ModMenuDefinitionTests.exe') $definitionArgs
        Run (Join-Path $bin 'ModMenuHookTests.exe') @((Join-Path $bin 'ModMenuHookFixture.dll'))
        Run (Join-Path $bin 'ModMenuPointerChainTests.exe') @((Join-Path $bin 'ModMenuHookFixture.dll'))
        foreach ($mouse in $mouseInputs) {
            foreach ($case in @('accepted','tampered','hash-tampered')) {
                Run (Join-Path $bin 'ModMenuMouseTweaksAdmissionTests.exe') @($mouse.Path,$case)
            }
        }
        Run (Join-Path $bin 'ModMenuPointerBridgeTests.exe') @()
        Run (Join-Path $bin 'ModMenuSmokeTests.exe') @((Join-Path $bin 'ModMenuBedrock.dll'))
        Run (Join-Path $bin 'ModMenuWorkspaceApiTests.exe') @((Join-Path $bin 'ModMenuBedrock.dll'))
        Run (Join-Path $bin 'ModMenuWorkspaceAbiHost.exe') @((Join-Path $bin 'ModMenuWorkspaceAbiFixture.dll'), (Join-Path $bin 'ModMenuWorkspaceExample.dll'))
        Run 'py' @('-3', 'tools/verify_artifacts.py', '--dll', (Join-Path $bin 'ModMenuBedrock.dll'), '--workspace-api')
        Run 'py' @('-3', 'tools/verify_bindings.py', (Join-Path $bin 'ModMenuBindingTests.exe'))
        foreach ($mouse in $mouseInputs) {
            if ((Get-FileHash -LiteralPath $mouse.Path -Algorithm SHA256).Hash -ne $mouse.SHA256) { throw 'MouseTweaks input changed during verification' }
        }
        $report.Modes += [pscustomobject]@{Mode=$mode;Suites=$suites;RebuiltDLLSHA256=(Get-FileHash -LiteralPath (Join-Path $bin 'ModMenuBedrock.dll') -Algorithm SHA256).Hash}
    }
    $report.Success = $true
    $null = New-Item -ItemType Directory -Path (Join-Path $project 'artifacts') -Force
    [IO.File]::WriteAllText((Join-Path $project 'artifacts/verification.json'), ($report | ConvertTo-Json -Depth 5), [Text.UTF8Encoding]::new($false))
    Write-Output "Clean $($Modes -join '/') verification passed: $suites suites per mode. No game process was started."
} finally { Pop-Location }
