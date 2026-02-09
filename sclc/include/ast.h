/*
 * ast: Abstract Syntax Tree implementation and node definitions.
 */

#ifndef AST
#define AST

#include "ds/arena.h"
#include "ds/dynamic_array.h"
#include "token.h"
#include "var.h"

#include <stddef.h>
#include <stdint.h>

/*
 * @enum term_kind: enumeration of all the terms supported by the parser.
 */
typedef enum term_kind {
  TERM_INT = 0,
  TERM_CHAR,
  TERM_STRING,
  TERM_IDENTIFIER,
  TERM_POINTER,
  TERM_DEREF,
  TERM_ADDOF,
  TERM_ARRAY_ACCESS,
  TERM_ARRAY_LITERAL,
  TERM_FUNCTION_CALL,
} term_kind;

typedef struct expr_node expr_node;

/*
 * @struct array_access_node: represents an array access or subscript node.
 */
typedef struct array_access_node {
  variable array_var;
  expr_node *index_expr;
} array_access_node;

/*
 * @struct array_literal_node: represents an array subscript node used to
 * declare and define arrays.
 */
typedef struct array_literal_node {
  dynamic_array elements;
} array_literal_node;

typedef struct fn_call_node {
  char *name;
  dynamic_array parameters;
} fn_call_node;

/*
 * @struct term_node: represents a term.
 */
typedef struct term_node {
  term_kind kind;
  size_t line;
  union {
    token_literal_value value;
    variable identifier;
    array_access_node array_access;
    array_literal_node array_literal;
    fn_call_node fn_call;
  };
} term_node;

/*
 * @struct term_binary_node: represents a binary term.
 */
typedef struct term_binary_node {
  term_node lhs;
  term_node rhs;
} term_binary_node;

/*
 * @enum expr_kind: enumeration of all the expressions supported by the parser.
 */
typedef enum expr_kind {
  EXPR_TERM = 0,
  EXPR_ADD,
  EXPR_SUBTRACT,
  EXPR_MULTIPLY,
  EXPR_DIVIDE,
  EXPR_MODULO
} expr_kind;

/*
 * @struct expr_node: represents an expression.
 */
typedef struct expr_node {
  expr_kind kind;
  size_t line;
  union {
    term_node term;
    struct {
      struct expr_node *left;
      struct expr_node *right;
    } binary;
  };
} expr_node;

/*
 * @enum rel_kind: enumeration of all the relational operations supported by the
 * parser.
 */
typedef enum rel_kind {
  REL_IS_EQUAL = 0,
  REL_NOT_EQUAL,
  REL_LESS_THAN,
  REL_LESS_THAN_OR_EQUAL,
  REL_GREATER_THAN,
  REL_GREATER_THAN_OR_EQUAL,
} rel_kind;

/*
 * @struct rel_node: represents a relational expression.
 */
typedef struct rel_node {
  rel_kind kind;
  size_t line;
  term_binary_node comparison;
} rel_node;

/*
 * @enum instr_kind: enumeration of all the instructions supported by the
 * parser.
 */
typedef enum instr_kind {
  INSTR_DECLARE = 0,
  INSTR_INITIALIZE,
  INSTR_DECLARE_ARRAY,
  INSTR_INITIALIZE_ARRAY,
  INSTR_ASSIGN,
  INSTR_ASSIGN_TO_ARRAY_SUBSCRIPT,
  INSTR_IF,
  INSTR_MATCH,
  INSTR_GOTO,
  INSTR_LABEL,
  INSTR_LOOP,
  INSTR_LOOP_BREAK,
  INSTR_LOOP_CONTINUE,
  INSTR_FN_DEFINE,
  INSTR_FN_DECLARE,
  INSTR_RETURN,
  INSTR_FN_CALL,
} instr_kind;

/*
 * @struct instr_node: represents an instruction. (declaration)
 *
 * all the child structs for each instruction defined below.
 */
typedef struct instr_node instr_node;

typedef struct initialize_variable_node {
  variable var;
  expr_node *expr;
} initialize_variable_node;

typedef struct declare_array_node {
  variable var;
  expr_node *size_expr;
} declare_array_node;

typedef struct initialize_array_node {
  variable var;
  expr_node *size_expr;
  array_literal_node literal;
} initialize_array_node;

typedef struct assign_node {
  variable identifier;
  expr_node *expr;
} assign_node;

typedef struct assign_to_array_subscript_node {
  variable var;
  expr_node *index_expr;
  expr_node *expr_to_assign;
} assign_to_array_subscript_node;

typedef enum cond_block_kind {
  COND_SINGLE_INSTR = 0,
  COND_MULTI_INSTR
} cond_block_kind;

typedef struct cond_block_node {
  cond_block_kind kind;

  union {
    instr_node *single;
    dynamic_array multi;
  };
} cond_block_node;

typedef struct if_node {
  rel_node rel;

  cond_block_node then;
  dynamic_array else_ifs;
  cond_block_node *else_;
} if_node;

typedef enum match_case_kind {
  MATCH_CASE_VALUES = 0, // 1, 2, 3
  MATCH_CASE_RANGE,      // 1...10
  MATCH_CASE_DEFAULT     // _
} match_case_kind;

typedef struct match_case_values_node {
  dynamic_array values;
} match_case_values_node;

typedef struct match_case_range_node {
  expr_node *start;
  expr_node *end;
} match_case_range_node;

typedef struct match_case_node {
  match_case_kind kind;

  union {
    match_case_values_node values;
    match_case_range_node range;
  };

  cond_block_node body;
} match_case_node;

typedef struct match_node {
  expr_node *expr;
  dynamic_array cases;
} match_node;

typedef struct goto_node {
  const char *label;
} goto_node;

typedef struct label_node {
  const char *label;
} label_node;

typedef enum loop_kind {
  LOOP_UNCONDITIONAL = 0,
  LOOP_WHILE,
  LOOP_DO_WHILE,
  LOOP_FOR
} loop_kind;

typedef struct loop_node {
  loop_kind kind;

  ht *variables;
  dynamic_array instrs;

  union {
    struct {
      uint8_t placeholder_buffer;
    } unconditional;

    struct {
      rel_node break_condition;
    } conditional;

    struct {
      variable iterator;
      expr_node *range_start;
      expr_node *range_end;
    } _for;
  };
} loop_node;

typedef enum fn_kind {
  FN_DEFINED = 0,
  FN_DECLARED,
} fn_kind;

typedef struct fn_node {
  char *name;
  fn_kind kind;
  dynamic_array returntypes;

  bool is_variadic;
  dynamic_array parameters;

  union {
    struct {
      ht *variables;
      dynamic_array instrs;
    } defined;

    struct {
      // JUST A PLACEHOLDER, MIGHT APPEAR IN WARNINGS
      uint8_t *placeholder_buffer;
    } declared;
  };
} fn_node;

typedef struct return_node {
  dynamic_array returnvals;
} return_node;

/*
 * @struct instr_node: represents an instruction. (definition)
 */
typedef struct instr_node {
  instr_kind kind;
  size_t line;
  union {
    variable declare_variable;
    initialize_variable_node initialize_variable;
    declare_array_node declare_array;
    initialize_array_node initialize_array;
    assign_node assign;
    assign_to_array_subscript_node assign_to_array_subscript;
    if_node if_;
    match_node match;
    goto_node goto_;
    label_node label;
    loop_node loop;
    fn_node fn_define_node;
    fn_node fn_declare_node;
    return_node ret_node;
    fn_call_node fn_call;
  };
} instr_node;

/*
 * @struct ast: Contains the abstract syntax tree for a compilation unit.
 */
typedef struct ast {
  mem_arena arena;
  dynamic_array instrs;
} ast;

/*
 * @brief: initializes an ast struct.
 */
void ast_init(ast *a);

/*
 * @brief: Frees all memory associated with an ast.
 *
 * @param program_ast: pointer to an initialized ast
 */
void ast_free(ast *program_ast);

/*
 * @brief: prints all information about a single instruction.
 *
 * @param program: pointer to an instruction node
 */
void print_instr(instr_node *instr);

/*
 * @brief: prints all the information about the whole AST (all instructions).
 *
 * @param program: pointer to an ast
 */
void print_ast(ast *program_ast);

#endif // !AST
