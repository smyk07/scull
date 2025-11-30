#include "codegen.h"
#include "ast.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"
#include "ds/stack.h"
#include "fasm_utils.h"
#include "utils.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @brief: generate assembly for a function call
 *
 * @param fn_call: pointer to fn_call_node
 * @param variables: hash table of variables
 * @param program: pointer to program_node
 * @param errors: error counter
 */
static void fn_call_asm(fn_call_node *fn_call, ht *variables,
                        program_node *program, unsigned int *errors);

/*
 * @brief: generate assembly for individual expressions. (declaration)
 *
 * @param instr: pointer ot an instr_node.
 * @param variables: hash table of variables.
 * @param if_count: counter for if instructions.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void instr_asm(instr_node *instr, ht *variables, unsigned int *if_count,
                      stack *loops, ht *functions, program_node *program,
                      unsigned int *errors);

/*
 * @brief: generate assembly for arithmetic expressions. (declaration)
 *
 * @param expr: pointer to an expr_node.
 * @param variables: hash table of variables.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void expr_asm(expr_node *expr, ht *variables, program_node *program,
                     unsigned int *errors);

int evaluate_const_expr(expr_node *expr, unsigned int *errors) {
  if (expr == NULL) {
    return 0;
  }

  switch (expr->kind) {
  case EXPR_TERM:
    if (expr->term.kind == TERM_INT) {
      return expr->term.value.integer;
    }
    scu_perror(errors, "Array size must be a constant expression\n");
    return 0;

  case EXPR_ADD:
    return evaluate_const_expr(expr->binary.left, errors) +
           evaluate_const_expr(expr->binary.right, errors);

  case EXPR_SUBTRACT:
    return evaluate_const_expr(expr->binary.left, errors) -
           evaluate_const_expr(expr->binary.right, errors);

  case EXPR_MULTIPLY:
    return evaluate_const_expr(expr->binary.left, errors) *
           evaluate_const_expr(expr->binary.right, errors);

  case EXPR_DIVIDE: {
    int right = evaluate_const_expr(expr->binary.right, errors);
    if (right == 0) {
      scu_perror(errors, "Division by zero in array size\n");
      return 0;
    }
    return evaluate_const_expr(expr->binary.left, errors) / right;
  }

  case EXPR_MODULO: {
    int right = evaluate_const_expr(expr->binary.right, errors);
    if (right == 0) {
      scu_perror(errors, "Division by zero in array size\n");
      return 0;
    }
    return evaluate_const_expr(expr->binary.left, errors) % right;
  }
  }
}

/*
 * @brief: get array size from declare_array_node
 *
 * @param node: pointer to declare_array_node
 * @param errors: counter variable to increment when an error is encountered.
 *
 * @return: size of the array in elements
 */
static int get_array_size_declare(declare_array_node *node,
                                  unsigned int *errors) {
  if (node == NULL || node->size_expr == NULL) {
    return 0;
  }
  return evaluate_const_expr(node->size_expr, errors);
}

/*
 * @brief: get array size from initialize_array_node
 *
 * @param node: pointer to initialize_array_node
 * @param errors: counter variable to increment when an error is encountered.
 *
 * @return: size of the array in elements
 */
static int get_array_size_initialize(initialize_array_node *node,
                                     unsigned int *errors) {
  if (node == NULL || node->size_expr == NULL) {
    return 0;
  }
  return evaluate_const_expr(node->size_expr, errors);
}

/*
 * @brief: get the absolute stack offset for an array
 */
static size_t get_array_base_offset(program_node *program, variable *array_var,
                                    ht *variables, unsigned int *errors) {
  size_t offset = variables->count * 8;

  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);

    if (instr.kind == INSTR_DECLARE_ARRAY) {
      if (strcmp(instr.declare_array.var.name, array_var->name) == 0) {
        int size = get_array_size_declare(&instr.declare_array, errors);
        return offset + (size * 4);
      }
      int size = get_array_size_declare(&instr.declare_array, errors);
      offset += size * 4;
    } else if (instr.kind == INSTR_INITIALIZE_ARRAY) {
      if (strcmp(instr.initialize_array.var.name, array_var->name) == 0) {
        int size = get_array_size_initialize(&instr.initialize_array, errors);
        return offset + (size * 4);
      }
      int size = get_array_size_initialize(&instr.initialize_array, errors);
      offset += size * 4;
    }
  }
  return offset;
}

/*
 * @brief: generate assembly for terms.
 *
 * @param term: pointer to a term_node.
 * @param variables: hash table of variables.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void term_asm(term_node *term, ht *variables, program_node *program,
                     unsigned int *errors) {
  switch (term->kind) {
  case TERM_INT:
    emit("mov rax, %d\n", term->value.integer);
    break;
  case TERM_CHAR: {
    emit("mov rax, %d\n", term->value.character);
    break;
  }
  case TERM_IDENTIFIER: {
    int index = get_var_stack_offset(variables, &term->identifier, NULL);
    if (term->identifier.is_array)
      emit("lea rax, [rbp - %d]\n", index * 8 + 8);
    else
      emit("mov rax, qword [rbp - %d]\n", index * 8 + 8);
    break;
  }
  case TERM_POINTER:
    break;
  case TERM_DEREF: {
    int index = get_var_stack_offset(variables, &term->identifier, NULL);
    emit("mov rbx, qword [rbp - %d]\n", index * 8 + 8);
    emit("mov rax, qword [rbx]\n");
    break;
  }
  case TERM_ADDOF: {
    int index = get_var_stack_offset(variables, &term->identifier, NULL);
    emit("lea rax, [rbp - %d]\n", index * 8 + 8);
    break;
  }

  case TERM_ARRAY_ACCESS: {
    size_t array_base = get_array_base_offset(
        program, &term->array_access.array_var, variables, errors);
    expr_asm(term->array_access.index_expr, variables, program, errors);
    emit("cdqe\n");
    emit("lea rdx, [rbp - %zu]\n", array_base);
    emit("mov eax, dword [rdx + rax*4]\n");
    break;
  }

  case TERM_ARRAY_LITERAL: {
    // nothing here cuz its done at initialization itself
    break;
  }

  case TERM_FUNCTION_CALL: {
    fn_call_asm(&term->fn_call, variables, program, errors);
    break;
  }
  }
}

/*
 * @brief: generate assembly for arithmetic expressions. (definition)
 *
 * @param expr: pointer to an expr_node.
 * @param variables: hash table of variables.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void expr_asm(expr_node *expr, ht *variables, program_node *program,
                     unsigned int *errors) {
  switch (expr->kind) {
  case EXPR_TERM:
    term_asm(&expr->term, variables, program, errors);
    break;
  case EXPR_ADD:
    expr_asm(expr->binary.left, variables, program, errors);
    emit("push rax\n");
    expr_asm(expr->binary.right, variables, program, errors);
    emit("pop rdx\n");
    emit("add rax, rdx\n");
    break;
  case EXPR_SUBTRACT:
    expr_asm(expr->binary.left, variables, program, errors);
    emit("push rax\n");
    expr_asm(expr->binary.right, variables, program, errors);
    emit("mov rdx, rax\n");
    emit("pop rax\n");
    emit("sub rax, rdx\n");
    break;
  case EXPR_MULTIPLY:
    expr_asm(expr->binary.left, variables, program, errors);
    emit("push rax\n");
    expr_asm(expr->binary.right, variables, program, errors);
    emit("pop rdx\n");
    emit("imul rax, rdx\n");
    break;
  case EXPR_DIVIDE:
  case EXPR_MODULO:
    expr_asm(expr->binary.left, variables, program, errors);
    emit("push rax\n");
    expr_asm(expr->binary.right, variables, program, errors);
    emit("mov rcx, rax\n");
    emit("pop rax\n");
    emit("cqo\n");
    emit("idiv rcx\n");
    if (expr->kind == EXPR_MODULO) {
      emit("mov rax, rdx\n");
    }
    break;
  }
}

/*
 * @brief: generate assembly for relational expressions
 *
 * @param rel: pointer to a rel_node.
 * @param variables: hash table of variables.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void rel_asm(rel_node *rel, ht *variables, program_node *program,
                    unsigned int *errors) {
  term_asm(&rel->comparison.lhs, variables, program, errors);
  emit("push rax\n");
  term_asm(&rel->comparison.rhs, variables, program, errors);
  emit("pop rdx\n");
  emit("cmp rdx, rax\n");

  switch (rel->kind) {
  case REL_IS_EQUAL:
    emit("sete al\n");
    break;
  case REL_NOT_EQUAL:
    emit("setne al\n");
    break;
  case REL_LESS_THAN:
    emit("setl al\n");
    break;
  case REL_LESS_THAN_OR_EQUAL:
    emit("setle al\n");
    break;
  case REL_GREATER_THAN:
    emit("setg al\n");
    break;
  case REL_GREATER_THAN_OR_EQUAL:
    emit("setge al\n");
    break;
  }

  emit("movzx rax, al\n");
}

/*
 * @brief: calculate stack size for a function
 *
 * @param fn: pointer to fn_node
 * @param errors: error counter
 * @return: total stack size needed (aligned to 16 bytes)
 */
static size_t calculate_function_stack_size(fn_node *fn, unsigned int *errors) {
  if (fn->kind != FN_DEFINED)
    return 0;

  size_t total = fn->parameters.count * 8;
  total += fn->defined.variables->count * 8;

  for (size_t i = 0; i < fn->defined.instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&fn->defined.instrs, i, &instr);

    if (instr.kind == INSTR_DECLARE_ARRAY) {
      total += get_array_size_declare(&instr.declare_array, errors) * 4;
    } else if (instr.kind == INSTR_INITIALIZE_ARRAY) {
      total += get_array_size_initialize(&instr.initialize_array, errors) * 4;
    }
  }

  if (total % 16 != 0) {
    total += 16 - (total % 16);
  }

  return total;
}

/*
 * @brief: generate assembly for a return statement
 *
 * @param ret: pointer to return_node
 * @param variables: hash table of variables
 * @param program: pointer to program_node
 * @param stack_size: size of function's stack frame
 * @param errors: error counter
 */
static void return_asm(return_node *ret, ht *variables, program_node *program,
                       size_t stack_size, unsigned int *errors) {
  if (ret->returnvals.count > 0) {
    expr_node ret_expr;
    dynamic_array_get(&ret->returnvals, 0, &ret_expr);
    expr_asm(&ret_expr, variables, program, errors);
  } else {
    emit("xor rax, rax\n");
  }

  if (stack_size > 0) {
    emit("add rsp, %zu\n", stack_size);
  }
  emit("pop rbp\n");
  emit("ret\n");
}

/*
 * @brief: generate assembly for a function definition
 *
 * @param fn: pointer to fn_node
 * @param functions: hash table of all functions
 * @param errors: error counter
 */
static void function_asm(fn_node *fn, ht *functions, unsigned int *errors) {
  if (fn->kind != FN_DEFINED)
    return;

  emit_without_indent("\npublic %s\n"
                      "%s:\n",
                      fn->name, fn->name);

  emit("push rbp\n");
  emit("mov rbp, rsp\n");

  size_t stack_size = calculate_function_stack_size(fn, errors);
  if (stack_size > 0) {
    emit("sub rsp, %zu\n", stack_size);
  }

  const char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  for (size_t i = 0; i < fn->parameters.count && i < 6; i++) {
    variable param;
    dynamic_array_get(&fn->parameters, i, &param);
    emit("mov qword [rbp - %zu], %s\n", (i + 1) * 8, arg_regs[i]);
  }

  unsigned int if_count = 0;
  stack loops;
  stack_init(&loops, sizeof(loop_node));

  program_node fn_program = {.loop_counter = 0, .instrs = fn->defined.instrs};

  int has_return = 0;
  for (size_t i = 0; i < fn->defined.instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&fn->defined.instrs, i, &instr);

    if (instr.kind == INSTR_RETURN) {
      return_asm(&instr.ret_node, fn->defined.variables, &fn_program,
                 stack_size, errors);
      has_return = 1;
    } else {
      instr_asm(&instr, fn->defined.variables, &if_count, &loops, functions,
                &fn_program, errors);
    }
  }

  if (!has_return) {
    emit("xor rax, rax\n");
    emit("add rsp, %zu\n", stack_size);
    emit("pop rbp\n");
    emit("ret\n");
  }

  stack_free(&loops);
}

/*
 * @brief: generate assembly for a function call
 *
 * @param fn_call: pointer to fn_call_node
 * @param variables: hash table of variables
 * @param program: pointer to program_node
 * @param errors: error counter
 */
static void fn_call_asm(fn_call_node *fn_call, ht *variables,
                        program_node *program, unsigned int *errors) {
  const char *arg_regs[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

  size_t stack_arg_count = 0;
  if (fn_call->parameters.count > 6) {
    stack_arg_count = fn_call->parameters.count - 6;
  }

  if (stack_arg_count > 0) {
    for (int i = fn_call->parameters.count - 1; i >= 6; i--) {
      expr_node arg;
      dynamic_array_get(&fn_call->parameters, i, &arg);
      expr_asm(&arg, variables, program, errors);
      emit("push rax\n");
    }
  }

  size_t total_pushed = stack_arg_count * 8;
  int needs_padding = 0;
  if ((total_pushed + 8) % 16 != 0) {
    needs_padding = 1;
    emit("sub rsp, 8\n");
  }

  if (fn_call->parameters.count > 1 && fn_call->parameters.count <= 6) {
    for (size_t i = 0; i < fn_call->parameters.count; i++) {
      expr_node arg;
      dynamic_array_get(&fn_call->parameters, i, &arg);
      expr_asm(&arg, variables, program, errors);
      emit("push rax\n");
    }
    for (int i = fn_call->parameters.count - 1; i >= 0; i--) {
      emit("pop %s\n", arg_regs[i]);
    }
  } else if (fn_call->parameters.count == 1) {
    expr_node arg;
    dynamic_array_get(&fn_call->parameters, 0, &arg);
    expr_asm(&arg, variables, program, errors);
    emit("mov %s, rax\n", arg_regs[0]);
  } else if (fn_call->parameters.count > 6) {
    for (size_t i = 0; i < 6; i++) {
      expr_node arg;
      dynamic_array_get(&fn_call->parameters, i, &arg);
      expr_asm(&arg, variables, program, errors);
      emit("push rax\n");
    }
    for (int i = 5; i >= 0; i--) {
      emit("pop %s\n", arg_regs[i]);
    }
  }

  emit("call %s\n", fn_call->name);

  if (needs_padding) {
    emit("add rsp, 8\n");
  }
  if (stack_arg_count > 0) {
    emit("add rsp, %zu\n", stack_arg_count * 8);
  }
}

/*
 * @brief: generate assembly for individual expressions. (definition)
 *
 * @param instr: pointer ot an instr_node.
 * @param variables: hash table of variables.
 * @param if_count: counter for if instructions.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void instr_asm(instr_node *instr, ht *variables, unsigned int *if_count,
                      stack *loops, ht *functions, program_node *program,
                      unsigned int *errors) {
  switch (instr->kind) {
  case INSTR_DECLARE:
    break;

  case INSTR_INITIALIZE: {
    int index =
        get_var_stack_offset(variables, &instr->initialize_variable.var, NULL);
    expr_asm(&instr->initialize_variable.expr, variables, program, errors);
    emit("mov qword [rbp - %d], rax\n", index * 8 + 8);
    break;
  }

  case INSTR_ASSIGN: {
    int index =
        get_var_stack_offset(variables, &instr->assign.identifier, NULL);
    expr_asm(&instr->assign.expr, variables, program, errors);
    if (instr->assign.identifier.type == TYPE_POINTER) {
      emit("mov rbx, qword [rbp - %d]\n", index * 8 + 8);
      emit("mov qword [rbx], rax\n");
    } else {
      emit("mov qword [rbp - %d], rax\n", index * 8 + 8);
    }
    break;
  }

  case INSTR_ASSIGN_TO_ARRAY_SUBSCRIPT:
    size_t array_base = get_array_base_offset(
        program, &instr->assign_to_array_subscript.var, variables, errors);
    expr_asm(&instr->assign_to_array_subscript.expr_to_assign, variables,
             program, errors);
    emit("push rax\n");
    expr_asm(instr->assign_to_array_subscript.index_expr, variables, program,
             errors);
    emit("mov rcx, rax\n");
    emit("lea rdx, [rbp - %zu]\n", array_base);
    emit("pop rax\n");
    emit("mov dword [rdx + rcx * 4], eax\n");
    break;

  case INSTR_DECLARE_ARRAY: {
    break;
  }

  case INSTR_INITIALIZE_ARRAY: {
    size_t array_base = get_array_base_offset(
        program, &instr->initialize_array.var, variables, errors);
    emit("lea rdx, [rbp - %zu]\n", array_base);

    for (size_t i = 0; i < instr->initialize_array.literal.elements.count;
         i++) {
      expr_node elem;
      dynamic_array_get(&instr->initialize_array.literal.elements, i, &elem);

      expr_asm(&elem, variables, program, errors);
      emit("mov dword [rdx + %zu], eax\n", i * 4);
    }
    break;
  }

  case INSTR_IF: {
    rel_asm(&instr->if_.rel, variables, program, errors);
    int label = (*if_count)++;
    emit("test rax, rax\n");
    emit("jz .endif%d\n", label);
    switch (instr->if_.kind) {
    case IF_SINGLE_INSTR:
      instr_asm(instr->if_.instr, variables, if_count, loops, functions,
                program, errors);
      break;

    case IF_MULTI_INSTR:
      for (size_t i = 0; i < instr->if_.instrs.count; i++) {
        struct instr_node _instr;
        dynamic_array_get(&instr->if_.instrs, i, &_instr);
        instr_asm(&_instr, variables, if_count, loops, functions, program,
                  errors);
      }
      break;
    }
    emit(".endif%d:\n", label);
    break;
  }

  case INSTR_GOTO:
    emit("jmp .%s\n", instr->goto_.label);
    break;

  case INSTR_LABEL:
    emit_without_indent(".%s:\n", instr->label.label);
    break;

  case INSTR_FASM:
    if (instr->fasm.kind == FASM_PAR) {
      int index = get_var_stack_offset(variables, &instr->fasm.argument, NULL);
      char *stmt =
          scu_format_string((char *)instr->fasm.content, index * 8 + 8);
      emit("%s\n", stmt);
      free(stmt);
    } else {
      emit("%s\n", instr->fasm.content);
    }
    break;

  case INSTR_LOOP:
    loop_node new_loop = {.kind = instr->loop.kind,
                          .loop_id = instr->loop.loop_id,
                          .instrs = {0},
                          .break_condition = {0}};
    stack_push(loops, &new_loop);

    switch (instr->loop.kind) {
    case LOOP_UNCONDITIONAL:
      emit_without_indent(".loop_%zu_start:\n", instr->loop.loop_id);
      for (unsigned int i = 0; i < instr->loop.instrs.count; i++) {
        struct instr_node _instr;
        dynamic_array_get(&instr->loop.instrs, i, &_instr);
        instr_asm(&_instr, variables, if_count, loops, functions, program,
                  errors);
      }
      emit_without_indent(".loop_%zu_end:\n", instr->loop.loop_id);
      break;

    case LOOP_WHILE:
      emit("jmp .loop_%zu_test\n", instr->loop.loop_id);

    case LOOP_DO_WHILE:
      emit_without_indent(".loop_%zu_start:\n", instr->loop.loop_id);
      for (unsigned int i = 0; i < instr->loop.instrs.count; i++) {
        struct instr_node _instr;
        dynamic_array_get(&instr->loop.instrs, i, &_instr);
        instr_asm(&_instr, variables, if_count, loops, functions, program,
                  errors);
      }
      emit_without_indent(".loop_%zu_test:\n", instr->loop.loop_id);
      rel_asm(&instr->loop.break_condition, variables, program, errors);
      emit("test rax, rax\n");
      emit("jz .loop_%zu_end\n", instr->loop.loop_id);
      emit("jmp .loop_%zu_start\n", instr->loop.loop_id);
      emit_without_indent(".loop_%zu_end:\n", instr->loop.loop_id);
      break;
    }

    loop_node loop;
    stack_pop(loops, &loop);
    break;

  case INSTR_LOOP_BREAK: {
    loop_node *_loop = stack_top(loops);
    emit("jmp .loop_%zu_end\n", _loop->loop_id);
    break;
  }

  case INSTR_LOOP_CONTINUE:
    loop_node *_loop = stack_top(loops);
    switch (_loop->kind) {
    case LOOP_UNCONDITIONAL:
      emit("jmp .loop_%zu_start\n", _loop->loop_id);
      break;
    case LOOP_WHILE:
    case LOOP_DO_WHILE:
      emit("jmp .loop_%zu_test\n", _loop->loop_id);
      break;
    }
    break;

  case INSTR_FN_CALL:
    fn_call_asm(&instr->fn_call, variables, program, errors);
    break;

  case INSTR_RETURN:
    if (instr->ret_node.returnvals.count > 0) {
      expr_node ret_expr;
      dynamic_array_get(&instr->ret_node.returnvals, 0, &ret_expr);
      expr_asm(&ret_expr, variables, program, errors);
    } else {
      emit("xor rax, rax\n");
    }
    break;

  default:
    break;
  }
}

static size_t calculate_total_stack_size(ht *variables, program_node *program,
                                         unsigned int *errors) {
  size_t total = variables->count * 8;

  for (unsigned int i = 0; i < program->instrs.count; i++) {
    instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);

    if (instr.kind == INSTR_DECLARE_ARRAY) {
      total += get_array_size_declare(&instr.declare_array, errors) * 4;
    } else if (instr.kind == INSTR_INITIALIZE_ARRAY) {
      total += get_array_size_initialize(&instr.initialize_array, errors) * 4;
    }
  }

  if (total % 16 != 0) {
    total += 16 - (total % 16);
  }
  return total;
}

void instrs_to_asm(program_node *program, ht *variables, stack *loops,
                   ht *functions, const char *filename, unsigned int *errors) {
  unsigned int if_count = 0;

  char *output_asm_file = scu_format_string("%s.s", filename);
  init_fasm_output(output_asm_file);

  int has_main = 0;
  fn_node *main_fn = ht_search(functions, "main");
  if (main_fn) {
    has_main = 1;
  }

  emit_without_indent("format ELF64\n");

  if (has_main) {
    emit_without_indent("LINE_MAX equ 1024\n");
    emit_without_indent("public _start\n");
  }

  for (unsigned int i = 0; i < program->instrs.count; i++) {
    struct instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);
    if (instr.kind == INSTR_FN_DECLARE) {
      emit_without_indent("\nextrn %s\n", instr.fn_declare_node.name);
    }
  }
  emit_without_indent("\n");

  emit_without_indent("section '.text' executable\n");

  for (unsigned int i = 0; i < program->instrs.count; i++) {
    struct instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);

    if (instr.kind == INSTR_FASM_DEFINE) {
      emit_without_indent("%s\n", instr.fasm_def.content);
      dynamic_array_remove(&program->instrs, i);
      i--;
    }
  }

  for (unsigned int i = 0; i < program->instrs.count; i++) {
    struct instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);

    if (instr.kind == INSTR_FN_DEFINE) {
      function_asm(&instr.fn_define_node, functions, errors);
    }
  }

  int has_global_code = 0;
  for (unsigned int i = 0; i < program->instrs.count; i++) {
    struct instr_node instr;
    dynamic_array_get(&program->instrs, i, &instr);

    if (instr.kind != INSTR_FN_DEFINE && instr.kind == INSTR_FN_DECLARE &&
        instr.kind != INSTR_FASM_DEFINE) {
      has_global_code = 1;
      break;
    }
  }

  if (has_global_code) {
    emit_without_indent("\n__global:\n");
    emit("push rbp\n");
    emit("mov rbp, rsp\n");

    size_t stack_size = calculate_total_stack_size(variables, program, errors);
    if (stack_size > 0) {
      emit("sub rsp, %zu\n", stack_size);
    }

    for (unsigned int i = 0; i < program->instrs.count; i++) {
      struct instr_node instr;
      dynamic_array_get(&program->instrs, i, &instr);

      if (instr.kind == INSTR_FN_DEFINE || instr.kind == INSTR_FN_DECLARE ||
          instr.kind == INSTR_FASM_DEFINE) {
        continue;
      }

      instr_asm(&instr, variables, &if_count, loops, functions, program,
                errors);
    }

    if (stack_size > 0) {
      emit("add rsp, %zu\n", stack_size);
    }
    emit("pop rbp\n");
    emit("ret\n");
  }

  if (has_main) {
    emit_without_indent("\n_start:\n");
    if (has_global_code) {
      emit("call __global\n");
    }
    emit("call main\n");
    emit("mov rdi, rax\n");
    emit("mov rax, 60\n");
    emit("syscall\n\n");
  }

  if (has_main) {
    emit_without_indent("section '.data' writeable\n");
    emit_without_indent("line rb LINE_MAX\n");
    emit_without_indent("newline db 10, 0\n");
    emit_without_indent("char_buf db 0, 0\n");
  }

  close_fasm_output();
}
