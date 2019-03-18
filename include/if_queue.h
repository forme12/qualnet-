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
// PROTOCOL :: Queue-Scheduler
//
// SUMMARY  :: FIFO, as the name implies First In First Out, dequeues a
// packet first that entered first in that queue. It uses the simplest queue
// management algorithm, i.e., the Drop Tail, where packets are dropped when
// there is no more space in that queue buffer.FIFO has several limitations.
// Since packets are discarded at burst (when the queue is full all the
// incoming packets are discarded) the utilization of the network varies
// quite a lot. When a certain number of applications are sending packets
// from different source with same priority value, they will be queued in
// the same queue at some intermediate node, to reach destination. Some of
// those applications may have congestion control (such as TCP) that react
// at the same time decreasing their sending rate. In this case the network
// oscillates between high traffic and low traffic and it is not able to
// create equilibrium. Moreover there can be malicious effects due to the
// source synchronization, where one source does not experience any drop and
// another does.
//
// LAYER ::
//
// CONFIG_PARAM ::
//
// VALIDATION :: $QUALNET_HOME/verification/fifo
//
// IMPLEMENTED_FEATURES :: When a certain number of applications are sending
// packets from different source with same priority value, they will be
// queued in the same queue at some intermediate node, to reach destination.
// Some of those applications may have congestion control (such as TCP) that
// react at the same time decreasing their sending rate. In this case the
// network oscillates between high traffic and low traffic and it is not
// able to create equilibrium. Moreover there can be malicious effects due
// to the source synchronization, where one source does not experience any
// drop and another does.
//
// OMITTED_FEATURES :: None
//
// ASSUMPTIONS :: None
//
// STANDARD ::
//
// RELATED :: None
// **/

// /**
// PACKAGE     :: QUEUES
// DESCRIPTION :: This file describes the member functions of the queue base class.
// **/

#ifndef IF_QUEUE_H
#define IF_QUEUE_H

#include "dynamic.h"
#include "stats_queue.h"
#ifdef ADDON_DB
namespace StatsQueueDB
{
    class QueueStatusTbBuilder;
    class StatsQueueEventHook;
}
struct MetaDataStruct;
#endif

#ifdef ADDON_BOEINGFCS
#include <google/dense_hash_map>
#endif

// /**
// ENUM        :: QueueBehavior
// DESCRIPTION :: This enumeration is used by both queues and schedulers
//                to determine the queue behavior.
// **/
enum QueueBehavior
{
    RESUME = 0, // 0 : RESUME the suspended queue
    SUSPEND = 1,// 1 : Suspend the queue
};

// /**
// ENUM        :: QueueOperation
// DESCRIPTION :: This enumeration is used by both queues and schedulers
//                to determine the operation of the retrieve functions.
// **/

enum QueueOperation
{
    PEEK_AT_NEXT_PACKET, // 0 : Handles false dequeue functionality
    DEQUEUE_PACKET,      // 1 : Handles dequeue functionality
    DISCARD_PACKET,      // 2 : Handles drop functionality
    DROP_PACKET,         // 3 : Handles forcefully drop functionality
    DROP_AGED_PACKET     // 4 : Handles forcefully drop due to aging
};


// /**
// CONSTANT    :: DEQUEUE_NEXT_PACKET : 0
// DESCRIPTION :: Denotes position of packet in the queue for dequeue
//                operation
// **/

#define DEQUEUE_NEXT_PACKET 0


// /**
// CONSTANT    :: ALL_PRIORITIES : -1
// DESCRIPTION :: This macro is used to specify that queue and scheduler
//                operations not consider priority value of queue or
//                packet
// **/

#define ALL_PRIORITIES -1


// /**
// CONSTANT    :: QOS_DEFAULT_INTERFACE_OBSERVATION_INTERVAL : 2 * SECOND
// DESCRIPTION :: This macro is used to specify the interface observation
//                interval for Qos Routing. Ref.(Qospf.h see
//                QOSPF_DEFAULT_INTERFACE_OBSERVATION_INTERVAL)
// **/

#define QOS_DEFAULT_INTERFACE_OBSERVATION_INTERVAL  (2 * SECOND)


// /**
// CONSTANT    :: STATISTICS_RESOLUTION : 1 * MICRO_SECOND
// DESCRIPTION :: This macro is used to support overflow issue to account
//                for long delay network such as in space applications, or
//                simply very very long simulations, divide delays
//                by STATISTICS_RESOLUTION during runtime, and multiply by
//                STATISTICS_RESOLUTION at the end of the simulation
//                when IO_PrintStat'ing
// **/

#define STATISTICS_RESOLUTION  (1 * MICRO_SECOND)


// /**
// CONSTANT    :: DEFAULT_QUEUE_DELAY_WEIGHT_FACTOR : 0.1
// DESCRIPTION :: This macro is used to define the weight to assign to
//                the most recent delay in calculating an exponential
//                moving average.  The value is fairly large because
//                the queue delay is used for QoS routing decisions.
// **/

#define DEFAULT_QUEUE_DELAY_WEIGHT_FACTOR  (0.1)


// /**
// CONSTANT    :: PACKET_ARRAY_INFO_FIELD_SIZE : 32
// DESCRIPTION :: The Queue structure will store a field of data in
//                addition to the Message itself, with a maximum size
//                of this value
// **/

#define PACKET_ARRAY_INFO_FIELD_SIZE 32


// /**
// STRUCT      :: PacketArrayEntry
// DESCRIPTION :: This structure represents an entry in the array of
//                stored messages.  The infoField (perhaps this should be
//                renamed to prevent confusion) will store a queue algorithm
//                dependent amount of data about each Message, as well as
//                the simulation time that Message was inserted.
// **/

typedef struct packet_array_entry_str
{
    Message* msg;
    clocktype insertTime;
    double infoField[PACKET_ARRAY_INFO_FIELD_SIZE / sizeof(double)];
    double serviceTag;
} PacketArrayEntry;

// /**
// STRUCT      :: QueueAgeInfo
// DESCRIPTION :: This structure contains information for each packet
//                inserted into the queue to uniquely identify it 
//                so that it can be removed from the queue due to age.
// **/

typedef struct queue_age_info_str
{
    int qNumber;
    Message* msgPtr;
    NodeId originatingNodeId;
    int sequenceNumber;
} QueueAgeInfo;


// /**
// CLASS       :: Queue
// DESCRIPTION :: This class contains array information, configuration
//                variables, default function behaviors, and statistics
//                for the queue.  All queue disciplines (except FIFO)
//                should be derived from this class.  The "void *dataPtr"
//                is gone, since derived classes can include any state
//                variables it needs to maintain.
// **/

#ifdef ADDON_BOEINGFCS

typedef
struct queue_per_dscp_stats
{
    Int64 numPacketsQueued;
    Int64 numPacketsDequeued;
    D_Float64 numPacketsDropped;
} QueuePerDscpStats;


typedef 
google::dense_hash_map <int,
    QueuePerDscpStats*> QueuePerDscpMap;

typedef 
google::dense_hash_map <int,
QueuePerDscpStats*>::iterator QueuePerDscpMapIter;
#endif

// Forward Declaration
class Scheduler;
class Queue;

class Queue
{
    friend class STAT_StatisticsList;
#ifdef ADDON_DB
    friend class StatsQueueDB::QueueStatusTbBuilder;
#endif  
  protected:
    Node* node; // Store pointer passed along with SetupQueue()
    PacketArrayEntry* packetArray;
    int numPackets;
    int maxPackets;
    int infoFieldSize;

    int bytesUsed;
    int queueSizeInBytes;

    char queueType[MAX_STRING_LENGTH];

#ifdef ADDON_BOEINGFCS
    BOOL collectDscpStats;
    QueuePerDscpMap *dscpStats;
    //Scheduler/Queue represented by this queue
    Scheduler *representedSched;
    Queue* representedQueue;
#endif

    int headIndex;
    int tailIndex;

    // Resource Management features
    BOOL queueSuspended;

    // QOS interface observation statistics
    int qDelay;
    int totalTransmission;
    clocktype qosMonitorInterval;

    clocktype queueCreationTime;

    // currentPeriod statistics
    clocktype currentStateStartTime;
    clocktype utilizedTime;
    BOOL stateIsIdle;
    int bytesDequeuedInPeriod;
    int packetsDequeuedInPeriod;
    clocktype currentPeriodStartTime;
    clocktype queueDelaysDuringPeriod;

    // standard statistics collected
    float delayAveragingWeight; // used to calculate running average delay
    BOOL isCollectStats;
    int numBytesDropped;

    clocktype lastChange;

    clocktype maxPktAge;
    int queueNumber;

    // Utility functions
    inline int  RetriveArrayIndex(int index);

    inline void UpdateQueueDelayStats(int packetArrayIndex,
                               const clocktype currentTime);

    void FinalizeQueue(Node* node,
                      const char* layer,
                      const char* protocol,
                      const int interfaceIndex,
                      const int instanceId,
                      const char* invokingProtocol);

    void FinalizeQueue(Node* node,
                      const char* layer,
                      const char* protocol,
                      const int interfaceIndex,
                      const int instanceId,
                      const int fcsQosQueue,
                      const char* invokingProtocol);

/*====================================================================*/
#ifdef ADDON_DB    
    StatsQueueDB::StatsQueueEventHook *queueDBHookPtr;    
#endif
/*====================================================================*/

  public:
    
/*====================================================================*/
#ifdef ADDON_DB    
      virtual StatsQueueDB::StatsQueueEventHook * 
          returnQueueDBHook() {return queueDBHookPtr ;}
#endif
/*====================================================================*/
    Queue() {
#ifdef ADDON_DB
        meta_data = NULL;
#endif
    };
    ~Queue();

     int getRetriveArrayIndex(int index);//YG
	 PacketArrayEntry* getpacketArray();  //YG

    virtual void SetupQueue(Node* node,
                            const char queueTypeString[],
                            const int queueSize,
                            const int infoFieldSize = 0,
                            const int interfaceIndex = 0, 
                            const int queueNumber = 0, 
                            const BOOL enableQueueStat = FALSE,
                            const BOOL showQueueInGui = FALSE,
                            const clocktype currentTime = 0,
                            const void* configInfo = NULL
#ifdef ADDON_DB
                            ,const char* queuePositiion = NULL
#endif
                            , const clocktype maxPktAge = CLOCKTYPE_MAX
#ifdef ADDON_BOEINGFCS
                            , const BOOL enablePerDscpStat = FALSE
                            ,Scheduler* repSched = NULL
                            ,Queue* repQueue = NULL
#endif
                            );

    virtual void insert(Message* msg,
                        const void* infoField,
                        BOOL* QueueIsFull,
                        const clocktype currentTime,
                        const double serviceTag = 0.0);

    virtual void insert(Node* node,
                        int interfaceIndex,
                        int priority,
                        Message* msg,
                        const void* infoField,
                        BOOL* QueueIsFull,
                        const clocktype currentTime,
                        const double serviceTag = 0.0);
    virtual void insert(Message* msg,
                        const void* infoField,
                        BOOL* QueueIsFull,
                        const clocktype currentTime,
                        TosType* tos,
                        const double serviceTag = 0.0);

    virtual void insert(Node* node,
                        int interfaceIndex,
                        int priority,
                        Message* msg,
                        const void *infoField,
                        BOOL* QueueIsFull,
                        const clocktype currentTime,
                        TosType* tos,
                        const double serviceTag = 0.0);

    virtual BOOL retrieve(Message** msg,
                          const int index,
                          const QueueOperation operation,
                          const clocktype currentTime,
                          double* serviceTag = NULL);

    virtual BOOL isEmpty(BOOL checkPredecessor = FALSE);

    virtual int bytesInQueue(BOOL checkPredecessor = FALSE);

    virtual int freeSpaceInQueue();

    virtual int packetsInQueue(BOOL checkPredecessor = FALSE);

    int sizeOfQueue();

    void setServiceTag(double serviceTag);

    virtual int replicate(Queue* newQueue);

    // Resource Management API
    void setQueueBehavior(BOOL suspend = FALSE);

    BOOL getQueueBehavior();

    // QOS interface observation API
    virtual void qosQueueInformationUpdate(int* qDelayVal,
                                int* totalTransmissionVal,
                                const clocktype currentTime,
                                BOOL isResetTotalTransmissionVal = FALSE);

    // CurrentPeriod statistics API
    inline int byteDequeuedInPeriod();

    inline clocktype utilizationInPeriod();

    inline clocktype averageTimeInQueue();

    inline void resetPeriod (clocktype currentTime);

    inline clocktype periodStartTime();


    virtual void finalize(Node* node,
                          const char* layer,
                          const int interfaceIndex,
                          const int instanceId,
                          const char* invokingProtocol = "IP",
                          const char* splStatStr = NULL);

    virtual BOOL getQueueStats();

    virtual void* getConfigParams();
    
    clocktype getPacketInsertTime(int pktIndex);

    virtual char* getQueueType();

    int getQueueNumber();

#ifdef ADDON_BOEINGFCS 
    //virtual char* getQueueType();
    clocktype getTopPacketInsertTime();
    QueuePerDscpMap *getDscpStats()
    {
        return dscpStats;
    };
    BOOL getCollectDscpStats()
    {
        return collectDscpStats;
    }

    double getHighWaterMark();
    double getLowWaterMark();

    virtual void signalCongestion(Node* node,
                                  int interfaceIndex,
                                  Message* msg);
#endif

#ifdef ADDON_DB
    // The DB meta data values for this queue
    MetaDataStruct* meta_data;
#endif

    STAT_QueueStatistics* stats;
};


//--------------------------------------------------------------------------
// Interface For Queue Setup
//--------------------------------------------------------------------------


// FUNCTION   :: QUEUE_Setup
// PURPOSE    :: This function runs the generic and then the
//               algorithm-specific queue initialization routine.
// PARAMETERS ::
// + node  : Node* : Pointer to node that owns the queue
// + queue : Queue** : Pointer of pointer to Queue class
// + queueTypeString[] : const char : Queue Type as specified in config file
// + queueSize : const int : Queue Size
// + intefaceIndex : const int : Used to set random seed value for each queue
// + queueNumber : const int : Used to set random seed value for each queue
// + infoFieldSize : const int : Default value 0;
// + enableQueueStat : const BOOL : Default value false
// + showQueueInGui : const BOOL : To show this Queue in GUI
// + currentTime : const clocktype : Current simulation time.
// + configInfo : const void* : Pointer to a structure that contains
//                              configuration parameters for a Queue Type
//                              This is not required for FIFO. Is it is
//                              found NULL for any other Queue Type the
//                              default setting will be applied for that
//                              queue Type. Default is NULL.
// RETURN     :: void : Null
// **/

void QUEUE_Setup(
    Node* node,
    Queue** queue,
    const char queueTypeString[],
    const int queueSize,
    const int interfaceIndex,
    const int queueNumber,
    const int infoFieldSize = 0,
    const BOOL enableQueueStat = FALSE,
    const BOOL showQueueInGui = FALSE,
    const clocktype currentTime = 0,
    const void* configInfo = NULL,
    const clocktype maxPktAge = CLOCKTYPE_MAX
#ifdef ADDON_DB
    , const char * queuePosition = NULL
#endif
#ifdef ADDON_BOEINGFCS
    , const BOOL enablePerDscpStat = FALSE
    ,Scheduler* repSched = NULL
    ,Queue* repQueue = NULL
#endif
    );

void QUEUE_HandleEvent(Node* node, Message* msg);

// Comments for Public Interface Functions copied below
// from source file to generate API Reference



// /**
// FUNCTION   :: Queue::insert
// PURPOSE    :: This function prototype determines the arguments that need
//               to be passed to a queue data structure in order to insert
//               a message into it.  The infoField parameter has a specified
//               size infoFieldSize, which is set at Initialization, and
//               points to the structure that should be stored along
//               with the Message.
// PARAMETERS ::
// + msg : Message* : Pointer to Message structure
// + infoField : const void* : The infoField parameter
// + QueueIsFull : BOOL* : returns Queue occupancy status
// + currentTime : const clocktype : Current Simulation time
// + serviceTag : const double : ServiceTag
// RETURN     :: void : Null
// **/

// /**
// FUNCTION   :: Queue::retrieve
// PURPOSE    :: This function prototype determines the arguments that need
//               to be passed to a queue data structure in order to dequeue,
//               peek at, or drop a message in its array of stored messages.
//               It now includes the "DropFunction" functionality as well.
// PARAMETERS ::
// + msg : Message** : The retrieved msg
// + index : const int : The position of the packet in the queue
// + operation : const QueueOperation : The retrieval mode
// + currentTime : const clocktype : Current Simulation time
// + serviceTag : double* : ServiceTag = NULL
// RETURN     :: BOOL : TRUE or FALSE
// **/

// /**
// FUNCTION   :: Queue::isEmpty
// PURPOSE    :: This function prototype returns a Boolean value of true
//               if the array of stored messages is empty, false otherwise.
// PARAMETERS :: None
// RETURN     :: BOOL : TRUE or FALSE
// **/

// /**
// FUNCTION   :: Queue::bytesInQueue
// PURPOSE    :: This function prototype returns the number of bytes stored
//               in the array.
// PARAMETERS :: None
// RETURN     :: int : Integer
// **/

// /**
// FUNCTION   :: Queue::freeSpaceInQueue
// PURPOSE    :: This function prototype returns free space in number of
//               bytes in the queue.
// PARAMETERS ::
// RETURN     :: int: number of bytes free.
// **/

// /**
// FUNCTION   :: Queue::packetsInQueue
// PURPOSE    :: This function prototype returns the number of Messages
//               stored in the packetArray.
// PARAMETERS :: None
// RETURN     :: int : Integer
// **/

// /**
// FUNCTION   :: Queue::sizeOfQueue
// PURPOSE    :: This function prototype returns the size of the Queue
// PARAMETERS :: None
// RETURN     :: int : Integer
// **/

// /**
// FUNCTION   :: Queue::setServiceTag
// PURPOSE    :: Set the service tag of the queue
// PARAMETERS ::
// + serviceTag : double : the value of the service tag
// RETURN     :: void : NULL
// **/

// /**
// FUNCTION   :: Queue::replicate
// PURPOSE    :: This function is proposed to replicate the state of the
//               queue, as if it had been the operative queue all along.
//               If there are packets in the existing queue, they are
//               transferred one-by-one into the new queue. This can result
//               in additional drops of packets that had previously been
//               stored. This function returns the number of additional
//               drops.
// PARAMETERS ::
// + oldQueue : Queue* : Old queue pointer
// RETURN     :: int : Old packetArray
// **/

// /**
// FUNCTION   :: Queue::setQueueBehavior
// PURPOSE    :: This function is proposed to identify and tag misbehaved
//               queue at the interface, so that they can be punished.
// PARAMETERS ::
// + suspend : BOOL : The queue status
// RETURN     :: void : Null
// **/

// /**
// FUNCTION   :: Queue::qosQueueInformationUpdate
// PURPOSE    :: This function is proposed for qos information update
//               for Qos Routings like Qospf.
// PARAMETERS ::
// + qDelayVal : int* : Returning qDelay value
// + totalTransmissionVal : int* : Returning totalTransmission value
// + currentTime : const clocktype : Current simulation time
// + isResetTotalTransmissionVal : BOOL : Default false
// RETURN     :: void : Null
// **/

// /**
// FUNCTION   :: Queue::byteDequeuedInPeriod
// PURPOSE    :: This function prototype returns the number of bytes dequeued,
//               not dropped, during a given period.  This period starts at
//               the beginning of the simulation, and restarts whenever the
//               Queue resetPeriod function is called.
// PARAMETERS :: None
// RETURN     :: int : Integer
// **/

// /**
// FUNCTION   :: Queue::utilizationInPeriod
// PURPOSE    :: This function prototype returns the queue utilization, or
//               the amount of time that the queue is nonempty, during a
//               given period. This period starts at the beginning of the
//               simulation, and restarts whenever the queue resetPeriod
//               function is called.
// PARAMETERS :: None
// RETURN     :: clocktype : Utilize Time.
// **/

// /**
// FUNCTION   :: Queue::averageTimeInQueue
// PURPOSE    :: This function prototype returns the average time a packet
//               spends in the queue, during a given period.  This period
//               starts at the beginning of the simulation, and restarts
//               whenever the QueueResetPeriodFunctionType function is called.
// PARAMETERS :: None
// RETURN     :: clocktype : Queue Delays.
// **/

// /**
// FUNCTION   :: Queue::resetPeriod
// PURPOSE    :: This function prototype resets the current period statistics
//               variables, and sets the currentPeriodStartTime to the
//               currentTime.
// PARAMETERS ::
// + currentTime : clocktype : Current simulation time.
// RETURN     :: void : Null
// **/

// /**
// FUNCTION   :: Queue::periodStartTime
// PURPOSE    :: This function prototype returns the currentPeriodStartTime.
// PARAMETERS ::
// RETURN     :: clocktype : Current period start time.
// **/

// /**
// FUNCTION   :: Queue::finalize
// PURPOSE    :: This function prototype outputs the final statistics for
//               this queue.  The layer, protocol, interfaceAddress, and
//               instanceId parameters are given to IO_PrintStat with each
//               of the queue's statistics.
// PARAMETERS ::
// + node : Node* : Pointer to Node structure
// + layer : const char* : The layer string
// + interfaceIndex : const int : The interface index
// + instanceId : const int : Instance Ids
// + invokingProtocol : const char* : The protocol string
// + splStatStr : const char* : Special string for stat print
// RETURN     :: void : Null
// **/

// /**
// FUNCTION   :: Queue::SetupQueue
// PURPOSE    :: This function runs queue initialization routine. Any
//               algorithm specific configurable parameters will be kept in
//               a structure and after feeding that structure the structure
//               pointer will be sent to this function via that void pointer
//               configInfo. Some parameters includes default values, to
//               prevent breaking existing models. [Uses: vide Pseudo code]
// PARAMETERS ::
// + node : Node* : Node pointer
// + queueTypeString[] : const char : Queue type string
// + queueSize : const int : Queue size in bytes
// + interfaceIndex : const int : used to set random seed
// + queueNumber : const int : used to set random seed
// + infoFieldSize : const int : Default infoFieldSize = 0
// + enableQueueStat : const BOOL : Default enableQueueStat = false
// + showQueueInGui : const BOOL : If want to show this Queue in GUI
//                                 Default value is false;
// + currentTime : const clocktype : Current simulation time
// + configInfo : const void* : pointer to a structure that contains
//                              configuration parameters for a Queue Type.
//                              This is not required for FIFO. Is it is
//                              found NULL for any other Queue Type the
//                              default setting will be applied for that
//                              queue Type. Default value is NULL
// RETURN     :: void : Null
// **/

#endif //IF_QUEUE_H

