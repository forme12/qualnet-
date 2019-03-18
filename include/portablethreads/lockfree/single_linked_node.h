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



#ifndef PT_SINGLE_LINKED_NODE_H

#define PT_SINGLE_LINKED_NODE_H

#include <portablethreads/config.h>

#include <portablethreads/lockfree/utility.h>

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

            // The pod check is actually needed for the queue implementation!

            template<typename T, typename CASType = PTPointerCAS>

            class SingleLinkedNode

            {

                typedef CASType PTPointerCAS;

                typedef typename PTPointerCAS::token_t token_t;

                typedef typename PTPointerCAS::int_t int_t;

            public:

                typedef T value_type;

                typedef T& reference;

                typedef const T& const_reference;

                ~SingleLinkedNode()

                {

                    destruct();

                }

                explicit

                SingleLinkedNode(bool pod = false)

                    :   next_(0)

                    ,   pod_(pod)

                {

                    raw_[sizeof(T)] = 0;

                }

                inline void construct(const T& t) volatile

                {

                    if(!pod_)

                    {

                        assert(!hasObject());

                    }

                    const_cast<PTPointerCAS&>(next_).assign(0, 0);

                    new (const_cast<char*>(raw_)) T(t);

                    if(!pod_)

                    {

                        raw_[sizeof(T)] = 1;

                    }

                    

                }

                inline void destruct() volatile

                {

                    // make sure only one thread ever destroys the object

                    if(!pod_)

                    {

                        if(hasObject())

                        {

                            raw_[sizeof(T)] = 0;

                            reinterpret_cast<T*>(const_cast<char*>(raw_))->~T();

                        }

                    }

                }

                inline const_reference data() volatile const 

                {

                    if(!pod_)

                    {

                        assert(hasObject());

                    }

                    return *reinterpret_cast<const T*>(const_cast<char*>(raw_));

                }

                inline reference data() volatile 

                {

                    if(!pod_)

                    {

                        assert(hasObject());

                    }

                    return *reinterpret_cast<T*>(const_cast<char*>(raw_));

                }

                inline token_t next() const { return next_.get(); }

                inline void next(int_t p, int_t u = 0) { next_.assign(p, u); }

                inline void next(const token_t& t) { next_.assign(t.pointer(), t.userbits()); }

                inline bool hasObject() const volatile { return raw_[sizeof(T)] != 0; }

                inline bool cas(token_t& nv, const token_t& ov)

                {

                    return next_.cas(nv, ov);

                }

                inline bool pod() const { return pod_; }

            private:

                SingleLinkedNode(const SingleLinkedNode&);

                SingleLinkedNode& operator=(const SingleLinkedNode&);

            private:    

                volatile char raw_[sizeof(T)+1];

                PTPointerCAS next_;

                const bool pod_;

            };

/*

            template<typename T, typename A, typename CASType>

            inline SingleLinkedNode<T>* recycleSingleLinkedNode(A& allocator, CASType& head, bool pod)

            {

                typedef SingleLinkedNode<T> Node;



                Node* node = 0; // b/c -Wall (uninit)

                if(stackPop(head, node))

                {

                    assert(node);

                    return node;

                }

                assert(sizeof(typename A::value_type) == sizeof(Node));

                return new (allocator.allocate(1, static_cast<void*>(0))) Node(pod);

            }



            template<typename CASType, typename T>

            inline void recycleSingleLinkedNode(CASType& head, SingleLinkedNode<T>* node)

            {

                stackPush(head, node);

            }

*/

        }

    }



}



#ifdef _MSC_VER

#   pragma warning(disable:4311)

#   pragma warning(disable:4312)

#endif



#endif



