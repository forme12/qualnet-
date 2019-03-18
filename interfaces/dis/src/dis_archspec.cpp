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

#include <cmath>
#include <ctime>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <sys/types.h>
#include <sys/timeb.h>
#else /* _WIN32 */
#include <unistd.h>
#include <sys/time.h>
#endif /* _WIN32 */

#include <cassert>

#include "WallClock.h"
#include "dis_archspec.h"

void
DisSleep(unsigned seconds)
{
#ifdef _WIN32
    // Win32 Sleep() sleeps in milliseconds.

    Sleep((DWORD) seconds * 1000);
#else /* _WIN32 */
    sleep(seconds);
#endif /* _WIN32 */
}

void
DisUsleep(unsigned microseconds)
{
#ifdef _WIN32
    // Win32 Sleep() sleeps in milliseconds.

    Sleep((DWORD) (microseconds / 1000));
#else /* _WIN32 */
    usleep(microseconds);
#endif /* _WIN32 */
}

double
DisFtime()
{
    return WallClock::getTrueRealTimeAsDouble ();
}

double
DisGetNumSecondsPastHour()
{
    time_t timeValue;
    unsigned numMicroseconds;

#ifdef _WIN32
    _timeb ftimeValue;
    _ftime(&ftimeValue);
    timeValue = ftimeValue.time;
    numMicroseconds = ftimeValue.millitm * 1000;
#else /* _WIN32 */
    timeval gettimeofdayValue;
    gettimeofday(&gettimeofdayValue, NULL);
    timeValue = (time_t) gettimeofdayValue.tv_sec;
    numMicroseconds = gettimeofdayValue.tv_usec;
#endif /* _WIN32 */

    tm* localtimeValue = localtime(&timeValue);

    double numSecondsPastHour
        = ((double) ((localtimeValue->tm_min * 60) + localtimeValue->tm_sec))
          + ((double) numMicroseconds) / 1e6;

    return numSecondsPastHour;
}

double
DisRint(double a)
{
#ifdef _WIN32
    // Always round up when the fractional value is exactly 0.5.
    // (Usually such functions round up only half the time.)

    return floor(a + 0.5);
#else /* _WIN32 */
    return rint(a);
#endif /* _WIN32 */
}
