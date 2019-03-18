/*  Copyright (c) January 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)

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



#ifndef X86_32_WIN32_MSVC_H

#define X86_32_WIN32_MSVC_H



#ifndef _MSC_VER

#   error "You must use a MS compatible compiler in order to use this header file!"

#endif



#define PT_HAVE_CAS

#define PT_HAVE_CAS2





#include <windows.h>



#if _MSC_VER <= 1200

#   pragma warning(disable:4786) //'identifier' : identifier was truncated to 'number' characters in the debug information

#   pragma warning(disable:4503)

#endif



namespace PortableThreads 

{

    typedef signed char int8;

    typedef unsigned char uint8;

    typedef short int16;

    typedef unsigned short uint16;

    typedef int int32;

    typedef unsigned int uint32;

    typedef __int64 int64;

    typedef unsigned __int64 uint64;



    namespace LockFree

    {

        namespace Private

        {



// Windows 98 and their ilk have different prototypes for 

// Interlocked-functions, compensate

#if _MSC_VER <= 1200



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



#else // _MSC_VER > 1200



            inline int32 pt_atomic_add(volatile int32* counter, int32 value)

            {

                return static_cast<int32>(

                    InterlockedExchangeAdd(

                        reinterpret_cast<volatile LONG*>(counter), 

                        static_cast<LONG>(value))

                ) + value;

            }

            /*

            * Atomic compare and exchange.  Compare OLD with MEM, if identical,

            * store NEW in MEM.  Return the initial value in MEM. Returns true if the

            * contents have been replaced.

            */

            inline int32 pt_atomic_cas_return_memory(volatile int32* inMemory, int32 newValue, int32 oldValue)

            {

                return static_cast<int32>(

                    InterlockedCompareExchange(

                        reinterpret_cast<volatile LONG*>(inMemory), 

                        static_cast<LONG>(newValue),

                        static_cast<LONG>(oldValue)

                    )

                );

            }

            inline int32 pt_atomic_set(volatile int32* inMemory, int32 newValue)

            {

                return static_cast<int32>(

                    InterlockedExchange(

                        reinterpret_cast<volatile LONG*>(inMemory), 

                        static_cast<LONG>(newValue)

                    )

                );

            }



#endif



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





            /*

                If you use the __fastcall calling convention, the compiler passes function arguments in 

                registers instead of on the stack. This can create problems in functions with __asm blocks 

                because a function has no way to tell which parameter is in which register. If the function 

                happens to receive a parameter in EAX and immediately stores something else in EAX, the original

                parameter is lost. In addition, you must preserve the ECX register in any function declared 

                with __fastcall.



                To avoid such register conflicts, don't use the __fastcall convention for functions that 

                contain an __asm block. If you specify the __fastcall convention globally with the /Gr compiler

                option, declare every function containing an __asm block with __cdecl or __stdcall. 

                (The __cdecl attribute tells the compiler to use the C calling convention for that function.) 

                If you are not compiling with /Gr, avoid declaring the function with the __fastcall attribute.



                When using __asm to write assembly language in C/C++ functions, you don't need

                to preserve the EAX, EBX, ECX, EDX, ESI, or EDI registers.

                (from MSDN)

            */



            inline void __cdecl pt_atomic_clear_lock(volatile uint8* mem)

            {

                __asm

                {

                    mov edx, mem

                    mov cl, 0

                    xchg [edx], cl

                }

            }



            inline bool __cdecl pt_atomic_set_lock(volatile uint8* mem)

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



            inline void __cdecl pt_barrier()

            {

                __asm

                {

                    pause

                }

            }



            inline bool __cdecl pt_atomic_cas(volatile int64* inMemory, int64 nv, int64 ov)

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

                    mov eax, oldLow

                    mov edx, oldHigh

                    mov ebx, newLow

                    mov ecx, newHigh

                    mov edi, inMemory

                    lock cmpxchg8b [edi]

                    sete yes

                }

                return yes;

            }



            inline uint64 __cdecl pt_ticks()

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



            const unsigned HARDWARE_POINTER_BITS = 32;

            const unsigned ALIGNMENT_BITS = 2;

        }

    }

    inline uint64 pt_seed()

    {

        return LockFree::Private::pt_ticks();

    }



}



#include <portablethreads/arch/arch-common.h>

#include <portablethreads/arch/native-pointer-cas.h>

#include <portablethreads/arch/native-atomic-number.h>

#include <portablethreads/arch/free-high-bits-muxer.h>



namespace PortableThreads

{

    namespace LockFree

    {

        namespace Private

        {

            template<unsigned USER_BITS>

            struct PTPointerCASType

            {

                typedef PointerCAS< FreeHighBitsMuxer<int64, 64, HARDWARE_POINTER_BITS, ALIGNMENT_BITS, USER_BITS> > PTPointerCAS;

            };



            typedef PTPointerCASType<0>::PTPointerCAS PTPointerCAS;

        }

        

    }

}



#endif

