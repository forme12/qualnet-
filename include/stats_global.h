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
// /**
// PACKAGE     :: STATS
// DESCRIPTION :: This file describes the implementation of the STATS API.
// **/

#ifndef _STATS_GLOBAL_H_
#define _STATS_GLOBAL_H_

#include "parallel.h"
#include "stats.h"
#include "stats_app.h"
#include "stats_net.h"
#include "stats_phy.h"
#include "stats_transport.h"
#include "stats_mac.h"
#include "stats_queue.h"

#ifdef USE_MPI
#include <mpi.h>
#endif

// The following block implements the DATA_Summarizer class.  The best way
// to think about DATA_Summarizer is that it takes a std::map defined on
// multiple partitions and adds partition's map to the other partition's
// maps.  This combines the maps into a single map with equal keys and
// values on all partitions.
//
// A DATA_Summarizer is defined based on a key data type and a summary data
// type.  The key data type must implement a < operator.  The
// DATA_Summarizer class can be overridden and a few virtual functions
// can be defined:
//     - AddDuplicateKey: Defines how the map should behave when multiple
//       keys are defined.  The default behavior is to use the = operator.
//       See STAT_AppSummarizer to see how a summarizer will add values.
//     - SummarySize: Used in MPI mode to determine the size of a summary
//       value class.  Default behavior is to return sizeof().
//     - SerializeSummary: Used in MPI mode to turn a summary value into a
//       byte stream.  Default behavior is to use memcpy().
//     - DeserializeSummary: Used in MPI mode to turn a byte stream into
//       a summary value.  Default behavior is to use memcpy().
//
// Each key and summary pair is added to the DATA_Summarizer class on each
// remote partition using the [] operator like a normal map.  Then each
// partition calls the Summarize function at the exact same time.  Upon
// returning from the Summarize() function all DATA_Summarizer classes
// on all partitions will be added to each other.  Unique keys are assigned
// using the = operator.  Duplicate keys are assigned using the
// AddDuplicateKey() function.
//
// Example:
// class NodeKey
// {
//     NodeId nodeId;
//     bool operator < (const NodeKey& rhs) { return nodeId < rhs.nodeId }
// }
//
// class PacketsSent
// {
//     int numSent;
// }
//
// // Note: The NodeKey and PacketsSent classes could be replaced with
// // simple integers.
// class PacketSummarizer : public DATA_Summarizer<NodeKey, PacketsSent>
// {
//    virtual void AddDuplicateKey(
//        PacketsSent& summary,
//        const PacketsSent& newSummary)
//    {
//        summary.numSent += newSummary.numSent;
//    }
// };
//
// Partition 0:
// PacketSummarizer summ;
// NodeKey key;
// PacketSent sent;
// key.nodeId = 1;
// sent.numSent = 3;
// summ[key] = sent
// key.nodeId = 3;
// sent.numSent = 5;
// summ[key] = sent
// summ.Summarize(partitionData);
// 
// Partition 1:
// PacketSummarizer summ;
// NodeKey key;
// PacketSent sent;
// key.nodeId = 2;
// sent.numSent = 9;
// summ[key] = sent
// key.nodeId = 3; // notice duplicate key
// sent.numSent = 3;
// summ[key] = sent
// summ.Summarize(partitionData)
//
// After the Summarize() call each partition's summ map looks lke this:
//     summ[1] == 3
//     summ[2] == 9
//     summ[3] == 8 == 5 + 3

// /**
// CLASS        :: DATA_Summarizer
// DESCRIPTION  :: Implements a map summarizer.  See the previous comment
//                 block for a more lengthy explanation.
//                 
// **/
template <class TagType, class SummaryType>
class DATA_Summarizer : public std::map<TagType, SummaryType>
{
public:
    clocktype m_LastSummarizeTime; 
    // A map of all summarized results
    typedef map<TagType, SummaryType> MapType;
    typedef typename MapType::iterator myIterator;
    
    DATA_Summarizer<TagType, SummaryType>() { m_LastSummarizeTime = -1; }

    // Called to add other partition's data to the map
    // User level code should use the [] operator for assignment
    void AddRemoteData(const TagType& tag, const SummaryType& stats);

    // Call on each partition at the same time to exchange data
    // Partition 0 will receive and summarize, other partitions will send
    // After calling this function partition 0 can iterate through as a map
    void Summarize(PartitionData* partition);

    // How the map should behave when multiple keys are defined
    // By default the assignment operator is used
    virtual void AddDuplicateKey(SummaryType& summary, const SummaryType& newSummary);

#ifdef USE_MPI
    // SummarySize returns the size of a summary value.  Only overload for
    // summary types that are dynamically sizeable or use C++ functions.
    // Do not need to overload for any class that can be memcpy'd.
    virtual UInt32 SummarySize(const SummaryType* summary);

    // Serialize a summary type.  The out argument is memory at
    // least the size returned by SummarySize()
    virtual void SerializeSummary(
        const SummaryType* summary,
        UInt8* out);

    // Turn in bytes into a summary type.
    virtual void DeserializeSummary(
        SummaryType* summary,
        const UInt8* in);
#else
#endif
};

// /**
// STRUCT       :: STAT_AppSummaryTag11
// DESCRIPTION  :: Uniquely identifies either the sending and receiving
//                 side of a summary statistic.  Usable where there is 1
//                 sender sending to 1 receiver.
//
// **/
struct STAT_AppSummaryTag11
{
    NodeId sender;
    int instanceId;

    bool operator< (const STAT_AppSummaryTag11 x) const
    {
        if (sender < x.sender)
        {
            return true;
        }
        else if (sender > x.sender)
        {
            return false;
        }
        else if (sender == x.sender)
        {
            if (instanceId < x.instanceId)
            {
                return true;
            }
            else if (instanceId > x.instanceId)
            {
                return false;
            }
            else if (instanceId == x.instanceId)
            {
                return false;
            }
        }
        return false;
    }
};

// /**
// STRUCT       :: STAT_AppSummaryTag1N
// DESCRIPTION  :: Uniquely identifies either the sending or receiving
//                 side of a summary statistic.  Usable where there is 1
//                 sender sending to N receiver(s).
//
// **/
struct STAT_AppSummaryTag1N
{
    NodeId sender;
    //NodeId receiver;
    Address receiver;
    int instanceId;

    bool operator< (const STAT_AppSummaryTag1N x) const
    {
        if (sender < x.sender)
        {
            return true;
        }
        else if (sender > x.sender)
        {
            return false;
        }
        else if (sender == x.sender)
        {
            if (instanceId < x.instanceId)
            {
                return true;
            }
            else if (instanceId > x.instanceId)
            {
                return false;
            }
            else if (instanceId == x.instanceId)
            {
                return false;
            }
        }
        return false;
    }
};

struct STAT_MulticastAppSummaryTag
{
    NodeId m_senderId;  
    NodeId m_destId;
    STAT_SessionIdType m_SessionId;
    NodeAddress m_receiverAddr;

    bool operator <(const STAT_MulticastAppSummaryTag& rhs) const
    {
        if (m_senderId < rhs.m_senderId)
        {
            return true;
        }
        if (m_senderId > rhs.m_senderId)
        {
            return false;
        }
        if (m_destId < rhs.m_destId)
        {
            return true;
        }
        if (m_destId > rhs.m_destId)
        {
            return false;
        }
        if (m_SessionId < rhs.m_SessionId)
        {
            return true;
        }
        if (m_SessionId > rhs.m_SessionId)
        {
            return false;
        }
        if (m_receiverAddr < rhs.m_receiverAddr)
        {
            return true;
        }
        if (m_receiverAddr > rhs.m_receiverAddr)
        {
            return false;
        }

        return false;
    }

};

struct STAT_TransportSummaryTag
{
    NodeAddress sourceAddr;
    NodeAddress destAddr;
    UInt16 sourcePort;
    UInt16 destPort;
    bool operator< (const STAT_TransportSummaryTag& rhs) const
    {
        if (sourceAddr < rhs.sourceAddr)
        {
            return true;
        }
        if (sourceAddr > rhs.sourceAddr)
        {
            return false;
        }
        if (destAddr < rhs.destAddr)
        {
            return true;
        }
        if (destAddr > rhs.destAddr)
        {
            return false;
        }
        if (sourcePort < rhs.sourcePort)
        {
            return true;
        }
        if (sourcePort > rhs.sourcePort)
        {
            return false;
        }
        if (destPort < rhs.destPort)
        {
            return true;
        }
        if (destPort > rhs.destPort)
        {
            return false;
        }
        return false;
    }
};

// STRUCT       :: STAT_NetSummaryTag11
// DESCRIPTION  :: Uniquely identifies either the sending and receiving
//                 side of a summary statistic.  Usable where there is 1
//                 sender sending to 1 receiver.
//
// **/
struct STAT_NetSummaryTag11
{
    NodeAddress sender;
    NodeAddress receiver;

    bool operator< (const STAT_NetSummaryTag11& rhs) const
    {
        if (sender < rhs.sender)
        {
            return true;
        }
        else if (sender > rhs.sender)
        {
            return false;
        }
        else if (sender == rhs.sender)
        {
            if (receiver < rhs.receiver)
            {
                return true;
            }
            else if (receiver > rhs.receiver)
            {
                return false;
            }
            else if (receiver == rhs.receiver)
            {
                return false;
            }
        }
        return false;
    }
};

// STRUCT       :: STAT_PhySummaryTag
// DESCRIPTION  :: Uniquely identifies either the sending and receiving
//                 side of a summary statistic at the phy layer.
//
// **/
struct STAT_PhySummaryTag
{
    NodeId sender;
    NodeId receiver;
    int channelIndex;

    bool operator< (const STAT_PhySummaryTag& rhs) const
    {
        if (sender < rhs.sender)
        {
            return true;
        }
        else if (sender > rhs.sender)
        {
            return false;
        }
        if (receiver < rhs.receiver)
        {
            return true;
        }
        else if (receiver > rhs.receiver)
        {
            return false;
        }
        if (channelIndex < rhs.channelIndex)
        {
            return true;
        }
        else if (channelIndex > rhs.channelIndex)
        {
            return false;
        }
        return false;
    }
};

// STRUCT       :: STAT_MacSummaryTag
// DESCRIPTION  :: Uniquely identifies either the sending and receiving
//                 side of a summary statistic at the mac layer.
//
// **/
struct STAT_MacSummaryTag
{
    NodeId sender;
    NodeId receiver;
    int interfaceIndex;

    bool operator< (const STAT_MacSummaryTag& rhs) const
    {
        if (sender < rhs.sender)
        {
            return true;
        }
        else if (sender > rhs.sender)
        {
            return false;
        }
        if (receiver < rhs.receiver)
        {
            return true;
        }
        else if (receiver > rhs.receiver)
        {
            return false;
        }
        if (receiver < rhs.receiver)
        {
            return true;
        }
        else if (receiver > rhs.receiver)
        {
            return false;
        }
        return false;
    }
};
// STRUCT       :: STAT_QueueSummaryTag
// DESCRIPTION  :: Uniquely identifies either the node's queue stats
//
// **/
struct STAT_QueueSummaryTag
{
    NodeId node;
    std::string queuePosition;
    int queueIndex;
    int interfaceIndex;

    bool operator< (const STAT_QueueSummaryTag& rhs) const
    {
        if (node < rhs.node)
        {
            return true;
        }
        else if (node > rhs.node)
        {
            return false;
        }
        if (queueIndex < rhs.queueIndex)
        {
            return true;
        }
        else if (queueIndex > rhs.queueIndex)
        {
            return false;
        }
        int compare = queuePosition.compare(rhs.queuePosition);
        if (compare < 0)
        {
            return true;
        }
        else if (compare > 0)
        {
            return false;
        }
        if (interfaceIndex < rhs.interfaceIndex)
        {
            return true;
        }
        else if (interfaceIndex > rhs.interfaceIndex)
        {
            return false;
        }
        return false;
    }
};

// Summarizer classes contain base definition for summarizing model statistics

class STAT_AppSummarizer : public DATA_Summarizer<STAT_AppSummaryTag11, STAT_AppSummaryStatistics>
{
public:
    virtual void AddDuplicateKey(
        STAT_AppSummaryStatistics& summary,
        const STAT_AppSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

class STAT_AppMulticastSummarizer : public DATA_Summarizer<STAT_AppSummaryTag1N, STAT_AppSummaryStatistics>
{
public:
    virtual void AddDuplicateKey(
        STAT_AppSummaryStatistics& summary,
        const STAT_AppSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

class STAT_TransportSummarizer : public DATA_Summarizer<STAT_TransportSummaryTag, STAT_TransportSummaryStatistics>
{
public:
    virtual void AddDuplicateKey(
        STAT_TransportSummaryStatistics& summary,
        const STAT_TransportSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

class STAT_NetSummarizer : public DATA_Summarizer<STAT_NetSummaryTag11, STAT_NetSummaryStatistics>
{
public:
    STAT_NetSummarizer()
    { 
        m_LastSummarizeTime = -1; 
    }
    virtual void AddDuplicateKey(
        STAT_NetSummaryStatistics& summary,
        const STAT_NetSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

class STAT_PhySummarizer : public DATA_Summarizer<STAT_PhySummaryTag, STAT_PhySessionStatistics>
{
public:
    STAT_PhySummarizer()
    { 
        m_LastSummarizeTime = -1; 
    }
    virtual void AddDuplicateKey(
        STAT_PhySessionStatistics& summary,
        const STAT_PhySessionStatistics& newSummary)
    {
        summary += newSummary;
    }
};
class STAT_MacSummarizer : public DATA_Summarizer<STAT_MacSummaryTag, STAT_MacSummaryStatistics>
{
public:
    STAT_MacSummarizer()
    { 
        m_LastSummarizeTime = -1; 
    }
    virtual void AddDuplicateKey(
        STAT_MacSummaryStatistics& summary,
        const STAT_MacSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

class STAT_QueueSummarizer : public DATA_Summarizer<STAT_QueueSummaryTag, STAT_QueueSummaryStatistics>
{
public:
    virtual void AddDuplicateKey(
        STAT_QueueSummaryStatistics& summary,
        const STAT_QueueSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};


class STAT_MulticastAppSessionSummarizer : public DATA_Summarizer<STAT_MulticastAppSummaryTag, STAT_MulticastAppSessionSummaryStatistics>
{
public:
    virtual void AddDuplicateKey(
        STAT_MulticastAppSessionSummaryStatistics& summary,
        const STAT_MulticastAppSessionSummaryStatistics& newSummary)
    {
        summary += newSummary;
    }
};

// /**
// CLASS        :: STAT_GlobalStatistics
// DESCRIPTION  :: A simple class that holds all global statistics for all
//                 layers.
//
// **/

// Forward declaration before defining STAT_GlobalStatistics
class STAT_GlobalAppStatisticsBridge;
class STAT_AppSummaryBridge;
class STAT_GlobalNetStatisticsBridge;
class STAT_NetSummaryBridge;
class STAT_GlobalTransportStatisticsBridge;
class STAT_TransportSummaryBridge;
class STAT_GlobalPhysicalStatisticsBridge;
class STAT_PhySummaryBridge;
class STAT_GlobalMacStatisticsBridge;
class STAT_MacSummaryBridge;
class STAT_GlobalQueueStatisticsBridge;
class STAT_QueueSummaryBridge;
class STAT_QueueStatusBridge;
class STAT_MulticastAppSummaryBridge;

class STAT_GlobalStatistics
{
public:
    // Aggregated statistics
    STAT_GlobalAppStatistics appAggregate;
    STAT_GlobalTransportStatistics transportAggregate;
    STAT_GlobalNetStatistics netAggregate;
    STAT_GlobalPhyStatistics phyAggregate;
    STAT_GlobalMacStatistics macAggregate;
    STAT_GlobalQueueStatistics queueAggregate;

    // Summarized statistics
    STAT_AppSummarizer appUnicastSummary;
    STAT_AppMulticastSummarizer appMulticastSummary;
    STAT_MulticastAppSessionSummarizer appMulticastSessionSummary;
    STAT_TransportSummarizer transportSummary;
    STAT_NetSummarizer netSummary;
    STAT_PhySummarizer phySummary;
    STAT_MacSummarizer macSummary;
    STAT_QueueSummarizer queueSummary;

    //inactive queue objects (ones that were swapped out by schedulers)
    std::vector<STAT_QueueStatistics*> inactiveQueues;

#ifdef ADDON_DB
    // Added to hold bridge values
    STAT_GlobalAppStatisticsBridge* appBridge;
    STAT_AppSummaryBridge* appSummaryBridge;
    STAT_GlobalNetStatisticsBridge* netBridge;
    STAT_NetSummaryBridge* netSummaryBridge;
    STAT_GlobalTransportStatisticsBridge* transportBridge;
    STAT_TransportSummaryBridge* transportSummaryBridge;
    STAT_GlobalPhysicalStatisticsBridge* phyBridge;
    STAT_PhySummaryBridge* phySummaryBridge;
    STAT_GlobalQueueStatisticsBridge* queueBridge;
    STAT_QueueSummaryBridge* queueSummaryBridge;
    STAT_QueueStatusBridge* queueStatusBridge;
    STAT_GlobalMacStatisticsBridge* macBridge;
    STAT_MacSummaryBridge* macSummaryBridge;
    STAT_MulticastAppSummaryBridge* multicastAppSummaryBridge;
#endif
    // member functions
    STAT_GlobalStatistics();
};

// /**
// CLASS        :: STAT_StatisticsList
// DESCRIPTION  :: Maintains all aggregated statistics.  All aggregated
//                 statistics should call their own SetInfo function then
//                 RegisterAggregatedStatistic on each partition when they
//                 are created.  The Aggregate function may be called at any
//                 time during the simulation to aggregate statistics.  If
//                 running in parallel then each partition must call
//                 Aggregate simultaneously.
//
// **/
class STAT_StatisticsList
{
private:
    // Whether finalized model statistics should be saved or deleted
    // Defaults to FALSE.  STATS-DELETE-WHEN-FINALIZED YES to set to TRUE.
    BOOL m_DeleteFinalizedModelStatistics;

    int m_NextAggregateId;
    std::vector<STAT_AggregatedStatistic*> m_AggregatedStats;

    // Higher order aggregated stats sorted by order
    std::vector<STAT_AggregatedStatistic*> m_OrderedAggregatedStats;

    typedef std::vector<STAT_AggregatedStatistic*>::iterator aggIterator;

    aggIterator FindAggregatedStatistic(STAT_AggregatedStatistic* stat);

    clocktype m_LastAggregateTime;

    // A sorted vector of all model statistics
    std::vector<STAT_ModelStatistics*> m_ModelStatistics;

public:
    STAT_GlobalStatistics global;

    STAT_StatisticsList(PartitionData* partition);

    BOOL DeleteFinalizedModelStatistics() { return m_DeleteFinalizedModelStatistics; }

    // Add a new model to the list of statistics.  Called automatically
    // by STAT_ModelStatistics
    void AddModelStatistics(STAT_ModelStatistics* stats);

    // Remove an old model from the list of statistics.  Called automatically
    // by STAT_ModelStatistics ONLY if DeleteFinalizedModelStatistics
    // is TRUE
    void RemoveModelStatistics(STAT_ModelStatistics* stats);

    // Register a new aggregated statistic
    void RegisterAggregatedStatistic(STAT_AggregatedStatistic* stat);

    // Perform aggregation.  Must be called by all partitions at the same time.
    void Aggregate(PartitionData* partition);

    // Perform app summarization.  Must be called by all partitions at the same time.
    void SummarizeApp(PartitionData* partition);

    // Perform transport summarization.  Must be called by all partitions at the same time.
    void SummarizeTransport(PartitionData* partition);

    // Perform net summarization.  Must be called by all partitions at the same time.
    void SummarizeNet(PartitionData* partition);

    // Perform phy summarization.  Must be called by all partitions at the same time.
    void SummarizePhy(PartitionData* partition);

    // Perform MAC summarization.  Must be called by all partitions at the same time.
    void SummarizeMac(PartitionData* partition);

    // Perform queue summarization.  Must be called by all partitions at the same time.
    void SummarizeQueue(PartitionData* partition);

    // Perform multicast app summarization for multicast session data.
    // Must be called by all partitions at the same time.
    void SummarizeMulticastApp(PartitionData* partition);

    // Stores only one copy of a unique string in memory.  For commonly used
    // strings.
    static const std::string* CacheString(const std::string& str);

    // Reset statistics.  Only works if dynamic statistics are enabled.
    void Reset(PartitionData* partition);
};

// DATA_Summarizer implementation
// Required to be in .h because of templates

template <class TagType, class SummaryType>
void DATA_Summarizer<TagType, SummaryType>::AddRemoteData(const TagType& tag, const SummaryType& stats)
{
    myIterator it;

    // Check if this tag is already present
    it = this->find(tag);
    if (it == this->end())
    {
        // First stat for this pair
        std::pair<TagType, SummaryType> data(tag, stats);
        this->insert(data);
    }
    else
    {
        // Second stat for this pair
        SummaryType& updateStats = it->second;
        AddDuplicateKey(updateStats, stats);
    }
}

// Return TRUE if running in parallel
// Needed to be defined here since template functions must be defined in the
// header and PartitionData may not be defined yet
BOOL STAT_IsParallel(PartitionData* partition);
int STAT_PartitionId(PartitionData* partition);
int STAT_NumPartitions(PartitionData* partition);

// TODO: Move to kernel level
template <class TagType, class SummaryType>
void DATA_Summarizer<TagType, SummaryType>::Summarize(
    PartitionData* partition)
{
    // Nothing to do if not in parallel mode
    if (!STAT_IsParallel(partition))
    {
        return;
    }
    int partitionId = STAT_PartitionId(partition);

#ifdef PARALLEL
#ifdef USE_MPI
    // Each partition will calculate the total size of its serialized map
    // then serialize the map into the "sendData" variable.
    // The sizes will be shared by calling MPI_Allgather.
    // Then each partition will allocate sufficient space for all
    // partition's serialized maps.
    // The maps will be shared by calling MPI_Allgatherv into the
    // variable "data."
    // Each partition reads from "data," deserializes the other
    // partition's <key,value> pairs and adds them to their own map.

    // Calculate total size of this partition's data set
    int size = 0;
    for (myIterator it = this->begin();
        it != this->end();
        ++it)
    {
        SummaryType& summary = it->second;

        // int is the sze of the summary
        size += sizeof(int) + sizeof(TagType) + SummarySize(&summary);
    }

    // Serialize outgoing data
    UInt8* sendData = (UInt8*) MEM_malloc(sizeof(UInt8) * size);
    UInt8* out = sendData;
    for (myIterator it = this->begin();
        it != this->end();
        ++it)
    {
        const TagType& tag = it->first;
        const SummaryType& summary = it->second;

        // Serialize size
        int thisSize = SummarySize(&summary);
        *((int*) out) = thisSize;
        out += sizeof(int);

        // Serialize tag
        memcpy(out, &tag, sizeof(TagType));
        out += sizeof(TagType);

        // Serialize summay
        SerializeSummary(&summary, out);
        out += thisSize;
    }
    ERROR_Assert(
        out - sendData == size,
        "Serialized size does not match computed size");

    PARALLEL_SynchronizePartitions(partition);

    // Receive/gather each partition's serialized size
    int* sizes = (int*) MEM_malloc(sizeof(int) * STAT_NumPartitions(partition));
    MPI_Allgather(
        &size,
        1,
        MPI_INT,
        sizes,
        1,
        MPI_INT, 
        MPI_COMM_WORLD);

    // Calculate each partition's displacement (the offset in the data array
    // where each partition's data will be placed) and the total size of
    // all data
    int* displ = (int*) MEM_malloc(sizeof(int) * STAT_NumPartitions(partition));
    int totalSize = sizes[0];
    displ[0] = 0;
    for (int i = 1; i < STAT_NumPartitions(partition); i++)
    {
        displ[i] = displ[i - 1] + sizes[i - 1];
        totalSize += sizes[i];
    }

    // Gather all serialized data from remote partitions
    UInt8* data = (UInt8*) MEM_malloc(sizeof(UInt8) * totalSize);
    MPI_Allgatherv(
        sendData,
        size,
        MPI_BYTE, 
        data,
        sizes,
        displ, 
        MPI_BYTE,
        MPI_COMM_WORLD);

    MEM_free(sendData);

    // Now deserialize gathered data
    for (int i = 0; i < STAT_NumPartitions(partition); i++)
    {
        if (partitionId == i)
        {
            continue;
        }

        UInt8* start = data + displ[i];
        UInt8* in = start;
        while (in - start < sizes[i])
        {
            // Deserialize size
            int* thisSize = (int*) in;
            in += sizeof(int);

            // Deserialize tag
            TagType* tag = (TagType*) in;
            in += sizeof(TagType);

            // Deserialize summary
            SummaryType summary;
            DeserializeSummary(&summary, in);
            in += *thisSize;

            // Add data
            AddRemoteData(*tag, summary);
        }
        ERROR_Assert(
            in - start == sizes[i],
            "Deserialized size does not match");
    }

    MEM_free(data);
    MEM_free(sizes);
    MEM_free(displ);
#else
    // gRemoteSummarizers is defined in stats_global.cpp.  At this point
    // each partition will fill in their slot in the array.
    extern void* gRemoteSummarizers[MAX_THREADS];
    DATA_Summarizer<TagType, SummaryType>* remote;
    gRemoteSummarizers[STAT_PartitionId(partition)] = this;

    // Synch after each partition sets its gRemoteSummarizers value
    PARALLEL_SynchronizePartitions(partition);

    // In shared memory partition 0 reads directly from the remote
    // partitions.  This step is performed sequentially.  It's probably just
    // as fast as am ore intelligent parallel update because most keys are
    // unique and they would have to be copied 
    if (STAT_PartitionId(partition) == 0)
    {
        for (int i = 1; i < STAT_NumPartitions(partition); i++)
        {
            remote = (DATA_Summarizer<TagType, SummaryType>*)
                gRemoteSummarizers[i];
            for (myIterator it = remote->begin(); it != remote->end(); ++it)
            {
                this->AddRemoteData(it->first, it->second);
            }
        }
    }

    // Synch when partition 0 has read from all remote partitions
    PARALLEL_SynchronizePartitions(partition);

    // Next each partition copies partition 0's class to itself
    // This step is performed in parallel
    if (STAT_PartitionId(partition) != 0)
    {
        remote = (DATA_Summarizer<TagType, SummaryType>*)
            gRemoteSummarizers[0];
        this->clear();
        this->insert(remote->begin(), remote->end());
    }

    // Synch when partitions have finished copying from partition 0
    PARALLEL_SynchronizePartitions(partition);
#endif
#endif
}

template <class TagType, class SummaryType>
void DATA_Summarizer<TagType, SummaryType>::AddDuplicateKey(
    SummaryType& summary,
    const SummaryType& newSummary)
{
    
    summary = newSummary;
}

#ifdef USE_MPI
template <class TagType, class SummaryType>
UInt32 DATA_Summarizer<TagType, SummaryType>::SummarySize(const SummaryType* summary)
{
    return sizeof(SummaryType);
}

template <class TagType, class SummaryType>
void DATA_Summarizer<TagType, SummaryType>::SerializeSummary(
    const SummaryType* summary,
    UInt8* out)
{
    memcpy(out, summary, SummarySize(summary));
}

template <class TagType, class SummaryType>
void DATA_Summarizer<TagType, SummaryType>::DeserializeSummary(
    SummaryType* summary,
    const UInt8* in)
{
    memcpy(summary, in, SummarySize(summary));
}
#endif // USE_MPI

#endif /* _STATS_GLOBAL_H_ */
