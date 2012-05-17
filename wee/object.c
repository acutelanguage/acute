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
#include <stdint.h>
#include <string.h>
#include "list.h"
#include "slot.h"
#include "hash.h"
#include "message.h"
#include "object.h"

#define DEFAULT_SLOTTABLE_SIZE 8

obj_t* obj_new(void)
{
    return obj_new_with_size(sizeof(obj_t));
}

obj_t* obj_new_with_size(size_t size)
{
    obj_t* obj = malloc(size);
    obj->slots = hash_new(DEFAULT_SLOTTABLE_SIZE);
    obj->cached_result = NULL;
    return obj;
}

void obj_destroy(obj_t* obj)
{
    hash_destroy(obj->slots);
    free(obj);
    obj = NULL;
}

bool obj_register_slot(obj_t* obj, char* name, void* value)
{
    return hash_insert(obj->slots, name, value);
}

obj_t* obj_lookup_local(obj_t* obj, char* name)
{
    // First, lets look up a "lookup" slot, and use that instead of this if we have it.
    obj_t* lookup = hash_get(obj->slots, "lookup");

    if(!lookup)
        return hash_get(obj->slots, name);
    else
    {
        //TODO: Implement blocks, call lookup -- a block, with the appropriate arguments
        //      and return its value, whatever that may be.
    }

    return NULL;
}

obj_t* obj_lookup(obj_t* obj, char* name)
{
    // First, lets look up a "lookup" slot, and use that instead of this if we have it.
    obj_t* lookup = hash_get(obj->slots, "lookup");
    // Maintain a list of objects we've scanned, to avoid lookup loops.
    list_t* objects_scanned = list_new();

    if(list_contains(objects_scanned, obj))
        return NULL;

    if(!lookup)
    {
        obj_t* value = hash_get(obj->slots, name);
        if(value)
            return value;
        // Now mark the object 'dirty' for our purposes. We've already scanned it.
        list_append(objects_scanned, obj);
        
        obj_t* parent = obj_lookup(obj, "parent");
        value = NULL;
        if(parent)
            value = obj_lookup(parent, name);
        //list_remove(objects_scanned, obj); // TODO: Implement
        return value;
    }
    else
    {
        //TODO: Implement blocks, call lookup -- a block, with the appropriate arguments
        //      and return its value, whatever that may be.
    }

    // Didn't find anything, oops! We don't raise an error here, since we only care about
    // returning a value we found or an invalid value. Callers should implement their own
    // error handling in cases they don't desire.
    return NULL;
}

bool obj_use_trait(obj_t* obj, obj_t* trait, hash_t* resolutions)
{
    __block bool is_error = false;

    hash_foreach(trait->slots, ^(char* key, void* value) {
        if(obj_lookup_local(obj, key))
        {
            char* name = (char*)hash_get(resolutions, key);
            if(!name)
            {
                is_error = true;
                return; // Should raise an error here too. User hasn't handled conflicts
            }
            else
                obj_register_slot(obj, name, value);
        }
        else
            obj_register_slot(obj, key, value);
    });
    return false;
}

obj_t* obj_perform(obj_t* target, obj_t* locals, msg_t* msg)
{
    obj_t* obj = obj_lookup(target, msg->name);
    if(obj && obj->cached_result)
        return obj->cached_result;

    if(obj)
    {
        obj_t* activate = obj_lookup(obj, "activate");
        // TODO: Implement blocks, call activate if it exists. Return the result.
    }
    obj = obj_lookup(target, "forward");
    if(obj)
    {
        // TODO: Implement blocks, call obj if it exists. Return the result.
    }
    return NULL;
}