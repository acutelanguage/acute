// Acute
// Copyright © 2016, Jeremy Tregunna, All Rights Reserved.

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

bool hash_init(hash_t* hash, size_t capacity)
{
    if(hash == NULL) {
        return false;
    }

    hash->count = 0;
    hash->capacity = capacity;

    hash->records = calloc(capacity, sizeof(hash_record_t*));
    if(hash->records == NULL)
    {
        return false;
    }

    return true;
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
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5) % hash->capacity; // Magic numbers, must love.

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
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5) % hash->capacity; // Magic numbers, must love.

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
    uint32_t hval = fnv_hash_1a(key, 0x811c9dc5) % hash->capacity; // Magic numbers, must love.

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
