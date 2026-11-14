[CmdletBinding()]
param()
$ErrorActionPreference='Stop'
Set-StrictMode -Version Latest
$project=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
& py -3 (Join-Path $project 'tools/verify_workspace_source.py')
if ($LASTEXITCODE -ne 0) { throw 'Candidate source audit failed' }
$report=Get-Content -LiteralPath (Join-Path $project 'artifacts/verification.json') -Raw | ConvertFrom-Json
if (!$report.Success -or $report.Version -ne '0.6.0-test3' -or $report.Modes.Count -ne 2 -or @($report.Modes | Where-Object { $_.Suites -ne 21 }).Count) { throw 'Fresh complete candidate verification required' }
$dll=Join-Path $project 'build/windows/x64/release/ModMenuBedrock.dll'
$release=$report.Modes | Where-Object { $_.Mode -eq 'release' }
if ((Get-FileHash -LiteralPath $dll -Algorithm SHA256).Hash -ne $release.RebuiltDLLSHA256) { throw 'DLL differs from verified candidate' }
$packageRoot=Join-Path $project ('releases/ModMenuBedrock-0.6.0-test3-workspaces-'+(Get-Date -Format 'yyyyMMdd-HHmmss'))
if (Test-Path -LiteralPath $packageRoot) { throw 'Refusing to overwrite a frozen package' }
$payload=Join-Path $packageRoot 'ModMenuBedrock'
$null=New-Item -ItemType Directory -Path $payload
$files=@('manifest.json','mod.json','LICENSE','THIRD_PARTY_NOTICES.txt','integration/WORKSPACES.md','integration/README.md','integration/SLIDERS.md','integration/modmenu.settings.schema.json','integration/modmenu.settings.v2.schema.json','integration/modmenu.settings.v3.schema.json','integration/sdk/modmenu_workspace_v1.h','docs/workspaces-testing.md','docs/workspaces-review.md','integration/workspace-example/Example.cpp','integration/workspace-example/manifest.json','integration/workspace-example/mod.json','integration/workspace-example/config.json','integration/workspace-example/modmenu.settings.json')
Copy-Item -LiteralPath $dll -Destination (Join-Path $payload 'ModMenuBedrock.dll')
foreach ($relative in $files) {
 $target=Join-Path $payload $relative
 $null=New-Item -ItemType Directory -Path (Split-Path -Parent $target) -Force
 Copy-Item -LiteralPath (Join-Path $project $relative) -Destination $target
}
function Sums([string]$Root) {
 $lines=Get-ChildItem -LiteralPath $Root -File -Recurse | Where-Object Name -ne 'SHA256SUMS.txt' | Sort-Object FullName | ForEach-Object { '{0}  {1}' -f (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash,([IO.Path]::GetRelativePath($Root,$_.FullName).Replace('\','/')) }
 [IO.File]::WriteAllLines((Join-Path $Root 'SHA256SUMS.txt'),[string[]]$lines,[Text.UTF8Encoding]::new($false))
}
Sums $payload
$zip=Join-Path $packageRoot 'ModMenuBedrock-0.6.0-test3-workspaces-win-x64.zip'
Compress-Archive -LiteralPath $payload -DestinationPath $zip
& py -3 (Join-Path $project 'tools/verify_artifacts.py') --dll (Join-Path $payload 'ModMenuBedrock.dll') --workspace-api --zip $zip --payload $payload
if ($LASTEXITCODE -ne 0) { throw 'Candidate ZIP audit failed' }
$exampleRoot=Join-Path $packageRoot 'optional-example/ModMenuWorkspaceExample'
$null=New-Item -ItemType Directory -Path $exampleRoot -Force
foreach ($name in @('manifest.json','mod.json','config.json','modmenu.settings.json')) { Copy-Item -LiteralPath (Join-Path $project "integration/workspace-example/$name") -Destination (Join-Path $exampleRoot $name) }
foreach ($name in @('LICENSE','THIRD_PARTY_NOTICES.txt')) { Copy-Item -LiteralPath (Join-Path $project $name) -Destination (Join-Path $exampleRoot $name) }
Copy-Item -LiteralPath (Join-Path $project 'build/windows/x64/release/ModMenuWorkspaceExample.dll') -Destination (Join-Path $exampleRoot 'ModMenuWorkspaceExample.dll')
Sums $exampleRoot
$exampleZip=Join-Path $packageRoot 'ModMenuWorkspaceExample-1.0.0-optional.zip'
Compress-Archive -LiteralPath $exampleRoot -DestinationPath $exampleZip
& py -3 (Join-Path $project 'tools/verify_artifacts.py') --dll (Join-Path $exampleRoot 'ModMenuWorkspaceExample.dll') --workspace-example --zip $exampleZip --payload $exampleRoot
if ($LASTEXITCODE -ne 0) { throw 'Optional example ZIP audit failed' }
& (Join-Path $project 'build/windows/x64/release/ModMenuWorkspaceAbiHost.exe') (Join-Path $project 'build/windows/x64/release/ModMenuWorkspaceAbiFixture.dll') (Join-Path $exampleRoot 'ModMenuWorkspaceExample.dll')
if ($LASTEXITCODE -ne 0) { throw 'Frozen example ABI check failed' }
foreach ($archive in @($zip,$exampleZip)) { [IO.File]::WriteAllText(($archive+'.sha256'),((Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash+'  '+[IO.Path]::GetFileName($archive)+"`n"),[Text.UTF8Encoding]::new($false)) }
Copy-Item -LiteralPath (Join-Path $project 'artifacts/verification.json') -Destination (Join-Path $packageRoot 'verification.json')
Write-Output "Frozen candidate: $packageRoot"
Write-Output 'No installed files were changed. No GitHub upload or Minecraft launch.'
