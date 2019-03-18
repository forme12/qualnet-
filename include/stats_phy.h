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
// PACKAGE     :: STATS_PHY
// DESCRIPTION :: This file describes Physical Layer stats
// **/

#ifndef _STATS_PHY_H_
#define _STATS_PHY_H_

#include "stats.h"

// /**
// DEFINE      :: STAT_PHY_THRESHOLD_MW
// DESCRIPTION :: Any signal power or interference smaller than this value
//                will not be counted
// **/
#define STAT_PHY_THRESHOLD_MW 1.0e-22

// /**
// ENUM        :: PhyMode
// DESCRIPTION :: Enlisted to differentiate various PHY types based on
//                behavior of locking or NOT locking during packet reception.
// **/
enum PhyMode
{
    PHY_LOCKING,
    PHY_NOLOCKING
};

// Forward declarations
class STAT_GlobalPhyStatistics;

// /**
// CLASS       :: STAT_PhySessionKey
// DESCRIPTION :: Implements a key for a phy session.  A session
//                is defined as statistics between a (senderId, receiverId,
//                channelIndex) tuple
// **/
class STAT_PhySessionKey
{
public:
    NodeId m_senderId;
    NodeId m_receiverId;
    int m_channelIndex;
    int m_phyIndex;

    bool operator <(const STAT_PhySessionKey& rhs) const
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
        if (m_channelIndex < rhs.m_channelIndex)
        {
            return true;
        }
        if (m_channelIndex > rhs.m_channelIndex)
        {
            return false;
        }
        if (m_phyIndex < rhs.m_phyIndex)
        {
            return true;
        }
        if (m_phyIndex > rhs.m_phyIndex)
        {
            return false;
        }

        return false;
    }
};

// /**
// CLASS        :: STAT_PhySessionStatistics
// DESCRIPTION  :: Implements statistics for a (senderId, receiverId,
//                 channelIndex) tuple.  Session statistics are updated
//                 by Stats API automatically.
//
// **/
struct STAT_PhySessionStatistics
{
    NodeId senderId;
    NodeId receiverId;
    int channelIndex;
    int phyIndex;

    double utilization;
    double totalInterference;
    double totalPathLoss;
    double totalDelay;
    double totalSignalPower;
    int numSignals;
    int numErrorSignals;
    clocktype lastSignalStartTime;

    STAT_PhySessionStatistics();
    STAT_PhySessionStatistics(NodeId sender, NodeId receiver, int channel, int phyIndex);

    STAT_PhySessionStatistics& operator+= (const STAT_PhySessionStatistics& rhs);
};

// /**
// CLASS        :: STAT_PhyStatistics
// DESCRIPTION  :: Implements a common physical layer statistics class.  See
//                 function comments for usage instructions.
//
// **/
class STAT_PhyStatistics : public STAT_ModelStatistics
{
private:
    // A map containing all sessions for this phy
    std::map<STAT_PhySessionKey, STAT_PhySessionStatistics> m_Sessions;

protected:
    // When the latest activity was started, used for utilization.
    // Value is CLOCKTYPE_MAX when no signal is locked.
    clocktype m_LockTime;

    // PHY_SIGNAL_LOCK_MSG_LIST is a vector list of message pointers
    // received at signal arrival from channel, that needs to be locked.
    // This vector will be used only with PhyMode "PHY_NOLOCKING".
    typedef std::vector<Message*> PHY_SIGNAL_LOCK_MSG_LIST;
    PHY_SIGNAL_LOCK_MSG_LIST* signalLockMsgList;
    typedef PHY_SIGNAL_LOCK_MSG_LIST::iterator PHY_SIGNAL_LOCK_LIST_ITER;

    // used to set and get the PHY category
    PhyMode phyMode;

    STAT_Sum m_SignalsTransmitted;
    STAT_Sum m_SignalsDetected;
    STAT_Sum m_SignalsLocked;
    STAT_Sum m_SignalsWithErrors;
    STAT_Sum m_SignalsInterferenceErrors;
    STAT_Sum m_SignalsToMac;
    STAT_Sum m_TotalTransmitTime;
    STAT_Sum m_TotalReceiveTime;
    STAT_Average m_AverageDelay;
    STAT_Utilization m_Utilization;
    STAT_Average_dBm m_AverageSignalPower_dBm;
    STAT_Average_dBm m_AverageInterference_dBm;
    STAT_Average m_AveragePathloss_dB;

    // Return the session for the (senderId, receiverId, channelIndex) tuple.
    // If session statistics do not exist for this session then new ones are created.
    STAT_PhySessionStatistics* GetSession(
        Node* node,
        NodeId senderId,
        NodeId receiverId,
        int channelIndex,
        int phyIndex);

    // Update session statistics.  Called by AddSignalLockedDataPoints,
    // AddSignalWithErrorsDataPoints and AddSignalToMacDataPoints
    void UpdateSession(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        double totalInterference_mW,
        double signalPower_mW,
        BOOL endSignal,
        BOOL isErrorSignal,
        clocktype duration = 0);

    // Update session statistics.
    // Called by API AddSignalTerminatedDataPoints
    // Currently this API is for phy mode PHY_LOCKING only
    void UpdateSessionForTerminatedDataPoints(
        Node* node,
        PhyData* phy,
        Message* rxMsg,
        Int32 rxChannelIndex,
        NodeAddress txNodeId,
        double pathloss_dB,
        clocktype rxStartTime,
        double totalInterference_mW,
        double signalPower_mW);

    // Update session statistics for Msg in the received PackedMsg on PHY.
    // Called by APIs AddSignalWithErrorsDataPointsForMsgInRecvPackedMsg()
    // and AddSignalToMacDataPointsForMsgInRecvPackedMsg() only.
    void UpdateSessionForMsgInRecvPackedMsg(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        BOOL isErrorSignal);


public:
    typedef std::map<STAT_PhySessionKey, STAT_PhySessionStatistics>::iterator SessionIterator;

    STAT_PhyStatistics(Node* node,
                       PhyMode phyMode = PHY_LOCKING);

    void AddToGlobal(STAT_GlobalPhyStatistics* stat); // called by Initialize

    // Call once for every signal that is transmitted
    void AddSignalTransmittedDataPoints(Node* node, clocktype duration);

    // Call once for every signal that is detected.  AddSignalLockedDataPoints
    // may be called next.
    void AddSignalDetectedDataPoints(Node* node);

    // Call once for every signal that is locked.  After calling
    // AddSignalLockedDataPoints the phy model must call either
    // AddSignalWithErrorsDataPoints (if the signal was not received) or
    // AddSignalToMacDataPoints (if the signal was received) or
    // in case of PHY_NOLOCKING AddSignalUnLockedDataPoints (if the signal
    // was terminated while receiving)
    void AddSignalLockedDataPoints(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        double power_mW);

    // May call before calling AddSignalWithErrorsDataPoints if the signal
    // was not received due to interference.  AddSignalWithErrorsDataPoints
    // must be called after calling
    // AddSignalWithInterferenceErrorsDataPoints.
    void AddSignalWithInterferenceErrorsDataPoints(Node* node);

    // Call when a signal is detected but could not be recieved
    void AddSignalWithErrorsDataPoints(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        double interference_mW,
        double signalPower_mW = 0,
        BOOL deleteMsg = TRUE);

    // Call for the case when there is an in-between
    // termination of current receiving signal.
    // Currently this API is for phy mode PHY_LOCKING only
    void AddSignalTerminatedDataPoints(
        Node* node,
        PhyData* phy,
        Message* rxMsg,
        Int32 rxChannelIndex,
        NodeAddress txNodeId,
        double pathloss_dB,
        clocktype rxStartTime,
        double interference_mW,
        double signalPower_mW = 0);
    // Call when a signal is detected and successfully received
    void AddSignalToMacDataPoints(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        clocktype delay,
        double interference_mW,
        double pathloss_dB,
        double signalPower_mW = 0,
        BOOL deleteMsg = TRUE);


    // This API is currently used to reset m_LockTime for both the phy modes
    // PHY_LOCKING and PHY_NOLOCKING.
    // This API should also be called after APIs
    // AddSignalWithErrorsDataPointsForMsgInRecvPackedMsg() and
    // AddSignalToMacDataPointsForMsgInRecvPackedMsg() to unlock and delete
    // the original received packed message on PHY.
    void AddSignalUnLockedDataPoints(
        Node* node,
        Message* msg);


    // Call when a signal is detected but could not be recieved.
    // This API is for handling individual msg that are extracted from the
    // message list under received packed message on PHY.
    // AddSignalUnLockedDataPoints() must be called after calling this
    // API to make sure of unlocking the original packed message.
    void AddSignalWithErrorsDataPointsForMsgInRecvPackedMsg(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        double interference_mW,
        double pathloss_dB,
        double signalPower_mW,
        BOOL isOriginalPackedMsg);

    // Call when a signal is detected and successfully received.
    // This API is for handling individual msg that are extracted from the
    // message list under received packed message on PHY.
    // AddSignalUnLockedDataPoints() must be called after calling this
    // API to make sure of unlocking the original packed message.
    void AddSignalToMacDataPointsForMsgInRecvPackedMsg(
        Node* node,
        PropRxInfo* propRxInfo,
        PhyData* phy,
        Int32 channelIndex,
        clocktype delay,
        double interference_mW,
        double pathloss_dB,
        double signalPower_mW,
        BOOL isOriginalPackedMsg);


    // Called when a node's statistics are to be finalized
    void Finalize(Node* node);

    // Set statistic names and descriptions
    void SetStatNames();

    // Return a list of all phy statistics
    virtual void GetList(std::vector<STAT_Statistic*>& stats);

    STAT_Sum& GetSignalsTransmitted() { return m_SignalsTransmitted; }
    STAT_Sum& GetSignalsDetected() { return m_SignalsDetected; }
    STAT_Sum& GetSignalsLocked() { return m_SignalsLocked; }
    STAT_Sum& GetSignalsWithErrors() { return m_SignalsWithErrors; }
    STAT_Sum& GetSignalsInterferenceErrors() { return m_SignalsInterferenceErrors; }
    STAT_Sum& GetSignalsToMac() { return m_SignalsToMac; }
    STAT_Sum& GetTotalTransmitTime() { return m_TotalTransmitTime; }
    STAT_Sum& GetTotalReceiveTime() { return m_TotalReceiveTime; }
    STAT_Utilization& GetUtilization() { return m_Utilization; }
    STAT_Average& GetAverageSignalPower_dBm() { return m_AverageSignalPower_dBm; }
    STAT_Average& GetAverageInterference_dBm() { return m_AverageInterference_dBm; }
    STAT_Average& GetAveragePathloss_dB() { return m_AveragePathloss_dB; }

    // SessionBegin and SessionEnd can be used to iterate over session statistics
    SessionIterator SessionBegin() { return m_Sessions.begin(); }
    SessionIterator SessionEnd() { return m_Sessions.end(); }
};

// /**
// CLASS        :: STAT_GlobalPhyStatistics
// DESCRIPTION  :: Implements global physical aggregated statistics.
//                 Values are handled automatically when phy models use
//                 the STAT_PhyStatistics class.
//
// **/
class STAT_GlobalPhyStatistics
{
protected:
    STAT_AggregatedSum m_SignalsTransmitted;
    STAT_AggregatedSum m_SignalsDetected;
    STAT_AggregatedSum m_SignalsLocked;
    STAT_AggregatedSum m_SignalsWithErrors;
    STAT_AggregatedSum m_SignalsInterferenceErrors;
    STAT_AggregatedSum m_SignalsToMac;
    STAT_AggregatedSum m_TotalTransmitTime;
    STAT_AggregatedSum m_TotalReceiveTime;
    STAT_AggregatedWeightedDataPointAverage m_AverageDelay;
    STAT_AggregatedAverage m_AverageUtilization;
    STAT_AggregatedWeightedDataPointAverage m_AverageSignalPower_dBm;
    STAT_AggregatedWeightedDataPointAverage m_AverageInterference_dBm;
    STAT_AggregatedWeightedDataPointAverage m_AveragePathloss_dB;

public:
    void Initialize(PartitionData* partition, STAT_StatisticsList* stats);

    STAT_AggregatedSum& GetSignalsTransmitted() { return m_SignalsTransmitted; }
    STAT_AggregatedSum& GetSignalsDetected() { return m_SignalsDetected; }
    STAT_AggregatedSum& GetSignalsLocked() { return m_SignalsLocked; }
    STAT_AggregatedSum& GetSignalsWithErrors() { return m_SignalsWithErrors; }
    STAT_AggregatedSum& GetSignalsInterferenceErrors() { return m_SignalsInterferenceErrors; }
    STAT_AggregatedSum& GetSignalsToMac() { return m_SignalsToMac; }
    STAT_AggregatedSum& GetTotalTransmitTime() { return m_TotalTransmitTime; }
    STAT_AggregatedSum& GetTotalReceiveTime() { return m_TotalReceiveTime; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageDelay() { return m_AverageDelay; }
    STAT_AggregatedAverage& GetAverageUtilization() { return m_AverageUtilization; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageSignalPower_dBm() { return m_AverageSignalPower_dBm; }
    STAT_AggregatedWeightedDataPointAverage& GetAverageInterference_dBm() { return m_AverageInterference_dBm; }
    STAT_AggregatedWeightedDataPointAverage& GetAveragePathloss_dB() { return m_AveragePathloss_dB; }
};

#endif /* _STATS_PHY_H_ */
