#ifndef FASM_H

#include "cstate.h"
#include "fstate.h"

void fasm_backend_init(cstate *cst, fstate *fst);

void fasm_backend_compile(cstate *cst, fstate *fst);

void fasm_backend_emit(cstate *cst, fstate *fst);

#endif // !FASM_H
