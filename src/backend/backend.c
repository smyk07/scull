#include "backend/backend.h"
#include "backend/fasm/fasm.h"
#include "utils.h"

#include <stdlib.h>

const char *target_kind_to_string(target_kind target) {
  switch (target) {
  case TARGET_FASM:
    return TARGET_FASM_STR;
  case TARGET_C:
    return TARGET_C_STR;
  case TARGET_LLVM_X86_64:
    return TARGET_LLVM_X86_64_STR;
  case TARGET_LLVM_ARM64:
    return TARGET_LLVM_ARM64_STR;
  case TARGET_LLVM_RISCV64:
    return TARGET_LLVM_RISCV64_STR;
  default:
    return "unknown";
  }
}

struct backend {
  target_kind target;

  /*
   * arguments to these function pointers will be resolved and made explicit
   * later on, just trying to setup a basic boilerplate right now.
   */
  void (*init_function)(cstate *cst, fstate *fst);
  void (*compile_function)(cstate *cst, fstate *fst);
  void (*emit_output_function)(cstate *cst, fstate *fst);
  void (*cleanup_function)(cstate *cst, fstate *fst);
};

backend *backend_create(target_kind target) {
  backend *b = scu_checked_malloc(sizeof(backend));

  b->target = target;

  switch (target) {
  case TARGET_FASM:
    b->init_function = fasm_codegen_init;
    b->compile_function = fasm_codegen_compile;
    b->emit_output_function = fasm_codegen_assmeble;
    b->cleanup_function = NULL;
    break;
  case TARGET_C:
  case TARGET_LLVM_X86_64:
  case TARGET_LLVM_ARM64:
  case TARGET_LLVM_RISCV64:
    break;
  }

  return b;
}

void backend_compile(backend *backend, cstate *cst, fstate *fst) {
  if (backend->init_function)
    backend->init_function(cst, fst);

  if (backend->compile_function)
    backend->compile_function(cst, fst);

  if (backend->emit_output_function)
    backend->emit_output_function(cst, fst);

  if (backend->cleanup_function)
    backend->cleanup_function(cst, fst);
}

void backend_destroy(backend *backend) { free(backend); }
