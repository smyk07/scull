/*
 * fstate: per-file data
 */

#ifndef FSTATE_H
#define FSTATE_H

#include "ast.h"
#include "common.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"
#include "ds/stack.h"

typedef struct fstate {
  /*
   * Stores the path to the file to be compiled.
   * Ex: main.scl, lib/io.scl
   */
  char *filepath;
  u64 filepath_len;

  /*
   * File path without the extention.
   * Ex: "lib/io.scl" -> "lib/io"
   */
  char *extracted_filepath;

  /*
   * Source buffer and its size in bytes.
   */
  char *code_buffer;
  u64 code_buffer_len;

  /*
   * Variables / artifacts for the whole compiler pipeline.
   */
  dynamic_array tokens;
  ast program_ast;
  ht variables;
  stack loops;
  ht functions;
} fstate;

/*
 * @brief: initializes a existing file state for the given source file.
 *
 * @param fst: Pointer to a fstate
 * @param filepath: Path to the source file to be compiled (e.g., "main.scl")
 */
void fstate_init(fstate *fst, const char *filepath);

/*
 * @brief: Frees all memory associated with a file state.
 *
 * @param fst: Pointer to the fstate to be freed
 */
void fstate_free(fstate *fst);

#endif // !FSTATE_H
