#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/types/types.h>
#include <cerver/types/string.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>
#include <cerver/http/request.h>
#include <cerver/http/response.h>
#include <cerver/http/json/json.h>

#include <cerver/utils/log.h>
#include <cerver/utils/utils.h>

#include "errors.h"
#include "todo.h"

#include "controllers/items.h"
#include "controllers/users.h"

#include "models/item.h"
#include "models/user.h"

// GET /api/todo/items
// get all the authenticated user's items
void todo_items_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = items_get_all_by_user_to_json (
			&user->oid, item_no_user_query_opts,
			&json_len
		);

		if (json) {
			(void) http_response_json_custom_reference_send (
				http_receive,
				HTTP_STATUS_OK,
				json, json_len
			);

			free (json);
		}

		else {
			(void) http_response_send (no_user_item, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

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

static TodoError todo_item_create_handler_internal (
	Item **item,
	const char *user_id, const String *request_body
) {

	TodoError error = TODO_ERROR_NONE;

	if (request_body) {
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
				*item = todo_item_create (
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
	}

	else {
		cerver_log_error ("Missing request body to create item!");

		error = TODO_ERROR_BAD_REQUEST;
	}

	return error;

}

// POST /api/todo/items
// a user has requested to create a new item
void todo_item_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		Item *item = NULL;

		TodoError error = todo_item_create_handler_internal (
			&item,
			user->id, request->body
		);

		if (error == TODO_ERROR_NONE) {
			#ifdef TODO_DEBUG
			item_print (item);
			#endif

			if (!item_insert_one (item)) {
				// update users values
				(void) user_add_items (user);

				// return success to user
				(void) http_response_send (
					item_created_success,
					http_receive
				);
			}

			else {
				(void) http_response_send (
					item_created_bad,
					http_receive
				);
			}
			
			todo_item_delete (item);
		}

		else {
			todo_error_send_response (error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// GET /api/todo/items/:id/info
// returns information about an existing item that belongs to a user
void todo_item_get_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *item_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		if (item_id) {
			size_t json_len = 0;
			char *json = NULL;

			if (!todo_item_get_by_id_and_user_to_json (
				item_id->str, &user->oid,
				item_no_user_query_opts,
				&json, &json_len
			)) {
				if (json) {
					(void) http_response_json_custom_reference_send (
						http_receive, HTTP_STATUS_OK, json, json_len
					);
					
					free (json);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			else {
				(void) http_response_send (no_user_item, http_receive);
			}
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

static u8 todo_item_update_handler_internal (
	Item *item, const String *request_body
) {

	u8 retval = 1;

	if (request_body) {
		const char *title = NULL;
		const char *description = NULL;

		json_error_t error =  { 0 };
		json_t *json_body = json_loads (request_body->str, 0, &error);
		if (json_body) {
			todo_item_parse_json (
				json_body,
				&title, &description
			);

			if (title) {
				(void) strncpy (item->title, title, ITEM_TITLE_LEN - 1);
				item->title_len = strlen (item->title);
			}

			if (description) {
				(void) strncpy (item->description, description, ITEM_DESCRIPTION_LEN - 1);
				item->description_len = strlen (item->description);
			}

			json_decref (json_body);

			retval = 0;
		}

		else {
			cerver_log_error (
				"json_loads () - json error on line %d: %s\n", 
				error.line, error.text
			);
		}
	}

	else {
		cerver_log_error ("Missing request body to update item!");
	}

	return retval;

}

// POST /api/todo/items/:id/update
// a user wants to update an existing item
void todo_item_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_init_from_string (&user->oid, user->id);

		Item *item = todo_item_get_by_id_and_user (
			request->params[0], &user->oid
		);

		if (item) {
			// get update values
			if (!todo_item_update_handler_internal (
				item, request->body
			)) {
				// update the item in the db
				if (!item_update_one (item)) {
					(void) http_response_send (oki_doki, http_receive);
				}

				else {
					(void) http_response_send (server_error, http_receive);
				}
			}

			todo_item_delete (item);
		}

		else {
			(void) http_response_send (bad_request_error, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// DELETE /api/todo/items/:id/remove
// deletes an existing user's item
void todo_item_delete_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	const String *item_id = request->params[0];

	User *user = (User *) request->decoded_data;
	if (user) {
		bson_oid_t oid = { 0 };
		bson_oid_init_from_string (&oid, item_id->str);

		if (!item_delete_one_by_oid_and_user (
			&oid, &user->oid
		)) {
			#ifdef TODO_DEBUG
			cerver_log_debug ("Deleted item %s", item_id->str);
			#endif

			(void) http_response_send (item_deleted_success, http_receive);
		}

		else {
			(void) http_response_send (item_deleted_bad, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}