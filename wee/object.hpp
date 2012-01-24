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

#ifndef __ACUTE__OBJECT_HPP__
#define __ACUTE__OBJECT_HPP__

#include <map>
#include <string>
#include <vector>
#include "gc.hpp"

namespace Acute
{
	class Object;
	class Message;
	class Mailbox;

	typedef std::map<std::string, Object*> SlotTable;

	class Object : public GCObject
	{
	private:
		// First three fields are for the GC.
		Object*              next;
		Object*              prev;
		unsigned int         colour:2;

		bool                 activatable;

		// The slot table is our local container to hold slot definitions.
		SlotTable            slots;

		// Our traits list contains other composable objects of behaviour and state.
		std::vector<Object*> traits;

	public:
		Object();
		~Object();

		void* operator new(size_t size, GarbageCollector& gc = *collector)
		{
			MemoryAddress addr(malloc(size));
			GCMarker* marker = gc.new_marker();
			marker->object = addr.as<GCObject>();
			gc.add_value(marker);
			return addr.as<Object>();
		}

		void add_slot(const std::string&, Object*);
		void remove_slot(const std::string&);
		void add_trait(Object*);

		// Receives a message. Messages dispatched to this object should already be
		// in the queue. Therefore, this method pops an item off, and begins the
		// lookup process.
		void receive(Object*);

		// Look up a slot
		bool local_lookup(const std::string, Object*&, Object*&);
		Object* lookup(const std::string, Object*&);
		Object* perform(Object*, Message*);
		Object* forward(Object*, Message*);
		Object* activate(Object*, Object*, Message*, Object*);
		bool is_activatable() { return activatable; }
		void set_activatable(bool value) { activatable = value; }

		SlotTable& slot_table() { return slots; }

		virtual int compare(Object*);

		virtual void generic_object_walk();
		virtual void walk();

		// Our object name.
		virtual const std::string object_name();

		// The mailbox is where messages come into. This allows us to decouple
		// message sending and message receiving.
		Mailbox*             mailbox;

	private:
		bool implements(const std::string&, Object*& obj);
	};

	class SlotExistsError
	{
	private:
		std::string string;
		Object*     offender;

	public:
		SlotExistsError(std::string s, Object* obj) : string(s), offender(obj) {}
		const std::string message() const { return "Conflict: Slot '" + string + "' found on '" + offender->object_name() + "'"; }
	};
}

#endif /* !__ACUTE__OBJECT_HPP__ */
