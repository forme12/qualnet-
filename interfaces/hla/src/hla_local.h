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

#ifndef HLA_LOCAL_H
#define HLA_LOCAL_H

// (2^31 - 1) / 3600, is used to compute or decode a DIS timestamp.
// The figure is rounded to a value just before further digits would be
// ignored.

const double g_hlaTimestampRatio = 596523.235277778;

static void
SetHlaMasterFlag(PartitionData* partitionData, NodeInput* nodeInput);

static void
HlaValidateConstants();

static void
HlaMallocFederateAmbassador();

static void
HlaMallocRtiAmbassador();

static void
HlaFreeFederateAmbassador();

static void
HlaFreeRtiAmbassador();

static void
HlaReadHlaParameters(PartitionData* partitionData, NodeInput* nodeInput);

static void
HlaReadRprFile(PartitionData* partitionData, NodeInput* nodeInput);

static void
HlaReadNetworksFile(PartitionData* partitionData, NodeInput* nodeInput);

static void
HlaAssertParse(
    bool assertCondition,
    unsigned lineNo,
    char* filename,
    char* cause);

static void
HlaInitMessenger(PartitionData* partitionData, NodeInput* nodeInput);

static bool
HlaGetEntityId(NodeAddress nodeId, HlaEntityId& entityId);

static void
HlaRegisterCtrlCHandler();

static void
HlaHandleSigInt(int a);

static void
HlaWaitForFirstObjectDiscovery();

static void
HlaRequestAttributeUpdatesIfTime(double double_elapsedPhysicalTime);

static void
HlaMessengerResultFcnBody(Node* node, Message* msg, BOOL success);

static void
HlaSendProcessMsgNotification(
    Node* node, const HlaSimulatedMsgInfo& smInfo, unsigned short success);

static void
HlaProcessTimeoutEvent(Node* node, Message* msg);

static void
HlaSendTimeoutNotification(
    Node* node, const HlaOutstandingSimulatedMsgInfo& osmInfo);

static void
HlaSendDataIxn(const HlaDataIxnInfo& dataIxnInfo);

static void
HlaPrintCommEffectsResult(
    Node* node, const HlaSimulatedMsgInfo& smInfo, const char* resultString);

static unsigned
HlaNonNegativeIntKeyHashFcn(const void* key, unsigned numBuckets);

static bool
HlaIntKeysEqualFcn(const void* key1, const void* key2);

// Functions which call RTI services ------------------------------------------

static void
HlaCreateFederation();

static void
HlaJoinFederation();

static void
HlaGetObjectAndInteractionClassHandles();

static void
HlaSubscribeAndPublish();

static void
HlaRequestAttributeUpdatesForClass();

static void
HlaRequestAttributeUpdates();

static void
HlaLeaveFederation();

static void
HlaDestroyFederation();

#endif /* HLA_LOCAL_H */
