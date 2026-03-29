#!/usr/bin/env bash

set -euo pipefail

# -e: 명령 하나라도 실패하면 즉시 종료
# -u: 선언 안 된 변수 사용 시 오류로 종료
# pipefail: 파이프라인(a | b)에서 앞 명령이 실패해도 실패로 처리
# 조용히 잘못 진행되는 걸 막아주는 옵션

echo "[1/2] Checking VS Code CLI..."
if ! command -v code >/dev/null 2>&1; then #/dev/null 표준 출력 버림, 2>&1 표준에러도 같은 곳에 버림
    echo "VS Code CLI 'code' not found."
    echo "Enable it in VS Code: Command Palette -> 'Shell Command: Install code command in PATH'."
    exit 1
fi

echo "[2/2] Installing VS Code extensions..."
# CMake integration
code --install-extension ms-vscode.cmake-tools
# C/C++ language service + GDB launch integration
code --install-extension ms-vscode.cpptools
# GLSL language support
code --install-extension filippofracascia.glsl-language-support

echo "Done. Installed extensions: ms-vscode.cmake-tools, ms-vscode.cpptools, filippofracascia.glsl-language-support"
