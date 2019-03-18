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

#ifndef _STATS_NET_H_
#define _STATS_NET_H_

#include "stats.h"

// Forward declarations

class STAT_GlobalNetStatistics;
struct OneHopNetworkData;

class STAT_NetStatistics : public STAT_ModelStatistics
{
protected:
    // Array of delays per interface
    clocktype* m_LastDelay;
    clocktype* m_LastUnicastDelay;
    clocktype* m_LastMulticastDelay;
    clocktype* m_LastBroadcastDelay;

    STAT_Sum m_DataPacketsSentUnicast;
    STAT_Sum m_DataPacketsReceivedUnicast;
    STAT_Sum m_DataPacketsForwardedUnicast;
    STAT_Sum m_ControlPacketsSentUnicast;
    STAT_Sum m_ControlPacketsReceivedUnicast;
    STAT_Sum m_ControlPacketsForwardedUnicast;
    STAT_Sum m_DataBytesSentUnicast;
    STAT_Sum m_DataBytesReceivedUnicast;
    STAT_Sum m_DataBytesForwardedUnicast;
    STAT_Sum m_ControlBytesSentUnicast;
    STAT_Sum m_ControlBytesReceivedUnicast;
    STAT_Sum m_ControlBytesForwardedUnicast;

    STAT_Sum m_DataPacketsSentBroadcast;
    STAT_Sum m_DataPacketsReceivedBroadcast;
    STAT_Sum m_DataPacketsForwardedBroadcast;
    STAT_Sum m_ControlPacketsSentBroadcast;
    STAT_Sum m_ControlPacketsReceivedBroadcast;
    STAT_Sum m_ControlPacketsForwardedBroadcast;
    STAT_Sum m_DataBytesSentBroadcast;
    STAT_Sum m_DataBytesReceivedBroadcast;
    STAT_Sum m_DataBytesForwardedBroadcast;
    STAT_Sum m_ControlBytesSentBroadcast;
    STAT_Sum m_ControlBytesReceivedBroadcast;
    STAT_Sum m_ControlBytesForwardedBroadcast;

    STAT_Sum m_DataPacketsSentMulticast;
    STAT_Sum m_DataPacketsReceivedMulticast;
    STAT_Sum m_DataPacketsForwardedMulticast;
    STAT_Sum m_ControlPacketsSentMulticast;
    STAT_Sum m_ControlPacketsReceivedMulticast;
    STAT_Sum m_ControlPacketsForwardedMulticast;
    STAT_Sum m_DataBytesSentMulticast;
    STAT_Sum m_DataBytesReceivedMulticast;
    STAT_Sum m_DataBytesForwardedMulticast;
    STAT_Sum m_ControlBytesSentMulticast;
    STAT_Sum m_ControlBytesReceivedMulticast;
    STAT_Sum m_ControlBytesForwardedMulticast;

    STAT_Throughput m_CarriedLoad;
    STAT_Sum m_PacketsDroppedNoRoute;
    STAT_Sum m_PacketsDroppedTtlExpired;
    STAT_Sum m_PacketsDroppedQueueOverflow;
    STAT_Sum m_PacketsDroppedOther;
    STAT_Average m_AverageDelay;
    STAT_Average m_AverageJitter;
    STAT_Average m_AverageDataHopCount;
    STAT_Average m_AverageControlHopCount;

    STAT_Throughput m_CarriedLoadUnicast;
    STAT_Throughput m_CarriedLoadMulticast;
    STAT_Throughput m_CarriedLoadBroadcast;
    STAT_Sum m_PacketsDroppedNoRouteUnicast;
    STAT_Sum m_PacketsDroppedNoRouteMulticast;
    STAT_Average m_AverageDelayUnicast;
    STAT_Average m_AverageDelayMulticast;
    STAT_Average m_AverageDelayBroadcast;
    STAT_Average m_AverageJitterUnicast;
    STAT_Average m_AverageJitterMulticast;
    STAT_Average m_AverageJitterBroadcast;
    STAT_Throughput m_OriginatedCarriedLoad;
    STAT_Throughput m_ForwardedCarriedLoad;

private:
    void SetStatNames();

    void AddToGlobal(STAT_GlobalNetStatistics* stat);

    // Called automatically by AddUnicastPacketRecievedFromMacDataPoints, etc.
    void AddPacketSentToTransportDataPoints(Node* node, Message* msg, int interfaceIndex, BOOL isData);

    // Called automatically by AddUnicastPacketSentToMacDataPoints, etc.
    void AddPacketSentToMacDataPoints(Node* node, Message* msg);

public:
    STAT_NetStatistics(Node* node);

    // Get a list of statistics the network model supports
    virtual void GetList(std::vector<STAT_Statistic*>& stats);

    STAT_Sum& GetDataPacketsSentUnicast() { return m_DataPacketsSentUnicast; }
    STAT_Sum& GetDataPacketsReceivedUnicast() { return m_DataPacketsReceivedUnicast; }
    STAT_Sum& GetDataPacketsForwardedUnicast() { return m_DataPacketsForwardedUnicast; }
    STAT_Sum& GetControlPacketsSentUnicast() { return m_ControlPacketsSentUnicast; }
    STAT_Sum& GetControlPacketsReceivedUnicast() { return m_ControlPacketsReceivedUnicast; }
    STAT_Sum& GetControlPacketsForwardedUnicast() { return m_ControlPacketsForwardedUnicast; }
    STAT_Sum& GetDataBytesSentUnicast() { return m_DataBytesSentUnicast; }
    STAT_Sum& GetDataBytesReceivedUnicast() { return m_DataBytesReceivedUnicast; }
    STAT_Sum& GetDataBytesForwardedUnicast() { return m_DataBytesForwardedUnicast; }
    STAT_Sum& GetControlBytesSentUnicast() { return m_ControlBytesSentUnicast; }
    STAT_Sum& GetControlBytesReceivedUnicast() { return m_ControlBytesReceivedUnicast; }
    STAT_Sum& GetControlBytesForwardedUnicast() { return m_ControlBytesForwardedUnicast; }

    STAT_Sum& GetDataPacketsSentBroadcast() { return m_DataPacketsSentBroadcast; }
    STAT_Sum& GetDataPacketsReceivedBroadcast() { return m_DataPacketsReceivedBroadcast; }
    STAT_Sum& GetDataPacketsForwardedBroadcast() { return m_DataPacketsForwardedBroadcast; }
    STAT_Sum& GetControlPacketsSentBroadcast() { return m_ControlPacketsSentBroadcast; }
    STAT_Sum& GetControlPacketsReceivedBroadcast() { return m_ControlPacketsReceivedBroadcast; }
    STAT_Sum& GetControlPacketsForwardedBroadcast() { return m_ControlPacketsForwardedBroadcast; }
    STAT_Sum& GetDataBytesSentBroadcast() { return m_DataBytesSentBroadcast; }
    STAT_Sum& GetDataBytesReceivedBroadcast() { return m_DataBytesReceivedBroadcast; }
    STAT_Sum& GetDataBytesForwardedBroadcast() { return m_DataBytesForwardedBroadcast; }
    STAT_Sum& GetControlBytesSentBroadcast() { return m_ControlBytesSentBroadcast; }
    STAT_Sum& GetControlBytesReceivedBroadcast() { return m_ControlBytesReceivedBroadcast; }
    STAT_Sum& GetControlBytesForwardedBroadcast() { return m_ControlBytesForwardedBroadcast; }

    STAT_Sum& GetDataPacketsSentMulticast() { return m_DataPacketsSentMulticast; }
    STAT_Sum& GetDataPacketsReceivedMulticast() { return m_DataPacketsReceivedMulticast; }
    STAT_Sum& GetDataPacketsForwardedMulticast() { return m_DataPacketsForwardedMulticast; }
    STAT_Sum& GetControlPacketsSentMulticast() { return m_ControlPacketsSentMulticast; }
    STAT_Sum& GetControlPacketsReceivedMulticast() { return m_ControlPacketsReceivedMulticast; }
    STAT_Sum& GetControlPacketsForwardedMulticast() { return m_ControlPacketsForwardedMulticast; }
    STAT_Sum& GetDataBytesSentMulticast() { return m_DataBytesSentMulticast; }
    STAT_Sum& GetDataBytesReceivedMulticast() { return m_DataBytesReceivedMulticast; }
    STAT_Sum& GetDataBytesForwardedMulticast() { return m_DataBytesForwardedMulticast; }
    STAT_Sum& GetControlBytesSentMulticast() { return m_ControlBytesSentMulticast; }
    STAT_Sum& GetControlBytesReceivedMulticast() { return m_ControlBytesReceivedMulticast; }
    STAT_Sum& GetControlBytesForwardedMulticast() { return m_ControlBytesForwardedMulticast; }

    STAT_Throughput& GetCarriedLoad() { return m_CarriedLoad; }
    STAT_Sum& GetPacketsDroppedNoRoute() { return m_PacketsDroppedNoRoute; }
    STAT_Sum& GetPacketsDroppedTtlExpired() { return m_PacketsDroppedTtlExpired; }
    STAT_Sum& GetPacketsDroppedQueueOverflow() { return m_PacketsDroppedQueueOverflow; }
    STAT_Sum& GetPacketsDroppedOther() { return m_PacketsDroppedOther; }
    STAT_Average& GetAverageDelay() { return m_AverageDelay; }
    STAT_Average& GetAverageJitter() { return m_AverageJitter; }
    STAT_Average& GetAverageDataHopCount() { return m_AverageDataHopCount; }
    STAT_Average& GetAverageControlHopCount() { return m_AverageControlHopCount; }

    STAT_Throughput& GetCarriedLoadUnicast() { return m_CarriedLoadUnicast; }
    STAT_Throughput& GetCarriedLoadMulticast() { return m_CarriedLoadMulticast; }
    STAT_Throughput& GetCarriedLoadBroadcast() { return m_CarriedLoadBroadcast; }
    STAT_Sum& GetPacketsDroppedNoRouteUnicast() { return m_PacketsDroppedNoRouteUnicast; }
    STAT_Sum& GetPacketsDroppedNoRouteMulticast() { return m_PacketsDroppedNoRouteMulticast; }
    STAT_Average& GetAverageDelayUnicast() { return m_AverageDelayUnicast; }
    STAT_Average& GetAverageDelayMulticast() { return m_AverageDelayMulticast; }
    STAT_Average& GetAverageDelayBroadcast() { return m_AverageDelayBroadcast; }
    STAT_Average& GetAverageJitterUnicast() { return m_AverageJitterUnicast; }
    STAT_Average& GetAverageJitterMulticast() { return m_AverageJitterMulticast; }
    STAT_Average& GetAverageJitterBroadcast() { return m_AverageJitterBroadcast; }
    STAT_Throughput& GetOriginatedCarriedLoad() { return m_OriginatedCarriedLoad; }
    STAT_Throughput& GetForwardedCarriedLoad() { return m_ForwardedCarriedLoad; }

    // Network protocols should call this function when a packet is
    // sent to the mac layer
    void AddPacketSentToMacDataPoints(
        Node* node,
        Message* msg,
        STAT_DestAddressType type,
        BOOL isData,
        BOOL isForward);
    
    // Network protocols should call this function when a packet is sent up
    // to the transport layer
    void AddPacketSentToTransportDataPoints(
        Node* node,
        Message* msg,
        STAT_DestAddressType type,
        BOOL isData,
        int interfacIndex);

    // Network protocols should call this function when a packet is received
    // from the mac layer
    void AddPacketReceivedFromMacDataPoints(
        Node* node,
        Message* msg,
        STAT_DestAddressType type,
        int interfaceIndex,
        BOOL isData);

    void AddPacketDroppedNoRouteDataPoints(Node* node);
    void AddPacketDroppedNoRouteDataPointsUnicast(Node* node);
    void AddPacketDroppedNoRouteDataPointsMulticast(Node* node);
    void AddPacketDroppedTtlExpiredDataPoints(Node* node);
    void AddPacketDroppedQueueOverflowDataPoints(Node* node);
    void AddPacketDroppedOtherDataPoints(Node* node);
};

// /**
// CLASS        :: STAT_GlobalNetStatistics
// DESCRIPTION  :: Implements global network aggregated statistics.
//                 Values are handled automatically by network_ip.cpp
//                 
// **/
class STAT_GlobalNetStatistics
{
protected:
    STAT_AggregatedSum m_DataPacketsSentUnicast;
    STAT_AggregatedSum m_DataPacketsReceivedUnicast;
    STAT_AggregatedSum m_DataPacketsForwardedUnicast;
    STAT_AggregatedSum m_ControlPacketsSentUnicast;
    STAT_AggregatedSum m_ControlPacketsReceivedUnicast;
    STAT_AggregatedSum m_ControlPacketsForwardedUnicast;
    STAT_AggregatedSum m_DataBytesSentUnicast;
    STAT_AggregatedSum m_DataBytesReceivedUnicast;
    STAT_AggregatedSum m_DataBytesForwardedUnicast;
    STAT_AggregatedSum m_ControlBytesSentUnicast;
    STAT_AggregatedSum m_ControlBytesReceivedUnicast;
    STAT_AggregatedSum m_ControlBytesForwardedUnicast;

    STAT_AggregatedSum m_DataPacketsSentBroadcast;
    STAT_AggregatedSum m_DataPacketsReceivedBroadcast;
    STAT_AggregatedSum m_DataPacketsForwardedBroadcast;
    STAT_AggregatedSum m_ControlPacketsSentBroadcast;
    STAT_AggregatedSum m_ControlPacketsReceivedBroadcast;
    STAT_AggregatedSum m_ControlPacketsForwardedBroadcast;
    STAT_AggregatedSum m_DataBytesSentBroadcast;
    STAT_AggregatedSum m_DataBytesReceivedBroadcast;
    STAT_AggregatedSum m_DataBytesForwardedBroadcast;
    STAT_AggregatedSum m_ControlBytesSentBroadcast;
    STAT_AggregatedSum m_ControlBytesReceivedBroadcast;
    STAT_AggregatedSum m_ControlBytesForwardedBroadcast;

    STAT_AggregatedSum m_DataPacketsSentMulticast;
    STAT_AggregatedSum m_DataPacketsReceivedMulticast;
    STAT_AggregatedSum m_DataPacketsForwardedMulticast;
    STAT_AggregatedSum m_ControlPacketsSentMulticast;
    STAT_AggregatedSum m_ControlPacketsReceivedMulticast;
    STAT_AggregatedSum m_ControlPacketsForwardedMulticast;
    STAT_AggregatedSum m_DataBytesSentMulticast;
    STAT_AggregatedSum m_DataBytesReceivedMulticast;
    STAT_AggregatedSum m_DataBytesForwardedMulticast;
    STAT_AggregatedSum m_ControlBytesSentMulticast;
    STAT_AggregatedSum m_ControlBytesReceivedMulticast;
    STAT_AggregatedSum m_ControlBytesForwardedMulticast;

    STAT_AggregatedThroughput m_CarriedLoad;
    STAT_AggregatedSum m_PacketsDroppedNoRoute;
    STAT_AggregatedSum m_PacketsDroppedTtlExpired;
    STAT_AggregatedSum m_PacketsDroppedQueueOverflow;
    STAT_AggregatedSum m_PacketsDroppedOther;
    STAT_AggregatedAverage m_AverageDelay;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitter;
    STAT_AggregatedWeightedDataPointAverage m_AverageDataHopCount;
    STAT_AggregatedWeightedDataPointAverage m_AverageControlHopCount;

    STAT_AggregatedThroughput m_CarriedLoadUnicast;
    STAT_AggregatedThroughput m_CarriedLoadMulticast;
    STAT_AggregatedThroughput m_CarriedLoadBroadcast;
    STAT_AggregatedSum m_PacketsDroppedNoRouteUnicast;
    STAT_AggregatedSum m_PacketsDroppedNoRouteMulticast;
    STAT_AggregatedAverage m_AverageDelayUnicast;
    STAT_AggregatedAverage m_AverageDelayMulticast;
    STAT_AggregatedAverage m_AverageDelayBroadcast;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitterUnicast;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitterMulticast;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitterBroadcast;

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetDataPacketsSentUnicast() { return m_DataPacketsSentUnicast; }
    STAT_AggregatedSum& GetDataPacketsReceivedUnicast() { return m_DataPacketsReceivedUnicast; }
    STAT_AggregatedSum& GetDataPacketsForwardedUnicast() { return m_DataPacketsForwardedUnicast; }
    STAT_AggregatedSum& GetControlPacketsSentUnicast() { return m_ControlPacketsSentUnicast; }
    STAT_AggregatedSum& GetControlPacketsReceivedUnicast() { return m_ControlPacketsReceivedUnicast; }
    STAT_AggregatedSum& GetControlPacketsForwardedUnicast() { return m_ControlPacketsForwardedUnicast; }
    STAT_AggregatedSum& GetDataBytesSentUnicast() { return m_DataBytesSentUnicast; }
    STAT_AggregatedSum& GetDataBytesReceivedUnicast() { return m_DataBytesReceivedUnicast; }
    STAT_AggregatedSum& GetDataBytesForwardedUnicast() { return m_DataBytesForwardedUnicast; }
    STAT_AggregatedSum& GetControlBytesSentUnicast() { return m_ControlBytesSentUnicast; }
    STAT_AggregatedSum& GetControlBytesReceivedUnicast() { return m_ControlBytesReceivedUnicast; }
    STAT_AggregatedSum& GetControlBytesForwardedUnicast() { return m_ControlBytesForwardedUnicast; }

    STAT_AggregatedSum& GetDataPacketsSentBroadcast() { return m_DataPacketsSentBroadcast; }
    STAT_AggregatedSum& GetDataPacketsReceivedBroadcast() { return m_DataPacketsReceivedBroadcast; }
    STAT_AggregatedSum& GetDataPacketsForwardedBroadcast() { return m_DataPacketsForwardedBroadcast; }
    STAT_AggregatedSum& GetControlPacketsSentBroadcast() { return m_ControlPacketsSentBroadcast; }
    STAT_AggregatedSum& GetControlPacketsReceivedBroadcast() { return m_ControlPacketsReceivedBroadcast; }
    STAT_AggregatedSum& GetControlPacketsForwardedBroadcast() { return m_ControlPacketsForwardedBroadcast; }
    STAT_AggregatedSum& GetDataBytesSentBroadcast() { return m_DataBytesSentBroadcast; }
    STAT_AggregatedSum& GetDataBytesReceivedBroadcast() { return m_DataBytesReceivedBroadcast; }
    STAT_AggregatedSum& GetDataBytesForwardedBroadcast() { return m_DataBytesForwardedBroadcast; }
    STAT_AggregatedSum& GetControlBytesSentBroadcast() { return m_ControlBytesSentBroadcast; }
    STAT_AggregatedSum& GetControlBytesReceivedBroadcast() { return m_ControlBytesReceivedBroadcast; }
    STAT_AggregatedSum& GetControlBytesForwardedBroadcast() { return m_ControlBytesForwardedBroadcast; }

    STAT_AggregatedSum& GetDataPacketsSentMulticast() { return m_DataPacketsSentMulticast; }
    STAT_AggregatedSum& GetDataPacketsReceivedMulticast() { return m_DataPacketsReceivedMulticast; }
    STAT_AggregatedSum& GetDataPacketsForwardedMulticast() { return m_DataPacketsForwardedMulticast; }
    STAT_AggregatedSum& GetControlPacketsSentMulticast() { return m_ControlPacketsSentMulticast; }
    STAT_AggregatedSum& GetControlPacketsReceivedMulticast() { return m_ControlPacketsReceivedMulticast; }
    STAT_AggregatedSum& GetControlPacketsForwardedMulticast() { return m_ControlPacketsForwardedMulticast; }
    STAT_AggregatedSum& GetDataBytesSentMulticast() { return m_DataBytesSentMulticast; }
    STAT_AggregatedSum& GetDataBytesReceivedMulticast() { return m_DataBytesReceivedMulticast; }
    STAT_AggregatedSum& GetDataBytesForwardedMulticast() { return m_DataBytesForwardedMulticast; }
    STAT_AggregatedSum& GetControlBytesSentMulticast() { return m_ControlBytesSentMulticast; }
    STAT_AggregatedSum& GetControlBytesReceivedMulticast() { return m_ControlBytesReceivedMulticast; }
    STAT_AggregatedSum& GetControlBytesForwardedMulticast() { return m_ControlBytesForwardedMulticast; }

    STAT_AggregatedThroughput& GetCarriedLoad() { return m_CarriedLoad; }
    STAT_AggregatedSum& GetPacketsDroppedNoRoute() { return m_PacketsDroppedNoRoute; }
    STAT_AggregatedSum& GetPacketsDroppedTtlExpired() { return m_PacketsDroppedTtlExpired; }
    STAT_AggregatedSum& GetPacketsDroppedQueueOverflow() { return m_PacketsDroppedQueueOverflow; }
    STAT_AggregatedSum& GetPacketsDroppedOther() { return m_PacketsDroppedOther; }
    STAT_AggregatedAverage& GetAverageDelay() { return m_AverageDelay; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitter() { return m_AverageJitter; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDataHopCount() { return m_AverageDataHopCount; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageControlHopCount() { return m_AverageControlHopCount; }

    STAT_AggregatedThroughput& GetCarriedLoadUnicast() { return m_CarriedLoadUnicast; }
    STAT_AggregatedThroughput& GetCarriedLoadMulticast() { return m_CarriedLoadMulticast; }
    STAT_AggregatedThroughput& GetCarriedLoadBroadcast() { return m_CarriedLoadBroadcast; }
    STAT_AggregatedSum& GetPacketsDroppedNoRouteUnicast() { return m_PacketsDroppedNoRouteUnicast; }
    STAT_AggregatedSum& GetPacketsDroppedNoRouteMulticast() { return m_PacketsDroppedNoRouteMulticast; }
    STAT_AggregatedAverage& GetAverageDelayUnicast() { return m_AverageDelayUnicast; }
    STAT_AggregatedAverage& GetAverageDelayMulticast() { return m_AverageDelayMulticast; }
    STAT_AggregatedAverage& GetAverageDelayBroadcast() { return m_AverageDelayBroadcast; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitterUnicast() { return m_AverageJitterUnicast; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitterMulticast() { return m_AverageJitterMulticast; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitterBroadcast() { return m_AverageJitterBroadcast; }
};

// /**
// CLASS        :: STAT_NetSummaryStatistics
// DESCRIPTION  :: Implements global network summary statistics.
//                 
// **/
class STAT_NetSummaryStatistics
{
public:
    NodeAddress senderAddr;
    NodeAddress receiverAddr;
    char destinationType[MAX_STRING_LENGTH];
    double dataPacketsSent;
    double dataPacketsRecd;
    double dataPacketsForward;
    double controlPacketsSent;
    double controlPacketsRecd;
    double controlPacketsForward;
    double dataBytesSent;
    double dataBytesRecd;
    double dataBytesForward;
    double controlBytesSent;
    double controlBytesRecd;
    double controlBytesForward;
    double dataDelay;
    double dataJitter;
    double dataJitterDataPoints;
    double controlDelay;
    double controlJitter;
    double controlJitterDataPoints;

// Moved inside ADDON_DB as compilation error was coming when ADDON-DB
// was not enabled

#ifdef ADDON_DB
    // Call to initialize this struct based on the statistics from the model
    void InitializeFromOneHop(Node* node, OneHopNetworkData* stats);
#endif /* ADDON_DB */
    // Call to update this struct based on statistics from another model's
    // summary statistics.  Can be called for a sender or receiver's stats.
    STAT_NetSummaryStatistics& operator +=(const STAT_NetSummaryStatistics& stats);
};

#endif /* _STATS_NET_H_ */
