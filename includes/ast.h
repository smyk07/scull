/*
 * ast: Abstract Syntax Tree implementation and node definitions.
 */

#ifndef AST
#define AST

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
  TERM_IDENTIFIER,
  TERM_POINTER,
  TERM_DEREF,
  TERM_ADDOF,
  TERM_ARRAY_ACCESS,
  TERM_ARRAY_LITERAL,
  TERM_FUNCTION_CALL,
} term_kind;

/*
 * @struct array_access_node: represents an array access or subscript node.
 */
typedef struct array_access_node {
  variable array_var;
  struct expr_node *index_expr;
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
    token_value value;
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
  expr_node expr;
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
  expr_node expr;
} assign_node;

typedef struct assign_to_array_subscript_node {
  variable var;
  expr_node *index_expr;
  expr_node expr_to_assign;
} assign_to_array_subscript_node;

typedef enum if_node_kind { IF_SINGLE_INSTR = 0, IF_MULTI_INSTR } if_node_kind;

typedef struct if_node {
  if_node_kind kind;
  rel_node rel;
  union {
    instr_node *instr;
    dynamic_array instrs;
  };
} if_node;

typedef struct goto_node {
  const char *label;
} goto_node;

typedef struct label_node {
  const char *label;
} label_node;

typedef enum loop_kind {
  LOOP_UNCONDITIONAL = 0,
  LOOP_WHILE,
  LOOP_DO_WHILE
} loop_kind;

typedef struct loop_node {
  loop_kind kind;
  size_t loop_id;
  rel_node break_condition;
  dynamic_array instrs;
} loop_node;

typedef enum fn_kind {
  FN_DEFINED = 0,
  FN_DECLARED,
} fn_kind;

typedef struct fn_node {
  char *name;
  fn_kind kind;
  dynamic_array returntypes;
  dynamic_array parameters;

  union {
    struct {
      ht *variables;
      dynamic_array instrs;
    } defined;

    struct {
      //...
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
 * @struct program_node: wrapper around a dynamic_array of instructions.
 */
typedef struct program_node {
  size_t loop_counter;
  dynamic_array instrs;
} program_node;

#endif // !AST
