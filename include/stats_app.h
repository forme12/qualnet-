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
// PACKAGE     :: STATS_APP
// DESCRIPTION :: This file describes Application Layer stats
// **/

#ifndef _STATS_APP_H_
#define _STATS_APP_H_

#include "stats.h"

class STAT_GlobalAppStatistics;
class STAT_AppStatistics;

enum STAT_AppStatsType
{
    STAT_AppSender,
    STAT_AppReceiver,
    STAT_AppSenderReceiver
};

class STAT_MulticastAppSessionKey
{
public:
    NodeId m_senderId;
    NodeId m_destId;
    STAT_SessionIdType m_SessionId;

    bool operator <(const STAT_MulticastAppSessionKey& rhs) const
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

        return false;
    }
};

class STAT_MulticastAppSessionStatistics
{
protected:
    clocktype m_LastDelay;

    STAT_Minimum m_SessionStart;
    STAT_SessionFinish m_SessionFinish;

    NodeId m_senderId;
    NodeId m_DestNodeId;
    STAT_SessionIdType m_SessionId;
    
    STAT_Sum m_MessagesSent;
    STAT_Sum m_MessagesReceived;
    STAT_Sum m_BytesSent;
    STAT_Sum m_BytesReceived;
    STAT_Sum m_FragmentsSent;
    STAT_Sum m_FragmentsReceived;

    STAT_Average m_AverageDelay;
    STAT_Throughput m_OfferedLoad;
    STAT_Throughput m_Throughput;
    STAT_Average m_AverageJitter;
    STAT_Average m_AverageHopCount;

    friend class STAT_AppStatistics;

public:
    NodeId GetSenderId() { return m_senderId; }
    NodeId GetDestNodeId() { return m_DestNodeId; }
    STAT_SessionIdType GetSessionId() { return m_SessionId; }

    STAT_Sum& GetMessagesSent() { return m_MessagesSent; }
    STAT_Sum& GetMessagesReceived() { return m_MessagesReceived; }
    STAT_Sum& GetBytesSent() { return m_BytesSent; }
    STAT_Sum& GetBytesReceived() { return m_BytesReceived; }
    STAT_Sum& GetFragmentsSent() { return m_FragmentsSent; }
    STAT_Sum& GetFragmentsReceived() { return m_FragmentsReceived; }

    STAT_Average& GetAverageDelay() { return m_AverageDelay; }
    STAT_Average& GetAverageJitter() { return m_AverageJitter; }
    STAT_Average& GetAverageHopCount() { return m_AverageHopCount; }
    STAT_Throughput& GetThroughput() { return m_Throughput; }
    STAT_Throughput& GetOfferload() { return m_OfferedLoad; }

    STAT_MulticastAppSessionStatistics(
        NodeId senderId, NodeId destNodeId, STAT_SessionIdType sessionId);
};

class STAT_MulticastAppSessionSummaryStatistics
{
public:
    char type[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];
    UInt8 tos;
    NodeAddress receiverAddress;
    NodeId m_senderId;
    NodeId m_DestNodeId;
    STAT_SessionIdType m_SessionId;

    double m_messagesSent;
    double m_messagesReceived;
    double m_bytesSent;
    double m_bytesReceived;
    double m_fragmentsReceived;
    double m_fragmentsSent;

    double m_averageDelay;
    double m_offerload;
    double m_throughput;
    double m_avgJitter;
    int m_numJitterPoints;
    double m_avgHopCount;

    // Call to initialize this struct based on the statistics from the model
    // Can be called for sender or receiver.
    void InitializeFromModel(
        clocktype now,
        STAT_MulticastAppSessionStatistics* stats,
        STAT_AppStatistics* parentStats);
    
    // Call to update this struct based on statistics from another model's
    // summary statistics.  Can be called for a sender or receiver's stats.
    STAT_MulticastAppSessionSummaryStatistics& operator +=(const STAT_MulticastAppSessionSummaryStatistics& stats);
};

class STAT_AppAddressStatistics
{
protected:
    clocktype m_LastDelay;

    STAT_Minimum m_SessionStart;
    STAT_SessionFinish m_SessionFinish;

    STAT_Minimum m_FirstMessageSent;
    STAT_Maximum m_LastMessageSent;
    STAT_Minimum m_FirstMessageReceived;
    STAT_Maximum m_LastMessageReceived;
    STAT_Sum m_MessagesSent;
    STAT_Sum m_MessagesReceived;
    STAT_Sum m_MessagesDropped;

    // Effective sent.  Equal to messages sent for unicast.  Equal to
    // messages sent * group members for multicast and messages sent *
    // potential receivers for broadcast.  Broadcast is not currently
    // implemented.
    STAT_Sum m_EffectiveMessagesSent;
    STAT_Sum m_EffectiveFragmentsSent;
    STAT_Sum m_EffectiveDataSent;

    STAT_Minimum m_FirstFragmentSent;
    STAT_Maximum m_LastFragmentSent;
    STAT_Minimum m_FirstFragmentReceived;
    STAT_Maximum m_LastFragmentReceived;
    STAT_Sum m_FragmentsSent;
    STAT_Sum m_FragmentsReceived;
    STAT_Sum m_FragmentsReceivedOutOfOrder;
    STAT_Sum m_DuplicateFragmentsReceived;

    STAT_Sum m_DataSent;
    STAT_Sum m_DataReceived;
    STAT_Sum m_OverheadSent;
    STAT_Sum m_OverheadReceived;
    STAT_Average m_AverageDelay;
    STAT_Sum m_TotalDelay;
    STAT_Throughput m_OfferedLoad;
    STAT_Throughput m_Throughput;
    STAT_SmoothedAverage m_Jitter;
    STAT_Average m_AverageJitter;
    STAT_Sum m_TotalJitter;
    STAT_Average m_AverageHopCount;

    friend class STAT_AppStatistics;
};

// /**
// CLASS        :: STAT_AppStatistics
// DESCRIPTION  :: Implements a common application layer statistics class.
//                 Applications may implement additional statistics.
//                 Applications should
//                 call SessionStart if the session starts before the first
//                 message is sent.  Applications should call SessionFinish
//                 when the session has finished.  Applications should call
//                 MessageSent and MessageReceived when they send and
//                 receive messages if they are message oriented models.
//                 Applications should call DataSent and DataReceived if
//                 they are data or stream oriented models.
//  实现一个公共应用程序层统计类。应用程序可以实现额外的统计数据。如果会话在发送第一个消息之前开始，
//应用程序应该调用SessionStart。应用程序应该在会话结束时调用SessionFinish。应用程序应该调用

//MessageSent和messagereceiif是面向消息的模型，当它们发送和接收消息时。
//如果应用程序是面向数据或流的模型，则应该调用DataSent和DataReceived。               
// **/
class STAT_AppStatistics : public STAT_ModelStatistics
{
private:
    void SetStatNames();

protected:
    const std::string* m_Type;
    std::string m_CustomName;

    BOOL m_AutoRefragment;
    int m_RefragmentBytesLeft;
    int m_RefragmentBytes;

    BOOL m_Initialized;

    // Which types of traffic this app can send
    BOOL m_Unicast;
    BOOL m_Broadcast;
    BOOL m_Multicast;

    // Whether the app sends traffic, receives traffic, or both
    STAT_AppStatsType m_StatsType;

    // Unique session description
    NodeId m_SourceNodeId;
    STAT_SessionIdType m_SessionId;

    BOOL m_IsSource;
    Address m_SourceAddr;
    Address m_DestAddr;

    // m_DestNodeId is ANY_ADDRESS for the sender side if m_DestAddr is a
    // broadcast or multicast address
    NodeId m_DestNodeId;

    // session TOS
    unsigned int m_Tos;

    STAT_AppAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

    std::map<STAT_MulticastAppSessionKey, STAT_MulticastAppSessionStatistics*> m_multicastAppSessionStats;

    STAT_MulticastAppSessionStatistics* GetMulticastSession(clocktype time,
        NodeId& senderId,
        NodeId destNodeId,
        STAT_SessionIdType sessionId);

    // Add this application's statistics to the globals.  Called
    // automatically by Initialize
    void AddToGlobal(STAT_GlobalAppStatistics* stat);

public:
    // Create a new app statistics class.  The application name may
    // be a special user-defined name.
    STAT_AppStatistics(
        Node* node,
        const std::string& name,
        STAT_DestAddressType addrType,
        STAT_AppStatsType statsType,
        const char* customName = NULL);

    // Enabling auto-refragment  will refragment incoming
    // fragments and automatically call the message received function
    // when a full message has been received.  If applications use
    // autoRefragment then they must not call MessageReceived.
    void EnableAutoRefragment();

    // Finish initializing application statistics.  This function must be
    // called after the constructor.
    void Initialize(
        Node* node,
        Address sourceAddr,
        Address destAddr,
        STAT_SessionIdType sessionId);

    BOOL IsInitialized() { return m_Initialized; }

    // Called when model statistics are finalized.
    // If over-ridden, STAT_AppStatistics::Finalize must also be called
    virtual void Finalize(Node* node);

    // Get a list of statistics the application supports
    virtual void GetList(std::vector<STAT_Statistic*>& stats);

    // Return the session id for a message
    static STAT_SessionIdType GetSessionId(Message* msg);

    const std::string& GetType() { return *m_Type; }
    std::string GetCustomName() { return m_CustomName; }
    NodeId GetIsSource() { return m_IsSource; }
    NodeId GetSourceNodeId() { return m_SourceNodeId; }
    NodeId GetDestNodeId() { return m_DestNodeId; }
    STAT_SessionIdType GetSessionId() { return m_SessionId; }
    Address GetSourceAddr() { return m_SourceAddr; }
    Address GetDestAddr() { return m_DestAddr; }
    BOOL GetUnicast() { return m_Unicast; }
    BOOL GetBroadcast() { return m_Broadcast; }
    BOOL GetMulticast() { return m_Multicast; }

    BOOL IsSessionStarted(STAT_DestAddressType type = STAT_Unicast);
    BOOL IsSessionFinished(STAT_DestAddressType type = STAT_Unicast);

    // Add the STAT_Timing information to this message.  This function is
    // automatically called by AddMessageSentDataPoints.  If a fragment-only
    // packet is sent (not a message) then AddTiming should be called in
    // lieu of calling AddMessageSentDataPoints.
    void AddTiming(
        Node* node,
        Message* msg,
        int dataSize,
        int overheadSize);

    // Call AddMessageSentDataPoints when a message is sent.  A message is a
    // logical chunk of data and is defined by an application.  For example,
    // an HTTP message would be a "get page" operation, a CBR message would
    // be a packet.  Each message is composed of 1 or more fragments.
    // AddMessageSentDataPoints should be called before
    // AddMessageReceivedDataPoints.
    void AddMessageSentDataPoints(
        Node* node,
        Message* msg, 
        int controlSize,
        int dataSize,
        int overheadSize,
        STAT_DestAddressType type = STAT_Unicast);

    // Call AddMessageReceivedDataPoints when a message is received.
    // AddMessageReceivedDataPoints should NOT be called if
    // auto-refragmentation is used.
    void AddMessageReceivedDataPoints(
        Node* node,
        Message* msg, 
        int controlSize, 
        int dataSize,
        int overheadSize, 
        STAT_DestAddressType type = STAT_Unicast);

    // Call AddFragmentSentDataPoints when a fragment is sent.  For UDP
    // applications this is when the packet is sent to UDP.  For TCP
    // applications this is in response to a MSG_APP_FromTransDataSent
    // message.
    void AddFragmentSentDataPoints(
        Node* node,
        int size, 
        STAT_DestAddressType type = STAT_Unicast);

    // Call AddFragmentReceivedDataPoints when a fragment is received.
    // AddFragmentReceivedDataPoints should be called before
    // AddMessageReceivedDataPoints.
    void AddFragmentReceivedDataPoints(
        Node* node, 
        Message* msg, 
        int size, 
        STAT_DestAddressType type = STAT_Unicast);

    void AddFragmentReceivedOutOfOrderDataPoints(
        Node* node,
        STAT_DestAddressType type = STAT_Unicast);

    void AddFragmentReceivedDuplicateDataPoints(
        Node* node,
        STAT_DestAddressType type = STAT_Unicast);

    void AddMessageDroppedDataPoints(
        Node* node,
        STAT_DestAddressType type = STAT_Unicast);

    typedef std::map<STAT_MulticastAppSessionKey, STAT_MulticastAppSessionStatistics*>::iterator MulticastAppSessionIterator;

    MulticastAppSessionIterator GetSessionBegin() {return m_multicastAppSessionStats.begin();}
    MulticastAppSessionIterator GetSessionEnd() {return m_multicastAppSessionStats.end();}

    // Call SessionStart when the session is started.  Sending or receiving
    // packets will call SessionStart if it was not started manually.
    void SessionStart(Node* node, STAT_DestAddressType type = STAT_Unicast);
    
    // Call SessionFinish when the session is finished
    void SessionFinish(Node* node, STAT_DestAddressType type = STAT_Unicast);

    // Functions for getting statistics
    STAT_Minimum& GetSessionStart(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_SessionStart; }
    STAT_SessionFinish& GetSessionFinish(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_SessionFinish; }

    STAT_Minimum& GetFirstMessageSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FirstMessageSent; }
    STAT_Maximum& GetLastMessageSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_LastMessageSent; }
    STAT_Minimum& GetFirstMessageReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FirstMessageReceived; }
    STAT_Maximum& GetLastMessageReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_LastMessageReceived; }
    STAT_Sum& GetMessagesSent(
        STAT_DestAddressType type = STAT_Unicast) 
        { return m_AddrStats[type].m_MessagesSent; }
    STAT_Sum& GetMessagesReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_MessagesReceived; }
    STAT_Sum& GetMessagesDropped(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_MessagesDropped; }

    STAT_Minimum& GetFirstFragmentSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FirstFragmentSent; }
    STAT_Maximum& GetLastFragmentSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_LastFragmentSent; }
    STAT_Minimum& GetFirstFragmentReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FirstFragmentReceived; }
    STAT_Maximum& GetLastFragmentReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_LastFragmentReceived; }
    STAT_Sum& GetFragmentsSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FragmentsSent; }
    STAT_Sum& GetFragmentsReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FragmentsReceived; }
    STAT_Sum& GetFragmentsReceivedOutOfOrder(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_FragmentsReceivedOutOfOrder; }
    STAT_Sum& GetDuplicateFragmentsReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_DuplicateFragmentsReceived; }

    STAT_Sum& GetDataSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_DataSent; }
    STAT_Sum& GetDataReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_DataReceived; }
    STAT_Sum& GetOverheadSent(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_OverheadSent; }
    STAT_Sum& GetOverheadReceived(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_OverheadReceived; }
    STAT_Average& GetAverageDelay(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_AverageDelay; }
    STAT_Sum& GetTotalDelay(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_TotalDelay; }
    STAT_Throughput& GetOfferedLoad(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_OfferedLoad; }
    STAT_Throughput& GetThroughput(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_Throughput; }
    STAT_SmoothedAverage& GetJitter(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_Jitter; }
    STAT_Average& GetAverageJitter(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_AverageJitter; }
    STAT_Sum& GetTotalJitter(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_TotalJitter; }
    STAT_Average& GetAverageHopCount(
        STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_AverageHopCount; }

    STAT_Sum& GetEffectiveMessagesSent(STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_EffectiveMessagesSent; }
    STAT_Sum& GetEffectiveFragmentsSent(STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_EffectiveFragmentsSent; }
    STAT_Sum& GetEffectiveDataSent(STAT_DestAddressType type = STAT_Unicast)
        { return m_AddrStats[type].m_EffectiveDataSent; }

    void setTos(unsigned int tos)
    { m_Tos = tos; }
    unsigned int getTos()
    { return m_Tos; }

    friend class STAT_AppSummaryStatistics;
};

class STAT_GlobalAppAddressStatistics
{
protected:
    STAT_AggregatedSum m_MessagesSent;
    STAT_AggregatedSum m_MessagesReceived;
    STAT_AggregatedSum m_FragmentsSent;
    STAT_AggregatedSum m_FragmentsReceived;
    STAT_AggregatedSum m_DataReceived;
    STAT_AggregatedSum m_DataSent;
    STAT_AggregatedSum m_OverheadReceived;
    STAT_AggregatedSum m_OverheadSent;
    STAT_AggregatedSum m_AggregatedEffectiveMessagesSent;
    STAT_AggregatedSum m_AggregatedEffectiveFragmentsSent;
    STAT_AggregatedSum m_AggregatedEffectiveDataSent;
    STAT_AggregatedWeightedDataPointAverage m_AverageDelay;
    STAT_AggregatedThroughput m_OfferedLoad;
    STAT_AggregatedThroughput m_Throughput;
    STAT_AggregatedWeightedDataPointAverage m_AverageJitter;
    STAT_AggregatedSum m_TotalJitter;
    STAT_AggregatedWeightedDataPointAverage m_AverageHopCount;
    STAT_AggregatedDivide m_MessageCompletionRate;

    friend class STAT_GlobalAppStatistics;
};

// /**
// CLASS        :: STAT_GlobalAppStatistics
// DESCRIPTION  :: Implements global application aggregated statistics.
//                 Values are handled automatically when applications use
//                 the STAT_AppStatistics class.
//                 
// **/
class STAT_GlobalAppStatistics
{
protected:
    STAT_GlobalAppAddressStatistics m_AddrStats[STAT_NUM_ADDRESS_TYPES];

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetMessagesSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_MessagesSent; }
    STAT_AggregatedSum& GetMessagesReceived(STAT_DestAddressType type)
        { return m_AddrStats[type].m_MessagesReceived; }
    STAT_AggregatedSum& GetFragmentsSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_FragmentsSent; }
    STAT_AggregatedSum& GetFragmentsReceived(STAT_DestAddressType type)
        { return m_AddrStats[type].m_FragmentsReceived; }
    STAT_AggregatedSum& GetDataReceived(STAT_DestAddressType type)
        { return m_AddrStats[type].m_DataReceived; }
    STAT_AggregatedSum& GetDataSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_DataSent; }
    STAT_AggregatedSum& GetOverheadReceived(STAT_DestAddressType type)
        { return m_AddrStats[type].m_OverheadReceived; }
    STAT_AggregatedSum& GetOverheadSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_OverheadSent; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDelay(
        STAT_DestAddressType type)
        { return m_AddrStats[type].m_AverageDelay; }
    STAT_AggregatedThroughput& GetOfferedLoad(STAT_DestAddressType type)
        { return m_AddrStats[type].m_OfferedLoad; }
    STAT_AggregatedThroughput& GetThroughput(STAT_DestAddressType type)
        { return m_AddrStats[type].m_Throughput; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageJitter(
        STAT_DestAddressType type)
        { return m_AddrStats[type].m_AverageJitter; }
    STAT_AggregatedSum& GetTotalJitter(STAT_DestAddressType type)
        { return m_AddrStats[type].m_TotalJitter; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageHopCount(STAT_DestAddressType type)
        { return m_AddrStats[type].m_AverageHopCount; }
    STAT_AggregatedDivide& GetMessageCompletionRate(
        STAT_DestAddressType type)
        { return m_AddrStats[type].m_MessageCompletionRate; }

    STAT_AggregatedSum &GetAggregatedEffectiveMessagesSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_AggregatedEffectiveMessagesSent; }
    STAT_AggregatedSum& GetAggregatedEffectiveFragmentsSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_AggregatedEffectiveFragmentsSent; }
    STAT_AggregatedSum& GetAggregatedEffectiveDataSent(STAT_DestAddressType type)
        { return m_AddrStats[type].m_AggregatedEffectiveDataSent; }
};

class STAT_AppSummaryStatistics
{
public:
    // Number of sending side statistics.  Generally this will be one but it
    // could be higher if both sides send packets.
    int numSenders;

    // Number of receiving side statistics.  Generally this will be one.  It
    // could be two if both sides of a unicast session send packets and it
    // could be much higher for broadcast/multicast sessions.
    int numReceivers;

    char type[MAX_STRING_LENGTH];
    char name[MAX_STRING_LENGTH];

    UInt8 tos;
    int sessionId;
    NodeId senderId;
    NodeId receiverId;
    NodeAddress receiverAddress;

    double messagesSent;
    double effectiveMessagesSent;
    double messagesReceived;
    double fragmentsSent;
    double effectiveFragmentsSent;
    double fragmentsReceived;
    double dataSent;
    double effectiveDataSent;
    double dataReceived;
    double overheadSent;
    double overheadReceived;
    double offeredLoad;
    double throughput;
    double averageDelay;
    int numJitterDataPoints;
    double averageJitter;
    double averageHopCount;

    // Call to initialize this struct based on the statistics from the model
    // Can be called for sender or receiver.
    void InitializeFromModel(Node* node, STAT_AppStatistics* stats);
    
    // Call to update this struct based on statistics from another model's
    // summary statistics.  Can be called for a sender or receiver's stats.
    STAT_AppSummaryStatistics& operator +=(const STAT_AppSummaryStatistics& stats);
};

#endif /* _STATS_APP_H_ */
