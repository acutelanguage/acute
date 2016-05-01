// Acute
// Copyright © 2016, Jeremy Tregunna, All Rights Reserved.

#ifndef __ACUTE__HASH_H__
#define __ACUTE__HASH_H__

#include <stdbool.h>

typedef struct hash_record
{
	char*               key;
	void*               value;
	struct hash_record* next;
} hash_record_t;

typedef struct
{
	size_t          capacity;
	size_t          count;
	hash_record_t** records;
} hash_t;

// Initialize a new empty hash table with the given capacity. Returns true on
// successful initialization, false on error.
extern bool hash_init(hash_t*, size_t);

// Destroy the hash table
extern void hash_destroy(hash_t*);

// Insert an item into the hash table. Returns true if successful.
extern bool hash_insert(hash_t*, char*, void*);

// Retrieve an item from the hash table
extern void* hash_get(hash_t*, char*);

// Delete an item from the hash table. Returns true if successful.
extern bool hash_delete(hash_t*, char*);

// Returns the number of items in the hash table
extern size_t hash_count(hash_t*);

#endif // !__ACUTE__HASH_H__