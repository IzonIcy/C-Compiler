#!/bin/sh

set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
BINARY="$ROOT_DIR/build/bin/C-Compiler"
CASES_FILE="$ROOT_DIR/tests/data/smoke_cases.txt"
TMP_DIR=$(mktemp -d)
LAST_OUTPUT="$TMP_DIR/last-output.txt"

cleanup() {
    rm -rf "$TMP_DIR"
}

trap cleanup EXIT INT TERM

run_case() {
    mode=$1
    input_file=$2
    expected_status=$3
    expected_snippet=$4

    set +e
    case "$mode" in
        lex_tokens)
            "$BINARY" --dump-tokens lex "$ROOT_DIR/$input_file" >"$LAST_OUTPUT" 2>&1
            ;;
        preprocess|parse|check|codegen)
            "$BINARY" "$mode" "$ROOT_DIR/$input_file" >"$LAST_OUTPUT" 2>&1
            ;;
        *)
            echo "unknown test mode: $mode" >&2
            exit 1
            ;;
    esac
    status=$?
    set -e

    if [ "$status" -ne "$expected_status" ]; then
        echo "unexpected exit code for $mode $input_file: got $status expected $expected_status" >&2
        cat "$LAST_OUTPUT" >&2
        exit 1
    fi

    if ! grep -Fq "$expected_snippet" "$LAST_OUTPUT"; then
        echo "expected output to contain '$expected_snippet' for $mode $input_file" >&2
        cat "$LAST_OUTPUT" >&2
        exit 1
    fi
}

while IFS='|' read -r mode input_file expected_status expected_snippet; do
    if [ -z "$mode" ] || [ "${mode#\#}" != "$mode" ]; then
        continue
    fi

    run_case "$mode" "$input_file" "$expected_status" "$expected_snippet"
done < "$CASES_FILE"
