#include <stdlib.h>
#include <string.h>

#include <time.h>

#include <cerver/types/types.h>

#include <cerver/utils/log.h>

#include <cmongo/collections.h>
#include <cmongo/crud.h>
#include <cmongo/model.h>

#include "models/item.h"

static CMongoModel *items_model = NULL;

static void item_doc_parse (
	void *item_ptr, const bson_t *item_doc
);

unsigned int items_model_init (void) {

	unsigned int retval = 1;

	items_model = cmongo_model_create (ITEMS_COLL_NAME);
	if (items_model) {
		cmongo_model_set_parser (items_model, item_doc_parse);

		retval = 0;
	}

	return retval;

}

void items_model_end (void) {

	cmongo_model_delete (items_model);

}

void *item_new (void) {

	Item *item = (Item *) malloc (sizeof (Item));
	if (item) {
		(void) memset (item, 0, sizeof (Item));
	}

	return item;

}

void item_delete (void *item_ptr) {

	if (item_ptr) free (item_ptr);

}

void item_print (const Item *item) {

	if (item) {
		char buffer[128] = { 0 };
		bson_oid_to_string (&item->oid, buffer);
		(void) printf ("id: %s\n", buffer);

		(void) printf ("title: %s\n", item->title);

		if (item->description_len) {
			(void) printf ("description: %s\n", item->description);
		}

		(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&item->date));
		(void) printf ("date: %s GMT\n", buffer);

		if (item->done) {
			(void) strftime (buffer, 128, "%d/%m/%y - %T", gmtime (&item->completed));
			(void) printf ("completed: %s GMT\n", buffer);
		}
	}

}

static void item_doc_parse (
	void *item_ptr, const bson_t *item_doc
) {

	Item *item = (Item *) item_ptr;

	bson_iter_t iter = { 0 };
	if (bson_iter_init (&iter, item_doc)) {
		char *key = NULL;
		bson_value_t *value = NULL;
		while (bson_iter_next (&iter)) {
			key = (char *) bson_iter_key (&iter);
			value = (bson_value_t *) bson_iter_value (&iter);

			if (!strcmp (key, "_id")) {
				bson_oid_copy (&value->value.v_oid, &item->oid);
				bson_oid_to_string (&item->oid, item->id);
			}

			else if (!strcmp (key, "user"))
				bson_oid_copy (&value->value.v_oid, &item->user_oid);

			else if (!strcmp (key, "title") && value->value.v_utf8.str) {
				(void) strncpy (
					item->title,
					value->value.v_utf8.str,
					ITEM_TITLE_LEN - 1
				);

				item->title_len = strlen (item->title);
			}

			else if (!strcmp (key, "description") && value->value.v_utf8.str) {
				(void) strncpy (
					item->description,
					value->value.v_utf8.str,
					ITEM_TITLE_LEN - 1
				);

				item->description_len = strlen (item->description);
			}

			else if (!strcmp (key, "date"))
				item->date = (time_t) bson_iter_date_time (&iter) / 1000;

			else if (!strcmp (key, "done"))
				item->done = value->value.v_bool;

			else if (!strcmp (key, "completed"))
				item->completed = (time_t) bson_iter_date_time (&iter) / 1000;
		}
	}

}

bson_t *item_query_oid (const bson_oid_t *oid) {

	bson_t *query = NULL;

	if (oid) {
		query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "_id", -1, oid);
		}
	}

	return query;

}

bson_t *item_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	bson_t *item_query = bson_new ();
	if (item_query) {
		(void) bson_append_oid (item_query, "_id", -1, oid);
		(void) bson_append_oid (item_query, "user", -1, user_oid);
	}

	return item_query;

}

u8 item_get_by_oid (
	Item *item, const bson_oid_t *oid, const bson_t *query_opts
) {

	u8 retval = 1;

	if (item && oid) {
		bson_t *item_query = bson_new ();
		if (item_query) {
			(void) bson_append_oid (item_query, "_id", -1, oid);
			retval = mongo_find_one_with_opts (
				items_model,
				item_query, query_opts,
				item
			);
		}
	}

	return retval;

}

u8 item_get_by_oid_and_user (
	Item *item,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
) {

	u8 retval = 1;

	if (item && oid && user_oid) {
		bson_t *item_query = item_query_by_oid_and_user (
			oid, user_oid
		);

		if (item_query) {
			retval = mongo_find_one_with_opts (
				items_model,
				item_query, query_opts,
				item
			);
		}
	}

	return retval;

}

u8 item_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
) {

	u8 retval = 1;

	if (oid && user_oid) {
		bson_t *item_query = item_query_by_oid_and_user (
			oid, user_oid
		);

		if (item_query) {
			retval = mongo_find_one_with_opts_to_json (
				items_model,
				item_query, query_opts,
				json, json_len
			);
		}
	}

	return retval;

}

bson_t *item_to_bson (const Item *item) {

    bson_t *doc = NULL;

    if (item) {
        doc = bson_new ();
        if (doc) {
            (void) bson_append_oid (doc, "_id", -1, &item->oid);

			(void) bson_append_oid (doc, "user", -1, &item->user_oid);

			(void) bson_append_utf8 (doc, "title", -1, item->title, item->title_len);
			(void) bson_append_utf8 (doc, "description", -1, item->description, item->description_len);
			
			(void) bson_append_date_time (doc, "date", -1, item->date * 1000);

			(void) bson_append_bool (doc, "done", -1, item->done);
			(void) bson_append_date_time (doc, "completed", -1, item->completed * 1000);
        }
    }

    return doc;

}

bson_t *item_update_bson (const Item *item) {

	bson_t *doc = NULL;

    if (item) {
        doc = bson_new ();
        if (doc) {
			bson_t set_doc = BSON_INITIALIZER;
			(void) bson_append_document_begin (doc, "$set", -1, &set_doc);

			(void) bson_append_utf8 (&set_doc, "title", -1, item->title, item->title_len);
			(void) bson_append_utf8 (&set_doc, "description", -1, item->description, item->description_len);

			(void) bson_append_bool (&set_doc, "done", -1, item->done);
			(void) bson_append_date_time (&set_doc, "completed", -1, item->completed * 1000);

			(void) bson_append_document_end (doc, &set_doc);
        }
    }

    return doc;

}

// get all the items that are related to a user
mongoc_cursor_t *items_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
) {

	mongoc_cursor_t *retval = NULL;

	if (user_oid && opts) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			retval = mongo_find_all_cursor_with_opts (
				items_model,
				query, opts
			);
		}
	}

	return retval;

}

char *items_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	size_t *json_len
) {

	char *json = NULL;

	if (user_oid) {
		bson_t *query = bson_new ();
		if (query) {
			(void) bson_append_oid (query, "user", -1, user_oid);

			json = mongo_find_all_cursor_with_opts_to_json (
				items_model,
				query, opts,
				"items", json_len
			);
		}
	}

	return json;

}

unsigned int item_insert_one (const Item *item) {

	return mongo_insert_one (
		items_model, item_to_bson (item)
	);

}

unsigned int item_update_one (const Item *item) {

	return mongo_update_one (
		items_model,
		item_query_oid (&item->oid),
		item_update_bson (item)
	);

}

unsigned int item_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
) {

	unsigned int retval = 1;

	if (oid && user_oid) {
		bson_t *item_query = item_query_by_oid_and_user (
			oid, user_oid
		);

		if (item_query) {
			retval = mongo_delete_one (
				items_model, item_query
			);
		}
	}

	return retval;

}