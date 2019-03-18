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
 
#ifndef WIN32_THREAD_H
#define WIN32_THREAD_H

#include <portablethreads/config.h>
#include <portablethreads/exception.h>
#include <windows.h>
#include <memory>

namespace PortableThreads 
{
	namespace OSSpecific
	{
		struct ThreadTraits
		{
			typedef struct { HANDLE handle_; DWORD id_; } thread_t;
			typedef DWORD os_id_t;
			typedef DWORD (WINAPI entry_function_t)(void*);
			inline static void create(thread_t& t, entry_function_t* entry, void* arg)
			{
				DWORD temp;
				const HANDLE h = CreateThread(0, 0, entry, arg, 0, &temp);
				if(h == NULL)
					throw PTResourceError("[PThread] Could not create thread");
				
				t.handle_ = h;
				t.id_ = temp;
			}
			static inline void kill(thread_t& t)
			{
				TerminateThread(t.handle_, 0);
			}
			static inline void initialize(thread_t& t)
			{
				t.handle_ = NULL;
				t.id_ = 0;
			}
			static inline os_id_t self() { return GetCurrentThreadId(); }
			static inline void yield() { Sleep(0); }
			static inline void free_resource(thread_t& t)
			{
				CloseHandle(t.handle_);
			}
			static inline bool equal(os_id_t lhs, os_id_t rhs)
			{
				return lhs == rhs;
			}
			static inline os_id_t thread_id(const thread_t& t)
			{
				return t.id_;
			}
		};
	}

	// No comparison operators for os_id_t -> DWORD builtin

}

#endif
