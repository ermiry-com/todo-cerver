#ifndef _TODO_ITEMS_H_
#define _TODO_ITEMS_H_

#include <bson/bson.h>

#include <cerver/types/string.h>

#include <cerver/collections/dlist.h>

#include "errors.h"

#include "models/item.h"
#include "models/user.h"

#define DEFAULT_TRANS_POOL_INIT			32

struct _HttpResponse;

extern const bson_t *item_no_user_query_opts;

extern struct _HttpResponse *no_user_item;

extern struct _HttpResponse *item_created_success;
extern struct _HttpResponse *item_created_bad;
extern struct _HttpResponse *item_deleted_success;
extern struct _HttpResponse *item_deleted_bad;

extern unsigned int todo_items_init (void);

extern void todo_items_end (void);

extern Item *todo_item_get (void);

extern Item *todo_item_get_by_id_and_user (
	const String *item_id, const bson_oid_t *user_oid
);

extern u8 todo_item_get_by_id_and_user_to_json (
	const char *item_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern TodoError todo_item_create (
	const User *user, const String *request_body
);

extern TodoError todo_item_update (
	const User *user, const String *item_id,
	const String *request_body
);

extern void todo_item_delete (void *item_ptr);

#endif