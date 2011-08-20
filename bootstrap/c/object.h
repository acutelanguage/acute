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
 * File: object.h
 * Description: Our objects
 ******************************************************************/

#ifndef __ACUTE__OBJECT_H__
#define __ACUTE__OBJECT_H__

#include "slots.h"

typedef struct object_s
{
	void* slots;     // Backed by a Judy array, see judy64d.c
} obj_t;

/* Create a new empty object. This object descends from nobody, has no slots. */
extern obj_t* obj_new_empty(void);

/* Create a new object descended from a parent. This parent is installed into the newly created slot table. */
extern obj_t* obj_new(obj_t*);

/* Register a new slot with the object. */
extern void obj_register_slot(obj_t*, char*, slot_t*);

#endif /* !__ACUTE__OBJECT_H__ */
