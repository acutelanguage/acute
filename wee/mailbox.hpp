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

#ifndef __ACUTE__MAILBOX_HPP__
#define __ACUTE__MAILBOX_HPP__

#include "Message.hpp"

namespace Acute
{
	// Mailbox is a lock-free queue safe for up to two concurrent threads manipulating
	// it so long as one is delivering and the other is receiving. So we still need a mutex
	// protecting the access, but fine grained: one for reading, one for writing. The way I
	// plan on doing this is to use the state machine, each state in a different thread. I
	// protect transitions via mutexes. So long as the states are constructed carefully, so
	// all writing is done in one, all reading in another, then we should be fine.
	class Mailbox
	{
	private:
		struct Node {
			Node(Message* msg, Object* obj = nullptr /* XXX: should be some reasonably default */) : message(msg), sender(obj), next(nullptr) {}
			Message* message;
			Object*  sender;
			Node* next;
		};
		Node* first;
		Node* divider;
		Node* last;

		void trim_to(Node* upto)
		{
			while(first != upto)
			{
				Node* tmp = first;
				if(tmp != nullptr)
				{
					first = tmp->next;
					delete tmp;
				}
			}
		}

	public:
		Mailbox()
		{
			first = divider = last = nullptr;
		}

		~Mailbox()
		{
			trim_to(nullptr);
		}

		void deliver(Message* msg, Object* sender)
		{
			last->next = new Node(msg, sender);
			trim_to(divider);
		}

		bool receive(Message* result)
		{
			if(first == last)
				first = divider = last = new Node(result);

			if(divider != last)
			{
				result = divider->next->message;
				return true;
			}
			return false;
		}
	};
}

#endif /* !__ACUTE__MAILBOX_HPP__ */
