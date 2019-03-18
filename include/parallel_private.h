// Copyright (c) 2001-2009, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive
//                          Suite 1250
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the QualNet
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

// /**
// PACKAGE     :: PARALLEL_PRIVATE
// DESCRIPTION :: This file contains declarations & definitions for internal parallel implementation.
// **/

#ifndef PARALLEL_PRIVATE_H
#define PARALLEL_PRIVATE_H

// Definition of PTQueue - CPU defines used to select the correct assembly language

// Windows64 - msvc with 64 bit
// This will just use generic



#ifdef  __POWERPC__
// PowerPC - Mac with gcc 64 bit
// This will Just use slow pthreads, but PowerPC isn't likely to have
// external interfaces anyway.
// For the portablethreads library we are only going to be using basic
// libpthread type functions (and thus no assembly language constructs)
// so we can pretend to be a sparc-based-solaris os
#  define PT_SOLARIS
#  define PT_SPARC
#  define PT_NATIVE_WORD 32
#  define PT_V9
# include "portablethreads/generic_ptqueue.h"
#else
# if defined (__GNUC__) && defined (__APPLE__) && defined (__i386)
// x86 macintosh
#  define PT_X86
#  define PT_NATIVE_WORD 32
#  define PT_LINUX
# elif defined (__sparc)
// Sparc - Solaris with sparc v9 (32 bit)
#  define PT_SOLARIS
#  define PT_SPARC
#  define PT_NATIVE_WORD 32
#  define PT_V9
# else
// x86 (winodws and linux)
#  define PT_X86
#  if defined (LP64) || ( __WORDSIZE == 64 ) || (_WIN64)
// 64 Bit
#   define PT_NATIVE_WORD 64
#   if defined (_WIN64)
// Windows64 - msvc with 64 bit
// This will just use generic normal windows critical sections
// but windows 64 isn't a common or preferred platform yet anyway.
#      define PT_WINDOWS
#      include "portablethreads/generic_ptqueue.h"
#   else
#      define PT_LINUX
#   endif
#  else
// 32 Bit
#   define PT_NATIVE_WORD 32
#   if defined (WIN32) || defined (_WIN32) 
#      define PT_WINDOWS
#   else
#      define PT_LINUX
#   endif
#  endif // WORDSIZE
# endif // __sparc

# undef GetObject
# undef interface
# if !defined (_WIN64)
# if defined(_WIN32) || defined(USE_MPI)
#  include <portablethreads/lockfree/queue.h>
#endif
# endif
#endif  // __POWERPC__

#endif
