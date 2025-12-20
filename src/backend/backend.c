#include "backend/backend.h"
#include "backend/llvm/llvm.h"
#include "utils.h"

#include <stdlib.h>

struct backend {
  /*
   * arguments to these function pointers will be resolved and made explicit
   * later on, just trying to setup a basic boilerplate right now.
   */
  void (*init_function)(cstate *cst, fstate *fst);
  void (*compile_function)(cstate *cst, fstate *fst);
  void (*emit_output_function)(cstate *cst, fstate *fst);
  void (*cleanup_function)(cstate *cst, fstate *fst);
};

backend *backend_create() {
  backend *b = scu_checked_malloc(sizeof(backend));

  b->init_function = llvm_backend_init;
  b->compile_function = llvm_backend_compile;
  b->emit_output_function = llvm_backend_emit;
  b->cleanup_function = llvm_backend_cleanup;

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

void backend_destroy(backend *backend) {
  if (!backend)
    return;

  backend->init_function = NULL;
  backend->compile_function = NULL;
  backend->emit_output_function = NULL;
  backend->cleanup_function = NULL;

  free(backend);
}
