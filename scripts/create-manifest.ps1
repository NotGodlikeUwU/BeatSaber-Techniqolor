$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskQpm=Get-Command qpm -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
if(-not $taskQpm) { $taskQpm='H:\BSTheater_Quest\tools\qpm\qpm.exe' }
Push-Location $taskProject
try {
    & $taskQpm qmod manifest --offline
    if($LASTEXITCODE -ne 0) { throw 'QPM manifest generation failed' }
    $taskConfig=Get-Content qpm.json -Raw | ConvertFrom-Json
    $taskManifest=Get-Content mod.json -Raw | ConvertFrom-Json
    foreach($taskDependency in $taskManifest.dependencies) {
        $taskRequested=@($taskConfig.dependencies | Where-Object id -EQ $taskDependency.id)
        if($taskRequested.Count -ne 1 -or $taskRequested[0].versionRange -notmatch '^\^\d+\.\d+\.\d+$') {
            throw "Runtime dependency $($taskDependency.id) must have a caret SemVer range in qpm.json"
        }
        # QPM emits resolved exact versions. Installer compatibility must use the declared range.
        $taskDependency.version=$taskRequested[0].versionRange
    }
    $taskJson=($taskManifest | ConvertTo-Json -Depth 30).Replace("`r`n","`n")+"`n"
    if(-not ($taskJson | Test-Json -SchemaFile (Join-Path $taskProject 'schemas\qmod.schema.json') -ErrorAction Stop)) { throw 'Generated manifest does not match the QMOD schema' }
    [IO.File]::WriteAllText((Join-Path $taskProject 'mod.json'),$taskJson,[Text.UTF8Encoding]::new($false))
    "Generated QMOD manifest $($taskManifest.version) with caret dependency ranges"
} finally { Pop-Location }
