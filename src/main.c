#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <signal.h>

#include <cerver/cerver.h>
#include <cerver/version.h>

#include <cerver/http/http.h>
#include <cerver/http/route.h>

#include <cerver/utils/log.h>
#include <cerver/utils/utils.h>

#include "todo.h"
#include "version.h"

#include "controllers/users.h"

#include "routes/service.h"
#include "routes/items.h"
#include "routes/users.h"

static Cerver *todo_api = NULL;

void end (int dummy) {
	
	if (todo_api) {
		cerver_stats_print (todo_api, false, false);
		cerver_log_msg ("\nHTTP Cerver stats:\n");
		http_cerver_all_stats_print ((HttpCerver *) todo_api->cerver_data);
		cerver_log_line_break ();
		cerver_teardown (todo_api);
	}

	(void) todo_end ();

	cerver_end ();

	exit (0);

}

static void todo_set_todo_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/todo
	HttpRoute *todo_route = http_route_create (REQUEST_METHOD_GET, "api/todo", todo_handler);
	http_cerver_route_register (http_cerver, todo_route);

	/* register todo children routes */
	// GET api/todo/version
	HttpRoute *todo_version_route = http_route_create (REQUEST_METHOD_GET, "version", todo_version_handler);
	http_route_child_add (todo_route, todo_version_route);

	// GET api/todo/auth
	HttpRoute *todo_auth_route = http_route_create (REQUEST_METHOD_GET, "auth", todo_auth_handler);
	http_route_set_auth (todo_auth_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (todo_auth_route, todo_user_parse_from_json, todo_user_delete);
	http_route_child_add (todo_route, todo_auth_route);

	/*** items ***/

	// GET api/todo/items
	HttpRoute *items_route = http_route_create (REQUEST_METHOD_GET, "items", todo_items_handler);
	http_route_set_auth (items_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (items_route, todo_user_parse_from_json, todo_user_delete);
	http_route_child_add (todo_route, items_route);

	// POST api/todo/items
	http_route_set_handler (items_route, REQUEST_METHOD_POST, todo_item_create_handler);

	// GET api/todo/items/:id
	HttpRoute *item_info_route = http_route_create (REQUEST_METHOD_GET, "items/:id/info", todo_item_get_handler);
	http_route_set_auth (item_info_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (item_info_route, todo_user_parse_from_json, todo_user_delete);
	http_route_child_add (todo_route, item_info_route);

	// PUT api/todo/items/:id/update
	HttpRoute *item_update_route = http_route_create (REQUEST_METHOD_PUT, "items/:id/update", todo_item_update_handler);
	http_route_set_auth (item_update_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (item_update_route, todo_user_parse_from_json, todo_user_delete);
	http_route_child_add (todo_route, item_update_route);

	// DELETE api/todo/items/:id/remove
	HttpRoute *item_remove_route = http_route_create (REQUEST_METHOD_DELETE, "items/:id/remove", todo_item_delete_handler);
	http_route_set_auth (item_remove_route, HTTP_ROUTE_AUTH_TYPE_BEARER);
	http_route_set_decode_data (item_remove_route, todo_user_parse_from_json, todo_user_delete);
	http_route_child_add (todo_route, item_remove_route);

}

static void todo_set_users_routes (HttpCerver *http_cerver) {

	/* register top level route */
	// GET /api/users
	HttpRoute *users_route = http_route_create (REQUEST_METHOD_GET, "api/users", users_handler);
	http_cerver_route_register (http_cerver, users_route);

	/* register users children routes */
	// POST api/users/login
	HttpRoute *users_login_route = http_route_create (REQUEST_METHOD_POST, "login", users_login_handler);
	http_route_child_add (users_route, users_login_route);

	// POST api/users/register
	HttpRoute *users_register_route = http_route_create (REQUEST_METHOD_POST, "register", users_register_handler);
	http_route_child_add (users_route, users_register_route);

}

static void start (void) {

	todo_api = cerver_create (
		CERVER_TYPE_WEB,
		"todo-api",
		PORT,
		PROTOCOL_TCP,
		false,
		CERVER_CONNECTION_QUEUE
	);

	if (todo_api) {
		/*** cerver configuration ***/
		cerver_set_receive_buffer_size (todo_api, CERVER_RECEIVE_BUFFER_SIZE);
		cerver_set_thpool_n_threads (todo_api, CERVER_TH_THREADS);
		cerver_set_handler_type (todo_api, CERVER_HANDLER_TYPE_THREADS);

		cerver_set_reusable_address_flags (todo_api, true);

		/*** web cerver configuration ***/
		HttpCerver *http_cerver = (HttpCerver *) todo_api->cerver_data;

		http_cerver_auth_set_jwt_algorithm (http_cerver, JWT_ALG_RS256);
		if (ENABLE_USERS_ROUTES) {
			http_cerver_auth_set_jwt_priv_key_filename (http_cerver, PRIV_KEY->str);
		}
		
		http_cerver_auth_set_jwt_pub_key_filename (http_cerver, PUB_KEY->str);

		todo_set_todo_routes (http_cerver);

		if (ENABLE_USERS_ROUTES) {
			todo_set_users_routes (http_cerver);
		}

		// add a catch all route
		http_cerver_set_catch_all_route (http_cerver, todo_catch_all_handler);

		if (cerver_start (todo_api)) {
			cerver_log_error (
				"Failed to start %s!",
				todo_api->info->name->str
			);

			cerver_delete (todo_api);
		}
	}

	else {
		cerver_log_error ("Failed to create cerver!");

		cerver_delete (todo_api);
	}

}

int main (int argc, char const **argv) {

	srand (time (NULL));

	// register to the quit signal
	(void) signal (SIGINT, end);
	(void) signal (SIGTERM, end);

	// to prevent SIGPIPE when writting to socket
	(void) signal (SIGPIPE, SIG_IGN);

	cerver_init ();

	cerver_version_print_full ();

	todo_version_print_full ();

	if (!todo_init ()) {
		start ();
	}

	else {
		cerver_log_error ("Failed to init todo!");
	}

	(void) todo_end ();

	cerver_end ();

	return 0;

}