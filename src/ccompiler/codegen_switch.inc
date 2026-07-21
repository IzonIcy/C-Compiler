static void cc_grow_switch_layout(CCSwitchLayout *layout) {
    size_t new_capacity;

    new_capacity = layout->case_capacity == 0 ? 4 : layout->case_capacity * 2;
    layout->case_nodes = cc_reallocate_or_die(layout->case_nodes, new_capacity * sizeof(*layout->case_nodes));
    layout->case_values = cc_reallocate_or_die(layout->case_values, new_capacity * sizeof(*layout->case_values));
    layout->case_capacity = new_capacity;
}

static void cc_switch_layout_add_case(CCSwitchLayout *layout, const CCAstNode *case_node, long long value) {
    if (layout->case_count == layout->case_capacity) {
        cc_grow_switch_layout(layout);
    }

    layout->case_nodes[layout->case_count] = case_node;
    layout->case_values[layout->case_count] = value;
    layout->case_count++;
}

static void cc_collect_switch_layout(CCCodegenContext *context, CCSwitchLayout *layout, const CCAstNode *statement) {
    size_t index;

    if (statement == NULL) {
        return;
    }

    switch (statement->kind) {
        case CC_AST_SWITCH_STATEMENT:
            return;
        case CC_AST_CASE_STATEMENT: {
            CCConstValue folded;

            if (statement->child_count == 0) {
                return;
            }

            folded = cc_eval_const_integer_expr(statement->children[0]);
            if (!folded.ok) {
                cc_add_diagnostic(context, statement->children[0]->span, "switch case is not a constant expression during code generation");
            } else {
                cc_switch_layout_add_case(layout, statement, folded.value);
            }

            if (statement->child_count > 1) {
                cc_collect_switch_layout(context, layout, statement->children[1]);
            }
            return;
        }
        case CC_AST_DEFAULT_STATEMENT:
            if (layout->default_node == NULL) {
                layout->default_node = statement;
            }
            if (statement->child_count > 0) {
                cc_collect_switch_layout(context, layout, statement->children[0]);
            }
            return;
        default:
            break;
    }

    for (index = 0; index < statement->child_count; index++) {
        cc_collect_switch_layout(context, layout, statement->children[index]);
    }
}

static void cc_free_switch_layout(CCSwitchLayout *layout) {
    free(layout->case_nodes);
    free(layout->case_values);
    memset(layout, 0, sizeof(*layout));
}
