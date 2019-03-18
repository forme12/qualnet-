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

/*
 * PURPOSE: System Queue/Scheduler Structures, Function Pointers
 */

#ifndef SCHEDULER_TYPES_H
#define SCHEDULER_TYPES_H

//#include "calendar.h"
#include "splaytree.h"

//-------- Function Pointer definitions for individual queues ---------

// /**
// API       :: SchedulerQInsertMessageFunction
// PURPOSE   :: Insert a message into the node's message queue
// PARAMETERS ::
// + node        : Node*       : Pointer to the node to insert into
// + msg         : Message*    : Pointer to the message to insert
// + time        : clocktype   : time to delay
// RETURN    :: void :
// **/
typedef void (*SchedulerQInsertMessageFunction) (Node *node, Message *msg, clocktype delay);


// /**
// API       :: SchedulerQExtractFirstMessageFunction
// PURPOSE   :: Remove the first message from the node's message queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
// + node           : Node*     : Pointer to the node
//                            to be extracted
// RETURN    ::  Message** : First message from queue
// **/
typedef Message* (*SchedulerQExtractFirstMessageFunction) (PartitionData *partitionData, Node *node);


// /**
// API       :: SchedulerQPeekFirstMessageFunction
// PURPOSE   :: Peek at the first message from the node's message queue
//              NOT REMOVED FROM QUEUE
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to be extracted
// RETURN    :: Message*    : Pointer to message
// **/
typedef Message* (*SchedulerQPeekFirstMessageFunction) (Node *node);


// /**
// API       :: SchedulerQDeleteMessageFunction
// PURPOSE   :: Delete a message from the nodes's message queue
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to delete message from
// RETURN    :: void :
// **/
typedef void (*SchedulerQDeleteMessageFunction) (Node *node, Message *msg);


// /**
// API       :: SchedulerQInsertNodeFunction
// PURPOSE   :: Insert a node into the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition
//                                       to be inserted
// + node           : Node *           : Pointer to the node
// RETURN    :: void :
// **/
typedef void (*SchedulerQInsertNodeFunction) (PartitionData *partitionData, Node *node);


// /**
// API       :: SchedulerQPeekNextNodeFunction
// PURPOSE   :: Peek at next node to process in the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: Node*   :next node to process
// **/
typedef Node* (*SchedulerQPeekNextNodeFunction) (PartitionData *partitionData);


// /**
// API       :: SchedulerQDeleteNodeFunction
// PURPOSE   :: Delete a node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// + node           : Node*     : Pointer to the node
// RETURN    :: void :
// **/
typedef void (*SchedulerQDeleteNodeFunction)  (PartitionData *partitionData, Node *node);

// /**
// API       :: SchedulerQCurrentNodeFunction
// PURPOSE   :: Current node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: void :
// **/
typedef Node* (*SchedulerQCurrentNodeFunction) (const PartitionData *partitionData);

// /**
// API       :: SchedulerQHasNodesFunction
// PURPOSE   :: Check for nodes in the scheduler's node queue.
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
//
// RETURN    :: BOOL    :scheduler has nodes in it's queue
// **/
typedef BOOL (*SchedulerQHasNodesFunction) (const PartitionData *partitionData);

// /**
// API       :: SchedulerQNextEventFunction
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*     : Pointer to the node
// RETURN    :: Message*   : next event
// **/
typedef Message* (*SchedulerQNextEventFunction) (PartitionData *partitionData, Node* node);

// /**
// API       :: SchedulerQNextEventTimeFunction
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: clocktype   : time of next event
// **/
typedef clocktype (*SchedulerQNextEventTimeFunction) (const PartitionData *partitionData);

// /**
// API       :: SchedulerQInitalizeFunction
// PURPOSE   :: Initalize Scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
typedef void (*SchedulerQInitalizeFunction)(PartitionData *partitionData);

// /**
// API       :: SchedulerQFinalizeFunction
// PURPOSE   :: Finalize Scheduler Queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
typedef void (*SchedulerQFinalizeFunction) (PartitionData *partitionData);

//-------------------------------------------------------------------

// -------------------- Queue Types available ---------------------
typedef enum {
    SPLAYTREE_QUEUE,
    // CALENDAR_QUEUE,     // NO LONGER USED
    LADDER_QUEUE,
    STDLIB_HEAP,
    CALENDAR_QUEUE2
} SchedulerQueueType;

class CalendarQ;

//------------ Declaration of Scheduler Queue Structure -------------
typedef struct scheduler_queue_str {

    SchedulerQueueType          schedQueueType;
    SplayTree                   splayTree;
    CalendarQ *                 calendarQ;

    BOOL                        isCollectingStats;

    // Standard Function Pointers for queue calls
    SchedulerQInsertMessageFunction         InsertMessage;
    SchedulerQExtractFirstMessageFunction   ExtractFirstMessage;
    SchedulerQPeekFirstMessageFunction      PeekFirstMessage;
    SchedulerQDeleteMessageFunction         DeleteMessage;
    SchedulerQInsertNodeFunction            InsertNode;
    SchedulerQPeekNextNodeFunction          PeekNextNode;
    SchedulerQDeleteNodeFunction            DeleteNode;
    SchedulerQCurrentNodeFunction           CurrentNode;
    SchedulerQHasNodesFunction              HasNodes;
    SchedulerQNextEventFunction             NextEvent;
    SchedulerQNextEventTimeFunction         NextEventTime;
    SchedulerQInitalizeFunction             Initalize;
    SchedulerQFinalizeFunction              Finalize;

} SchedulerInfo;

//--------------------------------------------------------------------

#endif /* SCHEDULER_TYPES_H */
