// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6701 Center Drive West
//                          Suite 520
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

#ifndef _STATS_QUEUE_H_
#define _STATS_QUEUE_H_

#include "stats.h"

class STAT_GlobalQueueStatistics; //forward declaration

class STAT_QueueStatistics : public STAT_ModelStatistics
{
private:    
    Node* m_Node;
    std::string m_QueuePosition;
    int m_InterfaceIndex;
    int m_QueueSize;
    std::string m_QueueType;
    int m_QueueIndex;

    STAT_Sum m_PacketsEnqueued;
    STAT_Sum m_PacketsDequeued;
    STAT_Sum m_PacketsDropped;
    STAT_Sum m_PacketsDroppedForcefully;
    STAT_Sum m_PacketsDroppedForcefullyFromAging;
    STAT_Sum m_BytesEnqueued;
    STAT_Sum m_BytesDequeued;
    STAT_Sum m_BytesDropped;
    STAT_Sum m_BytesDroppedForcefully;
    
    STAT_TimeWeightedAverage m_AverageQueueLength;
    STAT_Maximum m_PeakQueueLength;
    STAT_Maximum m_LongestTimeInQueue;

    STAT_Sum m_totalDelays;
    clocktype m_lastChange;
    clocktype m_creationTime;

    void AddToGlobal(STAT_GlobalQueueStatistics* stat);

public:
    STAT_QueueStatistics(Node* node,
        std::string queuePosition,
        int interfaceIndex,
        std::string queueType,
        int queueSize,
        int queueIndex,
        clocktype creationTime);

    // Initialize basic names and descriptions
    void SetStatNames();

    // Get a list of statistics the queue protocol supports
    virtual void GetList(std::vector<STAT_Statistic*>& stats);
    
    void EnqueuePacket(Message* msg,
        int totalBytesInQueueBeforeOperation,
        const clocktype currentTime);

    void DequeuePacket(Message* msg,
        int totalBytesInQueueBeforeOperation,
        const clocktype currentTime);

    void DropPacket(Message* msg,
        int totalBytesInQueueBeforeOperation,
        const clocktype currentTime);

    void DropPacketForcefully(Message* msg,
        int totalBytesInQueueBeforeOperation,
        const clocktype currentTime,
        bool fromAging = false);

    void DelayDetected(clocktype delay);

    void FinalizeQueue(const clocktype currentTime,
        int currentBytesInQueue);

    STAT_Sum& GetPacketsEnqueued() { return m_PacketsEnqueued; }
    STAT_Sum& GetPacketsDequeued() { return m_PacketsDequeued; }
    STAT_Sum& GetPacketsDropped() { return m_PacketsDropped; }
    STAT_Sum& GetPacketsDroppedForcefully() { return m_PacketsDroppedForcefully; }
    STAT_Sum& GetPacketsDroppedForcefullyFromAging() { return m_PacketsDroppedForcefullyFromAging; }
    STAT_Sum& GetBytesEnqueued() { return m_BytesEnqueued; }
    STAT_Sum& GetBytesDequeued() { return m_BytesDequeued; }
    STAT_Sum& GetBytesDropped() { return m_BytesDropped; }
    STAT_Sum& GetBytesDroppedForcefully() { return m_BytesDroppedForcefully; }

    STAT_TimeWeightedAverage& GetAverageQueueLength() { return m_AverageQueueLength; }
    STAT_Maximum& GetPeakQueueLength() { return m_PeakQueueLength; }
    STAT_Maximum& GetLongestTimeInQueue() { return m_LongestTimeInQueue; }

    Node* GetNode() { return m_Node; }
    std::string GetQueuePosition() { return m_QueuePosition; }    
    int GetInterfaceIndex() { return m_InterfaceIndex; }
    void SetInterfaceIndex(int index) { m_InterfaceIndex = index; }
    int GetQueueSize() { return m_QueueSize; }
    std::string GetQueueType() { return m_QueueType; }
    int GetQueueIndex() { return m_QueueIndex; }

    STAT_Sum& GetTotalDelays() { return m_totalDelays; }
};

// CLASS        :: STAT_GlobalQueueStatistics
// DESCRIPTION  :: Implements global queue aggregated statistics.
//                 Values are handled automatically when queues  use
//                 the STAT_QueueStatistics class.
//    
class STAT_GlobalQueueStatistics
{
protected:
    STAT_AggregatedSum m_PacketsEnqueued;
    STAT_AggregatedSum m_PacketsDequeued;
    STAT_AggregatedSum m_PacketsDropped;
    STAT_AggregatedSum m_PacketsDroppedForcefully;
    STAT_AggregatedSum m_BytesEnqueued;
    STAT_AggregatedSum m_BytesDequeued;
    STAT_AggregatedSum m_BytesDropped;
    STAT_AggregatedSum m_BytesDroppedForcefully;

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetPacketsEnqueued() { return m_PacketsEnqueued; }
    STAT_AggregatedSum& GetPacketsDequeued() { return m_PacketsDequeued; }
    STAT_AggregatedSum& GetPacketsDropped() { return m_PacketsDropped; }
    STAT_AggregatedSum& GetPacketsDroppedForcefully() { return m_PacketsDroppedForcefully; }
    STAT_AggregatedSum& GetBytesEnqueued() { return m_BytesEnqueued; }
    STAT_AggregatedSum& GetBytesDequeued() { return m_BytesDequeued; }
    STAT_AggregatedSum& GetBytesDropped() { return m_BytesDropped; }
    STAT_AggregatedSum& GetBytesDroppedForcefully() { return m_BytesDroppedForcefully; }
};

// CLASS        :: STAT_QueueSummaryStatistics
// DESCRIPTION  :: Implements global Queue summary statistics.
//         
class STAT_QueueSummaryStatistics
{
public:
    NodeId nodeId;
    int queueIndex;
    std::string queuePosition;
    std::string queueType;
    int interfaceIndex;
    int queueSize;

    UInt64 totalPacketsEnqueued;
    UInt64 totalPacketsDequeued;
    UInt64 totalPacketsDropped;
    UInt64 totalPacketsDroppedForcefully;
    UInt64 totalPacketsDroppedForcefullyFromAging;
    UInt64 totalBytesEnqueued;
    UInt64 totalBytesDequeued;
    UInt64 totalBytesDropped;
    UInt64 totalBytesDroppedForcefully;
    
    double totalDelays;

    UInt64 peakQueueLength;
    double longestTimeInQueue;
    
    double averageQueueLength;

    int curFreeSpace;

    // Call to initialize this struct based on the statistics from the model
    void InitializeFromModel(Node* node,
        STAT_QueueStatistics* stats,
        int currentFreeSpace);

    STAT_QueueSummaryStatistics& operator +=(const STAT_QueueSummaryStatistics& stats);
};
#endif /* _STATS_QUEUE_H_ */
