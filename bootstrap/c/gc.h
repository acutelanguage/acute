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
 * File: gc.h
 * Description: Copying garbage collector for C. Heavily inspired
 *              by the GC in Potion, which itself is inspired by
 *              Qish.
 ******************************************************************/

#ifndef __ACUTE__GC_H__
#define __ACUTE__GC_H__

#include "list.h"

typedef struct gc_s
{
	list_t* mark_stack;
	list_t* roots;
} gc_t;

extern void gc_collect(gc_t*);
extern void gc_mark_heap(gc_t*);
extern void gc_mark(gc_t*);
extern void gc_sweep(gc_t*);

#endif /* !__ACUTE__GC_H__ */
