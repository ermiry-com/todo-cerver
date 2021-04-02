#ifndef _TODO_ERRORS_H_
#define _TODO_ERRORS_H_

#define TODO_ERROR_MAP(XX)						\
	XX(0,	NONE, 				None)				\
	XX(1,	BAD_REQUEST, 		Bad Request)		\
	XX(2,	MISSING_VALUES, 	Missing Values)		\
	XX(3,	BAD_USER, 			Bad User)			\
	XX(4,	SERVER_ERROR, 		Server Error)

typedef enum TodoError {

	#define XX(num, name, string) TODO_ERROR_##name = num,
	TODO_ERROR_MAP (XX)
	#undef XX

} TodoError;

extern const char *todo_error_to_string (
	const TodoError type
);

extern void todo_error_send_response (
	const TodoError error,
	const struct _HttpReceive *http_receive
);

#endif