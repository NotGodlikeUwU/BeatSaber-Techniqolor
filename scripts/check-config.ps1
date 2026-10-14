$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskJson=Get-Content (Join-Path $taskProject 'mod.template.json') -Raw
if(-not ($taskJson | Test-Json -SchemaFile (Join-Path $taskProject 'schemas/qmod.schema.json') -ErrorAction Stop)) { throw 'Manifest template does not match the QMOD schema' }
$taskManifest=$taskJson | ConvertFrom-Json
$taskConfig=Get-Content (Join-Path $taskProject 'qpm.json') -Raw | ConvertFrom-Json
foreach($taskField in @('name','id','version')) {
    if($taskManifest.$taskField -ne $taskConfig.info.$taskField) { throw "Manifest and QPM $taskField differ" }
}
$taskExpected="$($taskConfig.info.name)-$($taskConfig.info.version).qmod"
if([IO.Path]::GetFileName($taskConfig.workspace.qmodOutput) -ne $taskExpected) { throw "QMOD output must be named $taskExpected" }
foreach($taskDependency in $taskConfig.dependencies) {
    if($taskDependency.versionRange -notmatch '^\^\d+\.\d+\.\d+$') { throw "Dependency $($taskDependency.id) must use a caret SemVer range" }
}
if(@($taskConfig.dependencies | Group-Object id | Where-Object Count -GT 1).Count) { throw 'Duplicate dependency declarations' }
"Configuration verified: $($taskManifest.name) $($taskManifest.version), QMOD schema and caret dependencies"
