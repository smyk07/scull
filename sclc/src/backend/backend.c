#include "backend/backend.h"
#include "backend/llvm/llvm.h"

#include <stdlib.h>

void backend_init(backend *backend, cstate *cst) {
  backend->setup = llvm_backend_init;

  backend->compile = llvm_backend_compile;
  backend->emit = llvm_backend_emit;
  backend->optimize = llvm_backend_optimize;
  backend->cleanup = llvm_backend_cleanup;

  backend->link = llvm_backend_link;

  // one-time backend setup
  backend->setup(cst);
}

void backend_compile(backend *backend, cstate *cst, fstate *fst) {
  if (backend->compile)
    backend->compile(cst, fst);

  if (backend->optimize)
    backend->optimize(cst, fst);

  if (backend->emit)
    backend->emit(cst, fst);

  if (backend->cleanup)
    backend->cleanup(cst, fst);
}

void backend_free(backend *backend) {
  if (!backend)
    return;

  backend->setup = NULL;
  backend->compile = NULL;
  backend->emit = NULL;
  backend->cleanup = NULL;
}
