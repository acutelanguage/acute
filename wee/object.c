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

#include <stdio.h>
#include <stdlib.h>
#include "sparse_array.h"
#include "list.h"
#include "object.h"

static inline uint32_t _hash_name(char* name)
{
    uint32_t hash = 6211;
    uint32_t c = 0;

    if(name != NULL)
    {
        while((c = (uint32_t)*name++))
            hash *= 33 + c;
    }

    return hash;
}

obj_t* obj_new(void)
{
    obj_t* obj = malloc(sizeof(*obj));
    obj->slots = sparse_array_new();
    obj->traits = list_new();
    return obj;
}

void obj_destroy(obj_t* obj)
{
    sparse_array_destroy(obj->slots);
    list_destroy(obj->traits);
    free(obj);
    obj = NULL;
}

obj_t* obj_lookup_local(obj_t* obj, const char* name)
{
    return NULL;
}