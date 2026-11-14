<#
Read-only verification of a matched binding bundle. No copying or process control.
The manifest is distributed beside this script as DLL-MANIFEST.json.
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$ModsPath,
    [string]$ManifestPath
)
$ErrorActionPreference = 'Stop'
$failures = [System.Collections.Generic.List[string]]::new()
function Get-BindingSHA256([string]$Path) {
    $stream = [System.IO.File]::OpenRead($Path)
    $hasher = [System.Security.Cryptography.SHA256]::Create()
    try {
        return ([System.BitConverter]::ToString($hasher.ComputeHash($stream))).Replace('-', '')
    } finally {
        $hasher.Dispose()
        $stream.Dispose()
    }
}
try {
    if (-not $ManifestPath) {
        $checkerDirectory = Split-Path -Parent $MyInvocation.MyCommand.Path
        $ManifestPath = Join-Path $checkerDirectory 'DLL-MANIFEST.json'
    }
    $modsRoot = (Resolve-Path -LiteralPath $ModsPath).ProviderPath
    if (-not (Test-Path -LiteralPath $modsRoot -PathType Container)) { throw 'ModsPath must be a directory.' }
    $bundle = Get-Content -LiteralPath $ManifestPath -Raw -Encoding UTF8 | ConvertFrom-Json
    if ($bundle.schemaVersion -ne 1 -or @($bundle.plugins).Count -ne 8) { throw 'Unsupported or incomplete bundle manifest.' }
    foreach ($plugin in $bundle.plugins) {
        $before = $failures.Count
        if ($plugin.plugin -notmatch '^[A-Za-z0-9]+$' -or $plugin.entry -notmatch '^[A-Za-z0-9]+\.dll$') { throw 'Unsafe manifest entry.' }
        $pluginRoot = Join-Path $modsRoot $plugin.plugin
        if (-not (Test-Path -LiteralPath $pluginRoot -PathType Container)) {
            $failures.Add("$($plugin.plugin): folder is missing")
            continue
        }
        foreach ($file in $plugin.files) {
            # Only the declared DLL and matching public metadata are read.
            if ($file.path -notin @($plugin.entry, 'manifest.json', 'mod.json', 'modmenu.settings.json')) { throw 'Unexpected manifest file.' }
            $path = Join-Path $pluginRoot $file.path
            if (-not (Test-Path -LiteralPath $path -PathType Leaf)) {
                $failures.Add("$($plugin.plugin): missing $($file.path)")
                continue
            }
            $item = Get-Item -LiteralPath $path
            if ($item.Length -ne $file.bytes -or (Get-BindingSHA256 $path) -ne $file.sha256) {
                $failures.Add("$($plugin.plugin): $($file.path) size/SHA-256 mismatch")
            }
        }
        try {
            $manifest = Get-Content -LiteralPath (Join-Path $pluginRoot 'manifest.json') -Raw -Encoding UTF8 | ConvertFrom-Json
            if ($manifest.version -cne $plugin.version -or $manifest.entry -cne $plugin.entry) {
                $failures.Add("$($plugin.plugin): installed manifest version/entry is not the matching candidate")
            }
            if ($plugin.plugin -ne 'ModMenuBedrock') {
                $settings = Get-Content -LiteralPath (Join-Path $pluginRoot 'modmenu.settings.json') -Raw -Encoding UTF8 | ConvertFrom-Json
                if ($settings.schemaVersion -ne 5 -or $settings.plugin.id -cne $plugin.plugin -or
                    $settings.plugin.entry -cne $plugin.entry -or $plugin.version -cnotin @($settings.plugin.versions)) {
                    $failures.Add("$($plugin.plugin): schema v5 description does not match the DLL identity")
                }
            }
        } catch {
            $failures.Add("$($plugin.plugin): missing or invalid JSON metadata ($($_.Exception.Message))")
        }
        if ($failures.Count -eq $before) { Write-Host "OK $($plugin.plugin) $($plugin.version)" }
    }
    # A second copy in a sibling loader directory can defeat an otherwise correct update.
    foreach ($plugin in $bundle.plugins) {
        $copies = @()
        $direct = Join-Path $modsRoot $plugin.entry
        if (Test-Path -LiteralPath $direct -PathType Leaf) { $copies += $direct }
        foreach ($directory in Get-ChildItem -LiteralPath $modsRoot -Directory) {
            $path = Join-Path $directory.FullName $plugin.entry
            if (Test-Path -LiteralPath $path -PathType Leaf) { $copies += $path }
        }
        if ($copies.Count -gt 1) { $failures.Add("$($plugin.plugin): multiple DLL copies in the active mods directory; keep backups elsewhere") }
    }
    if ($failures.Count) {
        foreach ($failure in $failures) { Write-Host "FAIL $failure" }
        Write-Host "FAILED: $($failures.Count) mismatch(es). No files were changed."
        exit 1
    }
    Write-Host 'PASS: all 8 matching DLL/manifest/description sets verified. No files were changed.'
    Write-Host 'Offline identity check only. Restart Minecraft after updating DLLs; gameplay still needs manual testing.'
    exit 0
} catch {
    Write-Host "ERROR: $($_.Exception.Message)"
    exit 2
}
