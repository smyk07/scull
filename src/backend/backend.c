#include "backend/backend.h"
#include "backend/llvm/llvm.h"

#include <stdlib.h>

void backend_init(backend *backend) {
  backend->init_function = llvm_backend_init;
  backend->compile_function = llvm_backend_compile;
  backend->emit_output_function = llvm_backend_emit;
  backend->cleanup_function = llvm_backend_cleanup;
  backend->link_function = llvm_backend_link;
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

void backend_free(backend *backend) {
  if (!backend)
    return;

  backend->init_function = NULL;
  backend->compile_function = NULL;
  backend->emit_output_function = NULL;
  backend->cleanup_function = NULL;
}
