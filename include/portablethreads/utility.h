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

#ifndef PT_UTILITY_H
#define PT_UTILITY_H
#include <portablethreads/config.h>

namespace PortableThreads 
{
	/*! \class PTGuard utility.h portablethreads/utility.h
		\brief Scope guard that locks on creation and unlocks on destruction.
	*/
	template<typename T> 
	class PTGuard
	{
	public:
		//!	This template class takes a mutex like parameter and locks it when the object is created.
 		PTGuard(T& mutex) 
			:	mutex_(mutex)
		{
			mutex_.lock();
		}
		
		//!  When the object is destroyed, the mutex is unlocked.
		~PTGuard()
		{
			mutex_.unlock();
		}
	private:
		T& mutex_;
	};

	/*! \class PTReverseGuard utility.h portablethreads/utility.h
		\brief Scope guard that unlocks on creation and locks on destruction.
	*/
	template<typename T> 
	class PTReverseGuard
	{
	public:
		//! Unlocks the mutex parameter on creation.
		PTReverseGuard(T& mutex) 
			:	mutex_(mutex)
		{
			mutex_.unlock();
		}
		//! Locks the mutex parameter on creation.
		~PTReverseGuard()
		{
			mutex_.lock();
		}
	private:
		T& mutex_;
	};
}

#endif
