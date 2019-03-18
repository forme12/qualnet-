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

#ifndef _DBAPI_H_
#define _DBAPI_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "fileio.h"
#include "node.h"
#include "db.h"

#ifdef ENTERPRISE_LIB
#include "db_multimedia.h"
#endif
#include "db_developer.h"
#ifdef ADDON_BOEINGFCS
#include "db_ces_malsr.h"
#include "db_ces_network_region.h"
#include "db_ces_mac_usap_dba.h"
#include "db_ces_link_adaptation.h"
#include "db_ces_network_mimdl.h"
#include "db_mac_wnw_dynamic_bandwidth_allocation.h"
#include "db_mac_wnw_queue_management.h"
#include "db_ces_mpr.h"
#include "db_srw.h"
#endif // ADDON_BOEINGFCS

#include "gestalt.h"
#include "db-core.h"

#define STATSDB_MIN_BUFFER_QUERY 10
#define STATSDB_MAX_BUFFER_QUERY 100
#define LARGE_INDEX 99999

#define STATSDB_DEFAULT_AGGREGATE_INTERVAL (600 * SECOND)
#define STATSDB_DEFAULT_SUMMARY_INTERVAL (600 * SECOND)
#define STATSDB_DEFAULT_STATUS_INTERVAL (600 * SECOND)

#define STATS_DEBUG_LOCK 0
typedef UInt8 STATSDB_TABLE_TYPE;

#define STATSDB_DESCRIPTION_TABLE 1
#define STATSDB_NODE_DESCRIPTION_TABLE 2
#define STATSDB_QUEUE_DESCRIPTION_TABLE 3
#define STATSDB_SCHEDULER_DESCRIPTION_TABLE 4
#define STATSDB_SESSION_DESCRIPTION_TABLE 5
#define STATSDB_CONNECTION_DESCRIPTION_TABLE 6
#define STATSDB_INTERFACE_DESCRIPTION_TABLE 7
#define STATSDB_PHY_DESCRIPTION_TABLE 8

#define STATSDB_STATUS_TABLE 9

#define STATSDB_AGGREGATE_TABLE 14
#define STATSDB_APP_AGGREGATE_TABLE 15
#define STATSDB_TRANS_AGGREGATE_TABLE 17
#define STATSDB_NETWORK__AGGREGATE_TABLE 18
#define STATSDB_MAC_AGGREGATE_TABLE 19
#define STATSDB_PHY_AGGREGATE_TABLE 20

#define STATSDB_SUMMARY_TABLE 21

#define STATSDB_EVENTS_TABLE 26
#define STATSDB_APP_EVENTS_TABLE 27
#define STATSDB_TRANS_EVENTS_TABLE 29
#define STATSDB_NETWORK_EVENTS_TABLE 30
#define STATSDB_MAC_EVENTS_TABLE 31
#define STATSDB_PHY_EVENTS_TABLE 32
#define STATSDB_QUEUE_EVENTS_TABLE 33

#define STATSDB_CONNECTIVITY_TABLE 34
#define STATSDB_APP_CONNECTIVITY_TABLE 35
#define STATSDB_TRANSPORT_CONNECTIVITY_TABLE 36
#define STATSDB_NETWORK_CONNECTIVITY_TABLE 37
#define STATSDB_MAC_CONNECTIVITY_TABLE 38
#define STATSDB_PHY_CONNECTIVITY_TABLE 39
#define STATSDB_MULTICAST_CONNECTIVITY_TABLE 40

typedef UInt8 STATSDB_DescriptionTableNum;

#define STATSDB_NODE_DESCRIPTION_TABLE_NUMBER 1
#define STATSDB_QUEUE_DESCRIPTION_TABLE_NUMBER 2
#define STATSDB_SCHEDULER_DESCRIPTION_TABLE_NUMBER 3
#define STATSDB_SESSION_DESCRIPTION_TABLE_NUMBER 4
#define STATSDB_CONNECTION_DESCRIPTION_TABLE_NUMBER 5
#define STATSDB_INTERFACE_DESCRIPTION_TABLE_NUMBER 6
#define STATSDB_PHY_DESCRIPTION_TABLE_NUMBER 7

typedef UInt8 STATSDB_TableCategory;

#define STATSDB_DESCRIPTION_CATEGORY 1
#define STATSDB_STATUS_CATEGORY 2
#define STATSDB_AGGREGATE_CATEGORY 3
#define STATSDB_SUMMARY_CATEGORY 4
#define STATSDB_EVENTS_CATEGORY 5
#define STATSDB_CONNECTIVITY_CATEGORY 6

typedef UInt8 STATSDB_AggregateTableNum;

#define STATSDB_APP_AGGREGATE_TABLE_NUMBER 1
#define STATSDB_TRANS_AGGREGATE_TABLE_NUMBER 3
#define STATSDB_NETWORK_AGGREGATE_TABLE_NUMBER 4
#define STATSDB_MAC_AGGREGATE_TABLE_NUMBER 5
#define STATSDB_PHY_AGGREGATE_TABLE_NUMBER 6
#define STATSDB_QUEUE_AGGREGATE_TABLE_NUMBER 7

typedef UInt8 STATSDB_SummaryTableNum;

#define STATSDB_APP_SUMMARY_TABLE_NUMBER 1
#define STATSDB_TRANS_SUMMARY_TABLE_NUMBER 3
#define STATSDB_NETWORK_SUMMARY_TABLE_NUMBER 4
#define STATSDB_MAC_SUMMARY_TABLE_NUMBER 5
#define STATSDB_PHY_SUMMARY_TABLE_NUMBER 6
#define STATSDB_QUEUE_SUMMARY_TABLE_NUMBER 7

typedef UInt8 STATSDB_EventsTableNum;

#define STATSDB_APP_EVENTS_TABLE_NUMBER 1
#define STATSDB_TRANS_EVENTS_TABLE_NUMBER 3
#define STATSDB_NETWORK_EVENTS_TABLE_NUMBER 4
#define STATSDB_MAC_EVENTS_TABLE_NUMBER 5
#define STATSDB_PHY_EVENTS_TABLE_NUMBER 6
#define STATSDB_QUEUE_EVENTS_TABLE_NUMBER 7

typedef UInt8 STATSDB_CONNECTIVITY_TABLE_NUMBER;

#define STATSDB_APP_CONNECTIVITY_TABLE_NUMBER 1
#define STATSDB_TRANSPORT_CONNECTIVITY_TABLE_NUMBER 2
#define STATSDB_NETWORK_CONNECTIVITY_TABLE_NUMBER 3
#define STATSDB_MAC_CONNECTIVITY_TABLE_NUMBER 4
#define STATSDB_PHY_CONNECTIVITY_TABLE_NUMBER 5
#define STATSDB_MULTICAST_CONNECTIVITY_TABLE_NUMBER 6

typedef UInt8 STATSDB_StatusTableNum;

#define STATSDB_NODE_STATUS_TABLE_NUMBER 1
#define STATSDB_INTERFACE_STATUS_TABLE_NUMBER 2
#define STATSDB_MULTICAST_STATUS_TABLE_NUMBER 3
#define STATSDB_QUEUE_STATUS_TABLE_NUMBER 4

#define STATSDB_ADMIN_DISTANCE 1
#define STATSDB_APPLICATION_TYPE 2
#define STATSDB_CHANNEL_INDEX 3
#define STATSDB_CONNECTION_ID 4
#define STATSDB_CONNECTION_TYPE 5
#define STATSDB_CONTROL_SIZE 6
#define STATSDB_DELAY 7
#define STATSDB_DEST_NETWORK_MASK 8
#define STATSDB_EVENT_TYPE 9
#define STATSDB_FRAGMENT_ID 10
#define STATSDB_HOP_COUNT 11
#define STATSDB_INTERFACE_INDEX 12
#define STATSDB_INTERFACE_ADDRESS 13
#define STATSDB_INTERFACE_NAME 14
#define STATSDB_INTERFERENCE_POWER 15
#define STATSDB_JITTER 16
#define STATSDB_MAC_PROTOCOL 17
#define STATSDB_FAILURE_TYPE 18
#define STATSDB_MESSAGE_TYPE 19
#define STATSDB_MSG_SEQUENCE_NUM 20
#define STATSDB_META_DATA 21
#define STATSDB_NETWORK_PROTOCOL 22
#define STATSDB_NETWORK_TYPE 23
#define STATSDB_NEXT_HOP_ADDRESS 24
#define STATSDB_OUTGOING_INTERFACE_INDEX 25
#define STATSDB_PATHLOSS 26
#define STATSDB_PHY_INDEX 27
#define STATSDB_PRIORITY 28
#define STATSDB_PROTOCOL_TYPE 29
#define STATSDB_QUEUE_INDEX 30
#define STATSDB_QUEUE_DISCIPLINE 31
#define STATSDB_RECEIVER_ADDRESS 32
#define STATSDB_RECEIVER_PORT 33
#define STATSDB_ROUTING_PROTOCOL_TYPE 34
#define STATSDB_SENDER_ADDRESS 35
#define STATSDB_SENDER_PORT 36
#define STATSDB_SERVICE_TIME 37
#define STATSDB_SESSION_ID 38
#define STATSDB_SIGNAL_POWER 39
#define STATSDB_SIZE 40
#define STATSDB_SUBNET_MASK 41
#define STATSDB_TRANSPORT_PROTOCOL 42

#define STATSDB_MULTICAST_PROTOCOL 43
#define STATSDB_NODE_POSITION 44
#define STATSDB_NODE_VELOCITY 45
#define STATSDB_NODE_ACTIVE_STATE 46
#define STATSDB_NODE_DAMAGE_STATE 47
#define STATSDB_QUEUE_PRIORITY 48
#define STATSDB_QUEUE_TYPE 49
#define STATSDB_DATA_RETRAX 50
#define STATSDB_CONTROL_RETRAX 51
#define STATSDB_HEADER_FLAGS 52
#define STATSDB_TRANSPORT_SEQ_NUM 53

typedef UInt8 STATSDB_MetaDataType;

#define STATSDB_NodeMetaData 1
#define STATSDB_QueueMetaData 2
#define STATSDB_SchedulerMetaData 3
#define STATSDB_SessionMetaData 4
#define STATSDB_ConnectionMetaData 5
#define STATSDB_InterfaceMetaData 6
#define STATSDB_PhyMetaData 7

#define STATS_DEBUG 0

#ifdef ADDON_NGCNMS
class StatsDBAppSummaryParam ;
#endif

struct StatsDb
{
    BOOL createDbFile;
    UTIL::Database::DatabaseDriver* driver;
    UTIL::Database::dbEngineType engineType;
    char statsDatabase[MAX_STRING_LENGTH];
    StatsDBLevelSetting levelSetting;
    StatsDBTable* statsTable;
    StatsDBDescTable* statsDescTable;
    StatsDBInterfaceDescContent* statsInterfaceDesc;
    StatsDBQueueDescContent* statsQueueDesc;
    StatsDBSessionDescContent *statsSessionDesc;
    StatsDBConnectionDescContent *statsConnectionDesc;
    StatsDBSchedulerDescContent *statsSchedulerAlgo;

    StatsDBAggregateTable* statsAggregateTable;
    StatsDBAppAggregateContent* statsAppAggregate;
    StatsDBNetworkAggregateContent* statsNetAggregate;
    StatsDBMacAggregateContent* statsMacAggregate;
    StatsDBPhyAggregateContent* statsPhyAggregate;
    StatsDBTransportAggregateContent* statsTransAggregate;

    StatsDBSummaryTable* statsSummaryTable;
    StatsDBAppSummaryContent* statsAppSummary;
    StatsDBMulticastAppSummaryContent* statsMulticastAppSummary;
    StatsDBTransSummaryContent* statsTransSummary;

    StatsDBNetworkSummaryContent* statsNetSummary;
    StatsDBMacSummaryContent* statsMacSummary;
    StatsDBPhySummaryContent* statsPhySummary;

    StatsDBStatusTable* statsStatusTable;
    StatsDBNodeStatusContent* statsNodeStatus;

    StatsDBEventsTable* statsEventsTable;
    StatsDBAppEventContent* statsAppEvents;
    StatsDBNetworkEventContent* statsNetEvents;
    StatsDBMacEventContent* statsMacEvents;
    StatsDBPhyEventContent* statsPhyEvents;
    StatsDBTransEventContent* statsTransEvents;
    StatsDBConnTable* statsConnTable;
    StatsDBNetworkConnContent* statsNetConn;
    StatsDBMacConnContent* statsMacConn;
    StatsDBPhyConnContent* statsPhyConn;
    StatsDBLinkUtilizationTable* StatsDBLinkUtilTable;

    Int32 numQueryBuffer;
    Int32 maxQueryBuffer;
    Int32 minQueryBuffer;
    std::string queryBuffer; //Buffer for sqlite queries - can be executed all at once
    std::list<std::string> buffer;  //Buffer for mysql queries - need to be executed individually

    /*---------------------------------*/
    StatsDb(): queueDbPtr(0), networkEventsBytesUsed(0), appEventsBytesUsed(0), engineType(UTIL::Database::dbSqlite)
    {
        networkEventsString = NULL;
    }
    ~StatsDb()
    {
        if (networkEventsString) 
        {
            MEM_free(networkEventsString);
        }
    }
    StatsQueueDB::StatsQueueDb* queueDbPtr;
    /*---------------------------------*/

    // Stores input from metadata file if one exists
    NodeInput metaDataInput;

#ifdef ENTERPRISE_LIB
    // Protocol Specific Code
    StatsDBOspfTable* statsOspfTable;
    StatsDBPimTable* statsPimTable;
#endif
    StatsDBIgmpTable* statsIgmpTable;
#ifdef ADDON_BOEINGFCS
    // Protocol Specific Code
    StatsDBRegionTableParams* statsRegionTableParams;
    StatsDBRapTableParams* statsRapTableParams;
    StatsDBMalsrTableParams* statsMalsrTableParams;
    StatsDBUsapCNTableParams* statsUsapCNTableParams;
    StatsDBUsapFDTableParams* statsUsapFDTableParams;
    StatsDBLinkAdaptationTableParams* statsLinkAdapTableParams;
    StatsDBMiMdlEventTableParams* statsMiMdlEventTableParams;
    StatsDBMiMdlPushPacketEventTableParams* statsMiMdlPushPktEventTableParams;
    StatDBUsapDbaEventTableParam* statsUsapDbaEventTableParams;
    StatDBMdlQueueStatusTableParam* statsMdlQueueStatsTableParams;
    StatsDBMprSetTableParams* statsMprSetTableParams;
    StatsDBSrwTableContainer* statsSrwTableContainer;
#endif // ADDON_BOEINGFCS

#ifdef ADDON_NGCNMS
    struct SenderSessionComparator
    {
        bool operator() (std::pair<Int32, Int32> p1,
                         std::pair<Int32, Int32> p2) const
        {
            bool retVal = FALSE;
            if (p1.first < p2.first)
            {
                retVal = TRUE;
            }
            else if ((p1.first == p2.first) && (p1.second < p2.second))
            {
                retVal = TRUE;
            }

            return retVal;
        }
    };

    // key - pair<senderId, sessionId>
    // value - sender's app summary param
    map<pair<Int32, Int32>, StatsDBAppSummaryParam,
    SenderSessionComparator > appSummaryNodePairList ;
    typedef map<pair<Int32, Int32>, StatsDBAppSummaryParam,
    SenderSessionComparator >::iterator ITER ;
#endif

    Int32 networkEventsBytesUsed;
    char* networkEventsString;

    Int32 appEventsBytesUsed;
    std::string appEventsString;

};

//--------------------------------------------------------------------//
// CLASS    : MetaDataStruct
// PURPOSE  : This object is used for recording user-defined columns
//            and their values for insertion into the stats database
//--------------------------------------------------------------------//
class MetaDataStruct
{
public:
    std::map<std::string, std::string> m_MetaData;

    void AddNodeMetaData(Node* node,
                         PartitionData* partition,
                         NodeInput* input);
    /*void AddInterfaceMetaData(Node* node,
                         PartitionData* partition,
                         const NodeInput* input);*/
    void AddInterfaceMetaData(Node* node,
                              Int32 interfaceIndex,
                              PartitionData* partition,
                              const NodeInput* nodeInput);
    void AddQueueMetaData(Node* node,
                          PartitionData* partition,
                          const NodeInput* input);
    void AddSchedulerMetaData(Node* node,
                              PartitionData* partition,
                              const NodeInput* input);

    void AddSessionMetaData(Node* node,
                            PartitionData* partition,
                            const NodeInput* nodeInput);

    void AddConnectionMetaData(Node* node,
                               PartitionData* partition,
                               const NodeInput* nodeInput);

    /*void AddPhyMetaData(Node* node,
            PartitionData* partition,
            const NodeInput* nodeInput);*/

    void AddPhyMetaData(Node* node,
                        Int32 interfaceIndex,
                        PartitionData* partition,
                        const NodeInput* nodeInput);
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBQueueDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the QUEUE_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBQueueDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    Int32 m_QueueIndex;
    // What the queue is for, e.g. network input, mac output, etc.
    std::string m_QueueType;

    std::string m_QueueDiscipline;
    BOOL m_QueueDisciplineSpecified;

    Int32 m_QueuePriority;
    BOOL m_QueuePrioritySpecified;

    Int32 m_QueueSize;
    BOOL m_QueueSizeSpecified;

    MetaDataStruct m_QueueMetaData;

    StatsDBQueueDesc(Int32 id,
                     Int32 index,
                     Int32 queueIndex,
                     std::string queueType);
    void SetQueueDiscipline(std::string discipline);
    void SetQueuePriority(Int32 priority);
    void SetQueueSize(Int32 size);
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBSchedulerDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the SCHEDULER_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBSchedulerDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    std::string m_SchedulerType;
    std::string m_SchedulingAlgorithm;
    MetaDataStruct m_SchedulerMetaData;

    StatsDBSchedulerDesc(Int32 nodeId,
                         Int32 index,
                         std::string type,
                         std::string algorithm);
};

class StatsDBSessionDesc
{
public:
    Int32 m_SessionId;
    Int32 m_SenderId;
    Int32 m_ReceiverId;
    std::string m_AppType;
    BOOL m_AppTypeSpecified;
    std::string m_SenderAddr;
    BOOL m_SenderAddrSpecified;

    std::string m_ReceiverAddr;
    BOOL m_ReceiverAddrSpecified;

    short m_SenderPort;
    BOOL m_SrcPortSpecified;

    short m_RecvPort;
    BOOL m_RecvPortSpecified;

    std::string m_TransportProtocol;
    BOOL m_TransportPortocolSpecified;

    MetaDataStruct m_SessionMetaData;

    StatsDBSessionDesc(Int32 sessionId,
                       Int32 senderId,
                       Int32 receiverId); // not unique...

    void SetSenderAddr(std::string sdrAddr);
    void SetReceiverAddr(std::string recvAddr);
    void SetSenderPort(short sdrPort);
    void SetRecvPort(short recvPort);
    void SetTransportProtocol (const std::string &protocol);
    void SetAppType(std::string AppType);//Bug95
};

class StatsDBConnectionDesc
{
public:
    double m_timeValue;
    Int32 m_SenderId;
    Int32 m_ReceiverId;

    std::string m_SenderAddr;
    BOOL m_SenderAddrSpecified;

    std::string m_ReceiverAddr;
    BOOL m_ReceiverAddrSpecified;

    short m_SenderPort;
    BOOL m_SrcPortSpecified;

    short m_RecvPort;
    BOOL m_RecvPortSpecified;

    std::string m_ConnectionType;
    BOOL m_ConnTypeSpecified;

    std::string m_NetworkProtocol;
    BOOL m_NetworkPortocolSpecified;

    MetaDataStruct m_ConnectionMetaData;

    StatsDBConnectionDesc(double, Int32, Int32);

    void SetSenderAddr(std::string sdrAddr);
    void SetReceiverAddr(std::string recvAddr);
    void SetSenderPort(short sdrPort);
    void SetRecvPort(short recvPort);
    void SetConnectionType(std::string connType);
    void SetNetworkProtocol (std::string protocol);
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBInterfaceDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the INTERFACE_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBInterfaceDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;

    std::string m_InterfaceName;
    BOOL m_InterfaceNameSpecified;

    std::string m_InterfaceAddr;
    BOOL m_InterfaceAddrSpecified;

    std::string m_SubnetMask;
    BOOL m_SubnetMaskSpecified;

    std::string m_NetworkType;
    BOOL m_NetworkTypeSpecified;

    std::string m_MulticastProtocol;
    BOOL m_MulticastProtocolSpecified;

    Int32 m_SubnetId;
    BOOL m_SubetIdSpecififed;

    MetaDataStruct m_InterfaceMetaData;

    StatsDBInterfaceDesc(Int32 nodeId,
                         Int32 interfaceIndex);

    void SetInterfaceName(std::string name);
    void SetInterfaceAddr(std::string addr);
    void SetSubnetMask(std::string mask);
    void SetNetworkType(std::string type);
    void SetMulticastProtocol(std::string protocol);
    void SetSubnetId(Int32 id);
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyDesc
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Description table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyDesc
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;
    Int32 m_PhyIndex;
    MetaDataStruct m_PhyMetaData;

    StatsDBPhyDesc(Int32 nodeId,
                   Int32 interfaceIndex,
                   Int32 phyIndex);
};
class StatsDBAppAggregateParam
{
public:
    UInt64 m_UnicastMessageSent;
    UInt64 m_UnicastMessageRecd;
    UInt64 m_MulticastMessageSent;
    UInt64 m_EffMulticastMessageSent;
    UInt64 m_MulticastMessageRecd;
    UInt64 m_UnicastByteSent;
    UInt64 m_UnicastByteRecd;
    UInt64 m_UnicastFragmentSent;
    UInt64 m_UnicastFragmentRecd;

    UInt64 m_MulticastByteSent;
    UInt64 m_EffMulticastByteSent;
    UInt64 m_MulticastByteRecd;
    UInt64 m_MulticastFragmentSent;
    UInt64 m_EffMulticastFragmentSent;
    UInt64 m_MulticastFragmentRecd;

    double m_UnicastMessageCompletionRate;
    double m_MulticastMessageCompletionRate;
    double m_UnicastOfferedLoad;
    double m_UnicastThroughput;
    double m_MulticastOfferedLoad;
    double m_MulticastThroughput;

    double m_UnicastDelay;
    BOOL m_UnicastDelaySpecified;

    double m_UnicastJitter;
    BOOL m_UnicastJitterSpecified;

    double m_MulticastDelay;
    BOOL m_MulticastDelaySpecified;

    double m_MulticastJitter;
    BOOL m_MulticastJitterSpecified;

    double m_UnicastHopCount;
    BOOL m_UnicastHopCountSpecified;

    double m_MulticastHopCount;
    BOOL m_MulticastHopCountSpecified;

    StatsDBAppAggregateParam();

    void SetUnicastDelay(double delay);
    void SetMulticastDelay(double delay);
    void SetUnicastJitter(double jitter);
    void SetMulticastJitter(double jitter);
    void SetUnicastHopCount(double hopCount);
    void SetMulticastHopCount(double hopCount);

};

class StatsDBTransAggregateParam
{
public:
    UInt64 m_UnicastSegmentSentToApp;
    UInt64 m_MulticastSegmentSentToApp;
    UInt64 m_BroadcastSegmentSentToApp;
    
    UInt64 m_UnicastSegmentSentToNet;
    UInt64 m_MulticastSegmentSentToNet;
    UInt64 m_BroadcastSegmentSentToNet;

    UInt64 m_UnicastByteSentToApp;
    UInt64 m_MulticastByteSentToApp;
    UInt64 m_BroadcastByteSentToApp;

    UInt64 m_UnicastByteSentToNet;
    UInt64 m_MulticastByteSentToNet;
    UInt64 m_BroadcastByteSentToNet;

    double m_UnicastOfferedLoad;
    double m_MulticastOfferedLoad;
    double m_BroadcastOfferedLoad;

    double m_UnicastThroughput;
    double m_MulticastThroughput;
    double m_BroadcastThroughput;

    double m_UnicastAverageDelay;
    double m_MulticastAverageDelay;
    double m_BroadcastAverageDelay;    
    BOOL UnicastAverageDelaySpecified;
    BOOL MulticastAverageDelaySpecified;
    BOOL BroadcastAverageDelaySpecified;
    double m_UnicastAverageJitter;
    double m_MulticastAverageJitter;
    double m_BroadcastAverageJitter;    
    BOOL UnicastAverageJitterSpecified;
    BOOL MulticastAverageJitterSpecified;
    BOOL BroadcastAverageJitterSpecified;
    
    double totalJitter;

    void SetUnicastAveDelay(double delay)
    {
        m_UnicastAverageDelay = delay;
        UnicastAverageDelaySpecified = TRUE;
    }

    void SetUnicastAveJitter(double jitter)
    {
        m_UnicastAverageJitter = jitter;
        UnicastAverageJitterSpecified = TRUE;
    }
    
    void SetMulticastAveDelay(double delay)
    {
        m_MulticastAverageDelay = delay;
        MulticastAverageDelaySpecified = TRUE;
    }

    void SetMulticastAveJitter(double jitter)
    {
        m_MulticastAverageJitter = jitter;
        MulticastAverageJitterSpecified = TRUE;
    }

    void SetBroadcastAveDelay(double delay)
    {
        m_BroadcastAverageDelay = delay;
        BroadcastAverageDelaySpecified = TRUE;
    }

    void SetBroadcastAveJitter(double jitter)
    {
        m_BroadcastAverageJitter = jitter;
        BroadcastAverageJitterSpecified = TRUE;
    }

    StatsDBTransAggregateParam()
    {
        m_UnicastSegmentSentToApp = 0;
        m_MulticastSegmentSentToApp = 0;
        m_BroadcastSegmentSentToApp = 0;
        
        m_UnicastSegmentSentToNet = 0;
        m_MulticastSegmentSentToNet = 0;
        m_BroadcastSegmentSentToNet = 0;

        m_UnicastByteSentToApp = 0;
        m_MulticastByteSentToApp = 0;
        m_BroadcastByteSentToApp = 0;

        m_UnicastByteSentToNet = 0;
        m_MulticastByteSentToNet = 0;
        m_BroadcastByteSentToNet = 0;
        
        m_UnicastOfferedLoad = 0;
        m_MulticastOfferedLoad = 0;
        m_BroadcastOfferedLoad = 0;
        m_UnicastThroughput = 0;
        m_MulticastThroughput = 0;
        m_BroadcastThroughput = 0;

        m_UnicastAverageDelay = 0;
        m_MulticastAverageDelay = 0;
        m_BroadcastAverageDelay = 0;    
        UnicastAverageDelaySpecified = FALSE;
        MulticastAverageDelaySpecified = FALSE;
        BroadcastAverageDelaySpecified = FALSE;
        
        m_UnicastAverageJitter = 0;
        m_MulticastAverageJitter = 0;
        m_BroadcastAverageJitter = 0;    
        UnicastAverageJitterSpecified = FALSE;
        MulticastAverageJitterSpecified = FALSE;
        BroadcastAverageJitterSpecified = FALSE;
        
        totalJitter = 0;
    }
};
class StatsDBNetworkAggregateParam
{
public:
    UInt64 m_UDataPacketsSent;
    UInt64 m_UDataPacketsRecd;
    UInt64 m_UDataPacketsForward;
    UInt64 m_UControlPacketsSent;
    UInt64 m_UControlPacketsRecd;
    UInt64 m_UControlPacketsForward;

    UInt64 m_MDataPacketsSent;
    UInt64 m_MDataPacketsRecd;
    UInt64 m_MDataPacketsForward;
    UInt64 m_MControlPacketsSent;
    UInt64 m_MControlPacketsRecd;
    UInt64 m_MControlPacketsForward;

    UInt64 m_BDataPacketsSent;
    UInt64 m_BDataPacketsRecd;
    UInt64 m_BDataPacketsForward;
    UInt64 m_BControlPacketsSent;
    UInt64 m_BControlPacketsRecd;
    UInt64 m_BControlPacketsForward;

    UInt64 m_UDataBytesSent;
    UInt64 m_UDataBytesRecd;
    UInt64 m_UDataBytesForward;
    UInt64 m_UControlBytesSent;
    UInt64 m_UControlBytesRecd;
    UInt64 m_UControlBytesForward;

    UInt64 m_MDataBytesSent;
    UInt64 m_MDataBytesRecd;
    UInt64 m_MDataBytesForward;
    UInt64 m_MControlBytesSent;
    UInt64 m_MControlBytesRecd;
    UInt64 m_MControlBytesForward;

    UInt64 m_BDataBytesSent;
    UInt64 m_BDataBytesRecd;
    UInt64 m_BDataBytesForward;
    UInt64 m_BControlBytesSent;
    UInt64 m_BControlBytesRecd;
    UInt64 m_BControlBytesForward;

    vector<double> m_CarrierLoad;

    vector<double> m_Delay;
    vector<BOOL> m_DelaySpecified;

    vector<double> m_Jitter;    // avgJitter for sequential
    vector<BOOL> m_JitterSpecified;

    vector<Int32> m_ipOutNoRoutes;
    vector<BOOL> m_ipOutNoRoutesSpecified;

    vector<double> m_totalJitter ; // jitter information for parallel
    vector<UInt64> m_jitterDataPoints;
    vector<BOOL> m_jitterDataPointsSpecified ;

    StatsDBNetworkAggregateParam();

    void SetDelay(double delay,
                  StatsDBNetworkAggregateContent::NetAggrTrafficType);
    void SetJitter(double jitter,
                   StatsDBNetworkAggregateContent::NetAggrTrafficType);
    void SetIpOutNoRoutes(
        Int32 ipOutNoRoutes,
        StatsDBNetworkAggregateContent::NetAggrTrafficType);

};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyAggregateParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Aggregate table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyAggregateParam
{
public:
    BOOL toInsert ;
    UInt64 m_NumTransmittedSignals;
    UInt64 m_NumLockedSignals;
    UInt64 m_NumReceivedSignals;
    UInt64 m_NumDroppedSignals;
    UInt64 m_NumDroppedInterferenceSignals;
    double m_Utilization;
    BOOL m_UtilizationSpecified;
    //int m_NumPhys;

    double m_AvgInterference;
    BOOL m_AvgInterferenceSpecified;

    double m_Delay;
    BOOL m_DelaySpecified;

    double m_PathLoss;
    BOOL m_PathLossSpecified;

    double m_SignalPower;
    BOOL m_SignalPowerSpecified;

    StatsDBPhyAggregateParam();

    void SetUtilization(double u);
    void SetPathLoss(double avgPathLoss);
    void SetDelay(double avgDelay);
    void SetSignalPower(double avgSignalPower);
    void SetAvgInterference(double avgInterference);
};


//--------------------------------------------------------------------//
// CLASS    : StatsDBMacAggregateParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the MAC_Aggregate table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBMacAggregateParam
{
public:
    BOOL toInsert;
    UInt64 m_DataFramesSent;
    UInt64 m_DataFramesReceived;
    UInt64 m_DataBytesSent;
    UInt64 m_DataBytesReceived;
    UInt64 m_ControlFramesSent;
    UInt64 m_ControlFramesReceived;
    UInt64 m_ControlBytesSent;
    UInt64 m_ControlBytesReceived;

    double m_AvgDelay;
    BOOL m_AvgDelaySpecified;

    double m_AvgJitter;
    BOOL m_AvgJitterSpecified;

    double m_CarriedLoad;

    StatsDBMacAggregateParam();

    void SetAvgDelay(double delay);
    void SetAvgJitter(double jitter);
};

class StatsDBAppJitterAggregateParam
{
public:
    Int32 m_PartitionId;
    Int64 m_TotalUnicastJitter;
    Int64 m_TotalMulticastJitter;
    UInt64 m_UnicastMessageReceived;
    UInt64 m_MulticastMessageReceived;
    StatsDBAppJitterAggregateParam();
};

class StatsDBAppSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_ReceiverId;
    char m_TargetAddr[MAX_STRING_LENGTH];
    Int32 m_SessionId;
    Int32 m_Tos;
    UInt64 m_MessageSent;
    UInt64 m_EffMessageSent;
    UInt64 m_MessageRecd;
    UInt64 m_ByteSent;
    UInt64 m_EffByteSent;
    UInt64 m_ByteRecd;
    UInt64 m_FragmentSent;
    UInt64 m_EffFragmentSent;
    UInt64 m_FragmentRecd;
    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    double m_MessageCompletionRate;
    BOOL m_MessageCompletionRateSpecified ;

    double m_OfferedLoad;
    BOOL m_OfferedLoadSpecified ;

    double m_Throughput;
    BOOL m_ThroughputSpecified ;

    double m_Delay;
    BOOL m_DelaySpecified;

    double m_Jitter;
    BOOL m_JitterSpecified;

    double m_HopCount;
    BOOL m_HopCountSpecified;

    StatsDBAppSummaryParam();

    void SetCompletionRate(double);
    void SetOfferedLoad(double);
    void SetThroughput(double);
    void SetDelay(double delay);
    void SetJitter(double jitter);
    void SetHopCount(double hopCount);
#ifdef ADDON_NGCNMS
    BOOL isRetrieved ;
#endif

};

class StatsDBMulticastAppSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_ReceiverId;
    char m_GroupAddr[MAX_STRING_LENGTH];
    Int32 m_SessionId;
    Int32 m_Tos;
    UInt64 m_MessageSent;
    UInt64 m_MessageRecd;
    UInt64 m_ByteSent;
    UInt64 m_ByteRecd;
    UInt64 m_FragmentSent;
    UInt64 m_FragmentRecd;
    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    double m_MessageCompletionRate;
    BOOL m_MessageCompletionRateSpecified ;

    double m_OfferedLoad;
    BOOL m_OfferedLoadSpecified ;

    double m_Throughput;
    BOOL m_ThroughputSpecified ;

    double m_Delay;
    BOOL m_DelaySpecified;

    double m_Jitter;
    BOOL m_JitterSpecified;

    double m_HopCount;
    BOOL m_HopCountSpecified;

    StatsDBMulticastAppSummaryParam();

    void SetCompletionRate(double);
    void SetOfferedLoad(double);
    void SetThroughput(double);
    void SetDelay(double delay);
    void SetJitter(double jitter);
    void SetHopCount(double hopCount);
#ifdef ADDON_NGCNMS
    BOOL isRetrieved ;
#endif

};

class StatsDBAppJitterSummaryParam
{
public:
    Int32 m_InitiatorId;
    Int32 m_SessionId;
    Int64 m_TotalJitter;
    UInt64 m_MessageRecd;
    StatsDBAppJitterSummaryParam();
};

struct StatsDBTransSummaryParam
{
    std::string senderAddr;
    std::string receiverAddr;
    short SenderPort;
    short ReceiverPort;

    UInt64 m_SegmentSentToApp;
    UInt64 m_SegmentSentToNet;
    UInt64 m_ByteSentToApp;
    UInt64 m_ByteSentToNet;
    double m_OfferedLoad;
    double m_Throughput;
    Int32 m_TotalHopCount;

    double m_AverageDelay;
    BOOL m_DelaySpecified;
    double m_AverageJitter;
    BOOL m_JitterSpecified;

    StatsDBTransSummaryParam()
    {
        SenderPort = 0;
        ReceiverPort = 0;

        m_SegmentSentToApp = 0;
        m_SegmentSentToNet = 0;
        m_ByteSentToApp = 0;
        m_ByteSentToNet = 0;
        m_OfferedLoad = 0;
        m_Throughput = 0;
        m_TotalHopCount = 0;

        m_AverageDelay = 0;
        m_DelaySpecified = FALSE;
        m_AverageJitter = 0;
        m_JitterSpecified = FALSE;
    }
    void SetAveDelay(double delay)
    {
        m_AverageDelay = delay ;
        m_DelaySpecified = TRUE;
    }
    void SetAveJitter(double jitter)
    {
        m_AverageJitter = jitter;
        m_JitterSpecified = TRUE;
    }
};

class StatsDBNetworkSummaryParam
{
public:
    char m_SenderAddr[MAX_STRING_LENGTH];
    char m_ReceiverAddr[MAX_STRING_LENGTH];
    char m_DestinationType[MAX_STRING_LENGTH];

    UInt64 m_UDataPacketsSent;
    UInt64 m_UDataPacketsRecd;
    UInt64 m_UDataPacketsForward;
    UInt64 m_UControlPacketsSent;
    UInt64 m_UControlPacketsRecd;
    UInt64 m_UControlPacketsForward;

    UInt64 m_UDataBytesSent;
    UInt64 m_UDataBytesRecd;
    UInt64 m_UDataBytesForward;
    UInt64 m_UControlBytesSent;
    UInt64 m_UControlBytesRecd;
    UInt64 m_UControlBytesForward;

    double m_Delay;
    BOOL m_DelaySpecified;

    double m_Jitter;
    BOOL m_JitterSpecified;

    double m_DataDelay;
    BOOL m_DataDelaySpecified ;

    double m_DataJitter;
    BOOL m_DataJitterSpecified ;

    double m_ControlDelay;
    BOOL m_ControlDelaySpecified ;

    double m_ControlJitter;
    BOOL m_ControlJitterSpecified ;

    StatsDBNetworkSummaryParam ();

    void SetDelay(double delay);
    void SetJitter(double jitter);

    enum NETSUMPARAM_DataType{
        DATA ,
        CONTROL
    } ;
    void SetDelay(double delay, NETSUMPARAM_DataType );
    void SetJitter(double jitter, NETSUMPARAM_DataType);

};

class StatsDBMacSummaryParam
{
public:
    Int32 m_NodeId;
    Int32 m_InterfaceIndex;

    UInt64 m_BroadcastDataFramesSent;
    UInt64 m_UnicastDataFramesSent;
    UInt64 m_BroadcastDataFramesReceived;
    UInt64 m_UnicastDataFramesReceived;
    UInt64 m_BroadcastDataBytesSent;
    UInt64 m_UnicastDataBytesSent;
    UInt64 m_BroadcastDataBytesReceived;
    UInt64 m_UnicastDataBytesReceived;
    UInt64 m_ControlFramesSent;
    UInt64 m_ControlFramesReceived;
    UInt64 m_ControlBytesSent;
    UInt64 m_ControlBytesReceived;

    UInt64 m_FramesDropped;
    UInt64 m_BytesDropped;

    double m_AvgDelay;
    BOOL m_AvgDelaySpecified;

    double m_AvgJitter;
    BOOL m_AvgJitterSpecified;

    StatsDBMacSummaryParam();

    void SetAvgDelay(double delay);
    void SetAvgJitter(double jitter);
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhySummaryParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Summary table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhySummaryParam
{
public:
    Int32 m_SenderId;
    Int32 m_RecieverId;
    Int32 m_PhyIndex;
    Int32 m_ChannelIndex;
    double m_Utilization;
    UInt64 m_NumSignals;
    UInt64 m_NumErrorSignals;

    double m_AvgInterference;
    BOOL m_AvgInterferenceSpecified;

    double m_Delay;
    BOOL m_DelaySpecified;

    double m_PathLoss;
    BOOL m_PathLossSpecified;

    double m_SignalPower;
    BOOL m_SignalPowerSpecified;

    StatsDBPhySummaryParam();

    void SetPathLoss(double avgPathLoss);
    void SetDelay(double avgDelay);
    void SetSignalPower(double avgSignalPower);
    void SetAvgInterference(double avgInterference);
};


class StatsDBAppEventParam
{
public:
    Int32 m_NodeId;
    Int32 m_SessionInitiator;
    Int32 m_ReceiverId;
    Address m_TargetAddr;
    BOOL m_TargetAddrSpecified;
    char m_MessageId[MAX_STRING_LENGTH];
    char m_EventType[MAX_STRING_LENGTH];
    //std::string m_MessageId;
    //std::string m_EventType;

    char m_ApplicationType[MAX_STRING_LENGTH];
    char m_ApplicationName[MAX_STRING_LENGTH];

    Int32 m_MsgSize;
    BOOL m_MsgSizeSpecified;

    Int32 m_MsgSeqNum;
    BOOL m_MsgSeqNumSpecified;

    Int32 m_FragId;
    BOOL m_FragIdSpecified;

    Int32 m_SessionId;
    BOOL m_SessionIdSpecified;

    Int32 m_Priority;
    BOOL m_PrioritySpecified;

    //std::string m_MsgFailureType;
    char m_MsgFailureType[MAX_STRING_LENGTH];
    BOOL m_MsgFailureTypeSpecified;

    clocktype m_Delay;
    BOOL m_DelaySpecified;

    clocktype m_Jitter;
    BOOL m_JitterSpecified;

    Int32 m_TotalMsgSize;
    BOOL m_IsFragmentation;
    BOOL m_fragEnabled;

    clocktype m_PktCreationTime;
    BOOL m_PktCreationTimeSpecified;

    UInt64 m_SocketInterfaceMsgId1;
    UInt64 m_SocketInterfaceMsgId2;
    BOOL m_SocketInterfaceMsgIdSpecified;

    StatsDBAppEventParam();

    void SetReceiverAddr(Address* receiverAddr);
    void SetReceiverAddr(NodeAddress receiverAddr);
    void SetMsgSeqNum(Int32 msgSeqNum);
    void SetFragNum(Int32 fragNum);
    void SetSessionId(Int32 id);
    void SetAppType(const char* appType);
    void SetAppName(const char* appName);
    void SetPriority(Int32 priority);
    void SetMessageFailure(char* failure);
    void SetDelay(clocktype delay);
    void SetJitter(clocktype jitter);
    void SetMsgSize(Int32 size);
    void SetPacketCreateTime(clocktype time);

    void SetSocketInterfaceMsgId(UInt64 SocketInterfaceMsgId1,
        UInt64 SocketInterfaceMsgId2)
    {
        m_SocketInterfaceMsgId1 = SocketInterfaceMsgId1;
        m_SocketInterfaceMsgId2 = SocketInterfaceMsgId2;
        m_SocketInterfaceMsgIdSpecified = TRUE;
    }

};


class StatsDBTransportEventParam
{
public:
    Int32 m_NodeId;
    char m_MessageId[MAX_STRING_LENGTH];
    Int32 m_MsgSize;

    //std::string m_ConnectionId;
    //BOOL m_ConnectionIdSpecified;
    short m_SenderPort;
    short m_ReceiverPort;

    Int32 m_MsgSeqNum;
    BOOL m_MsgSeqNumSpecified;

    //int  m_AppFragId;
    //BOOL m_AppFragIdSpecified;

    //int m_TcpSeqNumber; // Application Layer Frag Number
    //BOOL m_TcpSeqNumberSpecified;

    std::string m_ConnectionType;
    BOOL m_ConnTypeSpecified;

    Int32 m_HeaderSize;
    BOOL m_HdrSizeSpecified;

    //int m_Priority;
    //BOOL m_PrioritySpecified;

    std::string m_Flags;
    BOOL m_FlagsSpecified;

    std::string m_EventType;
    BOOL m_EventTypeSpecified;

    char m_FailureType[MAX_STRING_LENGTH];
    BOOL m_FailureTypeSpecified;
    StatsDBTransportEventParam(Int32 nodeId,
                               char* msgId,
                               Int32 size);

    StatsDBTransportEventParam(Int32 nodeId,
                               const std::string& msgId,
                               Int32 size);


    void SetMsgSeqNum(Int32 msgSeqNum);

    void SetConnectionType(const std::string& type);
    void SetHdrSize(Int32 size);
    void SetFlags(const std::string &flags);
    void SetEventType(const std::string &eventType);
    void SetMessageFailure(char* failure);

    clocktype transPktSendTime;
    BOOL transPktSendTimeSpecified;
    void SetPktSendTime(clocktype time);
};

class StatsDBNetworkEventParam
{
public:
    enum{
       DATA,
       CONTROL
    };
    Int32 m_NodeId;
    NodeAddress m_SenderAddr;
    NodeAddress m_ReceiverAddr;
    Int32 m_MsgSize;

    Int32 m_MsgSeqNum;
    BOOL m_MsgSeqNumSpecified;

    Int32 m_HeaderSize;
    BOOL m_HdrSizeSpecified;

    Int32 m_Priority;
    BOOL m_PrioritySpecified;

    unsigned char m_ProtocolType;
    BOOL m_ProtocolTypeSpecified;

    char m_PktType;
    BOOL m_PktTypeSpecified;

    Int32 m_InterfaceIndex;
    BOOL m_InterfaceIndexSpecified;

    double m_HopCount;
    BOOL m_HopCountSpecified;

    StatsDBNetworkEventParam();

    void SetMsgSeqNum(Int32 msgSeqNum);
    void SetFragNum(Int32 fragNum);
    void SetHdrSize(Int32 size);
    void SetPriority(Int32 priority);
    void SetProtocolType(char type);
    void SetPktType(char type);
    void SetInterfaceIndex(Int32 index);
    void SetHopCount(double count);

};

class StatsDBMacEventParam
{
public:
    Int32 m_NodeId;
    std::string m_MessageId;
    Int32 m_InterfaceIndex;
    Int32 m_MsgSize;
    std::string m_EventType;

    Int32 m_MsgSeqNum;
    BOOL m_MsgSeqNumSpecified;


    Int32 m_ChannelIndex;
    BOOL m_ChannelIndexSpecified;

    std::string m_FailureType;
    BOOL m_FailureTypeSpecified;

    Int32 m_HeaderSize;
    BOOL m_HdrSizeSpecified;

    std::string m_FrameType;
    BOOL m_FrameTypeSpecified;

    std::string m_DstAddrStr;
    BOOL m_DstAddrSpecified;

    std::string m_SrcAddrStr;
    BOOL m_SrcAddrSpecified;

    //int m_Priority;
    //BOOL m_PrioritySpecified;

    StatsDBMacEventParam(Int32 nodeId,
                         const std::string&,
                         Int32 phyIndex,
                         Int32 size,
                         const std::string &);

    void SetMsgId(const std::string &);
    void SetMsgEventType(const std::string &);
    void SetMsgSize(Int32 msgSize);
    void SetMsgSeqNum(Int32 msgSeqNum);
    void SetChannelIndex (Int32 index);
    void SetFailureType(const std::string &);
    void SetHdrSize(Int32 size);
    void SetFrameType(const std::string &);
    void SetDstAddr(const std::string &);
    void SetSrcAddr(const std::string &);
    //void SetPriority(int priority);
};

//--------------------------------------------------------------------//
// CLASS    : StatsDBPhyEventParam
// PURPOSE  : This object is used to record all the values to use
//            for inserting a new row into the PHY_Events table.
//            When an instance of this class is instatiated, the non-
//            required fields will be set to default values.
//--------------------------------------------------------------------//
class StatsDBPhyEventParam
{
public:
    Int32 m_NodeId;
    std::string m_MessageId;
    Int32 m_PhyIndex;
    Int32 m_MsgSize;
    std::string m_EventType;

    Int32 m_ChannelIndex;
    BOOL m_ChannelIndexSpecified;

    Int32 m_ControlSize; // Preamble size
    BOOL m_ControlSizeSpecified;

    std::string m_MessageFailureType;
    BOOL m_MessageFailureTypeSpecified;

    double m_SignalPower;
    BOOL m_SignalPowerSpecified;

    double m_Interference;
    BOOL m_InterferenceSpecified;

    double m_PathLoss;
    BOOL m_PathLossSpecified;

    StatsDBPhyEventParam(Int32 nodeId,
                         std::string messageId,
                         Int32 phyIndex,
                         Int32 msgSize,
                         std::string eventType);

    void SetChannelIndex(Int32 channel);
    void SetControlSize(Int32 size);
    void SetMessageFailureType(std::string type);
    void SetSignalPower(double signalPower);
    void SetInterference(double interference);
    void SetPathLoss(double pathLoss);

};

struct StatsDBAppConnParam
{
    std::string m_SrcAddress;
    std::string m_DstAddress;
    Int32 sessionId;

    StatsDBAppConnParam (const std::string &srcAddr,
                         const std::string &dstAddr,
                         Int32 s_id):
                         m_SrcAddress(srcAddr),
                         m_DstAddress(dstAddr)
    {
        sessionId = s_id;
    }
};

struct StatsDBTransConnParam
{
    std::string m_SrcAddress;
    std::string m_DstAddress;
    short m_SrcPort;
    short m_DstPort;

    StatsDBTransConnParam (const std::string &srcAddr,
                           const std::string &dstAddr,
                           short sport, short dport):
            m_SrcAddress(srcAddr), m_DstAddress(dstAddr)
    {
        m_SrcPort = sport;
        m_DstPort = dport;
    }
};

class StatsDBNetworkConnParam
{
public:
    Int32 m_NodeId;
    std::string m_DstAddress;
    Int32 m_Cost;

    std::string m_DstNetMask;
    BOOL m_DstMaskSpecified;

    Int32 m_OutgoingIntIndex;
    BOOL m_OutgoingIntIndexSpecified;

    std::string m_NextHopAddr;
    BOOL m_NextHopAddrSpecified;

    std::string m_RoutingProtocolType;
    BOOL m_RoutingProtocolSpecified;

    Int32 m_AdminDistance;
    BOOL m_AdminDistanceSpecified;

    StatsDBNetworkConnParam();

    void SetDstnetworkMask(std::string mask);
    void SetOutgoingInterface(Int32 index);
    void SetNextHopAddr(std::string addr);
    void SetRoutingProtocol(std::string protocol);
    void SetAdminDistance(Int32 distance);

};

struct StatsDBMacConnParam
{
    Int32 m_SenderId;
    Int32 m_ReceiverId;
    Int32 m_InterfaceIndex;
    Int32 m_ChannelIndex;
    char channelIndex_str[64];

    StatsDBMacConnParam()
    {
        m_InterfaceIndex = 0;
        m_ChannelIndex = 0;
    };
};

class StatsDBPhyConnParam
{
public:
    Int32 m_SenderId;
    Int32 m_ReceiverId;

    Int32 m_PhyIndex;
    BOOL m_PhyIndexSpecified; // toRemove

    Int32 m_ReceiverPhyIndex;

    //int m_ChannelIndex;
    string m_ChannelIndex;
    BOOL m_ChannelIndexSpecified; // toRemove

    void SetPhyIndex(Int32 index); // toRemove
    void SetChannelIndex(Int32 index); // toRemove

    BOOL senderListening ;
    BOOL receiverListening;

    AntennaModelType antennaType;
    BOOL reachableWorst;
    StatsDBPhyConnParam() ;
} ;

//--------------------------------------------------------------------//
// CLASS    : StatsDBInterfaceStatus
// PURPOSE  : This class is used to record all the values to use
//            for inserting a new row into the INTERFACE_Status table.
//--------------------------------------------------------------------//
class StatsDBInterfaceStatus
{
public:
    std::string m_address;
    BOOL m_interfaceEnabled;
    BOOL m_triggeredUpdate;
};


//--------------------------------------------------------------------//
// TYPE    : StatsDBDamageStateType
// PURPOSE  : This type is used by the DamageState column of the
//            NODE_Status table. Note that this type is defined only
//            for clarity -- when the insertion into the DB occurs,
//            a string value is used (either "Damaged" or "Undamaged")
//--------------------------------------------------------------------//
enum StatsDBDamageStateType
{
    STATS_DB_Undamaged,
    STATS_DB_Damaged,
};


//--------------------------------------------------------------------//
// TYPE    : StatsDBActiveStateType
// PURPOSE  : This type is used by the Active column of the
//            NODE_Status table. Note that this type is defined only
//            for clarity -- when the insertion into the DB occurs,
//            a string value is used (either "Enabled" or "Disabled")
//--------------------------------------------------------------------//
enum StatsDBActiveStateType
{
    STATS_DB_Enabled,
    STATS_DB_Disabled
};



//--------------------------------------------------------------------//
// CLASS    : StatsDBNodeStatus
// PURPOSE  : This class is used to record all the values to use
//            for inserting a new row into the NODE_Status table.
//--------------------------------------------------------------------//
class StatsDBNodeStatus
{
public:
    Int32 m_NodeId;
    BOOL m_TriggeredUpdate;

    double m_DimensionOnePosition;
    double m_DimensionTwoPosition;
    double m_DimensionThreePosition;
    StatsDBActiveStateType m_Active;
    StatsDBDamageStateType m_DamageState;
    double m_DimensionOneVelocity;
    double m_DimensionTwoVelocity;
    double m_DimensionThreeVelocity;

    BOOL m_IsGateway;
    BOOL m_PositionUpdated;
    BOOL m_ActiveStateUpdated;
    BOOL m_DamageStateUpdated;
    BOOL m_VelocityUpdated;

    StatsDBNodeStatus(Node* node, BOOL triggered);
};

struct StatsDBMulticastStatus
{

    std::string timeJoined;
    std::string timeLeft;
    std::string groupName;

    //BOOL joinPrint;
    //BOOL leavePrint;

    //StatsDBMulticastStatus() : joinPrint(FALSE), leavePrint(FALSE) {}

};


void AddInsertQueryToBufferStatsDb(StatsDb* db, const std::string &queryStr);
void FlushQueryBufferStatsDb(StatsDb* db);

void InitializePartitionStatsDb(StatsDb* statsDb);

void STATSDB_CreateNodeMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput);
void STATSDB_CreateQueueMetaDataColumns(PartitionData* partition,
                                        NodeInput* nodeInput);
void STATSDB_CreateInterfaceMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateConnectionMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateSchedulerMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreateSessionMetaDataColumns(PartitionData* partition,
        NodeInput* nodeInput);
void STATSDB_CreatePhyMetaDataColumns(PartitionData*, NodeInput*);

// API's for Description Table
void STATSDB_HandleNodeDescTableInsert(Node* node, PartitionData* partition);
void STATSDB_HandleQueueDescTableInsert(Node* node, StatsDBQueueDesc queueDesc);
class Scheduler;
void STATSDB_HandleSchedulerDescTableInsert(Node* node,
                                            Int32 interfaceIndex,
                                            const string& schedulerType,
                                            const string&);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol);

void STATSDB_HandleConnectionDescTableInsert(Node* node,
        const Address &, const Address & ,
        short, short, const std::string &);
void STATSDB_HandleInterfaceDescTableInsert(Node* node, StatsDBInterfaceDesc interfaceDesc);
void STATSDB_HandlePhyDescTableInsert(Node* node,
                                      Int32 interfaceIndex,
                                      Int32 phyIndex);

void STATSDB_PrintTable(StatsDb* db,
                        STATSDB_TABLE_TYPE type);

// API's for Aggregate Table
void STATSDB_HandleAppAggregateTableInsert(Node* node);
void STATSDB_HandleTransAggregateTableInsert(Node *);
void STATSDB_HandleNetworkAggregateTableInsert(Node* node);
void STATSDB_HandlePhyAggregateTableInsert(Node* node);
void STATSDB_HandleMacAggregateTableInsert(Node* node);
void STATSDB_HandleQueueAggregateTableInsert(Node* node);

// API's for Summary Table
void STATSDB_HandleAppSummaryTableInsert(Node* node);
void STATSDB_HandleTransSummaryTableInsert(Node *node);
void STATSDB_HandleNetworkSummaryTableInsert(Node* node);
void STATSDB_HandlePhySummaryTableInsert(Node* node,
        const StatsDBPhySummaryParam &phyParam);
void STATSDB_HandlePhySummaryTableInsert(Node* node);
void STATSDB_HandleQueueSummaryTableInsert(Node* node);
void STATSDB_HandleMulticastAppSummaryTableInsert(Node* node);

void STATSDB_HandleMulticastNetSummaryTableInsert(Node* node,
                        const StatsDBMulticastNetworkSummaryContent & stats);

void STATSDB_HandleMacSummaryTableInsert(Node* node);

// API's for Events Table
void STATSDB_HandleAppEventsTableUpdate(Node* node,
                                        void* data,
                                        const StatsDBAppEventParam & appParam);
void STATSDB_HandleTransEventsTableUpdate(Node* node,
        void* data,
        StatsDBTransportEventParam transParam);
void STATSDB_HandleNetworkEventsTableUpdate(Node* node,
        void* data,
        const StatsDBNetworkEventParam& networkParam, Message *msg,
        const char * failure, BOOL failureSpecified,
        const char *eventTyp);
void STATSDB_HandleMacEventsTableInsert(Node* node,
                                        void* data,
                                        const StatsDBMacEventParam & macParam);
void STATSDB_HandlePhyEventsTableInsert(Node* node,
                                        const StatsDBPhyEventParam &phyParam);

void HandleStatsDBMessageIdMappingInsert(Node *node,
    const std::string &, const std::string &,
    const std::string &);
void HandleStatsDBMessageIdMappingInsert(
         Node *node,
         const Message* oldMsg,
         const Message* newMsg,
         const std::string &protocol);
void HandleStatsDBTransportEventsInsert(Node* node,
                                        Message *msg,
                                        const StatsDBTransportEventParam & transParam);
// API's for Connectivity Table
void STATSDB_HandleAppConnTableInsert(Node *node,
                                      const StatsDBAppConnParam * appConnParam) ;
void STATSDB_HandleTransConnTableInsert(Node *node,
                                        const StatsDBTransConnParam * transConnParam);
void STATSDB_HandleMulticastConnTableInsert(Node *node,
        StatsDBConnTable::MulticastConnectivity multicastConnParam);
void STATSDB_HandleNetworkConnTableUpdate(Node* node,
        StatsDBNetworkConnParam ipParam);
void STATSDB_HandleMacConnTableUpdate(Node *node,
                                      const StatsDBMacConnParam &macParam);
void STATSDB_HandlePhyConnTableUpdate(Node* node,
                                      const StatsDBPhyConnParam & phyParam);


void StatsDBRetrieveDataDatabase(Node* node,
                                 StatsDb* db,
                                 std::string query,
                                 char*** data,
                                 Int32* ncol,
                                 Int32* nrow);

void StatsDbFinishEventTablesInsertion(PartitionData* partition);
void StatsDbFinalize(PartitionData* partition);
void StatsDbFinalize(void);
void StatsDbDriverClose(PartitionData* partition);


// APIs for Status Table
void STATSDB_HandleNodeStatusTableInsert(Node* node, StatsDBNodeStatus );
void STATSDB_HandleInterfaceStatusTableInsert(Node *node, BOOL);
void STATSDB_HandleInterfaceStatusTableInsert(Node* node,
                                              BOOL triggeredUpdate,
                                              Int32);
void STATSDB_HandleInterfaceStatusTableInsert(PartitionData* partition,
        BOOL triggeredUpdate, Message *msg);
void STATSDB_HandleInterfaceStatusTableInsert(Node* node, StatsDBInterfaceStatus );
void STATSDB_HandleMulticastStatusTableInsert(Node* node, Message *msg);
void STATSDB_HandleMulticastStatusTableInsert(Node* node) ;
void STATSDB_HandleQueueStatusTableInsertion(Node* node);



//--------------------------------------------------------------------------
// FUNCTION:  HandleLinkUtilizationTableInsert
// PURPOSE:  to process the Stats DB event inputa.
// PARAMETERS
// + node : Node* : Pointer to a node
// + LinkUtilizationParam : vector <StatsDBLinkUtilizationParam>*
//                       : pointer to the parameters.
// RETURN void.
//--------------------------------------------------------------------------


void
STATSDB_HandleLinkUtilizationTableInsert(Node* node,
                    vector<StatsDBLinkUtilizationParam>* linkUtilizationParam,
                    const std::string* str);



//--------------------------------------------------------------------------
// FUNCTION:  STATSDB_HandleLinkUtilizationPerNodeTableInsert
// PURPOSE:   to put node buffer for link utilization in to lupernodetable.
// PARAMETERS
// + node : Node* : Pointer to a node
//
//
// RETURN void.
//--------------------------------------------------------------------------

void STATSDB_HandleLinkUtilizationPerNodeTableInsert
(Node* node, const std::string* str);



//--------------------------------------------------------------------------
// FUNCTION:  STATSDB_HandleLinkUtilizationTableRetrieveFromPerNodeAndCalculate
// PURPOSE:  to process the Stats DB event inputa.
// PARAMETERS
// + node : Node* : Pointer to a node
// + LinkUtilizationParam : vector <StatsDBLinkUtilizationParam>*
//                       : pointer to the parameters.
// RETURN void.
//--------------------------------------------------------------------------

void
STATSDB_HandleLinkUtilTableCalculate(Node* node,
                vector<StatsDBLinkUtilizationParam>* linkUtilizationParam,
                const std::string* str);

#endif


