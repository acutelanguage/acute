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

#include <stdio.h>
#include <stdlib.h>
#include "object.h"
#include "closure.h"

/* Closures are implemented naively for two reasons. First, this is a bootstrap and it'll be thrown away once our full VM is able to
   compile itself. Secondly, because more and more platforms are adding W^X protection (including my own dev platform), mucking about
   with building closures using thunks didn't suit my fancy. */

closure_t* closure_new(obj_closure_t func)
{
	closure_t* closure = malloc(sizeof(*closure));
	closure->call = func;
	return closure;
}
