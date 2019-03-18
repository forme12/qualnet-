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
 * PURPOSE: Event (message) queue using standard template library heap
 */

#ifndef SCHED_STD_LIBRARY_H
#define SCHED_STD_LIBRARY_H

#include "scheduler_types.h"
//#include "scheduler.h"
//#include <vector>
#include <queue>


//------------ Declaration of Scheduler Queue Structure -------------

struct SchedulerStlEvent {
    SchedulerStlEvent (Message * msg, Node * node, clocktype timeValue) :
        m_timeValue (timeValue), m_msg (msg), m_node (node)
        {}

    clocktype   m_timeValue;
    Message *   m_msg;
    Node *      m_node;
    bool operator<  (const SchedulerStlEvent & rhs) const {
        // EVENT ORDERING:
        //  1) time
        //  2) <implicitly> nodeIndex
        //  3) eventType
        //  4) interface number (instanceId)
        //  5) originatingNodeId
        //  6) naturalOrder -- fifo added events
        // Tie-breaking is difficult. Imagine a MIMO radio
        // so that 2 messages are scheduled for the exact
        // same time. Which event should execute first?
        // Using natural ordering in this case works, but in
        // general with parallel the naturalOrder can vary between runs.
        // if mulitple nodes are involved.
        if (m_timeValue > rhs.m_timeValue) {
            return true;
        } else if (m_timeValue < rhs.m_timeValue) {
            return false;
        // lower node indexes first
        } else if (m_node->nodeIndex > rhs.m_node->nodeIndex) {
            return true;
        } else if (m_node->nodeIndex < rhs.m_node->nodeIndex) {
            return false;
        // lower eventTypes first
        } else if (m_msg->eventType > rhs.m_msg->eventType) {
            return true;
        } else if (m_msg->eventType < rhs.m_msg->eventType) {
            return false;
        // smaller interface indexes go first
        } else if (m_msg->instanceId > rhs.m_msg->instanceId) {
            return true;
        } else if (m_msg->instanceId < rhs.m_msg->instanceId) {
            return false;
        // smaller originating nodes go first
        } else if (m_msg->originatingNodeId > rhs.m_msg->originatingNodeId) {
            return true;
        } else if (m_msg->originatingNodeId < rhs.m_msg->originatingNodeId) {
            return false;
        // lower sequence ordering first
        } else if (m_msg->naturalOrder > rhs.m_msg->naturalOrder) {
            return true;
        } else if (m_msg->naturalOrder < rhs.m_msg->naturalOrder) {
            return false;
        }
        return false;
    }
};

// #define STDLIB_HEAP_INITIAL_CAPACITY    16384
// typedef std::priorty_queue <standardNode, std::vector <standardNode>

// defaults to using a vector for storage and ordering via comparison object
// of less <standardNode>
typedef std::priority_queue <SchedulerStlEvent>       StlHeap;


//------------------------------------------------------------------

// /**
// API       :: SCHED_STDLIB_InsertMessage
// PURPOSE   :: Insert a message into the node's message queue
// PARAMETERS ::
// + node        : Node*       : Pointer to the node to insert into
// + msg         : Message*    : Pointer to the message to insert
// + time        : clocktype   : time to delay
// RETURN    :: void :
// **/
void SCHED_STDLIB_InsertMessage(
    Node *node,
    Message *msg,
    clocktype delay);


// /**
// API       :: SCHED_STDLIB_ExtractFirstMessage
// PURPOSE   :: Remove the first message from the node's message queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*     : Pointer to the node
//                            to be extracted
// RETURN    ::  Message* : First message from queue
// **/
Message* SCHED_STDLIB_ExtractFirstMessage(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_STDLIB_PeekFirstMessage
// PURPOSE   :: Peek at the first message from the node's message queue
//              NOT REMOVED FROM QUEUE
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
// RETURN    :: Message*    : Pointer to message
// **/
Message* SCHED_STDLIB_PeekFirstMessage(
    Node *node);


// /**
// API       :: SCHED_STDLIB_DeleteMessage
// PURPOSE   :: Delete a message from the nodes's message queue
// PARAMETERS ::
// + node       : Node*     : Pointer to the node
//                            to delete message from
// RETURN    :: void :
// **/
void SCHED_STDLIB_DeleteMessage(
    Node *node,
    Message *msg);


// /**
// API       :: SCHED_STDLIB_InsertNode
// PURPOSE   :: Insert a node into the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition
//                                       to be inserted
// + node           : Node *           : Pointer to the node
// RETURN    :: void :
// **/
void SCHED_STDLIB_InsertNode(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_STDLIB_PeekNextNode
// PURPOSE   :: Peek at next node to process in the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: Node*   :next node to process
// **/
Node* SCHED_STDLIB_PeekNextNode(
    PartitionData *partitionData);


// /**
// API       :: SCHED_STDLIB_DeleteNode
// PURPOSE   :: Delete a node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// + node           : Node*     : Pointer to the node
// RETURN    :: void :
// **/
void SCHED_STDLIB_DeleteNode(
    PartitionData *partitionData,
    Node *node);


// /**
// API       :: SCHED_STDLIB_CurrentNode
// PURPOSE   :: Current node from the partition's scheduler queue
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the node
//                                       to be inserted
// RETURN    :: void :
// **/
Node* SCHED_STDLIB_CurrentNode(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_STDLIB_HasNodes
// PURPOSE   :: Check for nodes in the scheduler's node queue.
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
//
// RETURN    :: BOOL    : scheduler has nodes in it's queue
// **/
BOOL SCHED_STDLIB_HasNodes(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_STDLIB_NextEvent
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// + node           : Node*     : Pointer to the node
// RETURN    :: Message*   : next event
// **/
Message* SCHED_STDLIB_NextEvent(
    PartitionData *partitionData,
    Node* node);


// /**
// API       :: SCHED_STDLIB_NextEventTime
// PURPOSE   :: Get the next event time for this partition
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: clocktype   : time of next event
// **/
clocktype SCHED_STDLIB_NextEventTime(
    const PartitionData *partitionData);


// /**
// API       :: SCHED_Initialize
// PURPOSE   :: Initalize Scheduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void SCHED_STDLIB_Initialize(
    PartitionData *partitionData);


// /**
// API       :: SCHED_Finalize
// PURPOSE   :: Finalize Scheduler
// PARAMETERS ::
// + partitionData  : PartitionData*   : Pointer to the partition data
// RETURN    :: void :
// **/
void SCHED_STDLIB_Finalize(
    PartitionData *partitionData);

#endif /* SCHED_STDLIB_H */
