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
#include <map>
#include <vector>
#include "object.hpp"
#include "mailbox.hpp"
#include "integer.hpp"

namespace Acute
{
	Object::Object() : mailbox(new Mailbox()), slots(), traits()
	{
		collector->add_value(dynamic_cast<GCMarker*>(this));
	}

	Object::~Object()
	{
		delete mailbox;
	}

	void Object::add_slot(const std::string& name, Object* value)
	{
		slots.insert(std::pair<std::string, Object*>(name, value));
	}

	void Object::remove_slot(const std::string& name)
	{
		// DISCUSS: Should this be greedy or non-greedy? I.e., currently, we perform this algorithm:
		//          1. Look in our current slot table for a name
		//          2. Remove it.
		//
		// What if the slot you want to remove is in a trait? Now we can't just go removing it from the
		// trait, what about other objects that depend on it? But, what we can do, is copy that trait,
		// install that copy in place of the original in this objects trait list only.
		//
		// Should we implement it this way?
		slots.erase(name);
	}

	void Object::add_trait(Object* trait)
	{
		for(auto p : trait->slot_table())
		{
			Object* result;

			if(implements(p.first, result))
				throw SlotExistsError(p.first, result);

			Object* val = p.second;
			if(val->object_name() == "Block")
				// Skip any non-Block objects
				add_slot(p.first, val);
		}
	}

	// We don't want any conflicts. Returns true if we already implement name.
	bool Object::implements(const std::string& name, Object*& obj)
	{
		return slots.find(name) == slots.end();
	}

	void Object::receive(Object* ctx)
	{
		Message* msg = nullptr;
		if(mailbox->receive(msg))
		{
			Object* slot_context;
			lookup(msg->get_name(), slot_context);
		}
	}

	bool Object::local_lookup(const std::string str, Object*& value, Object*& slot_context)
	{
		SlotTable::iterator it = slots.find(str);
		if(it != slots.end())
		{
			slot_context = this;
			value = it->second;
			return true;
		}
		return false;
	}

	Object* Object::lookup(const std::string str, Object*& slot_context)
	{
		Object* value = nullptr;

		local_lookup(str, value, slot_context);
		// TODO:
		// Invoke a catch-all "missing method" callback if available. This will allow our users to
		// implement their own inheritance model if they so choose, and better yet, isolate it to
		// their own library code.

		return value;
	}

	Object* Object::perform(Object* locals, Message* msg)
	{
		Object* context;
		Object* value = lookup(msg->get_name(), context);

		if(value)
			return value->activate(this, locals, msg, context);

		return forward(locals, msg);
	}

	Object* Object::activate(Object* target, Object* locals, Message* msg, Object* slot_context)
	{
		if(is_activatable())
		{
			Object* context;
			Object* value = lookup("activate", context);

			if(value)
				value->activate(target, locals, msg, context);
		}

		return this;
	}

	Object* Object::forward(Object* locals, Message* msg)
	{
		Object* context;
		Object* value = lookup("forward", context);

		if(value)
			value->activate(this, locals, msg, context);

		// XXX: Raise exception
		return this;
	}

	int Object::compare(Object* other)
	{
		if(this == other)
			return 0;
		else if(this > other)
			return 1;
		else
			return -1;
	}

	void Object::generic_object_walk()
	{
		for(auto v : slots)
			collector->shade(v.second);

		for(auto t : traits)
			collector->shade(t);
	}

	void Object::walk()
	{
		generic_object_walk();
	}

	const std::string Object::object_name()
	{
		return "Object";
	}
}
