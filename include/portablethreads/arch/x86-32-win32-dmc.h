/*  Copyright (c) January 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
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

#ifndef X86_32_WIN32_DMC_H
#define X86_32_WIN32_DMC_H

#ifndef __DMC__
#	error "You must use a Digital Mars compiler in order to use this header file!"
#endif

#define PT_HAVE_CAS
#define PT_HAVE_CAS2

#include <windows.h>
#ifndef __GNUC__
#	define __GNUC__
#	define PT_DEFINED_GNUC
#endif
#include <portablethreads/arch/32-gcc.h>
#ifdef PT_DEFINED_GNUC
#	undef __GNUC__
#	undef PT_DEFINED_GNUC
#endif

namespace PortableThreads 
{
	namespace LockFree
	{
		namespace Private
		{
			inline int32 pt_atomic_add(volatile int32* counter, int32 value)
			{
				return static_cast<int32>(
					InterlockedExchangeAdd(
						(LPLONG)counter, 
						static_cast<LONG>(value)
				)) + value;
			}
			inline int32 pt_atomic_cas_return_memory(volatile int32* inMemory, int32 newValue, int32 oldValue)
			{
				return (int32)
					InterlockedCompareExchange(
						(PVOID*)inMemory, 
						(PVOID)newValue,
						(PVOID)oldValue);
			}
			inline int32 pt_atomic_set(volatile int32* inMemory, int32 newValue)
			{
				return static_cast<int32>(
					InterlockedExchange(
						(LPLONG)inMemory, 
						static_cast<LONG>(newValue)
					)
				);
			}


			inline int32 pt_atomic_sub(volatile int32* counter, int32 value)
			{
				return pt_atomic_add(counter, -value);
			}

			inline int32 pt_atomic_inc(volatile int32* counter)
			{
				return pt_atomic_add(counter, 1);
			}

			inline int32 pt_atomic_dec(volatile int32* counter)
			{
				return pt_atomic_add(counter, -1);
			}


			inline bool pt_atomic_cas(volatile int32* mem, int32 nv, int32 ov)
			{
				return pt_atomic_cas_return_memory(mem, nv, ov) == ov;
			}

			inline void pt_atomic_clear_lock(volatile uint8* mem)
			{
				__asm
				{
					mov edx, mem
					mov cl, 0
					xchg [edx], cl
				}
			}

			inline bool pt_atomic_set_lock(volatile uint8* mem)
			{
				bool yes;
				__asm
				{
					mov edx, mem
					mov al, 0
					mov cl, 1
					lock cmpxchg [edx], cl
					sete yes
				}
				return yes;
			}

			inline void pt_barrier()
			{
				__asm
				{
					nop
				}
			}

			inline bool pt_atomic_cas(volatile int64* mem, int64 nv, int64 ov)
			{
				// I REALLY have no idea why this doesn't work with an array, but it 
				// doesn't, so don't try to change the code to use an array!
				const int32 oldLow = static_cast<int32>(ov);
				const int32 oldHigh = static_cast<int32>(ov >> 32);
				const int32 newLow = static_cast<int32>(nv);
				const int32 newHigh = static_cast<int32>(nv >> 32);
				bool yes;
				
				__asm
				{
					push ebx
					push edi
					
					mov eax, oldLow
					mov edx, oldHigh
					mov ebx, newLow
					mov ecx, newHigh
					mov edi, mem
					lock cmpxch8b [edi]
					sete yes
					
					pop edi
					pop ebx
				}
				return yes;

			}

			inline uint64 pt_ticks()
			{
				uint32 buf[2];
				__asm 
				{
					rdtsc
					mov buf[0], eax
					mov buf[1], edx
				}
				return (static_cast<uint64>(buf[1]) << 32) | static_cast<uint64>(buf[0]); 
			}
		}
	}
	inline uint64 pt_seed()
	{
		return LockFree::Private::pt_ticks();
	}

}

#include <portablethreads/arch/arch-common.h>
#include <portablethreads/arch/native-dcas-pointer-cas.h>
#include <portablethreads/arch/native-atomic-number.h>
#include <portablethreads/arch/4-byte-pointer-to-int-compression.h>

namespace PortableThreads
{
	namespace LockFree
	{
		namespace Private
		{
			// assume 4 byte alignment
			template<typename T>
			inline int64 pt_inflate_pointer(T* p)
			{
				return pt_inflate_pointer<T, int64, 2>(p);
			}

			template<typename T>
			inline T* pt_deflate_pointer(int64 p)
			{
				return pt_deflate_pointer<T, int64, 2>(p);
			}
		}
	}
}

#endif
