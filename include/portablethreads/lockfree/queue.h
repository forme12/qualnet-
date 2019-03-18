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



#ifndef PT_LOCK_FREE_QUEUE_H

#define PT_LOCK_FREE_QUEUE_H

#include <portablethreads/config.h>

#include <portablethreads/lockfree/single_linked_node.h>

#include <portablethreads/lockfree/utility.h>

#include <cassert>

#include <iterator>

#include <memory> // std::allocator



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

            template<class T, class NodeType>

            class QueueIterator : public std::iterator<std::forward_iterator_tag, T>

            {

                typedef std::iterator<std::forward_iterator_tag, T> Base;

                typedef NodeType Node;

            public:

                typedef typename Base::pointer pointer;

                typedef typename Base::reference reference;

                operator const void*() const { return current_; }

                explicit

                QueueIterator(Node* node = 0)

                    :   current_(node)

                {}

                inline pointer operator->() const

                {

                    return &current_->data()->data();

                }

                inline reference operator*() const

                {

                    return current_->data()->data();

                }

                inline QueueIterator& operator++()

                {

                    if(current_)

                        current_ = reinterpret_cast<Node*>(current_->next().pointer());

                    return *this;

                }

                inline QueueIterator operator++(int)

                {

                    const QueueIterator temp(*this);

                    ++(*this);

                    return temp;

                }

            private:

                Node* current_;

            };

        }



        /*! \class PTQueue queue.h portablethreads/lock_free.h

            \brief  Lock-free queue with correct memory management.

        

            The queue implementation follows Michael and Scott's algorithm given

            in http://citeseer.ist.psu.edu/michael96simple.html



            Manipulation of the stack via the pushBack/popFront interface is thread-safe.



            When pushing an element into or removing an element from the queue,

            the element is ever copied or assigned within the context of 

            one thread.



            The queue allows for its content to be iterated over using

            forward iterators. The iterators may also be used to modify

            the content of the queue. Iterating over the queue yields

            undefined results if the stack is used concurrently.



            The allocator provided as template parameter must allow

            for concurrent allocation and deallocation. The STL allocator

            meets this requirement.



            Memory which is allocated by the queue implementation is

            freed on object destruction.    

        */

        template<typename T, typename A = std::allocator<T> >

        class PTQueue

        {

            typedef Private::SingleLinkedNode<T> TNode;

            typedef Private::SingleLinkedNode<TNode*> HelperNode;   

            struct Pair

            {

                Pair()

                    :   helperNode_(true)

                {}

                HelperNode helperNode_;

                TNode tnode_;

            };          

            typedef Private::PTPointerCAS PTPointerCAS;

            typedef PTPointerCAS::token_t token_t;

        public:

            typedef typename A::template rebind<Pair>::other allocator_type;

            typedef T value_type;

            typedef T& reference;

            typedef const T& const_reference;

            typedef Private::QueueIterator<T, HelperNode> iterator;

            typedef Private::QueueIterator<const T, HelperNode> const_iterator;

            ~PTQueue()

            {

                destroyStack(head_);

                destroyStack(helperNodeStore_);

            }

            //! Create an empty queue.

            explicit

            PTQueue()

                :   helperNodeStore_(0)

                ,   tnodeStore_(0)

                ,   head_(reinterpret_cast<PTPointerCAS::int_t>(recycleHelperNode()))

                ,   tail_(head_.get().pointer())

            {

                reinterpret_cast<HelperNode*>(head_.get().pointer())->construct(0);

            }

            //! Create an empty queue using the allocator provided.

            explicit

            PTQueue(A& allocator)

                :   allocator_(allocator)

                ,   helperNodeStore_(0)

                ,   tnodeStore_(0)

                ,   head_(reinterpret_cast<PTPointerCAS::int_t>(recycleHelperNode()))

                ,   tail_(head_.get().pointer())

                

            {

                reinterpret_cast<HelperNode*>(head_.get().pointer())->construct(0);

            }

            /*! \brief Add one element to the back of the queue.



                \param value Element to add.



                \retval An iterator to the element in the queue.

            */

            iterator pushBack(const_reference value)

            {

                TNode* tnode = recycleTNode();

                try

                {

                    tnode->construct(value);

                }

                catch(...)

                {

                    recycleTNode(tnode);

                    throw;

                }

                HelperNode* node = recycleHelperNode();

                node->construct(tnode);

                iterator it(node);

                

                token_t t, n, nt;

                while(true)

                {

                    t = tail_.get();

                    n = reinterpret_cast<HelperNode*>(t.pointer())->next();

                    if(t == tail_.get()) // diese Zeile ist wichtig!

                    {

                        if(n.pointer() == 0)

                        {

                            nt.pointer(reinterpret_cast<PTPointerCAS::int_t>(node));

                            if(reinterpret_cast<HelperNode*>(t.pointer())->cas(nt, n))

                                break;

                        }

                        else

                        {

                            tail_.cas(n, t);

                        }

                    }

                }

                nt.pointer(reinterpret_cast<PTPointerCAS::int_t>(node));

                tail_.cas(nt, t);



                return it;

            }



            /*! \brief Remove the element at the front of the queue.



                \param value Reference which is assigned the element removed from the queue.



                \retval true An element was removed from the queue.

                \retval false The queue was empty at the time.

            */

            bool popFront(reference value)

            {

                TNode* tnode = 0;

                token_t h, t, n;

                while(true)

                {

                    h = head_.get();

                    t = tail_.get();

                    n = reinterpret_cast<HelperNode*>(h.pointer())->next();

                    if(h == head_.get()) // diese Zeile ist wichtig!

                    {

                        if(h.pointer() == t.pointer())

                        {

                            if(n.pointer() == 0)

                                return false;

                            tail_.cas(n, t);

                        }

                        else

                        {

                            tnode = reinterpret_cast<HelperNode*>(n.pointer())->data();

                            if(head_.cas(n, h))

                                break;

                        }

                    }

                }

                

                recycleHelperNode(reinterpret_cast<HelperNode*>(h.pointer()));



                assert(tnode);

                value = tnode->data();

                tnode->destruct();

                recycleTNode(tnode);

                

                return true;

            }

            //! Get an iterator to the frontmost element.

            inline iterator begin() 

            { 

                return iterator(reinterpret_cast<HelperNode*>(

                    reinterpret_cast<HelperNode*>(head_.get().pointer())->next().pointer())); 

            }

            //! Get an iterator to the last element.

            inline iterator end() { return iterator(0); }

            //! Get an const_iterator to the frontmost element.

            inline const_iterator begin() const 

            { 

                return const_iterator(reinterpret_cast<HelperNode*>(

                    reinterpret_cast<HelperNode*>(head_.get().pointer())->next().pointer())); 

            }

            //! Get an const_iterator to the last element.

            inline const_iterator end() const { return const_iterator(0); }

        private:

            void destroyStack(PTPointerCAS& head)

            {

                for(HelperNode* temp = 0, *node; stackPop(head, node); node = temp)

                {

                    assert(node);

                    temp = reinterpret_cast<HelperNode*>(node->next().pointer());

                    Pair* pair = reinterpret_cast<Pair*>(node); // relying on C's pointer to first member == pointer to structure

                    pair->~Pair();

                    allocator_.deallocate(pair, 1);

                }

            }

            HelperNode* recycleHelperNode()

            {

                HelperNode* node = 0;

                if(Private::stackPop(helperNodeStore_, node))

                    return node;



                Pair* pair = allocator_.allocate(1, static_cast<void*>(0));

                new (pair) Pair;

                Private::stackPush(tnodeStore_, &pair->tnode_);

                return &pair->helperNode_;

            }

            inline void recycleHelperNode(HelperNode* node)

            {

                assert(node);

                Private::stackPush(helperNodeStore_, node);

            }

            TNode* recycleTNode()

            {

                TNode* node = 0;

                if(Private::stackPop(tnodeStore_, node))

                    return node;



                Pair* pair = allocator_.allocate(1, static_cast<void*>(0));

                new (pair) Pair;

                Private::stackPush(helperNodeStore_, &pair->helperNode_);

                return &pair->tnode_;

            }

            inline void recycleTNode(TNode* node)

            {

                assert(node);

                assert(!node->hasObject());

                Private::stackPush(tnodeStore_, node);

            }

        private:

            allocator_type allocator_;

            PTPointerCAS helperNodeStore_, tnodeStore_, head_, tail_;

        };

    }

}



#ifdef _MSC_VER

#   pragma warning(disable:4311)

#   pragma warning(disable:4312)

#endif



#endif



