#include <stdlib.h>

#include <time.h>

#include <cerver/types/string.h>

#include <cerver/collections/pool.h>

#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>

#include <cmongo/crud.h>
#include <cmongo/select.h>

#include "errors.h"

#include "models/item.h"
#include "models/user.h"

#include "controllers/items.h"

static Pool *items_pool = NULL;

const bson_t *item_no_user_query_opts = NULL;
static CMongoSelect *item_no_user_select = NULL;

HttpResponse *no_user_item = NULL;

HttpResponse *item_created_success = NULL;
HttpResponse *item_created_bad = NULL;
HttpResponse *item_deleted_success = NULL;
HttpResponse *item_deleted_bad = NULL;

void todo_item_delete (void *item_ptr);

static unsigned int todo_item_init_pool (void) {

	unsigned int retval = 1;

	items_pool = pool_create (item_delete);
	if (items_pool) {
		pool_set_create (items_pool, item_new);
		pool_set_produce_if_empty (items_pool, true);
		if (!pool_init (items_pool, item_new, DEFAULT_TRANS_POOL_INIT)) {
			retval = 0;
		}

		else {
			cerver_log_error ("Failed to init item pool!");
		}
	}

	else {
		cerver_log_error ("Failed to create item pool!");
	}

	return retval;

}

static unsigned int todo_item_init_query_opts (void) {

	unsigned int retval = 1;

	item_no_user_select = cmongo_select_new ();
	(void) cmongo_select_insert_field (item_no_user_select, "title");
	(void) cmongo_select_insert_field (item_no_user_select, "description");
	(void) cmongo_select_insert_field (item_no_user_select, "date");
	(void) cmongo_select_insert_field (item_no_user_select, "done");
	(void) cmongo_select_insert_field (item_no_user_select, "completed");

	item_no_user_query_opts = mongo_find_generate_opts (item_no_user_select);

	if (item_no_user_query_opts) retval = 0;

	return retval;

}

static unsigned int todo_item_init_responses (void) {

	unsigned int retval = 1;

	no_user_item = http_response_json_key_value (
		HTTP_STATUS_NOT_FOUND, "msg", "Failed to get user's item(s)"
	);

	item_created_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	item_created_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to create item!"
	);

	item_deleted_success = http_response_json_key_value (
		HTTP_STATUS_OK, "oki", "doki"
	);

	item_deleted_bad = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Failed to delete item!"
	);

	if (
		no_user_item
		&& item_created_success && item_created_bad
		&& item_deleted_success && item_deleted_bad
	) retval = 0;

	return retval;

}

unsigned int todo_items_init (void) {

	unsigned int errors = 0;

	errors |= todo_item_init_pool ();

	errors |= todo_item_init_query_opts ();

	errors |= todo_item_init_responses ();

	return errors;

}

void todo_items_end (void) {

	cmongo_select_delete (item_no_user_select);
	bson_destroy ((bson_t *) item_no_user_query_opts);

	pool_delete (items_pool);
	items_pool = NULL;

	http_response_delete (no_user_item);

	http_response_delete (item_created_success);
	http_response_delete (item_created_bad);
	http_response_delete (item_deleted_success);
	http_response_delete (item_deleted_bad);

}

Item *todo_item_get (void) {

	return (Item *) pool_pop (items_pool);

}

Item *todo_item_get_by_id_and_user (
	const String *item_id, const bson_oid_t *user_oid
) {

	Item *item = NULL;

	if (item_id) {
		item = (Item *) pool_pop (items_pool);
		if (item) {
			bson_oid_init_from_string (&item->oid, item_id->str);

			if (item_get_by_oid_and_user (
				item,
				&item->oid, user_oid,
				NULL
			)) {
				todo_item_delete (item);
				item = NULL;
			}
		}
	}

	return item;

}

u8 todo_item_get_by_id_and_user_to_json (
	const char *item_id, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (item_id) {
		bson_oid_t item_oid = { 0 };
		bson_oid_init_from_string (&item_oid, item_id);

		retval = item_get_by_oid_and_user_to_json (
			&item_oid, user_oid,
			query_opts,
			json, json_len
		);
	}

	return retval;

}

static void todo_item_parse_json (
	json_t *json_body,
	const char **title,
	const char **description
) {

	// get values from json to create a new item
	const char *key = NULL;
	json_t *value = NULL;
	if (json_typeof (json_body) == JSON_OBJECT) {
		json_object_foreach (json_body, key, value) {
			if (!strcmp (key, "title")) {
				*title = json_string_value (value);
				#ifdef TODO_DEBUG
				(void) printf ("title: \"%s\"\n", *title);
				#endif
			}

			else if (!strcmp (key, "description")) {
				*description = json_string_value (value);
				#ifdef TODO_DEBUG
				(void) printf ("description: \"%s\"\n", *description);
				#endif
			}
		}
	}

}

static Item *todo_item_create_actual (
	const char *user_id,
	const char *title,
	const char *description
) {

	Item *item = (Item *) pool_pop (items_pool);
	if (item) {
		bson_oid_init (&item->oid, NULL);

		bson_oid_init_from_string (&item->user_oid, user_id);

		if (title) {
			(void) strncpy (item->title, title, ITEM_TITLE_LEN - 1);
			item->title_len = strlen (item->title);
		}

		if (description) {
			(void) strncpy (item->description, description, ITEM_DESCRIPTION_LEN - 1);
			item->description_len = strlen (item->description);
		}

		item->date = time (NULL);
	}

	return item;

}

static TodoError todo_item_create_parse_json (
	Item **item,
	const char *user_id, const String *request_body
) {

	TodoError error = TODO_ERROR_NONE;

	const char *title = NULL;
	const char *description = NULL;

	json_error_t json_error =  { 0 };
	json_t *json_body = json_loads (request_body->str, 0, &json_error);
	if (json_body) {
		todo_item_parse_json (
			json_body,
			&title, &description
		);

		if (title) {
			*item = todo_item_create_actual (
				user_id,
				title, description
			);

			if (*item == NULL) error = TODO_ERROR_SERVER_ERROR;
		}

		else {
			error = TODO_ERROR_MISSING_VALUES;
		}

		json_decref (json_body);
	}

	else {
		cerver_log_error (
			"json_loads () - json error on line %d: %s\n", 
			json_error.line, json_error.text
		);

		error = TODO_ERROR_BAD_REQUEST;
	}

	return error;

}

TodoError todo_item_create (
	const User *user, const String *request_body
) {

	TodoError error = TODO_ERROR_NONE;

	if (request_body) {
		Item *item = NULL;

		error = todo_item_create_parse_json (
			&item,
			user->id, request_body
		);

		if (error == TODO_ERROR_NONE) {
			#ifdef TODO_DEBUG
			item_print (item);
			#endif

			if (!item_insert_one (item)) {
				// update users values
				(void) user_add_items (user);
			}

			else {
				error = TODO_ERROR_SERVER_ERROR;
			}
			
			todo_item_delete (item);
		}
	}

	else {
		cerver_log_error ("Missing request body to create item!");

		error = TODO_ERROR_BAD_REQUEST;
	}

	return error;

}

void todo_item_delete (void *item_ptr) {

	(void) memset (item_ptr, 0, sizeof (Item));
	(void) pool_push (items_pool, item_ptr);

}