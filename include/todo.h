#ifndef _TODO_H_
#define _TODO_H_

#include <stdbool.h>

#include "runtime.h"

#define DEFAULT_PORT					"5000"

struct _HttpResponse;

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const String *PRIV_KEY;
extern const String *PUB_KEY;

extern bool ENABLE_USERS_ROUTES;

// inits todo main values
extern unsigned int todo_init (void);

// ends todo main values
extern unsigned int todo_end (void);

#endif