param([string]$NdkPath,[string]$CachePath,[switch]$Offline)
$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskQpm=Get-Command qpm -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if(-not $taskQpm) { $taskQpm='H:\BSTheater_Quest\tools\qpm\qpm.exe' }
if(-not (Test-Path -LiteralPath $taskQpm)) { throw 'Install QPM and add it to PATH.' }
if(-not $NdkPath -and (Test-Path -LiteralPath 'H:\BSTheater_Quest\tools\qpm-data\ndk\android-ndk-r27d')) { $NdkPath='H:\BSTheater_Quest\tools\qpm-data\ndk\android-ndk-r27d' }
if($NdkPath) {
    if(-not (Test-Path -LiteralPath (Join-Path $NdkPath 'build\cmake\android.toolchain.cmake'))) { throw 'NdkPath is not an Android NDK' }
    ([IO.Path]::GetFullPath($NdkPath)).Replace('\','/') | Set-Content (Join-Path $taskProject 'ndkpath.txt')
}
if(-not $CachePath -and (Test-Path -LiteralPath 'H:\BSTheater_Quest\tools\qpm-data\cache')) { $CachePath='H:\BSTheater_Quest\tools\qpm-data\cache' }
if($CachePath) {
    @{cache=[IO.Path]::GetFullPath($CachePath);timeout=60000;symlink=$false} | ConvertTo-Json | Set-Content (Join-Path $taskProject 'qpm.settings.json')
}
Push-Location $taskProject
try {
    if($Offline) { & $taskQpm restore --offline } else { & $taskQpm restore }
    if($LASTEXITCODE -ne 0) { throw 'Dependency restore failed' }
} finally { Pop-Location }
