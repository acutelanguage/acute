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
#include "sparse_array.h"

static sparse_array_t* _sparse_array_empty = NULL;

sparse_array_t* sparse_array_new(void)
{
    static int item_count = 256;
    sparse_array_t* sa = calloc(1, sizeof(*sa));
    sa->shift = 0;
    sa->mask = 0xff000000;
    sa->data = calloc(item_count, sizeof(void*));
    memset(&sa->data, 0, item_count);

    if(_sparse_array_empty == NULL)
    {
        _sparse_array_empty = calloc(1, sizeof(*_sparse_array_empty));
        _sparse_array_empty->shift = 0;
        _sparse_array_empty->mask = 0xff;
        _sparse_array_empty->data = calloc(item_count, sizeof(void*));
    }

    return sa;
}

void sparse_array_destroy(sparse_array_t* sa)
{
    if(sa->shift != 0)
    {
        uint32_t max = (sa->mask >> sa->shift) + 1;
        for(uint32_t i = 0; i < max; i++)
            sparse_array_destroy((sparse_array_t*)sa->data[i]);
    }
    free(sa->data);
    free(sa);
}

void* sparse_array_lookup(sparse_array_t* sa, uint32_t index)
{
    while(sa->shift != 0)
    {
        uint32_t i = ((index & sa->mask) >> sa->shift);
        sa = (sparse_array_t*)sa->data[i];
    }
    
    return sa->data[index & sa->mask];
}

void sparse_array_put(sparse_array_t* sa, uint32_t index, void* value)
{
    static int item_count = 256;

    while(sa->shift != 0)
    {
        uint32_t i = ((index & sa->mask) >> sa->shift);
        if(sa->data[i] == _sparse_array_empty)
        {
            sparse_array_t* newsa = calloc(1, sizeof(*newsa));
            newsa->shift = sa->shift - 8;
            newsa->mask = sa->mask >> 8;
            newsa->data = calloc(item_count, sizeof(void*));
            memset(&newsa->data, 0, item_count);
            //init_pointers(newsarray);
            sa->data[i] = newsa;
        }
        sa = sa->data[i];
    }
    sa->data[index & sa->mask] = value;
}