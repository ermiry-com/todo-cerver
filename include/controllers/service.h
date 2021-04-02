#ifndef _TODO_SERVICE_H_
#define _TODO_SERVICE_H_

struct _HttpResponse;

extern struct _HttpResponse *missing_values;

extern struct _HttpResponse *todo_works;
extern struct _HttpResponse *current_version;

extern struct _HttpResponse *catch_all;

extern unsigned int todo_service_init (void);

extern void todo_service_end (void);

#endif