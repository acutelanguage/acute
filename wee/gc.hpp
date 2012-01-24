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

#ifndef __ACUTE__GC_HPP__
#define __ACUTE__GC_HPP__

#include <vector>
#include <stdint.h>
#include "address.hpp"

#define GC_MAX_FREE_OBJECTS 8192

namespace Acute
{
	class GCObject;
	class GarbageCollector;

	extern GarbageCollector* collector;

	enum GCColours
	{
		kGCColourBlack = 0,
		kGCColourGrey,
		kGCColourWhite,
		kGCColourFreed
	};

	struct GCMarker
	{
		GCMarker* next;
		GCMarker* prev;
		uint8_t   colour:2;
		uint8_t   reserved:6;
		GCObject* object;

		GCMarker(unsigned int c = kGCColourFreed) : colour(c) {}

		inline size_t count_in_set()
		{
			size_t count = 0;
			GCMarker* v = next;
			unsigned int c = colour;

			while(v->colour == c)
			{
				GCMarker*& n = v->next;
				v = n;
				++count;
			}

			return count;
		}

		inline bool set_is_empty()
		{
			return colour != next->colour;
		}

		inline void insert_after(GCMarker* other)
		{
			colour = other->colour;
			prev = other;
			next = other->next;
			other->next->prev = this;
			other->next= this;
		}

		inline void insert_before(GCMarker* other)
		{
			colour = other->colour;
			prev = other->prev;
			next = other;
			other->prev->next = this;
			other->prev = this;
		}

		inline void remove()
		{
			prev->next = next;
			next->prev = prev;
		}

		inline void resnap_after(GCMarker* other)
		{
			if(prev)
				remove();
			insert_after(other);
		}

		inline void loop()
		{
			prev = this;
			next = this;
		}

		inline bool is_empty()
		{
			return colour != next->colour;
		}

		virtual void walk() { }
	};

	class GarbageCollector
	{
	public:
		GarbageCollector();
		~GarbageCollector();

		GCMarker* new_marker();
		void add_value(GCMarker*);

		void scan_greys(size_t max = INT_MAX);
		size_t free_whites();
		void sweep();

		void make_grey(GCMarker* other)
		{
			other->resnap_after(greys);
		}

		void make_white(GCMarker* other)
		{
			other->resnap_after(whites);
		}

		void make_black(GCMarker* other)
		{
			other->resnap_after(blacks);
		}

		void make_freed(GCMarker* other)
		{
			other->resnap_after(freed);
		}

		void shade(GCMarker* marker)
		{
			if(marker->colour == kGCColourWhite)
				make_grey(marker);
		}

	private:
		GCMarker* blacks;
		GCMarker* greys;
		GCMarker* whites;
		GCMarker* freed;
		size_t    allocated;
		size_t    marks_alloc;
		size_t    marks_queued;
	};

	class GCObject : public GCMarker
	{
	public:
		GCMarker marker;

		void* operator new(size_t size, GarbageCollector& gc = *collector)
		{
			MemoryAddress addr(malloc(size));
			GCMarker* marker = gc.new_marker();
			marker->object = addr.as<GCObject>();
			gc.add_value(marker);
			return addr.as<GCObject>();
		}

		void operator delete(void* ptr)
		{
		}

		virtual void walk();
	};
}

#endif /* !__ACUTE__GC_HPP__ */
