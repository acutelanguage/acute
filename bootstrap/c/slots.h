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
 * File: slots.h
 * Description: Slot management and data structures
 ******************************************************************/

#ifndef __ACUTE__SLOTS_H__
#define __ACUTE__SLOTS_H__

struct object_s;

typedef struct slot_s
{
	struct object_s* data;
	unsigned         activatable:1;
} slot_t;

/* Create a new slot. This slot must have some data associated with it, and currently only accepts the value which will
   be used for the data pointer in the data union. Also takes a function which can release the data, and a flag to indicate
   whether we're activatable. */
extern slot_t* slot_new(struct object_s*, unsigned);

#endif /* !__ACUTE__SLOTS_H__ */
