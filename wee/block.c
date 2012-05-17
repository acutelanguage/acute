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
#include "block.h"
#include "str.h"
#include "object_space.h"

block_t* block_new(msg_t* body, list_t* names, obj_t* scope)
{
    block_t* blk = (block_t*)obj_new_with_size(sizeof(*blk));
    hash_insert(blk->slots, "message", body);
    list_t* arg_names = list_new();
    list_foreach(names, ^(size_t index, void* element) {
        char* name = (char*)element;
        str_t* str = str_new(name, strlen(name) + 1);
        list_append(arg_names, str);
    });
    hash_insert(blk->slots, "argumentNames", arg_names);
    hash_insert(blk->slots, "scope", scope);

    return blk;
}

void block_destroy(block_t* blk)
{
    msg_t* msg = (msg_t*)hash_get(blk->slots, "message");
    if(msg)
        msg_destroy(msg);
    list_t* arg_names = (list_t*)hash_get(blk->slots, "argumentNames");
    if(arg_names)
        list_destroy(arg_names);
    obj_t* scope = (obj_t*)hash_get(blk->slots, "scope");
    if(scope)
        free(scope);

    free(blk);
    blk = NULL;
}

static obj_t* block_locals_new(void)
{
    obj_t* locals = obj_new();
    return locals;
}

obj_t* block_activate(block_t* blk, obj_t* target, obj_t* sender, msg_t* msg, obj_t* slot_context)
{
    list_t* arg_names = (list_t*)hash_get(blk->slots, "argumentNames");
    obj_t* scope = (obj_t*)hash_get(blk->slots, "scope");
    if(scope == NULL)
        scope = target;

    obj_t* locals = block_locals_new();

    if(arg_names)
    {
        list_foreach(arg_names, ^(size_t index, void* element) {
            obj_t* val = msg_eval_arg_at(msg, sender, index);
            obj_register_slot(locals, (char*)element, val);
        });
    }

    msg_t* message = (msg_t*)hash_get(blk->slots, "message");
    return msg_perform_on(message, locals, locals);
}

obj_t* block_call(block_t* blk, obj_t* locals, msg_t* msg)
{
    return block_activate(blk, locals, locals, msg, locals);
}