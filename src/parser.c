#include "parser.h"
#include "arena.h"
#include "ast.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"
#include "lexer.h"
#include "token.h"
#include "utils.h"
#include "var.h"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static mem_arena *parser_arena;

void parser_init(dynamic_array *tokens, parser *p) {
  p->tokens = *tokens;
  p->index = 0;
}

/*
 * @brief: check the token at the current position of the parser.
 *
 * @param p: pointer to the parser state.
 * @param token: pointer to a new un-initialized token struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parser_current(parser *p, token *token) {
  dynamic_array_get(&p->tokens, p->index, token);
  if (token->kind == TOKEN_END) {
    scu_check_errors();
  }
}

/*
 * @brief: advance the parser state to the next token.
 *
 * @param p: pointer to the parser state.
 */
static void parser_advance(parser *p) { p->index++; }

/*
 * @brief: parse a arithmetic expression. (declaration)
 *
 * @param p: pointer to the parser state.
 * @param errors: counter variable to increment when an error is encountered.
 */
static expr_node *parse_expr(parser *p);

/*
 * @brief: parse an individual term.
 *
 * @param p: pointer to the parser state.
 * @param term: pointer to an un-initialized term_node struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_term_for_expr(parser *p, term_node *term) {
  token token = {0};

  parser_current(p, &token);
  term->line = token.line;
  if (token.kind == TOKEN_INT) {
    term->kind = TERM_INT;
    term->value.integer = token.value.integer;
    parser_advance(p);
  } else if (token.kind == TOKEN_CHAR) {
    term->kind = TERM_CHAR;
    term->value.character = token.value.character;
    parser_advance(p);
  } else if (token.kind == TOKEN_STRING) {
    term->kind = TERM_STRING;
    term->value.str = token.value.str;
    parser_advance(p);
  } else if (token.kind == TOKEN_IDENTIFIER) {
    term->kind = TERM_IDENTIFIER;
    term->identifier.line = token.line;
    term->identifier.name = token.value.str;

    parser_advance(p);
    parser_current(p, &token);
    if (token.kind == TOKEN_LSQBR) {
      term->kind = TERM_ARRAY_ACCESS;
      term->array_access.array_var.name = term->identifier.name;
      term->array_access.array_var.line = term->identifier.line;
      parser_advance(p);
      term->array_access.index_expr = parse_expr(p);
      parser_current(p, &token);
      if (token.kind != TOKEN_RSQBR) {
        scu_perror("Expected ']' at line %d\n", token.line);
      }
      parser_advance(p);
    } else if (token.kind == TOKEN_LPAREN) {
      term->kind = TERM_FUNCTION_CALL;
      term->fn_call.name = term->identifier.name;

      dynamic_array_init(&term->fn_call.parameters, sizeof(expr_node));
      parser_advance(p);
      parser_current(p, &token);

      while (token.kind != TOKEN_RPAREN) {
        expr_node *arg = parse_expr(p);
        dynamic_array_append(&term->fn_call.parameters, arg);

        parser_current(p, &token);
        if (token.kind == TOKEN_COMMA) {
          parser_advance(p);
          parser_current(p, &token);
        }
      }

      if (token.kind != TOKEN_RPAREN) {
        scu_perror("Expected ')' at line %d\n", token.line);
      }
      parser_advance(p);
    }
  } else if (token.kind == TOKEN_ADDRESS_OF) {
    term->kind = TERM_ADDOF;
    term->identifier.line = token.line;
    term->identifier.name = token.value.str;
    parser_advance(p);
  } else if (token.kind == TOKEN_POINTER) {
    term->kind = TERM_DEREF;
    term->identifier.line = token.line;
    term->identifier.name = token.value.str;
    parser_advance(p);
  } else {
    scu_perror("Expected a term (input, int, char, identifier, addof, "
               "pointer), got %s [line %d]\n",
               lexer_token_kind_to_str(token.kind), token.line);
    parser_advance(p);
  }
}

/*
 * @brief: parse a factor inside an arithmetic expression.
 *
 * @param p: pointer to the parser state.
 * @param errors: counter variable to increment when an error is encountered.
 */
static expr_node *parse_factor(parser *p) {
  token token = {0};
  parser_current(p, &token);
  if (token.kind == TOKEN_INT || token.kind == TOKEN_CHAR ||
      token.kind == TOKEN_IDENTIFIER || token.kind == TOKEN_POINTER ||
      token.kind == TOKEN_STRING || token.kind == TOKEN_ADDRESS_OF) {
    expr_node *node = arena_push_struct(parser_arena, expr_node);
    node->kind = EXPR_TERM;
    node->line = token.line;

    if (token.kind == TOKEN_INT) {
      node->term.kind = TERM_INT;
      node->term.value.integer = token.value.integer;
      parser_advance(p);
      return node;
    } else if (token.kind == TOKEN_CHAR) {
      node->term.kind = TERM_CHAR;
      node->term.value.character = token.value.character;
      parser_advance(p);
      return node;
    } else if (token.kind == TOKEN_STRING) {
      node->term.kind = TERM_STRING;
      node->term.value.str = token.value.str;
      parser_advance(p);
      return node;
    } else if (token.kind == TOKEN_IDENTIFIER) {
      node->term.kind = TERM_IDENTIFIER;
      node->term.identifier.line = token.line;
      node->term.identifier.name = token.value.str;
      parser_advance(p);
      parser_current(p, &token);
      if (token.kind == TOKEN_LSQBR) {
        node->term.kind = TERM_ARRAY_ACCESS;
        node->term.array_access.array_var.name = node->term.identifier.name;
        node->term.array_access.array_var.line = node->term.identifier.line;
        parser_advance(p);
        node->term.array_access.index_expr = parse_expr(p);
        parser_current(p, &token);
        if (token.kind != TOKEN_RSQBR) {
          scu_perror("Expected ']' at line %d\n", token.line);
        }
        parser_advance(p);
      } else if (token.kind == TOKEN_LPAREN) {
        node->term.kind = TERM_FUNCTION_CALL;
        node->term.fn_call.name = node->term.identifier.name;

        dynamic_array_init(&node->term.fn_call.parameters, sizeof(expr_node));
        parser_advance(p);
        parser_current(p, &token);

        while (token.kind != TOKEN_RPAREN) {
          expr_node *arg = parse_expr(p);
          dynamic_array_append(&node->term.fn_call.parameters, arg);

          parser_current(p, &token);
          if (token.kind == TOKEN_COMMA) {
            parser_advance(p);
            parser_current(p, &token);
          }
        }

        if (token.kind != TOKEN_RPAREN) {
          scu_perror("Expected ')' at line %d\n", token.line);
        }
        parser_advance(p);
      }
      return node;
    } else if (token.kind == TOKEN_POINTER) {
      node->term.kind = TERM_DEREF;
      node->term.identifier.line = token.line;
      node->term.identifier.name = token.value.str;
      parser_advance(p);
      return node;
    } else if (token.kind == TOKEN_ADDRESS_OF) {
      node->term.kind = TERM_ADDOF;
      node->term.identifier.line = token.line;
      node->term.identifier.name = token.value.str;
      parser_advance(p);
      return node;
    }
  } else if (token.kind == TOKEN_LPAREN) {
    parser_advance(p);
    expr_node *node = parse_expr(p);
    parser_current(p, &token);
    if (token.kind != TOKEN_RPAREN) {
      scu_perror("Syntax error: expected ')'\n");
    }
    parser_advance(p);
    return node;
  } else {
    scu_perror("Syntax error: expected term or '('\n");
    scu_check_errors();
  }
  return NULL;
}

/*
 * @brief: parse a term.
 *
 * @param p: pointer to the parser state.
 * @param errors: counter variable to increment when an error is encountered.
 */
static expr_node *parse_term(parser *p) {
  expr_node *left = parse_factor(p);
  while (1) {
    token token = {0};
    parser_current(p, &token);

    if (token.kind == TOKEN_MULTIPLY || token.kind == TOKEN_DIVIDE ||
        token.kind == TOKEN_MODULO) {
      parser_advance(p);
      expr_node *right = parse_factor(p);

      expr_node *parent = arena_push_struct(parser_arena, expr_node);

      parent->line = token.line;

      if (token.kind == TOKEN_MULTIPLY) {
        parent->kind = EXPR_MULTIPLY;
      } else if (token.kind == TOKEN_DIVIDE) {
        parent->kind = EXPR_DIVIDE;
      } else {
        parent->kind = EXPR_MODULO;
      }
      parent->binary.left = left;
      parent->binary.right = right;
      left = parent;
    } else {
      break;
    }
  }
  return left;
}

/*
 * @brief: parse a arithmetic expression. (definition)
 *
 * @param p: pointer to the parser state.
 * @param errors: counter variable to increment when an error is encountered.
 */
static expr_node *parse_expr(parser *p) {
  expr_node *left = parse_term(p);
  while (1) {
    token token = {0};
    parser_current(p, &token);

    if (token.kind == TOKEN_ADD || token.kind == TOKEN_SUBTRACT) {
      parser_advance(p);
      expr_node *right = parse_term(p);

      expr_node *parent = arena_push_struct(parser_arena, expr_node);
      parent->kind = (token.kind == TOKEN_ADD) ? EXPR_ADD : EXPR_SUBTRACT;
      parent->line = token.line;
      parent->binary.left = left;
      parent->binary.right = right;
      left = parent;
    } else {
      break;
    }
  }
  return left;
}

/*
 * @brief: parse a relational expression.
 *
 * @param p: pointer to the parser state.
 * @param rel: pointer to a rel_node struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_rel(parser *p, rel_node *rel) {
  typedef struct {
    token_kind token_type;
    rel_kind rel_type;
  } rel_mapping;

  static const rel_mapping mappings[] = {
      {TOKEN_IS_EQUAL, REL_IS_EQUAL},
      {TOKEN_NOT_EQUAL, REL_NOT_EQUAL},
      {TOKEN_LESS_THAN, REL_LESS_THAN},
      {TOKEN_LESS_THAN_OR_EQUAL, REL_LESS_THAN_OR_EQUAL},
      {TOKEN_GREATER_THAN, REL_GREATER_THAN},
      {TOKEN_GREATER_THAN_OR_EQUAL, REL_GREATER_THAN_OR_EQUAL}};

  token token = {0};
  term_node lhs = {0}, rhs = {0};

  parse_term_for_expr(p, &lhs);
  parser_current(p, &token);
  rel->line = token.line;

  for (size_t i = 0; i < sizeof(mappings) / sizeof(mappings[0]); i++) {
    if (token.kind == mappings[i].token_type) {
      parser_advance(p);
      parse_term_for_expr(p, &rhs);

      rel->kind = mappings[i].rel_type;
      rel->comparison.lhs = lhs;
      rel->comparison.rhs = rhs;
      return;
    }
  }

  scu_perror("Expected a relation (==, !=, <, <=, >, >=), got %s [line %d]\n",
             lexer_token_kind_to_str(token.kind), token.line);
}

/*
 * @brief: parse an instruction. (declaration)
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_instr(parser *p, instr_node *instr, size_t *loop_counter);

/*
 * @brief: parse a variable initialize instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_initialize(parser *p, instr_node *instr, type _type,
                             char *_name) {
  instr->kind = INSTR_INITIALIZE;
  instr->initialize_variable.var.type = _type;
  instr->initialize_variable.var.name = _name;
  parser_advance(p);

  instr->initialize_variable.expr = parse_expr(p);
}

/*
 * @brief: parse an array initialize instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_initialize_array(parser *p, instr_node *instr, type _type,
                                   char *_name, expr_node *size_expr) {
  instr->kind = INSTR_INITIALIZE_ARRAY;
  instr->initialize_array.var.type = _type;
  instr->initialize_array.var.name = _name;
  instr->initialize_array.size_expr = size_expr;
  parser_advance(p);

  token token = {0};
  parser_current(p, &token);

  if (token.kind != TOKEN_LBRACE) {
    scu_perror("Expected '{' at line %d\n", token.line);
    return;
  }
  parser_advance(p);

  dynamic_array_init(&instr->initialize_array.literal.elements,
                     sizeof(expr_node));

  while (1) {
    parser_current(p, &token);
    if (token.kind == TOKEN_RBRACE) {
      break;
    }

    expr_node *elem = parse_expr(p);
    dynamic_array_append(&instr->initialize_array.literal.elements, elem);

    parser_current(p, &token);
    if (token.kind == TOKEN_COMMA) {
      parser_advance(p);
    } else if (token.kind == TOKEN_RBRACE) {
      break;
    } else {
      scu_perror("Expected '}' or ',' at line %d\n", token.line);
      return;
    }
  }

  parser_advance(p);
}

/*
 * @brief: parse a variable declare instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_declare(parser *p, instr_node *instr) {
  token token = {0};

  type _type = TYPE_VOID;
  char *_name;
  int _line;
  bool is_array = false;
  expr_node *size_expr = NULL;

  parser_current(p, &token);
  instr->line = token.line;
  if (token.kind == TOKEN_TYPE_INT) {
    _type = TYPE_INT;
  } else if (token.kind == TOKEN_TYPE_CHAR) {
    _type = TYPE_CHAR;
  }
  parser_advance(p);

  parser_current(p, &token);
  if (_type == TYPE_CHAR && token.kind == TOKEN_POINTER)
    _type = TYPE_STRING;
  _name = token.value.str;
  _line = token.line;
  parser_advance(p);

  parser_current(p, &token);
  if (token.kind == TOKEN_LSQBR) {
    is_array = true;
    parser_advance(p);

    size_expr = parse_expr(p);

    parser_current(p, &token);
    if (token.kind != TOKEN_RSQBR) {
      scu_perror("Expected ']' at line %d\n", token.line);
      return;
    }
    parser_advance(p);
  }

  parser_current(p, &token);

  if (token.kind == TOKEN_ASSIGN) {
    if (is_array) {
      parse_initialize_array(p, instr, _type, _name, size_expr);
    } else {
      parse_initialize(p, instr, _type, _name);
    }
  } else {
    if (is_array) {
      instr->kind = INSTR_DECLARE_ARRAY;
      instr->declare_array.var.type = _type;
      instr->declare_array.var.name = _name;
      instr->declare_array.var.line = _line;
      instr->declare_array.size_expr = size_expr;
    } else {
      instr->kind = INSTR_DECLARE;
      instr->declare_variable.type = _type;
      instr->declare_variable.name = _name;
      instr->declare_variable.line = _line;
    }
  }
}

/*
 * @brief: parse a function call.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_fn_call(parser *p, instr_node *instr) {
  token token = {0};
  parser_current(p, &token);

  instr->kind = INSTR_FN_CALL;
  instr->line = token.line;
  instr->fn_call.name = token.value.str;

  parser_advance(p);
  parser_current(p, &token);

  if (token.kind != TOKEN_LPAREN) {
    scu_perror("Expected '(' after function name [line %d]\n", token.line);
  }

  parser_advance(p);
  parser_current(p, &token);

  dynamic_array_init(&instr->fn_call.parameters, sizeof(expr_node));

  while (token.kind != TOKEN_RPAREN) {
    expr_node *arg = parse_expr(p);
    dynamic_array_append(&instr->fn_call.parameters, arg);

    parser_current(p, &token);
    if (token.kind == TOKEN_COMMA) {
      parser_advance(p);
      parser_current(p, &token);
    }
  }

  if (token.kind != TOKEN_RPAREN) {
    scu_perror("Expected ')' after function arguments [line %d]\n", token.line);
  }

  parser_advance(p);
}

/*
 * @brief: parse an assign instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_assign(parser *p, instr_node *instr) {
  token token = {0};

  parser_current(p, &token);

  size_t ident_line = instr->line = token.line;
  char *ident_name = token.value.str;

  if (token.kind == TOKEN_POINTER) {
    instr->assign.identifier.type = TYPE_POINTER;
  }

  parser_advance(p);
  parser_current(p, &token);

  if (token.kind == TOKEN_LSQBR) {
    instr->kind = INSTR_ASSIGN_TO_ARRAY_SUBSCRIPT;
    instr->line = token.line;
    instr->assign_to_array_subscript.var.name = ident_name;
    instr->assign_to_array_subscript.var.line = ident_line;

    parser_advance(p);

    expr_node *index_expr = parse_expr(p);
    instr->assign_to_array_subscript.index_expr = index_expr;

    parser_current(p, &token);
    if (token.kind != TOKEN_RSQBR) {
      scu_perror("Expected ], found %s [line %d]\n",
                 lexer_token_kind_to_str(token.kind), token.line);
    }
    parser_advance(p);
    parser_current(p, &token);

    if (token.kind != TOKEN_ASSIGN) {
      scu_perror("Expected assign, found %s [line %d]\n",
                 lexer_token_kind_to_str(token.kind), token.line);
    }
    parser_advance(p);

    instr->assign_to_array_subscript.expr_to_assign = parse_expr(p);
  } else if (token.kind == TOKEN_LPAREN) {
    p->index--;
    parse_fn_call(p, instr);
  } else {
    instr->kind = INSTR_ASSIGN;
    instr->line = ident_line;
    instr->assign.identifier.name = ident_name;

    if (token.kind != TOKEN_ASSIGN) {
      scu_perror("Expected assign, found %s [line %d]\n",
                 lexer_token_kind_to_str(token.kind), token.line);
    }
    parser_advance(p);

    instr->assign.expr = parse_expr(p);
  }
}

/*
 * @brief: parse an if instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_if(parser *p, instr_node *instr, size_t *loop_counter) {
  token token = {0};

  instr->kind = INSTR_IF;

  parser_advance(p);

  parse_rel(p, &instr->if_.rel);

  parser_current(p, &token);
  instr->line = token.line;

  if (token.kind == TOKEN_THEN) {
    instr->if_.kind = IF_SINGLE_INSTR;
    parser_advance(p);

    instr->if_.instr = arena_push_struct(parser_arena, instr_node);
    parse_instr(p, instr->if_.instr, loop_counter);
  } else if (token.kind == TOKEN_LBRACE) {
    instr->if_.kind = IF_MULTI_INSTR;
    parser_advance(p);

    dynamic_array_init(&instr->if_.instrs, sizeof(instr_node));

    parser_current(p, &token);
    while (token.kind != TOKEN_RBRACE && token.kind != TOKEN_END) {
      instr_node *new_instr = arena_push_struct(parser_arena, instr_node);
      parse_instr(p, new_instr, loop_counter);
      dynamic_array_append(&instr->if_.instrs, new_instr);

      parser_current(p, &token);
    }

    if (token.kind != TOKEN_RBRACE) {
      scu_perror("Expected '}', found %s [line %d]\n",
                 lexer_token_kind_to_str(token.kind), token.line);
      return;
    }
    parser_advance(p);
  } else {
    scu_perror("Expected 'then' or '{', found %s [line %d]\n",
               lexer_token_kind_to_str(token.kind), token.line);
  }
}

/*
 * @brief: parse a goto instruction.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_goto(parser *p, instr_node *instr) {
  token token = {0};

  instr->kind = INSTR_GOTO;

  parser_advance(p);

  parser_current(p, &token);
  instr->line = token.line;
  if (token.kind != TOKEN_LABEL) {
    scu_perror("Expected label, found %s [line %d]\n",
               lexer_token_kind_to_str(token.kind), token.line);
  }
  parser_advance(p);

  instr->goto_.label = token.value.str;
}

/*
 * @brief: parse a label.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_label(parser *p, instr_node *instr) {
  token token = {0};

  instr->kind = INSTR_LABEL;

  parser_current(p, &token);
  instr->line = token.line;
  instr->label.label = token.value.str;

  parser_advance(p);
}

/*
 * @brief: parse loops.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param kind: the kind of loop to parse (UNCONDITIONAL or WHILE).
 * @param loop_counter: counter for unique loop IDs.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_loop(parser *p, instr_node *instr, loop_kind kind,
                       size_t *loop_counter) {
  token token = {0};

  parser_current(p, &token);
  instr->kind = INSTR_LOOP;
  instr->line = token.line;
  instr->loop.kind = kind;
  instr->loop.loop_id = (*loop_counter)++;
  dynamic_array_init(&instr->loop.instrs, sizeof(instr_node));

  parser_advance(p);

  if (kind == LOOP_WHILE) {
    parser_current(p, &token);
    parse_rel(p, &instr->loop.break_condition);
  }

  parser_current(p, &token);
  if (token.kind != TOKEN_LBRACE) {
    scu_perror("no opening brace for %s loop at %d\n",
               kind == LOOP_WHILE ? "while" : "dowhile", token.line);
  }

  parser_advance(p);
  parser_current(p, &token);
  while (token.kind != TOKEN_RBRACE) {
    if (token.kind == TOKEN_COMMENT) {
      parser_advance(p);
      parser_current(p, &token);
      continue;
    }
    instr_node *_instr = arena_push_struct(parser_arena, instr_node);

    parse_instr(p, _instr, loop_counter);
    dynamic_array_append(&instr->loop.instrs, _instr);
    parser_current(p, &token);
  }

  parser_advance(p);

  if (kind == LOOP_DO_WHILE) {
    parser_current(p, &token);
    parse_rel(p, &instr->loop.break_condition);
  }
}

/*
 * @brief: parse functions.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param loop_counter: counter for unique loop IDs.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_fn(parser *p, instr_node *instr, size_t *loop_counter) {
  token token = {0};
  parser_current(p, &token);
  instr->kind = INSTR_FN_DECLARE;
  instr->line = token.line;
  instr->fn_declare_node.kind = FN_DECLARED;

  parser_advance(p);
  parser_current(p, &token);
  instr->fn_declare_node.name = token.value.str;
  parser_advance(p);

  parser_current(p, &token);
  if (token.kind != TOKEN_LPAREN) {
    scu_perror("Syntax error: expected '('\n");
    return;
  }
  parser_advance(p);

  dynamic_array_init(&instr->fn_declare_node.parameters, sizeof(variable));

  while (1) {
    parser_current(p, &token);
    if (token.kind == TOKEN_RPAREN) {
      break;
    }

    if (token.kind == TOKEN_ELLIPSIS) {
      instr->fn_declare_node.is_variadic = true;
      parser_advance(p);
      break;
    }

    variable param = {0};

    switch (token.kind) {
    case TOKEN_TYPE_INT:
      param.type = TYPE_INT;
      break;
    case TOKEN_TYPE_CHAR:
      param.type = TYPE_CHAR;
      break;
    default:
      scu_perror("Expected type, got %s line %d\n",
                 lexer_token_kind_to_str(token.kind), token.line);
      return;
    }
    parser_advance(p);

    parser_current(p, &token);
    if (token.kind == TOKEN_POINTER) {
      param.type = TYPE_POINTER;
      parser_advance(p);
    }

    parser_current(p, &token);
    param.name = token.value.str;
    dynamic_array_append(&instr->fn_declare_node.parameters, &param);
    parser_advance(p);

    parser_current(p, &token);
    if (token.kind == TOKEN_COMMA) {
      parser_advance(p);
    }
  }

  parser_advance(p);

  dynamic_array_init(&instr->fn_declare_node.returntypes, sizeof(type));
  parser_current(p, &token);
  if (token.kind == TOKEN_COLON) {
    parser_advance(p);
    parser_current(p, &token);
    while (token.kind != TOKEN_LBRACE && token.kind != TOKEN_END) {
      type ret_type = TYPE_VOID;
      switch (token.kind) {
      case TOKEN_TYPE_INT:
        ret_type = TYPE_INT;
        break;
      case TOKEN_TYPE_CHAR:
        ret_type = TYPE_CHAR;
        break;
      default:
        break;
      }
      dynamic_array_append(&instr->fn_declare_node.returntypes, &ret_type);
      parser_advance(p);
      parser_current(p, &token);
      if (token.kind == TOKEN_COMMA) {
        parser_advance(p);
      } else {
        break;
      }
    }
  }

  parser_current(p, &token);
  if (token.kind == TOKEN_LBRACE) {
    instr->kind = INSTR_FN_DEFINE;
    instr->fn_define_node.kind = FN_DEFINED;
    instr->fn_define_node.defined.variables = ht_new(sizeof(variable));
    dynamic_array_init(&instr->fn_define_node.defined.instrs,
                       sizeof(instr_node));

    parser_advance(p);
    while (1) {
      parser_current(p, &token);
      if (token.kind == TOKEN_RBRACE)
        break;
      instr_node *_instr = arena_push_struct(parser_arena, instr_node);
      parse_instr(p, _instr, loop_counter);
      dynamic_array_append(&instr->fn_define_node.defined.instrs, _instr);
    }
    parser_advance(p);
  }
}

/*
 * @brief: parse return statements.
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_ret(parser *p, instr_node *instr) {
  token token = {0};
  parser_current(p, &token);
  instr->kind = INSTR_RETURN;
  instr->line = token.line;
  dynamic_array_init(&instr->ret_node.returnvals, sizeof(expr_node));

  parser_advance(p);

  parser_current(p, &token);

  while (token.kind != TOKEN_RBRACE) {
    expr_node *expr = parse_expr(p);
    dynamic_array_append(&instr->ret_node.returnvals, expr);

    parser_current(p, &token);

    if (token.kind == TOKEN_COMMA) {
      parser_advance(p);
      parser_current(p, &token);
    } else {
      break;
    }
  }
}

/*
 * @brief: parse an instruction. (definition)
 *
 * @param p: pointer to the parser state.
 * @param instr: pointer to a newly malloc'd instr struct.
 * @param errors: counter variable to increment when an error is encountered.
 */
static void parse_instr(parser *p, instr_node *instr, size_t *loop_counter) {
  token token = {0};

  parser_current(p, &token);

  switch (token.kind) {
  case TOKEN_TYPE_INT:
  case TOKEN_TYPE_CHAR:
    parse_declare(p, instr);
    break;
  case TOKEN_IDENTIFIER:
  case TOKEN_POINTER:
    parse_assign(p, instr);
    break;
  case TOKEN_IF:
    parse_if(p, instr, loop_counter);
    break;
  case TOKEN_GOTO:
    parse_goto(p, instr);
    break;
  case TOKEN_LABEL:
    parse_label(p, instr);
    break;
  case TOKEN_LOOP:
    parse_loop(p, instr, LOOP_UNCONDITIONAL, loop_counter);
    break;
  case TOKEN_WHILE:
    parse_loop(p, instr, LOOP_WHILE, loop_counter);
    break;
  case TOKEN_DO_WHILE:
    parse_loop(p, instr, LOOP_DO_WHILE, loop_counter);
    break;
  case TOKEN_BREAK:
    instr->kind = INSTR_LOOP_BREAK;
    instr->line = token.line;
    parser_advance(p);
    break;
  case TOKEN_CONTINUE:
    instr->kind = INSTR_LOOP_CONTINUE;
    instr->line = token.line;
    parser_advance(p);
    break;
  case TOKEN_FN:
    parse_fn(p, instr, loop_counter);
    break;
  case TOKEN_RETURN:
    parse_ret(p, instr);
    break;
  case TOKEN_COMMENT:
    parser_advance(p);
    break;
  default:
    scu_perror("unexpected token: %s - '%s' [line %d]\n",
               lexer_token_kind_to_str(token.kind), token.value, token.line);
    scu_check_errors();
  }
}

void parser_parse_program(parser *p, ast *program) {
  parser_arena = program->arena;

  token token = {0};
  parser_current(p, &token);

  while (token.kind != TOKEN_END) {
    if (token.kind == TOKEN_COMMENT) {
      parser_advance(p);
      parser_current(p, &token);
      continue;
    }
    instr_node *instr = arena_push_struct(parser_arena, instr_node);
    parse_instr(p, instr, &program->loop_counter);
    scu_check_errors();
    dynamic_array_append(&program->instrs, instr);
    parser_current(p, &token);
  }

  parser_arena = NULL;
}
