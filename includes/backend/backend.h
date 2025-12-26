/*
 * backend: code generation for sclc, initializes necessary functions based on
 * the selected target.
 */

#ifndef BACKEND_H
#define BACKEND_H

/* forward declarations of cstate and fstate to avoid recursive inclusion */
typedef struct cstate cstate;
typedef struct fstate fstate;

typedef struct backend backend;

/*
 * @brief: Creates and initializes a new backend instance.
 *
 * @return: Pointer to newly allocated backend
 */
backend *backend_create();

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
void backend_destroy(backend *backend);

#endif // !BACKEND_H
