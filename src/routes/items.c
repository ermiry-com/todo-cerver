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

// GET /api/todo/items
// get all the authenticated user's items
void todo_items_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		size_t json_len = 0;
		char *json = NULL;

		if (!items_get_all_by_user_to_json (
			&user->oid, item_no_user_query_opts,
			&json, &json_len
		)) {
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
			(void) http_response_send (no_user_item, http_receive);
		}
	}

	else {
		(void) http_response_send (bad_user_error, http_receive);
	}

}

// POST /api/todo/items
// a user has requested to create a new item
void todo_item_create_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		TodoError error = todo_item_create (user, request->body);
		switch (error) {
			case TODO_ERROR_NONE: {
				(void) http_response_send (
					item_created_success, http_receive
				);
			} break;

			default: {
				todo_error_send_response (error, http_receive);
			} break;
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

// POST /api/todo/items/:id/update
// a user wants to update an existing item
void todo_item_update_handler (
	const HttpReceive *http_receive,
	const HttpRequest *request
) {

	User *user = (User *) request->decoded_data;
	if (user) {
		TodoError error = todo_item_update (
			user, request->params[0],
			request->body
		);

		switch (error) {
			case TODO_ERROR_NONE: {
				(void) http_response_send (oki_doki, http_receive);
			} break;

			default: {
				todo_error_send_response (error, http_receive);
			} break;
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