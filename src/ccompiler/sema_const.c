#include "ccompiler/sema.h"
#include "ccompiler/util.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool cc_parse_integer_literal(const char *text, long long *value) {
    char *copy;
    char *end;
    size_t length;
    size_t index;

    if (text == NULL || text[0] == '\0') {
        return false;
    }

    length = strlen(text);
    while (length > 0) {
        char suffix;

        suffix = text[length - 1];
        if (suffix == 'u' || suffix == 'U' || suffix == 'l' || suffix == 'L') {
            length--;
            continue;
        }
        break;
    }

    copy = cc_reallocate_or_die(NULL, length + 1);
    memcpy(copy, text, length);
    copy[length] = '\0';

    if (length > 2 && copy[0] == '0' && (copy[1] == 'b' || copy[1] == 'B')) {
        long long parsed;

        parsed = 0;
        for (index = 2; index < length; index++) {
            if (copy[index] != '0' && copy[index] != '1') {
                free(copy);
                return false;
            }
            parsed = (parsed << 1) | (copy[index] - '0');
        }
        *value = parsed;
        free(copy);
        return true;
    }

    *value = strtoll(copy, &end, 0);
    {
        bool ok;

        ok = end != NULL && *end == '\0';
        free(copy);
        return ok;
    }
}

static bool cc_parse_char_literal(const char *text, long long *value) {
    const char *quote;
    unsigned char ch;

    if (text == NULL) {
        return false;
    }

    quote = strchr(text, '\'');
    if (quote == NULL || quote[1] == '\0') {
        return false;
    }

    quote++;
    if (*quote == '\\') {
        quote++;
        if (*quote == '0') {
            *value = 0;
            return true;
        }
        if (*quote == 'n') {
            *value = '\n';
            return true;
        }
        if (*quote == 'r') {
            *value = '\r';
            return true;
        }
        if (*quote == 't') {
            *value = '\t';
            return true;
        }
        if (*quote == '\\') {
            *value = '\\';
            return true;
        }
        if (*quote == '\'') {
            *value = '\'';
            return true;
        }
        if (*quote == '"') {
            *value = '"';
            return true;
        }
        if (*quote == 'x') {
            char *end;

            *value = strtoll(quote + 1, &end, 16);
            return end != quote + 1;
        }
        if (*quote >= '0' && *quote <= '7') {
            char buffer[8];
            size_t index;

            index = 0;
            while (index < sizeof(buffer) - 1 && quote[index] >= '0' && quote[index] <= '7') {
                buffer[index] = quote[index];
                index++;
            }
            buffer[index] = '\0';
            *value = strtoll(buffer, NULL, 8);
            return true;
        }
        *value = (unsigned char)*quote;
        return true;
    }

    ch = (unsigned char)*quote;
    *value = ch;
    return true;
}

static CCConstValue cc_invalid_const_value(void) {
    CCConstValue value;

    value.ok = false;
    value.value = 0;
    return value;
}

static CCConstValue cc_make_const_value(long long value) {
    CCConstValue result;

    result.ok = true;
    result.value = value;
    return result;
}

static CCConstValue cc_try_fold_binary_constant(const CCAstNode *node) {
    CCConstValue left;
    CCConstValue right;

    if (node->child_count < 2) {
        return cc_invalid_const_value();
    }

    left = cc_eval_const_integer_expr(node->children[0]);
    right = cc_eval_const_integer_expr(node->children[1]);
    if (!left.ok || !right.ok || node->text == NULL) {
        return cc_invalid_const_value();
    }

    if (strcmp(node->text, "+") == 0) {
        return cc_make_const_value(left.value + right.value);
    }
    if (strcmp(node->text, "-") == 0) {
        return cc_make_const_value(left.value - right.value);
    }
    if (strcmp(node->text, "*") == 0) {
        return cc_make_const_value(left.value * right.value);
    }
    if (strcmp(node->text, "/") == 0 && right.value != 0) {
        return cc_make_const_value(left.value / right.value);
    }
    if (strcmp(node->text, "%") == 0 && right.value != 0) {
        return cc_make_const_value(left.value % right.value);
    }
    if (strcmp(node->text, "<<") == 0) {
        return cc_make_const_value(left.value << right.value);
    }
    if (strcmp(node->text, ">>") == 0) {
        return cc_make_const_value(left.value >> right.value);
    }
    if (strcmp(node->text, "&") == 0) {
        return cc_make_const_value(left.value & right.value);
    }
    if (strcmp(node->text, "|") == 0) {
        return cc_make_const_value(left.value | right.value);
    }
    if (strcmp(node->text, "^") == 0) {
        return cc_make_const_value(left.value ^ right.value);
    }
    if (strcmp(node->text, "&&") == 0) {
        return cc_make_const_value((left.value != 0) && (right.value != 0));
    }
    if (strcmp(node->text, "||") == 0) {
        return cc_make_const_value((left.value != 0) || (right.value != 0));
    }
    if (strcmp(node->text, "==") == 0) {
        return cc_make_const_value(left.value == right.value);
    }
    if (strcmp(node->text, "!=") == 0) {
        return cc_make_const_value(left.value != right.value);
    }
    if (strcmp(node->text, "<") == 0) {
        return cc_make_const_value(left.value < right.value);
    }
    if (strcmp(node->text, "<=") == 0) {
        return cc_make_const_value(left.value <= right.value);
    }
    if (strcmp(node->text, ">") == 0) {
        return cc_make_const_value(left.value > right.value);
    }
    if (strcmp(node->text, ">=") == 0) {
        return cc_make_const_value(left.value >= right.value);
    }
    if (strcmp(node->text, ",") == 0) {
        return right;
    }

    return cc_invalid_const_value();
}

CCConstValue cc_eval_const_integer_expr(const CCAstNode *node) {
    long long value;

    if (node == NULL) {
        return cc_invalid_const_value();
    }

    switch (node->kind) {
        case CC_AST_LITERAL:
            if (node->text == NULL) {
                return cc_invalid_const_value();
            }
            if (cc_parse_integer_literal(node->text, &value) || cc_parse_char_literal(node->text, &value)) {
                return cc_make_const_value(value);
            }
            return cc_invalid_const_value();
        case CC_AST_UNARY_EXPRESSION: {
            CCConstValue operand;

            if (node->child_count == 0 || node->text == NULL) {
                return cc_invalid_const_value();
            }

            operand = cc_eval_const_integer_expr(node->children[0]);
            if (!operand.ok) {
                return operand;
            }

            if (strcmp(node->text, "plus") == 0) {
                return cc_make_const_value(+operand.value);
            }
            if (strcmp(node->text, "minus") == 0) {
                return cc_make_const_value(-operand.value);
            }
            if (strcmp(node->text, "bang") == 0) {
                return cc_make_const_value(!operand.value);
            }
            if (strcmp(node->text, "tilde") == 0) {
                return cc_make_const_value(~operand.value);
            }
            return cc_invalid_const_value();
        }
        case CC_AST_BINARY_EXPRESSION:
            return cc_try_fold_binary_constant(node);
        case CC_AST_CONDITIONAL_EXPRESSION: {
            CCConstValue condition;

            if (node->child_count < 3) {
                return cc_invalid_const_value();
            }

            condition = cc_eval_const_integer_expr(node->children[0]);
            if (!condition.ok) {
                return condition;
            }

            return condition.value != 0
                ? cc_eval_const_integer_expr(node->children[1])
                : cc_eval_const_integer_expr(node->children[2]);
        }
        case CC_AST_CAST_EXPRESSION:
            if (node->child_count < 2) {
                return cc_invalid_const_value();
            }
            return cc_eval_const_integer_expr(node->children[1]);
        default:
            return cc_invalid_const_value();
    }
}
