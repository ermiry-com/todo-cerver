#include <cerver/utils/log.h>

#include "version.h"

// print full todo version information
void todo_version_print_full (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Todo Cerver API Version: %s", TODO_VERSION_NAME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Release Date & time: %s - %s", TODO_VERSION_DATE, TODO_VERSION_TIME
	);

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Author: %s\n", TODO_VERSION_AUTHOR
	);

}

// print the version id
void todo_version_print_version_id (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Todo Cerver API Version ID: %s", TODO_VERSION
	);

}

// print the version name
void todo_version_print_version_name (void) {

	cerver_log_both (
		LOG_TYPE_NONE, LOG_TYPE_NONE,
		"Todo Cerver API Version: %s", TODO_VERSION_NAME
	);

}