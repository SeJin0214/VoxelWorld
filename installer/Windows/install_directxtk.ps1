
$scriptDir = $PSScriptRoot
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..\\..")) #절대 경로로 변경
$tempPath = [System.IO.Path]::GetFullPath((Join-Path $scriptDir ".\\directxtk"))
$destPath = [System.IO.Path]::GetFullPath((Join-Path $projectRoot ".\\third_party\\directxtk"))

$repoUrl = "https://github.com/microsoft/DirectXTK.git"

Write-Host "projectRoot = ${projectRoot}"
Write-Host "tempPath    = ${tempPath}"
Write-Host "destPath    = ${destPath}"

if (Test-Path $tempPath)
{
    Remove-Item $tempPath -Recurse -Force
}

if (Test-Path $destPath)
{
    Remove-Item $destPath -Recurse -Force
}

git clone $repoUrl $tempPath

New-Item -ItemType Directory -Path $destPath -Force | Out-Null
Copy-Item (Join-Path $tempPath "*") $destPath -Recurse -Force

Remove-Item $tempPath -Recurse -Force

$outPath = (Join-Path $destPath out)
cmake -S $destPath -B $outPath -A x64
cmake --build $outPath --config Debug
cmake --build $outPath --config Release

Write-Host "DirectXTK installed at ${destPath}"