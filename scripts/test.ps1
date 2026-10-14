$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
& cmake -S (Join-Path $taskProject 'tests') -B (Join-Path $taskProject 'build\host-tests')
if($LASTEXITCODE -ne 0) { throw 'Test configuration failed' }
& cmake --build (Join-Path $taskProject 'build\host-tests') --config Release
if($LASTEXITCODE -ne 0) { throw 'Test build failed' }
& ctest --test-dir (Join-Path $taskProject 'build\host-tests') -C Release --output-on-failure
if($LASTEXITCODE -ne 0) { throw 'Color contract tests failed' }
& (Join-Path $PSScriptRoot 'test-manifest.ps1')
