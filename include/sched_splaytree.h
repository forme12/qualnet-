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
 * PURPOSE: System Queue/Scheduler Structures, Function Pointers, Util Functions
 */

#ifndef SCHED_SPLAYTREE_H
#define SCHED_SPLAYTREE_H

#include "scheduler_types.h"
#include "scheduler.h"
#include "splaytree.h"


//------------ Declaration of Scheduler Queue Structure -------------
typedef struct scheduler_splaytree_str {

    SchedulerQueueType              schedQueueType;
    CalendarQInfo                   *schedQueue;

} SchedulerSplaytree;


//------------------------------------------------------------------

// /**
// API       :: SCHED_SPLAYTREE_InsertMessage
// PURPOSE   :: Insert a message into the node's message queue
// PARAMETERS ::
// + node        : Node*       : Pointer to the node to insert into
// + msg         : Message*    : Pointer to the message to insert
// + time        : clocktype   : time to delay
// RETURN    :: void :
// **/
void SCHED_SPLAYTREE_InsertMessage(
    Node *node,
    Message *msg,
    clocktype delay);


// /**
// API       :: SCHED_SPLAYTREE_ExtractFirstMessage
// PURPOSE   :: Remove the first message from the node's message queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*     : Pointer to the node
//                            to be extracted
// RETURN    ::  Message** : First message from queue
// **/
Message* SCHED_SPLAYTREE_ExtractFirstMessage(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_SPLAYTREE_PeekFirstMessage
// PURPOSE   :: Peek at the first message from the node's message queue
//              NOT REMOVED FROM QUEUE
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to be extracted
// RETURN    :: Message*    : Pointer to message
// **/
Message* SCHED_SPLAYTREE_PeekFirstMessage(
    Node *node);


// /**
// API       :: SCHED_SPLAYTREE_DeleteMessage
// PURPOSE   :: Delete a message from the nodes's message queue
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to delete message from
// RETURN    :: void :
// **/
void SCHED_SPLAYTREE_DeleteMessage(
    Node *node,
    Message *msg);


// /**
// API       :: SCHED_SPLAYTREE_InsertNode
// PURPOSE   :: Insert a node into the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition
//                                       to be inserted
// + node           : Node *           : Pointer to the node
// RETURN    :: void :
// **/
void SCHED_SPLAYTREE_InsertNode(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_SPLAYTREE_PeekNextNode
// PURPOSE   :: Peek at next node to process in the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: Node*   : next node to process
// **/
Node* SCHED_SPLAYTREE_PeekNextNode(
    PartitionData *partitionData);


// /**
// API       :: SCHED_SPLAYTREE_DeleteNode
// PURPOSE   :: Delete a node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// + node           : Node*     : Pointer to the node
// RETURN    :: void
// **/
void SCHED_SPLAYTREE_DeleteNode(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_SPLAYTREE_CurrentNode
// PURPOSE   :: Current node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: void :
// **/
Node* SCHED_SPLAYTREE_CurrentNode(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_SPLAYTREE_HasNodes
// PURPOSE   :: Check for nodes in the scheduler's node queue.
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
//
// RETURN    :: BOOL    :scheduler has nodes in it's queue
// **/
BOOL SCHED_SPLAYTREE_HasNodes(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_SPLAYTREE_NextEvent
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*     : Pointer to the node
// RETURN    :: Message*   : next event
// **/
Message* SCHED_SPLAYTREE_NextEvent(
    PartitionData *partitionData,
    Node* node);


// /**
// API       :: SCHED_NextEventTime
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: clocktype   : time of next event
// **/
clocktype SCHED_SPLAYTREE_NextEventTime(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_Initialize
// PURPOSE   :: Initalize Sceduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void SCHED_SPLAYTREE_Initialize(
    PartitionData *partitionData);


// /**
// API       :: SCHED_Finalize
// PURPOSE   :: Finalize Scheduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void SCHED_SPLAYTREE_Finalize(
    PartitionData *partitionData);


//----------------------------------------------------------------------

#endif /* SCHED_SPLAYTREE_H */
