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
// PACKAGE :: TYPES
// DESCRIPTION :: This file contains the portable types definitions and utils.
// **/
#ifndef QUALNET_TYPES_H
#define QUALNET_TYPES_H

#ifndef _WINDEF_

typedef int BOOL;

#ifndef FALSE
#define FALSE (0)
#define TRUE (1)
#endif

#endif // _WINDEF_

//#define WCHARS
#if defined (WIN32) || defined (_WIN32) || defined (_WIN64)
#define WINDOWS_OS
#ifdef _WIN64
#define P64
#endif
#elif defined (__unix)
#if defined (__LP64__) || ( __WORDSIZE == 64 )
#define LP64
#endif
#endif

// wide chars types and functions mapping
// -------------------------------------------------------------
#ifdef WCHARS
#include <wchar.h>
typedef wchar_t CHARTYPE;
#define CHARTYPE_strstr    wcsstr
#define CHARTYPE_strcpy    wcscpy
#define CHARTYPE_strncpy   wcsncpy
#define CHARTYPE_vsscanf   vswscanf
// these APIs are different between windows and unix
#ifdef WINDOWS_OS
#define CHARTYPE_vsnprintf _vsnwprintf
#else
#define CHARTYPE_vsnprintf vswprintf
#endif // WINDOWS_OS
#define CHARTYPE_sscanf    swscanf
#define CHARTYPE_printf    wprintf
#define CHARTYPE_sprintf   wsprintf
#define CHARTYPE_strlen    wcslen
#define CHARTYPE_Cast(x)      L##x
#else

// single byte chars types and functions mapping
// -------------------------------------------------------------
typedef char    CHARTYPE;
#define CHARTYPE_strstr    strstr
#define CHARTYPE_strcpy    strcpy
#define CHARTYPE_strncpy   strncpy
#define CHARTYPE_vsscanf   vsscanf
// these APIs are different between windows and unix
#ifdef WINDOWS_OS
#define CHARTYPE_vsnprintf _vsnprintf
#else
#define CHARTYPE_vsnprintf vsnprintf
#endif // WINDOWS_OS
#define CHARTYPE_sscanf    sscanf
#define CHARTYPE_printf    printf
#define CHARTYPE_sprintf   sprintf
#define CHARTYPE_strlen    strlen
#define CHARTYPE_Cast(x)      x
#endif

// unified type definitions

// /**
// CONSTANT :: PTRSIZE : depends on platform
// DESCRIPTION :: size of pointer in bits
// **/

// /**
// CONSTANT :: LONGSIZE : depends on platform
// DESCRIPTION :: size of long integer in bits
// **/

// /**
// TYPE :: Int8
// DESCRIPTION :: 8-bit signed integer
// **/

// /**
// TYPE :: UInt8
// DESCRIPTION :: 8-bit unsigned integer
// **/

// /**
// TYPE :: Int16
// DESCRIPTION :: 16-bit signed integer
// **/

// /**
// TYPE :: UInt16
// DESCRIPTION :: 16-bit unsigned integer
// **/

// /**
// TYPE :: Int32
// DESCRIPTION :: 32-bit signed integer
// **/

// /**
// TYPE :: UInt32
// DESCRIPTION :: 32-bit unsigned integer
// **/

// /**
// TYPE :: Int64
// DESCRIPTION :: 64-bit signed integer
// **/

// /**
// TYPE :: UInt64
// DESCRIPTION :: 64-bit unsigned integer
// **/

// /**
// TYPE :: Float32
// DESCRIPTION :: 32-bit floating-point number
// **/

// /**
// TYPE :: Float64
// DESCRIPTION :: 64-bit floating-point number
// **/

// /**
// TYPE :: IntPtr
// DESCRIPTION :: integer big enough to hold a pointer
// **/

// 64bit Windows
// -------------------------------------------------------------
#ifdef P64
#define PTRSIZE         64
#define LONGSIZE        32
typedef char            Int8;
typedef unsigned char   UInt8;
typedef short           Int16;
typedef unsigned short  UInt16;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef _int64          Int64;
typedef unsigned _int64 UInt64;
typedef _int64          TimeStampPtr;
typedef float           Float32;
typedef double          Float64;
typedef Int64           IntPtr; // integer big enough to hold a pointer
#else

// 64bit Unix/MacOS X
// -------------------------------------------------------------
#ifdef LP64
#define PTRSIZE         64
#define LONGSIZE        64
typedef char            Int8;
typedef unsigned char   UInt8;
typedef short           Int16;
typedef unsigned short  UInt16;
typedef int             Int32;
typedef unsigned int    UInt32;
typedef long            Int64;
typedef unsigned long   UInt64;
typedef Int64           TimeStampPtr;
typedef float           Float32;
typedef double          Float64;
typedef Int64           IntPtr; // integer big enough to hold a pointer
#else

// 32bit Windows/Unix
// -------------------------------------------------------------
#define PTRSIZE         32
#define LONGSIZE        32
typedef char            Int8;
typedef unsigned char   UInt8;
typedef short           Int16;
typedef unsigned short  UInt16;
typedef int             Int32;
typedef unsigned int    UInt32;
#ifdef WINDOWS_OS
typedef _int64          Int64;
typedef unsigned _int64 UInt64;
#else
typedef long long       Int64;
typedef unsigned long long UInt64;
#endif
typedef int             TimeStampPtr;
typedef float           Float32;
typedef double          Float64;
typedef Int32           IntPtr; // integer big enough to hold a pointer

#endif // LP64
#endif //  P64

// The following macro helps in declaring a value that occupies
// 64 bit but is accessed as a different type
// Used when we want to make sure some structures have the same
// memory footprint on all platforms
#define TYPES_SIZE64BIT(n)           union { n; Int64 size64BitSpaceHolder; }
#define TYPES_SIZE64BITARRAY(n,s)    union { n[s]; Int64 size64BitSpaceHolder[s]; }

// The following macros help in declaring 64 bit imediate values
// :will append the proper suffix code to indicate a 64bit value
// WINDOWS
// -------------------------------------------------------------
#ifdef WINDOWS_OS
#define TYPES_ToInt64(n)   n##i64
#define TYPES_ToUInt64(n)  n##ui64
// UNIX
// -------------------------------------------------------------
#else
#define TYPES_ToInt64(n)   n##ll
#define TYPES_ToUInt64(n)  n##ull
#endif // WINDOWS_OS

#if PTRSIZE == 32
#define TYPES_ToIntPtr(n)  n
#else
#define TYPES_ToIntPtr(n)  TYPES_ToInt64(n)
#endif

#ifdef WINDOWS_OS
#define TYPES_64BITFMT "I64"
#else /* UNIX */
#ifdef LP64
#define TYPES_64BITFMT "l"
#else
#define TYPES_64BITFMT "ll"
#endif
#endif

#ifdef WINDOWS_OS
#define TYPES_SIZEOFMFT ""
#else /* UNIX */
#ifdef LP64
#define TYPES_SIZEOFMFT "l"
#else
#if defined (__APPLE__) || defined (__MACH__)
#define TYPES_SIZEOFMFT "z"
#else
#define TYPES_SIZEOFMFT ""
#endif
#endif
#endif

#endif // QUALNET_TYPES_H
