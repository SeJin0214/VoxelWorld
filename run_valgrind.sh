#!/usr/bin/env bash
set -euo pipefail

PRESET="x64-Debug"
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${ROOT_DIR}/out/build/${PRESET}"
APP="${BUILD_DIR}/VoxelWorld"
LOG_DIR="${ROOT_DIR}/logs"
SUPPRESSION_FILE="${ROOT_DIR}/valgrind/valgrind.supp"
TIMESTAMP="$(date +%Y%m%d-%H%M%S)"
LOG_FILE="${LOG_DIR}/valgrind-${TIMESTAMP}.log"

if [[ ! -x "${APP}" ]]; then
    echo "Valgrind executable not found: ${APP}" >&2
    echo "Build it first with:" >&2
    echo "  cmake --preset ${PRESET}" >&2
    echo "  cmake --build --preset ${PRESET}" >&2
    exit 1
fi

mkdir -p "${LOG_DIR}"

VALGRIND_ARGS=(
    --tool=memcheck
    --leak-check=full
    --show-leak-kinds=definite,indirect
    --errors-for-leak-kinds=definite
    --num-callers=30
    --error-limit=no
    --log-file="${LOG_FILE}"
)

if [[ -f "${SUPPRESSION_FILE}" ]]; then
    VALGRIND_ARGS+=(--suppressions="${SUPPRESSION_FILE}")
fi

echo "Valgrind log: ${LOG_FILE}"
if [[ -f "${SUPPRESSION_FILE}" ]]; then
    echo "Valgrind suppressions: ${SUPPRESSION_FILE}"
fi

exec valgrind "${VALGRIND_ARGS[@]}" "${APP}" "$@"
