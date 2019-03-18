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
// SUMMARY  :: General Scheduler file
//
// LAYER ::
//
// STATISTICS::
//
// CONFIG_PARAM ::
//
// VALIDATION ::
//
// IMPLEMENTED_FEATURES ::
//
// OMITTED_FEATURES ::
//
// ASSUMPTIONS ::
//
// STANDARD ::
//
// RELATED ::
// **/

// /**
// PACKAGE     :: SCHEDULERS
// DESCRIPTION :: This file describes the member functions of the scheduler base class.
// **/


#ifndef IF_SCHEDULER_H
#define IF_SCHEDULER_H

#include "if_queue.h"

//--------------------------------------------------------------------------
// Scheduler API
//--------------------------------------------------------------------------

// /**
// CONSTANT    :: DEFAULT_QUEUE_COUNT : 3
// DESCRIPTION :: Default number of queue per interface
// **/

#define DEFAULT_QUEUE_COUNT 3


// /**
// STRUCT      :: QueueData
// DESCRIPTION :: This structure contains pointers to queue structures,
//                default function behaviors, and statistics for the
//                scheduler
// **/

typedef struct queue_data_str
{
    Queue* queue;
    int priority;
    float weight;
    float rawWeight;
    char* infoField;
} QueueData;


//class SchedGraphStat;

// /**
// CLASS       :: Scheduler
// DESCRIPTION :: Scheduler abstract base class
// **/

class Scheduler
{
    friend class STAT_StatisticsList;
#ifdef ADDON_DB
    friend class StatsQueueDB::QueueStatusTbBuilder;
#endif
protected:
    QueueData*    queueData;
    int           numQueues;
    int           maxQueues;
    int           infoFieldSize;
    int           packetsLostToOverflow;

    // CurrentPeriod statistics
    clocktype               currentStateStartTime;
    clocktype               utilizedTime;
    BOOL                    stateIsIdle;
    int                     bytesDequeuedInPeriod;
    int                     packetsDequeuedInPeriod;
    clocktype               currentPeriodStartTime;
    clocktype               queueDelaysDuringPeriod;

    // Scheduler statistic collection
    BOOL                    schedulerStatEnabled;
    void*         schedGraphStatPtr;
    // Utility function for packet retrieval from specified priority queue
    QueueData*    SelectSpecificPriorityQueue(int priority);


public:
    void setRawWeight(const int priority, double rawWeight);
    void normalizeWeight();
	 QueueData* getqueueData();  //YG 
	 int getnumQueues();  //YG

    virtual int numQueue();
    virtual int GetQueuePriority(int queueIndex);

    virtual void insert(Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime) = 0;

    virtual void insert(Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime,
                        TosType* tos) = 0;

    virtual void insert(Node* node,
                        int interfaceIndex,
                        Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime) = 0;

    virtual void insert(Node* node,
                        int interfaceIndex,
                        Message* msg,
                        BOOL* QueueIsFull,
                        const int priority,
                        const void* infoField,
                        const clocktype currentTime,
                        TosType* tos) = 0;

    virtual BOOL retrieve(const int priority,
                          const int index,
                          Message** msg,
                          int* msgPriority,
                          const QueueOperation operation,
                          const clocktype currentTime) = 0;

    // Resource Management API
    virtual void setQueueBehavior(const int priority,
                                  QueueBehavior suspend = RESUME);
    virtual QueueBehavior getQueueBehavior(const int priority);

    virtual BOOL isEmpty(const int priority,
                         BOOL checkPredecessor = FALSE);
#ifdef ADDON_BOEINGFCS
    virtual BOOL isEmpty(Node* node, const int priority, BOOL checkPredecessor = FALSE);
    virtual int sizeOfQueue(const int priority);
    virtual double weightOfQueue(const int priority);
    virtual char* getQueueType(const int priority);
    virtual BOOL getQueueStats(const int priority);
    virtual void* getConfigParams(const int priority);
    virtual QueuePerDscpMap* getPerDscpStats(const int priority);
    virtual BOOL getCollectDscpStats(const int priority);
#endif

    virtual int bytesInQueue(const int priority,
                             BOOL checkPredecessor = FALSE);

    virtual int numberInQueue(const int priority,
                              BOOL checkPredecessor = FALSE);

    virtual int addQueue(Queue* queue,
                         const int priority = ALL_PRIORITIES,
                         const double weight = 1.0) = 0;

    virtual void removeQueue(const int priority) = 0;

    virtual void swapQueue(Queue* queue, const int priority) = 0;

    virtual void qosInformationUpdate(int queueIndex,
                                      int* qDelayVal,
                                      int* totalTransmissionVal,
                                      const clocktype currentTime,
                                      BOOL isResetTotalTransmissionVal = FALSE);

    // Scheduler current period statistic collection
    //virtual int bytesDequeuedInPeriod(const int priority);
    //virtual clocktype utilizationInPeriod(const int priority);
    //virtual clocktype averageTimeInQueueDuringPeriod(const int priority);
    //virtual int resetPeriod(const clocktype currentTime);
    //virtual clocktype periodStartTime();

    // Scheduler statistic collection for graph
    virtual void collectGraphData(int priority,
                                  int packetSize,
                                  const clocktype currentTime);

    virtual void invokeQueueFinalize(Node* node,
                                     const char* layer,
                                     const int interfaceIndex,
                                     const int instanceId,
                                     const char* invokingProtocol = "IP",
                                     const char* splStatStr = NULL);

    virtual void finalize(Node* node,
                          const char* layer,
                          const int interfaceIndex,
                          const char* invokingProtocol = "IP",
                          const char* splStatStr = NULL) = 0;

    // used by CES
    virtual clocktype getTopPacketInsertTime()
    {
        return 0;
    }

    virtual clocktype getPacketInsertTime(int priority, int index)
    {
        int i = 0;

        if (priority == ALL_PRIORITIES)
        {
            ERROR_ReportError("Doesnt make sense to call this function with "
                              "priority = ALL_PRIORITIES");
        }
        else
        {
            for (i = 0; i < numQueues; i++)
            {
                if (priority == queueData[i].priority)
                {
                    return queueData[i].queue->getPacketInsertTime(index);
                }
            }
        }

        // Error: No Queue exists with such a priority value
        char errStr[MAX_STRING_LENGTH] = {0};
        sprintf(errStr, "Scheduler Error:"
                " No Queue exists with priority value %d", priority);
        ERROR_Assert(FALSE, errStr);
        return 0; // Unreachable
    }

    // Virtual Destructor for Scheduler Class
    virtual ~Scheduler(){};
};


// FUNCTION   :: SCHEDULER_Setup
// LAYER      ::
// PURPOSE    :: This function runs the generic and then algorithm-specific
//               scheduler initialization routine.
// PARAMETERS ::
// + scheduler : Scheduler** : Pointer of pointer to Scheduler class
// + schedulerTypeString[] : const char : Scheduler Type string
// + enableSchedulerStat : BOOL : Scheduler Statistics is set YES or NO
// + graphDataStr : const char* : Scheduler's graph statistics is set or not
// RETURN     :: void : Null


void SCHEDULER_Setup(
    Scheduler** scheduler,
    const char schedulerTypeString[],
    BOOL enableSchedulerStat = FALSE,
    const char* graphDataStr = "NA");


// FUNCTION   :: GenericPacketClassifier
// LAYER      ::
// PURPOSE    :: Classify a packet for a specific queue
// PARAMETERS ::
// + scheduler : Scheduler* : Pointer to a Scheduler class.
// + pktPriority : int : Incoming packet's priority
// RETURN     :: int : Integer.


int GenericPacketClassifier(Scheduler *scheduler,
                            int pktPriority);


// Comments from source file copied below to generate API Reference




// /**
// FUNCTION   :: Scheduler::SelectSpecificPriorityQueue
// PURPOSE    :: Returns pointer to QueueData associated with the queue.
//               this is a Private
// PARAMETERS ::
// + priority : int : Queue priority
// RETURN :: QueueData* : Pointer of queue
// **/

// /**
// FUNCTION   :: Scheduler::numQueue
// PURPOSE    :: Returns number of queues under this Scheduler
// PARAMETERS :: None
// RETURN :: int : Number of queue.
// **/

// /**
// FUNCTION   :: Scheduler::GetQueuePriority
// PURPOSE    :: Returns Priority for the queues under this Scheduler
// PARAMETERS ::
// + queueIndex : int : Queue index
// RETURN :: int : Return priority of a queue
// **/

// /**
// FUNCTION   :: Scheduler::isEmpty
// PURPOSE    :: Returns a Boolean value of TRUE if the array of stored
//               messages in each queue that the scheduler controls are
//               empty, and FALSE otherwise
// PARAMETERS ::
// + priority : const int : Priority of a queue
// RETURN :: BOOL : TRUE or FALSE
// **/

// /**
// FUNCTION   :: Scheduler::bytesInQueue
// PURPOSE    :: This function prototype returns the total number of bytes
//               stored in the array of either a specific queue, or all
//               queues that the scheduler controls.
// PARAMETERS ::
// + priority : const int : Priority of a queue
// RETURN :: BOOL : TRUE or FALSE
// **/

// /**
// FUNCTION   :: Scheduler::numberInQueue
// PURPOSE    :: This function prototype returns the number of messages
//               stored in the array of either a specific queue, or all
//               queues that the scheduler controls.
// PARAMETERS ::
// + priority : const int : Priority of a queue
// RETURN :: int : Bytes in queue is used.
// **/

// /**
// FUNCTION   :: Scheduler::qosInformationUpdate
// PURPOSE    :: This function enable Qos monitoring for all
//               queues that the scheduler controls.
// PARAMETERS ::
// + queueIndex : int : Queue index
// + qDelayVal : int* : Queue delay
// + totalTransmissionVal : int* : Transmission value
// + currentTime : const clocktype : Current simulation time
// + isResetTotalTransmissionVal : BOOL : Total Transmission is set or not
// RETURN :: void : Null
// **/

// /**
// FUNCTION   :: Scheduler::collectGraphData
// PURPOSE    :: This function enable data collection for performance
//               study of schedulers.
// PARAMETERS ::
// + priority : int : Priority of the queue
// + packetSize : int : Size of packet
// + currentTime : const clocktype : Current simulation time
// RETURN :: void : Null
// **/

// /**
// FUNCTION   :: Scheduler::invokeQueueFinalize
// PURPOSE    :: This function invokes queue finalization.
// PARAMETERS ::
// + node : Node* : Pointer to Node structure
// + layer : const char* : The layer string
// + interfaceIndex : const int : Interface Index
// + instanceId : const int : Instance Ids
// + invokingProtocol : const char* : The protocol string
// + splStatStr : const char* : Special string for stat print
// RETURN :: void : Null
// **/

// /**
// FUNCTION   :: SCHEDULER_Setup
// PURPOSE    :: This function runs the generic and then algorithm-specific
//               scheduler initialization routine.
// PARAMETERS ::
// + scheduler : Scheduler** : Pointer of pointer to Scheduler class
// + schedulerTypeString[] : const char : Scheduler Type string
// + enableSchedulerStat : BOOL : Scheduler Statistics is set YES or NO
// + graphDataStr : const char* : Scheduler's graph statistics is set or not
// RETURN :: void : Null
// **/

// /**
// FUNCTION   :: GenericPacketClassifier
// PURPOSE    :: Classify a packet for a specific queue
// PARAMETERS ::
// + scheduler : Scheduler* : Pointer to a Scheduler class.
// + pktPriority : int : Incoming packet's priority
// RETURN :: int : Integer.
// **/



#endif // IF_SCHEDULER_H

