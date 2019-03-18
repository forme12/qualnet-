/*  Copyright (c) March 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
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
 
#ifndef SPINLOCK_ATOMIC_NUMBER_H
#define SPINLOCK_ATOMIC_NUMBER_H

#ifndef PT_ARCH_COMMON_INCLUDED
#	error "Architectural types and functions must be included first!"
#endif 

namespace PortableThreads 
{
	namespace LockFree
	{
		namespace ArchSpecific
		{
			class PTAtomicNumber
			{
			public:
				typedef pt_int_type int_type;
				// Constructs an object with an initial value of v
				PTAtomicNumber(int_type v = 0)
					:	value_(v)
					,	lock_(0)
				{}
				inline int_type dec(int_type value = 1)
				{
					lock();
					value_ -= value;
					const int_type ret = value_;
					pt_barrier();
					unlock();
					return ret;
				}
				inline int_type inc(int_type value = 1)
				{
					lock();
					value_ += value;
					const int_type ret = value_;
					pt_barrier();
					unlock();
					return ret;
				}
				inline int_type get() const
				{
					return value_;
				}
				inline bool cas(int_type newValue, int_type oldValue)
				{
					bool ret = false;
					lock();
					if(value_ == oldValue)
					{
						value_ = newValue;
						pt_barrier();
						ret = true;
					}
					unlock();
					return ret;
				}
				inline PTAtomicNumber& operator=(int_type v)
				{
					pt_atomic_set(&value_, v);
					return *this;
				}
			private:
				PTAtomicNumber(const PTAtomicNumber&);
				PTAtomicNumber& operator=(const PTAtomicNumber&);
				inline void lock()
				{
					bool done;
					do { done = pt_atomic_set_lock(&lock_); } while(!done);
				}
				inline void unlock()
				{
					pt_atomic_clear_lock(&lock_);
				}
			private:
				volatile int_type value_;
				volatile uint8 lock_;
			};
		}
	}
}

#endif
