/*
 * sclc: a simple compiler, don't have a specific goal for it yet, just to
 * practice my programming compiler design and development skills.
 *
 * Copyright (C) 2025 Samyak Bambole <bambole@duck.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "codegen.h"
#include "cstate.h"
#include "ds/dynamic_array.h"
#include "fasm_utils.h"
#include "fstate.h"
#include "ld_utils.h"
#include "lexer.h"
#include "semantic.h"
#include "utils.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

int main(int argc, char *argv[]) {
  // Initialize compiler state
  cstate *state = cstate_create_from_args(argc, argv);

  clock_t start, end;
  double time_taken;
  start = clock();

  for (size_t i = 0; i < state->files.count; i++) {
    fstate *fst;
    dynamic_array_get(&state->files, i, &fst);

    // Lexing
    lexer_tokenize(fst->code_buffer, fst->code_buffer_len, fst->tokens,
                   state->include_dir, &fst->error_count);

    // Lexing debug statements
    if (state->options.verbose) {
      scu_pdebug("Lexing Debug Statements for %s:\n", fst->filepath);
      lexer_print_tokens(fst->tokens);
    }

    // Parsing
    parser_init(fst->tokens, fst->parser);
    parser_parse_program(fst->parser, fst->program, &fst->error_count);

    // Parsing debug statements
    if (state->options.verbose) {
      scu_pdebug("Parsing Debug Statements for %s:\n", fst->filepath);
      parser_print_program(fst->program);
    }

    // Semantic Analysis
    check_semantics(&fst->program->instrs, fst->variables, fst->functions,
                    &fst->error_count);

    // Semantic Debug Statement
    if (state->options.verbose)
      scu_pdebug("Semantic Analysis Complete for %s\n", fst->filepath);

    // Codegen & Assembler
    instrs_to_asm(fst->program, fst->variables, fst->loops, fst->functions,
                  fst->extracted_filepath, &fst->error_count);

    // Codegen & Assembler Debug Statements
    if (state->options.verbose)
      scu_pdebug("Codegen & Assembling Complete for %s\n", fst->filepath);

    fasm_assemble(fst->extracted_filepath,
                  scu_format_string("%s.s", fst->extracted_filepath));

    if (state->options.verbose)
      scu_psuccess("COMPILED %s\n", fst->filepath);
  }

  size_t total_len = 0;
  for (size_t i = 0; i < state->files.count; i++) {
    fstate *fst;
    dynamic_array_get(&state->files, i, &fst);
    total_len += strlen(fst->extracted_filepath) + 3;
  }

  char *obj_file_list = scu_checked_malloc(total_len + 1);
  obj_file_list[0] = '\0';

  for (size_t i = 0; i < state->files.count; i++) {
    fstate *fst;
    dynamic_array_get(&state->files, i, &fst);
    if (i > 0)
      strcat(obj_file_list, " ");
    strcat(obj_file_list, fst->extracted_filepath);
    strcat(obj_file_list, ".o");
  }

  ld_link(state->output_filepath, obj_file_list);

  free(obj_file_list);

  end = clock();
  time_taken = (double)(end - start) / CLOCKS_PER_SEC;

  if (state->options.verbose)
    scu_psuccess("  LINKED %s - %.2fs total time taken\n",
                 state->output_filepath, time_taken);

  cstate_free(state);

  return 0;
}
