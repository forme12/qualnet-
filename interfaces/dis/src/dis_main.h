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

#ifndef DIS_MAIN_H
#define DIS_MAIN_H

const unsigned g_disMaxDstEntityIdsInTimeoutNotification
    = (g_disMaxOutgoingDataPduVariableDatumsSize
       - sizeof(DisEntityId)     // Source EntityID
       - sizeof(unsigned short)  // Radio index
       - sizeof(unsigned)        // Timestamp
       - sizeof(unsigned)        // Number of packets
       - sizeof(unsigned))       // Number of entities
      / (sizeof(DisEntityId)     // Destination EntityID
         + sizeof(bool));        // Entity status

extern Dis g_dis;

// Initialization and main loop

void
DisInitDisVariable(EXTERNAL_Interface* iface);

void
DisReadParameters(EXTERNAL_Interface* iface, const NodeInput* nodeInput);

void
DisReadEntitiesFile(EXTERNAL_Interface* iface, NodeInput* nodeInput);

void
DisReadRadiosFile(EXTERNAL_Interface* iface, NodeInput* nodeInput);

void
DisReadNetworksFile(EXTERNAL_Interface* iface, NodeInput* nodeInput);

void
DisInitMessenger(EXTERNAL_Interface* iface);

void
DisMapHierarchyIds(NodeInput* nodeInput);

void
DisWaitForFirstEntityStatePdu(EXTERNAL_Interface* iface);

void
DisRegisterCtrlCHandler();

void
DisHandleSigInt(int a);

void
DisProcessPdu(const char* pduBuf, unsigned short length);

// Entity State PDU

void
DisProcessEntityStatePdu(const char* pduBuf, unsigned short length);

void
DisProcessMobilityFields(DisEntityStatePdu &pdu, DisEntity& entity);

void
DisScheduleMobilityEventIfNecessary(DisEntity& entity);

void
DisProcessDamageFields(DisEntityStatePdu& pdu, DisEntity& entity);

void
DisScheduleChangeInterfaceState(
    const DisEntity& entity,
    bool enableInterfaces,
    clocktype delay);

void
DisScheduleChangeInterfaceState(
    const DisRadio& radio, bool enableInterface);

void
DisScheduleChangeMaxTxPower(
    Node *node,
    const DisEntity& entity,
    unsigned damage,
    clocktype delay);

void
DisProcessHierarchyMobilityEvent(Node* node, Message* msg);

void
DisProcessChangeMaxTxPowerEvent(Node* node, Message* msg);

// Transmitter PDU

void
DisProcessTransmitterPdu(const char* pduBuf, unsigned short length);

void
DisProcessTransmitStateField(const DisTransmitterPdu& pdu, DisRadio& radio);

#ifdef MILITARY_RADIOS_LIB
void
DisScheduleRtssNotification(
    const DisRadio& radio,
    clocktype delay);

void
DisProcessSendRtssEvent(Node* node, Message* msg);

void
DisSendRtssNotification(Node* node);

void
DisPrintSentRtssNotification(Node* node);
#endif /* MILITARY_RADIOS_LIB */

// Signal PDU

void
DisProcessSignalPdu(const char* pduBuf, unsigned short length);

void
DisProcessCommEffectsRequest(const DisSignalPdu& pdu);

bool
DisParseMsgString(
    char*             msgString,
    const DisEntity*& dstEntityPtr,
    unsigned&         dataMsgSize,
    clocktype&        voiceMsgDuration,
    bool&             isVoice,
    clocktype&        timeoutDelay,
    unsigned&         timestamp);

void
DisSendSimulatedMsgUsingMessenger(
    const DisSimulatedMsgInfo& smInfo,
    const DisEntity* dstEntityPtr,
    double           requestedDataRate,
    unsigned         dataMsgSize,
    clocktype        voiceMsgDuration,
    bool             isVoice,
    clocktype        timeoutDelay,
    bool             unicast,
    const DisRadio*  dstRadioPtr,
    clocktype        sendDelay);

void
DisScheduleTimeout(
    const DisSimulatedMsgInfo& smInfo,
    clocktype timeoutDelay,
    clocktype sendDelay);

void
DisStoreOutstandingSimulatedMsgInfo(
    const DisSimulatedMsgInfo& smInfo,
    const DisEntity* dstEntityPtr,
    unsigned timestamp,
    clocktype sendTime);

void
DisMessengerResultFcn(Node* node, Message* msg, BOOL success);

void
DisMessengerResultFcnBody(Node* node, Message* msg, BOOL success);

void
DisSendProcessMsgNotification(
    Node* node,
    const DisSimulatedMsgInfo& smInfo,
    const DisOutstandingSimulatedMsgInfo& osmInfo,
    unsigned short success);

void
DisProcessTimeoutEvent(Node* node, Message* msg);

void
DisSendTimeoutNotification(
    Node* node,
    const DisSimulatedMsgInfo& smInfo,
    const DisOutstandingSimulatedMsgInfo& osmInfo);

void
DisPrintCommEffectsRequestProcessed(
    const DisRadio&  srcRadio,
    const DisEntity* dstEntityPtr,
    bool             unicast,
    const DisRadio*  dstRadioPtr,
    clocktype        sendTime);

void
DisPrintCommEffectsResult(
    Node* node, const DisSimulatedMsgInfo& smInfo, const char* resultString);

// Utility

DisRadio*
DisGetRadioPtr(const DisEntityId& entityId, unsigned short radioId);

void
DisPreparePduHeader(
    DisHeader& pduHeader,
    unsigned char pduType,
    unsigned char protocolFamily);

void
DisMallocEntities();

void
DisFreeEntities();

void
DisMallocRadios();

void
DisFreeRadios();

void
DisMallocNetworks();

void
DisFreeNetworks();

#endif /* DIS_MAIN_H */
