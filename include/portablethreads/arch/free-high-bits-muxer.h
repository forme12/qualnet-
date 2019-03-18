/*  Copyright (c) October 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
 *
 *  This is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version. 
 * 
 *  This file is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this file; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef FREE_HIGH_BITS_MUXER_H
#define FREE_HIGH_BITS_MUXER_H

#include <cassert>

#ifdef _MSC_VER
#   pragma warning(disable:4305) //'specialization': Verkürzung von 'unsigned int' in 'bool'
#endif 

namespace PortableThreads 
{
    namespace LockFree
    {
        namespace Private
        {
            // The assumption is a pointer is POINTER_BITS bits wide. The
            // pointer doesn't use all bits to represent a datum in memory
            // but only HARDWARE_POINTER_BITS bits. The other bits are 
            // either all zero or all ones:
            // [0...0|pointer] or [1...1|pointer]
            //
            // The muxing code multiplexes a (POINTER_BITS-HARDWARE_POINTER_BITS-USER_BITS-1)
            // bit counter into the highest bits of the pointer while preserving
            // the most significant bit to indicate whether the original pointer
            // had its high bits set to all ones or all zeros.
            //
            // We assume the least ALIGNMENT_BITS of the pointer are zero because data
            // is aligned in memory.
            template<
                typename T, 
                unsigned POINTER_BITS, 
                unsigned HARDWARE_POINTER_BITS, 
                unsigned ALIGNMENT_BITS = 0, 
                unsigned USER_BITS = 0
            >
            struct FreeHighBitsMuxer
            {
                enum PrivateConstants
                { 
                    USE_HALF_INDICATOR = (POINTER_BITS > HARDWARE_POINTER_BITS),
                    COUNTER_BITS = (USE_HALF_INDICATOR ? POINTER_BITS - HARDWARE_POINTER_BITS - 1 + ALIGNMENT_BITS - USER_BITS : POINTER_BITS - HARDWARE_POINTER_BITS + ALIGNMENT_BITS - USER_BITS)
                };
                typedef T int_t;
                typedef PortableThreads::LockFree::Private::token_t<COUNTER_BITS, USER_BITS> token_t;
                static inline int_t multiplex(int_t pointer, int_t count, int_t userbits)
                {
                    // create: [half-indicator bit|pointer|count|userbits] if USE_HALF_INDICATOR
                    // otherwise create [pointer|count|userbits]
                    assert(HARDWARE_POINTER_BITS <= POINTER_BITS);
                    assert(sizeof(T)*8 >= HARDWARE_POINTER_BITS - ALIGNMENT_BITS + USE_HALF_INDICATOR + USER_BITS + COUNTER_BITS);
                    assert(ALIGNMENT_BITS < HARDWARE_POINTER_BITS);

                    // Make sure the aligment assumption holds
                    assert((pointer & ALIGNMENT_MASK) == 0);

                    // Make sure user bits don't occupy anything else
                    assert((userbits & ~USER_BITS_MASK) == 0);

                    // counter must fit in reserved bits
                    count %= COUNTER_MAX;

                    if(COUNTER_BITS && USER_BITS) // compiler optimizes this away...
                        count <<= USER_BITS;

                    // move pointer to high region
                    pointer <<= POINTER_SHIFT;
                    assert((pointer & (COUNTER_MASK << USER_BITS)) == 0);
                    assert((pointer & USER_BITS_MASK) == 0);

                    if(USER_BITS)
                        pointer ^= userbits;

                    if(COUNTER_BITS) // compiler optimizes this away...
                        pointer ^= count;

                    return pointer;
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
                    // [[half-indicator bit]|pointer|count|userbits] -> [[1..1]|pointer] or [[0..0]|pointer]
                    return  (mux & MSB_MASK) == 0 
                            ? ((mux >> POINTER_SHIFT) & ~ALIGNMENT_MASK) & POINTER_MASK
                            // has to be a true or (|) because the half-indicator bit is still there!
                            : ((mux >> POINTER_SHIFT) & ~ALIGNMENT_MASK) | HIGH_ONES;
                }
                static inline int_t freeBits()
                {
                    return POINTER_BITS - HARDWARE_POINTER_BITS;
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
                static const int_t MSB_MASK = static_cast<int_t>(1) << (POINTER_BITS - 1);
                static const int_t COUNTER_MAX = static_cast<int_t>(1) << COUNTER_BITS;
                static const int_t COUNTER_MASK = COUNTER_MAX - 1;
                static const int_t USER_BITS_MASK = (static_cast<int_t>(1) << USER_BITS) - 1;
                static const int_t ALIGNMENT_MASK = (static_cast<int_t>(1) << ALIGNMENT_BITS) - 1;
                // only shift in case the hardware pointer is smaller than the virtual pointer
                static const int_t POINTER_SHIFT = USE_HALF_INDICATOR ? POINTER_BITS - HARDWARE_POINTER_BITS - 1 : POINTER_BITS - HARDWARE_POINTER_BITS;
                static const int_t HIGH_ONES = ((static_cast<int_t>(1) << (POINTER_BITS - HARDWARE_POINTER_BITS + 1)) - 1) << HARDWARE_POINTER_BITS;
                // mask of high bits, only nessary if hardware pointer is smaller than virtual pointer
                static const int_t POINTER_MASK = (static_cast<int_t>(1) << HARDWARE_POINTER_BITS) - 1;
            };
        }
    }
}

#ifdef _MSC_VER
#   pragma warning(default:4305)
#endif

#endif
