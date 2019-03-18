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

const unsigned g_hlaMaxDstEntityIdsInTimeoutNotification
    = (g_hlaMaxOutgoingDataIxnDatumValueSize
       - sizeof(HlaEntityId)     // Source EntityID
       - sizeof(unsigned short)  // Radio index
       - sizeof(unsigned)        // Timestamp
       - sizeof(unsigned)        // Number of packets
       - sizeof(unsigned))       // Number of entities
      / (sizeof(HlaEntityId)     // Destination EntityID
         + sizeof(bool));        // Entity status

class HlaFedAmb;

extern HlaFedAmb*          g_hlaFedAmb;
extern RTI::RTIambassador* g_hlaRtiAmb;

void
HlaValidateConstants();

void
HlaMallocFederateAmbassador();

void
HlaMallocRtiAmbassador();

void
HlaFreeFederateAmbassador();

void
HlaFreeRtiAmbassador();

void
HlaReadEntitiesFile();

void
HlaReadRadiosFile();

void
HlaReadNetworksFile();

unsigned
HlaGetNumLinesInFile(const char* path);

void
HlaCreateFederation();

void
HlaJoinFederation();

void
HlaGetObjectAndInteractionClassHandles();

void
HlaSubscribeAndPublish();

void
HlaLeaveFederation();

void
HlaDestroyFederation();

// TODO-HIGH:  Sort.

void
HlaRegisterObjects();

void
HlaUpdateAttributes();

void
HlaAssignObjectNames();

void
HlaPrepareMessageString(
    HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo,
    const HlaEntity* dstEntityPtr,
    bool isData,
    unsigned msgSize,
    double timeoutDelay);

void
HlaSendApplicationSpecificRadioSignalIxn(
    const HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo);

#endif /* HLA_MAIN_H */
