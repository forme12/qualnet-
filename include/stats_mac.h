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

#ifndef _STATS_MAC_H_
#define _STATS_MAC_H_

#include "stats.h"

class STAT_GlobalMacStatistics;


// /**
// CLASS       :: STAT_MacSessionKey
// DESCRIPTION :: Implements a key for a mac session.  A session
//                is defined as statistics between a (senderId, receiverId,
//                interfaceIndex) tuple
// **/
class STAT_MacSessionKey
{
public:
    int m_senderId;
    int m_receiverId;
    int m_interfaceIndex;

    bool operator <(const STAT_MacSessionKey& rhs) const
    {
        if (m_senderId < rhs.m_senderId)
        {
            return true;
        }
        if (m_senderId > rhs.m_senderId)
        {
            return false;
        }
        if (m_receiverId < rhs.m_receiverId)
        {
            return true;
        }
        if (m_receiverId > rhs.m_receiverId)
        {
            return false;
        }
        if (m_interfaceIndex < rhs.m_interfaceIndex)
        {
            return true;
        }
        if (m_interfaceIndex > rhs.m_interfaceIndex)
        {
            return false;
        }

        return false;
    }
};

class STAT_MacAddressStatistics
{
protected:
    
    STAT_Sum m_DataFramesSent;   
    STAT_Sum m_DataFramesReceived;
    STAT_Sum m_ControlFramesSent;
    STAT_Sum m_ControlFramesReceived;
    STAT_Sum m_DataBytesSent;
    STAT_Sum m_DataBytesReceived;
    STAT_Sum m_ControlBytesSent;
    STAT_Sum m_ControlBytesReceived;
    STAT_Average m_AverageQueuingDelay;
    STAT_Average m_AverageMediumAccessDelay;
    STAT_Average m_AverageMediumDelay;
    STAT_Average m_AverageJitter;

    clocktype m_LastDelay;  //delay in receiving from upper layer

public:
    STAT_Sum& GetDataFramesSent() { return m_DataFramesSent; }
    STAT_Sum& GetDataFramesReceived() { return m_DataFramesReceived; }
    STAT_Sum& GetControlFramesSent() { return m_ControlFramesSent; }
    STAT_Sum& GetControlFramesReceived() { return m_ControlFramesReceived; }
    STAT_Sum& GetDataBytesSent() { return m_DataBytesSent; }
    STAT_Sum& GetDataBytesReceived() { return m_DataBytesReceived; }
    STAT_Sum& GetControlBytesSent() { return m_ControlBytesSent; }
    STAT_Sum& GetControlBytesReceived() { return m_ControlBytesReceived; }
    STAT_Average& GetAverageQueuingDelay() { return m_AverageQueuingDelay; }
    STAT_Average& GetAverageMediumAccessDelay() { return m_AverageMediumAccessDelay; }
    STAT_Average& GetAverageMediumDelay() { return m_AverageMediumDelay; }
    STAT_Average& GetAverageJitter() { return m_AverageJitter; }
    
    clocktype GetLastDelay() { return m_LastDelay; }
    void SetLastDelay(clocktype val) { m_LastDelay = val; }

    friend class STAT_MacSessionStatistics;
};

class STAT_MacSessionStatistics
{
protected:
    // Statistics for unicast, multicast and broadcast
    STAT_MacAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    int m_senderId;
    int m_receiverId;
    int m_interfaceIndex;    

    STAT_Sum m_FramesDroppedSender;
    STAT_Sum m_FramesDroppedReceiver;
    STAT_Sum m_BytesDroppedSender;
    STAT_Sum m_BytesDroppedReceiver;

public:
    STAT_MacSessionStatistics(
        NodeId senderId,
        NodeId receiverId,
        int interfaceIndex);
    STAT_MacSessionStatistics();

    int GetSender() { return m_senderId; }
    int GetReceiver() { return m_receiverId; }
    int GetInterfaceIndex() { return m_interfaceIndex; }    

    STAT_Sum& GetDataFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesSent(); }
    STAT_Sum& GetDataFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesReceived(); }
    STAT_Sum& GetControlFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesSent(); }
    STAT_Sum& GetControlFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesReceived(); }
    STAT_Sum& GetDataBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesSent(); }
    STAT_Sum& GetDataBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesReceived(); }
    STAT_Sum& GetControlBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesSent(); }
    STAT_Sum& GetControlBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesReceived(); }
    STAT_Average& GetAverageQueuingDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageQueuingDelay(); }
    STAT_Average& GetAverageMediumAccessDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumAccessDelay(); }
    STAT_Average& GetAverageMediumDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumDelay(); }
    STAT_Average& GetAverageJitter(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageJitter(); }

    STAT_Sum& GetFramesDroppedSender() { return m_FramesDroppedSender; }
    STAT_Sum& GetFramesDroppedReceiver() { return m_FramesDroppedReceiver; }
    STAT_Sum& GetBytesDroppedSender() { return m_BytesDroppedSender; }
    STAT_Sum& GetBytesDroppedReceiver() { return m_BytesDroppedReceiver; }

    clocktype GetLastDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetLastDelay(); }
    void SetLastDelay(clocktype val, STAT_DestAddressType type) { m_AddrStats[type].SetLastDelay(val); }

    friend class STAT_MacStatistics;
};



class STAT_MacStatistics : public STAT_ModelStatistics
{
private:
    // A map containing all sessions for this mac
    std::map<STAT_MacSessionKey, STAT_MacSessionStatistics*> m_Sessions;

protected:
    // Statistics for unicast, multicast and broadcast
    STAT_MacAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    STAT_Throughput m_CarriedLoad;

    void AddToGlobal(STAT_GlobalMacStatistics* stat);
public:
    typedef std::map<STAT_MacSessionKey, STAT_MacSessionStatistics*>::iterator MacSessionIterator;

    STAT_MacStatistics(Node* node);

    // Initialize basic names and descriptions
    void SetStatNames();

    // Get a list of statistics the mac protocol supports
    virtual void GetList(std::vector<STAT_Statistic*>& stats);

    STAT_Sum& GetDataFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesSent(); }
    STAT_Sum& GetDataFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesReceived(); }
    STAT_Sum& GetControlFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesSent(); }
    STAT_Sum& GetControlFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesReceived(); }
    STAT_Sum& GetDataBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesSent(); }
    STAT_Sum& GetDataBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesReceived(); }
    STAT_Sum& GetControlBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesSent(); }
    STAT_Sum& GetControlBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesReceived(); }
    STAT_Average& GetAverageQueuingDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageQueuingDelay(); }
    STAT_Average& GetAverageMediumAccessDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumAccessDelay(); }
    STAT_Average& GetAverageMediumDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumDelay(); }
    STAT_Average& GetAverageJitter(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageJitter(); }

    STAT_Throughput& GetCarriedLoad() { return  m_CarriedLoad; }

    clocktype GetLastDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetLastDelay(); }    
    void SetLastDelay(clocktype val, STAT_DestAddressType type) { m_AddrStats[type].SetLastDelay(val); }
    
    // Mac protocols should call this function when a frame leaves the mac output queue
    void AddLeftOutputQueueDataPoints(Node* node,
        Message* msg,
        STAT_DestAddressType type,
        int senderId,
        int receiverId,
        int interfaceIndex);

    // Mac procols should call this function when a frame is sent to the phy layer
    void AddFrameSentDataPoints(Node* node,
        Message* msg,
        STAT_DestAddressType type,
        UInt64 controlSize,
        UInt64 dataSize,
        int interfaceIndex,
        int destID);
    
    // Mac protocols should call this function when a frme is received from the phy layer
    void AddFrameReceivedDataPoints(Node* node,
        Message* msg,
        STAT_DestAddressType type,
        UInt64 controlSize,
        UInt64 dataSize,
        int interfaceIndex);

    // Mac protocols should call this function when a frame is dropped on the sending end
    void AddFrameDroppedSenderDataPoints(Node* node, int receiverId,
        int interfaceIndex, UInt64 bytes);

    // Mac protocols should call this function when a frame is dropped on the receiving end
    void AddFrameDroppedReceiverDataPoints(Node* node, Message* msg,
        int interfaceIndex, UInt64 bytes, BOOL isAnyFrame);
        
    STAT_MacSessionStatistics* GetSession(
        Node* node,
        int senderId,
        int receiverId,
        int interfacendex);

    // SessionBegin and SessionEnd can be used to iterate over session statistics
    MacSessionIterator SessionBegin() { return m_Sessions.begin(); }
    MacSessionIterator SessionEnd() { return m_Sessions.end(); }
};

class STAT_GlobalMacAddressStatistics
{
protected:
    STAT_AggregatedSum m_DataFramesSent;
    STAT_AggregatedSum m_DataFramesReceived;
    STAT_AggregatedSum m_ControlFramesSent;
    STAT_AggregatedSum m_ControlFramesReceived;
    STAT_AggregatedSum m_DataBytesSent;
    STAT_AggregatedSum m_DataBytesReceived;
    STAT_AggregatedSum m_ControlBytesSent;
    STAT_AggregatedSum m_ControlBytesReceived;
    STAT_AggregatedWeightedDataPointAverage m_AverageQueuingDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageMediumAccessDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageMediumDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitter;

public:
    STAT_AggregatedSum& GetDataFramesSent() { return m_DataFramesSent; }
    STAT_AggregatedSum& GetDataFramesReceived() { return m_DataFramesReceived; }
    STAT_AggregatedSum& GetControlFramesSent() { return m_ControlFramesSent; }
    STAT_AggregatedSum& GetControlFramesReceived() { return m_ControlFramesReceived; }
    STAT_AggregatedSum& GetDataBytesSent() { return m_DataBytesSent; }
    STAT_AggregatedSum& GetDataBytesReceived() { return m_DataBytesReceived; }
    STAT_AggregatedSum& GetControlBytesSent() { return m_ControlBytesSent; }
    STAT_AggregatedSum& GetControlBytesReceived() { return m_ControlBytesReceived; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageQueuingDelay() { return m_AverageQueuingDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageMediumAccessDelay() { return m_AverageMediumAccessDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageMediumDelay() { return m_AverageMediumDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitter() { return m_AverageJitter; }
};

// /**
// CLASS        :: STAT_GlobalMacStatistics
// DESCRIPTION  :: Implements global mac aggregated statistics.
//                 Values are handled automatically when mac models use
//                 the STAT_MacStatistics class.
//                 
// **/
class STAT_GlobalMacStatistics
{
protected:
    // Statistics for unicast, multicast and broadcast
    STAT_GlobalMacAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];
    STAT_AggregatedThroughput m_CarriedLoad;

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetDataFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesSent(); }
    STAT_AggregatedSum& GetDataFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataFramesReceived(); }
    STAT_AggregatedSum& GetControlFramesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesSent(); }
    STAT_AggregatedSum& GetControlFramesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlFramesReceived(); }
    STAT_AggregatedSum& GetDataBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesSent(); }
    STAT_AggregatedSum& GetDataBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetDataBytesReceived(); }
    STAT_AggregatedSum& GetControlBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesSent(); }
    STAT_AggregatedSum& GetControlBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].GetControlBytesReceived(); }
    STAT_AggregatedWeightedDataPointAverage& GetAverageQueuingDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageQueuingDelay(); }
    STAT_AggregatedWeightedDataPointAverage& GetAverageMediumAccessDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumAccessDelay(); }
    STAT_AggregatedWeightedDataPointAverage& GetAverageMediumDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageMediumDelay(); }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitter(STAT_DestAddressType type) { return m_AddrStats[type].GetAverageJitter(); }
    
    STAT_AggregatedThroughput& GetCarriedLoad() { return m_CarriedLoad; }    
};

class STAT_MacAddressSummaryStatistics
{
public:
    double totalDataFramesSent;
    double totalDataFramesReceived;
    double totalControlFramesSent;
    double totalControlFramesReceived;
    double totalDataBytesSent;
    double totalDataBytesReceived;
    double totalControlBytesSent;
    double totalControlBytesReceived;

    double averageQueuingDelay;
    double numQueuingDelayDataPoints;

    double averageMediumAccessDelay;
    double numMediumAccessDelayDataPoints;

    double averageMediumDelay;
    double numMediumDelayDataPoints;

    double averageJitter;
    double numJitterDataPoints;
};
// /**
// CLASS        :: STAT_MacSummaryStatistics
// DESCRIPTION  :: Implements global MAC summary statistics.
//                 
// **/
class STAT_MacSummaryStatistics
{
public:
    int senderId;
    int receiverId;
    int interfaceIndex;

    STAT_MacAddressSummaryStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    double totalFramesDroppedSender;
    double totalFramesDroppedReceiver;
    double totalBytesDroppedSender;
    double totalBytesDroppedReceiver;

    // Call to initialize this struct based on the statistics from the model
    void InitializeFromModel(Node* node, STAT_MacSessionStatistics* stats);

    STAT_MacSummaryStatistics& operator +=(const STAT_MacSummaryStatistics& stats);
};
#endif /* _STATS_MAC_H_ */
