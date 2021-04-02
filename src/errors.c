#include <cerver/handler.h>

#include <cerver/http/http.h>
#include <cerver/http/response.h>

#include "todo.h"
#include "errors.h"

#include "controllers/service.h"

const char *todo_error_to_string (const TodoError type) {

	switch (type) {
		#define XX(num, name, string) case TODO_ERROR_##name: return #string;
		TODO_ERROR_MAP(XX)
		#undef XX
	}

	return todo_error_to_string (TODO_ERROR_NONE);

}

void todo_error_send_response (
	const TodoError error,
	const HttpReceive *http_receive
) {

	switch (error) {
		case TODO_ERROR_NONE: break;

		case TODO_ERROR_BAD_REQUEST:
			(void) http_response_send (bad_request_error, http_receive);
			break;

		case TODO_ERROR_MISSING_VALUES:
			(void) http_response_send (missing_values, http_receive);
			break;

		case TODO_ERROR_BAD_USER:
			(void) http_response_send (bad_user_error, http_receive);
			break;

		case TODO_ERROR_SERVER_ERROR:
			(void) http_response_send (server_error, http_receive);
			break;

		default: break;
	}

}