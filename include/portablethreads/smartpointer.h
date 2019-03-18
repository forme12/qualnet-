/*  Copyright (c) October 2004 Jean Gressmann (jsg@rz.uni-potsdam.de)
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

#ifndef SMARTPOINTER_H
#define SMARTPOINTER_H

#if defined(_MSC_VER)
#	define for if(0);else for
#	pragma warning(disable:4786)
#	pragma warning(disable:4521)
#	pragma warning(disable:4522)
#	pragma warning(disable:4355)
#	pragma warning(disable:4291)
#endif

#include <algorithm>
#include <cassert>

#include <portablethreads/config.h>
namespace PortableThreads {

// Thanks to Benjamin Kaufmann (kaufmann@cs.uni-potsdam.de)
// who provided for compile time assertions for void pointers
template <bool> struct StaticAssert;
template <> struct StaticAssert<true> {};
template <class T> struct is_void {enum {result = 0};};
template <> struct is_void<void> {enum {result = 1};};

// These freeResource() functions are required to handle their
// specific Null indifferently, say delete 0 is
// to do nothing.
template<class T>
struct PointerTraits_
{
	StaticAssert<!is_void<T>::result> Error_Smart_Pointer_Does_Not_Support_Void_Ptr;
	typedef T ValueType;
	typedef T& ReferenceType;
	typedef const T& ConstReferenceType;
	typedef T* PointerType;
	typedef const T* ConstPointerType;
	static const PointerType Null;

	static void freeResource(PointerType p)
	{
		enum { no_incomplete_types_please = sizeof(*p) };
		delete p;
	}
};

template<class T>
const typename PointerTraits_<T>::PointerType PointerTraits_<T>::Null = 0;

template<class T>
struct ArrayPointerTraits_
{
	StaticAssert<!is_void<T>::result> Error_Smart_Pointer_Does_Not_Support_Void_Ptr;
	typedef T ValueType;
	typedef T& ReferenceType;
	typedef const T& ConstReferenceType;
	typedef T* PointerType;
	typedef const T* ConstPointerType;
	static const PointerType Null;
	
	static void freeResource(PointerType p)
	{
		enum { no_incomplete_types_please = sizeof(*p) };
		delete [] p;
	}
};

template<class T>
const typename ArrayPointerTraits_<T>::PointerType ArrayPointerTraits_<T>::Null = 0;

// Single owner policy
template<class Traits>
class SingleOwner_
{
public:
	typedef typename Traits::PointerType PT;		
	typedef typename Traits::ConstPointerType CPT;		
	static CPT null() { return Traits::Null; }
private:
	PT managed_;	

	SingleOwner_(const SingleOwner_&);
	SingleOwner_& operator=(const SingleOwner_&);			
public:
	~SingleOwner_()
	{
		Traits::freeResource(managed_);
	}
	explicit
	SingleOwner_()
		:	managed_(Traits::Null)
	{}
	explicit
	SingleOwner_(PT p)
		:	managed_(p)		
	{}
	
	SingleOwner_(SingleOwner_& o)
		:	managed_(Traits::Null)		
	{
		swap(o);
	}

	inline  
	SingleOwner_& operator=(SingleOwner_& o)
	{
		swap(o);
		return *this;
	}
	inline
	SingleOwner_& operator=(PT p)
	{
		SingleOwner_(p).swap(*this);
		return *this;
	}
	inline
	bool unique() const { return true; }
	inline
	size_t count() const { return 1; }
	inline
	PT get() { return managed_; }
	inline
	CPT get() const { return managed_; }
	inline
	void swap(SingleOwner_& o) // nothrow guarantee
	{
		std::swap(managed_, o.managed_);		
	}
	inline
	void reset(PT p = Traits::Null)
	{
		if(p != managed_)
			SingleOwner_(p).swap(*this);		
	}
	inline
	PT release()
	{
		PT ret = managed_;
		managed_ = Traits::Null;
		return ret;		
	}
};

// reference counting based on a shared counter
template<class Traits>
class ReferenceCountedOwner_
{
public:	
	typedef typename Traits::PointerType PT;		
	typedef typename Traits::ConstPointerType CPT;		
	static CPT null() { return Traits::Null; }
private:
	class RefCount_
	{
		PT managed_;
		size_t count_;		
		RefCount_();
		RefCount_(const RefCount_&);
		RefCount_& operator=(const RefCount_&);
	public:	
		explicit
		RefCount_(PT p)
			:	managed_(p)
			,	count_(1)
		{}
		inline
		void inc()
		{
			++count_;
		}
		inline
		size_t count() const { return count_; }
		inline
		bool unique() const { return count_ == 1; }
		inline
		void dec()
		{
			if(!--count_)
			{
				PT deleteme = managed_;
				delete this;
				Traits::freeResource(deleteme);				
			}
				
		}
		inline
		void weakDec()
		{
			if(!--count_)
				delete this;
		}
		inline
		PT get() { return managed_;	}
		inline
		CPT get() const { return managed_; }
	};
	RefCount_* refCount_;
	
	inline
	void decIfValidRefCount()
	{
		if(refCount_)
			refCount_->dec();	
	}
	inline
	void incIfValidRefCount()  // nothrow guarantee
	{
		if(refCount_)
			refCount_->inc();
	}
	inline
	static RefCount_* createRefCount(PT p)
	{
		return p == Traits::Null ? 0 : new RefCount_(p);
	}	
public:
	~ReferenceCountedOwner_()
	{
		decIfValidRefCount();
	}
	explicit
	ReferenceCountedOwner_()
		:	refCount_(0)
	{}	
	ReferenceCountedOwner_(PT p)
		:	refCount_(createRefCount(p))
	{}
	ReferenceCountedOwner_(const ReferenceCountedOwner_& o)
		:	refCount_(o.refCount_)
	{
		incIfValidRefCount();
	}
	inline
	ReferenceCountedOwner_& operator=(const ReferenceCountedOwner_& o)		
	{
		ReferenceCountedOwner_(o).swap(*this);
		return *this;
	}
	inline
	ReferenceCountedOwner_& operator=(PT p)		
	{
		ReferenceCountedOwner_(p).swap(*this);
		return *this;
	}
	inline
	bool unique() const { return refCount_ ? refCount_->unique() : true; }  // nothrow guarantee
	inline
	size_t count() const { return refCount_ ? refCount_->count() : 1; }  // nothrow guarantee
	inline
	PT get() { return refCount_ ? refCount_->get() : Traits::Null; }  
	inline
	CPT get() const { return refCount_ ? refCount_->get() : Traits::Null; } 
	inline
	void swap(ReferenceCountedOwner_& o)  // nothrow guarantee
	{
		std::swap(refCount_, o.refCount_);
	}
	inline
	void reset(PT p = Traits::Null)
	{
		if(p != get())
			ReferenceCountedOwner_(p).swap(*this);		
	}
	inline
	PT release()
	{
		PT ret = get();
		if(refCount_)
			refCount_->weakDec();
		refCount_ = 0;
		return ret;
	}
};

// reference counting based on linked pointers
template<class Traits>
class LinkCountedOwner_
{
public:	
	typedef typename Traits::PointerType PT;		
	typedef typename Traits::ConstPointerType CPT;		
	static CPT null() { return Traits::Null; }
private:
	mutable LinkCountedOwner_* next_;
	mutable LinkCountedOwner_* prev_;
	PT managed_;
	void addToChain(const LinkCountedOwner_& o) // nothrow guarantee
	{
		prev_ = o.prev_;
		next_ = const_cast<LinkCountedOwner_*>(&o);
		o.prev_->next_ = this;
		o.prev_ = this;
	}
	inline
	void addToChainIfPointerIsValid(const LinkCountedOwner_& o) // nothrow guarantee
	{
		if(o.managed_ != Traits::Null)
			addToChain(o);
	}	
	void removeFromChain() const // nothrow guarantee
	{
		next_->prev_ = prev_;
		prev_->next_ = next_;
		next_ = const_cast<LinkCountedOwner_*>(this);
		prev_ = const_cast<LinkCountedOwner_*>(this);
	}
	inline
	void toInitialState()
	{
		removeFromChain();
		managed_ = Traits::Null;
	}
public:
	~LinkCountedOwner_()
	{
		if(unique())
			Traits::freeResource(managed_);
		else
			removeFromChain();
	}
	explicit
	LinkCountedOwner_()
		:	next_(this)
		,	prev_(this)
		,	managed_(Traits::Null)
	{}	
	LinkCountedOwner_(PT p)
		:	next_(this)
		,	prev_(this)
		,	managed_(p)
	{}
	LinkCountedOwner_(const LinkCountedOwner_& o)
		:	next_(this)
		,	prev_(this)
		,	managed_(o.managed_)
	{
		addToChainIfPointerIsValid(o);
	}
	inline
	LinkCountedOwner_& operator=(const LinkCountedOwner_& o)
	{
		LinkCountedOwner_(o).swap(*this);
		return *this;
	}
	inline
	LinkCountedOwner_& operator=(PT p)		
	{
		LinkCountedOwner_(p).swap(*this);
		return *this;
	}
	inline
	bool unique() const 
	{ 
		return next_ == this && prev_ == this; 
	}
	size_t count() const 
	{
		size_t c = 1;		
		for(const LinkCountedOwner_* current = this;
			current->next_ != this; 
			++c, current = current->next_);
		
		assert(c == reverse_count());
		return c;
	}
private:
	size_t reverse_count() const 
	{
		size_t c = 1;		
		for(const LinkCountedOwner_* current = this;
			current->prev_ != this; 
			++c, current = current->prev_);
		
		return c;
	}
public:
	inline
	PT get() { return managed_; }
	inline
	CPT get() const { return managed_; }
	void swap(LinkCountedOwner_& o) 
	{
		if(!unique() && !o.unique())
		{
			std::swap(prev_->next_, o.prev_->next_);
			assert(prev_->next_ == &o);
			assert(o.prev_->next_ == this);
			std::swap(next_->prev_, o.next_->prev_);
			assert(next_->prev_ == &o);
			assert(o.next_->prev_ == this);	
			std::swap(next_, o.next_);
			std::swap(prev_, o.prev_);
		}
		else if(unique() && !o.unique())
		{
			addToChain(o);
			o.removeFromChain();
		}
		else if(!unique() && o.unique())
		{
			o.addToChain(*this);
			removeFromChain();
		}
		std::swap(managed_, o.managed_);		
	}
	void reset(PT p = Traits::Null)
	{
		if(p != managed_)
		{
			if(unique())
			{
				std::swap(p, managed_);
				Traits::freeResource(p);
			}			
			else
			{
				removeFromChain();
				managed_ = p;
			}	
		}
	}
	inline
	PT release()
	{
		PT ret = managed_;
		toInitialState();
		return ret;		
	}
};

template
<
	class T,
	class Traits = PointerTraits_<T>,
	class RefCountPolicy = SingleOwner_<Traits>
>
class SmartPointer : public RefCountPolicy
{	
public:
	typedef typename Traits::PointerType PT;
	typedef typename Traits::ConstPointerType CPT;
	typedef typename Traits::ReferenceType RT;
	typedef typename Traits::ConstReferenceType CRT;

	explicit
	SmartPointer() 
	{}
	explicit
	SmartPointer(PT p) 
		:	RefCountPolicy(p)
	{}

	SmartPointer(const SmartPointer& o)
		:	RefCountPolicy(o)
	{}
	SmartPointer(SmartPointer& o)
		:	RefCountPolicy(o)
	{}
	inline
	SmartPointer& operator=(SmartPointer& o)
	{
		RefCountPolicy::operator=(o);
		return *this;
	}
	inline
	SmartPointer& operator=(const SmartPointer& o)
	{
		RefCountPolicy::operator=(o);
		return *this;
	}
	inline
	SmartPointer& operator=(PT p)
	{
		RefCountPolicy::operator=(p);
		return *this;
	}
	inline
	bool operator==(const SmartPointer& o) const
	{
		return this->get() == o.get();
	}
	inline
	bool operator!=(const SmartPointer& o) const
	{
		return this->get() != o.get();
	}
	inline
	operator bool() const { return this->get() != Traits::Null; }
	inline
	PT operator->() { return this->get(); }
	inline
	CPT operator->() const { return this->get(); }
	inline
	RT operator*() { return *this->get(); }
	inline
	CRT operator*() const { return *this->get(); }
};

// The next three classes are provided for conviences
template<class T, class Traits = PointerTraits_<T> >
class SmartPtr : public SmartPointer<T, Traits, SingleOwner_<Traits> >
{
	typedef SmartPointer<T, Traits, SingleOwner_<Traits> > Base;
	typedef typename Base::PT PT;
public:
	explicit
	SmartPtr()
		:	Base()
	{}
	explicit
	SmartPtr(PT p)
		:	Base(p)
	{}
	
	SmartPtr(SmartPtr& o)
		:	Base(o)
	{}
	inline
	SmartPtr& operator=(PT p)
	{
		Base::operator=(p);
		return *this;
	}
	inline
	SmartPtr& operator=(SmartPtr& o)
	{
		Base::operator=(o);
		return *this;
	}
};

template<class T, class Traits = PointerTraits_<T> >
class SharedPtr : public SmartPointer<T, Traits, ReferenceCountedOwner_<Traits> >
{
	typedef SmartPointer<T, Traits, ReferenceCountedOwner_<Traits> > Base;
	typedef typename Base::PT PT;
public:
	explicit
	SharedPtr()
		:	Base()
	{}
	
	SharedPtr(PT p)
		:	Base(p)
	{}
	SharedPtr(const SharedPtr& o)
		:	Base(o)
	{}
	inline
	SharedPtr& operator=(PT p)
	{
		Base::operator=(p);
		return *this;
	}
	inline
	SharedPtr& operator=(const SharedPtr& o)
	{
		Base::operator=(o);
		return *this;
	}
};

template<class T, class Traits = PointerTraits_<T> >
class StackPtr : public SmartPointer<T, Traits, LinkCountedOwner_<Traits> >
{
	typedef SmartPointer<T, Traits, LinkCountedOwner_<Traits> > Base;
	typedef typename Base::PT PT;
public:
	explicit
	StackPtr()
		:	Base()
	{}
	
	StackPtr(PT p)
		:	Base(p)
	{}
	StackPtr(const StackPtr& o)
		:	Base(o)
	{}
	inline
	StackPtr& operator=(PT p)
	{
		Base::operator=(p);
		return *this;
	}
	inline
	StackPtr& operator=(const StackPtr& o)
	{
		Base::operator=(o);
		return *this;
	}
};

}

#if defined(_MSC_VER)
#	undef for
#	pragma warning(default:4786)
#	pragma warning(default:4521)
#	pragma warning(default:4522)
#	pragma warning(default:4355)
#	pragma warning(default:4291)
#endif


#endif
