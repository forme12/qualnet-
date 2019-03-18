/*  Copyright (c) October 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
 *
 *  This is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version. 
 * 
 *	This file is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this file; if not, write to the Free Software
 *	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef PT_ATOMIC_NUMBER_H
#define PT_ATOMIC_NUMBER_H
#include <portablethreads/config.h>

#ifdef _MSC_VER
// This warning also appears when a 32-bit pointer gets cast into a 4 byte int
#	pragma warning(disable:4311) // A 64-bit pointer was truncated to a 32-bit int or 32-bit long.
// This warning also appears when a 32-bit int gets cast into a 32-bit pointer
#	pragma warning(disable:4312) // You attempted to assign a 32-bit value to a 64-bit integer. For example, casting a 32-bit int or 32-bit long to a 64-bit pointer
#endif

namespace PortableThreads 
{
	namespace LockFree
	{
		/*! \class PTAtomicNumber atomic_number.h portablethreads/lock_free.h
			\brief Atomic arithmetic operations on integers.

			PTAtomicNumber provides atomic (read thread-safe) lock-free
			manipulation of an integer value, the number. Atomic numbers
			are useful e.g. to keep track of how many threads have passed
			some piece of code or reference counting.

			Different from other number-representing classes PTAtomicNumber's 
			methods DO NOT return a reference to the object but the NEW VALUE
			of the counter after the invokation of a method. Because a
			PTAtomicNumber object may be used in the context of multiple threads,
			the value of the counter may already have changed by the time a thread
			examines the content of the counter via PTAtomicNumber::get().

			Unless otherwise stated all methods manipulate a PTAtomicNumber object
			atomically. Exceptions are constructors, assignment operators and the 
			destructor.


		*/
		class PTAtomicNumber : private ArchSpecific::PTAtomicNumber
		{
			typedef ArchSpecific::PTAtomicNumber Base;
		public:
			typedef Base::int_type int_type; //!< The integer type depends on the architecture but is as big as the native word of the platform (32-bit -> 4 byte, 64-bit -> 8 byte).
			//! Constructs an object with an initial value of v.
			PTAtomicNumber(int_type v = 0)
				:	Base(v)
			{}
			//! Decrement by \a value.
			inline int_type dec(int_type value = 1)
			{
				return Base::dec(value);
			}
			//! Increment by \a value.
			inline int_type inc(int_type value = 1)
			{
				return Base::inc(value);
			}
			
			//! Decrement by \a value.
			inline int_type operator-=(int_type value)
			{
				return dec(value);
			}
			//! Increment by \a value.
			inline int_type operator+=(int_type value)
			{
				return inc(value);
			}
			//! Decrement by 1 return, new value.
			inline int_type operator--()
			{
				return dec();
			}
			//! Decrement by 1 return, old value.
			inline int_type operator--(int)
			{
				return dec() + 1;
			}
			//! Increment by 1 return, new value.
			inline int_type operator++()
			{
				return inc();
			}
			//! Increment by 1 return, old value.
			inline int_type operator++(int)
			{
				return inc() - 1;
			}
			/*! \brief Get the value of the number.

				This method is NOT atomic.
			*/
			inline int_type get() const
			{
				return Base::get();
			}
			/*! \brief Atomically compare-and-swap the content of the number.

				Provided the old value of the number is know (\a oldValue)
				the method atomically sets the number to the value of
				parameter \a newValue.

				\retval true Exchange happened.
				\retval false Exchange didn't happen.
			*/
			inline bool cas(int_type newValue, int_type oldValue)
			{
				return Base::cas(newValue, oldValue);
			}
			/*! \brief Assign a value to the number.

				Even though the actual assigning takes place
				atomically, because a reference is returned
				the state of the object may have changed by
				the time a call to e.g. get() is executed.				
			*/
			inline PTAtomicNumber& operator=(int_type v)
			{
				Base::operator=(v);
				return *this;
			}
		};
	}
}

#ifdef _MSC_VER
#	pragma warning(disable:4311)
#	pragma warning(disable:4312)
#endif

#endif

