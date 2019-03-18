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


#ifndef WALLCLOCK_H
#define WALLCLOCK_H


#include <time.h>
#include <clock.h>


// /**
// PACKAGE     :: WALLCLOCK
// DESCRIPTION :: This file describes methods of the WallClock class whose
//                primary use is to keep track of the amount of real time
//                that has passed during the simulation.
// **/



// WallClock:
// A virtualized wall clock. Class accomidates pausing and resumption
// of the advance of a real time wall lock.
//
// The wall clock's value, the value of getRealTIme (), can be obtained
// as a double value in seconds, or as a clocktype.
// The wall clock's true, unadjusted time can be obtained by calling
// getTrueRealTime () and getTrueRealTimeAsDouble ().
//
// Used By: main/gui, main/external, partition, hla
//
// This class hides the platform specifics for obtaining an accurate
// wall clock time. On windows the HighPerformance timer registers are
// used, on unix the gettimeofday function is used.
//
class WallClock
{
public:
    WallClock (void * partitionData);

    void SetRealTime(clocktype timeDiff);
    static double getTrueRealTimeAsDouble();
    static clocktype getTrueRealTime();
    double getRealTimeAsDouble(); 
    clocktype getRealTime();
    void pause();
    void resume();
    void setRealTimeMultiple (double multiple);
    // /**
    // FUNCTION   :: WallClock::isPaused
    // PURPOSE    :: This method returns true if the WallClock is currently
    //               in the paused state.
    // PARAMETERS :: 
    // + void : : None
    // RETURN     :: BOOL : TRUE or FALSE
    // **/
    bool                isPaused () { return m_numPauses > 0; }

    // /**
    // FUNCTION   :: WallClock::getRealTimeMultiple
    // PURPOSE    :: Return the real time multiple
    // PARAMETERS :: None
    // RETURN     :: double
    // **/
    double              getRealTimeMultiple () { return m_realTimeMultiple; }

    // /**
    // FUNCTION   :: WallClock::disallowPause
    // PURPOSE    :: Pausing of the WallClock can be disabled by any
    //               external interface ambassador. Permission to pause is
    //               all or nothing, so if any external interface disables
    //               pause, no pausing is allowed. As an example, a simulation
    //               using IPNE and HLA is run. If the IPNE code disables
    //               pausing, then HLA won't be able to pause the WallClock
    //               or in other words the wall clock's value for time just
    //               keeps running.
    // PARAMETERS :: 
    // + void : : None
    // RETURN     :: void :
    // **/
    void                disallowPause () { m_allowPause = false; }

    // /**
    // FUNCTION   :: WallClock::disallowPause
    // PURPOSE    :: Allows pausing of the WallClock
    // PARAMETERS :: 
    // + void : : None
    // RETURN     :: void :
    // **/
    bool                allowsPause () { return m_allowPause; }

    // /**
    // FUNCTION   :: WallClock::getDurationPaused
    // PURPOSE    :: Get the amount of time, in seconds, spent paused.
    // PARAMETERS :: 
    // + void : : None
    // RETURN     :: double : The amount of time paused, in seconds.
    // **/
    double              getDurationPaused () { return m_durationPaused; }

protected:
    clocktype           convertToClocktype (double timeInSeconds)
    {
        return ((clocktype)(timeInSeconds * SECOND));
    }

    double              convertToDouble (clocktype timeInNS)
    {
        return (double) timeInNS / SECOND;
    }

private:
    // how many interfaces are currently pausing real-time
    int                 m_numPauses;
    double              m_durationPaused;       // in seconds
    bool                m_allowPause;
    void * m_partitionData;
    clocktype           m_initializedTime;

    // Multiple of real-time to run in
    double              m_realTimeMultiple;

    double              m_pauseStartTime;

    // NYI
    double              getNowHighPrecision ();
    double              getClockStartTime ();

    // Not allowed
    WallClock ();
    WallClock (const WallClock &);
    WallClock & operator= (const WallClock &);
};

#endif
