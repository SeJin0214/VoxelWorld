#!/usr/bin/env bash

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
project_root="$(cd "$script_dir/../.." && pwd)"
repo_url="https://github.com/benikabocha/tinyddsloader.git"
dest_path="$project_root/third_party/tinyddsloader"

if ! command -v git >/dev/null 2>&1; then
    echo "git is required to install tinyddsloader"
    exit 1
fi

rm -rf "$dest_path"
git clone --depth 1 "$repo_url" "$dest_path"

echo "Installed tinyddsloader into third_party/tinyddsloader"
