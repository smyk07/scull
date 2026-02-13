#include "lexer.h"
#include "common.h"
#include "ds/dynamic_array.h"
#include "token.h"
#include "utils.h"

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * @struct string_slice: represents a slice of strings with a specified length,
 * does not need null termination.
 */
typedef struct string_slice {
  const char *str;
  u64 len;
} string_slice;

/*
 * @brief: Converts a string_slice to a null terminated owned string.
 *
 * @param ss: pointer to a string_slice.
 * @param str: pointer to a char pointer where the allocated string will be
 * stored.
 */
static u32 string_slice_to_owned(string_slice *ss, char **str) {
  if (!ss || !ss->str || !str)
    return -1;

  *str = (char *)scu_checked_malloc(ss->len + 1);
  if (!*str)
    return -1;

  memcpy(*str, ss->str, ss->len);
  (*str)[ss->len] = '\0';

  return 0;
}

/*
 * @brief: Read the next character.
 *
 * @param l: pointer to lexer struct object.
 */
static char lexer_read_char(lexer *l);

/*
 * Initialize Lexer
 *
 * @param l: pointer to lexer struct object.
 * @param buffer: const char* which is the source buffer to be lexed.
 * @param buffer_len: size of buffer.
 */
static void lexer_init(lexer *l, const char *buffer, u64 buffer_len) {
  l->buffer = buffer;
  l->buffer_len = buffer_len;
  l->line = 1;
  l->pos = 0;
  l->read_pos = 0;
  l->ch = 0;

  lexer_read_char(l);
}

/*
 * @brief: Peek ahead
 *
 * @param l: pointer to lexer struct object.
 */
static char lexer_peek_char(lexer *l) {
  if (l->read_pos >= l->buffer_len) {
    return EOF;
  }

  return l->buffer[l->read_pos];
}

/*
 * @brief: Read and return the character at read position.
 *
 * @param l: pointer to lexer struct object.
 */
static char lexer_read_char(lexer *l) {
  if (l->ch == '\n') {
    l->line++;
  }

  l->ch = lexer_peek_char(l);
  l->pos = l->read_pos;
  l->read_pos += 1;

  return l->ch;
}

/*
 * @brief: Move lexer forward until it encounters another character.
 *
 * @param l: pointer to lexer struct object.
 */
static void skip_whitespaces(lexer *l) {
  while (isspace(l->ch)) {
    lexer_read_char(l);
  }
}

/*
 * @brief: Scans the buffer ahead and returns the next token.
 *
 * @param l: pointer to lexer struct object.
 */
static token lexer_next_token(lexer *l) {

// label to restart lexing process again incase a comment is encountered
restart:

  skip_whitespaces(l);

  if (l->ch == EOF) {
    lexer_read_char(l);
    return (token){.kind = TOKEN_END, .value.str = NULL, .line = l->line};
  }

  else if (isdigit(l->ch)) {
    string_slice slice = {.str = l->buffer + l->pos, .len = 0};
    while (isdigit(l->ch)) {
      slice.len += 1;
      lexer_read_char(l);
    }
    char *temp = NULL;
    string_slice_to_owned(&slice, &temp);

    u32 value = atoi(temp);
    free(temp);

    return (token){
        .kind = TOKEN_INT_LITERAL, .value.integer = value, .line = l->line};
  }

  else if (l->ch == '\'') {
    lexer_read_char(l);
    char char_value = l->ch;
    char escaped_char;
    if (l->ch == '\\') {
      lexer_read_char(l);
      switch (l->ch) {
      case 'n':
        escaped_char = '\n';
        break;
      case 't':
        escaped_char = '\t';
        break;
      case 'r':
        escaped_char = '\r';
        break;
      case '\\':
        escaped_char = '\\';
        break;
      case '\'':
        escaped_char = '\'';
        break;
      case '0':
        escaped_char = '\0';
        break;
      default:
        return (token){
            .kind = TOKEN_INVALID, .value.character = l->ch, .line = l->line};
      }
      char_value = escaped_char;
    }
    lexer_read_char(l);
    if (l->ch != '\'') {
      return (token){
          .kind = TOKEN_INVALID, .value.character = l->ch, .line = l->line};
    }
    lexer_read_char(l);

    return (token){.kind = TOKEN_CHAR_LITERAL,
                   .value.character = char_value,
                   .line = l->line};
  }

  else if (l->ch == '"') {
    lexer_read_char(l);

    u64 capacity = 16;
    u64 length = 0;
    char *string_value = scu_checked_malloc(capacity);

    if (l->ch == '"') {
      lexer_read_char(l);
      string_value[0] = '\0';
      return (token){.kind = TOKEN_STRING_LITERAL,
                     .value.str = string_value,
                     .line = l->line};
    }

    while (l->ch != '"' && l->ch != '\0' && l->ch != EOF) {
      if (length >= capacity - 1) {
        capacity *= 2;
        string_value = scu_checked_realloc(string_value, capacity);
      }

      if (l->ch == '\\') {
        lexer_read_char(l);
        char escaped_char;
        switch (l->ch) {
        case 'n':
          escaped_char = '\n';
          break;
        case 't':
          escaped_char = '\t';
          break;
        case 'r':
          escaped_char = '\r';
          break;
        case '\\':
          escaped_char = '\\';
          break;
        case '"':
          escaped_char = '"';
          break;
        case '0':
          escaped_char = '\0';
          break;
        default:
          free(string_value);
          return (token){
              .kind = TOKEN_INVALID, .value.character = l->ch, .line = l->line};
        }
        string_value[length++] = escaped_char;
      } else if (l->ch == '\n') {
        string_value[length++] = '\n';
        l->line++;
      } else {
        string_value[length++] = l->ch;
      }
      lexer_read_char(l);
    }

    if (l->ch != '"') {
      free(string_value);
      return (token){.kind = TOKEN_INVALID, .value.str = NULL, .line = l->line};
    }

    lexer_read_char(l);
    string_value[length] = '\0';

    return (token){.kind = TOKEN_STRING_LITERAL,
                   .value.str = string_value,
                   .line = l->line};
  }

#define LEX_ONE_CHAR_TOKEN(chr, tok_kind)                                      \
  else if (l->ch == chr) {                                                     \
    lexer_read_char(l);                                                        \
    return (token){.kind = tok_kind, .value.str = NULL, .line = l->line};      \
  }

#define LEX_TWO_CHAR_TOKEN(ch1, ch2, tok_kind, fallback_kind)                  \
  else if (l->ch == ch1) {                                                     \
    lexer_read_char(l);                                                        \
    if (l->ch == ch2) {                                                        \
      lexer_read_char(l);                                                      \
      return (token){.kind = tok_kind, .value.str = NULL, .line = l->line};    \
    }                                                                          \
    return (token){.kind = fallback_kind, .value.str = NULL, .line = l->line}; \
  }

  // Delimiters
  LEX_ONE_CHAR_TOKEN('(', TOKEN_LPAREN)
  LEX_ONE_CHAR_TOKEN(')', TOKEN_RPAREN)
  LEX_ONE_CHAR_TOKEN('{', TOKEN_LBRACE)
  LEX_ONE_CHAR_TOKEN('}', TOKEN_RBRACE)
  LEX_ONE_CHAR_TOKEN('[', TOKEN_LSQBR)
  LEX_ONE_CHAR_TOKEN(']', TOKEN_RSQBR)
  LEX_ONE_CHAR_TOKEN(',', TOKEN_COMMA)
  LEX_ONE_CHAR_TOKEN('_', TOKEN_UNDERSCORE)

  // Simple arithmetic operators
  LEX_ONE_CHAR_TOKEN('+', TOKEN_ADD)
  LEX_ONE_CHAR_TOKEN('/', TOKEN_DIVIDE)
  LEX_ONE_CHAR_TOKEN('%', TOKEN_MODULO)

  // Relational Operators
  LEX_TWO_CHAR_TOKEN('!', '=', TOKEN_NOT_EQUAL, TOKEN_INVALID)
  LEX_TWO_CHAR_TOKEN('<', '=', TOKEN_LESS_THAN_OR_EQUAL, TOKEN_LESS_THAN)
  LEX_TWO_CHAR_TOKEN('>', '=', TOKEN_GREATER_THAN_OR_EQUAL, TOKEN_GREATER_THAN)

#undef LEX_ONE_CHAR_TOKEN
#undef LEX_TWO_CHAR_TOKEN

  else if (l->ch == '=') {
    lexer_read_char(l);
    if (l->ch == '=') {
      lexer_read_char(l);
      return (token){
          .kind = TOKEN_IS_EQUAL, .value.str = NULL, .line = l->line};
    } else if (l->ch == '>') {
      lexer_read_char(l);
      return (token){.kind = TOKEN_DARROW, .value.str = NULL, .line = l->line};
    }
    return (token){.kind = TOKEN_ASSIGN, .value.str = NULL, .line = l->line};
  }

  else if (l->ch == '-') {
    lexer_read_char(l);
    if (l->ch == '-') {
      while (l->ch != '\n') {
        lexer_read_char(l);
      }
      goto restart;
    } else if (l->ch == '*') {
      while (l->ch != '\0') {
        if (l->ch == '*') {
          lexer_read_char(l);
          if (l->ch == '-') {
            lexer_read_char(l);
            goto restart;
          }
          continue;
        } else {
          lexer_read_char(l);
          continue;
        }
        return (token){
            .kind = TOKEN_INVALID, .value.character = l->ch, .line = l->line};
      }
    } else if (isdigit(l->ch)) {
      string_slice slice = {.str = l->buffer + l->pos, .len = 0};
      while (isdigit(l->ch)) {
        slice.len += 1;
        lexer_read_char(l);
      }
      char *temp = NULL;
      string_slice_to_owned(&slice, &temp);
      u32 value = -atoi(temp);
      free(temp);
      return (token){
          .kind = TOKEN_INT_LITERAL, .value.integer = value, .line = l->line};
    } else if (isalnum(l->ch)) {
      string_slice slice = {.str = l->buffer + l->pos, .len = 0};
      while (isalnum(l->ch) || l->ch == '_') {
        slice.len += 1;
        lexer_read_char(l);
      }
      char *directive = NULL;
      string_slice_to_owned(&slice, &directive);
      if (strcmp(directive, "include") == 0) {
        free(directive);
        return (token){
            .kind = TOKEN_PDIR_INCLUDE, .value.str = NULL, .line = l->line};
      }
    }
    return (token){.kind = TOKEN_SUBTRACT, .value.str = NULL, .line = l->line};
  }

  else if (l->ch == '*') {
    lexer_read_char(l);
    if (isalnum(l->ch) || l->ch == '_') {
      string_slice slice = {.str = l->buffer + l->pos, .len = 0};
      while (isalnum(l->ch) || l->ch == '_') {
        slice.len += 1;
        lexer_read_char(l);
      }
      char *value = NULL;
      string_slice_to_owned(&slice, &value);
      return (token){
          .kind = TOKEN_POINTER, .value.str = value, .line = l->line};
    }
    return (token){.kind = TOKEN_MULTIPLY, .value.str = NULL, .line = l->line};
  }

  else if (l->ch == '&') {
    lexer_read_char(l);
    string_slice slice = {.str = l->buffer + l->pos, .len = 0};
    while (isalnum(l->ch) || l->ch == '_') {
      slice.len += 1;
      lexer_read_char(l);
    }
    char *value = NULL;
    string_slice_to_owned(&slice, &value);
    return (token){
        .kind = TOKEN_ADDRESS_OF, .value.str = value, .line = l->line};
  }

  else if (l->ch == ':') {
    lexer_read_char(l);

    if (isalnum(l->ch) || l->ch == '_') {
      string_slice slice = {.str = l->buffer + l->pos, .len = 0};
      while (isalnum(l->ch) || l->ch == '_') {
        slice.len += 1;
        lexer_read_char(l);
      }
      char *value = NULL;
      string_slice_to_owned(&slice, &value);
      return (token){.kind = TOKEN_LABEL, .value.str = value, .line = l->line};
    } else {
      return (token){.kind = TOKEN_COLON, .value.str = NULL, .line = l->line};
    }
  }

  else if (l->ch == '.') {
    lexer_read_char(l);

    if (l->ch == '.') {
      lexer_read_char(l);

      if (l->ch == '.') {
        lexer_read_char(l);
        return (token){
            .kind = TOKEN_ELLIPSIS, .value.str = NULL, .line = l->line};
      }
    }

    return (token){.kind = TOKEN_INVALID, .value.str = NULL, .line = l->line};
  }

  else if (isalnum(l->ch) || l->ch == '_') {
    string_slice slice = {.str = l->buffer + l->pos, .len = 0};

    while (isalnum(l->ch) || l->ch == '_') {
      slice.len += 1;
      lexer_read_char(l);
    }

    char *value = NULL;
    string_slice_to_owned(&slice, &value);

#define LEX_KEYWORD(keyword_str, tok_kind)                                     \
  if (strcmp(value, keyword_str) == 0) {                                       \
    free(value);                                                               \
    return (token){.kind = tok_kind, .value.str = NULL, .line = l->line};      \
  }

    // Types
    LEX_KEYWORD("int", TOKEN_TYPE_INT)
    LEX_KEYWORD("char", TOKEN_TYPE_CHAR)

    // Control flow
    LEX_KEYWORD("if", TOKEN_IF)
    LEX_KEYWORD("else", TOKEN_ELSE)
    LEX_KEYWORD("then", TOKEN_THEN)
    LEX_KEYWORD("match", TOKEN_MATCH)
    LEX_KEYWORD("goto", TOKEN_GOTO)

    // Loops
    LEX_KEYWORD("loop", TOKEN_LOOP)
    LEX_KEYWORD("while", TOKEN_WHILE)
    LEX_KEYWORD("dowhile", TOKEN_DO_WHILE)
    LEX_KEYWORD("in", TOKEN_IN)
    LEX_KEYWORD("for", TOKEN_FOR)
    LEX_KEYWORD("continue", TOKEN_CONTINUE)
    LEX_KEYWORD("break", TOKEN_BREAK)

    // Functions
    LEX_KEYWORD("fn", TOKEN_FN)
    LEX_KEYWORD("return", TOKEN_RETURN)

#undef LEX_KEYWORD

    return (token){
        .kind = TOKEN_IDENTIFIER, .value.str = value, .line = l->line};
  }

  else {
    string_slice slice = {.str = l->buffer + l->pos, .len = 1};
    char *value = NULL;
    string_slice_to_owned(&slice, &value);
    lexer_read_char(l);
    return (token){.kind = TOKEN_INVALID, .value.str = value, .line = l->line};
  }
}

void lexer_tokenize(const char *buffer, u64 buffer_len, dynamic_array *tokens,
                    const char *include_dir) {
  lexer lexer;
  lexer_init(&lexer, buffer, buffer_len);

  token tok;
  do {
    tok = lexer_next_token(&lexer);

    if (tok.kind == TOKEN_PDIR_INCLUDE) {
      token incl_str_token = lexer_next_token(&lexer);
      u64 total_len =
          strlen(include_dir) + 1 + strlen(incl_str_token.value.str) + 1;
      char *filepath_to_include = malloc(total_len);
      snprintf(filepath_to_include, total_len, "%s/%s", include_dir,
               incl_str_token.value.str);
      char *incl_buffer = NULL;
      u64 incl_buffer_len = scu_read_file(filepath_to_include, &incl_buffer);

      lexer_tokenize(incl_buffer, incl_buffer_len, tokens, include_dir);

      dynamic_array_remove(tokens, tokens->count - 1);
      free(filepath_to_include);
      free(incl_str_token.value.str);
      free(incl_buffer);

      continue;
    }

    if (dynamic_array_append(tokens, &tok) != 0) {
      scu_perror("Failed to append token to array\n");
      exit(1);
    }
  } while (tok.kind != TOKEN_END);
}
