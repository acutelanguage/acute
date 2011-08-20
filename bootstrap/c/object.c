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
