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

obj_t* obj_new_empty(void)
{
	obj_t* r = malloc(sizeof(*r));
	r->slots = judy_open(INITIAL_LEVELS);
	return r;
}

obj_t* obj_new(obj_t* parent)
{
	obj_t* r = obj_new_empty();
	obj_register_slot(r, "parent", slot_new(parent, 0));
	return r;
}

void obj_register_slot(obj_t* self, char* str, slot_t* slot)
{
	judyslot* i = judy_cell(self->slots, (unsigned char*)str, strlen((char*)str));
	i = (judyslot*)slot;
}
