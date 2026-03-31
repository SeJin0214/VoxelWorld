param(
    [Parameter(Mandatory = $false)]
    [ValidateSet("win32", "glfw")]
    [string]$platform = "win32",

    [Parameter(Mandatory = $false)]
    [ValidateSet("dx11", "opengl3")]
    [string]$graphic = "dx11"
)

$scriptDir = $PSScriptRoot
$projectRoot = [System.IO.Path]::GetFullPath((Join-Path $scriptDir "..\.."))
$repoUrl = "https://github.com/ocornut/imgui.git"
$tempPath = Join-Path $scriptDir "imgui"
$destPath = Join-Path $projectRoot "third_party\imgui"

if (Test-Path $tempPath) {
    Remove-Item $tempPath -Recurse -Force
}

if (Test-Path $destPath) {
    Remove-Item $destPath -Recurse -Force
}

New-Item -ItemType Directory -Path $tempPath -Force | Out-Null
New-Item -ItemType Directory -Path $destPath -Force | Out-Null

git clone --depth 1 $repoUrl $tempPath

$backendSource = Join-Path $tempPath "backends"

$platformCpp = "imgui_impl_${platform}.cpp"
$platformH = "imgui_impl_${platform}.h"
$graphicCpp = "imgui_impl_${graphic}.cpp"
$graphicH = "imgui_impl_${graphic}.h"

$coreFiles = @(
    "imgui.cpp",
    "imgui.h",
    "imgui_internal.h",
    "imgui_draw.cpp",
    "imgui_tables.cpp",
    "imgui_widgets.cpp",
    "imgui_demo.cpp",
    "imconfig.h",
    "imstb_rectpack.h",
    "imstb_textedit.h",
    "imstb_truetype.h"
)

$backendFiles = @(
    $platformCpp,
    $platformH,
    $graphicCpp,
    $graphicH
)

if ($graphic -eq "opengl3") {
    $backendFiles += "imgui_impl_opengl3_loader.h"
}

foreach ($file in $coreFiles) {
    Copy-Item (Join-Path $tempPath $file) $destPath -Force
}

foreach ($file in $backendFiles) {
    Copy-Item (Join-Path $backendSource $file) $destPath -Force
}

Remove-Item $tempPath -Recurse -Force

Write-Host "Installed ImGui with $platform + $graphic"

