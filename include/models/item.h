#ifndef _MODELS_ITEM_H_
#define _MODELS_ITEM_H_

#include <time.h>

#include <bson/bson.h>
#include <mongoc/mongoc.h>

#include <cerver/types/types.h>

#define ITEMS_COLL_NAME         "items"

#define	ITEM_ID_LEN					32
#define ITEM_TITLE_LEN				256
#define ITEM_DESCRIPTION_LEN		1024

extern unsigned int items_model_init (void);

extern void items_model_end (void);

typedef struct Item {

	// item's unique id
	bson_oid_t oid;
	char id[ITEM_ID_LEN];

	// reference to the owner of this item
	bson_oid_t user_oid;

	size_t title_len;
	char title[ITEM_TITLE_LEN];

	size_t description_len;
	char description[ITEM_DESCRIPTION_LEN];

	// when the item was made
	time_t date;

	// when the item was completed
	bool done;
	time_t completed;

} Item;

extern void *item_new (void);

extern void item_delete (void *item_ptr);

extern void item_print (const Item *item);

extern bson_t *item_query_oid (const bson_oid_t *oid);

extern bson_t *item_query_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

extern u8 item_get_by_oid (
	Item *item, const bson_oid_t *oid, const bson_t *query_opts
);

extern u8 item_get_by_oid_and_user (
	Item *item,
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts
);

extern u8 item_get_by_oid_and_user_to_json (
	const bson_oid_t *oid, const bson_oid_t *user_oid,
	const bson_t *query_opts,
	char **json, size_t *json_len
);

extern bson_t *item_to_bson (const Item *item);

extern bson_t *item_update_bson (const Item *item);

// get all the items that are related to a user
extern mongoc_cursor_t *items_get_all_by_user (
	const bson_oid_t *user_oid, const bson_t *opts
);

extern char *items_get_all_by_user_to_json (
	const bson_oid_t *user_oid, const bson_t *opts,
	size_t *json_len
);

extern unsigned int item_insert_one (
	const Item *item
);

extern unsigned int item_update_one (
	const Item *item
);

extern unsigned int item_delete_one_by_oid_and_user (
	const bson_oid_t *oid, const bson_oid_t *user_oid
);

#endif