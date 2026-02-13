/*
 * llvm: LLVM Backend for code generation
 */

#ifndef LLVM_H
#define LLVM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "cstate.h"
#include "fstate.h"

/*
 * @brief: Initializes the LLVM backend for compilation.
 *
 * @param cst: Pointer to compiler state containing global compilation context
 */
void llvm_backend_init(cstate *cst);

/*
 * @brief: Compiles the parsed program to LLVM IR.
 *
 * @param cst: Pointer to compiler state containing global compilation context
 * @param fst: Pointer to file state containing the parsed program and artifacts
 */
void llvm_backend_compile(cstate *cst, fstate *fst);

/*
 * @brief: LLVM Optimization passes.
 *
 * @param cst: Pointer to compiler state containing global compilation context
 * @param fst: Pointer to file state containing the compiled IR
 */
void llvm_backend_optimize(cstate *cst, fstate *fst);

/*
 * @brief: Emits the compiled LLVM IR to the requested format (default is a
 * linkable output file).
 *
 * @param cst: Pointer to compiler state containing global compilation context
 * @param fst: Pointer to file state containing the compiled IR
 */
void llvm_backend_emit(cstate *cst, fstate *fst);

/*
 * @brief: Cleans up LLVM backend resources and frees associated memory.
 *
 * @param cst: Pointer to compiler state containing global compilation context
 * @param fst: Pointer to file state containing LLVM backend artifacts
 */
void llvm_backend_cleanup(cstate *cst, fstate *fst);

/*
 * @brief: Links the emitted object files
 *
 * @param cst: Pointer to compiler state containing global compilation context
 */
void llvm_backend_link(cstate *cst);

#ifdef __cplusplus
}
#endif

#endif // !LLVM_H
