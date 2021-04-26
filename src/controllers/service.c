#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <cerver/http/response.h>

#include <cerver/utils/utils.h>

#include "version.h"

HttpResponse *missing_values = NULL;

HttpResponse *todo_works = NULL;
HttpResponse *current_version = NULL;

HttpResponse *catch_all = NULL;

unsigned int todo_service_init (void) {

	unsigned int retval = 1;

	missing_values = http_response_json_key_value (
		HTTP_STATUS_BAD_REQUEST, "error", "Missing values!"
	);

	todo_works = http_response_json_key_value (
		HTTP_STATUS_OK, "msg", "Todo works!"
	);

	char *status = c_string_create (
		"%s - %s", TODO_VERSION_NAME, TODO_VERSION_DATE
	);

	if (status) {
		current_version = http_response_json_key_value (
			HTTP_STATUS_OK, "version", status
		);

		free (status);
	}

	catch_all = http_response_json_key_value (
		HTTP_STATUS_OK, "msg", "Todo Service!"
	);

	if (
		missing_values
		&& todo_works && current_version
		&& catch_all
	) retval = 0;

	return retval;

}

void todo_service_end (void) {

	http_response_delete (missing_values);

	http_response_delete (todo_works);
	http_response_delete (current_version);

	http_response_delete (catch_all);

}
