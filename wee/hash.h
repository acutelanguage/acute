/*
 * Acute Programming Language
 * Copyright (c) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __WEE__HASH_H__
#define __WEE__HASH_H__

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

// Create a new empty hash table with the given capacity
extern hash_t* hash_new(size_t);

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

#endif /* !__WEE__HASH_H__ */