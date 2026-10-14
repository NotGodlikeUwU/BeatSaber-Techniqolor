param([switch]$SkipBuild)
$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if(-not $SkipBuild) { & (Join-Path $PSScriptRoot 'build.ps1') }
& (Join-Path $PSScriptRoot 'create-manifest.ps1')
$taskConfig=Get-Content (Join-Path $taskProject 'qpm.json') -Raw | ConvertFrom-Json
$taskManifest=Get-Content (Join-Path $taskProject 'mod.json') -Raw | ConvertFrom-Json
$taskFiles=[ordered]@{'mod.json'=(Join-Path $taskProject 'mod.json')}
foreach($taskName in @($taskManifest.modFiles)+@($taskManifest.lateModFiles)+@($taskManifest.libraryFiles)) {
    if([IO.Path]::GetFileName($taskName) -ne $taskName) { throw "Unexpected library path: $taskName" }
    $taskMatches=@($taskConfig.workspace.qmodIncludeDirs | ForEach-Object { Join-Path (Join-Path $taskProject $_) $taskName } | Where-Object { Test-Path -LiteralPath $_ -PathType Leaf })
    if($taskMatches.Count -ne 1) { throw "Expected exactly one build file for $taskName" }
    $taskFiles[$taskName]=$taskMatches[0]
}
foreach($taskRelative in $taskConfig.workspace.qmodIncludeFiles) {
    $taskFull=[IO.Path]::GetFullPath((Join-Path $taskProject $taskRelative))
    if(-not $taskFull.StartsWith($taskProject+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'QMOD include outside project' }
    $taskName=[IO.Path]::GetRelativePath($taskProject,$taskFull).Replace('\','/')
    if($taskFiles.Contains($taskName)) { throw "Duplicate QMOD entry $taskName" }
    $taskFiles[$taskName]=$taskFull
}
$taskOutput=[IO.Path]::GetFullPath((Join-Path $taskProject $taskConfig.workspace.qmodOutput))
if(-not $taskOutput.StartsWith($taskProject+'\',[StringComparison]::OrdinalIgnoreCase)) { throw 'QMOD output outside project' }
$taskStream=[IO.File]::Open($taskOutput,[IO.FileMode]::Create)
$taskZip=[IO.Compression.ZipArchive]::new($taskStream,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach($taskFile in $taskFiles.GetEnumerator()) { [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($taskZip,$taskFile.Value,$taskFile.Key) | Out-Null }
} finally { $taskZip.Dispose(); $taskStream.Dispose() }
& (Join-Path $PSScriptRoot 'verify-package.ps1') -Package $taskOutput
"QMOD written: $taskOutput"
