/*  Copyright (c) March 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
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

#ifndef PT_THREAD_POOL_H
#define PT_THREAD_POOL_H
#include <portablethreads/config.h>
#include <portablethreads/thread.h>
#include <portablethreads/condition.h>
#include <portablethreads/semaphore.h>
#include <portablethreads/lock_free.h>
#include <portablethreads/smartpointer.h>

#include <vector>
#include <algorithm>

namespace PortableThreads 
{

	/************************************************************************/
	/* ThreadPool                                                           */
	/************************************************************************/

	// Thread pool with N worker threads. This template class takes a functor
	// and an argument to the functor as template arguments. If the argument
	// to the functor is void, no argument is passed. 
	// The thread pool provided synchronous and asynchronous assignment of
	// worker threads.
	// By default, the argument passed is copied, however, if a pointer is passed
	// it is assumned that it points to an object that was allocated with new and
	// may safely be deleted by a worker thread.
	// Allocation and deallocation of threads is synchronous with respect to a
	// thread pool object. Upon creation N worker threads are created. When a 
	// thread pool goes out of scope, all threads are shut down before the 
	// object is destroyed.

	namespace Private
	{
		template<class RequestProzessor /* functor */, class ArgumentT /* Argument to functor, may be void */>
		class WorkerControl_;
	}

	template<class RequestProzessor, class ArgumentT>
	class PThreadPool
	{
	public:
		typedef RequestProzessor WorkerType;
		typedef ArgumentT ArgumentType;
	public:
		// Construct a thread pool with n workers.
		PThreadPool(unsigned int noThreads)
			:	controller_(noThreads)
		{}	
		// Process one item. Call this if the 
		// argument type is void (no argument needed).
		// Caller is blocked until a worker was assigned.
		inline void process()
		{
			controller_.assignWorker()->process();
		}

		// Process one item. Call this if the 
		// argument type is X. The item passed is
		// copied.
		// Caller is blocked until a worker was assigned.
		template<class X>
		inline void process(const X& arg)
		{
			controller_.assignWorker()->process(new X(arg));
		}

		// Same as above but the argument passed is passed
		// as is. NOTE: This must be a pointer to a heap
		// allocated object.
		template<class X>
		void process(X* arg)
		{
			controller_.assignWorker()->process(arg);
		}

		// Same as process() but the method returns immediately
		// indicating if a worker was assigned.
		bool tryProcess()
		{
			InternalWorkerType* worker = controller_.tryAssignWorker();
			if(worker)
				worker->process();
			return worker != 0;
		}

		// Same as template<class X> process(const X& arg) but the method returns immediately
		// indicating if a worker was assigned.
		template<class X>
		bool tryProcess(const X& arg)
		{
			InternalWorkerType* worker = controller_.tryAssignWorker();
			if(worker)
				worker->process(new X(arg));
			return worker != 0;
		}

		// Returns the (current) number of busy workers.
		inline unsigned active() const
		{
			return static_cast<unsigned>(controller_.active());
		}
		// Returns the size of the thread pool.
		inline unsigned size() const
		{
			return static_cast<unsigned>(controller_.size());
		}
	private:
		PThreadPool();
		// members are not copy-constructable!
	private:
		typedef Private::WorkerControl_<RequestProzessor, ArgumentT> ControlType;
		typedef typename ControlType::WorkerType InternalWorkerType;
		ControlType controller_;	
	};

	/************************************************************************/
	/* ThreadPool (Implementation)                                          */
	/************************************************************************/

	#if !defined(_MSC_VER) || _MSC_VER >= 1300
		template<class RequestProzessor, class ArgumentT>
		struct Select_
		{
			static void process(ArgumentT* arg)
			{
				RequestProzessor()(*arg);
			}	
		};

		template<class RequestProzessor>
		struct Select_<RequestProzessor, void>
		{
			static void process(void*)
			{
				RequestProzessor()();
			}	
		};
	#endif

	namespace Private
	{
		template<class RequestProzessor, class ArgumentT>
		class Worker_ : private PThread
		{
			template<class U>
			class ThinSmrtPtr
			{	
				U* ptr;			
			public:
				explicit 
				ThinSmrtPtr(U* p = 0) 
					:	ptr(p) 
				{}
				ThinSmrtPtr(ThinSmrtPtr& foo)
				{
					Copy(foo);
				}

				~ThinSmrtPtr()
				{
					PointerTraits_<U>::freeResource(ptr);
				}
				
				
				ThinSmrtPtr& operator=(ThinSmrtPtr& foo)
				{
					if(&foo == this) return *this;
					PointerTraits_<U>::freeResource(ptr);
					Copy(foo);
					return *this;
				}

				ThinSmrtPtr& operator=(U* a)
				{
					if(a == ptr) return *this;
					PointerTraits_<U>::freeResource(ptr);
					ptr = a;
					return *this;
				}

				U* get() const
				{
					return ptr;
				}
			private:		
				void Copy(ThinSmrtPtr& foo)
				{
					ptr = foo.ptr;
					foo.ptr = 0;
				}
			};
			
		#if defined(_MSC_VER) && _MSC_VER < 1300
			template<class T>
			struct Select_
			{
				static void process(T* arg)
				{
					RequestProzessor()(*arg);
				}
			};

			template<>
			struct Select_<void>
			{
				static void process(void*)
				{
					RequestProzessor()();
				}		
			};
		#endif

			
			typedef WorkerControl_<RequestProzessor, ArgumentT> ControlType;
		public:	
			~Worker_()
			{
				shutdown();
				join();		
			}
			Worker_(ControlType* wc)
				:	arg_(0)
				,	wc_(wc)
				,	startProcessing_(true)
				,	keepRunning_(true)
			{
				assert(wc);
				run();
			}
			void process(ArgumentT* arg = 0)
			{		
				setArgument(arg);
				unblockThread();
			}
			void shutdown()
			{
				keepRunning_ = false;		
				unblockThread();	
			}
		private:
			Worker_();
			void threadMain()
			{
				while(true)
				{
					waitForRequest();			
					
					if(shutdownRequest())
						break;
				
					#if defined(_MSC_VER) && _MSC_VER < 1300
						Select_<ArgumentT>::process(ThinSmrtPtr<ArgumentT>(arg_).get());
					#else
						Select_<RequestProzessor, ArgumentT>::process(ThinSmrtPtr<ArgumentT>(arg_).get());
					#endif			
					enableRequests();
				}
			}
			
			inline bool shutdownRequest()
			{
				return !keepRunning_;
			}
			inline void waitForRequest()
			{
				// double lock by worker thread
				startProcessing_.lock();
			}
			inline void enableRequests()
			{
				wc_->addFreeWorker(this);
			}
			inline void setArgument(ArgumentT* arg)
			{
				arg_ = arg;
			}
			inline void unblockThread()
			{
				startProcessing_.unlock();
			}
		private:
			ThinSmrtPtr<ArgumentT> arg_;
			ControlType* wc_;
			PTMutex startProcessing_;
			volatile bool keepRunning_;	
		};

		template<class RequestProzessor, class ArgumentT>
		class WorkerControl_
		{
		public:
			typedef Worker_<RequestProzessor, ArgumentT> WorkerType;
			~WorkerControl_()
			{
				// decrementing the semaphore will ensure that no
				// worker is still active
				for(size_t i = 0; i < workers_.size(); ++i)
					freeCount_.down();
			}
			WorkerControl_(unsigned size)
				:	workers_(size)
				,	freeCount_(size)
				,	active_(0)
			{
				for(size_t i = 0; i < workers_.size(); ++i)
				{
					workers_[i] = new WorkerType(this);
					assert(workers_[i]);
					free_.push(workers_[i].get());
				}
			}
			inline void addFreeWorker(WorkerType* worker)
			{
				--active_;
				free_.push(worker);
				freeCount_.up();
			}
			WorkerType* assignWorker()
			{
				freeCount_.down();
				
				WorkerType* ret = 0;
				free_.pop(ret);
				assert(ret);
				++active_;
				return ret;
			}
			WorkerType* tryAssignWorker()
			{
				WorkerType* ret = 0;
				free_.pop(ret);
				if(ret)
					++active_;
				return ret;
			}
			inline size_t size() const { return workers_.size(); }
			inline size_t active() const { return (size_t)active_.get(); }
		private:
			WorkerControl_();
		private:
			typedef std::vector< StackPtr<WorkerType> > Workers;
			Workers workers_;
			typedef LockFree::PTStack<WorkerType*> FreeWorkersQueue;
			FreeWorkersQueue free_;
			PTSemaphore freeCount_;
			LockFree::PTAtomicNumber active_;
		};
	}
}

#endif
