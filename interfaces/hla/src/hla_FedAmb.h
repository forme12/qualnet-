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

#ifndef HLA_FEDAMB_H
#define HLA_FEDAMB_H

#include "hla_shared.h"

const unsigned g_hlaFederationNameBufSize = 64;
const unsigned g_hlaFederateNameBufSize   = 64;
const unsigned g_hlaPathBufSize            = 256;

class HlaInterfaceData;

class HlaFedAmb : public RTI::FederateAmbassador
{
public:
    char        m_federationName[g_hlaFederationNameBufSize];
    char        m_fedFilePath[g_hlaPathBufSize];
    char        m_federateName[g_hlaFederationNameBufSize];
    HlaInterfaceData *      m_ifaceData;

    // Objects.

    RTI::ObjectClassHandle      m_baseEntityHandle;
    RTI::AttributeHandle        m_beEntityIdentifierHandle;
    RTI::AttributeHandle        m_beOrientationHandle;
    RTI::AttributeHandle        m_beVelocityVectorHandle;
    RTI::AttributeHandle        m_beWorldLocationHandle;

    RTI::ObjectClassHandle      m_physicalEntityHandle;
    RTI::AttributeHandle        m_peDamageStateHandle;
    RTI::AttributeHandle        m_peMarkingHandle;

    RTI::ObjectClassHandle      m_embeddedSystemHandle;
    RTI::AttributeHandle        m_esEntityIdentifierHandle;

    RTI::ObjectClassHandle      m_radioTransmitterHandle;
    RTI::AttributeHandle        m_rtRadioIndexHandle;
    RTI::AttributeHandle        m_rtTransmitterOperationalStatusHandle;

    // Interactions.

    RTI::InteractionClassHandle m_commentHandle;
    RTI::ParameterHandle        m_commentOriginatingEntityHandle;
    RTI::ParameterHandle        m_commentReceivingEntityHandle;
    RTI::ParameterHandle        m_commentVariableDatumSetHandle;

    RTI::InteractionClassHandle m_dataHandle;
    RTI::ParameterHandle        m_dataOriginatingEntityHandle;
    RTI::ParameterHandle        m_dataReceivingEntityHandle;
    RTI::ParameterHandle        m_dataRequestIdentifierHandle;
    RTI::ParameterHandle        m_dataFixedDatumsHandle;
    RTI::ParameterHandle        m_dataVariableDatumSetHandle;

    RTI::InteractionClassHandle m_applicationSpecificRadioSignalHandle;
    RTI::ParameterHandle        m_asrsHostRadioIndexHandle;
    RTI::ParameterHandle        m_asrsDataRateHandle;
    RTI::ParameterHandle        m_asrsSignalDataLengthHandle;
    RTI::ParameterHandle        m_asrsSignalDataHandle;
    RTI::ParameterHandle        m_asrsTacticalDataLinkTypeHandle;
    RTI::ParameterHandle        m_asrsTdlMessageCountHandle;
    RTI::ParameterHandle        m_asrsUserProtocolIdHandle;

    HlaFedAmb(HlaInterfaceData * ifaceData);
    ~HlaFedAmb() throw(RTI::FederateInternalError);

    // Federation management services.

    void
    synchronizationPointRegistrationSucceeded(
        const char* label)
    throw (
        RTI::FederateInternalError);

    void
    synchronizationPointRegistrationFailed(
        const char* label)
    throw (
        RTI::FederateInternalError);

    void
    announceSynchronizationPoint(
        const char* label,
        const char* tag)
    throw (
        RTI::FederateInternalError);

    void
    federationSynchronized(
        const char* label)
    throw (
        RTI::FederateInternalError);

    void
    initiateFederateSave(
        const char* label)
    throw (
        RTI::UnableToPerformSave,
        RTI::FederateInternalError);

    void
    federationSaved()
    throw (
        RTI::FederateInternalError);

    void
    federationNotSaved()
    throw (
        RTI::FederateInternalError);

    void
    requestFederationRestoreSucceeded(
        const char* label)
    throw (
        RTI::FederateInternalError);

    void
    requestFederationRestoreFailed(
        const char* label)
    throw (
        RTI::FederateInternalError);

    void
    requestFederationRestoreFailed(
        const char* label,
        const char* reason)
    throw (
        RTI::FederateInternalError);

    void
    federationRestoreBegun()
    throw (
        RTI::FederateInternalError);

    void
    initiateFederateRestore(
        const char*               label,
              RTI::FederateHandle handle)
    throw (
        RTI::SpecifiedSaveLabelDoesNotExist,
        RTI::CouldNotRestore,
        RTI::FederateInternalError);

    void
    federationRestored()
    throw (
        RTI::FederateInternalError);

    void
    federationNotRestored()
    throw (
        RTI::FederateInternalError);

    // Declaration management services.

    void
    startRegistrationForObjectClass(
              RTI::ObjectClassHandle theClass)
    throw (
        RTI::ObjectClassNotPublished,
        RTI::FederateInternalError);

    void
    stopRegistrationForObjectClass(
              RTI::ObjectClassHandle theClass)
    throw (
        RTI::ObjectClassNotPublished,
        RTI::FederateInternalError);

    void
    turnInteractionsOn(
              RTI::InteractionClassHandle theHandle)
    throw (
        RTI::InteractionClassNotPublished,
        RTI::FederateInternalError);

    void
    turnInteractionsOff(
              RTI::InteractionClassHandle theHandle)
    throw (
        RTI::InteractionClassNotPublished,
        RTI::FederateInternalError);

    // Object management services.

    void
    discoverObjectInstance(
              RTI::ObjectHandle      theObject,
              RTI::ObjectClassHandle theObjectClass,
        const char*                  theTag)
    throw (
        RTI::CouldNotDiscover,
        RTI::ObjectClassNotKnown,
        RTI::FederateInternalError);

    void
    reflectAttributeValues(
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
        const RTI::FedTime&                     theTime,
        const char*                             theTag,
              RTI::EventRetractionHandle        theHandle)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateOwnsAttributes,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError);

    void
    reflectAttributeValues(
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
        const char*                             theTag)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateOwnsAttributes,
        RTI::FederateInternalError);

    void
    receiveInteraction(
              RTI::InteractionClassHandle       theInteraction,
        const RTI::ParameterHandleValuePairSet& theParameters,
        const RTI::FedTime&                     theTime,
        const char*                             theTag,
              RTI::EventRetractionHandle        theHandle)
    throw (
        RTI::InteractionClassNotKnown,
        RTI::InteractionParameterNotKnown,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError);

    void
    receiveInteraction(
              RTI::InteractionClassHandle       theInteraction,
        const RTI::ParameterHandleValuePairSet& theParameters,
        const char*                             theTag)
    throw (
        RTI::InteractionClassNotKnown,
        RTI::InteractionParameterNotKnown,
        RTI::FederateInternalError);

    void
    removeObjectInstance(
              RTI::ObjectHandle          theObject,
        const RTI::FedTime&              theTime,
        const char*                      theTag,
              RTI::EventRetractionHandle theHandle)
    throw (
        RTI::ObjectNotKnown,
        RTI::InvalidFederationTime,
        RTI::FederateInternalError);

    void
    removeObjectInstance(
              RTI::ObjectHandle theObject,
        const char*             theTag)
    throw (
        RTI::ObjectNotKnown,
        RTI::FederateInternalError);

    void
    attributesInScope(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateInternalError);

    void
    attributesOutOfScope(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateInternalError);

    void
    provideAttributeValueUpdate(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeNotOwned,
        RTI::FederateInternalError);

    void
    turnUpdatesOnForObjectInstance(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotOwned,
        RTI::FederateInternalError);

    void
    turnUpdatesOffForObjectInstance(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotOwned,
        RTI::FederateInternalError);

    // Ownership management services.

    void
    requestAttributeOwnershipAssumption(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& offeredAttributes,
        const char*                    theTag)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeAlreadyOwned,
        RTI::AttributeNotPublished,
        RTI::FederateInternalError);

    void
    attributeOwnershipDivestitureNotification(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& releasedAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeNotOwned,
        RTI::AttributeDivestitureWasNotRequested,
        RTI::FederateInternalError);

    void
    attributeOwnershipAcquisitionNotification(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& securedAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeAcquisitionWasNotRequested,
        RTI::AttributeAlreadyOwned,
        RTI::AttributeNotPublished,
        RTI::FederateInternalError);

    void
    attributeOwnershipUnavailable(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeAlreadyOwned,
        RTI::AttributeAcquisitionWasNotRequested,
        RTI::FederateInternalError);

    void
    requestAttributeOwnershipRelease(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& candidateAttributes,
        const char*                    theTag)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeNotOwned,
        RTI::FederateInternalError);

    void
    confirmAttributeOwnershipAcquisitionCancellation(
              RTI::ObjectHandle        theObject,
        const RTI::AttributeHandleSet& theAttributes)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::AttributeAlreadyOwned,
        RTI::AttributeAcquisitionWasNotCanceled,
        RTI::FederateInternalError);

    void
    informAttributeOwnership(
              RTI::ObjectHandle    theObject,
              RTI::AttributeHandle theAttribute,
              RTI::FederateHandle  theOwner)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateInternalError);

    void
    attributeIsNotOwned(
              RTI::ObjectHandle    theObject,
              RTI::AttributeHandle theAttribute)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateInternalError);

    void
    attributeOwnedByRTI(
              RTI::ObjectHandle    theObject,
              RTI::AttributeHandle theAttribute)
    throw (
        RTI::ObjectNotKnown,
        RTI::AttributeNotKnown,
        RTI::FederateInternalError);

    // Time management services.

    void
    timeRegulationEnabled(
        const RTI::FedTime& theTime)
    throw (
        RTI::InvalidFederationTime,
        RTI::EnableTimeRegulationWasNotPending,
        RTI::FederateInternalError);

    void
    timeConstrainedEnabled(
        const RTI::FedTime& theTime)
    throw (
        RTI::InvalidFederationTime,
        RTI::EnableTimeConstrainedWasNotPending,
        RTI::FederateInternalError);

    void
    timeAdvanceGrant(
        const RTI::FedTime& theTime)
    throw (
        RTI::InvalidFederationTime,
        RTI::TimeAdvanceWasNotInProgress,
        RTI::FederateInternalError);

    void
    requestRetraction(
              RTI::EventRetractionHandle theHandle)
    throw (
        RTI::EventNotKnown,
        RTI::FederateInternalError);

    // Functions not defined in the base class.

private:
    void
    ReflectEntityAttributes(
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes);

    void
    ReflectEntityEntityId(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectEntityMarking(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectEntityOrientation(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectEntityWorldLocation(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectEntityVelocityVector(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectEntityDamageState(
              HlaEntityObject&                  eo,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectRadioAttributes(
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes);

    void
    ReflectRadioEntityId(
              HlaRadioObject&                   ro,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectRadioRadioIndex(
              HlaRadioObject&                   ro,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReflectRadioTransmitterOperationalStatus(
              HlaRadio&                         radio,
              RTI::ObjectHandle                 theObject,
        const RTI::AttributeHandleValuePairSet& theAttributes,
              unsigned                          index);

    void
    ReceiveApplicationSpecificRadioSignalIxn(
        const RTI::ParameterHandleValuePairSet& theParameters);

    bool
    ReadApplicationSpecificRadioSignalIxnUserProtocolIdParameter(
        const RTI::ParameterHandleValuePairSet& theParameters,
        HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo);

    bool
    ReadApplicationSpecificRadioSignalIxnOtherParameters(
        const RTI::ParameterHandleValuePairSet& theParameters,
        HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo);

#ifdef MILITARY_RADIOS_LIB
    void
    ScheduleRtssNotification(
        const HlaRadio& radio,
        clocktype delay);
#endif /* MILITARY_RADIOS_LIB */

    void
    ScheduleMobilityEventIfNecessary(HlaEntity& entity);

    void
    ProcessCommEffectsRequest(
        HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo);

    bool
    ParseMsgString(
        char* msgString,
        const HlaEntity*& dstEntityPtr,
        unsigned& dataMsgSize,
        clocktype& voiceMsgDuration,
        bool& isVoice,
        clocktype& timeoutDelay,
        unsigned& timestamp);

    void
    SendSimulatedMsgUsingMessenger(
        const HlaSimulatedMsgInfo& smInfo,
        const HlaEntity* dstEntityPtr,
        double requestedDataRate,
        unsigned dataMsgSize,
        clocktype voiceMsgDuration,
        bool isVoice,
        clocktype timeoutDelay,
        bool unicast,
        const HlaRadio* dstRadioPtr,
        clocktype sendDelay);

    void
    ScheduleTimeout(
        const HlaSimulatedMsgInfo& smInfo,
        clocktype timeoutDelay,
        clocktype sendDelay);

    void
    StoreOutstandingSimulatedMsgInfo(
        const HlaSimulatedMsgInfo& smInfo,
        const HlaEntity* dstEntitPtr,
        unsigned timestamp,
        clocktype sendTime);

    void
    ReceiveDataIxn(
        const RTI::ParameterHandleValuePairSet& theParameters);

    bool
    ReadDataIxnParameters(
        const RTI::ParameterHandleValuePairSet& theParameters,
        HlaDataIxnInfo& dataIxnInfo);

    void
    PrintCommEffectsRequestProcessed(
        const HlaRadio& srcRadio,
        const HlaEntity* dstEntityPtr,
        bool unicast,
        const HlaRadio* dstRadioPtr,
        clocktype sendTime);

    void
    ProcessTerminateCesRequest();

    void
    ScheduleChangeInterfaceState(
        const HlaEntity& entity,
        bool enableInterfaces,
        clocktype delay);

    void
    ScheduleChangeInterfaceState(
        const HlaRadio& radio, bool enableInterface);

    void
    ScheduleChangeMaxTxPower(
        Node* node,
        const HlaEntity& entity,
        unsigned damageState,
        clocktype delay);
};//class HlaFedAmb//

#endif /* HLA_FEDAMB_H */
