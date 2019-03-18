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

#ifndef _DB_H_
#define _DB_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "network.h"
#include "main.h"
#include "stats.h"

#ifdef _WIN32
#include <hash_map>
using namespace std;
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#define MAX_DATABASE_TABLES 7
#define MAX_INSERT_SIZE 1000
#define MIN_EVENT_TABLE_BUFFER 500
#define DB_LONG_BUFFER_LENGTH MAX_STRING_LENGTH*5
#define DB_APP_EVENTS_DEFAULT_BUFFER_LENGTH DB_LONG_BUFFER_LENGTH*2

#define STATSDB_DEFAULT_CONNECTIVITY_INTERVAL (600 * SECOND)
#define STATSDB_DEFAULT_LINK_UTILIZATION_INTERVAL (300 * SECOND)

typedef std::pair<std::string, std::string> string_pair;
typedef std::vector<string_pair> DBColumns;

// Forward Declaration
class StatsDBAppEventParam;
class StatsDBNetworkEventParam;
class StatsDBPhyEventParam;

enum StatsDBLevelSetting
{
    dbLevelNone = 0,
    dbLevelLow = 1,
    dbLevelMedium = 2,
    dbLevelHigh = 3
};
struct StatsDBTable
{
    BOOL createDescriptionTable;
    BOOL createStatusTable;
    BOOL createAggregateTable;
    BOOL createSummaryTable;
    BOOL createEventsTable;
    BOOL createConnectivityTable;
    BOOL createUrbanPropTable;
};

// Description Table Structures.
struct StatsDBDescTable
{
    BOOL createNodeDescTable;
    BOOL createQueueDescTable;
    BOOL createSchedulerDescTable;
    BOOL createSessionDescTable;
    BOOL createConnectionDescTable;
    BOOL createInterfaceDescTable;
    BOOL createPhyDescTable;
};

struct StatsDBQueueDescContent
{
    BOOL isDiscipline;
    BOOL isPriority;
    BOOL isSize;
};

struct StatsDBSessionDescContent
{
    BOOL isSenderAddr;
    BOOL isReceiverAddr;
    BOOL isSenderPort;
    BOOL isRecvPort;
    BOOL isTransportProtocol;
    BOOL isAppType;

    StatsDBSessionDescContent()
    {
        isSenderAddr = FALSE;
        isReceiverAddr = FALSE;
        isSenderPort = FALSE;
        isRecvPort = FALSE;
        isTransportProtocol = FALSE;
        isAppType = FALSE;
    }
};

struct StatsDBSchedulerDescContent
{
    BOOL isSchedulerAlgo;
    StatsDBSchedulerDescContent()
    {
        isSchedulerAlgo = FALSE ;
    }
};


struct StatsDBConnectionDescContent
{
    BOOL isSenderAddr;
    BOOL isReceiverAddr;
    BOOL isSenderPort;
    BOOL isRecvPort;
    BOOL isConnectionType;
    BOOL isNetworkProtocol;

    StatsDBConnectionDescContent()
    {
        // the four below should be removed
        isSenderAddr = TRUE;
        isReceiverAddr = TRUE;
        isSenderPort = TRUE;
        isRecvPort = TRUE;
        // the above four should be removed
        isConnectionType = FALSE;
        isNetworkProtocol = FALSE;
    }
};
struct StatsDBInterfaceDescContent
{
    std::vector<std::pair<std::string, std::string> > interfaceDescTableDef;
    BOOL isName;
    BOOL isAddress;
    BOOL isSubnetMask;
    BOOL isNetworkProtocol;
    BOOL isMulticastProtocol;
    BOOL isSubnetId;

    StatsDBInterfaceDescContent()
    {
        isName = FALSE;
        isAddress = FALSE;
        isSubnetMask = FALSE;
        isNetworkProtocol = FALSE;
        isMulticastProtocol = FALSE;
        isSubnetId = FALSE;
    }
};

// Status Table Structures
struct StatsDBMulticastStatus ;
struct StatsDBStatusTable
{
    BOOL createNodeStatusTable;
    BOOL createInterfaceStatusTable;
    BOOL createQueueStatusTable;
    BOOL createMulticastStatusTable;
    BOOL createMalsrStatusTable;
    clocktype statusInterval;
    BOOL endSimulation ;

    struct ltndaddr
    {
        const bool operator () (const std::pair<Int32, NodeAddress> &s1,
            const std::pair<Int32, NodeAddress>& s2) const
        {
                //return s1.first < s2.first || s1.second< s2.second;
            if (s1.first != s2.first)
            {
                return s1.first < s2.first;
            } else return s1.second < s2.second;
        }
    };
    std::map<std::pair<Int32, NodeAddress>,
        StatsDBMulticastStatus*, ltndaddr> map_MultiStatus;

    typedef std::map<std::pair<Int32, NodeAddress>,
        StatsDBMulticastStatus*, ltndaddr>::const_iterator Const_MultiStatusIter;
    typedef std::map<std::pair<Int32, NodeAddress>,
        StatsDBMulticastStatus*, ltndaddr>::iterator MultiStatusIter;
};

struct StatsDBNodeStatusContent
{
    BOOL isPosition;
    BOOL isActiveState;
    BOOL isDamageState;
    BOOL isVelocity;
    BOOL isGateway;
};

// Aggregate Table Structures.
struct StatsDBAggregateTable
{
    BOOL createAppAggregateTable;
    BOOL createTransAggregateTable;
    BOOL createNetworkAggregateTable;
    BOOL createMacAggregateTable;
    BOOL createPhyAggregateTable;
    BOOL createQueueAggregateTable;
    clocktype aggregateInterval;
    BOOL endSimulation;
};

struct StatsDBAppAggregateContent
{
    BOOL isUnicastDelay;
    BOOL isUnicastJitter;
    BOOL isUnicastHopCount;
    BOOL isMulticastDelay;
    BOOL isMulticastJitter;
    BOOL isMulticastHopCount;
    BOOL isAvgDelay;
    BOOL isAvgJitter;
    BOOL isAvgThroughput;
    BOOL isAvgOfferload;
};

struct StatsDBNetworkAggregateContent
{
    static const Int32 s_numTrafficTypes;
    Int32 i;
    enum NetAggrTrafficType{
        UNICAST,
        MULTICAST,
        BROADCAST
    } ;
    vector<BOOL> isDelay;
    vector<BOOL> isJitter;
    vector<BOOL> isIpOutNoRoutes;
    vector<string> m_trafficStr ;
    StatsDBNetworkAggregateContent()
    {
        for (i = 0; i < s_numTrafficTypes; ++i)
        {
            isDelay.push_back(FALSE) ;
            isJitter.push_back(FALSE) ;
            isIpOutNoRoutes.push_back(FALSE) ;
        }
        m_trafficStr.push_back("Unicast") ;
        m_trafficStr.push_back("Multicast") ;
        m_trafficStr.push_back("Broadcast") ;
    }
};

struct StatsDBMacAggregateAddressContent
{
    BOOL isAvgQueuingDelay;
    BOOL isAvgMediumAccessDelay;
    BOOL isAvgMediumDelay;
    BOOL isAvgJitter;
};
struct StatsDBMacAggregateContent
{
    StatsDBMacAggregateAddressContent addrTypes[STAT_NUM_ADDRESS_TYPES];
};
struct StatsDBPhyAggregateContent
{
    BOOL isAvgPathLoss;
    BOOL isAvgSignalPower;
    BOOL isAvgDelay;
};


struct StatsDBTransportAggregateContent
{
    static const Int32 s_numAddressTypes;
    Int32 i;
    enum TransAggrAddressType{
        UNICAST,
        MULTICAST,
        BROADCAST
    } ;
    vector<BOOL> isDelay;
    vector<BOOL> isJitter;
    vector<string> m_addressStr;
    StatsDBTransportAggregateContent()
    {
        for (i = 0; i < s_numAddressTypes; ++i)
        {
            isDelay.push_back(FALSE) ;
            isJitter.push_back(FALSE) ;
        }
        m_addressStr.push_back("Unicast") ;
        m_addressStr.push_back("Multicast") ;
        m_addressStr.push_back("Broadcast") ;
    }
};


// Summary Table Structures.
struct StatsDBSummaryTable
{
    BOOL createAppSummaryTable;
    BOOL createMulticastAppSummaryTable;
    BOOL createMulticastNetSummaryTable;
    BOOL createTransSummaryTable;
    BOOL createNetworkSummaryTable;
    BOOL createMacSummaryTable;
    BOOL createPhySummaryTable;
    BOOL createQueueSummaryTable;
    clocktype summaryInterval;
    BOOL endSimulation;
};

struct StatsDBAppSummaryContent
{
    BOOL isDelay;
    BOOL isJitter;
    BOOL isHopCount;
};

struct StatsDBMulticastAppSummaryContent
{
    BOOL isDelay;
    BOOL isJitter;
    BOOL isHopCount;
};

class StatsDBMulticastNetworkSummaryContent
{
public:
    char m_ProtocolType[20];
    Int32 m_NumDataSent;
    Int32 m_NumDataRecvd;
    Int32 m_NumDataForwarded;
    Int32 m_NumDataDiscarded;

    StatsDBMulticastNetworkSummaryContent()
    {
        strcpy(m_ProtocolType,"");
        m_NumDataSent = 0;
        m_NumDataRecvd = 0;
        m_NumDataForwarded = 0;
        m_NumDataDiscarded = 0;
    }
};

struct StatsDBQosSummaryContent
{
    BOOL isIngressByteCount;
    BOOL isEgressByteCount;
    BOOL isMlpp;
    BOOL isClass;
    BOOL isNumAdmitted;
    BOOL isNumDenied;
};

struct StatsDBTransSummaryAddressContent
{
    BOOL isDelay;
    BOOL isJitter;
    
    StatsDBTransSummaryAddressContent()
    {
        isDelay = FALSE ;
        isJitter = FALSE ;
    }
};
struct StatsDBTransSummaryContent
{
    StatsDBTransSummaryAddressContent addrTypes[STAT_NUM_ADDRESS_TYPES];
};

struct StatsDBNetworkSummaryContent
{
    BOOL isDataDelay;
    BOOL isControlDelay;
    BOOL isDataJitter ;
    BOOL isControlJitter;
};

struct StatsDBMacSummaryAddressContent
{
    BOOL isAvgQueuingDelay;
    BOOL isAvgMediumAccessDelay;
    BOOL isAvgMediumDelay;
    BOOL isAvgJitter;
};
struct StatsDBMacSummaryContent
{
    StatsDBMacSummaryAddressContent addrTypes[STAT_NUM_ADDRESS_TYPES];
};

struct MacSummaryInfo
{
    Int32 senderId;
    Int32 interfaceIndex;
    clocktype macSummaryPktSendTime;
};

struct StatsDBPhySummaryContent
{
    BOOL isAvgPathLoss;
    BOOL isAvgSignalPower;
    BOOL isAvgDelay;
};

// Events Table Structures.
struct StatsDBEventsTable
{
    BOOL createAppEventsTable;
    BOOL createTransEventsTable;
    BOOL createNetworkEventsTable;
    BOOL createMacEventsTable;
    BOOL createPhyEventsTable;
    BOOL createQueueEventsTable;
    BOOL createMessageIdMappingTable;
};

struct StatsDBAppEventContent
{
    BOOL isMsgSeqNum;
    BOOL isFragId;
    BOOL isSession;
    BOOL isPriority;
    BOOL isMsgFailureType;
    BOOL isDelay;
    BOOL isJitter;
    BOOL appStatsDBSend;
    BOOL appStatsDBReceive;

    BOOL isSocketInterfaceMsgIds;
    BOOL recordDuplicate;  // whether to record dups in event table
    BOOL recordOutOfOrder; // whether to record out of order in event table
    BOOL recordFragment;   // whether to record fragments in event table
    Int32  seqCacheSize;     // Limit of the sequence number cache for
                           // determining duplicate and out of order
    BOOL multipleValues;
    Int32 bufferSizeInBytes;
    std::string appEventsTbColsName;

    StatsDBAppEventContent()
    {
        isMsgSeqNum = FALSE;
        isFragId = FALSE;
        isSession = FALSE;
        isPriority = FALSE;
        isMsgFailureType = FALSE;
        isDelay = FALSE;
        isJitter = FALSE;
        appStatsDBSend = FALSE;
        appStatsDBReceive = FALSE;
        bufferSizeInBytes = DB_APP_EVENTS_DEFAULT_BUFFER_LENGTH;
        multipleValues = FALSE;
        recordDuplicate = FALSE;
        recordOutOfOrder = FALSE;
        recordFragment = FALSE;
        seqCacheSize = 0;
    }

};

struct StatsDBTransEventContent
{
    BOOL isSenderPort;
    BOOL isReceiverPort;
    BOOL isConnType;
    BOOL isHdrSize ;
    BOOL isFlags;
    BOOL isEventType;
    BOOL isMsgSeqNum;
    BOOL isMsgFailureType;

    StatsDBTransEventContent()
    {
        isSenderPort = FALSE;
        isReceiverPort = FALSE;
        isConnType = FALSE;
        isHdrSize = FALSE;
        isFlags = FALSE;
        isEventType = FALSE;
        isMsgSeqNum = FALSE;
        isMsgFailureType = FALSE;
    }
};

struct StatsDBNetworkEventContent
{
    BOOL isMsgSeqNum;
    BOOL isFragId;
    BOOL isMacProtocol;
    BOOL isControlSize;
    BOOL isPriority;
    BOOL isProtocolType;
    BOOL isPktFailureType;
    BOOL isPktType;
    BOOL isInterfaceIndex;
    BOOL isHopCount;
    BOOL networkStatsDBControl;
    BOOL networkStatsDBIncoming;
    BOOL networkStatsDBOutgoing;

    Int32 bufferSizeInBytes;
    BOOL multipleValues;
    static const char formatString[];
    char networkEventsTbColsName[MAX_STRING_LENGTH *2];
    Int32 lengthOfNetworkEventsTbName;
};

struct StatsDBMacEventContent
{
    BOOL macStatsDBControl;
    BOOL macStatsDBIncoming;
    BOOL macStatsDBOutgoing;

    BOOL isMsgSeqNum;
    BOOL isChannelIndex;
    BOOL isMessageFailureType;
    BOOL isFrameType;

    StatsDBMacEventContent()
    {
        macStatsDBControl = FALSE;
        macStatsDBIncoming = FALSE;
        macStatsDBOutgoing = FALSE;
        isMsgSeqNum = FALSE ;
        isChannelIndex = FALSE ;
        isMessageFailureType = FALSE ;
        isFrameType = FALSE ;
    }
};

struct StatsDBPhyEventContent
{
    BOOL isChannelIndex;
    BOOL isControlSize;
    BOOL isMessageFailureType;
    BOOL isSignalPower;
    BOOL isInterference;
    BOOL isPathLoss;
};

// Connectivity Table Structures
struct StatsDBAppConnParam ;
struct StatsDBTransConnParam ;
struct StatsDBConnTable
{

    BOOL endSimulation ;
    BOOL createAppConnTable;
    BOOL createTransConnTable;
    BOOL createNetworkConnTable;
    BOOL createMacConnTable;
    BOOL createPhyConnTable;
    BOOL createMulticastConnTable;

    clocktype AppConnTableInterval;
    clocktype TransConnTableInterval;
    clocktype macConnSampleTimeInterval;
    clocktype networkConnSampleTimeInterval;
    clocktype connectSampleTimeInterval;
    clocktype multicastConnSampleTimeInterval;
    // APP Connectivity table
    // key: senderSessionId & senderAddress
    struct ltaddr
    {
        const bool operator () (const std::pair<Address, Int32> &s1,
            const std::pair<Address, Int32> &s2) const
        {
            if (s1.first.networkType == NETWORK_IPV4 && s2.first.networkType == NETWORK_IPV4)
            {
                //return s1.first.interfaceAddr.ipv4 < s2.first.interfaceAddr.ipv4 ||
                //    s1.second < s2.second;
                if (s1.first.interfaceAddr.ipv4 != s2.first.interfaceAddr.ipv4)
                {
                    return s1.first.interfaceAddr.ipv4 < s2.first.interfaceAddr.ipv4;
                } else  {
                    return s1.second < s2.second;
                }
            }
            else if (s1.first.networkType == NETWORK_IPV6 && s2.first.networkType == NETWORK_IPV6)
            {
                return  FALSE;
                /*(
                    //s1.first.interfaceAddr.ipv6.s6_addr32[0] < s2.first.interfaceAddr.ipv6.s6_addr32[0] ||
                    s1.first.interfaceAddr.ipv6.u6_addr.u6_addr32[0] < s2.first.interfaceAddr.ipv6.u6_addr.u6_addr32[0] || \
                    s1.first.interfaceAddr.ipv6.u6_addr.u6_addr32[1] < s2.first.interfaceAddr.ipv6.u6_addr.u6_addr32[1] || \
                    s1.first.interfaceAddr.ipv6.u6_addr.u6_addr32[2] < s2.first.interfaceAddr.ipv6.u6_addr.u6_addr32[2] || \
                    s1.first.interfaceAddr.ipv6.u6_addr.u6_addr32[3] < s2.first.interfaceAddr.ipv6.u6_addr.u6_addr32[3] || \
                    s1.second < s2.second);    */
            }
            ERROR_Assert(FALSE, "Error in ltaddrpair operator.");
            return FALSE;
        }
    };

    // client addr and sessionId serve as the key
    typedef std::map<std::pair<Address, Int32>,
        StatsDBAppConnParam*,
        ltaddr> V_AppConnParam;
    V_AppConnParam* v_AppConnParam;
    typedef std::map<std::pair<Address, Int32>,
        StatsDBAppConnParam*,
        ltaddr>::iterator V_AppConnParamIter;

    // Transport Connectivity table
    // key: senderAddress & senderPort
    typedef std::multimap<std::pair<Address, Int32>,
        StatsDBTransConnParam*, ltaddr> V_TransConnParam;
    V_TransConnParam* v_TransConnParam;
    typedef std::multimap<std::pair<Address, Int32>,
        StatsDBTransConnParam*, ltaddr>::iterator V_TransConnParamIter;

    // MAC Connectivity table
    struct ltndintpair
    {
        const bool operator () (const std::pair<Int32, Int32> &s1,
            const std::pair<Int32, Int32>& s2) const
        {
               // return s1.first - s2.first < 0 || s1.second - s2.second < 0;
            if (s1.first != s2.first) {
                return s1.first < s2.first;
            }
            else return s1.second < s2.second;
        }
    };
    struct macConnTableElement
    {
        BOOL isConnected;
        std::string channelIndex_str;
        Int32 interfaceIndex;
        Int32 channelIndex;

        macConnTableElement() {isConnected = FALSE;}
        macConnTableElement(BOOL c, std::string s, Int32 ch)
        {
            isConnected = c;
            channelIndex_str = s;
            channelIndex = ch ;
        }
    };
    typedef std::multimap<std::pair<Int32, Int32>,
        macConnTableElement*, ltndintpair>::iterator MacConnTable_Iterator;
    std::multimap<std::pair<Int32, Int32>,
        macConnTableElement*,
        ltndintpair> MacConnTable; // per interval

    // phy connectivity data structure
    struct AdjacencyNode
    {
        Int32 channelIndex;
    };

    // multicast connectivity data structure
    struct MulticastConnectivity
    {
        NodeId nodeId;
        NodeAddress destAddr;
        char rootNodeType[10];
        NodeId rootNodeId;
        Int32 outgoingInterface;
        NodeId upstreamNeighborId;
        Int32 upstreamInterface;
    };

    std::vector<MulticastConnectivity> MulticastConnTable;

    //typedef multimap<pair<int, int>, AdjacencyNode*> AdjacencyNodeList;
    //hash_map<pair<int, int>, AdjacencyNodeList *> SenderList;

    StatsDBConnTable()
    {
        endSimulation = TRUE;
        createAppConnTable = FALSE;
        createTransConnTable = FALSE;
        createNetworkConnTable = FALSE;
        createMacConnTable = FALSE;
        createPhyConnTable = FALSE;
        createMulticastConnTable = FALSE;
        v_AppConnParam = NULL;
        v_TransConnParam = NULL;

        AppConnTableInterval = STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
        TransConnTableInterval = STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
        macConnSampleTimeInterval = STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
        connectSampleTimeInterval = STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
        multicastConnSampleTimeInterval = STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
    }
};

struct StatsDBNetworkConnContent
{
    BOOL isDstMaskAddr;
    BOOL isOutgoingInterfaceIndex;
    BOOL isNextHopAddr;
    BOOL isRoutingProtocol;
    BOOL isAdminDistance;
};
struct StatsDBMacConnContent
{
    BOOL isInterfaceIndex;
    BOOL isChannelIndex;

    StatsDBMacConnContent ()
    {
        isInterfaceIndex = FALSE;
        isChannelIndex = FALSE;
    }
};
struct StatsDBPhyConnContent
{
    BOOL isPhyIndex;
    BOOL isChannelIndex;
};

struct StatsDBNetworkAggregate
{
#if 0
    UInt64 uDataPacketsSent;
    UInt64 uDataPacketsRecd;
    UInt64 uDataPacketsForward;
    UInt64 uControlPacketsSent;
    UInt64 uControlPacketsRecd;
    UInt64 uControlPacketsForward;
    UInt64 mDataPacketsSent;
    UInt64 mDataPacketsRecd;
    UInt64 mDataPacketsForward;
    UInt64 mControlPacketsSent;
    UInt64 mControlPacketsRecd;
    UInt64 mControlPacketsForward;
    UInt64 bDataPacketsSent;
    UInt64 bDataPacketsRecd;
    UInt64 bDataPacketsForward;
    UInt64 bControlPacketsSent;
    UInt64 bControlPacketsRecd;
    UInt64 bControlPacketsForward;
    UInt64 uDataBytesSent;
    UInt64 uDataBytesRecd;
    UInt64 uDataBytesForward;
    UInt64 uControlBytesSent;
    UInt64 uControlBytesRecd;
    UInt64 uControlBytesForward;
    UInt64 mDataBytesSent;
    UInt64 mDataBytesRecd;
    UInt64 mDataBytesForward;
    UInt64 mControlBytesSent;
    UInt64 mControlBytesRecd;
    UInt64 mControlBytesForward;
    UInt64 bDataBytesSent;
    UInt64 bDataBytesRecd;
    UInt64 bDataBytesForward;
    UInt64 bControlBytesSent;
    UInt64 bControlBytesRecd;
    UInt64 bControlBytesForward;
    clocktype firstPacketSendTime;
    clocktype lastPacketSendTime;
#endif
    // Number of IP datagrams discarded because no route could be found.

    UInt32 ipUnicastOutNoRoutes;
    UInt32 ipMulticastOutNoRoutes ;
};

struct StatsDBMacAggregate
{
    long dataFramesSent;
    long dataFramesReceived;
    long dataBytesSent;
    long dataBytesReceived;
    long controlFramesSent;
    long controlFramesReceived;
    long controlBytesSent;
    long controlBytesReceived;
    clocktype totalDelay;
    clocktype totalJitter;

    // <<snderId, interfaceIndex>, clocktype>
    std::map<pair<Int32, Int32>, pair<clocktype, UInt64> > perSourceInfo;

    StatsDBMacAggregate()
    {
        dataFramesSent = 0;
        dataFramesReceived = 0;
        dataBytesSent = 0;
        dataBytesReceived = 0;
        controlFramesSent = 0;
        controlFramesReceived = 0;
        controlBytesSent = 0;
        controlBytesReceived = 0;
        totalDelay = 0;
        totalJitter = 0;
    }
};
struct StatsDBPhyAggregate
{
    Int32 m_SignalsTransmitted;

    Int32 m_SignalsReceived;

    Int32 m_SignalsDropped;
};

struct StatsDBTransAggregate
{
    Int32 m_SegmentSentToApp;
    Int32 m_SegmentSentToNet;
    Int32 m_ByteSentToApp;
    Int32 m_ByteSentToNet;
    double m_OfferedLoad;
    double m_Throughput;
    double m_AverageDelay;
    double m_AverageJitter;

    clocktype totalDelay;
    clocktype lastDelayTime;
    clocktype totalJitter;
    clocktype actJitter;

    StatsDBTransAggregate()
    {
        m_SegmentSentToApp = 0;
        m_SegmentSentToNet = 0;
        m_ByteSentToApp = 0;
        m_ByteSentToNet = 0;
        m_OfferedLoad = 0;
        m_Throughput = 0;
        m_AverageDelay = 0;
        m_AverageJitter = 0;

        totalDelay = 0;
        lastDelayTime = 0;
        totalJitter = 0;
        actJitter = 0;
    }
};

struct StatsDBMacSummary
{
    Int32 bcast_dataFramesSent;
    Int32 ucast_dataFramesSent;
    Int32 bcast_dataFramesReceived;
    Int32 ucast_dataFramesReceived;
    Int32 bcast_dataBytesSent;
    Int32 ucast_dataBytesSent;
    Int32 bcast_dataBytesReceived;
    Int32 ucast_dataBytesReceived;

    Int32 controlFramesSent;
    Int32 controlFramesReceived;
    Int32 controlBytesSent;
    Int32 controlBytesReceived;
    Int32 FramesDropped;
    Int32 BytesDropped;

    clocktype totalDelay;
    clocktype totalJitter;

    // <<snderId, interfaceIndex>, clocktype>
    std::map<pair<Int32, Int32>, pair<clocktype, UInt64> > perSourceInfo;

    StatsDBMacSummary()
    {
        bcast_dataFramesSent = 0;
        ucast_dataFramesSent = 0;
        bcast_dataFramesReceived = 0;
        ucast_dataFramesReceived = 0;
        bcast_dataBytesSent = 0;
        ucast_dataBytesSent = 0;
        bcast_dataBytesReceived = 0;
        ucast_dataBytesReceived = 0;

        controlFramesSent = 0;
        controlFramesReceived = 0;
        controlBytesSent = 0;
        controlBytesReceived = 0;
        FramesDropped = 0;
        BytesDropped = 0;

        totalDelay = 0;
        totalJitter = 0;

    }
};

struct OneHopNeighborStats
{
    UInt64 uDataPacketsSent;
    UInt64 uDataPacketsRecd;
    UInt64 uDataPacketsForward;
    UInt64 uControlPacketsSent;
    UInt64 uControlPacketsRecd;
    UInt64 uControlPacketsForward;

    UInt64 uDataBytesSent;
    UInt64 uDataBytesRecd;
    UInt64 uDataBytesForward;
    UInt64 uControlBytesSent;
    UInt64 uControlBytesRecd;
    UInt64 uControlBytesForward;

    clocktype firstPacketSendTime;
    clocktype lastPacketSendTime;

    // differs between data & control
    clocktype dataPacketDelay;
    clocktype dataPacketJitter;
    clocktype lastDataPacketDelayTime;
    clocktype controlPacketDelay;
    clocktype controlPacketJitter;
    clocktype lastControlPacketDelayTime ;

    // the following is for the network_aggr table
    clocktype totalJitter;
    clocktype lastDelayTime;
};

struct OneHopNetworkData
{
    NodeId neighborId;
    std::string srcAddr;
    std::string rcvAddr;
    std::string destinationType;
    BOOL isServer;

    // the following is for the summary and aggregate
    OneHopNeighborStats oneHopStats;
};

struct TransSrcDstData
{
    //std::string connectionId;

    short SenderPort;
    short ReceiverPort;

    Int32 SegmentsSentToApp;
    Int32 SegmentsSentToNet;
    Int32 BytesSentToApp;
    Int32 BytesSentToNet;

    double OfferedLoad;
    double Throughput;

    clocktype TotalDelay;
    clocktype lastDelayTime;
    clocktype TotalJitter;
    clocktype actJitter;

    // if IsSender, then bit 0 is set
    // if IsReceiver, then bit 1 is set
    UInt32 isSender ;

    static void SetIsSender(UInt32 *_isSender, BOOL type)
    {
        if (type)
        {
            // isSender is bit 0 (from right)

            // clears bit 0
            *_isSender = *_isSender & (~(maskInt(32, 32)));

            // setting bit 0
            //printf("LshiftInt(1, 8) = %x *_isSender = %x\n", LshiftInt(1, 8), *_isSender) ;
            *_isSender = *_isSender | LshiftInt(1, 32);
        }else
        {
            // isReceiver is bit 1 (from right)

            // clears bit 1
            *_isSender = *_isSender & (~(maskInt(31, 31)));

            // setting bit 1
            *_isSender = *_isSender | LshiftInt(1, 31);
        }
    }

    static BOOL GetIsSender(UInt32 _isSender, BOOL type)
    {
        UInt32 _is = _isSender;
        if (type)
        {
            // finding sender

            // clears bit 0
            _is = _is & maskInt(32, 32);

            // right shifts 32-32 bits so that bit 0 contains the value of isSender
            _is = RshiftInt(_is, 32);


        }else {
            // finding receiver

            // clears bit 1
            _is = _is & maskInt(31, 31);

            // right shifts 32-31 bits so that bit 0 contains the value of isSender
            _is = RshiftInt(_is, 31);
            //printf("_isSender = %x, _is = %d \n", _isSender, _is) ;

        }
        return (BOOL)_is;

    }
    BOOL isUpdated ;
    TransSrcDstData(const std::string & eventType)
    {
        SenderPort = 0;
        ReceiverPort = 0;
        SegmentsSentToApp = 0;
        SegmentsSentToNet = 0;
        BytesSentToApp = 0;
        BytesSentToNet = 0;

        OfferedLoad = 0;
        Throughput = 0;

        TotalDelay = 0;
        lastDelayTime = 0;
        TotalJitter = 0;
        actJitter = 0;

        isSender = 0 ;
        isUpdated = FALSE;
    }
};

struct PhyOneHopNeighborStats
{
    double utilization;
    double totalInterference;
    double totalPathLoss;
    double totalDelay;
    double totalSignalPower;
    clocktype lastSignalStartTime;
    Int32 numSignals;
    Int32 numErrorSignals;
};

struct PhyOneHopNeighborData
{
    NodeAddress sourceId;
    PhyOneHopNeighborStats phyOneHopStats;
};

struct MacOneHopNeighborStats
{
    double utilization;
    double totalDelay;
    Int32 numSignals;
    Int32 numErrorSignals;
};

#define DEFAULT_MSG_ID_SIZE 64
#if 0
struct StatsDBMappingParam
{
    char app_msgId[DEFAULT_MSG_ID_SIZE];
    //char udp_msgId[DEFAULT_MSG_ID_SIZE];
    //char tcp_msgId[DEFAULT_MSG_ID_SIZE];
    char transrouting_msgId[DEFAULT_MSG_ID_SIZE] ;
    char net_msgId[DEFAULT_MSG_ID_SIZE];
    char mac_msgId[DEFAULT_MSG_ID_SIZE];

    BOOL app_msgId_specified;
    //BOOL udp_msgId_specified;
    //BOOL tcp_msgId_specified;
    BOOL transrouting_msgId_specified ;
    BOOL net_msgId_specified;
    BOOL mac_msgId_specified;

    //std::string phy_msgId;
};
#else
struct StatsDBMappingParam
{
    char msgId[DEFAULT_MSG_ID_SIZE];
};
#endif

struct StatsDBPhySummDelayInfo
{
    clocktype txDelay;
    clocktype sendTime;
};

struct StatsDBUrbanPropData
{
    Int32 txNodeId;
    Int32 rxNodeId;
    Coordinates txPosition;
    Coordinates rxPosition;
    double frequency;
    double distance;
    bool freeSpace;
    bool LoS;
    bool txInCanyon;
    bool rxInCanyon;
    bool txIndoors;
    bool rxIndoors;
    std::string modelSelected;
    double pathloss;
    double freeSpacePathloss;
    double twoRayPathloss;
    double itmPathloss;
    Int32 numWalls;
    Int32 numFloors;
    double outsideDistance;
    double insideDistance1;
    double insideDistance2;
    double outsidePathloss;
    double insidePathloss1;
    double insidePathloss2;

    StatsDBUrbanPropData() {
        frequency = 0.0;
        distance = 0.0;
        freeSpace = false;
        LoS = false;
        txInCanyon = false;
        rxInCanyon = false;
        txIndoors = false;
        rxIndoors = false;
        pathloss = 0.0;
        freeSpacePathloss = 0.0;
        twoRayPathloss = 0.0;
        itmPathloss = 0.0;
        numWalls = 0;
        numFloors = 0;
        outsideDistance = 0;
        insideDistance1 = 0;
        insideDistance2 = 0;
        outsidePathloss = 0;
        insidePathloss1 = 0;
        insidePathloss2 = 0;
    }
};

struct StatsDBMessageNextPrevHop
{
    NodeAddress nextHopId;   // node ID of next hop
    NodeAddress prevHopAddr; // IPv4 address of my outgoing inf
};

struct StatsDBMessageAddrInfo
{
    NodeAddress srcAddr;
    NodeAddress dstAddr;
};


void StatsDBAddMessageAppMsgId(Node* node,
                               Message* msg,
                               StatsDBAppEventParam* appParam);
void StatsDBAddMessageUdpMsgId(Node* node, Message* msg);
void StatsDBAddMessageTcpMsgId(Node* node, Message* msg);
void StatsDBAddMessageTcpMsgId(Node* node, Message* msg, Int32 index);
void StatsDBAppendMessageNetworkMsgId(
    Node* node,
    Message* msg,
    Int32 fragId);
void StatsDBTrimMessageNetworkMsgId(Node* node, Message* msg) ;
void StatsDBAppendMessageMsgId(
    Node* node,
    Message* msg,
    Int32 fragId,
    char l);
void StatsDBTrimMessageMsgId(Node* node, Message* msg, char l);
void STATSDB_Initialize(NodeInput* nodeInput,
                       char* experimentPrefix);

void STATSDB_Initialize(PartitionData* partition,
                       NodeInput* nodeInput,
                       char* experimentPrefix);
void InitializeStatsDbNetworkAggregateStats(
    StatsDBNetworkAggregate* netAggregateStats);
void InitializeStatsDbNetworkSummaryStats(
    OneHopNeighborStats* stats);

/* FUNCTION     STATSDB_Regression
 * PURPOSE      Run database regression.  This function does not return.
 *
 * Parameters
 *    prefix: char*: The name of the experiment
 */
void STATSDB_Regression(char* prefix);

// API's for description Table
void InitializeStatsDBDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBNodeDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBQueueDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBSchedulerDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBSessionDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBConnectionDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBInterfaceDescTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBPhyDescTable(PartitionData* partition, NodeInput* nodeInput);

// APIs for Status Table
void InitializeStatsDBStatusTable(PartitionData* partition, NodeInput* nodeInput);
void StatsDBSendStatusTimerMessage(PartitionData *);
void InitializeStatsDBNodeStatusTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBInterfaceStatusTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBMulticastStatusTable(PartitionData* partition,
    NodeInput* nodeInput);
void InitializeStatsDBQueueStatusTable(PartitionData* partition,
    NodeInput* nodeInput);
// API's For Aggregate Table
void InitializeStatsDBAggregateTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBAppAggregateTable(PartitionData* paritionData, NodeInput* nodeInput);

void HandleStatsDBAppAggregateInsertion(Node* node);
void StatsDBSendAggregateTimerMessage(PartitionData* partition);

// API For Transport Aggregate Table
void InitializeStatsDBTransAggregateTable(PartitionData* , NodeInput* );
void HandleStatsDBTransAggregateInsertion(Node *node);

void InitializeStatsDBNetworkAggregateTable(PartitionData* paritionData, NodeInput* nodeInput);
void HandleStatsDBNetworkAggregateInsertion(Node* node);

//void HandleStatsDBNetworkAggregateUpdate(Node* node,
//                                         Message* msg,
//                                         BOOL inComing,
//                                         int interfaceIndex);

void InitializeStatsDBMacAggregateTable(PartitionData* partition, NodeInput* nodeInput);
void HandleStatsDBMacAggregateInsertion(Node* node);
void InitializeStatsDBPhyAggregateTable(PartitionData* partition, NodeInput* nodeInput);
void HandleStatsDBPhyAggregateInsertion(Node* node);
void InitializeStatsDBQueueAggregateTable(PartitionData* partition, NodeInput* nodeInput);
void HandleStatsDBQueueAggregateInsertion(Node* node);

// API's For Summary Table
void InitializeStatsDBMulticastAppSummaryTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMulticastNetSummaryTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBSummaryTable(PartitionData* partition, NodeInput* nodeInput);
void InitializeStatsDBAppSummaryTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMacSummaryTable(PartitionData* partition,
                                NodeInput* nodeInput);
void InitializeStatsDBPhySummaryTable(PartitionData* paritionData, NodeInput* nodeInput);
void HandleStatsDBAppSummaryInsertion(Node* node);
void HandleStatsDBMulticastAppSummaryInsertion(Node* node);

void InitializeStatsDBTransSummaryTable(PartitionData* partition,
                                NodeInput* nodeInput);
void HandleStatsDBTransSummaryInsertion(Node *node);
void InitializeStatsDBNetworkSummaryTable(PartitionData* paritionData, NodeInput* nodeInput);
void HandleStatsDBNetworkSummaryInsertion(Node* node);

void InitializeStatsDBQueueSummaryTable(PartitionData* partition, NodeInput* nodeInput);
void HandleStatsDBQueueSummaryInsertion(Node* node);

struct AddressInterfacePair
{
    Int32 interfaceIndex;
    BOOL interfaceIndexSpecified;
    NodeAddress hopAddr;
    BOOL hopAddrSpecified;
    AddressInterfacePair()
    {
        interfaceIndexSpecified = FALSE;
        hopAddrSpecified = FALSE;
    }

    void SetInterfaceIndex(Int32 i)
    {
        interfaceIndex = i;
        interfaceIndexSpecified = TRUE;
    }
    void SetHopAddr(NodeAddress a)
    {
        hopAddr = a ;
        hopAddrSpecified = TRUE;
    }
};

void HandleStatsDBNetworkOutUpdate(
    Node* node,
    Message* msg,
    NodeAddress nextHop,
    Int32 outgoingInterface);
void HandleStatsDBNetworkInUpdate(
    Node* node,
    Message* msg,
    NodeAddress prevHop,
    Int32 incomingInterface);

void HandleStatsDBNetworkSummaryInUpdate(
    Node* node,
    Message* msg,
    NodeAddress prevHop,
    Int32 incomingInterface);

void HandleStatsDBMacSummaryInsertion(Node* node);
void HandleStatsDBPhySummaryInsertion(Node* node);
void HandleStatsDBPhySummaryInsertionForMacProtocols(Node* node);


void HandleStatsDBPhySummaryUpdateForMacProtocols(
    Node* node,
    Int32 interfaceIndex,
    NodeAddress sourceId,
    std::map<Int32, MacOneHopNeighborStats>* macOneHopData,
    clocktype txDelay,
    clocktype totalDelay,
    BOOL isErrorMessage);

void StatsDBSendSummaryTimerMessage(PartitionData* partition);

void HandleStatsDBMacConnInsertion(Node* node);

void StatsDBSendConnTimerMessage(PartitionData* partition);
// API's for Events Table
void InitializeStatsDBEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBAppEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBTransEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBNetworkEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMacEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBPhyEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBQueueEventsTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMessageIdMappingTable(PartitionData* partition,
                                            NodeInput* nodeInput);
void HandleNetworkDBEvents(Node* node,
                           Message* msg,
                           Int32 interfaceIndex,
                           const std::string& eventType,
                           const std::string& failure,
                           NodeAddress srcAddr,
                           NodeAddress dstAddr,
                           TosType priority,
                           unsigned char protocol,
                           Int32 hdrSize = 0);
void HandleNetworkDBEventsForPimSm(
    Node* node,
    Message* msg,
    Int32 interfaceIndex,
    const std::string& eventType,
    const std::string& failure,
    NodeAddress srcAddr,
    NodeAddress dstAddr,
    TosType priority,
    unsigned char protocol,
    Int32 ipHdrSize = 0);

void HandleStatsDBAppSendEventsInsertion(
    Node* node,
    Message* msg,
    Int32 msgSize,
    StatsDBAppEventParam* appParam);
void HandleStatsDBAppReceiveEventsInsertion(
    Node* node,
    Message* msg,
    const char* eventType,
    clocktype delay,
    clocktype jitter,
    Int32 size,
    Int32 numReqMsgRcvd);
void HandleStatsDBAppDropEventsInsertion(
    Node* node,
    Message* msg,
    const char* eventType,
    clocktype delay,
    Int32 size,
    char* failureType);


struct IpHeaderType;
void HandleStatsDBNetworkEventsInsertion(
    Node* node,
    Message* msg,
    Int32 interfaceIndex,
    IpHeaderType* ipHeader,
    const std::string& eventType,
    const std::string& failure,
    NodeAddress srcAddr,
    NodeAddress dstAddr,
    TosType priority,
    unsigned char protocol,
    Int32 ipHdrSize);

#ifdef ADDON_NGCNMS
void HandleStatsDBNetworkEventsInsertionForOtherTables(
    Node* node,
    Message* msg,
    Int32 msgSize,
    StatsDBNetworkEventParam* ipParam);
#endif
// API's for Connectivity Table
void InitializeStatsDBConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBAppConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBTransConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBNetworkConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMacConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBPhyConnTable(PartitionData* paritionData, NodeInput* nodeInput);
void InitializeStatsDBMulticastConnTable(PartitionData* partitionData, NodeInput* nodeinput);

void InitializeStatsDBUrbanPropTable(PartitionData* partitionData, NodeInput* nodeInput);
void STATSDB_HandleUrbanPropInsertion(const Node* node,
                                      StatsDBUrbanPropData* urbanData);

void STATSDB_HandleAppConnCreation(
    Node* node,
    NodeAddress,
    NodeAddress,
    Int32);
void STATSDB_HandleAppConnCreation(Node* node,
                                   const Address &clientAddr,
                                   const Address &serverAddr,
                                   Int32 sessionId);

void HandleStatsDBAppConnInsertion(Node* node);

void STATSDB_HandleTransConnCreation(Node* node,
                                     const Address &clientAddr,
                                     Int32 clientPort,
                                     const Address &serverAddr,
                                     Int32 serverPort);
void STATSDB_HandleMulticastConnCreation(Node *node,
    const StatsDBConnTable::MulticastConnectivity & stats);
void STATSDB_HandleMulticastConnDeletion(Node *node,
    const StatsDBConnTable::MulticastConnectivity & stats);
void STATSDB_HandleMulticastConnUpdateUpstreamInfo(Node *node,
    const StatsDBConnTable::MulticastConnectivity & stats,
    BOOL createIfNotExist = FALSE);
void HandleStatsDBTransConnInsertion(Node* node) ;

void HandleStatsDBNetworkConnInsertion(Node* firstNode);
void StatsDBHandleStatsDBPhyConnInsertion(Node* node,
      PartitionData* partition, Message *msg);
void StatsDBHandleMulticastConnInsertion(Node* node, Message *msg);
void StatsDBSendChannelNodeMsg(PartitionData * partitionData/*,
                               Message *timerMsg*/);
void StatsDBProcessAllNodeMsg(PartitionData * partition, Message *msg);
void StatsDBSendSatComNodeMsg(PartitionData * partitionData/*,
                               Message *timerMsg*/);
void StatsDBProcessSatComAllNodeMsg(
    PartitionData * partition, Message *msg);
void StatsDBSendPhyConnectivityTimerMsg(
    PartitionData *partitionData, Message *msg);
std::string STATSDB_InterfaceToString(Int32 index);
std::string STATSDB_TypeToString(BOOL isControl);
std::string STATSDB_ChannelToString(
    Node* node,
    Int32 interfaceIndex,
    Int32 channelIndex);

std::string STATSDB_Int64ToString(Int64 num);
std::string STATSDB_UInt64ToString(UInt64 num);
std::string STATSDB_IntToString(Int32 num);
std::string STATSDB_DoubleToString(double f);
// For transport summary Table

struct ltaddrpair
{
    const bool operator () (const std::pair<Address, Address> &s1,
        const std::pair<Address, Address>& s2) const
    {
        if (s1.first.networkType == NETWORK_IPV4 &&
            s2.first.networkType == NETWORK_IPV4)
        {
            if (s1.first.interfaceAddr.ipv4 != s2.first.interfaceAddr.ipv4)
            {
                return s1.first.interfaceAddr.ipv4 <
                    s2.first.interfaceAddr.ipv4;
            }else {
                if (s1.second.networkType == NETWORK_IPV4 &&
                    s2.second.networkType == NETWORK_IPV4)
                {
                    return s1.second.interfaceAddr.ipv4 <
                        s2.second.interfaceAddr.ipv4;
                }else ERROR_Assert(FALSE, "Error in stats db address comparison.");
            }
        }
        else if (s1.first.networkType == NETWORK_IPV6 && s2.first.networkType == NETWORK_IPV6)
        {
            /*return  (
                s1.first.interfaceAddr.ipv6.s6_addr32[0] < s2.first.interfaceAddr.ipv6.s6_addr32[0] || \
                s1.first.interfaceAddr.ipv6.s6_addr32[1] < s2.first.interfaceAddr.ipv6.s6_addr32[1] || \
                s1.first.interfaceAddr.ipv6.s6_addr32[2] < s2.first.interfaceAddr.ipv6.s6_addr32[2] || \
                s1.first.interfaceAddr.ipv6.s6_addr32[3] < s2.first.interfaceAddr.ipv6.s6_addr32[3] );                     */
            return FALSE;
        }
        ERROR_Assert(FALSE, "Error in ltaddrpair operator.");
        return FALSE;
    }
};

struct ltportpair
{
    const bool operator () (const std::pair<short, short> &s1,
        const std::pair<short, short>& s2) const
    {

        if (s1.first != s2.first)
        {
            return s1.first < s2.first;
        }else {
            return s1.second < s2.second;

        }

        return FALSE;
    }
};

//typedef std::multimap<std::pair<Address, Address>, TransSrcDstData *, ltaddrpair> TransSessionData;
typedef std::map<std::pair<short, short>, TransSrcDstData *, ltportpair> MapTransSrcDstData ;
typedef MapTransSrcDstData::iterator MapTransSrcDstDataIter ;
typedef std::multimap<std::pair<Address, Address>, MapTransSrcDstData *, ltaddrpair> TransSessionData;
typedef TransSessionData::iterator TransSessionDataIter;


typedef std::multimap<std::pair<Address, Address>, OneHopNetworkData *, ltaddrpair> NetworkSumAggrData;
typedef NetworkSumAggrData::iterator NetworkSumAggrDataIter;

/*Queue DB------------------------------------------------------*/
struct StatsDb;
class Queue;

using namespace std;

namespace StatsQueueDB{

string Int64ToString(Int64 num);
string IntToString(Int32 num);
string DoubleToString(double f);
string UInt64ToString(UInt64 num) ;

class QueueEventTbBuilder
{
protected:
    vector<string_pair> eventTableStr;

public:
    void InitializeQueueEventTable(PartitionData* paritionData,
        NodeInput* nodeInput);

    void BuildEvent(StatsDb* db, const vector<string> &);
};

enum StatsQueueEventType
{
    STATS_PEEK_AT_NEXT_PACKET,
    STATS_DEQUEUE_PACKET,      // 1 : Handles dequeue functionality
    STATS_DISCARD_PACKET,      // 2 : Handles drop functionality
    STATS_DROP_PACKET,
    STATS_DROP_AGED_PACKET, // handled inside queues
    STATS_QUEUE_DROP_PACKET,
    STATS_INSERT_PACKET = 1000
};

class StatsQueueEventHook
{
private:
    Queue* _queue;

protected:
    string queueType;
    Node* node;
    Int32 interfaceIndex;
    string queuePosition;

public:
    StatsQueueEventHook(
        const char t[],
        Node* node,
        Int32 intf_index,
        Queue* queuePtr,
        const char* queue_position):
        queueType(t),
        node(node),
        interfaceIndex(intf_index)
        {
            _queue = queuePtr;
            if (queue_position)
            {
                queuePosition = string(queue_position);
            }
        }

    virtual void eventHook(
        Message* msg,
        BOOL QueueIsFull,
        Int32 queuePriority,
        StatsQueueEventType eventType,
        clocktype insertTime = 0);

    ~StatsQueueEventHook () {}
};

class StatsQueueDb
{
    QueueEventTbBuilder eventBuilder;

public:

    StatsQueueDb() {};
    void STATSDB_HandleQueueEventTableUpdate(Node*node,
        const vector<string>&);

    void STATSDB_InitializeQueueEventTable(PartitionData* paritionData,
        NodeInput* nodeInput);
};
}
/*Queue DB------------------------------------------------------*/


// /**
// ENUM        :: StatsDbType
// DESCRIPTION :: Enumerates the type of statsdb protocol
// **/
typedef
enum
{
   STATSDB_MULTICAST_APP_SUMMARY,
   STATSDB_NETWORK_MULTICAST,
   STATSDB_NETWORK_SUMMARY,
   STATSDB_APP_SUMMARY,
   STATSDB_APP_AGGREGATE,
   STATSDB_PLACEHOLDER,
   STATSDB_MAC_LINK_UTILIZATION
}
StatsDbType;

//----------------------------------------------------------
// Multicast Group event structure
//----------------------------------------------------------
// /**
// STRUCT      :: GroupEventInfo
// DESCRIPTION :: This structure holds the join/leave information
//                such as the join/leave time, group joined/left,
//                and the nodeid of the member who joins/leaves.
// **/

typedef struct Group_Event_Info
{
   Int32 memberId;
   NodeAddress mcastAddr;
   clocktype joinTime;
   clocktype leaveTime;
}GroupEventInfo;

clocktype StatsDBGetTimestamp(Node* node);
void HandleMulticastGroupInfoMessage(Node* node,
                                     Message* msg);

void STATSDB_BuildMulticastEffective(Node* node,
                                     Message* msg);

void STATSDB_SendMulticastGroupInfo(Node* node,
                                    Message* msg,
                                    NodeAddress groupAddr);

Int32 STATSDB_MulticastMembershipCount(
    Node* node,
    NodeAddress groupAddr,
    AppData::APPL_MULTICAST_NODE_MAP** nodeMap);

static
void StatsDBAddMessageMsgId(Node *node, Message *msg);

void StatsDBAddMessageNextPrevHop(Node *node,
                                  Message *msg,
                                  NodeAddress nextHop,
                                  NodeAddress prevHop);

void StatsDBAddMessageAddrInfo(Node *node,
                               Message *msg,
                               NodeAddress srcAddr,
                               NodeAddress dstAddr);
//
//  This function is for message packing. Usually, the packed
//  message doesn't contain info fields of messages being packed in.
//  Calling this API will help put a copy of the address info field
//  of all messages being packed as info fields of the packed message.
void StatsDBCopyMessageAddrInfo(Node *node,
                                Message *dstMsg,
                                Message *srcMsg);


void StatsDBAddMessageMsgIdIfNone(Node *node, Message *msg);

void HandleTransportDBEvents(Node* node,
                             Message* msg,
                             Int32,
                             const std::string& eventType,
                             const Address &,
                             const Address &,
                             short,
                             short,
                             Int32,
                             char* failureType = NULL);

void HandleTransportDBEvents(Node* node,
                             Message* msg,
                             Int32 interfaceIndex,
                             const std::string& eventType,
                             unsigned short sourcePort,
                             unsigned short destPort,
                             Int32 hdrSize,
                             Int32 msgSize);

void HandleTransportDBSummary(Node* node,
                              Message* msg,
                              Int32 interfaceIndex,
                              const std::string& eventType,
                              const Address &srcAddr,
                              const Address &dstAddr,
                              short sport,
                              short dport,
                              Int32 msgSize);

void HandleTransportDBAggregate(Node* node,
                                Message* msg,
                                Int32 interfaceIndex,
                                const std::string& eventType,
                                Int32 msgSize);

void HandleMulticastDBStatus(Node* node,
                             Int32 interfaceIndex,
                             const std::string& eventType,
                             NodeAddress srcAddr,
                             NodeAddress groupAddr);

void HandleStatsDBQueueStatusInsertion(Node* node);
void InitializeStatsDBQueueStatusTable(PartitionData* partition,
                NodeInput* nodeInput);
void StatsDBInitializeMacStructure(Node*, Int32);
void STATSDB_ProcessEvent(PartitionData *partition, Message *msg);
void StatsDBInitializeNetSummaryStructure(Node *node) ;
void StatsDB_PhyRecordStartTransmittingSignal(
    Node* node,
    Int32 phyIndex,
    Message* msg) ;
void StatsDB_PROPRecordSignalRelease(
    Node* node,
    Message* msg,
    Int32 phyIndex,
    Int32 channelIndex,
    float txPower_dBm) ;

struct StatDBLinkUtilNodeInterfaceIdentifier
{
    UInt32 nodeId;
    UInt32 interfaceIndex;
};
typedef vector<StatDBLinkUtilNodeInterfaceIdentifier> vectorNodeIfId;
struct StatDBLinkUtilizationSlotDescriptor
{
    UInt32 slotId;
    std::string slotType;
    vectorNodeIfId nodeIdentifier;
    //BOOL ifCollision;

};
typedef vector<StatDBLinkUtilizationSlotDescriptor> vectorSlotDesp;
struct StatDBLinkUtilizationFrameDescriptorPerNode
{

    UInt64 frameId;//frame is unique
    //int cycleId;
    UInt32 subnetId;
    UInt32 channelIndex;
    UInt32 nodeId;
    UInt32 interfaceIndex;
    UInt32 numSlotsPerFrame;
    vectorSlotDesp slotDesp;
    UInt32 regionId;
    unsigned char macProtocol;
    //  double slotUtilization;
    //  int bytesTransmitted;

};
typedef vector<StatDBLinkUtilizationFrameDescriptorPerNode>
                vectorFrameDesp;
//vector <StatDBLinkUtilizationFrameDescriptor>* statDBGULBuffer;

struct StatsDBLinkUtilizationTable
{
    BOOL createLinkUtilizationTable;
    BOOL createWNWLinkUtilizationTable;
    BOOL createNCWLinkUtilizationTable;
    clocktype wnwLinkUtilizationTableInterval;
    clocktype ncwLinkUtilizationTableInterval;
    //new this after user turning on table.

    StatsDBLinkUtilizationTable() :
        createLinkUtilizationTable(FALSE),
        createWNWLinkUtilizationTable( FALSE),
        createNCWLinkUtilizationTable(FALSE),
        wnwLinkUtilizationTableInterval(CLOCKTYPE_MAX),
        ncwLinkUtilizationTableInterval(CLOCKTYPE_MAX)
    {

    };

};

struct StatDBLinkUtilizationFrameDescriptor
{
    UInt64 frameId;
    UInt32 channelIndex;
    UInt32 subnetIndex;
    UInt32 regionId;
    UInt32 numSlotsPerFrame;
    UInt32 numSlotsForData;
    vectorSlotDesp slotDesp;
};
typedef vector<StatDBLinkUtilizationFrameDescriptor>
                vectorLinkUtilFrameDesp;

struct StatsDBLinkUtilizationParam
{
public:

    UInt64 frameId;
    UInt32 channelIndex;
    UInt32 subnetId;
    UInt32 regionId;
    UInt64 numAllocSlots;
    UInt64 numIdleSlots;
    UInt64 numSlotsForData;
    UInt64 numAllocSlotsNotUsed;

    UInt64 minNumSlots;//the min for each db update-interval
    UInt64 maxNumSlots;//the max for each db update-interval
    UInt64 averageNumSlots;//the average for each db update-interval
    UInt64 numFrameCounted;

    StatsDBLinkUtilizationParam() :
        frameId(0),
        subnetId(0),
        channelIndex(0),
        regionId(0),
        numAllocSlots(0),
        numIdleSlots(0),
        numSlotsForData(0),
        numAllocSlotsNotUsed(0),
        minNumSlots(0),
        maxNumSlots(0),
        averageNumSlots(0),
        numFrameCounted(0)

    {

    };
};

//--------------------------------------------------------------------------
// FUNCTION:   InitializeStatsDBLinkUtilizationTable
// PURPOSE:  to create LinkUtilization table according to user input.
// PARAMETERS
// + node : Node* : Pointer to a node
// + nodeInput : NodeInput*: pointer to the node input.
// RETURN void.
//--------------------------------------------------------------------------
void
InitializeStatsDBLinkUtilizationTable(PartitionData* partition,
                NodeInput* nodeInput);

//--------------------------------------------------------------------------
// FUNCTION:   CreateStatsLinkUtilizationTable
// PURPOSE:  to create LinkUtilization Table.
// PARAMETERS
// + partitionData : PartitionData* : Pointer to partition data.
// + str : std::string* : pointer to protocol name string
// RETURN void.
//--------------------------------------------------------------------------
void
CreateDBStatsLinkUtilizationTable(PartitionData* partition,
                NodeInput* nodeInput,
                const std::string* str);
//--------------------------------------------------------------------------
// FUNCTION:  StatsDBSendLinkUtilizationTimerMessage
// PURPOSE:  to start  timer for updating LinkUtilization.
// PARAMETERS
// + partitionData : PartitionData* : Pointer to partition data.
// RETURN void.
//--------------------------------------------------------------------------
void
StatsDBSendLinkUtilizationTimerMessage(PartitionData* partition);
//--------------------------------------------------------------------------
// FUNCTION:  HandleStatsDBLinkUtilizationInsertion
// PURPOSE:   to handle link utilization insert.
// PARAMETERS
// + node: Node* : Pointer to node.
// + str : std::string* : pointer to protocol name string
// RETURN void.
//--------------------------------------------------------------------------

void
HandleStatsDBLinkUtilizationInsertion(Node* node, const std::string* str);

//--------------------------------------------------------------------------
// FUNCTION:  StatsDBSendLinkUtilizationTimerMessage
// PURPOSE:  to start  timer for updating LinkUtilization.
// PARAMETERS
// + partitionData : PartitionData* : Pointer to partition data.
// RETURN void.
//--------------------------------------------------------------------------
void
StatsDBSendLinkUtilizationTimerMessage(PartitionData* partition);

//--------------------------------------------------------------------------
// FUNCTION:  HandleStatsDBLinkUtilizationPerNodeInsertion
// PURPOSE:  insert to pernode table.
// PARAMETERS
// + node : Node* : Pointer to node
// + str  : .std::string* : pointer to string for protocol name
// RETURN void.
//--------------------------------------------------------------------------
void
HandleStatsDBLinkUtilizationPerNodeInsertion(Node*node,
                const std::string* str);

//--------------------------------------------------------------------------
// FUNCTION:  StatsDB_CheckPacketPhyDrop
// PURPOSE:  check if need to report phy drop event::
// node not on listenable channel.
// PARAMETERS
// + PartitionData : partition* : Pointer to partition
// + PropChannel : propChannel* : Pointer to the listenable channel
// + int : numNodes : number of nodes on the listenable channel
// + int : channelIndex : index of the listenable channel
// + Message : msg* : Pointer to the message
// RETURN void.
//--------------------------------------------------------------------------
void StatsDB_CheckPacketPhyDrop(
    PartitionData* partition,
    PropChannel* propChannel,
    Int32 numNodes,
    Int32 channelIndex,
    Message* msg);

void HandleStatsDBPhyConnInsertion(
    PartitionData* partitionData);

//--------------------------------------------------------------------------
// FUNCTION:  ExecuteMultipleNoReturnQueries
// PURPOSE : performs multiple no return sql statements execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + query : std::vector<std::string> : actual queries
// NOTES
// RETURN void.
//--------------------------------------------------------------------------
void ExecuteMultipleNoReturnQueries(
    StatsDb* db,
    std::vector<std::string>& insertList);

//--------------------------------------------------------------------------
// FUNCTION:  CreateTable
// PURPOSE : formulates CREATE TABLE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + name : std::string : desired name of table
// + columns : std::vector<std::pair<std::string, std::string> > : 
// +     column name and type
// + uniqueColumns: std::vector<std::string> : columns requiring unique vals
// + constraintName : optional name for a column or table constraint
// + overwrite : bool : if table name already exists, indicate 
//   whether not to overwrite
// RETURN void.
//--------------------------------------------------------------------------
void CreateTable(
    StatsDb* db,
    const std::string& name,
    const DBColumns& columns,
    const std::vector<std::string>& uniqueColumns,
    const std::string& constraintName = "",
    bool overwrite = false);
 
//--------------------------------------------------------------------------
// FUNCTION:  CreateTable
// PURPOSE : formulates CREATE TABLE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + name : std::string : desired name of table
// + columns : std::vector<std::pair<std::string, std::string> > : 
// +     column name and type
// + constraintName : optional name for a column or table constraint
// + overwrite : bool : if table name already exists, indicate 
//   whether not to overwrite
// RETURN void.
//--------------------------------------------------------------------------
void CreateTable(
    StatsDb* db,
    const std::string& name,
    const DBColumns& columns,
    const std::string& constraintName = "",
    bool overwrite = false);

//--------------------------------------------------------------------------
// FUNCTION:  DropTable
// PURPOSE : formulates DROP TABLE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + name : std::string : name of desired table to drop
// RETURN void.
//--------------------------------------------------------------------------
void DropTable(
    StatsDb* db,
    const std::string& name);

//--------------------------------------------------------------------------
// FUNCTION:  AddColumn
// PURPOSE : formulates ALTER TABLE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of table
// + name : std::string : desired name of column
// + valueType : std::string : desired value type of the column
// RETURN void.
//--------------------------------------------------------------------------
void AddColumn(
    StatsDb* db,
    const std::string& table,
    const std::string& name,
    const std::string& valueType);

//--------------------------------------------------------------------------
// FUNCTION:  GetInsertValuesSQL
// PURPOSE : formulates INSERT SQL statement
// PARAMETERS
// + table : std::string : name of desired table to perform select on
// + columns : std::vector<std::string> : column names of data that
//      you want insert
// + values : std::vector<std::string> : column values data that
//      you want insert
// RETURN 
// + std:string : full query for the insert statement
//--------------------------------------------------------------------------
std::string GetInsertValuesSQL(
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& newValues);

//--------------------------------------------------------------------------
// FUNCTION:  InsertValues
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of desired table to perform select on
// + columns : std::vector<std::string> : column names of data that
//      you want insert
// + values : std::vector<std::string> : column values data that
//      you want insert
// RETURN void.
//--------------------------------------------------------------------------
void InsertValues(
    StatsDb* db,
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& newValues);

//--------------------------------------------------------------------------
// FUNCTION:  InsertValues
// PURPOSE : formulates UPDATE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of desired table to perform update on
// + columns : std::vector<std::string> : column names you are updating
// + values : std::vector<std::string> : the new desired values
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN void.
//--------------------------------------------------------------------------
void UpdateValues(
    StatsDb* db,
    const std::string &table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& values,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  GetUpdateValuesSQL
// PURPOSE : formulates UPDATE SQL statement
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of desired table to perform update on
// + columns : std::vector<std::string> : column names you are updating
// + values : std::vector<std::string> : the new desired values
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN 
// + std:string : full query for the update statement
//--------------------------------------------------------------------------
std::string GetUpdateValuesSQL(
    const std::string &table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& values,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  DeleteValues
// PURPOSE : formulates DELETE query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of desired table to drop
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN void.
//--------------------------------------------------------------------------
void DeleteValues(
    StatsDb* db,
    const std::string &table,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  Select
// PARAMETERS
// PURPOSE : formulates SELECT query and performs statement execution
// + db : StatsDb* : Pointer to the database
// + table : std::string : name of desired table to perform select on
// + columns : std::vector<std::string> : column names of data that
//      you want returned
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN void.
//--------------------------------------------------------------------------
std::string Select(
    StatsDb* db,
    const std::string &table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  GetSelectSQL
// PURPOSE : formulates SELECT query
// PARAMETERS
// + table : std::string : name of desired table to perform select on
// + columns : std::vector<std::string> : column names of data that
//      you want returned
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN
// + std::string : full query for the select statement
//--------------------------------------------------------------------------
std::string GetSelectSQL(
    const std::string &table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  Select
// PURPOSE : performs SELECT query
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + query : std::string : actual full query you want to perform
// RETURN
// + std::string : returned data from select query
//--------------------------------------------------------------------------
std::string Select(
    StatsDb* db,
    const std::string& query);

//--------------------------------------------------------------------------
// FUNCTION:  CreateIndex
// PURPOSE : formulates CREATE INDEX query and performs statement execution
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + table : std::string : which table to use in the index
// + name : std::string : desired name of index
// + columns : std::vector<std::string> : which columns to use in the index
// RETURN void.
//--------------------------------------------------------------------------
void CreateIndex(
    StatsDb* db,
    const std::string& name,
    const std::string& table,
    const std::vector<std::string>& columns);

//--------------------------------------------------------------------------
// FUNCTION:  GatherWhereClause
// PURPOSE : formulates WHERE clause to be added to another query
// PARAMETERS
// + qualifierColumns : std::vector<std::string> : column names for 
//      qualifying check
// + qualifierValues : std::vector<std::string> : column values for 
//      qualifying check
// + joinOperator : std::string : logical join operator to use for
//      qualifying check
// RETURN
// + std::string : partial query for the where clause of a statement
//--------------------------------------------------------------------------
std::string GatherWhereClause(
    const std::vector<std::string>& qualifierColumns,
    const std::vector<std::string>& qualifierValues,
    const std::string& joinOperator = "AND");

//--------------------------------------------------------------------------
// FUNCTION:  Synch
// PURPOSE : flushes stored buffer of queries to perform later by executing
//      them all in a single statement; an attempt to minimize number of 
//      lock acquistions
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// RETURN void.
//--------------------------------------------------------------------------
void Synch(StatsDb* db);

//--------------------------------------------------------------------------
// FUNCTION:  ParallelBarrier
// PURPOSE : forces synchronization of all partitions before proceeding
// PARAMETERS
// + partitionData : PartitionData* : Pointer to the partition
// RETURN void.
//--------------------------------------------------------------------------
void ParallelBarrier(PartitionData* partition);

//--------------------------------------------------------------------------
// FUNCTION:  SynchPlusParallelBarrier
// PURPOSE : combination call for Synch and ParallelBarrier  
// PARAMETERS
// + partitionData : PartitionData* : Pointer to the partition
// RETURN void.
//--------------------------------------------------------------------------
void SynchPlusParallelBarrier(PartitionData* partition);

//--------------------------------------------------------------------------
// FUNCTION:  UnlockTables
// PURPOSE : formulates UNLOCK query and performs statement execution
// PARAMETERS
// + partitionData : PartitionData* : Pointer to the partition
// RETURN void.
//--------------------------------------------------------------------------
void UnlockTables(PartitionData* partition);

//--------------------------------------------------------------------------
// FUNCTION:  LockTable
// PURPOSE : formulates LOCK query statement and performs statement execution
// PARAMETERS
// + tableName : std::string : name of db table you wish to lock
// + partitionData : PartitionData* : Pointer to the partition
// + read : bool : true if a read lock, false if a write lock
// RETURN void.
//--------------------------------------------------------------------------
void LockTable(const std::string& tableName, PartitionData* partition, bool read);

//--------------------------------------------------------------------------
// FUNCTION:  AddInsertQueryToBuffer
// PURPOSE : adds query to a stored buffer of queries to perform later,
//  an attempt to minimize number of lock acquistions
// PARAMETERS
// + db : StatsDb* : Pointer to the database
// + query : std::string : full query
// RETURN void.
//--------------------------------------------------------------------------
void AddInsertQueryToBuffer(StatsDb* db, 
                      const std::string& query);

struct StatsDBAppSummaryParam;   // Required forward definition!
BOOL HandleAppSummaryRetrieveDatabase(
    Node* node,
    StatsDb* db,
    StatsDBAppSummaryParam* appParam,
    std::string query);

#endif // _DB_H_
