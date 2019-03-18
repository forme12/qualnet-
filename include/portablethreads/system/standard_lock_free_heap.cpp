#include <cstdlib>
#include <algorithm>
#include <cassert>
#include <iostream>
#include <portablethreads/exception.h>
#include <portablethreads/lockfree/utility.h>
#include <portablethreads/lockfree/heap.h>

// in case windows.h is included
#ifndef max
#	undef max
#endif
#ifdef min
#	undef min
#endif

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
			public:
				typedef std::size_t size_t;
				typedef void* (*AllocateMemory)(size_t); //!< Signature of the memory allocation function.
				typedef void (*FreeMemory)(void*); //!< Signature of the memory deallocation function.
				~PTHeapImpl();
				/*! \brief Create a heap.

					By default the heap uses ::operator new to allocate memory
					and ::operator delete to allocate memory.

					\param alloc Function pointer to a function used to allocate memory for the heap.
						If NULL then ::operator new is used.
					\param dealloc Function pointer to a function used to free heap memory.
						If NULL then ::operator delete is used.
				*/
				explicit
				PTHeapImpl(AllocateMemory alloc = 0, FreeMemory dealloc = 0);
				explicit
				PTHeapImpl(const PTHeapImpl&);
				PTHeapImpl& operator=(const PTHeapImpl&);
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
				/*! Swap contents with another heap

					Ensure that no references to memory of one of the 
					heaps exist, other wise undefined behavior results.
				*/
				void swap(PTHeapImpl& other);
				//! Smallest memory block in bytes that the heap caches.
				static size_t smallestAllocatableBlock();
				//! Smallest memory block in bytes that the heap caches.
				static size_t largestAllocatableBlock();
			private:
				static void* globalOperatorNew(size_t);
				static void globalOperatorDelete(void*);
				static size_t getIndex(size_t s);
			private:
				void initializeStacks();
				char* allocatePage();
				void* heapAllocate(size_t size);
				void* cacheAllocate(size_t size);
				void addPage(size_t index);
				void addPage(char* page, size_t pagesize, size_t index);
			private:
				AllocateMemory allocate_;
				FreeMemory deallocate_;
				PTPointerCAS* pages_;
				PTPointerCAS* chunks_;
			private:
				static const size_t MIN_ALLOC_SIZE;
				static const size_t MAX_ALLOC_SIZE;
				static const size_t PAGE_SIZE;
				static const size_t ADJUST_INDEX;
				static const size_t MAX_INDEX;
			};

			namespace 
			{
				enum { STRUCT_ALIGN = 2*sizeof(size_t) };

				template<unsigned N>
				struct Fits
				{
					enum { ALIGN_ADD = sizeof(PTPointerCAS) - N) };
				};

				template<>
				struct Fits<0>
				{
					enum { ALIGN_ADD = 0 };
				};

				enum { MM_FIELDS_SIZE = sizeof(PTPointerCAS) + Fits<sizeof(PTPointerCAS) % STRUCT_ALIGN>::ALIGN_ADD };
				
				template<bool>
				struct StaticAssert;

				template<>
				struct StaticAssert<true>
				{};

				
				static StaticAssert<MM_FIELDS_SIZE % STRUCT_ALIGN == 0> DataAfterMMFieldIsStructAligned;
				
				struct OutOfMemory : public std::bad_alloc
				{
					~OutOfMemory() throw()
					{}
					const char *what() const throw()
					{
						return "[PTHeapImpl] Memory allocation failed";
					}
				};

				void stackPush(PTPointerCAS& head, char* raw)
				{
					PTPointerCAS::token_t ov, nv;
					do
					{
						ov = head.get();
						reinterpret_cast<PTPointerCAS*>(raw)->assign(ov);
						nv.pointer(reinterpret_cast<PTPointerCAS::int_t>(raw));
						nv.userbits(ov.userbits());
					}
					while(!head.cas(nv, ov));
				}

				bool stackPop(PTPointerCAS& head, char*& node)
				{
					for(PTPointerCAS::token_t h = head.get(), nh; h.pointer(); h = head.get())
					{
						nh = reinterpret_cast<PTPointerCAS*>(h.pointer())->get();
						if(head.cas(nh, h))
						{
							node = reinterpret_cast<char*>(h.pointer());
							return true;
						}
					}
					return false;
				}

				inline PTPointerCAS::int_t indexToPointer(size_t index)
				{
					return static_cast<PTPointerCAS::int_t>(index << LockFree::Private::ALIGNMENT_BITS);
				}
				inline size_t pointerToIndex(PTPointerCAS::int_t index)
				{
					return static_cast<size_t>(index) >> LockFree::Private::ALIGNMENT_BITS;
				}
			}

			const size_t PTHeapImpl::MIN_ALLOC_SIZE = sizeof(void*) == 4 ? 16 : 32;
			const size_t PTHeapImpl::MAX_ALLOC_SIZE = 2048;
			const size_t PTHeapImpl::PAGE_SIZE = 4096;
			const size_t PTHeapImpl::ADJUST_INDEX = sizeof(void*) == 4 ? 4 : 5; // must satisfy 1 << ADJUST_INDEX == MIN_ALLOC_SIZE
			const size_t PTHeapImpl::MAX_INDEX = 11;  // must satisfy 1 << MAX_INDEX == MAX_ALLOC_SIZE


			size_t PTHeapImpl::smallestAllocatableBlock()
			{
				return MIN_ALLOC_SIZE;
			}
			size_t PTHeapImpl::largestAllocatableBlock()
			{
				return MAX_ALLOC_SIZE;
			}
			

			size_t PTHeapImpl::getIndex(size_t s)
			{
				if(s <= MIN_ALLOC_SIZE)
					return ADJUST_INDEX;

				size_t index = ADJUST_INDEX;		
				for(size_t cmp = static_cast<size_t>(1) << ADJUST_INDEX; 
					cmp < s; ++index, cmp <<= 1);			

				assert(index <= MAX_INDEX);
				return index;
			}

			void* PTHeapImpl::globalOperatorNew(size_t s)
			{
				return ::operator new(s);
			}
			void PTHeapImpl::globalOperatorDelete(void* p)
			{
				::operator delete(p);
			}


			PTHeapImpl::~PTHeapImpl()
			{
				for(char* p = 0; stackPop(*pages_, p);)
				{
					assert(p);
					deallocate_(p);
				}
				deallocate_(chunks_);
			}

			PTHeapImpl::PTHeapImpl(AllocateMemory a, FreeMemory r)
				:	allocate_(a ? a : &globalOperatorNew)
				,	deallocate_(r ? r : &globalOperatorDelete)
				,	pages_(0)
				,	chunks_(0)
			{
				if(allocate_ == 0)
				{
					throw PTParameterError("[PTHeapImpl] Passed NULL pointer for allocation function");
				}
				if(deallocate_ == 0)
				{
					throw PTParameterError("[PTHeapImpl] Passed NULL pointer for deallocation function");
				}
				initializeStacks();
			}
			PTHeapImpl::PTHeapImpl(const PTHeapImpl& other)
				:	allocate_(other.allocate_)
				,	deallocate_(other.deallocate_)
				,	pages_(0)
				,	chunks_(0)
			{
				initializeStacks();
			}

			PTHeapImpl& PTHeapImpl::operator=(const PTHeapImpl& other)
			{
				PTHeapImpl(other).swap(*this);
				return *this;
			}
			void PTHeapImpl::initializeStacks()
			{
				assert(allocate_);
				assert(deallocate_);

				const size_t s = getIndex(MAX_ALLOC_SIZE) - getIndex(MIN_ALLOC_SIZE) + 2;
				chunks_ = static_cast<PTPointerCAS*>(allocate_(sizeof(PTPointerCAS)*s));
				if(!chunks_)
					throw OutOfMemory();
				
				for(size_t i = 0; i < s; ++i)
					new (&chunks_[i]) PTPointerCAS();
				pages_ = chunks_ + s - 1;
			}
			char* PTHeapImpl::allocatePage()
			{
				char* raw = static_cast<char*>(allocate_(PAGE_SIZE));
				if(!raw)
					throw OutOfMemory();
				new (raw) PTPointerCAS();
				stackPush(*pages_, raw);
				return static_cast<char*>(raw) + MM_FIELDS_SIZE;
			}
			void PTHeapImpl::swap(PTHeapImpl& other)
			{
				std::swap(allocate_, other.allocate_);
				std::swap(deallocate_, other.deallocate_);
				std::swap(pages_, other.pages_);
				std::swap(chunks_, other.chunks_);
			}
			void* PTHeapImpl::allocate(size_t size)
			{
				if(size <= MAX_ALLOC_SIZE)
					return cacheAllocate(size);
				return heapAllocate(size);
			}
			void* PTHeapImpl::heapAllocate(size_t size)
			{
				char* raw = static_cast<char*>(allocate_(size + MM_FIELDS_SIZE));
				if(!raw)
					throw OutOfMemory();
				new (raw) PTPointerCAS(indexToPointer(MAX_INDEX+1));
				
				return raw + MM_FIELDS_SIZE;
			}
			void PTHeapImpl::addPage(size_t index)
			{
				addPage(allocatePage(), PAGE_SIZE - MM_FIELDS_SIZE, index);
			}
			void PTHeapImpl::addPage(char* page, size_t pagesize, size_t index)
			{
				const size_t chunksize = static_cast<size_t>(1) << index;
				assert(chunksize % STRUCT_ALIGN == 0);
				const size_t totalskip = chunksize + MM_FIELDS_SIZE;
				assert(totalskip % STRUCT_ALIGN == 0);
				const size_t entries = pagesize / totalskip;
				
				for(size_t i = 0; i < entries; ++i, page += totalskip)
				{
					new (page) PTPointerCAS();
					stackPush(chunks_[index - ADJUST_INDEX], page);
				}

				if(index > ADJUST_INDEX)
					addPage(page, pagesize - entries * totalskip, index - 1);
			}
			void* PTHeapImpl::cacheAllocate(size_t size)
			{
				const size_t index = getIndex(size);
				assert(index >= ADJUST_INDEX);
				assert(index <= MAX_INDEX);

				char* raw;		
				while(!stackPop(chunks_[index - ADJUST_INDEX], raw))
				{			
					addPage(index);
				}
				reinterpret_cast<PTPointerCAS*>(raw)->assign(PTPointerCAS::token_t(indexToPointer(index)));
				return raw + MM_FIELDS_SIZE;
			}

			void PTHeapImpl::deallocate(void* p)
			{
				assert(p);
				// find out if memory is managed by the cache
				char* raw = static_cast<char*>(p) - MM_FIELDS_SIZE;
				const size_t index = pointerToIndex(reinterpret_cast<PTPointerCAS*>(raw)->get().pointer());
				
				if(index <= MAX_INDEX)
				{
					assert(index >= ADJUST_INDEX);
					stackPush(chunks_[index - ADJUST_INDEX], raw);
				}
				else
				{
					assert(index == MAX_INDEX+1);
					deallocate_(raw);
				}	
			}
		}
	}

}

