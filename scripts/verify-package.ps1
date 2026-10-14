param([string]$Package)
$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskConfig=Get-Content (Join-Path $taskProject 'qpm.json') -Raw | ConvertFrom-Json
if(-not $Package) { $Package=Join-Path $taskProject $taskConfig.workspace.qmodOutput }
Add-Type -AssemblyName System.IO.Compression
$taskZip=[IO.Compression.ZipFile]::OpenRead($Package)
try {
    $taskNames=@($taskZip.Entries | ForEach-Object FullName)
    if(@($taskNames | Group-Object | Where-Object Count -GT 1).Count) { throw 'Duplicate archive entries' }
    if(-not $taskZip.GetEntry('mod.json') -or -not $taskZip.GetEntry('LICENSE')) { throw 'Missing manifest or license' }
    $taskReader=[IO.StreamReader]::new($taskZip.GetEntry('mod.json').Open())
    try { $taskJson=$taskReader.ReadToEnd() } finally { $taskReader.Dispose() }
    if(-not ($taskJson | Test-Json -SchemaFile (Join-Path $taskProject 'schemas\qmod.schema.json') -ErrorAction Stop)) { throw 'QMOD schema validation failed' }
    $taskManifest=$taskJson | ConvertFrom-Json
    if($taskManifest.name -ne 'Techniqolor' -or $taskManifest.id -ne 'techniqolor' -or $taskManifest.version -ne $taskConfig.info.version -or $taskManifest.packageVersion -ne '1.40.8_7379' -or $taskManifest.modloader -ne 'Scotland2') { throw 'Wrong QMOD identity or target' }
    if(@($taskManifest.modFiles).Count -ne 1 -or $taskManifest.modFiles[0] -ne 'libtechniqolor.so') { throw 'Unexpected modFiles' }
    foreach($taskDependency in @('beatsaber-hook','custom-types','paper2_scotland2','songcore','bsml')) {
        if(-not ($taskManifest.dependencies | Where-Object id -EQ $taskDependency)) { throw "Missing dependency $taskDependency" }
    }
    foreach($taskDependency in $taskManifest.dependencies) {
        $taskRequested=$taskConfig.dependencies | Where-Object id -EQ $taskDependency.id
        if($taskDependency.version -notmatch '^\^\d+\.\d+\.\d+$' -or $taskDependency.version -ne $taskRequested.versionRange) {
            throw "Runtime dependency $($taskDependency.id) must use its declared caret SemVer range"
        }
    }
    $taskEntry=$taskZip.GetEntry('libtechniqolor.so')
    if(-not $taskEntry) { throw 'Missing native library' }
    $taskStream=[IO.MemoryStream]::new()
    $taskInput=$taskEntry.Open()
    try { $taskInput.CopyTo($taskStream) } finally { $taskInput.Dispose() }
    $taskBytes=$taskStream.ToArray(); $taskStream.Dispose()
    if($taskBytes.Length -lt 64 -or $taskBytes[0] -ne 127 -or $taskBytes[1] -ne 69 -or $taskBytes[2] -ne 76 -or $taskBytes[3] -ne 70 -or $taskBytes[4] -ne 2 -or [BitConverter]::ToUInt16($taskBytes,18) -ne 183) { throw 'Native library is not ELF64 AArch64' }
    $taskHash=[Convert]::ToHexString([Security.Cryptography.SHA256]::HashData($taskBytes))
    $taskLocalHash=(Get-FileHash (Join-Path $taskProject 'build\libtechniqolor.so') -Algorithm SHA256).Hash
    if($taskHash -ne $taskLocalHash) { throw 'Packaged native library differs from build' }
    "QMOD verified: official schema, $($taskManifest.version), caret dependencies, ELF64 AArch64, license, SHA256 $taskHash"
} finally { $taskZip.Dispose() }
