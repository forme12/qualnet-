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

#ifndef PT_MUTEX_H
#define PT_MUTEX_H
#include <portablethreads/config.h>
#ifdef PT_WINDOWS
#	include <portablethreads/win32/mutex.h>
#endif
#ifdef PT_UNIX
#	include <portablethreads/unix/mutex.h>
#endif

namespace PortableThreads
{
	/*! \class PTMutex mutex.h portablethreads/mutex.h
		\brief Mutex

		PTMutex implements a mutual exclusion object. Threads that are waiting to
		acquire the object are suspended in their execution until the object becomes
		available.
	*/
	class PTMutex : private OSSpecific::PTMutex
	{
		typedef OSSpecific::PTMutex Base;
	public:
		/*! \brief Create a mutex object

			\param locked If true the mutex is created locked, unlocked otherwise.

			\exception PTResourceError Thrown if a operating system mutex cannot be created.
		*/
		PTMutex(bool locked = false)
			:	Base(locked)
		{}
		/*! \brief Attempt to lock the mutex.

			\retval true The calling thread successfully locked (acquired) the mutex.
			\retval false Locking attempt failed.
		*/
		inline bool tryLock() 
		{ 
			return Base::tryLock();		
		}
		/*! \brief Lock the mutex.

			The calling thread is blocked until it has successfully locked (acquired)
			the mutex.
		*/
		inline void lock() 
		{ 
			Base::lock();	
		}
		/*! \brief Unlock the mutex.

			If the mutex was unlocked this call has no effect. If the mutex was
			locked it is unlocked.
			In case there are threads waiting to acquire the mutex, one of the threads
			calls to lock() or tryLock() succeeds when the unlock() call takes effect.
		*/
		inline void unlock() 
		{ 
			Base::unlock();	
		}
	};
}



#endif
