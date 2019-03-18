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

#if !defined(__THREAD_NTTHREAD_H__)
#define __THREAD_NTTHREAD_H__

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

typedef HANDLE UTIL_ThreadID;
#define UTIL_ReturnThreadID() GetCurrentThread()
#define UTIL_ThreadIDsEqual(x, y) (x == y)

typedef int UTIL_ThreadKey;
#define UTIL_ThreadKeyInit(keyAddr) ( (*keyAddr = TlsAlloc()) == 0 )
#define UTIL_ThreadKeySet(key, value) (TlsSetValue(key, value))
#define UTIL_ThreadKeyGet(key) (TlsGetValue(key))

typedef HANDLE UTIL_Mutex;
#define UTIL_MutexInit(mutexAddr) (*mutexAddr = CreateMutex(NULL, FALSE, ""))
// #define UTIL_MutexLock(mutexAddr) (WaitForSingleObject(*mutexAddr, INFINITE))
static int UTIL_MutexLockInternal(
    UTIL_Mutex *mutexAddr,
    const char *filename,
    const int lineNum)
{
    int retVal = WaitForSingleObject(*mutexAddr, INFINITE);

    switch(retVal)
    {
        case WAIT_OBJECT_0: // ok
        {
            return 0;
        }
        default:
        {
            fprintf(stderr, "Unknown error %d in  UTIL_MutexLock %s line %d\n",
                    retVal, filename, lineNum);
            break;
        }
    }

    abort();
}
#define UTIL_MutexLock(mutexAddr) (UTIL_MutexLockInternal(mutexAddr, __FILE__, __LINE__))
#define UTIL_MutexUnlock(mutexAddr) (ReleaseMutex(*mutexAddr))

#endif // __THREAD_NTTHREAD_H__
