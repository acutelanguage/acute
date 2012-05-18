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
#include "message.h"

msg_t* msg_new(const char* name, list_t* arguments, msg_t* next)
{
    msg_t* msg = (msg_t*)obj_new_with_size(sizeof(*msg));
    size_t name_len = strlen(name) + 1;
    msg->name = malloc(sizeof(name_len));
    memcpy(msg->name, name, name_len);
    msg->arguments = arguments;
    msg->next = next;
    return msg;
}

void msg_destroy(msg_t* msg)
{
    free(msg->name);
    free(msg);
    msg = NULL;
}

obj_t* msg_perform_on(msg_t* msg, obj_t* target, obj_t* locals)
{
    obj_t* result = target;
    msg_t* m = msg;

    do
    {
        result = obj_perform(target, locals, m);
        target = result;
    } while((m = m->next));

    return result;
}

obj_t* msg_eval_arg_at(msg_t* msg, obj_t* sender, size_t index)
{
    msg_t* arg = list_at(msg->arguments, index);
    if(!arg)
        return NULL;

    if(arg->cached_result)
        return arg->cached_result;

    return msg_perform_on(arg, sender, sender);
}