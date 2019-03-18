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
 
#ifndef PT_SPINLOCK_H
#define PT_SPINLOCK_H
#include <portablethreads/config.h>

namespace PortableThreads 
{
	/*! \class PTSpinlock spinlock.h portablethreads/spinlock.h
		\brief Spinlock

		PTSpinlock implements a mutual exclusion object. Threads that are waiting to
		acquire the object are busy waiting on the object until they acquire it.
	*/
	class PTSpinlock
	{
	public:
		/*! \brief Create a spinlock object.

			\param locked If true, the spinlock is locked, unlocked otherwise.
		*/
		PTSpinlock(bool locked = false)
			:	lock_(0)
		{
			if(locked)
				lock();
		}
		/*! \brief Spin the lock until it is locked.

			Busy-wait for the spinlock to become available. During
			the wait the calling thread consumes massive amounts of
			CPU resources.
		*/
		inline void lock()
		{
			bool done;
			do { done = LockFree::Private::pt_atomic_set_lock(&lock_); } while(!done);
		}
		/*! \brief Take one shot at acquiring the lock.

			\retval true Lock was acquired by the calling thread.
			\retval false Lock could not be acquired by the calling thread.
		*/
		inline bool tryLock()
		{
			return LockFree::Private::pt_atomic_set_lock(&lock_);
		}
		/*! \brief Unlock the spinlock.

			Calling this method will enable one other thread which is
			currently spinning on the lock (trying to lock it) to
			acquire the lock.
		*/
		inline void unlock()
		{
			LockFree::Private::pt_atomic_clear_lock(&lock_);
		}
	private:
		PTSpinlock(const PTSpinlock&);
		PTSpinlock& operator=(const PTSpinlock&);
	private:
		volatile uint8 lock_;
	};
}

#endif

