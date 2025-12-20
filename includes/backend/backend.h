#ifndef BACKEND_H
#define BACKEND_H

/* forward declarations of cstate and fstate to avoid recursive inclusion */
typedef struct cstate cstate;
typedef struct fstate fstate;

typedef struct backend backend;

backend *backend_create();

void backend_compile(backend *backend, cstate *cst, fstate *fst);

void backend_destroy(backend *backend);

#endif // !BACKEND_H
