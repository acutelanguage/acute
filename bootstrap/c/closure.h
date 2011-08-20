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

typedef struct closure_s* volatile closure_t;

typedef obj_t (*obj_closure_t)(closure_t);

struct closure_s
{
	obj_closure_t call;
	obj_t receiver;
	void* message;
	void* call;
};

/* Creates a closure. Must supply a pointer to the function to be executed. Not elegant, but to require clang as a compiler is probably
   not a wise move for some people, as much as I'd like to. So since we have no portable closures baked into C, we have to build our
   own. */
extern closure_t closure_new(obj_closure_t*);

#endif /* !__ACUTE__CLOSURE_H__ */
