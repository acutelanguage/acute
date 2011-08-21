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

#include <sys/types.h>
#include "list.h"

typedef struct gcable_s
{
	unsigned object_size:28; // How big is our object? (Max size: 256 MB)
	unsigned reserved:3;
	unsigned marked:1;       // Has this object been marked by the GC?
	void*    slots;          // Backed by a Judy array, see judy64d.c
} gcable_t;

typedef struct gc_s gc_t;

/* Initialize the garbage collector. Must pass in a maximum heap size as an argument. */
extern void gc_initialize(size_t);
/* Destroy the garbage collector. Once this function is called, the GC is dead. It must be re-initialized. All bookkeeping info will be
   lost. */
extern void gc_destroy(void);

/* Allocate some memory and register the returned pointer with the GC. */
extern gcable_t* gc_alloc(size_t);

/* Forces a garbage collection. */
extern void gc_collect(void);

/* The GC can keep track of objects through this mechanism in addition to gc_alloc above. If you do not allocate an object through the
   gc_alloc function, then you must explicitly add its root via this function. Otherwise, the GC will know nothing about it. */
extern void gc_add_root(void*);

#endif /* !__ACUTE__GC_H__ */
