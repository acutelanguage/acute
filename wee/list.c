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
#include <string.h>
#include "list.h"

list_t* list_new(void)
{
    return list_new_size(8);
}

list_t* list_new_size(size_t capacity)
{
    list_t* list = malloc(sizeof(*list));
    list->capacity = capacity;
    list->size = 0;
    list->data = calloc(capacity, sizeof(void*));
    return list;
}

void list_destroy(list_t* list)
{
    free(list->data);
    free(list);
    list = NULL;
}

void list_append(list_t* list, void* item)
{
    if(list->size == list->capacity)
    {
        list->data = realloc(list->data, list->capacity * 2);
        list->capacity *= 2;
    }
    list->data[list->size + 1] = item;
}

void list_put_at(list_t* list, void* item, size_t index)
{
    if(index >= list->size || index >= list->capacity)
        list->data[index] = item;
}

void* list_at(list_t* list, size_t index)
{
    if(index <= list->size)
        return list->data[index];
    return NULL;
}

size_t list_count(list_t* list)
{
    return list->size;
}

void list_foreach(list_t* list, void (^iterator)(size_t index, void* element))
{
    for(size_t c = 0; c < list->size; c++)
        iterator(c, list->data[c]);
}