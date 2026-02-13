/*
 * backend: code generation for sclc, initializes necessary functions based on
 * the selected target.
 */

#ifndef BACKEND_H
#define BACKEND_H

/* forward declarations of cstate and fstate to avoid recursive inclusion */
typedef struct cstate cstate;
typedef struct fstate fstate;

typedef struct backend {
  void (*setup)(cstate *cst); // one time setup for the underlying backend to
                              // which we are providing an abstract layer

  void (*compile)(cstate *cst, fstate *fst);  // Compile a single file
  void (*emit)(cstate *cst, fstate *fst);     // Optimize IR
  void (*optimize)(cstate *cst, fstate *fst); // Emit object file
  void (*cleanup)(cstate *cst, fstate *fst);  // cleanup file specific resources

  void (*link)(cstate *cst); // link all object files
} backend;

/*
 * @brief: Initialize a new backend instance.
 *
 * @param backend: Pointer to the backend instance
 * @param cst: Pointer to compiler state containing global compilation context
 *
 * @return: Pointer to newly allocated backend
 */
void backend_init(backend *backend, cstate *cst);

/*
 * @brief: Compiles the parsed program using the backend.
 *
 * @param backend: Pointer to the backend instance
 * @param cst: Pointer to compiler state containing global compilation context
 * @param fst: Pointer to file state containing the parsed program and artifacts
 */
void backend_compile(backend *backend, cstate *cst, fstate *fst);

/*
 * @brief: Frees all memory associated with a backend instance.
 *
 * @param backend: Pointer to the backend to be freed
 */
void backend_free(backend *backend);

#endif // !BACKEND_H
