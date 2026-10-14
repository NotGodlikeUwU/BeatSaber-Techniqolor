param([switch]$SkipBuild)
$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }
$taskQpm=Get-Command qpm -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if(-not $taskQpm) { $taskQpm='H:\BSTheater_Quest\tools\qpm\qpm.exe' }
Push-Location $taskProject
try {
    & $taskQpm qmod zip --skip_build --offline
    if($LASTEXITCODE -ne 0) { throw 'QMOD packaging failed' }
    & (Join-Path $PSScriptRoot 'verify-package.ps1')
} finally { Pop-Location }
