#!/usr/bin/env bash

set -euo pipefail

# BASH_SOURCE: 배쉬가 제공하는 현재 스크립트 경로
script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)" # 절대경로 구하기 

include_vscode_tools=false
if [[ "${1:-}" == "--with-vscode" ]]; then
    include_vscode_tools=true
fi

scripts=(
    "install_glfw.sh"
    "install_glm.sh"
    "install_opengl.sh"
    "install_imgui.sh"
    "install_stb_image.sh"
    "install_tinyddsloader.sh"
)

if [[ "${include_vscode_tools}" == true ]]; then
    scripts+=("install_vscode_dev_tools.sh")
fi

total="${#scripts[@]}"
index=1
for script_name in "${scripts[@]}"; do
    script_path="${script_dir}/${script_name}"

    if [[ ! -x "${script_path}" ]]; then
        echo "Installer is missing or not executable: ${script_path}" >&2
        exit 1
    fi

    echo "[${index}/${total}] Running ${script_name}"
    "${script_path}"
    ((index++))
done

echo "All selected installers completed successfully."
