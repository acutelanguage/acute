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

#ifndef __ACUTE__ADDRESS_HPP__
#define __ACUTE__ADDRESS_HPP__

namespace Acute
{
	struct MemoryAddress
	{
		uintptr_t address;

		MemoryAddress() : address(0) {}
		MemoryAddress(void* addr) : address(reinterpret_cast<uintptr_t>(addr)) {}

		operator void*()
		{
			return reinterpret_cast<void*>(address);
		}

		MemoryAddress operator+(uintptr_t i)
		{
			return MemoryAddress(reinterpret_cast<void*>(address + i));
		}

		MemoryAddress operator+=(uintptr_t i)
		{
			address += i;
			return *this;
		}

		uintptr_t operator-(MemoryAddress addr)
		{
			return address - addr.address;
		}

		MemoryAddress operator-(uintptr_t i)
		{
			return MemoryAddress(reinterpret_cast<void*>(address - i));
		}

		MemoryAddress operator&(uintptr_t mask)
		{
			return MemoryAddress(reinterpret_cast<void*>(address & mask));
		}

		bool is_null()
		{
			return address == 0;
		}

		static MemoryAddress null()
		{
			return MemoryAddress(0);
		}

		uintptr_t as_int()
		{
			return address;
		}

		template<typename T> T* as()
		{
			return reinterpret_cast<T*>(reinterpret_cast<void*>(address));
		}
	};
}

#endif /* !__ACUTE__ADDRESS_HPP__ */
