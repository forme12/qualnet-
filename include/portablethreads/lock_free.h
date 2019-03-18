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

#ifndef PT_LOCK_FREE_H
#define PT_LOCK_FREE_H
#include <portablethreads/lockfree/stack.h>
#include <portablethreads/lockfree/queue.h>
#include <portablethreads/lockfree/heap.h>
#include <portablethreads/lockfree/allocator.h>
#include <portablethreads/lockfree/atomic_number.h>

#ifdef _MSC_VER
// This warning also appears when a 32-bit pointer gets cast into a 4 byte int
#	pragma warning(disable:4311) // A 64-bit pointer was truncated to a 32-bit int or 32-bit long.
// This warning also appears when a 32-bit int gets cast into a 32-bit pointer
#	pragma warning(disable:4312) // You attempted to assign a 32-bit value to a 64-bit integer. For example, casting a 32-bit int or 32-bit long to a 64-bit pointer
#endif

namespace PortableThreads 
{
	namespace LockFree
	{
	}
}


#ifdef _MSC_VER
#	pragma warning(default:4311)
#	pragma warning(default:4312)
#endif

#endif

