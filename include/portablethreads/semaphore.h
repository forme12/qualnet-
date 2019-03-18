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

#ifndef PT_SEMAPHORE_H
#define PT_SEMAPHORE_H
#include <portablethreads/config.h>
#ifdef PT_WINDOWS
#	include <portablethreads/win32/semaphore.h>
#endif
#ifdef PT_UNIX
#	include <portablethreads/unix/semaphore.h>
#endif

namespace PortableThreads
{
	/*! \class PTSemaphore semaphore.h portablethreads/semaphore.h
		\brief Semaphore

		PTSemaphore implements a semaphore object. Threads that are waiting on the
		semaphore are suspended until the semaphore's count is greater than 0.

		The count value of a semaphore determines how many threads may call
		PTSemaphore::down() without blocking.
	*/	
	class PTSemaphore : private OSSpecific::PTSemaphore
	{
		typedef OSSpecific::PTSemaphore Base;
	public:
		/*! \brief Create a semaphore object.

			\param initially Initial value of the semaphore's count.

			\exception PTResourceError Thrown if no operating system semaphore can be allocated.
		*/
		PTSemaphore(unsigned initially = 0)
			:	Base(initially)
		{}
		/*! \brief Attempt to decrement the semaphore's counter.

			\retval true The counter was greater than zero prior to the call.
			\retval false The counter was less or equal to zero prior to the call.
		*/
		inline bool tryDown()
  		{
			return Base::tryDown();
		}
		/*! \brief Decrement the semaphore's counter.

			The calling thread is suspended if the counter is zero or
			less prior to the called.
		*/
		inline void down()
  		{ 
    		Base::down();
		}
		/*! \brief Increment the semaphore's counter.

			If the counter was less than zero prior to the call one thread
			which has called down() is resumed.
		*/
		inline void up()
  		{
  			Base::up();
  		}
	};
}


#endif
