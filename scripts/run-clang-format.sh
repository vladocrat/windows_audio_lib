#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

CLANG_FORMAT="${CLANG_FORMAT:-clang-format}"
MODE="${1:-check}"

files=$(find "$ROOT_DIR/include" "$ROOT_DIR/src" -name '*.h' -o -name '*.cpp')

if [ -z "$files" ]; then
    echo "No source files found"
    exit 0
fi

case "$MODE" in
    check)
        echo "Checking formatting..."
        output=$(echo "$files" | xargs "$CLANG_FORMAT" --dry-run --Werror 2>&1) || {
            echo "FORMAT_FAILED"
            echo "$output"
            echo ""
            echo "Files with violations:"
            echo "$output" | grep -oP '^[^:]+' | sort -u
            exit 1
        }
        echo "FORMAT_OK"
        ;;
    fix)
        echo "Fixing formatting..."
        echo "$files" | xargs "$CLANG_FORMAT" -i
        echo "FORMAT_FIXED"
        ;;
    *)
        echo "Usage: $0 [check|fix]"
        exit 1
        ;;
esac
