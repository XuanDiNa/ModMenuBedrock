$ErrorActionPreference='Stop'
. (Join-Path $PSScriptRoot '../tools/Collect-ModMenuLL-Diagnostics.ps1')
$taskTemp=Join-Path ([IO.Path]::GetTempPath()) ('MMB-CollectorTests-'+[Guid]::NewGuid().ToString('N'))
New-Item -ItemType Directory -Path $taskTemp | Out-Null
$script:checks=0
$script:failures=0
function Check([bool]$Condition,[string]$Name){
    $script:checks++
    if(-not $Condition){$script:failures++;Write-Host "FAIL $Name"}
}
try{
    $file=Join-Path $taskTemp 'fixture.exe'
    [byte[]]$bytes=New-Object byte[] 1028
    $stream=[IO.MemoryStream]::new($bytes,$true)
    $writer=[IO.BinaryWriter]::new($stream)
    $writer.Write([uint16]0x5A4D);$stream.Position=60;$writer.Write([int]128)
    $stream.Position=128;$writer.Write([uint32]0x4550);$writer.Write([uint16]0x8664);$writer.Write([uint16]2)
    $writer.Write([uint32]0x6AD79EE4);$stream.Position=148;$writer.Write([uint16]240)
    $stream.Position=152;$writer.Write([uint16]0x20B)
    $stream.Position=168;$writer.Write([uint32]0x1000)
    $stream.Position=176;$writer.Write([uint64]0x140000000)
    $stream.Position=208;$writer.Write([uint32]0x3000)
    $stream.Position=216;$writer.Write([uint32]0x12345678);$writer.Write([uint16]2)
    $stream.Position=392;$writer.Write([Text.Encoding]::ASCII.GetBytes(".text`0`0`0"));$writer.Write([uint32]4);$writer.Write([uint32]0x1000);$writer.Write([uint32]4);$writer.Write([uint32]512)
    $stream.Position=428;$writer.Write([uint32]0x60000020)
    $stream.Position=432;$writer.Write([Text.Encoding]::ASCII.GetBytes(".data`0`0`0"));$writer.Write([uint32]4);$writer.Write([uint32]0x2000);$writer.Write([uint32]4);$writer.Write([uint32]1024)
    $stream.Position=468;$writer.Write([uint32]3221225536)
    $stream.Position=512;$writer.Write([Text.Encoding]::ASCII.GetBytes('abcd'))
    $stream.Position=1024;$writer.Write([Text.Encoding]::ASCII.GetBytes('1234'));$writer.Dispose();$stream.Dispose()
    [IO.File]::WriteAllBytes($file,$bytes)
    $before=(Get-FileHash -LiteralPath $file).Hash
    $r=Get-ModMenuFileFingerprint -Path $file
    Check ($null -ne $r -and $r.status -eq 'ok') 'readable file produces a report'
    Check ($null -ne $r -and $r.sha256 -eq $before -and $r.bytes -eq 1028) 'complete on-disk identity'
    Check ($null -ne $r -and $r.pe.valid -and $r.pe.machine -eq '8664' -and $r.pe.timestamp -eq '6AD79EE4' -and $r.pe.imageSize -eq '00003000' -and $r.pe.checksum -eq '12345678' -and $r.pe.subsystem -eq 2) 'parse exact PE identity fields'
    $text=@(if($r){$r.pe.sections | Where-Object {$_.name -eq '.text'}})
    Check ($text.Count -eq 1 -and $text[0].sha256 -eq '88D4266FD4E6338D13B845FCF289579D209C897823B9217DA3E161936F031589' -and $text[0].bytes -eq 4 -and $text[0].rva -eq '00001000') 'hash declared raw text bytes only'
    Check ($null -ne $r -and -not $r.reference.reviewedGameImage) 'unknown executable is evidence only, never admission'
    Check ((Get-FileHash -LiteralPath $file).Hash -eq $before) 'collector leaves source file unchanged'
    $invalid=Join-Path $taskTemp 'invalid.bin';[IO.File]::WriteAllBytes($invalid,[Text.Encoding]::ASCII.GetBytes('abc'))
    $bad=Get-ModMenuFileFingerprint -Path $invalid
    Check ($null -ne $bad -and $bad.status -eq 'ok' -and -not $bad.pe.valid -and $bad.sha256 -eq 'BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD') 'malformed PE remains identifiable without parsing unsafe offsets'
    $bytes[60]=255;$bytes[61]=255;$bytes[62]=255;$bytes[63]=127;[IO.File]::WriteAllBytes($invalid,$bytes)
    $bad=Get-ModMenuFileFingerprint -Path $invalid
    Check ($null -ne $bad -and -not $bad.pe.valid) 'out-of-bounds NT offset is refused'
    $bytes[60]=128;$bytes[61]=0;$bytes[62]=0;$bytes[63]=0
    $bytes[412]=255;$bytes[413]=255;$bytes[414]=255;$bytes[415]=127;[IO.File]::WriteAllBytes($invalid,$bytes)
    $bad=Get-ModMenuFileFingerprint -Path $invalid
    Check ($null -ne $bad -and -not $bad.pe.valid) 'out-of-bounds section cannot read beyond file'
    $missing=Get-ModMenuFileFingerprint -Path (Join-Path $taskTemp 'missing.exe')
    Check ($null -ne $missing -and $missing.status -eq 'unavailable' -and -not $missing.Contains('path')) 'missing files report bounded error without private paths'
    if($r){Check ((ConvertTo-Json -InputObject $r -Depth 12) -notmatch [regex]::Escape($taskTemp)) 'fingerprint excludes absolute paths'}
    $game=Join-Path $taskTemp 'Minecraft.Windows.exe';Copy-Item -LiteralPath $file -Destination $game
    $report=Get-ModMenuDiagnosticReport -GameExe $game
    Check ($report.readOnly -and -not $report.admissionPolicyChanged -and -not $report.loadedMemoryBytesCollected -and $report.gameExe.sha256 -eq $before) 'full report uses selected file without granting authority'
    Check ($report.installedFiles.Count -eq 3 -and @($report.installedFiles | Where-Object {$_.onDiskFile.status -ne 'unavailable'}).Count -eq 0) 'only three known plugin file locations are inspected'
    $json=ConvertTo-Json -InputObject $report -Depth 16
    Check ($json -notmatch [regex]::Escape($taskTemp)) 'full report redacts instance paths'
    $output=Join-Path $taskTemp 'report.json';Invoke-ModMenuLLDiagnostic -GameExe $game -OutputPath $output
    $saved=Get-Content -LiteralPath $output -Raw | ConvertFrom-Json
    Check ($saved.gameExe.sha256 -eq $before) 'entry command creates a valid JSON report'
    $prior=(Get-FileHash -LiteralPath $output).Hash;$refused=$false
    try{Invoke-ModMenuLLDiagnostic -GameExe $game -OutputPath $output}catch{$refused=$true}
    Check ($refused -and (Get-FileHash -LiteralPath $output).Hash -eq $prior) 'entry command never overwrites an existing report'
    Check ((Get-FileHash -LiteralPath $game).Hash -eq $before) 'full collection leaves selected executable unchanged'
    Write-Host "$script:checks collector checks / $script:failures failures"
}finally{
    if([IO.Path]::GetFullPath($taskTemp).StartsWith([IO.Path]::GetTempPath(),[StringComparison]::OrdinalIgnoreCase)){
        Remove-Item -LiteralPath $taskTemp -Recurse -Force
    }
}
if($script:failures){exit 1}
