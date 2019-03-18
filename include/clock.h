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
// PACKAGE     :: CLOCK
// DESCRIPTION :: This file describes data structures and functions used for time-related operations.
// **/

#ifndef QUALNET_CLOCK_H
#define QUALNET_CLOCK_H

#include "types.h"
#include "main.h"


typedef Int64 clocktype;

// /**
// CONSTANT    :: CLOCKTYPE_MAX : Platform dependent
// DESCRIPTION :: CLOCKTYPE_MAX is the maximum value of clocktype.
// This value can be anything as long as it is less than or equal to the
// maximum value of the type which is typedefed to clocktype.
// Users can simulate the model up to CLOCKTYPE_MAX - 1.
// **/

#define CLOCKTYPE_MAX TYPES_ToInt64(0x7fffffffffffffff)

// /**
// MACRO       :: ctoa
// DESCRIPTION :: like sprintf, prints a clocktype to a string
// **/
#define ctoa(clock, string) CHARTYPE_sprintf( \
                        string, \
                        CHARTYPE_Cast("%" TYPES_64BITFMT "d"), \
                        clock)

// /**
// MACRO       :: atoc
// DESCRIPTION :: like atoi or atof, converts a string to a clocktype
// **/
#define atoc(string, clock) CHARTYPE_sscanf( \
                          string,\
                          CHARTYPE_Cast("%" TYPES_64BITFMT "d"), \
                          clock)

// Units of time defined as units of clocktype.
//   1 ns = 1 unit of clocktype.


// /**
// CONSTANT    :: NANO_SECOND : ((clocktype) 1)
// DESCRIPTION :: Defined as basic unit of clocktype
// **/
#define NANO_SECOND              ((clocktype) 1)

// /**
// CONSTANT    :: MICRO_SECOND : (1000 * NANO_SECOND)
// DESCRIPTION :: Defined as 1000 times the basic unit of clocktype
// **/
#define MICRO_SECOND             (1000 * NANO_SECOND)

// /**
// CONSTANT    :: MILLI_SECOND  : (1000 * MICRO_SECOND)
// DESCRIPTION :: unit of time equal to 1000 times MICRO_SECOND
// **/
#define MILLI_SECOND             (1000 * MICRO_SECOND)

// /**
// CONSTANT    :: SECOND : (1000 * MILLI_SECOND)
// DESCRIPTION :: simulation unit of time =1000 times MILLI_SECOND
// **/
#define SECOND                   (1000 * MILLI_SECOND)

// /**
// CONSTANT    :: MINUTE : (60 * SECOND)
// DESCRIPTION :: unit of simulation time = 60 times SECOND
// **/
#define MINUTE                   (60 * SECOND)

// /**
// CONSTANT    :: HOUR  :    (60 * MINUTE)
// DESCRIPTION :: unit of simulation time = 60 times MINUTE
// **/
#define HOUR                     (60 * MINUTE)

// /**
// CONSTANT    :: DAY  :    (24 * HOUR)
// DESCRIPTION :: unit of simulation time = 24 times HOUR
// **/
#define DAY                      (24 * HOUR)

// /**
// CONSTANT    :: PROCESS_IMMEDIATELY  : 0
// DESCRIPTION :: Used to prioratize a process
// **/
#define PROCESS_IMMEDIATELY 0

// /**
// MACRO       :: getSimTime(node) : (*(node->currentTime))
// DESCRIPTION :: To get the simulation time of a node
// **/
#define getSimTime(node) (*(node->currentTime))

// /**
// MACRO       :: getSimStartTime(node) : (*(node->startTime))
// DESCRIPTION :: To get the simulation start time of a node 获取一个节点的仿真时间
// **/
#define getSimStartTime(node) (*(node->startTime))

// /**
// MACRO       :: TIME_getSimTime(node) : (*(node->currentTime))
// DESCRIPTION :: Gets current simulation time of a node
// **/
#define TIME_getSimTime(node) (*(node->currentTime))

// /**
// API         :: TIME_ConvertToClock
// PURPOSE     :: Read the string in "buf" and provide the corresponding
// clocktype value for the string using the following conversions:
//               NS - nano-seconds
//               MS - milli-seconds
//               S  - seconds (default if no specification)
//               H  - hours
//               D  - days
// PARAMETERS  ::
// + buf        : char* : The time string
// RETURN      :: clocktype : Time in clocktype
// **/
clocktype
TIME_ConvertToClock(const char *buf);

// /**
// API         :: TIME_PrintClockInSecond
// PURPOSE     :: Print a clocktype value in second.The result is copied
//                to string in Seconds
// PARAMETERS  ::
// + clock          : clocktype : Time in clocktype
// + stringInSecond : char * : string containing time in seconds
// RETURN      :: void :
// **/
void
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[]);

// /**
// API         :: TIME_PrintClockInSecond
// PURPOSE     :: Print a clocktype value in second.The result is copied
//                to string in Seconds
// PARAMETERS  ::
// + clock          : clocktype : Time in clocktype
// + stringInSecond : char * : string containing time in seconds
// + node           : Node * : Input node
// RETURN      :: void :
// **/
void 
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[], Node* node);

// /**
// API         :: TIME_PrintClockInSecond
// PURPOSE     :: Print a clocktype value in second.The result is copied
//                to string in Seconds
// PARAMETERS  ::
// + clock          : clocktype : Time in clocktype
// + stringInSecond : char * : string containing time in seconds
// + partition      : PartitionData * : Input partition
// RETURN      :: void :
// **/
void 
TIME_PrintClockInSecond(clocktype clock, char stringInSecond[], PartitionData* partition);


// /**
// API         :: TIME_ReturnMaxSimClock
// PURPOSE     :: Return the maximum simulation clock
// PARAMETERS  ::
// + node : Node* : Input node
// RETURN      :: clocktype : Returns maximum simulation time
// **/
clocktype 
TIME_ReturnMaxSimClock(Node *node);

// /**
// API         :: TIME_ReturnStartSimClock
// PURPOSE     :: Return the simulation start clock
// PARAMETERS  ::
// + node : Node* : Input node
// RETURN      :: clocktype : Returns simulation start time
// **/
clocktype 
TIME_ReturnStartSimClock(Node *node);

#endif
