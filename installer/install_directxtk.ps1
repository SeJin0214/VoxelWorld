
$scriptDir = $PSScriptRoot
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..")) #절대 경로로 변경
$tempPath = [System.IO.Path]::GetFullPath((Join-Path $scriptDir ".\\directxtk"))
$destPath = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..\\third_party\\directxtk"))

$repoUrl = "https://github.com/microsoft/DirectXTK.git"

Write-Host "projectRoot = ${projectRoot}"
Write-Host "tempPath    = ${tempPath}"
Write-Host "destPath    = ${destPath}"

if (!(Test-Path $tempPath))
{
    New-Item -ItemType Directory -Path $tempPath -Force | Out-Null
}

if (!(Test-Path $destPath))
{
    New-Item -ItemType Directory -Path $destPath -Force | Out-Null
}

git clone $repoUrl $tempPath

Copy-Item (Join-Path $tempPath "*") $destPath -Recurse -Force

Remove-Item $tempPath -Recurse -Force

Write-Host "DirextXTK installed at ${destPath}"