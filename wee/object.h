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

#ifndef __WEE__OBJECT_H__
#define __WEE__OBJECT_H__

#include "hash.h"

struct msg_s;

#define OBJECT_HEADER hash_t* slots

typedef struct
{
	OBJECT_HEADER;
} obj_t;

// Create a new empty object
extern obj_t* obj_new(void);

// Create a new empty object and allocate a given size. Use this when allocating
// objects from custom C-level objects, like msg_t.
extern obj_t* obj_new_with_size(size_t);

// Destroy an object
extern void obj_destroy(obj_t*);

// Add a key->value mapping to our slot table
extern bool obj_register_slot(obj_t*, char*, void*);

// Look up a slot without following the inheritance graph
extern obj_t* obj_lookup_local(obj_t*, char*);

// Look up a slot while following the parent inheritance graph
extern obj_t* obj_lookup(obj_t*, char*);

// Use another object as a trait. Returns true if successful.
// The hash_t is used for name resolutions. If a name conflicts, you must
// explicitly resolve it, and put an: old_name -> new_name mapping in the
// hash table.
extern bool obj_use_trait(obj_t*, obj_t*, hash_t*);

// Perform a message on an object
extern obj_t* obj_perform(obj_t*, obj_t*, struct msg_s*);

#endif /* !__WEE__OBJECT_H__ */