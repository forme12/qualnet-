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

#ifndef PT_LOCK_FREE_ALLOCATOR_H
#define PT_LOCK_FREE_ALLOCATOR_H
#include <portablethreads/config.h>
#include <portablethreads/exception.h>
#include <portablethreads/lockfree/heap.h>
#include <limits>
#include <cstddef>

// windows.h
#ifdef min
#	undef min
#endif
#ifdef max
#	undef max
#endif

namespace PortableThreads 
{
	namespace LockFree
	{
		namespace Private
		{
			class PTAllocatorBase
			{
			protected:
				inline PTHeap& heap() { return heap_; }
			private:
				static PTHeap heap_;
			};
		}

		/*! \class PTAllocator lock_free_allocator.h portablethreads/lock_free.h
			\brief STL-compatible allocator class which uses an instance of 
				PortableThreads' lock-free heap implementation PTHeap to acquire
				memory.

			See the std::allocator for documentation.
		*/
		template<typename T>
		class PTAllocator : public Private::PTAllocatorBase
		{
		public:
			typedef const T* const_pointer;
			typedef const T& const_reference;
			typedef std::ptrdiff_t difference_type;
			typedef T* pointer;
			typedef T& reference;
			typedef std::size_t size_type;
			typedef T value_type;
		public:
			explicit
			PTAllocator()
			{}
			template<class Other>
			PTAllocator(const PTAllocator<Other>& other)
			{}
			inline pointer address(reference r) const { return &r; }
			inline const_pointer address(const_reference r) const { return &r; }
			inline pointer allocate(size_type count)
			{
				return static_cast<pointer>(heap().allocate(count*sizeof(T)));
			}
			template<class Other>
			inline pointer allocate(size_type count, const Other* = 0)
			{
				return static_cast<pointer>(heap().allocate(count*sizeof(T)));
			}
			inline void construct(pointer p, const_reference r)
			{
				new (p) T(r);
			}
			inline void deallocate(pointer p, size_type)
			{
				heap().deallocate(p);
			}
			inline void destroy(pointer p)
			{
				p->~T();
			}
			inline size_type max_size() const
			{
				return std::numeric_limits<size_type>::max() / 2;
			}
			template<class Other>
			struct rebind 
			{
				typedef PTAllocator<Other> other;
			};
			template<class Other>
			inline PTAllocator& operator=(const PTAllocator<Other>& other)
			{
				return *this;
			}
		};

		template<typename T>
		inline bool operator==(const PTAllocator<T>& lhs, const PTAllocator<T>& rhs)
		{
			return true;
		}

		template<typename T>
		inline bool operator!=(const PTAllocator<T>& lhs, const PTAllocator<T>& rhs)
		{
			return false;
		}


		/*! \class PTUsePTHeapMixin lock_free_allocator.h portablethreads/lock_free.h
			\brief Mixin class that provides memory management for a given class using
				PortableThreads' lock-free heap implementation PTHeap.

		*/
		class PTUsePTHeapMixin
		{
		public:
			typedef std::size_t size_t;
			/*! \brief Set the PTHeap object to use.

				Once an object of type PTUsePTHeapMixin has
				been created and memory has been allocated using
				that object changing the heap via setHeap(PTHeap&)
				results in undefined behavior.
			*/
			static void setHeap(PTHeap& heap);
			//! allocate \a s bytes of memory.
			void* operator new(size_t s);
			//! deallocate \a s bytes of memory starting at address \a p.
			void operator delete(void* p, size_t s);
			//! allocate \a s bytes of memory.
			void* operator new[](size_t s);
			//! deallocate \a s bytes of memory starting at address \a p.
			void operator delete[](void* p, size_t s);
		private:
			static PTHeap* heap_;
		};
	}
}

#endif

