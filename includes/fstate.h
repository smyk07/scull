/*
 * fstate: per-file data
 */

#ifndef FSTATE_H
#define FSTATE_H

#include "ds/dynamic_array.h"
#include "ds/stack.h"
#include "parser.h"

#include <stddef.h>

typedef struct fstate {
  /*
   * Stores the path to the file to be compiled.
   * Ex: main.scl, lib/io.scl
   */
  const char *filepath;

  /*
   * File path without the extention.
   * Ex: "lib/io.scl" -> "lib/io"
   */
  char *extracted_filepath;

  /*
   * Source buffer and its size in bytes.
   */
  char *code_buffer;
  size_t code_buffer_len;

  /*
   * Variables / artifacts for the whole compiler pipeline.
   */
  dynamic_array *tokens;
  parser *parser;
  ast *program_ast;
  ht *variables;
  stack *loops;
  ht *functions;
} fstate;

/*
 * @brief: Creates and initializes a new file state for the given source file.
 *
 * @param filepath: Path to the source file to be compiled (e.g., "main.scl")
 *
 * @return: Pointer to newly allocated and initialized fstate, or NULL on error
 */
fstate *create_new_fstate(const char *filepath);

/*
 * @brief: Frees all memory associated with a file state.
 *
 * @param fst: Pointer to the fstate to be freed
 */
void free_fstate(fstate *fst);

#endif // !FSTATE_H
