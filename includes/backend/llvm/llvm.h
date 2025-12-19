#ifndef LLVM_H
#define LLVM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cstate.h"
#include "fstate.h"

void llvm_backend_init(cstate *cst, fstate *fst);

void llvm_backend_compile(cstate *cst, fstate *fst);

void llvm_backend_emit(cstate *cst, fstate *fst);

void llvm_backend_cleanup(cstate *cst, fstate *fst);

#ifdef __cplusplus
}
#endif

#endif // !LLVM_H
