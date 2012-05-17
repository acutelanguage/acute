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
#include <pthread.h>
#include "hash.h"
#include "object.h"
#include "object_space.h"

static object_space_t* global_object_space = NULL;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;

static void object_space_init(void)
{
    global_object_space = malloc(sizeof(*global_object_space));
    // Chicken-egg problem commences below.
    obj_t* lobby = obj_new();
    obj_t* object = obj_new();
    obj_register_slot(lobby, "parent", object);
    obj_register_slot(object, "parent", lobby);
    object_space_register_proto(global_object_space, "Object", object);
}

object_space_t* object_space_get(void)
{
    pthread_once(&once_control, object_space_init);
    return global_object_space;
}

// Destroy the object space.
void object_space_destroy(object_space_t* space)
{
    obj_t* object = hash_get(space->lobby->slots, "Object");
    free(object);
    obj_destroy(space->lobby);
    free(space);
    space = NULL;
}

// Register a new prototype
void object_space_register_proto(object_space_t* space, char* name, obj_t* value)
{
    obj_register_slot(space->lobby, name, value);
}