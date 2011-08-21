/*******************************************************************
 * Acute Programming Language
 * Copyright (C) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * This software project, which includes this module, is protected
 * under Canadian copyright legislation, as well as international
 * treaty. It may be used only as directed by the copyright holder.
 * The copyright holder hereby consents to usage and distribution
 * based on the terms and conditions of the MIT license, which may
 * be found in the LICENSE.MIT file included in this distribution.
 *******************************************************************
 * Project: Acute Programming Language
 * File: object.c
 * Description: Objects, objects, objects!
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "object.h"
#include "judy64d.h"
#include "closure.h"

#define INITIAL_LEVELS 4

obj_t* obj_new_empty(size_t size)
{
	if(size == 0)
		size = sizeof(struct object_s);

	obj_t* r = malloc(size);
	r->slots = judy_open(INITIAL_LEVELS);
	r->marked = 0;
	r->object_size = size;
	return r;
}

obj_t* obj_new(size_t size, obj_t* parent)
{
	obj_t* r = obj_new_empty(size);
	obj_register_slot(r, "parent", slot_new(parent, 0));
	return r;
}

void obj_release(obj_t* obj)
{
	judy_close(obj->slots);
	free(obj);
	obj = NULL;
}

void obj_register_slot(obj_t* self, char* str, slot_t* slot)
{
	judyslot* i = judy_cell(self->slots, (unsigned char*)str, strlen((char*)str));
	i = (judyslot*)slot;
}

slot_t* obj_lookup_slot_locally(obj_t* self, char* name)
{
	slot_t* slot = (slot_t*)judy_slot(self->slots, (unsigned char*)name, 4);
	return slot;
}

slot_t* obj_lookup_slot(obj_t* self, char* name)
{
	closure_t* lookup = (closure_t*)obj_lookup_slot_locally(self, "lookup");
	slot_t* slot = NULL;
	slot_t* parent_slot = NULL;

	// Need to construct a message type here, pass arguments, all that stuff.
	closure_env_t env;
	env.receiver = self;
	env.locals = self; // This is wrong, need an additional parameter.
	// Create a message here to lookup, pass along the arguments we need to.
	slot = obj_lookup_slot_locally(self, "lookup");
	if(slot)
	{
		slot_t* other = (slot_t*)obj_perform(self, slot, &env);
		return (slot_t*)obj_perform(self, other, &env);
	}
	else
	{
		slot = obj_lookup_slot_locally(self, name);
		if(slot)
			return slot;

		while((parent_slot = obj_lookup_slot_locally(self, "parent")) != NULL)
		{
			// FIXME: Prevent loops
			slot = obj_lookup_slot_locally(parent_slot->data, name);
			if(slot)
				return slot;
		}
	}

	// We should raise an exception here instead, but no exceptions yet.
	return NULL;
}

obj_t* obj_perform(obj_t* self, slot_t* slot, closure_env_t* env)
{
	if(slot->activatable)
	{
		closure_t* func = (closure_t*)slot->data;
		func->env = env;
		closure_t* activate = (closure_t*)obj_lookup_slot(slot->data, "activate");
		if(activate)
		{
			// Create an activation record including scope and all that jazz and set it in the env
			return activate->call(env);
		}
	}
	return slot->data;
}
