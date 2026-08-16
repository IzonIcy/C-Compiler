/*
 * libFuzzer harness for the full compiler pipeline.
 *
 * Build with:
 *   make fuzz
 *
 * Run (add a corpus dir for better coverage):
 *   ./build/bin/fuzz-pipeline -max_total_time=60 build/fuzz-corpus
 *
 * Every input is pushed through two paths:
 *   1. the lexer directly, in memory
 *   2. the full pipeline: preprocess -> lex -> parse -> sema -> codegen
 *
 * ASan/UBSan are enabled, so any crash, out-of-bounds access, or undefined
 * behavior in the compiler aborts the run and prints a reproducer.
 */

#include "ccompiler/codegen.h"
#include "ccompiler/lexer.h"
#include "ccompiler/parser.h"
#include "ccompiler/preprocessor.h"
#include "ccompiler/sema.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *const k_fuzz_input_path = "/tmp/lumi-fuzz-input.c";

static void fuzz_lex_in_memory(const uint8_t *data, size_t size) {
    char *text;
    CCLexResult result;

    text = malloc(size + 1);
    if (text == NULL) {
        return;
    }
    if (size > 0) {
        memcpy(text, data, size);
    }
    text[size] = '\0';

    memset(&result, 0, sizeof(result));
    cc_lex_source(
        &(CCSourceView){
            .path = "<fuzz>",
            .text = text,
            .length = size,
        },
        &result
    );
    cc_lex_result_free(&result);
    free(text);
}

static void fuzz_full_pipeline(void) {
    CCPreprocessResult preprocess_result;
    CCLexResult lex_result;
    CCParseResult parse_result;
    CCSemaResult sema_result;
    CCSemaOptions sema_options;
    CCCodegenResult codegen_result;

    memset(&preprocess_result, 0, sizeof(preprocess_result));
    memset(&lex_result, 0, sizeof(lex_result));
    memset(&parse_result, 0, sizeof(parse_result));
    memset(&sema_result, 0, sizeof(sema_result));
    memset(&codegen_result, 0, sizeof(codegen_result));
    memset(&sema_options, 0, sizeof(sema_options));

    cc_preprocess_file(k_fuzz_input_path, &preprocess_result);
    if (preprocess_result.diagnostics.count > 0) {
        goto cleanup;
    }

    cc_lex_source(
        &(CCSourceView){
            .path = k_fuzz_input_path,
            .text = preprocess_result.text,
            .length = strlen(preprocess_result.text),
        },
        &lex_result
    );
    if (lex_result.diagnostics.count > 0) {
        goto cleanup;
    }

    cc_parse_translation_unit(&lex_result, &parse_result);
    if (parse_result.diagnostics.count > 0) {
        goto cleanup;
    }

    cc_sema_check_translation_unit(&parse_result, &sema_options, &sema_result);
    if (sema_result.diagnostics.count > 0) {
        goto cleanup;
    }

    cc_codegen_translation_unit(&parse_result, &codegen_result);

cleanup:
    cc_codegen_result_free(&codegen_result);
    cc_sema_result_free(&sema_result);
    cc_parse_result_free(&parse_result);
    cc_lex_result_free(&lex_result);
    cc_preprocess_result_free(&preprocess_result);
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    FILE *stream;

    stream = fopen(k_fuzz_input_path, "wb");
    if (stream == NULL) {
        return 0;
    }
    if (size > 0) {
        fwrite(data, 1, size, stream);
    }
    fclose(stream);

    fuzz_lex_in_memory(data, size);
    fuzz_full_pipeline();
    return 0;
}