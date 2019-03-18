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

#ifndef SCHEDULER_H
#define SCHEDULER_H

//------------------------------------------------------------------
//-- New global system scheduler calls (replaces existing SCHED_) --
//------------------------------------------------------------------

// -- inlines --

// /**
// API       :: SCHED_InsertMessage
// PURPOSE   :: Insert a message into the node's message queue
// PARAMETERS ::
// + node        : Node*       : Pointer to the node to insert into
// + msg         : Message*    : Pointer to the message to insert
// + time        : clocktype   : time to delay
// RETURN    :: void :
// **/
static
void SCHED_InsertMessage(
    Node *node,
    Message *msg,
    clocktype delay)
{
    node->partitionData->schedulerInfo->InsertMessage(node, msg, delay);
}


// /**
// API       :: SCHED_ExtractFirstMessage
// PURPOSE   :: Remove the first message from the node's message queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*            : Pointer to the node
//                                       to be extracted
// RETURN    ::  Message* : First message from queue
// **/
static
Message* SCHED_ExtractFirstMessage(
    PartitionData *partitionData,
    Node *node)
{
    return node->partitionData->schedulerInfo->ExtractFirstMessage(partitionData, node);
}


// /**
// API       :: SCHED_PeekFirstMessage
// PURPOSE   :: Peek at the first message from the node's message queue
//              NOT REMOVED FROM QUEUE
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to be extracted
// RETURN    :: Message*    : Pointer to message
// **/
static
Message* SCHED_PeekFirstMessage(
    Node *node)
{
    return node->partitionData->schedulerInfo->PeekFirstMessage(node);
}


// /**
// API       :: SCHED_DeleteMessage
// PURPOSE   :: Delete a message from the nodes's message queue
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to delete message from
// RETURN    :: void :
// **/
static
void SCHED_DeleteMessage(
    Node *node,
    Message *msg)
{
    node->partitionData->schedulerInfo->DeleteMessage(node, msg);
}


// /**
// API       :: SCHED_InsertNode
// PURPOSE   :: Insert a node into the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition
//                                       to be inserted
// + node           : Node *           : Pointer to the node
// RETURN    :: void :
// **/
static
void SCHED_InsertNode(
    PartitionData *partitionData,
    Node* node)
{
    partitionData->schedulerInfo->InsertNode(partitionData, node);
}


// /**
// API       :: SCHED_PeekNextNode
// PURPOSE   :: Peek at next node to process in the partition's scheduler queue
//              NOT REMOVED FROM QUEUE
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: Node*   :next node to process
// **/
static
Node* SCHED_PeekNextNode(
    PartitionData *partitionData)
{
    return partitionData->schedulerInfo->PeekNextNode(partitionData);
}


// /**
// API       :: SCHED_DeleteNode
// PURPOSE   :: Delete a node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*    : Pointer to the node
//                                        to be inserted
// + node           : Node*             : Pointer to the node
// RETURN    :: void :
// **/
static
void SCHED_DeleteNode(
    PartitionData *partitionData,
    Node *node)
{
    partitionData->schedulerInfo->DeleteNode(partitionData, node);
}


// /**
// API       :: SCHED_CurrentNode
// PURPOSE   :: Current node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: void :
// **/
static
Node* SCHED_CurrentNode(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->CurrentNode(partitionData);
}


// /**
// API       :: SCHED_HasNodes
// PURPOSE   :: Check for nodes in the scheduler's node queue.
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
//
// RETURN    :: BOOL    :scheduler has nodes in it's queue
// **/
static
BOOL SCHED_HasNodes(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->HasNodes(partitionData);
}


// /**
// API       :: SCHED_NextEvent
// PURPOSE   :: Get the next event for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*    : Pointer to the partition data
// + node           : Node*             : Pointer to the node
// RETURN    :: Message*   : next event
// **/
static
Message* SCHED_NextEvent(
    PartitionData *partitionData,
    Node *node)
{
    return partitionData->schedulerInfo->NextEvent(partitionData, node);
}

// /**
// API       :: SCHED_NextEventTime
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: clocktype   : time of next event
// **/
static
clocktype SCHED_NextEventTime(
    const PartitionData *partitionData)
{
    return partitionData->schedulerInfo->NextEventTime(partitionData);
}


// /**
// API       :: SCHED_Initalize
// PURPOSE   :: Initalize Scheduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + NodeInput      : nodeInput*       : Pointer to the node input data
// RETURN    :: void :
// **/
void SCHED_Initalize(
    PartitionData *partitionData,
    const NodeInput *nodeInput);


// /**
// API       :: SCHED_Finalize
// PURPOSE   :: Finalize Scheduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void SCHED_Finalize(
    PartitionData *partitionData);

//---------------------------------------------------------------------

#endif /* SCHEDULER_H */
