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

#ifndef __QUALNET_MUTEX_H__
#define __QUALNET_MUTEX_H__

#ifdef _WIN32
// Internal DOC:
// On Windows, we use InitializeCriticalSectionAndSpinCount ()
// This function requires windows NT4+ (in msvc 6 this windows 2000)
// From MSDN:http://msdn.microsoft.com/library/default.asp?url=/library/en-us/dllproc/base/initializecriticalsectionandspincount.asp
//      To compile an application that uses this function,
//      define _WIN32_WINNT as 0x0403 or later. For more information,
//      see Using the Windows Headers.
// Note, as a consequence, if your source file has already included windows.h
// before including this file you will get compilation errors.
#define _WIN32_WINNT 0x0500
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#else
#include <pthread.h>
#endif //WIN32
#include "qualnet_error.h"


// /**
// PACKAGE     :: MUTEX
// DESCRIPTION :: This file describes objects for use in creating critical
//                regions (synchronized access) for global variables or
//                data structures that have to be shared between threads.
// **/

// /**
// CLASS    :: QNThreadMutex, QNParittionMutex, QNThreadLock, QNPartitionLock
//
// SUMMARY  :: You have two choices for the type of lock to use to protect
//             your critical-region/shared data:
//             QNThreadMutex -    When a shared object will be accessed
//                                by multiple threads within the same
//                                partition.
//             QNPartitionMutex - When a shared object, like a global variable,
//                                only needs to be protected by access across
//                                partitions. This mutex is smart in that a
//                                shared-memory parallel run will be safe,
//                                while an MPI simulation will be efficent
//                                (becuase the lock isn't needed and has no
//                                cost)
//
//             To use:
//             Step 1) Define one mutex to protect your data as part of your
//                     initialization (e.g. call new QNThreadMutex () to create
//                     a new mutex OR define an instance of QNThreadMutex next
//                     to your global variable).
//             Step 2) You lock the mutex in your critical section by
//                     declaring a local lock variable, passing in your
//                     mutex as an argument of initialization (e.g.
//                     call QNThreadLock (&myMutex)).
//
//              EXAMPLE:
//              void
//              addWorkToQueue (EXTERNAL_Interface * interface,
//                              Command command) {
//                  MyIfaceData * data = (MyIfaceData *) interface->data;
//                    ...
//                    // error checking or prepartion
//                    ...
//                    {
//                      // Start of the critical section
//                      // The creation of this variable aquires the lock.
//                      QNThreadLock myLock (data->workQueueMutex);
//                      ...
//                      // add command to queue
//                      ...
//                  } // End of the critical section
//                    // myLock leaves scope. Its destructor auto-releases the
//                    // mutex for you.
//
// **/


#ifdef _WIN32
// Internal DOC:
//      On Windows we have a choice of using CreateMutex or
//      CriticalRegions. The CriticalRegion is simpler, and meets all
//      of our needs plus it provides a "spinCount-before-waiting" feature.
#define OS_Mutex CRITICAL_SECTION
#define OS_LockMutex(p) EnterCriticalSection (p)
#define OS_UnlockMutex(p) LeaveCriticalSection (p)
#else
#define OS_Mutex pthread_mutex_t
#define OS_LockMutex(p) pthread_mutex_lock (p)
#define OS_UnlockMutex(p) pthread_mutex_unlock (p)
#endif

class QNThreadMutex {
public:
    QNThreadMutex ()
    {
#ifdef _WIN32
        // Default the spincount to about 16000. NYI, examine greedy-cpu config
        // InitializeCriticalSection(&m_osMutex);
        if (InitializeCriticalSectionAndSpinCount (&m_osMutex, 16000) == 0) {
            ERROR_ReportError("Error: Windows failed to create mutex handle");
        }
#else
        if (pthread_mutex_init(&m_osMutex, NULL) != 0) {
            ERROR_ReportError("Error: pthread failed to create mutex");
        }
#endif
    }

    void lock () { OS_LockMutex (&m_osMutex); }

    void unlock () { OS_UnlockMutex (&m_osMutex); }

private:
    // Not allowed to copy or assign to mutex objects.
    QNThreadMutex (const QNThreadMutex &);
    QNThreadMutex & operator= (const QNThreadMutex &);

    OS_Mutex            m_osMutex;
};


// Internal DOC:
// Construction (initialization) is aquistion of resource.
// This is a good idea because:
//  1) Early returns, breaks, etc. will all release the lock before returning.
//  2) Reduced burden on user of class - they don't have to remember the unlock
//     function name and they don't have to remember to call it.
//  3) Exceptions will also release the lock because dtor's still fire.
//  4) Fewer lines of code. Also becomes very hard to write bad/complex code
//     that would return a locked resource that would have to be manually
//     released.
// (for more, see Meyers: Effective C++ items 13,14)
//
// Please note, you should ensure that your code does not recursively lock.
// Currently this class does not guard against recursive locking - the
// behavior will be just whatever the OS mutex provides. On most linux,
// this means a recursive lock call _will_ deadlock you.
class QNThreadLock {
public:

    // /**
    // FUNCTION   :: QNThreadLock::QNThreadLock
    // PURPOSE    :: This constructor is used to begin a critical
    //               region.
    // PARAMETERS ::
    // + mutex : QNThreadMutex : Pointer to the Thread mutex to lock for this
    //                           critical section. Please note, that your code
    //                           should be careful to avoid recursively locking
    //                           the mutex (i.e. don't lock the same mutex
    //                           twice)
    // RETURN     :: None :
    // **/
    explicit QNThreadLock (QNThreadMutex * mutex)
        : m_mutex(mutex) { m_mutex->lock (); }
    ~QNThreadLock () { m_mutex->unlock (); }
private:
    QNThreadMutex *    m_mutex;

    // Not allowed to copy or assign to lock objects.
    QNThreadLock (const QNThreadLock &);
    QNThreadLock & operator= (const QNThreadLock &);

    // No default ctor, must provide mutex
    QNThreadLock ();
};





class QNPartitionMutex {
public:
    QNPartitionMutex ()
    {
#ifndef USE_MPI // When mpi the mutex is a noop
#ifdef _WIN32
        // Default the spincount to about 16000. NYI, examine greedy-cpu config
        if (InitializeCriticalSectionAndSpinCount (&m_osMutex, 16000) == 0) {
            ERROR_ReportError("Error: Windows failed to create mutex handle");
        }
#else
        if (pthread_mutex_init(&m_osMutex, NULL) != 0) {
            ERROR_ReportError("Error: pthread failed to create mutex");
        }
#endif //_WIN32
#endif // USE_MPI
    }

#ifndef USE_MPI // When mpi the mutex is a noop
    void lock () { OS_LockMutex (&m_osMutex); }
    void unlock () { OS_UnlockMutex (&m_osMutex); }
#else
    void lock () { }
    void unlock () { }
#endif // USE_MPI

private:
    // Not allowed to copy or assign to mutex objects.
    QNPartitionMutex (const QNPartitionMutex &);
    QNPartitionMutex & operator= (const QNPartitionMutex &);

#ifndef USE_MPI // When mpi the mutex is a noop
    OS_Mutex            m_osMutex;
#endif // USE_MPI
};


class QNPartitionLock {
public:
    // /**
    // FUNCTION   :: QNPartitionLock::QNPartitionLock
    // PURPOSE    :: This constructor is used to begin a critical
    //               region.
    // PARAMETERS ::
    // + mutex : QNPartitionMutex : Pointer to the Partition mutex to lock
    //                              for this critical section. Please note,
    //                              that your code should be careful to avoid
    //                              recursively locking the mutex (i.e. don't
    //                              lock the same mutex twice)
    // RETURN     :: None :
    // **/
#ifndef USE_MPI // When mpi the mutex is a noop
    explicit QNPartitionLock (QNPartitionMutex * mutex)
        : m_mutex(mutex) { m_mutex->lock (); }
    ~QNPartitionLock () { m_mutex->unlock (); }
#else
    explicit QNPartitionLock (QNPartitionMutex * mutex) { }
    ~QNPartitionLock () { }
#endif // USE_MPI

private:
#ifndef USE_MPI // When mpi the mutex is a noop
    QNPartitionMutex *    m_mutex;
#endif // USE_MPI

    // Not allowed to copy or assign to lock objects.
    QNPartitionLock (const QNPartitionLock &);
    QNPartitionLock & operator= (const QNPartitionLock &);

    // No default ctor, client must provide a mutex
    QNPartitionLock ();
};

#endif /* __QUALNET_MUTEX_H__ */
