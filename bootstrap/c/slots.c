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

#include <stdio.h>
#include "slots.h"
#include "object.h"

slot_t slot_new(struct object_s* volatile data, unsigned activatable)
{
	slot_t r = NULL;
	BEGIN_SIMPLE_FRAME(1, data, 1, r);

	r = qish_allocate(sizeof(*r));
	r->data = data;

	EXIT_FRAME();
	return r;
}

