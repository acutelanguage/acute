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
 * File: closure.h
 * Description: Executable chunks of code.
 ******************************************************************/

#ifndef __ACUTE__CLOSURE_H__
#define __ACUTE__CLOSURE_H__

#include "object.h"

typedef struct closure_env_s
{
	obj_t* receiver;
	obj_t* locals;
	void* message;
	void* activation;
} closure_env_t;

typedef obj_t* (*obj_closure_t)(closure_env_t*);

typedef struct closure_s
{
	unsigned       object_size:28; // How big is our object? (Max size: 256 MB)
	unsigned       reserved:3;
	unsigned       marked:1;       // Has this object been marked by the GC?
	obj_closure_t  call;
	closure_env_t* env;
} closure_t;

/* Creates a closure. Must supply a pointer to the function to be executed. Must set the env field of the closure_t returned before
   it's called, on the caller side. This should be the environment you're capturing. Not elegant, but to require clang as a compiler is
   probably not a wise move for some people, as much as I'd like to. So since we have no portable closures baked into C, we have to
   build our own. */
extern closure_t* closure_new(obj_closure_t);

#endif /* !__ACUTE__CLOSURE_H__ */
