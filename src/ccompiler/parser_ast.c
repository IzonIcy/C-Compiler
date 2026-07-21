#include "ccompiler/parser.h"

#include <stdio.h>

static void cc_print_indent(FILE *out, unsigned indent) {
    while (indent-- > 0) {
        fputc(' ', out);
    }
}

void cc_ast_print(FILE *out, const CCAstNode *node, unsigned indent) {
    size_t index;

    if (node == NULL) {
        return;
    }

    cc_print_indent(out, indent);
    fprintf(out, "%s", cc_ast_kind_name(node->kind));
    if (node->text != NULL) {
        fprintf(out, ": %s", node->text);
    }
    fputc('\n', out);

    for (index = 0; index < node->child_count; index++) {
        cc_ast_print(out, node->children[index], indent + 2);
    }
}
