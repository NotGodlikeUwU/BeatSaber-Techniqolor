$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskSchema=Join-Path $taskProject 'schemas\qmod.schema.json'
$taskFixture=Get-Content (Join-Path $taskProject 'tests\fixtures\invalid-semver-manifest.json') -Raw | ConvertFrom-Json
$taskChecks=0
foreach($taskVersion in @('0.1','01.1.0','0.01.0','0.1.01','0.1.0.0')) {
    $taskFixture.version=$taskVersion
    if(($taskFixture | ConvertTo-Json -Depth 10) | Test-Json -SchemaFile $taskSchema -ErrorAction SilentlyContinue) { throw "Official schema accepted invalid version $taskVersion" }
    $taskChecks++
}
$taskFixture.version='0.1.0'
if(-not (($taskFixture | ConvertTo-Json -Depth 10) | Test-Json -SchemaFile $taskSchema)) { throw 'Official schema rejected 0.1.0' }
$taskChecks++
& (Join-Path $PSScriptRoot 'verify-package.ps1') | Out-Null
$taskChecks++
$taskConfig=Get-Content (Join-Path $taskProject 'qpm.json') -Raw | ConvertFrom-Json
$taskPackage=Join-Path $taskProject $taskConfig.workspace.qmodOutput
$taskTestDir=Join-Path $taskProject 'build\package-regression'
New-Item -ItemType Directory -Path $taskTestDir -Force | Out-Null
$taskSource=[IO.Compression.ZipFile]::OpenRead($taskPackage)
try {
    foreach($taskCase in @('short-version','exact-dependency')) {
        $taskOutput=Join-Path $taskTestDir ($taskCase+'.qmod')
        $taskStream=[IO.File]::Open($taskOutput,[IO.FileMode]::Create)
        $taskZip=[IO.Compression.ZipArchive]::new($taskStream,[IO.Compression.ZipArchiveMode]::Create)
        try {
            foreach($taskEntry in $taskSource.Entries) {
                $taskInput=$taskEntry.Open()
                $taskTarget=$taskZip.CreateEntry($taskEntry.FullName).Open()
                try {
                    if($taskEntry.FullName -eq 'mod.json') {
                        $taskReader=[IO.StreamReader]::new($taskInput)
                        $taskManifest=$taskReader.ReadToEnd() | ConvertFrom-Json
                        if($taskCase -eq 'short-version') { $taskManifest.version='0.1' }
                        else { $taskManifest.dependencies[0].version='=6.4.2' }
                        $taskData=[Text.Encoding]::UTF8.GetBytes(($taskManifest | ConvertTo-Json -Depth 30))
                        $taskTarget.Write($taskData,0,$taskData.Length)
                    } else { $taskInput.CopyTo($taskTarget) }
                } finally { $taskInput.Dispose(); $taskTarget.Dispose() }
            }
        } finally { $taskZip.Dispose(); $taskStream.Dispose() }
        $taskRejected=$false
        try { & (Join-Path $PSScriptRoot 'verify-package.ps1') -Package $taskOutput | Out-Null }
        catch { $taskRejected=$true }
        if(-not $taskRejected) { throw "Package verifier accepted regression $taskCase" }
        $taskChecks++
    }
} finally { $taskSource.Dispose() }
"$taskChecks manifest regression checks passed: official SemVer schema and real QMOD mutations"
