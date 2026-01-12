/*
 * token: Definitions of lexical token.
 */

#ifndef TOKEN
#define TOKEN

#include "ds/dynamic_array.h"

#include <stddef.h>

/*
 * @enum token_kind: enumeration of all kinds of tokens supported by the lexer.
 */
typedef enum token_kind {
  /*
   * Keywords
   */
  TOKEN_GOTO = 0,
  TOKEN_IF,
  TOKEN_ELSE,
  TOKEN_THEN,
  TOKEN_TYPE_INT,
  TOKEN_TYPE_CHAR,
  TOKEN_LOOP,
  TOKEN_WHILE,
  TOKEN_DO_WHILE,
  TOKEN_CONTINUE,
  TOKEN_BREAK,
  TOKEN_FN,
  TOKEN_RETURN,

  /*
   * Preprocessor Directives
   */
  TOKEN_PDIR_INCLUDE,

  /*
   * Literals
   */
  TOKEN_IDENTIFIER,
  TOKEN_LABEL,
  TOKEN_INT,
  TOKEN_CHAR,
  TOKEN_STRING,
  TOKEN_POINTER,
  TOKEN_ELLIPSIS, //'...'

  /*
   * Brackets
   */
  TOKEN_LPAREN,
  TOKEN_RPAREN,
  TOKEN_LBRACE,
  TOKEN_RBRACE,
  TOKEN_LSQBR,
  TOKEN_RSQBR,

  /*
   * Delimiters
   */
  TOKEN_COMMA,
  TOKEN_COLON,

  /*
   * Arithmetic Operators
   */
  TOKEN_ASSIGN,
  TOKEN_ADD,
  TOKEN_SUBTRACT,
  TOKEN_MULTIPLY,
  TOKEN_DIVIDE,
  TOKEN_MODULO,
  TOKEN_ADDRESS_OF,

  /*
   * Conditional Operators
   */
  TOKEN_IS_EQUAL,
  TOKEN_NOT_EQUAL,
  TOKEN_LESS_THAN,
  TOKEN_LESS_THAN_OR_EQUAL,
  TOKEN_GREATER_THAN,
  TOKEN_GREATER_THAN_OR_EQUAL,

  /*
   * Special Tokens
   */
  TOKEN_INVALID,
  TOKEN_COMMENT,
  TOKEN_END
} token_kind;

/*
 * @union token_literal_value: holds the "value" of any particular literal
 * token. Values can be integers, characters, or strings for labels.
 */
typedef union token_literal_value {
  int integer;
  char character;
  char *str;
} token_literal_value;

/*
 * @struct token: reprents a token and its metadata.
 */
typedef struct token {
  token_kind kind;
  token_literal_value value;
  size_t line; // <-- Where the token is placed in the source buffer.
} token;

/*
 * @brief: Converts a token_kind enum value to its string representation.
 *
 * @param kind: token_kind enum value.
 * @return: string representation of the token.
 */
const char *lexer_token_kind_to_str(token_kind kind);

/*
 * @brief: Print the whole token stream, required for debugging.
 *
 * @param tokens: dynamic_array of tokens.
 */
void lexer_print_tokens(dynamic_array *tokens);

/*
 * @brief: Free / Destroy tokens before termination. This function is needed due
 * to there being malloc'd strings in token->value.
 *
 * @param tokens: dynamic_array of tokens.
 */
void free_tokens(dynamic_array *tokens);

#endif // !TOKEN
