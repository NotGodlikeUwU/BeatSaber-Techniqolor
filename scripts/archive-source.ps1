$ErrorActionPreference='Stop'
$taskProject=[IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
$taskOutput=Join-Path $taskProject 'Techniqolor-0.1-source.zip'
$taskFiles=@('CMakeLists.txt','qpm.json','qpm.shared.json','mod.template.json','README.md','RELEASE_NOTES.md','LICENSE','THIRD_PARTY_NOTICES.md','.gitignore','.gitattributes') | ForEach-Object { Get-Item -LiteralPath (Join-Path $taskProject $_) }
foreach($taskDirectory in @('src','include','shared','cmake','scripts','tests','docs','third_party')) {
    $taskFiles+=@(Get-ChildItem -LiteralPath (Join-Path $taskProject $taskDirectory) -Recurse -File)
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
