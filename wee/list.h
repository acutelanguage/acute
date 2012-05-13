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

#ifndef __WEE__LIST_H__
#define __WEE__LIST_H__

typedef struct
{
	size_t capacity;
	size_t size;
	void** data;
} list_t;

// Create a new list
extern list_t* list_new(void);

// Creates a list and preallocates its data array to the supplied size
extern list_t* list_new_size(size_t);

// Destroy the list
extern void list_destroy(list_t*);

// Add an item to the end of the list
extern void list_append(list_t*, void*);

// Add an item at a specific index
extern void list_put_at(list_t*, void*, size_t);

// Retrieve an item at a given index
extern void* list_at(list_t*, size_t);

// Retrieve the count of items in the list
extern size_t list_count(list_t*);

// Iterate items in the list
extern void list_foreach(list_t*, void (^iterator)(size_t index, void* element));

#endif /* !__WEE__LIST_H__ */