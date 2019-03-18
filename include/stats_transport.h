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

#ifndef _STATS_TRANSPORT_H_
#define _STATS_TRANSPORT_H_

#include "stats.h"

class STAT_GlobalTransportStatistics;

// /**
// CLASS       :: STAT_TransportSummarySessionKey
// DESCRIPTION :: Implements a key for a transport session.  A session
//                is defined as statistics between (source addr, source
//                port) and (dest addr, dest port) pairs.
// **/
class STAT_TransportSummarySessionKey
{
public:
    Address m_sourceAddr;
    Address m_destAddr;
    int m_sourcePort;
    int m_destPort;

    bool operator <(const STAT_TransportSummarySessionKey rhs) const
    {
        if (m_sourceAddr < rhs.m_sourceAddr)
        {
            return true;
        }
        if (m_sourceAddr > rhs.m_sourceAddr)
        {
            return false;
        }
        if (m_destAddr < rhs.m_destAddr)
        {
            return true;
        }
        if (m_destAddr > rhs.m_destAddr)
        {
            return false;
        }
        if (m_sourcePort < rhs.m_sourcePort)
        {
            return true;
        }
        if (m_sourcePort > rhs.m_sourcePort)
        {
            return false;
        }
        if (m_destPort < rhs.m_destPort)
        {
            return true;
        }
        if (m_destPort > rhs.m_destPort)
        {
            return false;
        }

        return false;
    }
};

// /**
// CLASS       :: STAT_TransportAddressStatistics
// DESCRIPTION :: Implements statistics for one address type.  This is used
//                as the basis for unicast, multicast and broadcast statistics.
// **/
class STAT_TransportAddressStatistics
{
protected:
    STAT_Sum m_DataSegmentsSent;
    STAT_Sum m_DataSegmentsReceived;
    STAT_Sum m_DataBytesSent;
    STAT_Sum m_DataBytesReceived;
    STAT_Sum m_OverheadBytesSent;
    STAT_Sum m_OverheadBytesReceived;
    STAT_Sum m_ControlSegmentsSent;
    STAT_Sum m_ControlSegmentsReceived;
    STAT_Sum m_ControlBytesSent;
    STAT_Sum m_ControlBytesReceived;

    STAT_Throughput m_OfferedLoad;
    STAT_Throughput m_Throughput;
    STAT_Throughput m_Goodput;

    STAT_Average m_AverageDelay;
    STAT_Average m_AverageDeliveryDelay;
    STAT_Average m_AverageJitter;
    STAT_Average m_AverageDeliveryJitter;

public:
    clocktype m_LastDelay;
    clocktype m_LastDeliveryDelay;
    friend class STAT_TransportStatistics;

    STAT_Sum& GetDataSegmentsSent() { return m_DataSegmentsSent; }
    STAT_Sum& GetDataSegmentsReceived() { return m_DataSegmentsReceived; }
    STAT_Sum& GetDataBytesSent() { return m_DataBytesSent; }
    STAT_Sum& GetDataBytesReceived() { return m_DataBytesReceived; }
    STAT_Sum& GetOverheadBytesSent() { return m_OverheadBytesSent; }
    STAT_Sum& GetOverheadBytesReceived() { return m_OverheadBytesReceived; }
    STAT_Sum& GetControlSegmentsSent() { return m_ControlSegmentsSent; }
    STAT_Sum& GetControlSegmentsReceived() { return m_ControlSegmentsReceived; }
    STAT_Sum& GetControlBytesSent() { return m_ControlBytesSent; }
    STAT_Sum& GetControlBytesReceived() { return m_ControlBytesReceived; }

    STAT_Throughput& GetOfferedLoad() { return m_OfferedLoad; }
    STAT_Throughput& GetThroughput() { return m_Throughput; }
    STAT_Throughput& GetGoodput() { return m_Goodput; }

    STAT_Average& GetAverageDelay() { return m_AverageDelay; }
    STAT_Average& GetAverageDeliveryDelay() { return m_AverageDeliveryDelay; }
    STAT_Average& GetAverageJitter() { return m_AverageJitter; }
    STAT_Average& GetAverageDeliveryJitter() { return m_AverageDeliveryJitter; }

    void SetLastDelay(clocktype val) { m_LastDelay = val; }
    clocktype GetLastDelay() { return m_LastDelay; }
    void SetLastDeliveryDelay(clocktype val) { m_LastDeliveryDelay = val; }
    clocktype GetLastDeliveryDelay() { return m_LastDeliveryDelay; }
    STAT_TransportAddressStatistics();
};

// /**
// CLASS       :: STAT_TransportSessionStatistics
// DESCRIPTION :: Implements statistics for a transport session.  A session
//                is defined as statistics between (source addr, source
//                port) and (dest addr, dest port) pairs.
// **/
class STAT_TransportSessionStatistics
{
protected:
    // Statistics for unicast, multicast and broadcast
    STAT_TransportAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    // m_SourceAddr, m_destAddr, m_sourcePort and m_destPort are used to
    // uniquely identify a transport session.  Keys for TCP and UDP must be
    // kept separate.
    Address m_sourceAddr;
    Address m_destAddr;
    int m_sourcePort;
    int m_destPort;

public:
    // Constructor
    STAT_TransportSessionStatistics(
        Node* node,
        Address sourceAddr,
        Address destAddr,
        int sourcePort,
        int destPort);

    Address GetSourceAddr() { return m_sourceAddr; }
    Address GetDestAddr() { return m_destAddr; }
    int GetSourcePort() { return m_sourcePort; }
    int GetDestPort() { return m_destPort; }
    
    STAT_Sum& GetDataSegmentsSent(STAT_DestAddressType type) {
        return m_AddrStats[type].GetDataSegmentsSent(); }
    STAT_Sum& GetDataSegmentsReceived(STAT_DestAddressType type) {
        return m_AddrStats[type].GetDataSegmentsReceived(); }
    STAT_Sum& GetDataBytesSent(STAT_DestAddressType type) {
        return m_AddrStats[type].GetDataBytesSent(); }
    STAT_Sum& GetDataBytesReceived(STAT_DestAddressType type) {
        return m_AddrStats[type].GetDataBytesReceived(); }
    STAT_Sum& GetOverheadBytesSent(STAT_DestAddressType type) {
        return m_AddrStats[type].GetOverheadBytesSent(); }
    STAT_Sum& GetOverheadBytesReceived(STAT_DestAddressType type) {
        return m_AddrStats[type].GetOverheadBytesReceived(); }
    STAT_Sum& GetControlSegmentsSent(STAT_DestAddressType type) {
        return m_AddrStats[type].GetControlSegmentsSent(); }
    STAT_Sum& GetControlSegmentsReceived(STAT_DestAddressType type) {
        return m_AddrStats[type].GetControlSegmentsReceived(); }
    STAT_Sum& GetControlBytesSent(STAT_DestAddressType type) {
        return m_AddrStats[type].GetControlBytesSent(); }
    STAT_Sum& GetControlBytesReceived(STAT_DestAddressType type) {
        return m_AddrStats[type].GetControlBytesReceived(); }

    STAT_Throughput& GetOfferedLoad(STAT_DestAddressType type) {
        return m_AddrStats[type].GetOfferedLoad(); }
    STAT_Throughput& GetThroughput(STAT_DestAddressType type) {
        return m_AddrStats[type].GetThroughput(); }
    STAT_Throughput& GetGoodput(STAT_DestAddressType type) {
        return m_AddrStats[type].GetGoodput(); }

    STAT_Average& GetAverageDelay(STAT_DestAddressType type) {
        return m_AddrStats[type].GetAverageDelay(); }
    STAT_Average& GetAverageDeliveryDelay(STAT_DestAddressType type) {
        return m_AddrStats[type].GetAverageDeliveryDelay(); }
    STAT_Average& GetAverageJitter(STAT_DestAddressType type) {
        return m_AddrStats[type].GetAverageJitter(); }
    STAT_Average& GetAverageDeliveryJitter(STAT_DestAddressType type) {
        return m_AddrStats[type].GetAverageDeliveryJitter(); }

    void SetLastDelay(STAT_DestAddressType type, clocktype val) { m_AddrStats[type].SetLastDelay(val); }
    clocktype GetLastDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetLastDelay(); }
    void SetLastDeliveryDelay(STAT_DestAddressType type,clocktype val) { m_AddrStats[type].SetLastDeliveryDelay(val); }
    clocktype GetLastDeliveryDelay(STAT_DestAddressType type) { return m_AddrStats[type].GetLastDeliveryDelay(); }

    friend class STAT_TransportStatistics;
};

// /**
// CLASS       :: STAT_TransportStatistics
// DESCRIPTION :: Implements statistics for UDP or TCP.
// **/
class STAT_TransportStatistics : public STAT_ModelStatistics
{
protected:
    // Statistics for unicast, multicast and broadcast
    STAT_TransportAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];
    
    // UDP or TCP
    std::string m_Protocol;

    // Contains all active sessions
    std::map<STAT_TransportSummarySessionKey, STAT_TransportSessionStatistics*> m_Session;

    // Return the session for the (sourceAddr, sourcePort) and (destAddr, destPort) pairs.
    // If session statistics do not exist for this session then new ones are created.
    STAT_TransportSessionStatistics* GetSession(
        Node* node,
        Address sourceAddr,
        Address destAddr,
        int sourcePort,
        int destPort);

    void AddToGlobal(STAT_GlobalTransportStatistics* stat);
public:
    // Data type used for iterating session statistics
    typedef std::map<STAT_TransportSummarySessionKey, STAT_TransportSessionStatistics*>::iterator SessionIter;

    // Constructor
    STAT_TransportStatistics(Node* node, const std::string& protocol);

    // Initialize basic names and descriptions
    void SetStatNames();

    // Get a list of statistics the transport protocol supports
    virtual void GetList(std::vector<STAT_Statistic*>& stats);

    STAT_Sum& GetDataSegmentsSent(STAT_DestAddressType type) { return m_AddrStats[type].m_DataSegmentsSent; }
    STAT_Sum& GetDataSegmentsReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_DataSegmentsReceived; }
    STAT_Sum& GetDataBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_DataBytesSent; }
    STAT_Sum& GetDataBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_DataBytesReceived; }
    STAT_Sum& GetOverheadBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_OverheadBytesSent; }
    STAT_Sum& GetOverheadBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_OverheadBytesReceived; }
    STAT_Sum& GetControlSegmentsSent(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlSegmentsSent; }
    STAT_Sum& GetControlSegmentsReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlSegmentsReceived; }
    STAT_Sum& GetControlBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlBytesSent; }
    STAT_Sum& GetControlBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlBytesReceived; }

    STAT_Throughput& GetOfferedLoad(STAT_DestAddressType type) { return m_AddrStats[type].m_OfferedLoad; }
    STAT_Throughput& GetThroughput(STAT_DestAddressType type) { return m_AddrStats[type].m_Throughput; }
    STAT_Throughput& GetGoodput(STAT_DestAddressType type) { return m_AddrStats[type].m_Goodput; }
    STAT_Average& GetAverageDelay(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDelay; }
    STAT_Average& GetAverageDeliveryDelay(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDeliveryDelay; }
    STAT_Average& GetAverageJitter(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageJitter; }
    STAT_Average& GetAverageDeliveryJitter(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDeliveryJitter; }

    // SessionBegin and SessionEnd can be used to iterate over sesion statistics
    SessionIter SessionBegin() { return m_Session.begin(); }
    SessionIter SessionEnd() { return m_Session.end(); }

    // Transport protocols should call this function when a packet is received from the upper layer
    void AddReceiveFromUpperLayerDataPoints(Node* node, Message* msg);

    // Transport procols should call this function when a segment is sent to
    // the network layer.
    void AddSegmentSentDataPoints(
        Node* node,
        Message* msg,
        UInt64 controlSize,
        UInt64 dataSize,
        UInt64 overheadSize,
        const Address& sourceAddr,
        const Address& destAddr,
        int sourcePort,
        int destPort);
    
    // Transport protocols should call this function when a segment is received
    // from the network layer
    void AddSegmentReceivedDataPoints(
        Node* node,
        Message* msg,
        STAT_DestAddressType type,
        UInt64 controlSize,
        UInt64 dataSize,
        UInt64 overheadSize,
        const Address& sourceAddr,
        const Address& destAddr,
        int sourcePort,
        int destPort);

    // Transport protocols should call this function when sending a packet to the upper layer
    void AddSentToUpperLayerDataPoints(
        Node* node,
        Message* msg, 
        const Address& sourceAddr,
        const Address& destAddr,
        int sourcePort,
        int destPort);
};

// /**
// CLASS       :: STAT_GlobalTransportAddressStatistics
// DESCRIPTION :: Implements statistics for one address type.  This is used
//                as the basis for unicast, multicast and broadcast statistics
//                at the global (aggregate) level.
// **/
class STAT_GlobalTransportAddressStatistics
{
protected:
    STAT_AggregatedSum m_DataSegmentsSent;
    STAT_AggregatedSum m_DataSegmentsReceived;
    STAT_AggregatedSum m_DataBytesSent;
    STAT_AggregatedSum m_DataBytesReceived;
    STAT_AggregatedSum m_OverheadBytesSent;
    STAT_AggregatedSum m_OverheadBytesReceived;
    STAT_AggregatedSum m_ControlSegmentsSent;
    STAT_AggregatedSum m_ControlSegmentsReceived;
    STAT_AggregatedSum m_ControlBytesSent;
    STAT_AggregatedSum m_ControlBytesReceived;

    STAT_AggregatedThroughput m_OfferedLoad;
    STAT_AggregatedThroughput m_Throughput;
    STAT_AggregatedThroughput m_Goodput;
    STAT_AggregatedWeightedDataPointAverage m_AverageDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageDeliveryDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitter;
    STAT_AggregatedWeightedDataPointAverage m_AverageDeliveryJitter;

    friend class STAT_GlobalTransportStatistics;
};

// /**
// CLASS        :: STAT_GlobalTransportStatistics
// DESCRIPTION  :: Implements global transport aggregated statistics.
//                 Values are handled automatically when transport models use
//                 the STAT_TransportStatistics class.
//                 
// **/
class STAT_GlobalTransportStatistics
{
protected:
    STAT_GlobalTransportAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetDataSegmentsSent(STAT_DestAddressType type) { return m_AddrStats[type].m_DataSegmentsSent; }
    STAT_AggregatedSum& GetDataSegmentsReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_DataSegmentsReceived; }
    STAT_AggregatedSum& GetDataBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_DataBytesSent; }
    STAT_AggregatedSum& GetDataBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_DataBytesReceived; }
    STAT_AggregatedSum& GetOverheadBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_OverheadBytesSent; }
    STAT_AggregatedSum& GetOverheadBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_OverheadBytesReceived; }
    STAT_AggregatedSum& GetControlSegmentsSent(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlSegmentsSent; }
    STAT_AggregatedSum& GetControlSegmentsReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlSegmentsReceived; }
    STAT_AggregatedSum& GetControlBytesSent(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlBytesSent; }
    STAT_AggregatedSum& GetControlBytesReceived(STAT_DestAddressType type) { return m_AddrStats[type].m_ControlBytesReceived; }

    STAT_AggregatedThroughput& GetOfferedLoad(STAT_DestAddressType type) { return m_AddrStats[type].m_OfferedLoad; }
    STAT_AggregatedThroughput& GetThroughput(STAT_DestAddressType type) { return m_AddrStats[type].m_Throughput; }
    STAT_AggregatedThroughput& GetGoodput(STAT_DestAddressType type) { return m_AddrStats[type].m_Goodput; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDelay(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDeliveryDelay(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDeliveryDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitter(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageJitter; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDeliveryJitter(STAT_DestAddressType type) { return m_AddrStats[type].m_AverageDeliveryJitter; }
};

class STAT_TransportAddressSummaryStatistics
{
public:
    double totalDataSegmentsSent;
    double totalDataSegmentsReceived;
    double totalDataBytesSent;
    double totalDataBytesReceived;
    double totalOverheadBytesSent;
    double totalOverheadBytesReceived;
    double totalControlSegmentsSent;
    double totalControlSegmentsReceived;
    double totalControlBytesSent;
    double totalControlBytesReceived;

    double offeredLoad;
    double throughput;
    double goodput;

    double averageDelay;
    double numDelayDataPoints;
    double averageJitter;
    double numJitterDataPoints;
};

// /**
// CLASS       :: STAT_TransportSummaryStatistics
// DESCRIPTION :: Implements statistics for one pair of (sourceAddr, sourcePort)
//                (destAddr, destPort) transport layer sessions.
// **/
class STAT_TransportSummaryStatistics
{
public:
    Address sourceAddr;
    Address destAddr;
    int sourcePort;
    int destPort;

    STAT_TransportAddressSummaryStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    // Call to initialize this struct based on the statistics from the model
    // Can be called for sender or receiver.
    void InitializeFromModel(Node* node, STAT_TransportSessionStatistics* stats);

    // Call to update this struct based on statistics from another model's
    // summary statistics.  Can be called for a sender or receiver's stats.
    STAT_TransportSummaryStatistics& operator +=(const STAT_TransportSummaryStatistics& stats);
};

#endif /* _STATS_TRANSPORT_H_ */

