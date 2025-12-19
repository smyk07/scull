#ifndef BACKEND_H
#define BACKEND_H

/* forward declarations of cstate and fstate to avoid recursive inclusion */
typedef struct cstate cstate;
typedef struct fstate fstate;

typedef enum target_kind {
  TARGET_FASM = 0,

  TARGET_LLVM_X86_64,
  TARGET_LLVM_AARCH64,
  TARGET_LLVM_RISCV64,

  TARGET_C,
} target_kind;

#define TARGET_FASM_STR "fasm"
#define TARGET_LLVM_X86_64_STR "x86_64"
#define TARGET_LLVM_AARCH64_STR "aarch64"
#define TARGET_LLVM_RISCV64_STR "riscv64"
#define TARGET_C_STR "c"

const char *target_kind_to_string(target_kind target);

typedef struct backend backend;

backend *backend_create(target_kind target);

void backend_compile(backend *backend, cstate *cst, fstate *fst);

void backend_destroy(backend *backend);

#endif // !BACKEND_H
