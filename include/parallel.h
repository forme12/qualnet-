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
// PACKAGE     :: PARALLEL
// DESCRIPTION :: This file describes data structures and functions used for parallel programming.
// **/

#ifndef PARALLEL_H
#define PARALLEL_H

#ifdef PARALLEL //Parallel

#include <stdio.h>

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <set>

#include "clock.h"
#include "main.h"
#include "mac_link.h"

using namespace std;

#ifdef _WIN32
#include <hash_map>
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

// for sending a message to all other partitions
#define PARALLEL_SEND_ALL_OTHERS -1

// /**
// ENUM         :: SynchronizationAlgorithm
// DESCRIPTION  :: Possible algorithms to use in the parallel runtime.
//                 Synchronous is used by default.
// **/
enum SynchronizationAlgorithm {
    SYNCHRONOUS,
    REAL_TIME,  // synchronized against the real time clock
    BEST_EFFORT // unsynchronized (currently unimplemented)
};

// /**
// CONSTANT     :: MAX_THREADS : 512
// DESCRIPTION  :: The maximum number of processes that can be used in
//                 parallel QualNet.  Customers do not receive parallel.cpp,
//                 so cannot effectively change this value.
// **/
#define MAX_THREADS 512

/** Lookahead **/

/*
 * Users of Lookahead/EOT should consider LookaheadHandle's to be opaque.
 * Internally they are just integers - for the array slot value
 * inside the vector of LookaheadLocators held in the LookaheadCalculator.
 */
typedef int LookaheadHandle;

// /**
// STRUCT       :: LookaheadLocator
// DESCRIPTION  :: This struct is allows us to be able to remove from the
//                 LookaheadCalculator's heap. This way lookahead handles can
//                 request they be removed. Internally, as the heap re-heapifies
//                 these locators are updated.
// **/
struct LookaheadLocator {
    int     eotHeapIndex;
};

// /**
// STRUCT       :: EotHeapElement
// DESCRIPTION  :: Basic data structure for simplifying lookahead calculation.
// **/
struct EotHeapElement {

    clocktype eot;
    LookaheadHandle    lookaheadLocatorIndex;
};

// /**
// STRUCT       :: LookaheadCalculator
// DESCRIPTION  :: Stores a heap of EOT elements to calculate lookahead.
// **/
struct LookaheadCalculator {
    EotHeapElement*             eotHeap;
    int                         numberElementsInHeap;
    int                         maxElementsInHeap;
    clocktype                   maxLookahead;
    clocktype                   minLookahead;
    bool                        usingEOT;
    bool                        pastInitPhase;
    std::vector <LookaheadLocator>    *lookaheadLocators;

    LookaheadCalculator();
};

class MessageSendRemoteInfo
{
public:
    MessageSendRemoteInfo (Message * msg, int destPartitionId) :
        m_msg (msg), m_destPartitionId (destPartitionId), m_isOobRequest (false)
    {
    }
    void setOobRequest () { m_isOobRequest = true; }
    bool isOobRequest () { return m_isOobRequest; }

    Message *   m_msg;
    int         m_destPartitionId;        // PARALLEL_SEND_ALL_OTHERS is valid here.
    bool        m_isOobRequest;

};

#define PARALLEL_PROP_DELAY_DEFAULT_INTERVAL (6 * SECOND)
#define PARALLEL_PROP_DELAY_DELTA (0)
//(0.001)

class PARALLEL_PropDelay_NodePositionData
{        
public:
     enum MsgType
    {        
        NODE_POSITION_INFO
    } ;

    PARALLEL_PropDelay_NodePositionData () {
        dynamicPropDelayEnabled = FALSE;
        dynamicPropDelayInterval = PARALLEL_PROP_DELAY_DEFAULT_INTERVAL;
    }
    typedef std::map<int, Coordinates> NODE_POSITION_DATABASE ;    
    typedef NODE_POSITION_DATABASE::const_iterator _C_ITER;
    typedef NODE_POSITION_DATABASE::iterator _ITER;

    bool dynamicPropDelayEnabled ;
    clocktype dynamicPropDelayInterval ;

    Coordinates ReturnNodePosition(int);    

    // for all nodes in the system 
    vector<NODE_POSITION_DATABASE* > dynamicPropDelayDistanceTb ;

    // for nodes in my partition who moved in the interval
    NODE_POSITION_DATABASE nodesChangedPositionTb ;

    std::map<int, double> shortestDistancePartition ;

#ifdef USE_MPI    
    vector<Message*> msgList ;
    vector<Message*> snd_msgList ;
#endif
    
} ;

void PARALLEL_PropDelayObtainNodesOnOtherPartitionPositions(
    PartitionData* partitionData, 
    int            numNodes,
    NodeInput*     nodeInput,
    NodeId*        nodeIdArray,
    NodePositions* nodePositionsPtr) ;

void PARALLEL_PropDelay_SendTimerMessage(
    PartitionData * partitionData);

void PARALLEL_PropDelay_CollectConnectSample(
    Node*    node,
    Message* msg);

clocktype PARALLEL_PropDelay_ReturnPropDelay(
    PartitionData* partition);

//* Parameters related to parallel execution.
extern bool                     g_looseSynchronization;
extern clocktype                g_looseLookahead;
extern SynchronizationAlgorithm g_syncAlgorithm;
extern bool                     g_useDynamicPropDelay;
extern int                      g_numberOfSynchronizations;
extern bool                     g_allowLateDeliveries;
extern bool                     g_useRealTimeThread;
extern bool                     g_trySomethingNew;

// /**
// API :: PARALLEL_AllocateLookaheadHandle
// LAYER   :: Kernel
// PURPOSE :: Obtains a new lookahead handle that allows a protocol
//            to indicate minimum delay values for output. This minimum
//            delay is called EOT - earliest output time.获取一个新的lookahead句柄，该句柄允许协议表示输出的最小延迟值。
//这种最小的延迟称为EOT最早的输出时间。
// PARAMETERS ::
// + node : Node* : the active node
// RETURN :: LookaheadHandle : Returns a reference to the node's lookahead data.
// **/
LookaheadHandle PARALLEL_AllocateLookaheadHandle(Node* node);

// /**
// API :: PARALLEL_AddLookaheadHandleToLookaheadCalculator
// LAYER   :: Kernel
// PURPOSE :: Adds a new LookaheadHandle to the lookahead calculator.
// PARAMETERS ::
// + node            : Node*           : the active node
// + lookaheadHandle : LookaheadHandle : the node's lookahead handle
// + eotOfNode       : clocktype       : the node's EOT
// RETURN :: void :
// **/
void PARALLEL_AddLookaheadHandleToLookaheadCalculator(
    Node*                node,
    LookaheadHandle      lookaheadHandle,
    clocktype            eotOfNode);

// /**
// API :: PARALLEL_SetLookaheadHandleEOT
// LAYER   :: Kernel
// PURPOSE :: Protocols that use EOT will make use of this function more than
//            any other to update the earliest output time as the simulation
//            progresses. Use of EOT is an all-or-nothing option. If your
//            protocol uses EOT, it _must_ use EOT pervasively.
// PARAMETERS ::
// + node            : Node*           : the active node
// + lookaheadHandle : LookaheadHandle : the node's lookahead handle
// + eot             : clocktype       : the node's current EOT
// RETURN :: void :
// **/
void PARALLEL_SetLookaheadHandleEOT(Node*           node,
                                    LookaheadHandle lookaheadHandle,
                                    clocktype       eot);

// /**
// API :: PARALLEL_RemoveLookaheadHandleFromLookaheadCalculator
// LAYER   :: Kernel
// PURPOSE :: Removes a LookaheadHandle from the lookahead calculator.
// PARAMETERS ::
// + node            : Node*           : the active node
// + lookaheadHandle : LookaheadHandle : the node's lookahead handle
// + eotOfNode       : clocktype*      : the node's current EOT
// RETURN :: void :
// **/
void PARALLEL_RemoveLookaheadHandleFromLookaheadCalculator(
    Node*                node,
    LookaheadHandle      lookaheadHandle,
    clocktype*           currentEOT);

// /**
// API :: PARALLEL_SetMinimumLookaheadForInterface
// LAYER   :: Kernel
// PURPOSE :: Sets a minimum delay for messages going out on this interface.
//            This is typically set by the protocol running on that interface.
// PARAMETERS ::
// + node         : Node*     : the active node
// + minLookahead : clocktype : the protocol's minimum lookahead
// RETURN :: void :
// **/
void PARALLEL_SetMinimumLookaheadForInterface(Node*     node,
                                              clocktype minLookahead);

// /**
// FUNCTION :: PARALLEL_InitLookaheadCalculator
// LAYER    :: Kernel
// PURPOSE :: Initializes lookahead calculation.  For kernel use only.
// PARAMETERS ::
// + lookaheadCalculator : LookaheadCalculator : the lookahead calculator
// RETURN :: void :
// **/
void PARALLEL_InitLookaheadCalculator(
    LookaheadCalculator* lookaheadCalculator);


// /**
// FUNCTION     :: PARALLEL_AssignNodesToPartitions
// LAYER        :: Kernel
// PURPOSE      :: Using their positions or other information, assigns
//                 each node to a partition. For kernel use only.
// PARAMETERS   ::
// + numNodes           : int             : the number of nodes
// + numberOfPartitions : int             : the number of partitions
// + nodeInput          : NodeInput*      : the input configuration file
// + nodePos            : NodePosition*   : the node positions
// + map                : AddressMapType* : node ID <--> IP address mappings
// RETURN       :: int : the number of partitions used
// **/
int PARALLEL_AssignNodesToPartitions(int                   numNodes,
                                     int                   numberOfPartitions,
                                     NodeInput*            nodeInput,
                                     NodePositions*        nodePos,
                                     const AddressMapType* map);
// /**
// API          :: PARALLEL_GetPartitionForNode
// LAYER        :: Kernel
// PURPOSE      :: Allows parallel code to determine to what partition a
//                 node is assigned.  If a Node* is available, it's much
//                 quicker to just look it up directly
// PARAMETERS   ::
// + nodeId : NodeId : the node's ID
// RETURN       :: int : the partition to which the node is assigned
// **/
int  PARALLEL_GetPartitionForNode(NodeId nodeId);

// /**
// FUNCTION     :: PARALLEL_InitializeParallelRuntime
// LAYER        :: Kernel
// PURPOSE      :: Sets global variables and stuff. For kernel use only.
// PARAMETERS   ::
// + numberOfThreads : int : the number of processors to use.
// RETURN       :: void :
// **/
void PARALLEL_InitializeParallelRuntime(int numberOfThreads);

// /**
// FUNCTION     :: PARALLEL_CreatePartitionThreads
// LAYER        :: Kernel
// PURPOSE      :: Creates the threads for parallel execution and starts
//                 them running. For kernel use only.
// PARAMETERS   ::
// + numberOfThreads : int            : the number of threads to create.
// + nodeInput       : NodeInput*     : the input configuration
// + partitionArray  : PartitionData* : an array containing the partition data
//                                      structures to give to each thread.
// RETURN       :: void :
// **/
void PARALLEL_CreatePartitionThreads(int            numberOfThreads,
                                     NodeInput*     nodeInput,
                                     PartitionData* partitionArray[]);

/* -----------------------------------------------------------------
 * FUNCTION    PARALLEL_SynchronizePartitions ()
 * PURPOSE     Public api that allow partition creeation and others
 //            to synchronize their steps across all partitions.
 * -----------------------------------------------------------------*/
void
PARALLEL_SynchronizePartitions(PartitionData* partitionData);

// /**
// FUNCTION     :: PARALLEL_GetRemoteMessages
// LAYER        :: Kernel
// PURPOSE      :: Collects all the messages received from other partitions.
//                 For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       :: void :
// **/
void PARALLEL_GetRemoteMessages(PartitionData* partitionData);

// /**
// FUNCTION     :: PARALLEL_GetRemoteMessagesAndBarrier
// LAYER        :: Kernel
// PURPOSE      :: Collects all the messages received from other partitions.
//                 This function also acts as a barrier. For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       :: void :
// **/
void PARALLEL_GetRemoteMessagesAndBarrier(PartitionData* partitionData);

// /**
// API          :: PARALLEL_SendRemoteMessages
// LAYER        :: Kernel
// PURPOSE      :: Sends one or more messages to a remote partition.
//                 For kernel use only.
// PARAMETERS   ::
// + msgList       : Message*       : a linked list of Messages
// + partitionData : PartitionData* : a pointer to the partition
// + partitionId   : int            : the partition's ID
// RETURN       ::  :
// **/
void PARALLEL_SendRemoteMessages(Message*       msgList,
                                 PartitionData* partitionData,
                                 int            partitionId);

// /**
// API          :: PARALLEL_DeliverRemoteMessages
// LAYER        :: Kernel
// PURPOSE      :: Delivers cached messages to all remote partitions.
//                 For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       ::  :
// **/
void PARALLEL_DeliverRemoteMessages(PartitionData* partitionData);

// /**
// API          :: PARALLEL_SendRemoteMessagesOob
// LAYER        :: Kernel
// PURPOSE      :: Sends one or more messages to a remote partition.
//                 These messages are oob messages and will be
//                 processed immediately.
//                 For kernel use only.
// PARAMETERS   ::
// + msgList       : Message*       : a linked list of Messages
// + partitionData : PartitionData* : a pointer to the partition
// + partitionId   : int            : the partition's ID
// + isResponse    : bool           : if it's a response to an OOB message
// RETURN       :: void :
// **/
void PARALLEL_SendRemoteMessagesOob(Message*       msgList,
                                    PartitionData* partitionData,
                                    int            partitionId,
                                    bool           isResponse);
// /**
// API        :: PARALLEL_SendMessageToAllPartitions
// LAYER      :: Kernel
// PURPOSE    :: Sends a message to all remote partitions, but not the current
//               one. By default, duplicates will be sent to all remote
//               partitions and the original freed, but if freeMsg is false,
//               the original message will not be freed.
// PARAMETERS ::
// + msg           : Message*       : the message(s) to send
// + partitionData : PartitionData* : the sending partition
// + freeMsg       : bool           : whether or not to free the original
//                                    message.
// RETURN :: void :
// **/
void PARALLEL_SendMessageToAllPartitions(Message*       msg,
                                         PartitionData* partitionData,
                                         bool           freeMsg = true);

// /**
// API          :: PARALLEL_SendRemoteLinkMessage
// LAYER        :: Kernel
// PURPOSE      :: Sends one LINK message to a remote partition.
// PARAMETERS   ::
// + node    : Node*     : the sending node
// + msg     : Message*  : the message to be sent
// + link    : LinkData* : info about the link
// + txDelay : clocktype : the transmission delay, not including propagation
// RETURN       ::  :
// **/
void PARALLEL_SendRemoteLinkMessage(Node*     node,
                                    Message*  msg,
                                    LinkData* link,
                                    clocktype txDelay,
                                    WirelessLinkSiteParameters* params);

// /**
// FUNCTION     :: PARALLEL_UpdateSafeTime
// LAYER        :: Kernel
// PURPOSE      :: A generic function for calculating the window of safe events
//                 For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       :: void :
// **/
void PARALLEL_UpdateSafeTime(PartitionData* partitionData);

// /**
// FUNCTION     :: PARALLEL_ReturnEarliestGlobalEventTime
// LAYER        :: Kernel
// PURPOSE      :: Returns the earliest global event time.  Required for
//                 interfacing to time-sensitive external programs.
//                 For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       :: clocktype : the time of the earliest event across all
//                             partitions
// **/
clocktype PARALLEL_ReturnEarliestGlobalEventTime(PartitionData* partitionData);

// /**
// FUNCTION     :: PARALLEL_Exit
// LAYER        :: Kernel
// PURPOSE      :: Exits from the parallel system, killing threads, etc.
//                 For kernel use only.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN       :: void :
// **/
void PARALLEL_Exit(PartitionData* partitionData);

// /**
// API          :: PARALLEL_SetProtocolIsNotEOTCapable
// LAYER        :: Kernel
// PURPOSE      :: Currently, EOT can only be used if supported by all
//                 protocols running in the scenario.  If any protocol
//                 is not capable, only the minimum lookahead is used.
// PARAMETERS   ::
// + node : Node* : the node's data
// RETURN       :: void :
// **/
void PARALLEL_SetProtocolIsNotEOTCapable(Node* node);

// /**
// API     :: PARALLEL_EnableDynamicMobility
// LAYER   :: Kernel
// PURPOSE :: Forces the runtime to consider mobility events when calculating
//            EOT/ECOT.  Mobility events are ignored by default.
//            This function should be called during the initialization of
//            models where changes in position or direction of one node may affect
//            the behavior of other nodes.
// PARAMETERS ::
// RETURN :: void :
// **/
void PARALLEL_EnableDynamicMobility();

// /**
// API     :: PARALLEL_SetGreedy
// LAYER   :: Kernel
// PURPOSE :: Tells the kernel to use spin locks on barriers if true, or
//            to use blocking barriers otherwise.  In greedy mode, the
//            Simulator needs a dedicated CPU per partition.
// 
// PARAMETERS ::
// + greedy : bool : should it be greedy or not?
// RETURN :: void :
// **/
void PARALLEL_SetGreedy(bool isItGreedy = true);

// /**
// API     :: PARALLEL_IsGreedy
// LAYER   :: Kernel
// PURPOSE :: Checks whether SetGreedy has been called.
// 
// PARAMETERS ::
// RETURN :: bool : true if greedy mode is enabled.
// **/
bool PARALLEL_IsGreedy();

// /**
// API     :: PARALLEL_PreFlight
// LAYER   :: Kernel
// PURPOSE :: Initializes parallel operation.
// 
// PARAMETERS ::
// + partitionData : PartitionData* : the partition to initialize.
// RETURN :: void :
// **/
void PARALLEL_PreFlight(PartitionData* partitionData);

// /**
// API     :: PARALLEL_ScheduleMessagesOnPartition
// LAYER   :: Kernel
// PURPOSE :: Takes a list of messages or an OOB message and schedules
//            them for execution on the current partition.  Typically
//            these messages have arrived from a remote partition.
// 
// PARAMETERS ::
// + partitionData : PartitionData* : the partition.
// + msgList       : Message*       : a list of normal simulation messages.
// + oobMessage    : Message**      : an out of bounds message.
// + gotOobMessage : bool*          : returns true if Oob response is received
// + isMT          : bool           : is this called from a worker thread
// RETURN :: void :
// **/
void PARALLEL_ScheduleMessagesOnPartition(
    PartitionData* partitionData,
    Message*       msgList,
    Message**      oobMessage = NULL,
    bool*          gotOobMessage = NULL,
    bool           isMT = false);

// /**
// API     :: PARALLEL_EndSimulation
// LAYER   :: Kernel
// PURPOSE :: Shuts down the parallel engine, including whatever
//            synchronization is required.
// 
// PARAMETERS ::
// + partitionData : PartitionData* : the partition to terminate.
// RETURN :: void :
// **/
void PARALLEL_EndSimulation(PartitionData* partitionData);

// /**
// API     :: PARALLEL_BuildStatFile
// LAYER   :: Kernel
// PURPOSE :: Builds the final stat file when running in parallel node.
//            Should only be called once from partition 0.
// PARAMETERS ::
// + numPartitions    : int            : number of partitions
// + statFileName   : char*            : name of stat file
// + experimentPrefix : char*          : experiment prefix
// RETURN :: void :
// **/
void PARALLEL_BuildStatFile(int numPartitions,
                            char* statFileName,
                            char* experimentPrefix);

// /**
// API     :: PARALLEL_NumberOfSynchronizations
// LAYER   :: Kernel
// PURPOSE :: Return the number of synchronizations performed per partition
// PARAMETERS ::
// RETURN :: void :
// **/
int PARALLEL_NumberOfSynchronizations();

// /**
// API     :: PARALLEL_StartRealTimeThread
// LAYER   :: Kernel
// PURPOSE :: Tells the kernel to use an independent thread to constantly
//            update realtime.
// PARAMETERS   ::
// + partitionData : PartitionData* : a pointer to the partition
// RETURN :: void :
// **/
void PARALLEL_StartRealTimeThread(PartitionData* partitionData);

#endif //endParallel

#endif /* _PARALLEL_H_ */
