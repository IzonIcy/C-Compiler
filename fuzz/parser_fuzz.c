/*
 * libFuzzer harness focused on the parser (and its error recovery).
 *
 * Build with:
 *   make fuzz-parse
 *
 * Run (add a corpus dir for better coverage):
 *   ./build/bin/fuzz-parse -max_total_time=60 build/fuzz-corpus-parse
 *
 * Unlike the full-pipeline harness, this one feeds EVERY lexed token stream
 * into the parser — including streams from sources whose preprocessing or
 * lexing produced diagnostics. That exercises the parser's synchronization
 * and recovery paths, which the pipeline harness deliberately skips.
 *
 * ASan/UBSan are enabled, so any crash, out-of-bounds access, or undefined
 * behavior in the compiler aborts the run and prints a reproducer.
 */

#include "ccompiler/lexer.h"
#include "ccompiler/parser.h"
#include "ccompiler/preprocessor.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const k_fuzz_input_path = "/tmp/cc-fuzz-parse-input.c";

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    FILE *stream;
    CCPreprocessResult preprocess_result;
    CCLexResult lex_result;
    CCParseResult parse_result;

    stream = fopen(k_fuzz_input_path, "wb");
    if (stream == NULL) {
        return 0;
    }
    if (size > 0) {
        fwrite(data, 1, size, stream);
    }
    fclose(stream);

    memset(&preprocess_result, 0, sizeof(preprocess_result));
    memset(&lex_result, 0, sizeof(lex_result));
    memset(&parse_result, 0, sizeof(parse_result));

    /* Preprocessing failures are fine; fall back to fuzzing raw bytes so the
     * parser still gets exercised. */
    cc_preprocess_file(k_fuzz_input_path, &preprocess_result);
    if (preprocess_result.text != NULL) {
        cc_lex_source(
            &(CCSourceView){
                .path = k_fuzz_input_path,
                .text = preprocess_result.text,
                .length = strlen(preprocess_result.text),
            },
            &lex_result
        );
    } else {
        char *raw = malloc(size + 1);
        if (raw == NULL) {
            goto cleanup;
        }
        if (size > 0) {
            memcpy(raw, data, size);
        }
        raw[size] = '\0';
        cc_lex_source(
            &(CCSourceView){
                .path = k_fuzz_input_path,
                .text = raw,
                .length = size,
            },
            &lex_result
        );
        free(raw);
    }

    /* Always parse — clean streams and diagnostic-bearing ones alike. The
     * parser's job is to terminate safely and report sane spans. */
    cc_parse_translation_unit(&lex_result, &parse_result);

cleanup:
    cc_parse_result_free(&parse_result);
    cc_lex_result_free(&lex_result);
    cc_preprocess_result_free(&preprocess_result);
    return 0;
}
