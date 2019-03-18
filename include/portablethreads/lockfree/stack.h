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

#ifndef PT_LOCK_FREE_STACK_H
#define PT_LOCK_FREE_STACK_H
#include <portablethreads/config.h>
#include <portablethreads/lockfree/single_linked_node.h>
#include <portablethreads/lockfree/utility.h>
#include <cassert>
#include <iterator>
#include <memory> // std::allocator

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
		namespace Private
		{
			template<class T>
			class StackIterator : public std::iterator<std::forward_iterator_tag, T> 
			{
				typedef std::iterator<std::forward_iterator_tag, T> Base;
				typedef SingleLinkedNode<T> Node;
			public:
				typedef typename Base::pointer pointer;
				typedef typename Base::reference reference;
				operator const void*() const { return current_; }
				explicit
				StackIterator(Node* node = 0)
					:	current_(node)
				{}
				inline pointer operator->() const
				{
					return &current_->data();
				}
				inline reference operator*() const
				{
					return current_->data();
				}
				inline StackIterator& operator++()
				{
					if(current_)
						current_ = reinterpret_cast<Node*>(current_->next().pointer());
					return *this;
				}
				inline StackIterator operator++(int)
				{
					const StackIterator temp(*this);
					++(*this);
					return temp;
				}
			private:
				Node* current_;
			};
		}


		/*! \class PTStack stack.h portablethreads/lock_free.h
			\brief Lock-free stack with correct memory management.

			This stack implementation is based on Treiber's original
			lock-free stack implementation.
			
			Reference: R. K. Treiber. Systems Programming: Coping with
			Parallelism. In RJ 5118, IBM Almaden Research
			Center, April 1986.

			Manipulation of the stack via the push/pop interface is thread-safe.

			When pushing an element onto or popping an element from the stack,
			the element is ever copied or assigned within the context of 
			one thread.

			The stack allows for its content to be iterated over using
			forward iterators. The iterators may also be used to modify
			the content of the stack. Iterating over the stack yields
			undefined results if the stack is used concurrently.

			Memory which is allocated by the stack implementation is
			freed on object destruction. 			
		*/
		template<class T, class A = std::allocator<T> >
		class PTStack
		{
			typedef Private::SingleLinkedNode<T> Node;
		public:
			typedef typename A::template rebind<Node>::other allocator_type;		
			typedef T value_type;
			typedef T& reference;
			typedef Private::StackIterator<T> iterator;
			typedef Private::StackIterator<const T> const_iterator;
			typedef const T& const_reference;
			~PTStack()
			{
				for(Node* current = reinterpret_cast<Node*>(head_.get().pointer());
					current; current = destroyNode(current));
				for(Node* current = reinterpret_cast<Node*>(storeHead_.get().pointer());
					current; current = destroyNode(current));
			}
			//! Create an empty stack.
			explicit
			PTStack()
				:	head_(0)
				,	storeHead_(0)
			{}
			//! Create an empty stack using the allocator provided.
			explicit
			PTStack(allocator_type& a)
				:	allocator_(a)
				,	head_(0)
				,	storeHead_(0)
			{}
			/*! \brief Push one element on the stack.

				\param t Element to push onto the stack.

				\retval An iterator to the element on the stack.
			*/
			inline iterator push(const_reference t)
			{
				Node* node = recycle();
				try
				{
					node->construct(t);
				}
				catch(...)
				{
					recycle(node);
					throw;
				}
				iterator ret(node);

				Private::stackPush(head_, node);

				return ret;
			}
			/*! \brief Pop topmost element from the stack.

				If the pop operation does not succeed, the reference \a t is
				left unchanged.

				\param t Reference which is assigned the element popped from the stack

				\retval true An element was removed from the stack.
				\retval false The stack was empty at the time.
			*/
			inline bool pop(reference t)
			{
				for(Node* node; Private::stackPop(head_, node);)
				{
					assert(node);
					t = node->data();
					node->destruct();
					recycle(node);
					return true;
				}
				return false;
			}
			//! Get an iterator to the topmost stack element.
			inline iterator begin() 
			{ 
				return iterator(reinterpret_cast<Node*>(head_.get().pointer())); 
			}
			//! Get an iterator to the last stack element.
			inline iterator end() { return iterator(0); }
			//! Get an const_iterator to the topmost stack element.
			inline const_iterator begin() const 
			{ 
				return const_iterator(
					reinterpret_cast<const Node*>(head_.get().pointer())); 
			}
			//! Get an const_iterator to the last stack element.
			inline const_iterator end() const { return const_iterator(0); }
		private:
			inline Node* destroyNode(Node* node)
			{
				Node* temp = reinterpret_cast<Node*>(node->next().pointer());
				node->~Node();
				allocator_.deallocate(node, 1);
				return temp;
			}
			inline Node* recycle()
			{
				Node* node = 0; // b/c -Wall (uninit)
				if(stackPop(storeHead_, node))
				{
					assert(node);
					return node;
				}
				return new (allocator_.allocate(1, static_cast<void*>(0))) Node;
			}
			inline void recycle(Node* node)
			{
				assert(node);
				assert(!node->hasObject());
				Private::stackPush(storeHead_, node);
			}
			
		private:
			allocator_type allocator_;
			Private::PTPointerCAS head_, storeHead_;
		};

	}

}

#ifdef _MSC_VER
#	pragma warning(disable:4311)
#	pragma warning(disable:4312)
#endif

#endif

