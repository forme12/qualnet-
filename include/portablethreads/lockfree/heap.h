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

#ifndef PT_LOCK_FREE_HEAP_H
#define PT_LOCK_FREE_HEAP_H
#include <portablethreads/config.h>
#include <cstddef>
#include <memory>

namespace PortableThreads 
{
	namespace LockFree
	{
		namespace Private
		{
			class PTHeapImpl;
		}

		/*! \class PTHeap lock_free_heap.h portablethreads/lock_free.h
			\brief PTHeap provides a lock-free interface to memory management. 
			
			PTHeap utilizes the function pair ::operator new and ::operator delete
			by default to allocate memory. Other memory allocation/deallocation
			functions can be used as well.

			Memory allocation via a PTHeap object either succeeds or an exception
			is raised. The specific exception type depends on the memory management
			functions used. When ::operator new or std::malloc is used, an exception
			deriving from std::bad_alloc is thrown by the implementation.

			The allocator provided as template parameter must allow
			for concurrent allocation and deallocation. The STL allocator
			meets this requirement.

			Memory allocated by the heap implementation is freed when the object
			is destroyed.
		*/
		class PTHeap
		{
		public:
			typedef std::size_t size_t;
			typedef void* (*AllocateMemory)(size_t); //!< Signature of the memory allocation function.
			typedef void (*FreeMemory)(void*); //!< Signature of the memory deallocation function.
			~PTHeap();
			/*! \brief Create a lock-free heap.

				By default the heap uses ::operator new to allocate memory
				and ::operator delete to allocate memory.

				\param alloc Function pointer to a function used to allocate memory for the heap.
					If NULL then ::operator new is used.
				\param dealloc Function pointer to a function used to free heap memory.
					If NULL then ::operator delete is used.
			*/
			explicit
			PTHeap(AllocateMemory alloc = 0, FreeMemory dealloc = 0);
			explicit
			PTHeap(const PTHeap&);
			PTHeap& operator=(const PTHeap&);
			/*! \brief Allocate \a size bytes.

				Attempt to allocate \a size bytes of memory. If the 
				function globalOperatorNew(size_t) or malloc(size_t)
				are used, an exception (see exceptions) is raised if
				the memory allocation fails. Depending on the 
				function used for memory allocation this method
				may or may not throw an exception.
				
				\exception std::bad_alloc Memory allocation failed.
			*/
			void* allocate(size_t size);
			/*! Deallocate memory

				If the pointer \a p isn't a pointer previously
				returned by this object's allocate(size_t) method,
				the results are undefined.
			*/
			void deallocate(void* p);
			void swap(PTHeap& other);
			//! Smallest memory block in bytes that the heap caches.
			static size_t smallestAllocatableBlock();
			//! Smallest memory block in bytes that the heap caches.
			static size_t largestAllocatableBlock();
		private:
			static void* globalOperatorNew(size_t);
			static void globalOperatorDelete(void*);
		private:
			std::auto_ptr<Private::PTHeapImpl> pimpl_;
		};
	}
}

namespace std
{
	//! Overloaded std::swap for PortableThreads::LockFree::PTHeap
	inline void swap(PortableThreads::LockFree::PTHeap& lhs, PortableThreads::LockFree::PTHeap& rhs)
	{
		lhs.swap(rhs);
	}
}

#endif

