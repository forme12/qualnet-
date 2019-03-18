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

#ifndef PT_CONDITION_H
#define PT_CONDITION_H
#include <portablethreads/config.h>
#include <portablethreads/semaphore.h>
#include <portablethreads/lockfree/atomic_number.h>

namespace PortableThreads
{
	/*! \class PTCondition condition.h portablethreads/condition.h
		\brief Condition variable.
	*/
	class PTCondition
	{
	public:
		~PTCondition();
		/*!	\brief Create a condition variable 

			\param autoreset If set to *true* the condition variable will reset
				itself to the non-signaled state after it has been signaled.
			\param signaled If set to *true* the condition variable is created 
				signaled.

			\exception PTParameterError A condition variable cannot be created autresetting and signaled.
		*/
		PTCondition(bool autoreset = true, bool signaled = false);
		/*! \brief Causes waiting thread to become unblocked.

			The invokation of the method releases any waiting threads that are currently
			blocking on the condition variable. N Concurrent invokations of the method
			may result in the condition variable to become signaled anywhere from 1 to N 
			times.
		*/
		void signal();
		/*! \brief Resets a signaled condition variable to the non-signaled state.

			If the condition variable is in the non-signaled state or the
			condition variable uses autoressting (see PTCondition(bool, bool))
			the invokation of this method has no effect.
		*/
		void reset();
		/*! \brief Wait for the condition variable to become signaled.

			The execution of the calling thread is suspended until signal()
			is called.
		*/
		void wait();
		/*! \brief Wait for the condition variable to become signaled.

			The execution of the calling thread is suspended for at period of 
			atmost 
				\a seconds * 1000 + \a milliseconds ms. 
			If during this time signal() is called at
			least once, the waiting thread is unblocked. 

			During timed waiting a thread is woken every
			\a wakeup ms to see whether the condition
			variable became signaled. Smaller values increase
			the accuracy of the timed wait but consume more
			resources. If the parameter is not zero the
			wait period is set to \wakeup . Otherwise
			the wait period remains unchanged. 
			The default value is 100.

			\param seconds Seconds to sleep.
			\param seconds Milliseconds to add to sleep.
			\param wakeup Interrupt wait every \a wakeup ms to check if the condition became signaled.

			\retval true The condition variable became signaled during the wait.
			\retval false The condition variable did not become signaled during the wait.
		*/
		bool wait(unsigned seconds, unsigned milliseconds = 0, unsigned wakeup = 100);
	private:
		PTCondition(const PTCondition&);
		PTCondition& operator=(const PTCondition&);
	private:
		typedef LockFree::PTAtomicNumber::int_type int_type;
		PTSemaphore sem_;
		LockFree::PTAtomicNumber waiters_;
		volatile int incarnation_;
		bool autoreset_;
	};
}

#endif
