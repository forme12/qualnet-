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
 
#ifndef WIN32_SEMAPHORE_H
#define WIN32_SEMAPHORE_H
#include <portablethreads/config.h>
#include <portablethreads/exception.h>
#include <windows.h>

namespace PortableThreads 
{
	namespace OSSpecific
	{
		class PTSemaphore
		{
		public:
			static const unsigned MAX = static_cast<unsigned>(-1) >> 1;
			PTSemaphore(unsigned initially = 0) 
				:	sem_(CreateSemaphore(0, initially > MAX ? MAX : initially, MAX, 0))
			{
				if(sem_ == NULL)
					throw PTResourceError("[PTSemaphore] Could not create semaphore");
			}
		  	
			~PTSemaphore()
			{
				CloseHandle(sem_);
			}
			// try to get a bit of the semaphore's count. If successfull,
			// return true, else false
			inline bool tryDown()
  			{
      			return WaitForSingleObject(sem_, 0) == WAIT_OBJECT_0;			
			}
		    
			// wait for a share of the semaphore's count. Block while
			// waiting
			inline void down()
  			{
    			while(WaitForSingleObject(sem_, INFINITE) != WAIT_OBJECT_0);
			}
			// release one's share of the semaphore's count. Never blocks.
			inline void up()
  			{
    			ReleaseSemaphore(sem_, 1, 0);    		
  			}
		private:
			// Semaphores cannot be copied or assigned
			PTSemaphore(const PTSemaphore&);
  			PTSemaphore& operator=(const PTSemaphore&);
		private:
			HANDLE sem_;
		};
	}
}

#endif
