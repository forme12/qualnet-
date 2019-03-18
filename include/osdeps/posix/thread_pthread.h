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

#if !defined(__THREAD_PTHREAD_H__)
#define __THREAD_PTHREAD_H__

#include <pthread.h>
#include <errno.h>

typedef pthread_t UTIL_ThreadID;
#define UTIL_ReturnThreadID() pthread_self()
#define UTIL_ThreadIDsEqual(x, y) (pthread_equal(x, y))

typedef pthread_key_t UTIL_ThreadKey;
#define UTIL_ThreadKeyInit(keyAddr) (pthread_key_create(keyAddr, NULL))
#define UTIL_ThreadKeySet(key, value) (pthread_setspecific(key, value))
#define UTIL_ThreadKeyGet(key) (pthread_getspecific(key))

typedef pthread_mutex_t UTIL_Mutex;
#define UTIL_MutexInit(mutexAddr) (pthread_mutex_init(mutexAddr, NULL))
static int UTIL_MutexLockInternal(
    UTIL_Mutex *mutexAddr,
    const char *filename,
    const int lineNum)
{
    int retVal = pthread_mutex_lock(mutexAddr);

    switch(retVal)
    {
        case 0: // ok
        {
            return 0;
        }
        case EINVAL:
        {
            fprintf(stderr, "Invalid mutex in UTIL_MutexLock, %s line %d\n",
                    filename, lineNum);
            break;
        }
        case EDEADLK:
        {
            fprintf(stderr, "A deadlock would occur on this UTIL_MutexLock "
                    "%s line %d\n", filename, lineNum);
            break;
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
#define UTIL_MutexUnlock(mutexAddr) (pthread_mutex_unlock(mutexAddr))

#endif // __THREAD_PTHREAD_H__
