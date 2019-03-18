// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#ifndef _SNT_HLA_FED_AMB_H_
#define _SNT_HLA_FED_AMB_H_

#include <RTI.hh>
#include <map>

namespace SNT_HLA
{

class Interaction;
class HLAClass;
class HLAObject;

class FedAmb : public RTI::FederateAmbassador
{
    private:
    public:
        RTI::RTIambassador *rti;
        std::map<RTI::InteractionClassHandle, Interaction*> allInteractionClasses;
        std::map<RTI::ObjectClassHandle, HLAClass*> allObjectClasses;
        std::map<RTI::ObjectHandle, HLAObject*> allObjects;
        Interaction* findInteractionClass(RTI::InteractionClassHandle theHandle);
        HLAClass* findObjectClass(RTI::ObjectClassHandle theClass);
        HLAClass* findObjectClass(const std::string &className);
        HLAObject* findObject(RTI::ObjectHandle theObject);
        bool federationCreated;
        bool federationJoined;
        bool update_occurred;
        int numUpdatesReceived;

    public:
        FedAmb() : rti(0), numUpdatesReceived(0) {}
        void Init(const std::string& federationName, const std::string& fedPath, const std::string& federateName) throw (RTI::Exception);
        void Cleanup(const std::string& federationName) throw (RTI::Exception);
        void Subscribe(HLAClass* cls) throw (RTI::Exception);
        void Collect(int timeout);
        void Tick();
        virtual void synchronizationPointRegistrationSucceeded(const char *label) throw (RTI::FederateInternalError);
        virtual void synchronizationPointRegistrationFailed(const char *label) throw (RTI::FederateInternalError);
        virtual void announceSynchronizationPoint(const char *label, const char *tag) throw (RTI::FederateInternalError);
        virtual void federationSynchronized(const char *label) throw (RTI::FederateInternalError);
        virtual void initiateFederateSave(const char *label) throw (RTI::UnableToPerformSave, RTI::FederateInternalError);
        virtual void federationSaved() throw (RTI::FederateInternalError);
        virtual void federationNotSaved() throw (RTI::FederateInternalError);
        virtual void requestFederationRestoreSucceeded(const char *label) throw (RTI::FederateInternalError);
        virtual void requestFederationRestoreFailed(const char *label, const char *reason) throw (RTI::FederateInternalError);
        virtual void federationRestoreBegun() throw (RTI::FederateInternalError);
        virtual void initiateFederateRestore(const char *label, RTI::FederateHandle handle) throw (RTI::SpecifiedSaveLabelDoesNotExist, RTI::CouldNotRestore, RTI::FederateInternalError);
        virtual void federationRestored() throw (RTI::FederateInternalError);
        virtual void federationNotRestored() throw (RTI::FederateInternalError); 
        virtual void startRegistrationForObjectClass(RTI::ObjectClassHandle theClass) throw (RTI::ObjectClassNotPublished, RTI::FederateInternalError);
        virtual void stopRegistrationForObjectClass(RTI::ObjectClassHandle theClass) throw (RTI::ObjectClassNotPublished, RTI::FederateInternalError);
        virtual void turnInteractionsOn(RTI::InteractionClassHandle theHandle) throw (RTI::InteractionClassNotPublished, RTI::FederateInternalError);
        virtual void turnInteractionsOff(RTI::InteractionClassHandle theHandle) throw (RTI::InteractionClassNotPublished, RTI::FederateInternalError);
        virtual void discoverObjectInstance(RTI::ObjectHandle theObject, RTI::ObjectClassHandle theObjectClass, const char * theObjectName) throw (RTI::CouldNotDiscover, RTI::ObjectClassNotKnown, RTI::FederateInternalError);
        virtual void reflectAttributeValues(RTI::ObjectHandle theObject, const RTI::AttributeHandleValuePairSet& theAttributes, const RTI::FedTime& theTime, const char *theTag, RTI::EventRetractionHandle theHandle) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes, RTI::InvalidFederationTime, RTI::FederateInternalError);
        virtual void reflectAttributeValues(RTI::ObjectHandle theObject, const RTI::AttributeHandleValuePairSet& theAttributes, const char *theTag) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateOwnsAttributes, RTI::FederateInternalError);
        virtual void receiveInteraction(RTI::InteractionClassHandle theInteraction, const RTI::ParameterHandleValuePairSet& theParameters, const RTI::FedTime& theTime, const char *theTag, RTI::EventRetractionHandle theHandle) throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError);
        virtual void receiveInteraction(RTI::InteractionClassHandle theInteraction, const RTI::ParameterHandleValuePairSet& theParameters, const char *theTag) throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::FederateInternalError);
        virtual void removeObjectInstance(RTI::ObjectHandle theObject, const RTI::FedTime& theTime, const char *theTag, RTI::EventRetractionHandle theHandle) throw (RTI::ObjectNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError);
        virtual void removeObjectInstance(RTI::ObjectHandle theObject, const char *theTag) throw (RTI::ObjectNotKnown, RTI::FederateInternalError);
        virtual void attributesInScope(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError);
        virtual void attributesOutOfScope(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError);
        virtual void provideAttributeValueUpdate(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError);
        virtual void turnUpdatesOnForObjectInstance(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError); 
        virtual void turnUpdatesOffForObjectInstance(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError); 
        virtual void requestAttributeOwnershipAssumption(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& offeredAttributes, const char *theTag) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError); 
        virtual void attributeOwnershipDivestitureNotification(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& releasedAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::AttributeDivestitureWasNotRequested, RTI::FederateInternalError);
        virtual void attributeOwnershipAcquisitionNotification(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& securedAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAcquisitionWasNotRequested, RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError); 
        virtual void attributeOwnershipUnavailable(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotDefined, RTI::AttributeAlreadyOwned, RTI::AttributeAcquisitionWasNotRequested, RTI::FederateInternalError); 
        virtual void requestAttributeOwnershipRelease(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& candidateAttributes, const char *theTag) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError); 
        virtual void confirmAttributeOwnershipAcquisitionCancellation(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotDefined, RTI::AttributeAlreadyOwned, RTI::AttributeAcquisitionWasNotCanceled, RTI::FederateInternalError); 
        virtual void informAttributeOwnership(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute, RTI::FederateHandle theOwner) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError); 
        virtual void attributeIsNotOwned(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError);
        virtual void attributeOwnedByRTI(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError);
        virtual void timeRegulationEnabled(const RTI::FedTime& theFederateTime) throw (RTI::InvalidFederationTime, RTI::EnableTimeRegulationWasNotPending, RTI::FederateInternalError);
        virtual void timeConstrainedEnabled(const RTI::FedTime& theFederateTime) throw (RTI::InvalidFederationTime, RTI::EnableTimeConstrainedWasNotPending, RTI::FederateInternalError);
        virtual void timeAdvanceGrant(const RTI::FedTime& theTime) throw (RTI::InvalidFederationTime, RTI::TimeAdvanceWasNotInProgress, RTI::FederationTimeAlreadyPassed, RTI::FederateInternalError);
        virtual void requestRetraction(RTI::EventRetractionHandle theHandle) throw (RTI::EventNotKnown, RTI::FederateInternalError);
    };
};

#endif
