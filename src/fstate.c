#include "fstate.h"
#include "arena.h"
#include "ast.h"
#include "ds/dynamic_array.h"
#include "lexer.h"
#include "parser.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

fstate *create_new_fstate(const char *filepath) {
  fstate *fst = scu_checked_malloc(sizeof(fstate));

  fst->filepath = filepath;
  fst->extracted_filepath = scu_extract_name(fst->filepath);

  fst->code_buffer_len =
      scu_read_file(fst->filepath, &fst->code_buffer, &fst->error_count);
  if (fst->code_buffer == NULL) {
    scu_perror(&fst->error_count, "Failed to read file: %s\n", fst->filepath);
    exit(1);
  }

  fst->tokens = scu_checked_malloc(sizeof(dynamic_array));
  dynamic_array_init(fst->tokens, sizeof(token));

  fst->parser = scu_checked_malloc(sizeof(parser));

  fst->program = scu_checked_malloc(sizeof(program_node));
  fst->program->arena = arena_create(4 << 20); // allocate 4 megabytes for now
  dynamic_array_init(&fst->program->instrs, sizeof(instr_node));
  fst->program->loop_counter = 0;

  fst->loops = scu_checked_malloc(sizeof(stack));
  stack_init(fst->loops, sizeof(loop_node));

  fst->variables = ht_new(sizeof(variable));

  fst->functions = ht_new(sizeof(fn_node));

  return fst;
}

void free_fstate(fstate *fst) {
  free(fst->extracted_filepath);
  free(fst->code_buffer);

  free_tokens(fst->tokens);
  dynamic_array_free(fst->tokens);
  free(fst->tokens);

  free(fst->parser);

  free_if_instrs(fst->program);
  free_loops(fst->program);
  free_fns(fst->program);
  dynamic_array_free(&fst->program->instrs);
  arena_destroy(fst->program->arena);
  free(fst->program);

  stack_free(fst->loops);
  free(fst->loops);

  ht_del_ht(fst->variables);

  ht_del_ht(fst->functions);

  free(fst);
}
