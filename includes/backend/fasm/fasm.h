#ifndef FASM_H

#include "cstate.h"
#include "fstate.h"

void fasm_codegen_init(cstate *cst, fstate *fst);

void fasm_codegen_compile(cstate *cst, fstate *fst);

void fasm_codegen_assmeble(cstate *cst, fstate *fst);

#endif // !FASM_H
