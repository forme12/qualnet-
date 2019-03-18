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

#if !defined(__UTIL_ATOMIC_H__)
#define __UTIL_ATOMIC_H__

#define CONFIG_SMP

#if defined(__sparc__)
  #define USE_ATOMIC_PTHREAD
#elif defined(__MACH__) || defined(__linux__) || defined(__OpenBSD__) || defined(__linux)
  #define USE_ATOMIC_ASM
#elif defined(__CYGWIN__) && defined(__i386__)
  #define USE_ATOMIC_ASM
#elif defined(_WIN32)
  #define USE_ATOMIC_WIN
#else
  #define USE_ATOMIC_GENERIC
#endif


#if defined(USE_ATOMIC_PTHREAD)
//  #warning "Using pthread atomics"
  #include "../include/osdeps/posix/pthread_atomic.h"
#elif defined(USE_ATOMIC_ASM)
  #if defined(__ppc__)
//  #warning "Using ppc asm atomics"
    #include "../include/osdeps/darwin/ppc_atomic.h"
  #elif defined(__i386__)
//  #warning "Using i386 asm atomics"
    #include "../include/osdeps/linux/i386_atomic.h"
  #elif defined(__x86_64)
    // #include <asm/atomic.h>
    #include "../include/osdeps/linux/x86-64_atomic.h"
  #endif
#elif defined(USE_ATOMIC_WIN)
//   #warning "Using i386/win asm atomics"
//   #define WIN32_USE_ASM
  #include "../include/osdeps/windows/i386_atomic.h"
#elif defined(USE_ATOMIC_GENERIC)
  #ifndef _WIN32
    #warning "Using generic (non-atomic) atomics -- not thread safe"
  #endif
  #include "../include/osdeps/generic/generic_atomic.h"
#else
  #error "Should have picked one of the above options"
#endif

static void UTIL_AtomicSetFloat(UTIL_AtomicInteger *v, float x) {
    int ix = *(int*)&x;
    UTIL_AtomicSet(v, ix);
}

static float UTIL_AtomicReadFloat(UTIL_AtomicInteger *v) {
    int ix = UTIL_AtomicRead(v);
    return *(float*)&ix;
}

#ifndef _LP64
static void UTIL_AtomicSetGP(UTIL_AtomicInteger *v, void *x) {
    int ix = (int)x;
    UTIL_AtomicSet(v,ix);
}

static void *UTIL_AtomicReadGP(UTIL_AtomicInteger* v) {
    int ix = UTIL_AtomicRead(v);
    return (void*)ix;
}
#endif //!defined(_LP64)


#ifdef USE_THREADS
  #if (defined(_WIN32))
    #include <windows.h>

    typedef HANDLE UTIL_ThreadID;
    #define UTIL_ReturnThreadID() GetCurrentThread()
    #define UTIL_ThreadIDsEqual(x, y) (x == y)

    typedef int UTIL_ThreadKey;
    #define UTIL_ThreadKeyInit(keyAddr) ( (*keyAddr = TlsAlloc()) == 0 )
    #define UTIL_ThreadKeySet(key, value) (TlsSetValue(key, value))
    #define UTIL_ThreadKeyGet(key) (TlsGetValue(key))
  #else // _WIN32
    #include <pthread.h>

    typedef pthread_t UTIL_ThreadID;
    #define UTIL_ReturnThreadID() pthread_self()
    #define UTIL_ThreadIDsEqual(x, y) (pthread_equal(x, y))

    typedef pthread_key_t UTIL_ThreadKey;
    #define UTIL_ThreadKeyInit(keyAddr) (pthread_key_create(keyAddr, NULL))
    #define UTIL_ThreadKeySet(key, value) (pthread_setspecific(key, value))
    #define UTIL_ThreadKeyGet(key) (pthread_getspecific(key))
  #endif // _WIN32
#else
  typedef int UTIL_ThreadID;
  #define UTIL_ReturnThreadID() (0)
  #define UTIL_ThreadIDsEqual(x, y) (TRUE)

  typedef int** UTIL_ThreadKey;
  #define UTIL_ThreadKeyInit(keyAddr) ((*keyAddr = &MEM_GlobalKey) == NULL)
  #define UTIL_ThreadKeySet(key, value) (*key = (int *)value)
  #define UTIL_ThreadKeyGet(key) (*key)
#endif

#endif // __UTIL_ATOMIC_H__
