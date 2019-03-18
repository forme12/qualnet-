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
/*
For win64 we have no atmoic asm instructions because msvc for
windows64 doesn't support _asm directives. A seperate .asm compilation
file could be created, but windows64 isn't a useful platform currently
so it isn't worth the effort.
*/

#ifndef X86_32_WIN32_MSVC_H
#define X86_32_WIN32_MSVC_H

#ifndef _MSC_VER
#	error "You must use a MS compatible compiler in order to use this header file!"
#endif

#define PT_HAVE_CAS
#define PT_HAVE_CAS2


#include <windows.h>

#if _MSC_VER <= 1200
#	pragma warning(disable:4786) //'identifier' : identifier was truncated to 'number' characters in the debug information
#	pragma warning(disable:4503)
#endif

namespace PortableThreads 
{
	typedef signed char int8;
	typedef unsigned char uint8;
	typedef short int16;
	typedef unsigned short uint16;
	typedef int int32;
	typedef unsigned int uint32;
	typedef __int64 int64;
	typedef unsigned __int64 uint64;


}


#include <portablethreads/arch/arch-common.h>

#define PT_POINTER_CAS_INCLUDED

#define PT_HAVE_CAS
#define PT_HAVE_CAS2

#endif
