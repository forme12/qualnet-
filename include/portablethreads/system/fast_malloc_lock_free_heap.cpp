#include <algorithm>
#include <cassert>
#include <portablethreads/exception.h>

using namespace std;

namespace PortableThreads 
{
	namespace LockFree
	{

		/**************************************************************/
		/* PTHeapImpl                                                 */
		/**************************************************************/
		namespace Private
		{
			class PTHeapImpl
			{
				typedef PTHeap::AllocateMemory AllocateMemory;
				typedef PTHeap::FreeMemory FreeMemory;
				typedef PTHeap::size_t size_t;
			public:
				explicit
				PTHeapImpl(AllocateMemory a, FreeMemory r);
				void* allocate(size_t size);
				void deallocate(void* p);
				void swap(PTHeapImpl& other);
				static size_t smallestAllocatableBlock();
				static size_t largestAllocatableBlock();
			private:
				PTHeapImpl();
			private:
				AllocateMemory allocate_;
				FreeMemory deallocate_;
			};

			// NOTE: These values should be the same as for the
			// real lock-free heap
			size_t PTHeapImpl::smallestAllocatableBlock()
			{
				return 64;
			}
			size_t PTHeapImpl::largestAllocatableBlock()
			{
				return 2048;
			}
			

			PTHeapImpl::PTHeapImpl(AllocateMemory a, FreeMemory r)
				:	allocate_(a)
				,	deallocate_(r)
			{
				if(!allocate_)
				{
					throw PTParameterError("[PTHeap] Passed pointer to NULL for memory allocation function");
				}
				if(!deallocate_)
				{
					throw PTParameterError("[PTHeap] Passed pointer to NULL for memory deallocation function");
				}
			}

			
			void PTHeapImpl::swap(PTHeapImpl& other)
			{
				std::swap(allocate_, other.allocate_);
				std::swap(deallocate_, other.deallocate_);
			}
			void* PTHeapImpl::allocate(size_t size)
			{
				void * p = allocate_(size);
				if(!p)
					throw OutOfMemory();
				return p;
			}
			
			void PTHeapImpl::deallocate(void* p)
			{
				deallocate_(p);	
			}
		}
	}

}

