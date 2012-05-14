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
#include <stdint.h>
#include <stdbool.h>
#include "hash.h"

static inline uint32_t fnv_hash_1a(char* str, uint32_t hval)
{
    unsigned char* s = (unsigned char*)str;

    while(*s)
    {
        hval ^= (uint32_t)*s++;
        hval *= 0x01000193;
    }

    return hval;
}

hash_t* hash_new(size_t capacity)
{
    hash_t* hash = malloc(sizeof(*hash));
    if(hash == NULL)
        return NULL;

    hash->count = 0;
    hash->capacity = capacity;

    hash->records = calloc(capacity, sizeof(hash_record_t*));
    if(hash->records == NULL)
    {
        free(hash);
        return NULL;
    }

    return hash;
}

void hash_destroy(hash_t* hash)
{
    hash_record_t* record = NULL;
    hash_record_t* old = NULL;

    for(size_t i = 0; i < hash->capacity; i++)
    {
        record = hash->records[i];
        while(record)
        {
            free(record->key);
            old = record;
            record = record->next;
            free(old);
        }
    }

    free(hash->records);
    free(hash);
    hash = NULL;
}



bool hash_insert(hash_t* hash, char* key, void* value)
{
    hash_record_t* record = NULL;
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5); // Magic numbers, must love.

    record = hash->records[hval];
    while(record)
    {
        if(!strcmp(record->key, key))
        {
            record->value = value;
            hash->count += 1;
            return true;
        }
        record = record->next;
    }

    record = malloc(sizeof(*record));
    if(record == NULL)
        return false;
    record->key = strdup(key);
    if(record->key == NULL)
    {
        free(record);
        return false;
    }

    record->value = value;
    record->next = hash->records[hval];

    if(hash->count == hash->capacity)
    {
        size_t capacity = hash->capacity * 2;
        hash->records = realloc(hash->records, capacity);
        memset(hash->records + capacity * sizeof(hash_record_t*), 0, (capacity - hash->capacity) * sizeof(hash_record_t*));
    }
    hash->count += 1;
    hash->records[hval] = record;

    return true;
}

void* hash_get(hash_t* hash, char* key)
{
    hash_record_t* record = NULL;
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5); // Magic numbers, must love.

    record = hash->records[hval];
    while(record)
    {
        if(!strcmp(record->key, key))
            return record->value;
        record = record->next;
    }

    return NULL;
}

bool hash_delete(hash_t* hash, char* key)
{
    hash_record_t* record = NULL;
    hash_record_t* previous = NULL;
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5); // Magic numbers, must love.

    record = hash->records[hval];
    while(record)
    {
        if(!strcmp(record->key, key))
        {
            free(record->key);
            if(previous)
                previous->next = record->next;
            else
                hash->records[hval] = record->next;
            free(record);
            hash->count -= 1;
            return true;
        }
        previous = record;
        record = record->next;
    }

    return false;
}

size_t hash_count(hash_t* hash)
{
    return hash->count;
}

void hash_foreach(hash_t* hash, void (^iterator)(char* key, void* value))
{
    hash_record_t* record = NULL;
    for(size_t n = 0; n < hash_count(hash); n++)
    {
        record = hash->records[n];
        if(record)
            iterator(record->key, record->value);
    }
}