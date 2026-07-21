#!/bin/sh

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BINARY="$ROOT_DIR/build/bin/C-Compiler"
TMP_DIR=$(mktemp -d)
CC_OUTPUT="$TMP_DIR/cc-output.txt"
CLANG_OUTPUT="$TMP_DIR/clang-output.txt"

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT INT TERM

if ! command -v clang >/dev/null 2>&1; then
    echo "skipping clang differential check (clang not installed)"
    exit 0
fi

run_diff_case() {
    input_file=$1

    set +e
    "$BINARY" check "$ROOT_DIR/$input_file" >"$CC_OUTPUT" 2>&1
    cc_status=$?
    clang -std=c11 -fsyntax-only "$ROOT_DIR/$input_file" >"$CLANG_OUTPUT" 2>&1
    clang_status=$?
    set -e

    cc_ok=0
    clang_ok=0
    if [ "$cc_status" -eq 0 ]; then
        cc_ok=1
    fi
    if [ "$clang_status" -eq 0 ]; then
        clang_ok=1
    fi

    if [ "$cc_ok" -ne "$clang_ok" ]; then
        echo "clang differential mismatch for $input_file" >&2
        echo "--- C-Compiler output ---" >&2
        cat "$CC_OUTPUT" >&2
        echo "--- clang output ---" >&2
        cat "$CLANG_OUTPUT" >&2
        exit 1
    fi
}

while IFS= read -r input_file; do
    if [ -z "$input_file" ] || [ "${input_file#\#}" != "$input_file" ]; then
        continue
    fi
    run_diff_case "$input_file"
done <<'EOF'
# expected success in both compilers
tests/typedef_chain_ok.c
tests/pointer_arith_ok.c
tests/struct_usage_ok.c
tests/cast_ok.c
# expected failure in both compilers
tests/test_error.c
tests/struct_usage_fail.c
EOF
