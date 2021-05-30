#ifndef _TODO_H_
#define _TODO_H_

#include <stdbool.h>

#include "runtime.h"

#define DEFAULT_PORT					5000

#define MONGO_URI_SIZE					256
#define MONGO_APP_NAME_SIZE				32
#define MONGO_DB_SIZE					32

#define PRIV_KEY_SIZE					128
#define PUB_KEY_SIZE					128

struct _HttpCerver;

extern struct _HttpCerver *http_cerver;

extern RuntimeType RUNTIME;

extern unsigned int PORT;

extern unsigned int CERVER_RECEIVE_BUFFER_SIZE;
extern unsigned int CERVER_TH_THREADS;
extern unsigned int CERVER_CONNECTION_QUEUE;

extern const char *PRIV_KEY;
extern const char *PUB_KEY;

extern bool ENABLE_USERS_ROUTES;

// inits todo main values
extern unsigned int todo_init (void);

// ends todo main values
extern unsigned int todo_end (void);

#endif