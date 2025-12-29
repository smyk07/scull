/*
 * parser: Parser for the simple-compiler
 */

#ifndef PARSER
#define PARSER

#include "ast.h"
#include "ds/dynamic_array.h"

#include <stddef.h>

/*
 * @struct parser: represents the parser state.
 */
typedef struct parser {
  dynamic_array tokens;
  size_t index;
} parser;

/*
 * @brief: Initializes the parser struct.
 *
 * @param tokens: pointer to dynamic_array of tokens.
 * @param p: pointer to an uninitialized parser struct.
 */
void parser_init(dynamic_array *tokens, parser *p);

/*
 * @brief: parses a dynamic_array of tokens into an AST.
 *
 * @param p: pointer to an uninitialized parser struct.
 * @param program: pointer to a program_node (empty dynamic_array of
 * instructions).
 */
void parser_parse_program(parser *p, ast *program);

#endif
