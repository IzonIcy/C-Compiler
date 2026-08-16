#!/bin/sh

# Golden-file regression tests: run codegen on a fixed corpus and diff the IR
# output against committed golden files. Any unintended change to the emitted
# IR fails the test.
#
# Regenerate goldens after an intentional change:
#   UPDATE_GOLDEN=1 sh tests/golden_smoke.sh

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BINARY="$ROOT_DIR/build/bin/C-Compiler"
GOLDEN_DIR="$ROOT_DIR/tests/golden"
TMP_DIR=$(mktemp -d)

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT INT TERM

run_golden_case() {
    name=$1
    input_file=$2
    golden_file="$GOLDEN_DIR/$name.ir"
    output_file="$TMP_DIR/$name.ir"

    if ! "$BINARY" codegen "$ROOT_DIR/$input_file" >"$output_file" 2>&1; then
        echo "golden case '$name': codegen failed for $input_file" >&2
        cat "$output_file" >&2
        exit 1
    fi

    if [ "${UPDATE_GOLDEN:-0}" = "1" ]; then
        cp "$output_file" "$golden_file"
        echo "updated golden: $golden_file"
        return
    fi

    if ! diff -u "$golden_file" "$output_file" >"$TMP_DIR/diff.txt" 2>&1; then
        echo "golden mismatch for '$name' ($input_file)" >&2
        echo "regenerate with: UPDATE_GOLDEN=1 sh tests/golden_smoke.sh" >&2
        cat "$TMP_DIR/diff.txt" >&2
        exit 1
    fi
}

run_golden_case typedef_chain tests/typedef_chain_ok.c
run_golden_case pointer_arith tests/pointer_arith_ok.c
run_golden_case struct_usage tests/struct_usage_ok.c
run_golden_case cast_ok tests/cast_ok.c
run_golden_case feature_showcase examples/feature_showcase.c

echo "golden tests passed"