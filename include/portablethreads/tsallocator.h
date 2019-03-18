/*  Copyright (c) January 2005 Jean Gressmann (jsg@rz.uni-potsdam.de)
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

#ifndef PT_TSALLOCATOR_H
#define PT_TSALLOCATOR_H
#include <portablethreads/config.h>
#include <portablethreads/utility.h>
#include <portablethreads/mutex.h>
#include <portablethreads/smartpointer.h>
#include <map>
#include <string>
#include <new>
#include <cassert>

namespace PortableThreads 
{
	namespace Private
	{
		// Base class for allocated objects so they can be used
		// polymorphically in TSAllocator
		struct VarInterface
		{
			virtual ~VarInterface() = 0;	
		};


		// Concrete class used in TSAllocator to store allocated
		// objects. This class features a mutex via PLightMutex
		// to ensure mutual exclusive usage.
		// Also a counter is included to implement reference counting.
		// This is not done here, however, but in TSProxy
		template<typename T> 
		struct VolatileVar : public VarInterface
		{
			VolatileVar() 
				:	value_()
				,	refCount_(1)
 			{}
 			VolatileVar(const T& arg)
				:	value_(arg)
				,	refCount_(1)		
  			{}	
			volatile T value_;
 			volatile unsigned refCount_;
  			// This needs to be a PMutex because under Windows
			// using PLightMutex will allow the same thread to 
			// create two LockPointers to the same shared var
			PTMutex lock_;
		};
	}


	// This class provides thead-safe allocation, referencing and destruction
	// of objects. Each allocated object is stored in a map using template
	// parameter U as key.
	template<class U = std::string>
	class PThreadSafeAllocator
	{
	public:	
		typedef U KeyType;	

		// create an object of type T and put it in the map S2Var
		// This is done in a thread-safe manner
		template<typename T> 
		bool create(const KeyType& name, const T& var, T* = 0)
		{
			PTGuard<MutexType> dynUnlock(lock_);

			if(allocatedObjects_.find(name) != allocatedObjects_.end()) 
				return false;
		
			allocatedObjects_.insert(typename S2Var::value_type(name, new Private::VolatileVar<T>(var)));
		
			return true;		
		}
		
		
		// Destory a member before the thread goes out of scope
		bool destroy(const KeyType& name)
		{
			PTGuard<MutexType> dynUnlock(lock_);

			Iterator res = allocatedObjects_.find(name);
			
			if(res == allocatedObjects_.end())
  				return false;
			
			allocatedObjects_.erase(res);		
			return true;				
		}


		// Reference a create var using >>name<< als key. 
		// If no such member exists, a runtime assertion occurs
		// in TSProxy (that is if you use TSProxy to reference the var)
		// unless NDEBUG has been defined.
		// Unless NDEBUG has been defined TSAllocator will use 
		// a dynamic_cast to ensure the object referenced is 
		// of proper type.
		template<typename T> 
		Private::VolatileVar<T>* reference(const KeyType& name, T* = 0) const
		{
			PTGuard<MutexType> dynUnlock(lock_);

			Iterator res = allocatedObjects_.find(name);
			if(res == allocatedObjects_.end()) 
				return 0;
			
			#ifndef NDEBUG	
				return dynamic_cast< Private::VolatileVar<T>* >(res->second.get());
			#else
				return static_cast< Private::VolatileVar<T>* >(res->second.get());
			#endif
		}
	private:
		typedef SharedPtr<Private::VarInterface> SafeVar;
		typedef std::map<U, SafeVar> S2Var;
		typedef typename S2Var::iterator Iterator;
		typedef PTMutex MutexType;
		
		mutable S2Var allocatedObjects_;
		// Here it's ok to use the faster mutex since mt problems only occur
		// when two threads try to access the allocator in parallel
		mutable MutexType lock_;
	};

	// There is one global TSAllocator (Threads.cpp) that is declared here
	// This allocator is used by default by the TSProxy class
	extern PThreadSafeAllocator<std::string> TSA;

	template<class T> class PTLockPointer;
	template<class T> class PTNoLockPointer;

	// Proxy class that uses an TSAllocator compatible allocator 
	// to thread-safe allocate, reference and destroy shared objects
	template<class KeyTypeT, class ValueTypeT>
	class PThreadSafeProxy
	{
	public:
		typedef PTLockPointer<ValueTypeT> LPType;
		typedef PTNoLockPointer<ValueTypeT> PType;
		typedef KeyTypeT KeyType;
		typedef ValueTypeT ValueType;
	private:
		typedef PThreadSafeAllocator<KeyType> AllocatorType;

		friend class PTLockPointer<ValueType>;
		friend class PTNoLockPointer<ValueType>;
	 	
		KeyType name_;	
 		AllocatorType& allocator_;
		Private::VolatileVar<ValueType>* protegee_;
		
		// increment the reference count of a shared 
		// object in the allocator
 		void inc()
		{
			protegee_->lock_.lock();
			++protegee_->refCount_;
			protegee_->lock_.unlock();
		}
		
		// decrement the reference cout of a shared
		// object int the allocator. If the reference
		// cout reaches zero, destroy the die shared
		// object via the allocator's Destroy()-method.
		void dec()
		{
			protegee_->lock_.lock();
			--protegee_->refCount_;
			if(protegee_->refCount_ == 0)
			{
				allocator_.destroy(name_);			
				protegee_ = 0;
  			}	
			else 
  			{ 		
				protegee_->lock_.unlock();
			}
		}
		// we don't want default construction since there is 
		// alsolutely no point without an allocator 
		PThreadSafeProxy();
	public:	
		~PThreadSafeProxy()
		{
			dec();
		}
	 	
		// Try to create a shared object referenced by >>name<<. Creation
		// will fail, if the name used has already been used in the allocator.
		// In that case just a reference is created by incrementing the
		// reference count of the shared object in the allocator
		explicit 
		PThreadSafeProxy(const KeyType& name, const ValueType& val = ValueType(), AllocatorType& alloc = TSA)
  			:	name_(name)
			,	allocator_(alloc)
			,	protegee_(0)
		{
			const bool ret = allocator_.create(name, val);
  			protegee_ = allocator_.reference(name, static_cast<ValueType*>(0));
			// here is where we make sure the d'tor is ok!
			assert(protegee_);
			if(!ret)
				inc();		
		}
		// Copy a reference to a shared object in the allocator.
		// Since it is guaranteed that the shared object exists,
		// just increment its reference count
		explicit
		PThreadSafeProxy(const PThreadSafeProxy& o) 
			:	name_(o.name_)
			,	allocator_(o.allocator_)
			,	protegee_(o.protegee_)
		{
			assert(protegee_);
			inc();
		}
		PThreadSafeProxy& operator=(const PThreadSafeProxy& o)
		{
			if(&o == this || protegee_ == o.protegee_) 
				return *this;
			
			dec();
			
			name_ = o.name_;
			protegee_ = o.protegee_;
			allocator_ = o.allocator_;
			inc();
			
			return *this;		
		}
		// check if we are ready to go
		inline operator const bool() const { return protegee_ != 0; }
	};


	template<class T>
	class PTLockPointer
	{
		PTMutex& lock_;
		T* ptr_;

		// We will not permit objects of this class to 
		// be create by accident or via type conversion
		// This make sense since there is no point
		// to lock a shared object without an shared
		// object, right? ;-)
		PTLockPointer();
		PTLockPointer(const PTLockPointer&);
		PTLockPointer& operator=(const PTLockPointer&);
	public:
		// Through the lifetime of a LockPointer the
		// shared object is locked, unlock it the d'tor
		~PTLockPointer()
		{
			lock_.unlock();
		}
		
		// Lock the shared object referenced by 
		// a TSProxy object so it is accessable for
		// this thread only
		template<class U>
		explicit PTLockPointer(const PThreadSafeProxy<U,T>& p) 
			:	lock_(p.protegee_->lock_)
			,	ptr_(const_cast<T*>(&p.protegee_->value_))
		{
			lock_.lock();
		}
		
		inline T& operator*() { return *ptr_; }
		inline T* operator->() { return ptr_; }
		inline T* get() { return ptr_; }
	};

	template<class T>
	class PTNoLockPointer
	{
		T* ptr_;

		// see LockPointer
		PTNoLockPointer();
		PTNoLockPointer(const PTNoLockPointer&);
		PTNoLockPointer& operator=(const PTNoLockPointer&);
	public:
		// Just get a pointer to the shared object
		// referenced by the parameter TSProxy
		// Beware to use this class! There is no
		// locking done so synchronization of the
		// shared object is entirely up to YOU!
		template<class U>
		explicit PTNoLockPointer(const PThreadSafeProxy<U,T>& p)
			:	ptr_(const_cast<T*>(&p.protegee_->value_))
		{}
		
		inline T& operator*() { return *ptr_; }
		inline T* operator->() { return ptr_; }
		inline T* get() { return ptr_; }
	};

	/*
	*	Use this if you want to create a PTLockedPointer<Type>
	*	who's scope is limited by the braces immediatly following
	*	the macro's name:
	*
	*	PT_LOCKED_SCOPE(int, foo, NameOfProxy)
	*	{
	*		// do something using foo like assigning
	*		// 777 to a shared int
	*		*foo = 777;
	*
	*	} // scope of foo ends here, shared var is unlocked
	*/
	#define PT_LOCKED_SCOPE(Type, Var, Proxy) \
		if(int i_ = 0) ;	\
		else	\
		for(PortableThreads::PTLockPointer<Type> Var(Proxy); i_ < 1; ++i_)
		

}

#endif
