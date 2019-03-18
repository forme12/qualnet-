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

#ifndef PT_CONFIG_H
#define PT_CONFIG_H


// Windows detection
#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) || defined(__MINGW32__) || defined(__DMC__)
#	ifndef PT_WINDOWS
#		define PT_WINDOWS
#	endif
#	ifndef PT_X86
#		define PT_X86
#	endif
#endif

// Size detection for Windows OS with MS VC compatible compiler
#if defined(_WIN32) && !defined(_WIN64)
#	ifdef PT_NATIVE_WORD
#		undef PT_NATIVE_WORD
#	endif
#	define PT_NATIVE_WORD 32
#endif

#if defined(_WIN32) && defined(_WIN64)
#	ifndef PT_NATIVE_WORD // in case we don't want to specify ourselves, assume 64 bit mode
#		define PT_NATIVE_WORD 64
#	endif
#endif

// Flag errors...
#if !defined(PT_WINDOWS) && !defined(PT_SOLARIS) && !defined(PT_LINUX)
#	error "You need to define either PT_WINDOWS, PT_SOLARIS, or PT_LINUX!"
#endif
#if !defined(PT_NATIVE_WORD)
#	error "You need to define PT_NATIVE_WORD to be either 32 or 64!"
#endif
#if !defined(PT_X86) && !defined(PT_SPARC) && !defined(PT_IA64)
#	error "You need to define either PT_X86, PT_SPARC, or PT_IA64!"
#endif

// architecture & os specific code
#ifdef PT_WINDOWS
#	ifdef _MSC_VER
#		if PT_NATIVE_WORD == 32
#			include <portablethreads/arch/x86-32-win32-msvc.h>
#			define PT_COMPATIBLE_FOUND
#		endif
#		if PT_NATIVE_WORD == 64
#			include <portablethreads/arch/x86-64-win64-msvc.h>
#			define PT_COMPATIBLE_FOUND
#		endif
#	endif
#	ifdef __MINGW32__
#		if PT_NATIVE_WORD == 32
#			include <portablethreads/arch/x86-32-win32-mingw.h>
#			define PT_COMPATIBLE_FOUND
#		endif
#	endif
#	ifdef __DMC__ // Digital Mars
#		if PT_NATIVE_WORD == 32
#			include <portablethreads/arch/x86-32-win32-dmc.h>
#			define PT_COMPATIBLE_FOUND
#		endif
#	endif
#endif
#ifdef PT_LINUX
#	ifndef PT_UNIX
#		define PT_UNIX
#	endif
#	ifdef __GNUC__
#		ifdef PT_X86
#			if PT_NATIVE_WORD == 32
#				include <portablethreads/arch/x86-32-linux-gcc.h>
#				define PT_COMPATIBLE_FOUND
#			endif
#			if PT_NATIVE_WORD == 64
#				include <portablethreads/arch/x86-64-linux-gcc.h>
#				define PT_COMPATIBLE_FOUND
#			endif			
#		endif
#		ifdef PT_SPARC
#			if PT_NATIVE_WORD == 32
#				if defined(PT_V9) || defined(PT_V8PLUS)
#					include <portablethreads/arch/sparc-32-v8plus-linux-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#				ifdef PT_V8
#					include <portablethreads/arch/sparc-32-v8-linux-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#			endif
#			if PT_NATIVE_WORD == 64
#				ifdef PT_V9
#					include <portablethreads/arch/sparc-64-v9-linux-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#			endif
#		endif
#		ifdef PT_IA64
#			if PT_NATIVE_WORD == 64
#				include <portablethreads/arch/ia-64-linux-gcc.h>
#				define PT_COMPATIBLE_FOUND
#			endif			
#		endif
#	endif
#endif
#ifdef PT_SOLARIS
#	ifndef PT_UNIX
#		define PT_UNIX
#	endif
#	ifdef __GNUC__
#		ifdef PT_SPARC
#			if PT_NATIVE_WORD == 32
#				if defined(PT_V9) || defined(PT_V8PLUS)
#					include <portablethreads/arch/sparc-32-v8plus-sunos-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#				ifdef PT_V8
#					include <portablethreads/arch/sparc-32-v8-sunos-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#			endif
#			if PT_NATIVE_WORD == 64
#				ifdef PT_V9
#					include <portablethreads/arch/sparc-64-v9-sunos-gcc.h>
#					define PT_COMPATIBLE_FOUND
#				endif
#			endif
#		endif
#		ifdef PT_X86
#			if PT_NATIVE_WORD == 32
#				include <portablethreads/arch/x86-32-sunos-gcc.h>
#				define PT_COMPATIBLE_FOUND
#			endif	
#			if PT_NATIVE_WORD == 64
#				include <portablethreads/arch/x86-64-sunos-gcc.h>
#				define PT_COMPATIBLE_FOUND
#			endif	
#		endif
#	endif
#endif

// Check if we have detected a compatible configuration
#ifndef PT_COMPATIBLE_FOUND
#	error "You are trying to compile Portable Threads with an unsupported compiler/define combination!"
#else
#	undef PT_COMPATIBLE_FOUND
#endif

// See to it that type definitions (int sizes, how to do pointer cas)
// are properly included
#if !defined(PT_ARCH_COMMON_INCLUDED) || !defined(PT_POINTER_CAS_INCLUDED)
#	error "Architectural files not properly included! Check platform defines!"
#else
#	undef PT_ARCH_COMMON_INCLUDED
#	undef PT_POINTER_CAS_INCLUDED
#endif


#endif
