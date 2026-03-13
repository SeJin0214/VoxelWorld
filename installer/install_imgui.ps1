
param(
    [Parameter(Mandatory = $true)] # 이 파라미터 필수로 넣어야 함
    [ValidateSet("win32", "glfw")]   # 이 Set만 허용한다.
    [string]$platform,

    [Parameter(Mandatory = $true)] 
    [ValidateSet("dx11", "opengl3")]  
    [string]$graphic
)

$scriptDir = $PSScriptRoot # 스크립트가 있는 디렉토리
$repoUrl = "https://github.com/ocornut/imgui.git"
$tempPath = Join-Path $scriptDir "\\imgui" # 임시 공간
$destPath = Join-Path $scriptDir "..\\third_party\\imgui" # 옮길 곳

if (!(Test-Path $tempPath)) {
    New-Item -ItemType Directory -Path $tempPath -Force | Out-Null 
    #Force는 중간 경로 없는 것도 만들어준다.
    #Out-Null 결과 출력 로그 버림
}

if (!(Test-Path $destPath)) {
    New-Item -ItemType Directory -Path $destPath -Force | Out-Null 
}

git clone $repoUrl $tempPath

#  Join-Path 역슬래시 알아서 붙여줌
$backendSource = Join-Path $tempPath "backends" # 폴더 명

$platformCpp = "imgui_impl_${platform}.cpp"
$platformH   = "imgui_impl_${platform}.h"

$graphicCpp = "imgui_impl_${graphic}.cpp"
$graphicH   = "imgui_impl_${graphic}.h"

$coreFiles = @( # 골뱅이는 배열 만드는 것
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

foreach ($file in $coreFiles) {
    Copy-Item (Join-Path $tempPath $file) $destPath -Force
}

Copy-Item (Join-Path $backendSource $platformCpp) $destPath -Force
Copy-Item (Join-Path $backendSource $platformH) $destPath -Force
Copy-Item (Join-Path $backendSource $graphicCpp) $destPath -Force
Copy-Item (Join-Path $backendSource $graphicH) $destPath -Force

Remove-Item $tempPath -Recurse -Force #하위 폴더도 재귀적으로 지우기

Write-Host "Installed Imgui with $platform + $graphic"