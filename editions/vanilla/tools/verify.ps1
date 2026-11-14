[CmdletBinding()]
param([string]$GameImagesRoot = '', [string]$MenuFixtures = '', [ValidateSet('debug','release')][string[]]$Modes=@('debug','release'), [switch]$Incremental)
$ErrorActionPreference = 'Stop'
Set-StrictMode -Version Latest
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
function Run([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Exe failed with exit code $LASTEXITCODE" }
}
if ($GameImagesRoot) { $GameImagesRoot = (Resolve-Path -LiteralPath $GameImagesRoot).Path }
if ($MenuFixtures) { $MenuFixtures = (Resolve-Path -LiteralPath $MenuFixtures).Path }
Push-Location -LiteralPath $project
try {
    Run 'py' @('-3', 'tools/verify_integration.py')
    Run 'py' @('-3', 'tools/verify_slider_schema.py')
    Run 'py' @('-3', 'tools/verify_workspaces.py')
    $report = [ordered]@{Version='0.6.0-test5-leftalt';Success=$false;Modes=@();PrivateImageChecks=[bool]$GameImagesRoot;PrivateMenuFixtures=[bool]$MenuFixtures}
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
            Run (Join-Path $bin "ModMenu$($test)Tests.exe") @()
        }
        $nativeArgs = @(); if ($GameImagesRoot) { $nativeArgs = @($GameImagesRoot) }
        Run (Join-Path $bin 'ModMenuNativeTests.exe') $nativeArgs
        $definitionArgs = @(); if ($MenuFixtures) { $definitionArgs = @($MenuFixtures) }
        Run (Join-Path $bin 'ModMenuDefinitionTests.exe') $definitionArgs
        Run (Join-Path $bin 'ModMenuHookTests.exe') @((Join-Path $bin 'ModMenuHookFixture.dll'))
        Run (Join-Path $bin 'ModMenuPointerChainTests.exe') @((Join-Path $bin 'ModMenuHookFixture.dll'))
        Run (Join-Path $bin 'ModMenuPointerBridgeTests.exe') @()
        Run (Join-Path $bin 'ModMenuSmokeTests.exe') @((Join-Path $bin 'ModMenuBedrock.dll'))
        Run (Join-Path $bin 'ModMenuWorkspaceApiTests.exe') @((Join-Path $bin 'ModMenuBedrock.dll'))
        Run (Join-Path $bin 'ModMenuWorkspaceAbiHost.exe') @((Join-Path $bin 'ModMenuWorkspaceAbiFixture.dll'), (Join-Path $bin 'ModMenuWorkspaceExample.dll'))
        Run 'py' @('-3', 'tools/verify_artifacts.py', '--dll', (Join-Path $bin 'ModMenuBedrock.dll'), '--workspace-api')
        Run 'py' @('-3', 'tools/verify_bindings.py', (Join-Path $bin 'ModMenuBindingTests.exe'))
        $report.Modes += [pscustomobject]@{Mode=$mode;Suites=22;RebuiltDLLSHA256=(Get-FileHash -LiteralPath (Join-Path $bin 'ModMenuBedrock.dll') -Algorithm SHA256).Hash}
    }
    $report.Success = $true
    $null = New-Item -ItemType Directory -Path (Join-Path $project 'artifacts') -Force
    [IO.File]::WriteAllText((Join-Path $project 'artifacts/verification.json'), ($report | ConvertTo-Json -Depth 5), [Text.UTF8Encoding]::new($false))
    Write-Output "Clean $($Modes -join '/') verification passed: 22 suites per mode. No game process was started."
} finally { Pop-Location }
