#!/usr/bin/env bash

set -euo pipefail

platform="glfw"
graphic="opengl3"

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd "$script_dir/../.." && pwd)"
repo_url="https://github.com/ocornut/imgui.git"
temp_root="$(mktemp -d)"
temp_path="$temp_root/imgui"
dest_path="$project_root/third_party/imgui_linux"
backend_source="$temp_path/backends"

platform_cpp="imgui_impl_${platform}.cpp"
platform_h="imgui_impl_${platform}.h"
graphic_cpp="imgui_impl_${graphic}.cpp"
graphic_h="imgui_impl_${graphic}.h"

core_files=(
    "imgui.cpp"
    "imgui.h"
    "imgui_internal.h"
    "imgui_draw.cpp"
    "imgui_tables.cpp"
    "imgui_widgets.cpp"
    "imgui_demo.cpp"
    "imconfig.h"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
)

cleanup() {
    rm -rf "$temp_root"
}

trap cleanup EXIT

rm -rf "$dest_path"
mkdir -p "$dest_path"

git clone --depth 1 "$repo_url" "$temp_path"

for file in "${core_files[@]}"; do
    cp "$temp_path/$file" "$dest_path/"
done

cp "$backend_source/$platform_cpp" "$dest_path/"
cp "$backend_source/$platform_h" "$dest_path/"
cp "$backend_source/$graphic_cpp" "$dest_path/"
cp "$backend_source/$graphic_h" "$dest_path/"

echo "Installed ImGui with $platform + $graphic"
