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

#include "object_space.hpp"
#include "object.hpp"
#include "array.hpp"
#include "integer.hpp"
#include "message.hpp"
#include "nil.hpp"
#include "string.hpp"

namespace Acute
{
	ObjectSpace::ObjectSpace()
	{
		Object* lobby = new Object();
		lobby->add_slot("ObjectSpace", static_cast<Object*>(this));
		add_slot("Lobby", lobby);

		add_slot("Object", new Object());
		add_slot("Array", static_cast<Object*>(new Array()));
		add_slot("Integer", static_cast<Object*>(new Integer(0)));
		add_slot("Message", static_cast<Object*>(new Message()));
		add_slot("nil", static_cast<Object*>(Nil::instance()));
		add_slot("String", static_cast<Object*>(new String("")));
	}

	const std::string ObjectSpace::object_name()
	{
		return "ObjectSpace";
	}
}
