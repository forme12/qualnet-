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



#ifndef PT_LOCK_FREE_UTILITY_H

#define PT_LOCK_FREE_UTILITY_H

#include <portablethreads/config.h>

#include <cassert>



#ifdef _MSC_VER

// This warning also appears when a 32-bit pointer gets cast into a 4 byte int

#   pragma warning(disable:4311) // A 64-bit pointer was truncated to a 32-bit int or 32-bit long.

// This warning also appears when a 32-bit int gets cast into a 32-bit pointer

#   pragma warning(disable:4312) // You attempted to assign a 32-bit value to a 64-bit integer. For example, casting a 32-bit int or 32-bit long to a 64-bit pointer

#endif



namespace PortableThreads 

{

    namespace LockFree

    {

        namespace Private

        {

            template<typename CASType, typename T>

            inline void stackPush(CASType& head, T* node)

            {

                assert(sizeof(T*) <= sizeof(typename CASType::int_t));



                typename CASType::token_t ov, nv;

                do

                {

                    ov = head.get();

                    node->next(ov);

                    nv.pointer(reinterpret_cast<typename CASType::int_t>(node));

                    nv.userbits(ov.userbits());

                }

                while(!head.cas(nv, ov));

            }



            // NOTE: this function does NOT guarantee that node has

            // the same value as before when returning unsuccessfully!

            template<typename CASType, typename T>

            inline bool stackPop(CASType& head, T*& node)

            {

                assert(sizeof(T*) <= sizeof(typename CASType::int_t));



                for(typename CASType::token_t h = head.get(), nh; h.pointer(); h = head.get())

                {

                    nh = reinterpret_cast<T*>(h.pointer())->next();

                    if(head.cas(nh, h))

                    {

                        node = reinterpret_cast<T*>(h.pointer());

                        return true;

                    }

                }

                return false;

            }

        }

    }



}



#ifdef _MSC_VER

#   pragma warning(disable:4311)

#   pragma warning(disable:4312)

#endif



#endif



