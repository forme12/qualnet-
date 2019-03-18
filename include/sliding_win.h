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

#ifndef _SLIDING_WIN_H
#define _SLIDING_WIN_H

#include "clock.h"

// /**
// PACKAGE     :: SLIDING-WINDOW
// DESCRIPTION :: This file describes data structures and functions to
//                implement a sliding window.
// **/

// /**
// STRUCT       :: MsTmWin
// DESCRIPTION  :: sliding time window averager structure
// **/
typedef struct {
        double*     pSlot;      // time slots for data measurement
        int         nSlot;      // # of slots
        clocktype   sSize;      // size for each slot
        clocktype   tmBase;     // base time
        clocktype   tmStart;    // time of the first data
        double      total;      // totally cumulative data amount
        double      weight;     // weight factor between -1 and 1
} MsTmWin;

// /**
// API       :: MsTmWinInit
// PURPOSE   :: initialize time sliding window with the given parameters
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + sSize   : clocktype : sliding window slot size
// + nSlot   : int       : sliding window number of slots
// + weight  : double    : weight for average computation
// + theTime : clocktype : the current time
// RETURN    :: void :
// **/
void MsTmWinInit(MsTmWin* pWin, clocktype sSize, int nSlot, double weight,
                 clocktype theTime);

// API       :: MsTmWinClear
// PURPOSE   :: clears time sliding window
// PARAMETERS ::
// + pWin   : MsTmWin*  : pointer to the time sliding window
// RETURN    :: void :
// **/
void MsTmWinClear(MsTmWin* pWin);

// /**
// API       :: MsTmWinInit
// PURPOSE   :: resets time sliding window with the given parameters
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + sSize   : clocktype : sliding window slot size
// + nSlot   : int       : sliding window number of slots
// + weight  : double    : weight for average computation
// + theTime : clocktype : the current time
// RETURN    :: void :
// **/
void MsTmWinReset(MsTmWin* pWin, clocktype sSize, int nSlot, double weight,
                  clocktype theTime);

// /**
// API       :: MsTmWinNewData
// PURPOSE   :: updates time sliding window with the given new data
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + data    : double    : new data
// + theTime : clocktype : the current time
// RETURN    :: void :
// **/
void MsTmWinNewData(MsTmWin* pWin, double data, clocktype theTime);

// /**
// API       :: MsTmWinWinSize
// PURPOSE   :: returns the window size
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + theTime : clocktype : the current time
// RETURN    :: clocktype : the window size based on the current time
// **/
clocktype MsTmWinWinSize(MsTmWin* pWin, clocktype theTime);

// /**
// API       :: MsTmWinSum
// PURPOSE   :: computes the data sum of the window
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + theTime : clocktype : the current time
// RETURN    :: double : the data sum of the window
// **/
double MsTmWinSum(MsTmWin* pWin, clocktype theTime);

// /**
// API       :: MsTmWinAvg
// PURPOSE   :: computes the data average of the window
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + theTime : clocktype : the current time
// RETURN    :: double : the data average of the window
// **/
double MsTmWinAvg(MsTmWin* pWin, clocktype theTime);

// /**
// API       :: MsTmWinTotalSum
// PURPOSE   :: computes the total data sum
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + theTime : clocktype : the current time
// RETURN    :: double : the total data sum
// **/
double MsTmWinTotalSum(MsTmWin* pWin, clocktype theTime);

// /**
// API       :: MsTmWinTotalAvg
// PURPOSE   :: computes the total data average
// PARAMETERS ::
// + pWin    : MsTmWin*  : pointer to the time sliding window
// + theTime : clocktype : the current time
// RETURN    :: double : the total data average
// **/
double MsTmWinTotalAvg(MsTmWin* pWin, clocktype theTime);

#endif  // _SLIDING_WIN_H

