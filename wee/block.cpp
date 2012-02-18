/*
 * Acute Programming Language
 * Copyright (c) 2011, Jeremy Tregunna, All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <string>
#include "block.hpp"
#include "object.hpp"
#include "message.hpp"

namespace Acute
{
	template<class T>
	Block<T>::Block(Message* body, ArgNames argNames, Object* ctx) : message(body), argument_names(argNames), scope(ctx)
	{
	}

	template<class T>
	Object* Block<T>::activate(Object* target, Object* locals, Message* body, Object* slot_context)
	{
		Object* s = scope;
		my_locals = new Object();

		if(!s)
			s = target;

		for(int i = 0; i < argument_names.size(); i++)
		{
			Object* arg = message->object_at_arg(i, locals);
			my_locals->add_slot(argument_names.at(i));
		}
		my_locals->add_slot("self", s);

		return message->perform_on(my_locals, my_locals);
	}

	template<class T>
	Object* Block<T>::call(void)
	{
		if(this->builtin)
		{
			Object* locals = new Object(); // TODO: Need a real locals object
			return builtin(locals, new Message());
		}
		// TODO: call activate.
		return nullptr;
	}

	template<class T>
	void Block<T>::walk()
	{
		generic_object_walk();
		collector->shade(my_locals);
		if(scope)
			collector->shade(scope);
		collector->shade(message);
	}
}