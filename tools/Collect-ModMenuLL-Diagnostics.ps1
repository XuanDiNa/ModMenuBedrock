[CmdletBinding()]
param([string]$GameExe='', [string]$OutputPath='')

function Get-ModMenuStreamHash {
    param([IO.FileStream]$Stream,[long]$Offset,[long]$Count)
    $Stream.Position=$Offset
    $sha=[Security.Cryptography.SHA256]::Create()
    try{
        [byte[]]$buffer=New-Object byte[] 1048576
        while($Count -gt 0){
            $read=$Stream.Read($buffer,0,[int][Math]::Min($Count,$buffer.Length))
            if($read -le 0){throw [IO.EndOfStreamException]::new()}
            [void]$sha.TransformBlock($buffer,0,$read,$buffer,0)
            $Count-=$read
        }
        [void]$sha.TransformFinalBlock([byte[]]@(),0,0)
        return [BitConverter]::ToString($sha.Hash).Replace('-','')
    }finally{$sha.Dispose()}
}

function Get-ModMenuPeFingerprint {
    param([IO.FileStream]$Stream)
    $invalid=[ordered]@{valid=$false;reason='invalid-or-unsupported-pe'}
    if($Stream.Length -lt 64){return $invalid}
    $Stream.Position=0
    [byte[]]$header=New-Object byte[] ([int][Math]::Min($Stream.Length,16384))
    $used=0
    while($used -lt $header.Length){
        $n=$Stream.Read($header,$used,$header.Length-$used)
        if($n -le 0){return $invalid};$used+=$n
    }
    if([BitConverter]::ToUInt16($header,0) -ne 0x5A4D){return $invalid}
    $nt=[BitConverter]::ToInt32($header,60)
    if($nt -lt 64 -or $nt -gt 4096 -or $nt+264 -gt $header.Length){return $invalid}
    $sections=[BitConverter]::ToUInt16($header,$nt+6)
    if([BitConverter]::ToUInt32($header,$nt) -ne 0x4550 -or $sections -lt 1 -or $sections -gt 96 -or
       [BitConverter]::ToUInt16($header,$nt+20) -ne 240 -or
       [BitConverter]::ToUInt16($header,$nt+24) -ne 0x20B -or
       $nt+264+$sections*40 -gt $header.Length){return $invalid}
    $rawSections=@()
    for($i=0;$i -lt $sections;$i++){
        $section=$nt+264+$i*40
        $name=[Text.Encoding]::ASCII.GetString($header,$section,8).TrimEnd([char]0)
        # Report standard section identifiers only, never arbitrary file strings.
        if($name -notmatch '^\.[A-Za-z0-9_]{1,7}$'){$name='other-section-'+$i}
        $rva=[BitConverter]::ToUInt32($header,$section+12)
        [long]$count=[BitConverter]::ToUInt32($header,$section+16)
        [long]$raw=[BitConverter]::ToUInt32($header,$section+20)
        if($raw -gt $Stream.Length -or $count -gt $Stream.Length-$raw){return $invalid}
        $rawSections+=,[ordered]@{name=$name;rva=('{0:X8}' -f $rva);rawOffset=$raw;bytes=$count;
            flags=('{0:X8}' -f [BitConverter]::ToUInt32($header,$section+36))}
    }
    # Diagnostic inputs must not cause unbounded re-hashing of overlapping ranges.
    $lastEnd=0L;$total=0L
    foreach($s in @($rawSections | Where-Object {$_.bytes -gt 0} | Sort-Object {[long]$_.rawOffset})){
        if($s.rawOffset -lt $lastEnd){return $invalid}
        $lastEnd=$s.rawOffset+$s.bytes;$total+=$s.bytes
        if($total -gt $Stream.Length){return $invalid}
    }
    foreach($s in $rawSections){$s['sha256']=Get-ModMenuStreamHash $Stream $s.rawOffset $s.bytes}
    return [ordered]@{
        valid=$true;machine=('{0:X4}' -f [BitConverter]::ToUInt16($header,$nt+4));
        timestamp=('{0:X8}' -f [BitConverter]::ToUInt32($header,$nt+8));
        imageBase=('{0:X16}' -f [BitConverter]::ToUInt64($header,$nt+48));
        imageSize=('{0:X8}' -f [BitConverter]::ToUInt32($header,$nt+80));
        checksum=('{0:X8}' -f [BitConverter]::ToUInt32($header,$nt+88));
        subsystem=[BitConverter]::ToUInt16($header,$nt+92);sections=@($rawSections)
    }
}

function Get-ModMenuFileFingerprint {
    param([Parameter(Mandatory)][string]$Path)
    $file=$null
    try{
        $file=[IO.File]::Open($Path,[IO.FileMode]::Open,[IO.FileAccess]::Read,([IO.FileShare]::Read -bor [IO.FileShare]::Delete))
        if($file.Length -gt 1073741824){return [ordered]@{status='unavailable';error='file-over-1GiB'}}
        $sha=Get-ModMenuStreamHash $file 0 $file.Length
        $pe=Get-ModMenuPeFingerprint $file
        $gameHashes=@(
            '988F6A61403B4381AA41E6C66D64CCE441B14E561A9B10F647C2758EF3F2672A',
            '019F4EE7C82DB0E0C187653BC76C61CF75502FF6866A5CC6603FA0C909A386A2',
            '807508E53ADE36497C30C95770016B9CABBC9DE9E7FA83130D28BA586B3C410F',
            'FAD0788DB090874B7BADC6353BBBCBD38FAC9172DA1F5D3F30771056E8A7A6A5'
        )
        $text=@($pe.sections | Where-Object {$_.name -eq '.text'})
        return [ordered]@{status='ok';bytes=$file.Length;sha256=$sha;pe=$pe;reference=[ordered]@{
            reviewedGameImage=($gameHashes -contains $sha);
            sameReviewed26_60Text=($text.Count -eq 1 -and $text[0].bytes -eq 250370560 -and $text[0].sha256 -eq '7CA5D1BAEF049979E9785D5BBAFE7100A829E1107A3D374975962CC64ECC76FE');
            releasedModMenuLLTest1=($sha -eq '9829BB748BC75A657C6AF592B776C3FFC938C5EBB36546CA129FE84DF129291E');
            officialLeviLamina26_60_7=($sha -eq 'D2BB36559FEBF88D8A28CE65BF063DB0372C9D4FD6BAEFCCAC7CCF900812FF19')
        }}
    }catch{
        return [ordered]@{status='unavailable';error='read-or-hash-failed';hresult=$_.Exception.HResult}
    }finally{if($file){$file.Dispose()}}
}

function Get-ModMenuDiagnosticReport {
    param([Parameter(Mandatory)][string]$GameExe)
    $resolved=[IO.Path]::GetFullPath($GameExe)
    if([IO.Path]::GetFileName($resolved) -ine 'Minecraft.Windows.exe' -or -not [IO.File]::Exists($resolved)){
        throw 'Select the actual Minecraft.Windows.exe used by the player.'
    }
    $instance=[IO.Path]::GetDirectoryName($resolved)
    $cache=@{}
    function Read-FileIdentity([string]$File){
        $key=[IO.Path]::GetFullPath($File)
        if(-not $cache.ContainsKey($key)){$cache[$key]=Get-ModMenuFileFingerprint $key}
        return $cache[$key]
    }
    $processes=@()
    foreach($process in @(Get-Process -Name 'Minecraft.Windows' -ErrorAction SilentlyContinue | Select-Object -First 8)){
        $matchesExe=$false;$pathAvailable=$false
        try{$path=$process.MainModule.FileName;$pathAvailable=(-not [string]::IsNullOrEmpty($path));$matchesExe=$pathAvailable -and [IO.Path]::GetFullPath($path).Equals($resolved,[StringComparison]::OrdinalIgnoreCase)}catch{}
        $modules=@();$moduleStatus='not-selected-instance'
        if($matchesExe){
            try{
                foreach($m in @($process.Modules)){
                    if($m.ModuleName -iin @('ModMenuBedrockLL.dll','ModMenuBedrock.dll','LeviLamina.dll')){
                        $relative='outside-selected-instance'
                        if($m.FileName.StartsWith($instance+'\',[StringComparison]::OrdinalIgnoreCase)){
                            # No usernames or arbitrary directory names in exported reports.
                            foreach($known in @('mods\ModMenuBedrockLL\ModMenuBedrockLL.dll','mods\ModMenuBedrock\ModMenuBedrock.dll','mods\LeviLamina\LeviLamina.dll')){
                                if($m.FileName.Equals((Join-Path $instance $known),[StringComparison]::OrdinalIgnoreCase)){$relative=$known.Replace('\','/')}
                            }
                            if($relative -eq 'outside-selected-instance'){$relative='other-path-in-selected-instance'}
                        }
                        $modules+=,[ordered]@{name=$m.ModuleName;location=$relative;onDiskFile=(Read-FileIdentity $m.FileName)}
                    }
                }
                $moduleStatus='enumerated'
            }catch{$moduleStatus='access-unavailable'}
        }
        $processes+=,[ordered]@{pid=$process.Id;imagePathAvailable=$pathAvailable;matchesSelectedGameExe=$matchesExe;moduleStatus=$moduleStatus;modules=@($modules)}
    }
    $installed=@()
    foreach($relative in @('mods\ModMenuBedrockLL\ModMenuBedrockLL.dll','mods\ModMenuBedrock\ModMenuBedrock.dll','mods\LeviLamina\LeviLamina.dll')){
        $path=Join-Path $instance $relative
        $manifestPath=Join-Path ([IO.Path]::GetDirectoryName($path)) 'manifest.json'
        $manifest=[ordered]@{status='missing-or-disabled'}
        if([IO.File]::Exists($manifestPath)){
            try{
                if((Get-Item -LiteralPath $manifestPath).Length -gt 65536){throw 'oversized'}
                $j=[IO.File]::ReadAllText($manifestPath) | ConvertFrom-Json
                $manifest=[ordered]@{status='present'}
                foreach($field in @('name','entry','version','type','platform')){
                    $value=[string]$j.$field
                    if($value -match '^[A-Za-z0-9_.+\-]{1,128}$'){$manifest[$field]=$value}
                }
            }catch{$manifest=[ordered]@{status='invalid-or-unreadable'}}
        }
        $installed+=,[ordered]@{location=$relative.Replace('\','/');manifest=$manifest;onDiskFile=(Read-FileIdentity $path)}
    }
    return [ordered]@{schemaVersion=1;tool='ModMenuLL-Diagnostics-1';collectedUtc=[DateTime]::UtcNow.ToString('o');
        readOnly=$true;admissionPolicyChanged=$false;loadedMemoryBytesCollected=$false;
        gameExe=(Read-FileIdentity $resolved);runningProcesses=@($processes);installedFiles=@($installed);
        scope='Only selected EXE metadata, SHA-256/section hashes, three known plugin identities and matching process module paths (redacted). No logs, game content, settings, credentials or memory dumped.'}
}

function Invoke-ModMenuLLDiagnostic {
    param([string]$GameExe='', [string]$OutputPath='')
    if([string]::IsNullOrWhiteSpace($GameExe)){
        Add-Type -AssemblyName System.Windows.Forms
        $dialog=[Windows.Forms.OpenFileDialog]::new()
        try{
            $dialog.Title='Select the Minecraft.Windows.exe used by this instance'
            $dialog.Filter='Minecraft.Windows.exe|Minecraft.Windows.exe'
            $dialog.CheckFileExists=$true
            if($dialog.ShowDialog() -ne [Windows.Forms.DialogResult]::OK){Write-Host 'Cancelled. Nothing collected.';return}
            $GameExe=$dialog.FileName
        }finally{$dialog.Dispose()}
    }
    if([string]::IsNullOrWhiteSpace($OutputPath)){
        $OutputPath=Join-Path $PSScriptRoot ('ModMenuLL-report-'+[DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')+'.json')
    }
    $fullOutput=[IO.Path]::GetFullPath($OutputPath)
    if([IO.File]::Exists($fullOutput)){throw 'Report already exists; choose a new output filename.'}
    Write-Host 'Reading file identities. No game, plugin, or configuration files will be changed.'
    $report=Get-ModMenuDiagnosticReport -GameExe $GameExe
    $json=ConvertTo-Json -InputObject $report -Depth 16
    $utf8=[Text.UTF8Encoding]::new($false)
    $output=[IO.File]::Open($fullOutput,[IO.FileMode]::CreateNew,[IO.FileAccess]::Write,[IO.FileShare]::Read)
    try{[byte[]]$bytes=$utf8.GetBytes($json);$output.Write($bytes,0,$bytes.Length)}finally{$output.Dispose()}
    Write-Host ('Report saved: '+$fullOutput)
    Write-Host 'Return this JSON report and the complete LeviLamina log from the same launch.'
}

if($MyInvocation.InvocationName -ne '.'){
    $ErrorActionPreference='Stop'
    try{Invoke-ModMenuLLDiagnostic -GameExe $GameExe -OutputPath $OutputPath}catch{
        Write-Host ('Collection failed (no game files changed): '+$_.Exception.Message)
        exit 1
    }
}
