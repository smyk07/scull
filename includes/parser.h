/*
 * parser: Parser for the simple-compiler
 */

#ifndef PARSER
#define PARSER

#include "ast.h"
#include "ds/dynamic_array.h"

#include <stddef.h>

/*
 * @brief: parses a dynamic_array of tokens into an AST.
 *
 * @param tokens: pointer to dynamic_array of tokens.
 * @param program: pointer to a program_node (empty dynamic_array of
 * instructions).
 */
void parser_parse_program(dynamic_array *tokens, ast *program);

#endif
