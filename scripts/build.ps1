param([switch]$Clean)
$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskBuild=[IO.Path]::GetFullPath((Join-Path $taskProject 'build'))
if($taskBuild -ne "$taskProject\build") { throw 'Unexpected build path' }
if($Clean -and (Test-Path -LiteralPath $taskBuild)) { Remove-Item -LiteralPath $taskBuild -Recurse -Force }
if(-not (Test-Path (Join-Path $taskProject 'extern.cmake'))) { throw 'Restore dependencies first: qpm restore' }
$taskTools='H:\BSTheater_Quest\tools\qpm'
if(Test-Path (Join-Path $taskTools 'ninja.exe')) { $env:Path="$taskTools;"+$env:Path }
& cmake -S $taskProject -B $taskBuild -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo
if($LASTEXITCODE -ne 0) { throw 'CMake configuration failed' }
& cmake --build $taskBuild -j 2
if($LASTEXITCODE -ne 0) { throw 'Android build failed' }
