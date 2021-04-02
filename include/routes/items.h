#ifndef _TODO_ROUTES_ITEMS_H_
#define _TODO_ROUTES_ITEMS_H_

struct _HttpReceive;
struct _HttpResponse;

// GET /api/todo/items
// get all the authenticated user's items
extern void todo_items_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// POST /api/todo/items
// a user has requested to create a new item
extern void todo_item_create_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// GET /api/todo/items/:id/info
// returns information about an existing item that belongs to a user
extern void todo_item_get_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// PUT /api/todo/items/:id/update
// a user wants to update an existing item
extern void todo_item_update_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

// DELETE /api/todo/items/:id/remove
// deletes an existing user's item
extern void todo_item_delete_handler (
	const struct _HttpReceive *http_receive,
	const struct _HttpRequest *request
);

#endif