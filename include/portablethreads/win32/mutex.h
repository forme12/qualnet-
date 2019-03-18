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
 
#ifndef WIN32_MUTEX_H
#define WIN32_MUTEX_H
#include <portablethreads/config.h>
#include <portablethreads/exception.h>
#include <windows.h>

namespace PortableThreads 
{
	namespace OSSpecific
	{
		class PTMutex
		{
		public:
			~PTMutex()
			{
				CloseHandle(mutex_);
			}
			// create a mutex. Default constructor creates unlocked mutex
			PTMutex(bool locked = false) 
				:	mutex_(locked ? CreateSemaphore(0, 0, 1, 0) : CreateSemaphore(0, 1, 1, 0))
			{
#ifdef _WIN64
// 8-2007: MSVC for 64 bit windows is flawed and bug filled. It will not 
// correctly compile/link std::runtime_error::riase () of PTResourceError
				// if(mutex_ == NULL)
					//throw PTResourceError("[PTMutex] Could not create semaphore");
#else
				if(mutex_ == NULL)
					throw PTResourceError("[PTMutex] Could not create semaphore");
#endif
			}
		  	// try to lock the mutex. Returns true on success, false otherwise
			inline bool tryLock()
  			{
      			return WaitForSingleObject(mutex_, 0) == WAIT_OBJECT_0;			
			}
			// lock the mutex. If the mutex is currently locked block till it unlocks
			inline void lock()
  			{
    			while(WaitForSingleObject(mutex_, INFINITE) != WAIT_OBJECT_0);
			}
			// unlock the mutex. This is a noop if the mutex is unlocked
			inline void unlock()
  			{
    			ReleaseSemaphore(mutex_, 1, 0);    		
  			}		
		private:
			// Mutexes cannot be copied or assigned
			PTMutex(const PTMutex&);
  			PTMutex& operator=(const PTMutex&);
		private:
			HANDLE mutex_;
		};
	}
}
#endif
