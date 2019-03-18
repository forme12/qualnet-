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

#ifndef PT_TIME_H
#define PT_TIME_H
#include <portablethreads/config.h>
#include <cassert>

namespace PortableThreads 
{
	/*!	\brief Suspends the calling thread.
		\param time Seconds to suspend the calling thread.
	*/
	void pt_second_sleep(unsigned time);
	/*!	\brief Suspends the calling thread.
		\param time Milliseconds to suspend the calling thread.
	*/
	void pt_milli_sleep(unsigned time);
	/*!	\brief Suspends the calling thread.
		\param time Microseconds to suspend the calling thread.
	*/
	void pt_micro_sleep(unsigned time);
	//! \brief Causes the calling thread to give up the CPU.
	void pt_yield();

	
	/*! \class PTime time.h portablethreads/time.h 
		\brief Time stamps and measuring. 
	
		On UNIX systems this has (should have) micro second precision
		on WIN32 systems precision depends on the hardware but should be better or equivalent to
		UNIX precision.
		
		PTime also provides a stop clock interface (via start, stop, difference) and time stamps.
	*/
	class PTime
	{
	public:
		typedef uint64 time_type; //!< unsigned integer type used for time measurement
		~PTime();
		PTime();
		//! Obtain the current time stamp.
		static time_type stamp();
		//! Get the frequency of the clock in Hz.
		static time_type frequency();
		//! Starts (and resets) the stopwatch
		void start();
		//! Stops the stopwatch
		void stop();
		/*! \brief Get the time elapsed between a call to start() and a call to stop().
		
			Divide this value by the return value of frequency()
			to get the time in seconds.
			NOTE: Do not use integer division here, the time measured may
			be well less than one second.
		*/
		time_type difference() const;
	private:
		static time_type calculateFrequency();
	private:
		time_type start_, end_;
	private:
		static const time_type frequency_;
	};
}

#endif
