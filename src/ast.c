#include "ast.h"
#include "ds/arena.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"

#include <stdio.h>
#include <stdlib.h>

void ast_init(ast *a) {
  arena_init(&a->arena, 4 << 20); // allocate 4 megabytes for now
  dynamic_array_init(&a->instrs, sizeof(instr_node));
  a->loop_counter = 0;
}

/*
 * @brief: prints a variable.
 *
 * @param var: pointer to a "variable" struct.
 */
static void check_var_and_print(variable *var) {
  switch (var->type) {
  case TYPE_INT:
  case TYPE_CHAR:
  case TYPE_STRING:
    printf("%s", var->name);
    break;
  case TYPE_POINTER:
    printf("*%s", var->name);
    break;
  case TYPE_VOID:
    break;
  }
}

/*
 * @brief: prints an expression node. (declaration)
 *
 * @param expr: pointer to an expression node.
 */
static void check_expr_and_print(expr_node *expr);

/*
 * @brief: prints a term node.
 *
 * @param expr: pointer to a term node.
 */
static void check_term_and_print(term_node *term) {
  switch (term->kind) {
  case TERM_INT:
    printf("%d", term->value.integer);
    break;
  case TERM_CHAR:
    printf("\'%c\'", term->value.character);
    break;
  case TERM_STRING:
    printf("\"%s\"", term->value.str);
    break;
  case TERM_IDENTIFIER:
    printf("%s", term->identifier.name);
    break;
  case TERM_POINTER:
  case TERM_DEREF:
    printf("*%s", term->identifier.name);
    break;
  case TERM_ADDOF:
    printf("&%s", term->identifier.name);
    break;
  case TERM_ARRAY_ACCESS:
    printf("%s[", term->array_access.array_var.name);
    check_expr_and_print(term->array_access.index_expr);
    printf("]");
    break;
  case TERM_ARRAY_LITERAL:
    printf("{...}");
    break;
  case TERM_FUNCTION_CALL:
    printf("%s(", term->fn_call.name);
    for (size_t i = 0; i < term->fn_call.parameters.count; i++) {
      expr_node arg;
      dynamic_array_get(&term->fn_call.parameters, i, &arg);
      check_expr_and_print(&arg);
      if (i < term->fn_call.parameters.count - 1) {
        printf(", ");
      }
    }
    printf(")");
    break;
  }
}

/*
 * @brief: prints an expression node. (definition)
 *
 * @param expr: pointer to an expression node.
 */
static void check_expr_and_print(expr_node *expr) {
  switch (expr->kind) {
  case EXPR_TERM:
    check_term_and_print(&expr->term);
    break;
  case EXPR_ADD:
    printf("(");
    check_expr_and_print(expr->binary.left);
    printf(" + ");
    check_expr_and_print(expr->binary.right);
    printf(")");
    break;
  case EXPR_SUBTRACT:
    printf("(");
    check_expr_and_print(expr->binary.left);
    printf(" - ");
    check_expr_and_print(expr->binary.right);
    printf(")");
    break;
  case EXPR_MULTIPLY:
    printf("(");
    check_expr_and_print(expr->binary.left);
    printf(" * ");
    check_expr_and_print(expr->binary.right);
    printf(")");
    break;
  case EXPR_DIVIDE:
    printf("(");
    check_expr_and_print(expr->binary.left);
    printf(" / ");
    check_expr_and_print(expr->binary.right);
    printf(")");
    break;
  case EXPR_MODULO:
    printf("(");
    check_expr_and_print(expr->binary.left);
    printf(" %% ");
    check_expr_and_print(expr->binary.right);
    printf(")");
    break;
  }
}

/*
 * @brief: prints an individual binary node.
 *
 * @param bnode: pointer to a binary node.
 * @param operator: the operator to print between the two nodes.
 */
static void check_binary_node_and_print(term_binary_node *bnode,
                                        char *operator) {
  check_term_and_print(&bnode->lhs);
  printf(" %s ", operator);
  check_term_and_print(&bnode->rhs);
  printf("\n");
}

static void check_rel_node_and_print(rel_node *rel) {
  switch (rel->kind) {
  case REL_IS_EQUAL:
    check_binary_node_and_print(&rel->comparison, "==");
    break;
  case REL_NOT_EQUAL:
    check_binary_node_and_print(&rel->comparison, "!=");
    break;
  case REL_LESS_THAN:
    check_binary_node_and_print(&rel->comparison, "<");
    break;
  case REL_LESS_THAN_OR_EQUAL:
    check_binary_node_and_print(&rel->comparison, "<=");
    break;
  case REL_GREATER_THAN:
    check_binary_node_and_print(&rel->comparison, ">");
    break;
  case REL_GREATER_THAN_OR_EQUAL:
    check_binary_node_and_print(&rel->comparison, ">=");
    break;
  }
}

/*
 * @brief: print an instruction.
 *
 * @param instr: pointer to an instruction.
 */
void print_instr(instr_node *instr) {
  printf("[line %zu] ", instr->line);
  switch (instr->kind) {
  case INSTR_DECLARE:
    printf("declare: ");
    check_var_and_print(&instr->declare_variable);
    printf("\n");
    break;

  case INSTR_INITIALIZE:
    printf("initialize: ");
    check_var_and_print(&instr->initialize_variable.var);
    printf(" = ");
    switch (instr->initialize_variable.var.type) {
    case TYPE_INT:
    case TYPE_POINTER:
      check_expr_and_print(instr->initialize_variable.expr);
      printf("\n");
      break;
    case TYPE_CHAR:
      switch (instr->initialize_variable.expr->kind) {
      case EXPR_TERM:
        printf("\'%c\'\n",
               instr->initialize_variable.expr->term.value.character);
        break;
      default:
        break;
      }
      break;
    default:
      break;
    }
    break;

  case INSTR_ASSIGN:
    printf("assign: ");
    check_var_and_print(&instr->assign.identifier);
    printf(" = ");
    check_expr_and_print(instr->assign.expr);
    printf("\n");
    break;

  case INSTR_ASSIGN_TO_ARRAY_SUBSCRIPT:
    printf("assign to array subscript: ");
    check_var_and_print(&instr->assign_to_array_subscript.var);
    printf("[");
    check_expr_and_print(instr->assign_to_array_subscript.index_expr);
    printf("] = ");
    check_expr_and_print(instr->assign_to_array_subscript.expr_to_assign);
    printf("\n");
    break;

  case INSTR_DECLARE_ARRAY:
    printf("declare array: ");
    check_var_and_print(&instr->declare_array.var);
    printf("[");
    check_expr_and_print(instr->declare_array.size_expr);
    printf("]\n");
    break;

  case INSTR_INITIALIZE_ARRAY:
    printf("initialize array: ");
    check_var_and_print(&instr->initialize_array.var);
    printf("[");
    check_expr_and_print(instr->initialize_array.size_expr);
    printf("] = {");
    for (size_t i = 0; i < instr->initialize_array.literal.elements.count;
         i++) {
      expr_node elem;
      dynamic_array_get(&instr->initialize_array.literal.elements, i, &elem);
      check_expr_and_print(&elem);
      if (i < instr->initialize_array.literal.elements.count - 1) {
        printf(", ");
      }
    }
    printf("}\n");
    break;

  case INSTR_IF:
    printf("if ");
    check_rel_node_and_print(&instr->if_.rel);
    switch (instr->if_.kind) {
    case IF_SINGLE_INSTR:
      printf("\t then: ");
      print_instr(instr->if_.instr);
      break;
    case IF_MULTI_INSTR:
      for (unsigned int i = 0; i < instr->if_.instrs.count; i++) {
        instr_node _instr;
        dynamic_array_get(&instr->if_.instrs, i, &_instr);
        printf("\t");
        print_instr(&_instr);
      }
      break;
    }
    break;

  case INSTR_GOTO:
    printf("goto: %s\n", instr->goto_.label);
    break;

  case INSTR_LABEL:
    printf("label: %s\n", instr->label.label);
    break;

  case INSTR_LOOP:
    switch (instr->loop.kind) {
    case LOOP_UNCONDITIONAL:
      printf("loop %zu starts: \n", instr->loop.loop_id);
      break;
    case LOOP_WHILE:
      printf("while loop %zu starts, break condition: ", instr->loop.loop_id);
      check_rel_node_and_print(&instr->loop.break_condition);
      break;
    case LOOP_DO_WHILE:
      printf("do-loop %zu starts, break condition: ", instr->loop.loop_id);
      check_rel_node_and_print(&instr->loop.break_condition);
      break;
    }
    for (unsigned int i = 0; i < instr->loop.instrs.count; i++) {
      instr_node _instr;
      dynamic_array_get(&instr->loop.instrs, i, &_instr);
      printf("\t");
      print_instr(&_instr);
    }
    break;

  case INSTR_LOOP_BREAK:
    printf("loop break\n");
    break;

  case INSTR_LOOP_CONTINUE:
    printf("loop continue\n");
    break;

  case INSTR_FN_DECLARE:
    printf("function %s: %s(",
           instr->fn_declare_node.kind == FN_DECLARED ? "declaration"
                                                      : "definition",
           instr->fn_declare_node.name);

    for (size_t i = 0; i < instr->fn_declare_node.parameters.count; i++) {
      variable param;
      dynamic_array_get(&instr->fn_declare_node.parameters, i, &param);
      check_var_and_print(&param);
      if (i < instr->fn_declare_node.parameters.count - 1) {
        printf(", ");
      }
    }
    if (instr->fn_declare_node.is_variadic) {
      printf(", ...");
    }
    printf(")");

    if (instr->fn_declare_node.returntypes.count > 0) {
      printf(" : ");
      for (size_t i = 0; i < instr->fn_declare_node.returntypes.count; i++) {
        type ret_type;
        dynamic_array_get(&instr->fn_declare_node.returntypes, i, &ret_type);
        switch (ret_type) {
        case TYPE_INT:
          printf("int");
          break;
        case TYPE_CHAR:
          printf("char");
          break;
        case TYPE_POINTER:
          printf("pointer");
          break;
        default:
          printf("unknown");
          break;
        }
        if (i < instr->fn_declare_node.returntypes.count - 1) {
          printf(", ");
        }
      }
    }
    printf("\n");

    if (instr->fn_declare_node.kind == FN_DEFINED) {
      printf("\tfunction body:\n");
      for (size_t i = 0; i < instr->fn_declare_node.defined.instrs.count; i++) {
        instr_node body_instr;
        dynamic_array_get(&instr->fn_declare_node.defined.instrs, i,
                          &body_instr);
        printf("\t\t");
        print_instr(&body_instr);
      }
    }
    break;

  case INSTR_FN_DEFINE:
    printf("function definition: %s(", instr->fn_define_node.name);
    for (size_t i = 0; i < instr->fn_define_node.parameters.count; i++) {
      variable param;
      dynamic_array_get(&instr->fn_define_node.parameters, i, &param);
      check_var_and_print(&param);
      if (i < instr->fn_define_node.parameters.count - 1) {
        printf(", ");
      }
    }
    if (instr->fn_define_node.is_variadic) {
      printf(", ...");
    }
    printf(")");

    if (instr->fn_define_node.returntypes.count > 0) {
      printf(" : ");
      for (size_t i = 0; i < instr->fn_define_node.returntypes.count; i++) {
        type ret_type;
        dynamic_array_get(&instr->fn_define_node.returntypes, i, &ret_type);
        switch (ret_type) {
        case TYPE_INT:
          printf("int");
          break;
        case TYPE_CHAR:
          printf("char");
          break;
        case TYPE_POINTER:
          printf("pointer");
          break;
        default:
          printf("unknown");
          break;
        }
        if (i < instr->fn_define_node.returntypes.count - 1) {
          printf(", ");
        }
      }
    }
    printf("\n");
    printf("\tfunction body:\n");
    for (size_t i = 0; i < instr->fn_define_node.defined.instrs.count; i++) {
      instr_node body_instr;
      dynamic_array_get(&instr->fn_define_node.defined.instrs, i, &body_instr);
      printf("\t\t");
      print_instr(&body_instr);
    }
    break;

  case INSTR_RETURN:
    printf("return: ");
    if (instr->ret_node.returnvals.count == 0) {
      printf("void\n");
    } else {
      for (size_t i = 0; i < instr->ret_node.returnvals.count; i++) {
        expr_node ret_expr;
        dynamic_array_get(&instr->ret_node.returnvals, i, &ret_expr);
        check_expr_and_print(&ret_expr);
        if (i < instr->ret_node.returnvals.count - 1) {
          printf(", ");
        }
      }
      printf("\n");
    }
    break;

  case INSTR_FN_CALL:
    printf("function call: %s(", instr->fn_call.name);
    for (size_t i = 0; i < instr->fn_call.parameters.count; i++) {
      expr_node arg;
      dynamic_array_get(&instr->fn_call.parameters, i, &arg);
      check_expr_and_print(&arg);
      if (i < instr->fn_call.parameters.count - 1) {
        printf(", ");
      }
    }
    printf(")\n");
    break;
  }
}

void print_ast(ast *program_ast) {
  for (unsigned int i = 0; i < program_ast->instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&program_ast->instrs, i, &instr);
    print_instr(&instr);
  }
}

/*
static void free_fn_calls(ast *program);

static void free_ret_node(ast *program);

static void free_loops(ast *program);

static void free_if_instrs(ast *program) {
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node *instr = program->instrs.items + (i * program->instrs.item_size);
    if (instr->kind == INSTR_IF && instr->if_.kind == IF_MULTI_INSTR) {
      ast temp = {NULL, instr->if_.instrs, 0};
      free_if_instrs(&temp);
      free_fn_calls(&temp);
      free_loops(&temp);
      free_ret_node(&temp);
      dynamic_array_free(&instr->if_.instrs);
    }
  }
}

static void free_loops(ast *program) {
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node *instr = program->instrs.items + (i * program->instrs.item_size);
    if (instr->kind == INSTR_LOOP) {
      ast temp = {NULL, instr->loop.instrs, 0};
      free_if_instrs(&temp);
      free_fn_calls(&temp);
      free_loops(&temp);
      free_ret_node(&temp);
      dynamic_array_free(&instr->loop.instrs);
    }
  }
}

static void free_fn_calls(ast *program) {
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node *instr = program->instrs.items + (i * program->instrs.item_size);
    if (instr->kind == INSTR_FN_CALL) {
      dynamic_array_free(&instr->fn_call.parameters);
    }
  }
}

static void free_ret_node(ast *program) {
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node *instr = program->instrs.items + (i * program->instrs.item_size);
    if (instr->kind == INSTR_RETURN) {
      dynamic_array_free(&instr->ret_node.returnvals);
    }
  }
}

static void free_fns(ast *program) {
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node *instr = program->instrs.items + (i * program->instrs.item_size);
    if (instr->kind == INSTR_FN_DECLARE || instr->kind == INSTR_FN_DEFINE) {
      dynamic_array_free(&instr->fn_declare_node.parameters);
      dynamic_array_free(&instr->fn_declare_node.returntypes);

      if (instr->kind == INSTR_FN_DEFINE) {
        ht_del_ht(instr->fn_define_node.defined.variables);

        ast temp = {NULL, instr->fn_define_node.defined.instrs, 0};

        free_if_instrs(&temp);
        free_fn_calls(&temp);
        free_loops(&temp);
        free_ret_node(&temp);

        dynamic_array_free(&temp.instrs);
      }
    }
  }
}
*/

static void free_expr_node(expr_node *expr);

static void free_exprs(dynamic_array *exprs);

static void free_term_node(term_node *term) {
  switch (term->kind) {
  case TERM_INT:
  case TERM_CHAR:
  case TERM_STRING:
  case TERM_IDENTIFIER:
  case TERM_POINTER:
  case TERM_DEREF:
  case TERM_ADDOF:
    break;
  case TERM_ARRAY_ACCESS:
    free_expr_node(term->array_access.index_expr);
    break;
  case TERM_ARRAY_LITERAL:
    dynamic_array_free(&term->array_literal.elements);
    break;
  case TERM_FUNCTION_CALL:
    free_exprs(&term->fn_call.parameters);
    break;
  }
}

static void free_expr_node(expr_node *expr) {
  switch (expr->kind) {
  case EXPR_TERM:
    free_term_node(&expr->term);
    break;
  case EXPR_ADD:
  case EXPR_SUBTRACT:
  case EXPR_MULTIPLY:
  case EXPR_DIVIDE:
  case EXPR_MODULO:
    free_expr_node(expr->binary.left);
    free_expr_node(expr->binary.right);
    break;
  }
}

static void free_exprs(dynamic_array *exprs) {
  for (unsigned int i = 0; i < exprs->count; i++) {
    expr_node expr;
    dynamic_array_get(exprs, i, &expr);
    free_expr_node(&expr);
  }
  dynamic_array_free(exprs);
}

static void free_rel_node(rel_node *rel) {
  free_term_node(&rel->comparison.lhs);
  free_term_node(&rel->comparison.rhs);
}

static void free_instrs(dynamic_array *instrs);

static void free_instr(instr_node *instr) {
  switch (instr->kind) {
  case INSTR_INITIALIZE:
    free_expr_node(instr->initialize_variable.expr);
    break;

  case INSTR_DECLARE_ARRAY:
    free_expr_node(instr->declare_array.size_expr);
    break;

  case INSTR_INITIALIZE_ARRAY:
    free_expr_node(instr->initialize_array.size_expr);
    free_exprs(&instr->initialize_array.literal.elements);
    break;

  case INSTR_ASSIGN:
    free_expr_node(instr->assign.expr);
    break;

  case INSTR_ASSIGN_TO_ARRAY_SUBSCRIPT:
    free_expr_node(instr->assign_to_array_subscript.index_expr);
    free_expr_node(instr->assign_to_array_subscript.expr_to_assign);
    break;

  case INSTR_IF:
    free_rel_node(&instr->if_.rel);
    if (instr->if_.kind == IF_MULTI_INSTR)
      free_instrs(&instr->if_.instrs);
    else
      free_instr(instr->if_.instr);
    break;

  case INSTR_LOOP:
    free_rel_node(&instr->loop.break_condition);
    free_instrs(&instr->loop.instrs);
    break;

  case INSTR_FN_DEFINE:
    ht_destroy(instr->fn_define_node.defined.variables);
    free_instrs(&instr->fn_define_node.defined.instrs);
  case INSTR_FN_DECLARE:
    dynamic_array_free(&instr->fn_declare_node.returntypes);
    dynamic_array_free(&instr->fn_declare_node.parameters);
    break;

  case INSTR_RETURN:
    free_exprs(&instr->ret_node.returnvals);
    break;

  case INSTR_FN_CALL:
    free_exprs(&instr->fn_call.parameters);
    break;

  case INSTR_DECLARE:
  case INSTR_GOTO:
  case INSTR_LABEL:
  case INSTR_LOOP_BREAK:
  case INSTR_LOOP_CONTINUE:
    break;
  }
}

static void free_instrs(dynamic_array *instrs) {
  for (unsigned int i = 0; i < instrs->count; i++) {
    instr_node instr;
    dynamic_array_get(instrs, i, &instr);
    free_instr(&instr);
  }
  dynamic_array_free(instrs);
}

void free_ast(ast *program_ast) {
  if (program_ast == NULL)
    return;
  free_instrs(&program_ast->instrs);
  arena_free(&program_ast->arena);
}
