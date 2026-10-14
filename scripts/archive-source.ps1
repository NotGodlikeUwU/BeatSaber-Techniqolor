$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskConfig=Get-Content (Join-Path $taskProject 'qpm.json') -Raw | ConvertFrom-Json
$taskOutput=Join-Path $taskProject ("Techniqolor-"+$taskConfig.info.version+"-source.zip")
$taskFiles=@('CMakeLists.txt','qpm.json','qpm.shared.json','mod.template.json','README.md','LICENSE','.gitignore','.gitattributes') | ForEach-Object { Get-Item -LiteralPath (Join-Path $taskProject $_) }
foreach($taskDirectory in @('src','include','shared','cmake','scripts','tests','docs','third_party','schemas')) {
    $taskPath=Join-Path $taskProject $taskDirectory
    if(Test-Path -LiteralPath $taskPath) { $taskFiles+=@(Get-ChildItem -LiteralPath $taskPath -Recurse -File) }
}
$taskZipStream=[IO.File]::Open($taskOutput,[IO.FileMode]::Create)
$taskZip=[IO.Compression.ZipArchive]::new($taskZipStream,[IO.Compression.ZipArchiveMode]::Create)
try {
    foreach($taskFile in $taskFiles) {
        $taskRelative=[IO.Path]::GetRelativePath($taskProject,$taskFile.FullName).Replace('\','/')
        if($taskRelative.StartsWith('../') -or [IO.Path]::IsPathRooted($taskRelative)) { throw 'Source file outside project' }
        [IO.Compression.ZipFileExtensions]::CreateEntryFromFile($taskZip,$taskFile.FullName,$taskRelative) | Out-Null
    }
} finally { $taskZip.Dispose(); $taskZipStream.Dispose() }
"Source archive: $taskOutput ($($taskFiles.Count) files)"
