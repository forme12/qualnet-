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

#ifndef HLA_MAIN_H
#define HLA_MAIN_H

struct HlaInterfaceData;

const unsigned g_hlaMaxDstEntityIdsInTimeoutNotification
    = (g_hlaMaxOutgoingDataIxnDatumValueSize
       - sizeof(HlaEntityId)     // Source EntityID
       - sizeof(unsigned short)  // Radio index
       - sizeof(unsigned)        // Timestamp
       - sizeof(unsigned)        // Number of packets
       - sizeof(unsigned))       // Number of entities
      / (sizeof(HlaEntityId)     // Destination EntityID
         + sizeof(bool));        // Entity status

extern bool                     g_hlaActive;

extern HlaInterfaceData*        g_ifaceData;

void
HlaValidateConstants();

HlaInterfaceData *
HlaMallocHla(PartitionData* partitionData);

void
HlaMallocFederateAmbassador(HlaInterfaceData * ifaceData);

void
HlaMallocRtiAmbassador(HlaInterfaceData* ifaceData);

void
HlaMallocRadios(HlaInterfaceData* ifaceData);

void
HlaMallocEntities(HlaInterfaceData * ifaceData);

void
HlaMallocNetworks(HlaInterfaceData * ifaceData);

void
HlaFreeHla(HlaInterfaceData * ifaceData);

void
HlaFreeFederateAmbassador(HlaInterfaceData * ifaceData);

void
HlaFreeRtiAmbassador(HlaInterfaceData * ifaceData);

void
HlaFreeEntities(HlaInterfaceData * ifaceData);

void
HlaFreeRadios(HlaInterfaceData * ifaceData);

void
HlaFreeNetworks(HlaInterfaceData * ifaceData);

void
HlaFreeOutstandingSimulatedMsgInfoHashes(HlaInterfaceData * ifaceData);

void
HlaInitHlaVariable(EXTERNAL_Interface* iface, HlaInterfaceData * ifaceData);

void
HlaReadParameters(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput);

void
HlaReadEntitiesFile(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput);

void
HlaReadRadiosFile(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput);

void
HlaReadNetworksFile(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput);

void
HlaMapDefaultDstRadioPtrsToRadios(HlaInterfaceData * ifaceData);

void
HlaMapHierarchyIds(HlaInterfaceData * ifaceData);

void
HlaInitMessenger(PartitionData* partitionData, NodeInput* nodeInput);

void
HlaCreateFederation(HlaInterfaceData * ifaceData);

void
HlaJoinFederation(HlaInterfaceData * ifaceData);

void
HlaRegisterCtrlCHandler();

void
HlaHandleSigInt(int a);

void
HlaEndSimulation(HlaInterfaceData * ifaceData);

void
HlaGetObjectAndInteractionClassHandles(HlaInterfaceData * ifaceData);

void
HlaSubscribeAndPublish(HlaInterfaceData * ifaceData);

void
HlaWaitForFirstObjectDiscovery(HlaInterfaceData * ifaceData);

void
HlaCheckRequestAttributeUpdates(HlaInterfaceData * ifaceData);

void
HlaRequestAttributeUpdatesForClass();

void
HlaRequestAttributeUpdates(HlaInterfaceData * ifaceData);

double
HlaGetElapsedPhysicalTime (HlaInterfaceData * ifaceData);

void
HlaLeaveFederation(HlaInterfaceData * ifaceData);

void
HlaDestroyFederation(HlaInterfaceData * ifaceData);

#ifdef MILITARY_RADIOS_LIB
void
HlaProcessSendRtssEvent(Node* node, Message* msg);
#endif /* MILITARY_RADIOS_LIB */

void
HlaAppMessengerResultFcn(Node* node, Message* msg, BOOL success);

void
HlaAppMessengerResultFcnBody(Node* node, Message* msg, BOOL success);

void
HlaSendProcessMsgNotification(
    Node* node,
    const HlaSimulatedMsgInfo& smInfo,
    const HlaOutstandingSimulatedMsgInfo& osmInfo,
    unsigned short success);

void
HlaAppProcessTimeoutEvent(Node* node, Message* msg);

void
HlaSendTimeoutNotification(
    Node* node,
    const HlaSimulatedMsgInfo& smInfo,
    const HlaOutstandingSimulatedMsgInfo& osmInfo);

#ifdef MILITARY_RADIOS_LIB
void
HlaSendRtssNotification(Node* node);

void
HlaPrintSentRtssNotification(Node* node);
#endif /* MILITARY_RADIOS_LIB */

void
HlaSendDataIxn(HlaInterfaceData * ifaceData,
    const HlaDataIxnInfo& dataIxnInfo);

void
HlaPrintCommEffectsResult(HlaInterfaceData * ifaceData,
    Node* node, const HlaSimulatedMsgInfo& smInfo, const char* resultString);

void
HlaAppProcessChangeMaxTxPowerEvent(Node* node, Message* msg);

void
HlaSendNodeIdDescriptionNotifications(HlaInterfaceData * ifaceData);

void
HlaSendNodeIdDescriptionNotification(const HlaRadio& radio);

void
HlaSendMetricDefinitionNotifications(HlaInterfaceData * ifaceData);

void
HlaSendMetricDefinitionNotification(const MetricData& metric, int metricLayer);

void
HlaScheduleCheckMetricUpdate(HlaInterfaceData * ifaceData);

void
HlaAppProcessCheckMetricUpdateEvent(Node* node, Message* msg);

void
HlaSendMetricUpdateNotification(HlaInterfaceData* ifaceData);

void
HlaAppProcessMoveHierarchy(Node* node, Message* msg);

void
HlaDetermineStringDatumLengthThenSendCommentIxn(
    HlaInterfaceData * ifaceData,
    HlaCommentIxnInfo& commentIxnInfo);

void
HlaSendCommentIxn(HlaInterfaceData * ifaceData,
    const HlaCommentIxnInfo& commentIxnInfo);

#endif /* HLA_MAIN_H */
