#include "fstate.h"
#include "ast.h"
#include "ds/dynamic_array.h"
#include "ds/ht.h"
#include "lexer.h"
#include "utils.h"
#include "var.h"

#include <stdlib.h>
#include <string.h>

void fstate_init(fstate *fst, const char *filepath) {
  fst->filepath_len = strlen(filepath) + 1;
  fst->filepath = scu_checked_malloc(fst->filepath_len * sizeof(char));
  strcpy(fst->filepath, filepath);

  fst->extracted_filepath = scu_extract_name(fst->filepath);

  fst->code_buffer_len = scu_read_file(fst->filepath, &fst->code_buffer);
  if (fst->code_buffer == NULL) {
    scu_perror("Failed to read file: %s\n", fst->filepath);
    exit(1);
  }

  dynamic_array_init(&fst->tokens, sizeof(token));

  ast_init(&fst->program_ast);

  stack_init(&fst->loops, sizeof(loop_node));

  ht_init(&fst->variables, sizeof(variable));

  ht_init(&fst->functions, sizeof(fn_node));
}

void fstate_free(fstate *fst) {
  free(fst->filepath);
  free(fst->extracted_filepath);
  free(fst->code_buffer);

  free_tokens(&fst->tokens);
  dynamic_array_free(&fst->tokens);

  free_ast(&fst->program_ast);

  stack_free(&fst->loops);

  ht_free(&fst->variables);

  ht_free(&fst->functions);
}
