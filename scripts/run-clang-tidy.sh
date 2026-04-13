#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

CLANG_TIDY="${CLANG_TIDY:-clang-tidy}"
BUILD_DIR="${1:-$ROOT_DIR/build}"

if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "TIDY_NO_COMPILE_DB"
    echo "compile_commands.json not found in $BUILD_DIR"
    echo "Run: cmake -B build -DBUILD_TESTS=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
    exit 1
fi

files=$(find "$ROOT_DIR/src" -name '*.cpp')

if [ -z "$files" ]; then
    echo "No source files found"
    exit 0
fi

echo "Running clang-tidy..."
output=$(echo "$files" | xargs "$CLANG_TIDY" -p "$BUILD_DIR" \
    --warnings-as-errors='bugprone-use-after-move,bugprone-dangling-handle,bugprone-undefined-memory-manipulation' \
    2>&1) || {
    echo "TIDY_FAILED"
    echo "$output"
    exit 1
}

if echo "$output" | grep -q "warning:"; then
    echo "TIDY_WARNINGS"
    echo "$output"
    exit 0
fi

echo "TIDY_OK"
