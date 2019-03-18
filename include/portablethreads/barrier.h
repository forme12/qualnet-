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

#ifndef PT_BARRIER_H
#define PT_BARRIER_H
#include <portablethreads/config.h>
#include <portablethreads/semaphore.h>
#include <portablethreads/lockfree/atomic_number.h>

namespace PortableThreads 
{
	/*! \class PTBarrier barrier.h portablethreads/barrier.h
		\brief Implements a barrier at which a number of threads wait until the last thread arrives.
	*/
	class PTBarrier
	{
	public:
		~PTBarrier();
		/*! \brief Create a barrier
			
			\param n Create that blocks \a n threads until the last thread calls wait().

			\exception PTParameterError \a n must not be zero.
		*/
		PTBarrier(unsigned n);
		/*! \brief Wait at the barrier.

			Causes the first N (see PTBarrier(unsigned)) threads to block.
			The last of the N threads unlocks the N-1 waiting threads and
			resets the barrier.

			\retval true A thread waited successfully at the barrier.
			\retval false A thread either called wait() while N threads were 
				already waiting.
		*/
		bool wait();
	private:
		PTBarrier();
	private:
		typedef LockFree::PTAtomicNumber::int_type int_type;
		const int_type size_;
		LockFree::PTAtomicNumber counter_;
		PTSemaphore sem_;
	};
}

#endif
