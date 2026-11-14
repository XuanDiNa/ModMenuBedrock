[CmdletBinding()]
param(
    [Parameter(Mandatory)][string]$LlvmBin,
    [Parameter(ValueFromRemainingArguments)][string[]]$XmakeArguments
)
$ErrorActionPreference='Stop'
$vswhere=Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vs=& $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if(!$vs){throw 'Visual Studio x64 C++ tools not found'}
$bat=Join-Path $vs 'VC\Auxiliary\Build\vcvars64.bat'
# Import only toolchain variables into this child process. No persistent PATH edits.
if($env:MMB_VS_ENV_READY -ne $vs){
    $variables=& cmd.exe /d /s /c ('call "'+$bat+'" >nul && set')
    if($LASTEXITCODE -ne 0){throw 'Cannot load Visual Studio toolchain environment'}
    foreach($line in $variables){
        if($line -match '^(PATH|INCLUDE|LIB|LIBPATH)=(.*)$'){
            [Environment]::SetEnvironmentVariable($matches[1],$matches[2],'Process')
        }
    }
    $env:MMB_VS_ENV_READY=$vs
}
$compiler=Join-Path (Resolve-Path -LiteralPath $LlvmBin).Path 'clang-cl.exe'
if(!(Test-Path -LiteralPath $compiler)){throw 'clang-cl.exe missing'}
$compilerDir=Split-Path -Parent $compiler
if(!$env:PATH.StartsWith($compilerDir+';',[StringComparison]::OrdinalIgnoreCase)){$env:PATH=$compilerDir+';'+$env:PATH}
& xmake @XmakeArguments
exit $LASTEXITCODE
