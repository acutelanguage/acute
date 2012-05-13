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

#ifndef __WEE__SPARSE_ARRAY_H__
#define __WEE__SPARSE_ARRAY_H__

#include <stdint.h>

/* Sparse arrays are used for our dispatch tables. All the values are weakly
   linked. */
typedef struct
{
	uint32_t mask;
	uint32_t shift;
	void** data;
} sparse_array_t;

// Create a new empty sparse array
extern sparse_array_t* sparse_array_new();

// Delete the sparse array
extern void sparse_array_destroy(sparse_array_t*);

// Puts a value at a given index.
// Takes a pointer to the array, an index and the value.
extern void sparse_array_put(sparse_array_t*, uint32_t, void*);

// Takes a pointer to a sparse array and an index
extern void* sparse_array_lookup(sparse_array_t*, uint32_t);


#endif /* !__WEE__SPARSE_ARRAY_H__ */