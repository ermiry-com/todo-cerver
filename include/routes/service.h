#ifndef _TODO_ROUTES_SERVICE_H_
#define _TODO_ROUTES_SERVICE_H_

struct _HttpReceive;
struct _HttpRequest;


// GET /api/todo
extern void todo_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/todo/version
extern void todo_version_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/todo/auth
extern void todo_auth_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET *
extern void todo_catch_all_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif