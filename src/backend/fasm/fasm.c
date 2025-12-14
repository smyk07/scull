#include "backend/fasm/fasm.h"
#include "backend/fasm/fasm_codegen.h"
#include "backend/fasm/fasm_utils.h"
#include "utils.h"

#include <stdlib.h>

void fasm_codegen_init(cstate *, fstate *fst) {
  char *output_asm_file = scu_format_string("%s.s", fst->extracted_filepath);
  init_fasm_output(output_asm_file);
  free(output_asm_file);
}

void fasm_codegen_compile(cstate *, fstate *fst) {
  instrs_to_fasm(fst->program, fst->variables, fst->loops, fst->functions,
                 &fst->error_count);
}

void fasm_codegen_assmeble(cstate *, fstate *fst) {
  char *asm_file = scu_format_string("%s.s", fst->extracted_filepath);
  fasm_assemble(fst->extracted_filepath, asm_file);
  free(asm_file);
}
