/*  Copyright (c) January 2006 Jean Gressmann (jsg@rz.uni-potsdam.de)
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

#ifndef IA_64_MUXER_H
#define IA_64_MUXER_H

#include <cassert>

#ifdef _MSC_VER
#	pragma warning(disable:4305) //'specialization': Verkürzung von 'unsigned int' in 'bool'
#endif 

namespace PortableThreads 
{
	namespace LockFree
	{
		namespace Private
		{
			// Intel Itanium 2: 50 bit physical addresses
			// VA layout: [3 bit tag|extended from MSB|pointer data]
			template<
				typename T, 
				unsigned HARDWARE_POINTER_BITS, 
				unsigned ALIGNMENT_BITS = 0, 
				unsigned USER_BITS = 0
			>
			struct IA64Muxer
			{
				enum { TAG_BITS = 3, POINTER_BITS = sizeof(T)*8, COUNTER_BITS = POINTER_BITS - HARDWARE_POINTER_BITS + ALIGNMENT_BITS - USER_BITS };
				typedef T int_t;
				typedef token_t<COUNTER_BITS, USER_BITS> token_t;
				static inline int_t multiplex(int_t pointer, int_t count, int_t userbits)
				{
					// create: [tag|pointer|count|userbits]
					assert(HARDWARE_POINTER_BITS <= POINTER_BITS);
					assert(ALIGNMENT_BITS < HARDWARE_POINTER_BITS);
					assert(sizeof(T)*8 - HARDWARE_POINTER_BITS + ALIGNMENT_BITS - USER_BITS - COUNTER_BITS >= 0);						

					// Make sure the aligment assumption holds
					assert((pointer & ALIGNMENT_MASK) == 0);

					// Make sure user bits don't occupy anything else
					assert((userbits & ~USER_BITS_MASK) == 0);

					// counter must fit in reserved bits
					count %= COUNTER_MAX;

					if(USER_BITS) // compiler optimizes this away...
						count <<= USER_BITS;

					// move pointer to high region
					const int_t tag = pointer & TAG_MASK;
					pointer <<= POINTER_SHIFT;
					// clear tag bits for later muxing
					pointer &= ~TAG_MASK;
					assert((pointer & (COUNTER_MASK << USER_BITS)) == 0);
					assert((pointer & USER_BITS_MASK) == 0);

					if(USER_BITS)
						pointer ^= userbits;

					if(COUNTER_BITS) // compiler optimizes this away...
						pointer ^= count;

					return pointer ^ tag;
				}
				static inline int_t userbits(int_t mux)
				{
					return mux & USER_BITS_MASK;
				}
				static inline int_t count(int_t mux)
				{
					return (mux >> USER_BITS) & COUNTER_MASK;
				}
				static inline int_t pointer(int_t mux)
				{
					// [tag|pointer|count|userbits] -> [tag|pointer msb spread|pointer]
					const int_t tag = mux & TAG_MASK;
					const bool zerofill = (mux & MSB_MASK) == 0;
					// shift out userbits and counter to avoid remainder of counter 
					// be in pointer if just POINTER_SHIFT is used
					mux >>= POINTER_SHIFT;
					// clear high bits now so we later on don't have shifted in trash in them
					mux &= POINTER_MASK;
					// clear alignment bits
					mux &= ~ALIGNMENT_MASK;
					// fill unimplemented bits according to zerofill bit
					if(!zerofill)
						mux ^= HIGH_ONES;

					return mux ^ tag;
				}
				static inline int_t freeBits()
				{
					return POINTER_BITS - HARDWARE_POINTER_BITS;
				}
				static inline int_t tagBits()
				{
					return TAG_BITS;
				}
				static inline int_t counterBits()
				{
					return COUNTER_BITS;
				}
				static inline int_t pointerBits()
				{
					return POINTER_BITS;
				}
				static inline int_t hardwarePointerBits()
				{
					return HARDWARE_POINTER_BITS;
				}
				static inline int_t alignmentBits()
				{
					return ALIGNMENT_BITS;
				}
				static inline int_t userBits()
				{
					return USER_BITS;
				}
			private:
				static const int_t TAG_MASK = ((static_cast<int_t>(1) << TAG_BITS) - 1) << (POINTER_BITS - TAG_BITS);
				static const int_t MSB_MASK = static_cast<int_t>(1) << (POINTER_BITS - 1 - TAG_BITS);
				static const int_t COUNTER_MAX = static_cast<int_t>(1) << COUNTER_BITS;
				static const int_t COUNTER_MASK = COUNTER_MAX - 1;
				static const int_t USER_BITS_MASK = (static_cast<int_t>(1) << USER_BITS) - 1;
				static const int_t ALIGNMENT_MASK = (static_cast<int_t>(1) << ALIGNMENT_BITS) - 1;
				// only shift in case the hardware pointer is smaller than the virtual pointer
				static const int_t POINTER_SHIFT = POINTER_BITS - HARDWARE_POINTER_BITS;
				static const int_t HIGH_ONES = ((static_cast<int_t>(1) << (POINTER_BITS - HARDWARE_POINTER_BITS)) - 1) << (HARDWARE_POINTER_BITS - TAG_BITS);
				// mask of high bits, only nessary if hardware pointer is smaller than virtual pointer
				static const int_t POINTER_MASK = (static_cast<int_t>(1) << (HARDWARE_POINTER_BITS-TAG_BITS)) - 1;
			};
		}
	}
}


#endif
