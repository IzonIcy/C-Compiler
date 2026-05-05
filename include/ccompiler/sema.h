#ifndef CCOMPILER_SEMA_H
#define CCOMPILER_SEMA_H

#include <stdbool.h>
#include <stddef.h>

#include "ccompiler/parser.h"

typedef struct {
    CCDiagnosticBuffer diagnostics;
    size_t function_count;
    size_t global_count;
    size_t typedef_count;
} CCSemaResult;

typedef struct {
    bool warnings_enabled;
} CCSemaOptions;

void cc_sema_check_translation_unit(
    const CCParseResult *parse_result,
    const CCSemaOptions *options,
    CCSemaResult *result
);
void cc_sema_result_free(CCSemaResult *result);


// Utility: Constant integer folding result
// ok: true if the node represents a valid constant expr, value contains the integer
//     false if not a constant expr or not foldable here
// value: folded/constant value, only meaningful if ok=true
//
typedef struct {
    bool ok;
    long long value;
} CCConstValue;

// Fold/evaluate an AST node as a constant integer expression (e.g. for array size, case label, static assert...)
// Returns ok=false if not a constant integer expr. Does not allocate or mutate AST.
CCConstValue cc_eval_const_integer_expr(const struct CCAstNode *node);

#endif

