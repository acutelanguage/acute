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

#ifndef __ACUTE__MESSAGE_HPP__
#define __ACUTE__MESSAGE_HPP__

#include <vector>
#include <string>
#include "object.hpp"

namespace Acute
{
	class Message : public Object
	{
	public:
		Message() : name(""), arguments() {}
		Message(const std::string& n, std::vector<Message*> args) : name(n), arguments(args) {}

		Object* perform_on(Object*, Object*);

		virtual const std::string object_name();

		std::string get_name() { return name; }
		std::vector<Message*> get_arguments() { return arguments; }

		Object* object_at_arg(int, Object*);

		void set_next(Message* m) { next = m; }

	private:
		std::string           name;
		std::vector<Message*> arguments;

	public:
		Message*              next;
	};
}

#endif /* !__ACUTE__MESSAGE_HPP__ */
