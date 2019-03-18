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

// iostream.h is needed to support DMSO RTI 1.3v6 (non-NG).

#ifdef NOT_RTI_NG
#include <iostream.h>
#else /* NOT_RTI_NG */
#include <iostream>
using namespace std;
#endif /* NOT_RTI_NG */

#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <climits>
#include <cassert>
#include <cerrno>

#include "api.h"
#include "WallClock.h"
#include "external_util.h"
#include "external_socket.h"
#include "phy_abstract.h"

#ifdef MILITARY_RADIOS_LIB
#include "mac_link16.h"
#endif /* MILITARY_RADIOS_LIB */

#include "app_messenger.h"
#include "dynamic.h"

#include "hla_shared.h"
#include "hla_main.h"
#include "hla_FedAmb.h"
#include "fedtime.hh"

// The RTI standard uses friend name injection of this function inside the
// RTIfedTime class to push the declaration into the global scope. However
// visibility of injected names was removed from the C++ standard a while
// back and more recent compilers do not support it. This declaration places
// the function in the global namespace so the HlaFedAmb members can find it.
RTI_STD::ostream RTI_EXPORT & operator<< (RTI_STD::ostream&, const RTI::FedTime&);

HlaFedAmb::HlaFedAmb(HlaInterfaceData* ifaceData) : m_ifaceData(ifaceData)
{
}

HlaFedAmb::~HlaFedAmb()
throw(RTI::FederateInternalError)
{
}

// Federation Management Services ---------------------------------------------

void
HlaFedAmb::synchronizationPointRegistrationSucceeded(
    const char* label)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: synchronizationPointRegistrationSucceeded" << endl;
}

void
HlaFedAmb::synchronizationPointRegistrationFailed(
    const char* label)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: synchronizationPointRegistrationFailed" << endl;
}

void
HlaFedAmb::announceSynchronizationPoint(
    const char* label,
    const char* tag)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: announceSynchronizationPoint" << endl;
}

void
HlaFedAmb::federationSynchronized(
    const char* label)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationSynchronized" << endl;
}

void
HlaFedAmb::initiateFederateSave(
    const char* label)
throw (
    RTI::UnableToPerformSave,
    RTI::FederateInternalError)
{
    cout << "FED: initiateFederateSave" << endl;
}

void
HlaFedAmb::federationSaved()
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationSaved" << endl;
}

void
HlaFedAmb::federationNotSaved()
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationNotSaved" << endl;
}

void
HlaFedAmb::requestFederationRestoreSucceeded(
    const char* label)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: requestFederationRestoreSucceeded" << endl;
}

void
HlaFedAmb::requestFederationRestoreFailed (
    const char* label)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: requestFederationRestoreFailed" << endl;
}

void
HlaFedAmb::requestFederationRestoreFailed (
    const char* label,
    const char* reason)
throw (
    RTI::FederateInternalError)
{
    cout << "FED: requestFederationRestoreFailed" << endl;
}

void
HlaFedAmb::federationRestoreBegun()
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationRestoreBegun" << endl;
}

void
HlaFedAmb::initiateFederateRestore(
    const char* label,
          RTI::FederateHandle handle)
throw (
    RTI::SpecifiedSaveLabelDoesNotExist,
    RTI::CouldNotRestore,
    RTI::FederateInternalError)
{
    cout << "FED: initiateFederateRestore" << endl;
}

void
HlaFedAmb::federationRestored()
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationRestored" << endl;
}

void
HlaFedAmb::federationNotRestored()
throw (
    RTI::FederateInternalError)
{
    cout << "FED: federationNotRestored" << endl;
}

// Declaration Management Services --------------------------------------------

void
HlaFedAmb::startRegistrationForObjectClass(
    RTI::ObjectClassHandle theClass)
throw (
    RTI::ObjectClassNotPublished,
    RTI::FederateInternalError)
{
    cout << "FED: startRegistrationForObjectClass" << endl;
}

void
HlaFedAmb::stopRegistrationForObjectClass(
    RTI::ObjectClassHandle theClass)
throw (
    RTI::ObjectClassNotPublished,
    RTI::FederateInternalError)
{
    cout << "FED: stopRegistrationForObjectClass" << endl;
}

void
HlaFedAmb::turnInteractionsOn(
    RTI::InteractionClassHandle theHandle)
throw (
    RTI::InteractionClassNotPublished,
    RTI::FederateInternalError)
{
    //cout << "FED: turnInteractionsOn" << endl;
}

void
HlaFedAmb::turnInteractionsOff(
    RTI::InteractionClassHandle theHandle)
throw (
    RTI::InteractionClassNotPublished,
    RTI::FederateInternalError)
{
    //cout << "FED: turnInteractionsOff" << endl;
}

// Object Management Services -------------------------------------------------

void
HlaFedAmb::discoverObjectInstance(
          RTI::ObjectHandle      theObject,
          RTI::ObjectClassHandle theObjectClass,
    const char*                  theObjectName)
throw (
    RTI::CouldNotDiscover,
    RTI::ObjectClassNotKnown,
    RTI::FederateInternalError)
{
    //cout << "FED: discoverObjectInstance" << endl;

    if (m_ifaceData->m_hla->dbl_referencePhysicalTime == 0.0)
    {
        m_ifaceData->m_hla->dbl_referencePhysicalTime
            = m_ifaceData->m_hla->partitionData->wallClock->getRealTimeAsDouble ();
    }

    if (theObjectClass == m_physicalEntityHandle
        || theObjectClass == m_radioTransmitterHandle)
    {
        if (m_ifaceData->m_hla->handleToObjectClass.find(theObject) !=
                m_ifaceData->m_hla->handleToObjectClass.end())
        {
            HlaReportError("Object already exists", __FILE__, __LINE__);
        }

        m_ifaceData->m_hla->handleToObjectClass[theObject] = theObjectClass;
    }
    else
    {
        HlaReportError("Unknown object class", __FILE__, __LINE__);
    }

    if (theObjectClass == m_physicalEntityHandle)
    {
        HlaEntityObject& eo =
            m_ifaceData->m_hla->handleToEntityObject[theObject];


        eo.entityIdExists    = false;
        eo.markingDataExists = false;
        eo.entityPtr         = NULL;
    }
    else
    if (theObjectClass == m_radioTransmitterHandle)
    {
        HlaVerify(theObjectName != NULL,
                  "Object name is NULL",
                  __FILE__, __LINE__);

        HlaRadioObject& ro =
            m_ifaceData->m_hla->handleToRadioObject[theObject];

        ro.entityIdExists   = false;
        ro.radioIndexExists = false;
        ro.radioPtr         = NULL;

        HlaVerify(strlen(theObjectName) + 1 <= sizeof(ro.objectName),
                  "Object name for RadioTransmitter object is too long."
                  "  Please increase the buffer size in source code.");

        strcpy(ro.objectName, theObjectName);

        HlaObjectNameToHandleMap::iterator it =
            m_ifaceData->m_hla->objectNameToHandle.find(ro.objectName);
        if (it != m_ifaceData->m_hla->objectNameToHandle.end())
        {
            HlaReportError("Object name for RadioTransmitter object already "
                           "exists", __FILE__, __LINE__);
        }
        m_ifaceData->m_hla->objectNameToHandle[ro.objectName] = theObject;
    }
}

void
HlaFedAmb::reflectAttributeValues (
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
    const RTI::FedTime&                     theTime,
    const char*                             theTag,
    RTI::EventRetractionHandle theHandle)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateOwnsAttributes,
    RTI::InvalidFederationTime,
    RTI::FederateInternalError)
{
    cout << "FED: reflectAttributeValues (TSO)" << endl;
}

void
HlaFedAmb::reflectAttributeValues(
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
    const char*                             theTag)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateOwnsAttributes,
    RTI::FederateInternalError)
{
    //cout << "FED: reflectAttributeValues (RO)" << endl;

    HlaHandleToObjectClassMap::iterator it =
        m_ifaceData->m_hla->handleToObjectClass.find(theObject);

    if (it == m_ifaceData->m_hla->handleToObjectClass.end())
    {
        HlaReportError("Tying to update attributes of an unknown object",
                       __FILE__, __LINE__);
    }

    const unsigned objectClass = it->second;

    if (objectClass == m_physicalEntityHandle)
    {
        ReflectEntityAttributes(theObject, theAttributes);
    }
    else
    if (objectClass == m_radioTransmitterHandle)
    {
        ReflectRadioAttributes(theObject, theAttributes);
    }
    else
    {
        HlaReportError("Unknown object class", __FILE__, __LINE__);
    }
}

void
HlaFedAmb::receiveInteraction(
          RTI::InteractionClassHandle       theInteraction,
    const RTI::ParameterHandleValuePairSet& theParameters,
    const RTI::FedTime&                     theTime,
    const char*                             theTag,
          RTI::EventRetractionHandle        theHandle)
throw (
    RTI::InteractionClassNotKnown,
    RTI::InteractionParameterNotKnown,
    RTI::InvalidFederationTime,
    RTI::FederateInternalError)
{
    cout << "FED: receiveInteraction (TSO) at " << theTime << endl;
}

void
HlaFedAmb::receiveInteraction(
          RTI::InteractionClassHandle       theInteraction,
    const RTI::ParameterHandleValuePairSet& theParameters,
    const char*                             theTag)
throw (
    RTI::InteractionClassNotKnown,
    RTI::InteractionParameterNotKnown,
    RTI::FederateInternalError)
{
    //cout << "FED: receiveInteraction (RO)" << endl;

    if (theInteraction == m_applicationSpecificRadioSignalHandle)
    {
        ReceiveApplicationSpecificRadioSignalIxn(theParameters);
    }
    else
    {
        if (m_ifaceData->m_hla->debug)
        {
            try
            {
                char* classNameString =
                    m_ifaceData->m_hlaRtiAmb->getInteractionClassName(theInteraction);

                assert(classNameString != NULL);

                cout << "FED: Received non-subscribed interaction "
                        << classNameString << endl;

                delete [] classNameString;
                classNameString = NULL;
            }
            catch (RTI::Exception& e)
            {
                cout << "FED: Received non-subscribed interaction, and can't get"
                        " class name (class handle = " << theInteraction << ")";
                cout << "FED: " << &e << endl;
            }
        }//if//
    }//if//
}

void
HlaFedAmb::removeObjectInstance(
          RTI::ObjectHandle theObject,
    const RTI::FedTime&     theTime,
    const char*             theTag,
    RTI::EventRetractionHandle theHandle)
throw (
    RTI::ObjectNotKnown,
    RTI::InvalidFederationTime,
    RTI::FederateInternalError)
{
    cout << "FED: removeObjectInstance (TSO)" << endl;
}

void
HlaFedAmb::removeObjectInstance(
          RTI::ObjectHandle theObject,
    const char*             theTag)
throw (
    RTI::ObjectNotKnown,
    RTI::FederateInternalError)
{
    //cout << "FED: removeObjectInstance (RO)" << endl;

    HlaHandleToObjectClassMap::iterator classIt =
        m_ifaceData->m_hla->handleToObjectClass.find(theObject);

    if (classIt == m_ifaceData->m_hla->handleToObjectClass.end())
    {
        HlaReportError("Tying to remove an object of unknown class",
                       __FILE__, __LINE__);
    }

    const unsigned objectClass = classIt->second;

    if (objectClass == m_physicalEntityHandle)
    {
        HlaHandleToEntityObjectMap::iterator entityIt =
            m_ifaceData->m_hla->handleToEntityObject.find(theObject);
        if (entityIt == m_ifaceData->m_hla->handleToEntityObject.end())
        {
            HlaReportError("Tying to remove an unknown entity object",
                           __FILE__, __LINE__);
        }
        const HlaEntityObject& eo = entityIt->second;

        if (eo.entityPtr != NULL)
        {
            HlaEntity& entity = *eo.entityPtr;

            assert(entity.mappedToHandle);
            entity.mappedToHandle = false;

            if (m_ifaceData->m_hla->debug)
            {
                cout << "FED: Removing entity (" << entity.entityIdString
                        << ") " << entity.markingData << endl;
            }
        }

        m_ifaceData->m_hla->handleToEntityObject.erase(entityIt);
    }
    else
    {
        HlaHandleToRadioObjectMap::iterator radioIt =
            m_ifaceData->m_hla->handleToRadioObject.find(theObject);
        if (radioIt == m_ifaceData->m_hla->handleToRadioObject.end())
        {
            HlaReportError("Tying to remove an unknown radio object",
                           __FILE__, __LINE__);
        }
        const HlaRadioObject& ro = radioIt->second;

        if (ro.radioPtr != NULL)
        {
            const HlaRadio& radio = *ro.radioPtr;
            assert(radio.entityPtr != NULL);
            const HlaEntity& entity = *radio.entityPtr;

            if (m_ifaceData->m_hla->debug)
            {
                cout << "FED: Removing node " << radio.node->nodeId
                        << " (" << entity.entityIdString << ", "
                        << radio.radioIndex << ") "
                        << entity.markingData << endl;
            }
        }

        HlaObjectNameToHandleMap::iterator handleIt =
            m_ifaceData->m_hla->objectNameToHandle.find(ro.objectName);
        m_ifaceData->m_hla->objectNameToHandle.erase(handleIt);

        m_ifaceData->m_hla->handleToRadioObject.erase(radioIt);
    }

     m_ifaceData->m_hla->handleToObjectClass.erase(classIt);
}

void
HlaFedAmb::attributesInScope(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: attributesInScope" << endl;
}

void
HlaFedAmb::attributesOutOfScope(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: attributesOutOfScope" << endl;
}

void
HlaFedAmb::provideAttributeValueUpdate(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeNotOwned,
    RTI::FederateInternalError)
{
    cout << "FED: provideAttributeValueUpdate" << endl;
}

void
HlaFedAmb::turnUpdatesOnForObjectInstance(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotOwned,
    RTI::FederateInternalError)
{
    cout << "FED: turnUpdatesOnForObjectInstance" << endl;
}

void
HlaFedAmb::turnUpdatesOffForObjectInstance (
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotOwned,
    RTI::FederateInternalError)
{
    cout << "FED: turnUpdatesOffForObjectInstance" << endl;
}

// Ownership Management Services ----------------------------------------------

void
HlaFedAmb::requestAttributeOwnershipAssumption(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& offeredAttributes,
    const char*                    theTag)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeAlreadyOwned,
    RTI::AttributeNotPublished,
    RTI::FederateInternalError)
{
    cout << "FED: requestAttributeOwnershipAssumption" << endl;
}

void
HlaFedAmb::attributeOwnershipDivestitureNotification(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& releasedAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeNotOwned,
    RTI::AttributeDivestitureWasNotRequested,
    RTI::FederateInternalError)
{
    cout << "FED: attributeOwnershipDivestitureNotification" << endl;
}

void
HlaFedAmb::attributeOwnershipAcquisitionNotification(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& securedAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeAcquisitionWasNotRequested,
    RTI::AttributeAlreadyOwned,
    RTI::AttributeNotPublished,
    RTI::FederateInternalError)
{
    cout << "FED: attributeOwnershipAcquisitionNotification" << endl;
}

void
HlaFedAmb::attributeOwnershipUnavailable(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeAlreadyOwned,
    RTI::AttributeAcquisitionWasNotRequested,
    RTI::FederateInternalError)
{
    cout << "FED: attributeOwnershipUnavailable" << endl;
}

void
HlaFedAmb::requestAttributeOwnershipRelease(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& candidateAttributes,
    const char*                    theTag)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeNotOwned,
    RTI::FederateInternalError)
{
    cout << "FED: requestAttributeOwnershipRelease" << endl;
}

void
HlaFedAmb::confirmAttributeOwnershipAcquisitionCancellation(
          RTI::ObjectHandle        theObject,
    const RTI::AttributeHandleSet& theAttributes)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::AttributeAlreadyOwned,
    RTI::AttributeAcquisitionWasNotCanceled,
    RTI::FederateInternalError)
{
    cout << "FED: confirmAttributeOwnershipAcquisitionCancellation" << endl;
}

void
HlaFedAmb::informAttributeOwnership(
    RTI::ObjectHandle    theObject,
    RTI::AttributeHandle theAttribute,
    RTI::FederateHandle  theOwner)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: informAttributeOwnership" << endl;
}

void
HlaFedAmb::attributeIsNotOwned(
    RTI::ObjectHandle    theObject,
    RTI::AttributeHandle theAttribute)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: attributeIsNotOwned" << endl;
}

void
HlaFedAmb::attributeOwnedByRTI(
    RTI::ObjectHandle    theObject,
    RTI::AttributeHandle theAttribute)
throw (
    RTI::ObjectNotKnown,
    RTI::AttributeNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: attributeOwnedByRTI" << endl;
}

// Time Management Services ---------------------------------------------------

void
HlaFedAmb::timeRegulationEnabled(
    const RTI::FedTime& theTime)
throw (
    RTI::InvalidFederationTime,
    RTI::EnableTimeRegulationWasNotPending,
    RTI::FederateInternalError)
{
    cout << "FED: timeRegulationEnabled to " << theTime << endl;
}

void
HlaFedAmb::timeConstrainedEnabled(
    const RTI::FedTime& theTime)
throw (
    RTI::InvalidFederationTime,
    RTI::EnableTimeConstrainedWasNotPending,
    RTI::FederateInternalError)
{
    cout << "FED: timeConstrainedEnabled to " << theTime << endl;
}

void
HlaFedAmb::timeAdvanceGrant(
    const RTI::FedTime& theTime)
throw (
    RTI::InvalidFederationTime,
    RTI::TimeAdvanceWasNotInProgress,
    RTI::FederateInternalError)
{
    cout << "FED: timeAdvanceGrant to " << theTime << endl;
}

void
HlaFedAmb::requestRetraction(
    RTI::EventRetractionHandle theHandle)
throw (
    RTI::EventNotKnown,
    RTI::FederateInternalError)
{
    cout << "FED: requestRetraction" << endl;
}

// Private functions not defined in the base class ----------------------------

// Public functions not defined in the base class -----------------------------

void
HlaFedAmb::ReflectEntityAttributes(
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes)
{
    HlaHandleToEntityObjectMap::iterator entityIt =
        m_ifaceData->m_hla->handleToEntityObject.find(theObject);

    HlaEntityObject& eo = entityIt->second;

    if (eo.entityIdExists && eo.markingDataExists && eo.entityPtr == NULL)
    {
        return;
    }

    if (eo.entityPtr == NULL)
    {
        assert(!eo.entityIdExists || !eo.markingDataExists);

        unsigned i;
        for (i = 0; i < theAttributes.size(); i++)
        {
            RTI::AttributeHandle ah = theAttributes.getHandle(i);

            if (ah == m_beEntityIdentifierHandle)
            {
                ReflectEntityEntityId(eo, theObject, theAttributes, i);

                if (eo.markingDataExists) { break; }
            }
            else
            if (ah == m_peMarkingHandle)
            {
                ReflectEntityMarking(eo, theObject, theAttributes, i);

                if (eo.entityIdExists) { break; }
            }
        }//for//

        if (i == theAttributes.size()) { return; }

        assert(eo.entityIdExists && eo.markingDataExists);

        HlaMarkingDataToEntityMap::iterator markIt
            = m_ifaceData->m_hla->markingDataToEntity.find(eo.markingData);
        if (markIt == m_ifaceData->m_hla->markingDataToEntity.end())
        {
            return;
        }

        HlaEntity *entity = markIt->second;

        if (entity->mappedToHandle)
        {
            char warningString[MAX_STRING_LENGTH];
            sprintf(
                warningString,
                "reflectAttributeValues(), two or more PhysicalEntity objects"
                " with duplicate MarkingData = %s", eo.markingData);

            HlaReportWarning(warningString);
            return;
        }

        eo.entityPtr = entity;

        entity->entityId = eo.entityId;
        HlaMakeEntityIdString(entity->entityId, entity->entityIdString);
        entity->mappedToHandle = true;

        if (m_ifaceData->m_hla->entityIdToEntity.find(entity->entityId) != 
            m_ifaceData->m_hla->entityIdToEntity.end())
        {
            char errorString[MAX_STRING_LENGTH];

            sprintf(
                errorString,
                "reflectAttributeValue(), two or more PhysicalEntity objects"
                " with duplicate EntityID = %s", entity->entityIdString);

            HlaReportError(errorString);
        };

        m_ifaceData->m_hla->entityIdToEntity[entity->entityId] = entity;

        if (m_ifaceData->m_hla->debug)
        {
            cout << "FED: Mapped to object (" << entity->entityIdString
                    << ") " << entity->markingData << endl;
        }
    }

    assert(eo.entityIdExists && eo.markingDataExists && eo.entityPtr != NULL);

    bool schedulePossibleMobilityEvent = false;

    unsigned i;
    for (i = 0; i < theAttributes.size(); i++)
    {
        RTI::AttributeHandle ah = theAttributes.getHandle(i);

        // Ordered by expected frequency of occurrence, highest first.

        if (ah == m_beOrientationHandle)
        {
            ReflectEntityOrientation(eo, theObject, theAttributes, i);
            schedulePossibleMobilityEvent = true;
        }
        else
        if (ah == m_beWorldLocationHandle)
        {
            ReflectEntityWorldLocation(eo, theObject, theAttributes, i);
            schedulePossibleMobilityEvent = true;
        }
        else
        if (ah == m_beVelocityVectorHandle)
        {
            ReflectEntityVelocityVector(eo, theObject, theAttributes, i);
            schedulePossibleMobilityEvent = true;
        }
        else
        if (ah == m_peDamageStateHandle)
        {
            ReflectEntityDamageState(eo, theObject, theAttributes, i);
        }
        else
        if (ah == m_beEntityIdentifierHandle)
        {
            // Ignore.
        }
        else
        if (ah == m_peMarkingHandle)
        {
            // Ignore.
        }
        else
        {
            HlaReportError("Unknown attribute handle", __FILE__, __LINE__);
        }
    }//for//

    if (schedulePossibleMobilityEvent)
    {
        HlaEntity& entity = *eo.entityPtr;
        ScheduleMobilityEventIfNecessary(entity);
    }
}

void
HlaFedAmb::ReflectEntityEntityId(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    RTI::ULong size = theAttributes.getValueLength(index);

    if (size != sizeof(eo.entityId))
    {
        HlaReportError("Unexpected attribute size", __FILE__, __LINE__);
    }

    theAttributes.getValue(index, (char*) &eo.entityId, size);
    HlaNtohEntityId(eo.entityId);
    eo.entityIdExists = true;
}

void
HlaFedAmb::ReflectEntityMarking(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    // Warning:  The Marking parameter's MarkingData field may not be null
    // terminated when received over HLA.

    RTI::ULong size;
    const char* marking = theAttributes.getValuePointer(index, size);
    assert(marking != NULL);

    // The code below should work for any value of size, including 0.

    unsigned numBytesToCopy;

    if (size <= g_hlaMarkingBufSize)
    {
        if (size > 0)
        {
            numBytesToCopy = size - 1;
        }
        else
        {
            numBytesToCopy = 0;
        }
    }
    else
    {
        numBytesToCopy = g_hlaMarkingBufSize - 1;
    }

    assert(numBytesToCopy <= sizeof(eo.markingData));

    memcpy(eo.markingData, &marking[1], numBytesToCopy);

    // Always null-terminate MarkingData when stored in QualNet.

    eo.markingData[numBytesToCopy] = 0;

    HlaTrimLeft(eo.markingData);
    HlaTrimRight(eo.markingData);

    eo.markingDataExists = true;
}

void
HlaFedAmb::ReflectEntityOrientation(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    assert(eo.entityPtr != NULL);

    RTI::ULong size = theAttributes.getValueLength(index);

    float nboOrientation[3];

    if (size != sizeof(nboOrientation))
    {
        HlaReportError("Unexpected attribute size", __FILE__, __LINE__);
    }

    theAttributes.getValue(index, (char*) nboOrientation, size);

    float float_orientationPsi   = nboOrientation[0];
    float float_orientationTheta = nboOrientation[1];
    float float_orientationPhi   = nboOrientation[2];

    EXTERNAL_ntoh(&float_orientationPsi, sizeof(float_orientationPsi));
    EXTERNAL_ntoh(&float_orientationTheta, sizeof(float_orientationTheta));
    EXTERNAL_ntoh(&float_orientationPhi, sizeof(float_orientationPhi));

    short azimuth;
    short elevation;

    HlaEntity& entity = *eo.entityPtr;

    HlaConvertRprFomOrientationToQualNetOrientation(
        entity.lat,
        entity.lon,
        float_orientationPsi,
        float_orientationTheta,
        float_orientationPhi,
        azimuth,
        elevation);

    assert(azimuth >= 0 && azimuth <= 359);
    assert(elevation >= -90 && elevation <= 90);

    entity.azimuth   = azimuth;
    entity.elevation = elevation;
}

void
HlaFedAmb::ReflectEntityWorldLocation(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    assert(eo.entityPtr != NULL);

    RTI::ULong size = theAttributes.getValueLength(index);

    double nboWorldLocation[3];

    if (size != sizeof(nboWorldLocation))
    {
        HlaReportError("Unexpected attribute size", __FILE__, __LINE__);
    }

    theAttributes.getValue(index, (char*) nboWorldLocation, size);

    double x = nboWorldLocation[0];
    double y = nboWorldLocation[1];
    double z = nboWorldLocation[2];

    EXTERNAL_ntoh(&x, sizeof(x));
    EXTERNAL_ntoh(&y, sizeof(y));
    EXTERNAL_ntoh(&z, sizeof(x));

    double lat;
    double lon;
    double alt;

    HlaConvertGccToLatLonAlt(x, y, z, lat, lon, alt);

    assert(lat >= -90.0 && lat <= 90.0);
    assert(lon >= -180.0 && lon <= 180.0);

    HlaEntity& entity = *eo.entityPtr;

    if (lat < m_ifaceData->m_hla->minLat || lat > m_ifaceData->m_hla->maxLat
        || lon < m_ifaceData->m_hla->minLon || lon > m_ifaceData->m_hla->maxLon)
    {
        return;
    }

    entity.x = x;
    entity.y = y;
    entity.z = z;

    entity.lat = lat;
    entity.lon = lon;
    entity.alt = alt;
}

void
HlaFedAmb::ReflectEntityVelocityVector(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    assert(eo.entityPtr != NULL);

    RTI::ULong size = theAttributes.getValueLength(index);

    float nboVelocityVector[3];

    if (size != sizeof(nboVelocityVector))
    {
        HlaReportError("Unexpected attribute size", __FILE__, __LINE__);
    }

    theAttributes.getValue(index, (char*) nboVelocityVector, size);

    float xVelocity = nboVelocityVector[0];
    float yVelocity = nboVelocityVector[1];
    float zVelocity = nboVelocityVector[2];

    EXTERNAL_ntoh(&xVelocity, sizeof(xVelocity));
    EXTERNAL_ntoh(&yVelocity, sizeof(yVelocity));
    EXTERNAL_ntoh(&zVelocity, sizeof(zVelocity));

    double speed
        = pow(pow((double) xVelocity, 2)
                + pow((double) yVelocity, 2)
                + pow((double) zVelocity, 2),
              0.5);

    assert(speed >= 0.0);

    HlaEntity& entity = *eo.entityPtr;
    entity.speed = speed;
}

void
HlaFedAmb::ReflectEntityDamageState(
          HlaEntityObject&                  eo,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    assert(eo.entityPtr != NULL);

    RTI::ULong size = theAttributes.getValueLength(index);

    unsigned damageState;

    if (size != sizeof(damageState))
    {
        HlaReportError("Unexpected attribute size", __FILE__, __LINE__);
    }

    theAttributes.getValue(index, (char*) &damageState, size);

    EXTERNAL_ntoh(&damageState, sizeof(damageState));

    if (m_ifaceData->m_hla->nawcGatewayCompatibility)
    {
        // NAWC Gateway copies the Entity State PDU Entity Appearance Record
        // field exactly.  This field must be parsed to the RPR-FOM DamageState
        // parameter.

        damageState &= 24;
        damageState >>= 3;
    }

    HlaVerify(damageState >= HLA_DAMAGE_STATE_MIN
              && damageState <= HLA_DAMAGE_STATE_MAX,
              "Unexpected attribute value",
              __FILE__, __LINE__);

    HlaEntity& entity = *eo.entityPtr;

    assert(entity.damageState >= HLA_DAMAGE_STATE_MIN
           && entity.damageState <= HLA_DAMAGE_STATE_MAX);

    if (entity.damageState == damageState) { return; }

    cout << "FED: (" << entity.entityIdString
            << ") " << entity.markingData << " DamageState = ";

    switch (damageState)
    {
        case HLA_DAMAGE_STATE_NO_DAMAGE:
            cout << "NoDamage";
            break;
        case HLA_DAMAGE_STATE_SLIGHT_DAMAGE:
            cout << "SlightDamage";
            break;
        case HLA_DAMAGE_STATE_MODERATE_DAMAGE:
            cout << "ModerateDamage";
            break;
        case HLA_DAMAGE_STATE_DESTROYED:
            cout << "Destroyed";
            break;
        default:
            assert(0);
    }

    cout << endl;

    if (entity.numRadioPtrs > 0)
    {
        // Get node pointer and delay.

        assert(entity.radioPtrs[0] != NULL);
        const HlaRadio& radio = *entity.radioPtrs[0];

        Node* node = radio.node;

        clocktype simTime = TIME_getSimTime(node);
        clocktype delay
            = MAX(EXTERNAL_QueryExternalTime(m_ifaceData->m_hla->iface) - simTime, 0);

        // Turn off/on interfaces.

        if (entity.damageState != HLA_DAMAGE_STATE_DESTROYED
            && damageState == HLA_DAMAGE_STATE_DESTROYED)
        {
            ScheduleChangeInterfaceState(entity, false, delay);
        }
        else
        if (entity.damageState == HLA_DAMAGE_STATE_DESTROYED
            && damageState != HLA_DAMAGE_STATE_DESTROYED)
        {
            ScheduleChangeInterfaceState(entity, true, delay);
        }

        // Schedule check for reducing TX power.

        if (entity.damageState < HLA_DAMAGE_STATE_SLIGHT_DAMAGE
            && damageState == HLA_DAMAGE_STATE_SLIGHT_DAMAGE)
        {
            // Less-damaged state -> SlightDamage.

            ScheduleChangeMaxTxPower(
                node, entity, HLA_DAMAGE_STATE_SLIGHT_DAMAGE, delay);
        }
        else
        if (entity.damageState < HLA_DAMAGE_STATE_MODERATE_DAMAGE
            && damageState == HLA_DAMAGE_STATE_MODERATE_DAMAGE)
        {
            // Less-damaged state -> ModerateDamage.

            ScheduleChangeMaxTxPower(
                node, entity, HLA_DAMAGE_STATE_MODERATE_DAMAGE, delay);
        }
        else
        if (entity.damageState < HLA_DAMAGE_STATE_DESTROYED
            && damageState == HLA_DAMAGE_STATE_DESTROYED)
        {
            // Less-damaged state -> Destroyed.

            ScheduleChangeMaxTxPower(
                node, entity, HLA_DAMAGE_STATE_DESTROYED, delay);
        }
        else
        if (entity.damageState != HLA_DAMAGE_STATE_NO_DAMAGE
            && damageState == HLA_DAMAGE_STATE_NO_DAMAGE)
        {
            // More-damaged state -> NoDamage.

            ScheduleChangeMaxTxPower(
                node, entity, HLA_DAMAGE_STATE_NO_DAMAGE, delay);
        }
        else
        if (entity.damageState > damageState)
        {
            // More-damaged state -> Less-damaged state (but not NoDamage).
            // Don't change max TX power.
        }
        else
        {
            assert(0);
        }
    }//if//

    entity.damageState = damageState;
}

void
HlaFedAmb::ReflectRadioAttributes(
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes)
{
    HlaHandleToRadioObjectMap::iterator radioIt =
        m_ifaceData->m_hla->handleToRadioObject.find(theObject);

    HlaRadioObject& ro = radioIt->second;

    if (ro.entityIdExists && ro.radioIndexExists && ro.radioPtr == NULL)
    {
        return;
    }

    if (ro.radioPtr == NULL)
    {
        assert(!ro.entityIdExists || !ro.radioIndexExists);

        unsigned i;
        for (i = 0; i < theAttributes.size(); i++)
        {
            RTI::AttributeHandle ah = theAttributes.getHandle(i);

            if (ah == m_esEntityIdentifierHandle)
            {
                ReflectRadioEntityId(ro, theObject, theAttributes, i);

                if (ro.radioIndexExists) { break; }
            }
            else
            if (ah == m_rtRadioIndexHandle)
            {
                ReflectRadioRadioIndex(ro, theObject, theAttributes, i);

                if (ro.entityIdExists) { break; }
            }
        }//for//

        if (i == theAttributes.size()) { return; }

        assert(ro.entityIdExists && ro.radioIndexExists);

        HlaEntityIdToEntityMap::iterator entityIt =
            m_ifaceData->m_hla->entityIdToEntity.find(ro.entityId);

        if (entityIt == m_ifaceData->m_hla->entityIdToEntity.end())
        {
            ro.entityIdExists = false;
            ro.radioIndexExists = false;
            return;
        }

        HlaEntity* entity = entityIt->second;

        HlaRadioKey radioKey;
        strcpy(radioKey.markingData, entity->markingData);
        radioKey.radioIndex = ro.radioIndex;

        HlaRadioKeyToRadioMap::iterator it =
            m_ifaceData->m_hla->radioKeyToRadio.find(radioKey);
        if (it == m_ifaceData->m_hla->radioKeyToRadio.end())
        {
            return;
        }

        HlaRadio *radio = it->second;

        ro.radioPtr = radio;
        radio->mappedToHandle = true;

        if (m_ifaceData->m_hla->debug)
        {
            const HlaEntity *entity = radio->entityPtr;

            cout << "FED: Assigned node " << radio->node->nodeId
                    << " (" << entity->entityIdString
                    << ", " << radio->radioIndex << ") "
                    << entity->markingData << endl << flush;
        }

#ifdef MILITARY_RADIOS_LIB
        int macProtocol;
        if (radio->node->partitionId == radio->node->partitionData->partitionId)
        {
            macProtocol =
                radio->node->macData[MAC_DEFAULT_INTERFACE]->macProtocol;
        }
        else
        {
            std::string protocolNumStr;
            std::string path;
            D_Hierarchy* h = &radio->node->partitionData->dynamicHierarchy;
            if (!h->IsEnabled ())
            {
                HlaReportError(
                    "DYNAMIC-ENABLED isn't set for the parallel simulation. As a result failed to determine MAC type for remote node",
                 __FILE__, __LINE__);
            }
            h->BuildNodeInterfaceIndexPath(radio->node, MAC_DEFAULT_INTERFACE,
                "macType", path);
            h->ReadAsString(path.c_str (), protocolNumStr);
            macProtocol = atoi (protocolNumStr.c_str());
        }

        if (macProtocol == MAC_PROTOCOL_TADIL_LINK11
            || macProtocol == MAC_PROTOCOL_TADIL_LINK16)
        {
            // Schedule the RTSS notification to be sent 1 nanosecond from
            // the current simulation time.
            // Why 1 nanosecond and not 0?
            // For a delay of 0, the RTSS will be sent immediately, i.e.,
            // while still executing a reflectAttributeValues() callback,
            // which is illegal in HLA.

            clocktype delay = 1 * NANO_SECOND;

            ScheduleRtssNotification(*radio, delay);
        }
#endif /* MILITARY_RADIOS_LIB */
    }

    assert(ro.entityIdExists && ro.radioIndexExists && ro.radioPtr != NULL);

    unsigned i;
    for (i = 0; i < theAttributes.size(); i++)
    {
        RTI::AttributeHandle ah = theAttributes.getHandle(i);

        // Ordered by expected frequency of occurrence, highest first.

        if (ah == m_rtTransmitterOperationalStatusHandle)
        {
            ReflectRadioTransmitterOperationalStatus(
                *ro.radioPtr, theObject, theAttributes, i);
        }
        else
        if (ah == m_esEntityIdentifierHandle)
        {
            // Ignore.
        }
        else
        if (ah == m_rtRadioIndexHandle)
        {
            // Ignore.
        }
        else
        {
            HlaReportError("Unknown attribute handle", __FILE__, __LINE__);
        }
    }//for//
}

void
HlaFedAmb::ReflectRadioEntityId(
          HlaRadioObject&                   ro,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    RTI::ULong size = theAttributes.getValueLength(index);

    HlaVerify(size == sizeof(ro.entityId),
              "Unexpected attribute size",
              __FILE__, __LINE__);

    theAttributes.getValue(index, (char*) &ro.entityId, size);
    HlaNtohEntityId(ro.entityId);
    ro.entityIdExists = true;
}

void
HlaFedAmb::ReflectRadioRadioIndex(
          HlaRadioObject&                   ro,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    RTI::ULong size = theAttributes.getValueLength(index);

    HlaVerify(size == sizeof(unsigned char)
                || size == sizeof(ro.radioIndex)
                || size == sizeof(unsigned),
              "Unexpected attribute size",
              __FILE__, __LINE__);

    switch (size)
    {
        case sizeof(unsigned char):
            unsigned char ucRadioIndex;
            theAttributes.getValue(index, (char*) &ucRadioIndex, size);
            ro.radioIndex = (unsigned short) ucRadioIndex;
            break;
        case sizeof(ro.radioIndex):
            theAttributes.getValue(index, (char*) &ro.radioIndex, size);
            EXTERNAL_ntoh(&ro.radioIndex, sizeof(ro.radioIndex));
            break;
        case sizeof(unsigned):
            unsigned uRadioIndex;
            theAttributes.getValue(index, (char*) &uRadioIndex, size);
            EXTERNAL_ntoh(&uRadioIndex, sizeof(uRadioIndex));
            ro.radioIndex = (unsigned short) uRadioIndex;
            break;
        default:
            assert(0);
    }

    ro.radioIndexExists = true;
}

void
HlaFedAmb::ReflectRadioTransmitterOperationalStatus(
          HlaRadio&                         radio,
          RTI::ObjectHandle                 theObject,
    const RTI::AttributeHandleValuePairSet& theAttributes,
          unsigned                          index)
{
    RTI::ULong size = theAttributes.getValueLength(index);

    unsigned char txOperationalStatus;

    HlaVerify(size == sizeof(txOperationalStatus),
              "Unexpected attribute size",
              __FILE__, __LINE__);

    theAttributes.getValue(index, (char*) &txOperationalStatus, size);

    // (Prevent warning with gcc 3.0.4.)
    // Normally, we would assert
    // txOperationalStatus >= HLA_TX_OPERATIONAL_STATUS_MIN, but
    // HLA_TX_OPERATIONAL_STATUS_MIN should be 0, and
    // txOperationalStatus is unsigned already.

    assert(HLA_TX_OPERATIONAL_STATUS_MIN == 0);

    HlaVerify(txOperationalStatus <= HLA_TX_OPERATIONAL_STATUS_MAX,
              "Unexpected attribute value",
              __FILE__, __LINE__);

    assert(radio.txOperationalStatus <= HLA_TX_OPERATIONAL_STATUS_MAX);

    if (txOperationalStatus == radio.txOperationalStatus) { return; }

    assert(radio.entityPtr != NULL);
    const HlaEntity& entity = *radio.entityPtr;

    // Display full information if debugging is on; otherwise, just display
    // on/off changes to TransmitterOperationalStatus.

    if (m_ifaceData->m_hla->debug)
    {
        cout << "FED: (" << entity.entityIdString
                << ", " << radio.radioIndex << ") " << entity.markingData
                << " TransmitterOperationalStatus = ";

        switch (txOperationalStatus)
        {
            case HLA_TX_OPERATIONAL_STATUS_OFF:
                cout << "Off";
                break;
            case HLA_TX_OPERATIONAL_STATUS_ON_BUT_NOT_TRANSMITTING:
                cout << "OnButNotTransmitting";
                break;
            case HLA_TX_OPERATIONAL_STATUS_ON_AND_TRANSMITTING:
                cout << "OnAndTransmitting";
                break;
            default:
                assert(0);
        }

        cout << endl;
    }
    else
    if ((radio.txOperationalStatus == HLA_TX_OPERATIONAL_STATUS_OFF
         && txOperationalStatus != HLA_TX_OPERATIONAL_STATUS_OFF)
        ||
        (radio.txOperationalStatus != HLA_TX_OPERATIONAL_STATUS_OFF
         && txOperationalStatus == HLA_TX_OPERATIONAL_STATUS_OFF))
    {
        cout << "FED: (" << entity.entityIdString
                << ", " << radio.radioIndex << ") " << entity.markingData
                << " TransmitterOperationalStatus = ";

        switch (txOperationalStatus)
        {
            case HLA_TX_OPERATIONAL_STATUS_OFF:
                cout << "Off";
                break;
            case HLA_TX_OPERATIONAL_STATUS_ON_BUT_NOT_TRANSMITTING:
            case HLA_TX_OPERATIONAL_STATUS_ON_AND_TRANSMITTING:
                cout << "(On)";
                break;
            default:
                assert(0);
        }

        cout << endl;
    }

    if (txOperationalStatus != HLA_TX_OPERATIONAL_STATUS_OFF
        && radio.txOperationalStatus != HLA_TX_OPERATIONAL_STATUS_OFF)
    {
        // Radio was on, and is still on, but just a different mode.
    }
    else
    if (txOperationalStatus == HLA_TX_OPERATIONAL_STATUS_OFF)
    {
        // Radio has been turned off.

        if (entity.damageState != HLA_DAMAGE_STATE_DESTROYED)
        {
            ScheduleChangeInterfaceState(radio, false);
        }
    }
    else
    {
        // Radio has been turned on.

        if (entity.damageState != HLA_DAMAGE_STATE_DESTROYED)
        {
            ScheduleChangeInterfaceState(radio, true);
        }
    }

    radio.txOperationalStatus = txOperationalStatus;
}

void
HlaFedAmb::ReceiveApplicationSpecificRadioSignalIxn(
    const RTI::ParameterHandleValuePairSet& theParameters)
{
    if (m_ifaceData->m_hla->debug)
    {
        cout << "FED: Received ApplicationSpecificRadioSignal interaction"
                << endl;
    }

    HlaApplicationSpecificRadioSignalIxnInfo asrsIxnInfo;
    memset(&asrsIxnInfo, 0, sizeof(asrsIxnInfo));

    if (!ReadApplicationSpecificRadioSignalIxnUserProtocolIdParameter(
             theParameters, asrsIxnInfo))
    { return; }

    const unsigned cerUserProtocolId = 10000;

    if (asrsIxnInfo.userProtocolId == cerUserProtocolId)
    {
        if (!ReadApplicationSpecificRadioSignalIxnOtherParameters(
                 theParameters, asrsIxnInfo))
        { return; }

        ProcessCommEffectsRequest(asrsIxnInfo);
    }
    else
    {
        if (m_ifaceData->m_hla->debug2)
        {
            cout << "FED: Ignoring ApplicationSpecificRadioSignal interaction"
                    " with UserProtocolID " << asrsIxnInfo.userProtocolId
                    << endl;
        }
    }
}

bool
HlaFedAmb::ReadApplicationSpecificRadioSignalIxnUserProtocolIdParameter(
    const RTI::ParameterHandleValuePairSet& theParameters,
    HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo)
{
    unsigned i;
    for (i = 0; i < theParameters.size(); i++)
    {
        RTI::ParameterHandle ph = theParameters.getHandle(i);

        if (ph == m_asrsUserProtocolIdHandle)
        {
            RTI::ULong size = theParameters.getValueLength(i);

            if (size != sizeof(asrsIxnInfo.userProtocolId))
            {
                HlaReportError(
                    "Unexpected parameter size", __FILE__, __LINE__);
            }

            theParameters.getValue(
                i, (char*) &asrsIxnInfo.userProtocolId, size);

            EXTERNAL_ntoh(&asrsIxnInfo.userProtocolId,
                    sizeof(asrsIxnInfo.userProtocolId));

            return true;
        }
    }//for//

    return false;
}

bool
HlaFedAmb::ReadApplicationSpecificRadioSignalIxnOtherParameters(
    const RTI::ParameterHandleValuePairSet& theParameters,
    HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo)
{
    unsigned i;
    for (i = 0; i < theParameters.size(); i++)
    {
        RTI::ParameterHandle ph = theParameters.getHandle(i);
        RTI::ULong size = theParameters.getValueLength(i);

        if (ph == m_asrsHostRadioIndexHandle)
        {
            HlaVerify(size <= sizeof(asrsIxnInfo.hostRadioIndex),
                      "Unexpected parameter size",
                      __FILE__, __LINE__);

            theParameters.getValue(i, asrsIxnInfo.hostRadioIndex, size);

            // Convert last byte of buffer into null terminator, just in case.

            asrsIxnInfo.hostRadioIndex[sizeof(asrsIxnInfo.hostRadioIndex) - 1]
                = 0;
        }
        else
        if (ph == m_asrsDataRateHandle)
        {
            HlaVerify(size == sizeof(asrsIxnInfo.dataRate),
                      "Unexpected parameter size",
                      __FILE__, __LINE__);

            theParameters.getValue(i, (char*) &asrsIxnInfo.dataRate, size);

            EXTERNAL_ntoh(&asrsIxnInfo.dataRate, sizeof(asrsIxnInfo.dataRate));
        }
        else
        if (ph == m_asrsSignalDataLengthHandle)
        {
            HlaVerify(size == sizeof(asrsIxnInfo.signalDataLength),
                     "Unexpected parameter size",
                     __FILE__, __LINE__);

            theParameters.getValue(
                i, (char*) &asrsIxnInfo.signalDataLength, size);

            EXTERNAL_ntoh(&asrsIxnInfo.signalDataLength,
                    sizeof(asrsIxnInfo.signalDataLength));
        }
        else
        if (ph == m_asrsSignalDataHandle)
        {
            HlaVerify(size <= sizeof(asrsIxnInfo.signalData),
                     "Unexpected parameter size",
                     __FILE__, __LINE__);

            theParameters.getValue(i, asrsIxnInfo.signalData, size);

            // Convert last byte of buffer into null terminator, just in case.

            asrsIxnInfo.signalData[sizeof(asrsIxnInfo.signalData) - 1]
                = 0;
        }
        else
        if (ph == m_asrsTacticalDataLinkTypeHandle)
        {
            // Not used.
        }
        else
        if (ph == m_asrsTdlMessageCountHandle)
        {
            // Not used.
        }
        else
        if (ph == m_asrsUserProtocolIdHandle)
        {
            // Already read.
        }
        else
        {
            // Ignore unknown parameter handle.
        }
    }//for//

    return true;
}

#ifdef MILITARY_RADIOS_LIB
void
HlaFedAmb::ScheduleRtssNotification(
    const HlaRadio& radio,
    clocktype delay)
{
    Node* node = radio.node;

    Message* msg = MESSAGE_Alloc(node,
                                 EXTERNAL_LAYER,
                                 EXTERNAL_HLA,
                                 MSG_EXTERNAL_HLA_SendRtss);

    assert(delay >= 0);

    MESSAGE_Send(node, msg, delay);
}
#endif /* MILITARY_RADIOS_LIB */

void
HlaFedAmb::ScheduleMobilityEventIfNecessary(HlaEntity& entity)
{
    assert(m_ifaceData->m_hla->partitionData != NULL);
    clocktype simTime = m_ifaceData->m_hla->partitionData->theCurrentTime;

    clocktype mobilityEventTime
        = MAX(EXTERNAL_QueryExternalTime(m_ifaceData->m_hla->iface),
              simTime + 1);

    if (mobilityEventTime
        < entity.lastScheduledMobilityEventTime + m_ifaceData->m_hla->mobilityInterval)
    { return; }

    if (entity.lat < m_ifaceData->m_hla->minLat || entity.lat > m_ifaceData->m_hla->maxLat
        || entity.lon < m_ifaceData->m_hla->minLon || entity.lon > m_ifaceData->m_hla->maxLon)
    { return; }

    bool scheduleWorldLocation = false;
    bool scheduleOrientation   = false;
    bool scheduleSpeed         = false;

    if (fabs(entity.x - entity.xScheduled) >= m_ifaceData->m_hla->xyzEpsilon
        || fabs(entity.y - entity.yScheduled) >= m_ifaceData->m_hla->xyzEpsilon
        || fabs(entity.z - entity.zScheduled) >= m_ifaceData->m_hla->xyzEpsilon)
    { scheduleWorldLocation = true; }

    if (entity.azimuth != entity.azimuthScheduled
        || entity.elevation != entity.elevationScheduled)
    {
        scheduleOrientation = true;
    }

    if (entity.speed != entity.speedScheduled)
    { scheduleSpeed = true; }

    if (!scheduleWorldLocation && !scheduleOrientation && !scheduleSpeed)
    { return; }

    double entityX;
    double entityY;
    double entityZ;

    double entityLat;
    double entityLon;
    double entityAlt;

    if (scheduleWorldLocation)
    {
        entityX = entity.x;
        entityY = entity.y;
        entityZ = entity.z;

        entityLat = entity.lat;
        entityLon = entity.lon;
        entityAlt = entity.alt;
    }
    else
    {
        entityX = entity.xScheduled;
        entityY = entity.yScheduled;
        entityZ = entity.zScheduled;

        entityLat = entity.latScheduled;
        entityLon = entity.lonScheduled;
        entityAlt = entity.altScheduled;
    }

    Orientation orientation;

    if (scheduleOrientation)
    {
        orientation.azimuth   = entity.azimuth;
        orientation.elevation = entity.elevation;
    }
    else
    {
        orientation.azimuth   = entity.azimuthScheduled;
        orientation.elevation = entity.elevationScheduled;
    }

    double speed;

    if (scheduleSpeed)
    {
        speed = entity.speed;
    }
    else
    {
        speed = entity.speedScheduled;
    }

    // Move entity (hierarchy).

    if (m_ifaceData->m_hla->partitionData->guiOption
        && entity.hierarchyIdExists
        && (scheduleOrientation || scheduleWorldLocation))
    {
        Node* node;

        if (entity.numRadioPtrs > 0)
        {
            HlaRadio& radio = *entity.radioPtrs[0];
            node = radio.node;
        }
        else
        {
            node = m_ifaceData->m_hla->partitionData->firstNode;
        }

        Coordinates coordinates;
        coordinates.latlonalt.latitude  = entityLat;
        coordinates.latlonalt.longitude = entityLon;
        coordinates.latlonalt.altitude  = entityAlt;

        Message* msg = MESSAGE_Alloc(node,
                                     EXTERNAL_LAYER,
                                     EXTERNAL_HLA,
                                     MSG_EXTERNAL_HLA_HierarchyMobility);
        MESSAGE_InfoAlloc(node, msg, sizeof(HlaHierarchyMobilityInfo));
        HlaHierarchyMobilityInfo& info
            = *((HlaHierarchyMobilityInfo*) MESSAGE_ReturnInfo(msg));
        info.hierarchyId = entity.hierarchyId;
        info.coordinates = coordinates;
        info.orientation = orientation;

        // Execute the hierarchy move one nanosecond before the node move
        // time.  This is a workaround for a GUI issue where the screen will
        // get all messed up if the GUI command to move the node is sent
        // before the GUI command to move the hierarchy -- even if both GUI
        // commands are sent with the same timestamp.

        clocktype delay = (mobilityEventTime - 1) - simTime ;
        assert(delay >= 0);

        MESSAGE_Send(node, msg, delay);
    }//if//

    // Move radios (nodes).

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        HlaRadio& radio = *entity.radioPtrs[i];

        // Schedule QualNet node movement.

        Coordinates coordinates;

        double radioLat;
        double radioLon;
        double radioAlt;

        if (scheduleWorldLocation)
        {
            // When calculating positions for hosted radios, QualNet assumes
            // the host entity is pointed in the same direction as the GCC
            // x-axis, with no roll with respect to the earth's equatorial
            // plane, and with the top of the entity facing the same direction
            // as the north pole.
            //
            // Relative positions for the entity y- and z-axes are flipped with
            // respect to the GCC y- and z-axes.

            double radioX = entityX + radio.relativePositionX;
            double radioY = entityY - radio.relativePositionY;
            double radioZ = entityZ - radio.relativePositionZ;

            HlaConvertGccToLatLonAlt(
                radioX, radioY, radioZ, radioLat, radioLon, radioAlt);

            assert(radioLat >= -90.0 && radioLat <= 90.0
                   && radioLon >= -180.0 && radioLon <= 180.0);

            coordinates.latlonalt.latitude  = radioLat;
            coordinates.latlonalt.longitude = radioLon;
            coordinates.latlonalt.altitude  = radioAlt;
        }
        else
        {
            coordinates.latlonalt.latitude  = radio.latScheduled;
            coordinates.latlonalt.longitude = radio.lonScheduled;
            coordinates.latlonalt.altitude  = radio.altScheduled;
        }
        // This ia parallel safe (though timing of position
        // update for nodes of remote partitions is best-effort)
        EXTERNAL_ChangeNodePositionOrientationAndSpeedAtTime(
            m_ifaceData->m_hla->iface,
            radio.node,
            mobilityEventTime,
            coordinates.latlonalt.latitude,
            coordinates.latlonalt.longitude,
            coordinates.latlonalt.altitude,
            orientation.azimuth,
            orientation.elevation,
            speed);

        if (scheduleWorldLocation)
        {
            radio.latScheduled = radioLat;
            radio.lonScheduled = radioLon;
            radio.altScheduled = radioAlt;
        }
    }//for//

    if (scheduleWorldLocation)
    {
        entity.xScheduled = entity.x;
        entity.yScheduled = entity.y;
        entity.zScheduled = entity.z;

        entity.latScheduled = entity.lat;
        entity.lonScheduled = entity.lon;
        entity.altScheduled = entity.alt;
    }

    if (scheduleOrientation)
    {
        entity.azimuthScheduled   = entity.azimuth;
        entity.elevationScheduled = entity.elevation;
    }

    if (scheduleSpeed)
    {
        entity.speedScheduled = entity.speed;
    }

    if (!m_ifaceData->m_hla->newEventScheduled) { m_ifaceData->m_hla->newEventScheduled = true; }

    entity.lastScheduledMobilityEventTime = mobilityEventTime;

    if (m_ifaceData->m_hla->debug2)
    {
        char clocktypeString[g_hlaClocktypeStringBufSize];
        TIME_PrintClockInSecond(mobilityEventTime, clocktypeString);

        cout << "FED: Moving (" << entity.entityIdString << ")"
                << " to (" << entityLat << "," << entityLon << "," << entityAlt
                << ") (" << orientation.azimuth << "," << orientation.elevation
                << ") (" << (unsigned) speed << "), time " << clocktypeString
                << endl;
    }
}

void
HlaFedAmb::ProcessCommEffectsRequest(
    HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo)
{
    HlaSimulatedMsgInfo smInfo;

    double              requestedDataRate;
    const HlaEntity*    dstEntityPtr = NULL;
    unsigned            dataMsgSize;
    clocktype           voiceMsgDuration;
    bool                isVoice;
    clocktype           timeoutDelay;
    unsigned            timestamp;

    bool                unicast;
    const HlaRadio*     dstRadioPtr = NULL;

    HlaObjectNameToHandleMap::iterator handleIt =
        m_ifaceData->m_hla->objectNameToHandle.find(asrsIxnInfo.hostRadioIndex);

    if (handleIt == m_ifaceData->m_hla->objectNameToHandle.end())
    {
        char warningString[MAX_STRING_LENGTH];
        sprintf(
            warningString,
            "Can't map to RadioTransmitter object using HostRadioIndex = %s",
            asrsIxnInfo.hostRadioIndex);
        HlaReportWarning(warningString);

        return;
    }

    unsigned handle = handleIt->second;

    HlaHandleToRadioObjectMap::iterator radioIt =
        m_ifaceData->m_hla->handleToRadioObject.find(handle);

    const HlaRadioObject& srcRadioObject = radioIt->second;

    if (srcRadioObject.radioPtr == NULL)
    {
        if (m_ifaceData->m_hla->debug)
        {
            char warningString[MAX_STRING_LENGTH];

            strcpy(warningString,
                   "RadioTransmitter object not mapped to a Qualnet node");

            if (srcRadioObject.entityIdExists)
            {
                char entityIdString[g_hlaEntityIdStringBufSize];
                HlaMakeEntityIdString(srcRadioObject.entityId, entityIdString);

                char newString[MAX_STRING_LENGTH];
                sprintf(newString, ", EntityID = %s", entityIdString);
                strcat(warningString, newString);
            }
            if (srcRadioObject.radioIndexExists)
            {
                char newString[MAX_STRING_LENGTH];
                sprintf(newString, ", RadioIndex = %u",
                        srcRadioObject.radioIndex);
                strcat(warningString, newString);
            }

            HlaReportWarning(warningString);
        }

        return;
    }

    const HlaRadio& srcRadio = *srcRadioObject.radioPtr;

    smInfo.srcRadioPtr = &srcRadio;
    //smInfo.srcNodeId = srcRadio.node->nodeId;

    requestedDataRate = (double) asrsIxnInfo.dataRate;

    if (!ParseMsgString(
             asrsIxnInfo.signalData,
             dstEntityPtr,
             dataMsgSize,
             voiceMsgDuration,
             isVoice,
             timeoutDelay,
             timestamp))
    { return; }

    if (dstEntityPtr == srcRadio.entityPtr)
    {
        HlaReportWarning("Can't send message to self");
        return;
    }

    if (srcRadio.networkPtr == NULL)
    {
        HlaReportWarning("Can't find associated network");
        return;
    }

    if (srcRadio.networkPtr->unicast)
    {
        unicast = true;
        if (dstEntityPtr == NULL)
        {
            dstRadioPtr = srcRadio.defaultDstRadioPtr;
            dstEntityPtr = dstRadioPtr->entityPtr;
        }
        else
        {
            // Find dst radio in same network as src radio.

            const HlaNetwork& network = *srcRadio.networkPtr;

            unsigned i;
            for (i = 0; i < network.radios.size(); i++)
            {
                const HlaRadio& dstRadio = *network.radios[i];

                if (dstRadio.node == srcRadio.node) { continue; }

                if (dstRadio.entityPtr->entityId == dstEntityPtr->entityId)
                { break; }
            }

            if (i == network.numRadioPtrs)
            {
                HlaReportWarning("Dst EntityID is not in same network");
                return;
            }

            dstRadioPtr = network.radios[i];
        }
    }
    else
    {
        unicast = false;
    }

    if (!unicast)
    {
        // This is a broadcast, so check that there is at least one receiver.

        const HlaNetwork& network = *srcRadio.networkPtr;

        unsigned i;
        for (i = 0; i < network.radios.size(); i++)
        {
            const HlaRadio& dstRadio = *network.radios[i];

            if (dstRadio.node->nodeId == srcRadio.node->nodeId) { continue; }

            break;
        }

        if (i == network.numRadioPtrs)
        {
            if (m_ifaceData->m_hla->debug)
            {
                HlaReportWarning(
                    "Ignoring broadcast to network with no potential"
                    " receivers");
            }

            return;
        }//if//
    }//if//

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        m_ifaceData->m_hla->nodeIdToPerNodeData.find(srcRadio.node->nodeId);
    HlaData& srcHlaData = nodeIt->second;

    smInfo.msgId = srcHlaData.nextMsgId;
    srcHlaData.nextMsgId++;

    if (srcHlaData.nextMsgId > m_ifaceData->m_hla->maxMsgId)
    {
        HlaReportError("Maximum msgId reached", __FILE__, __LINE__);
    }

    Node* srcNode = srcRadio.node;

    clocktype simTime = TIME_getSimTime(srcNode);
    clocktype sendTime
        = MAX(EXTERNAL_QueryExternalTime(m_ifaceData->m_hla->iface), simTime);

    const clocktype sendDelay = sendTime - TIME_getSimTime(srcNode);

    SendSimulatedMsgUsingMessenger(
        smInfo,
        dstEntityPtr,
        requestedDataRate,
        dataMsgSize,
        voiceMsgDuration,
        isVoice,
        timeoutDelay,
        unicast,
        dstRadioPtr,
        sendDelay);

    ScheduleTimeout(smInfo, timeoutDelay, sendDelay);

    StoreOutstandingSimulatedMsgInfo(
        smInfo,
        dstEntityPtr,
        timestamp,
        sendTime);

    if (m_ifaceData->m_hla->debug)
    {
        PrintCommEffectsRequestProcessed(
            srcRadio, dstEntityPtr, unicast, dstRadioPtr, sendTime);
    }
}

bool
HlaFedAmb::ParseMsgString(
    char* msgString,
    const HlaEntity*& dstEntityPtr,
    unsigned& dataMsgSize,
    clocktype& voiceMsgDuration,
    bool& isVoice,
    clocktype& timeoutDelay,
    unsigned& timestamp)
{
    char token[g_hlaSignalDataBufSize];
    char* next = msgString;

    IO_GetDelimitedToken(token, next, "\n", &next);
    if (strcmp(token, "HEADER") != 0)
    {
        HlaReportWarning("Message string did not start with HEADER");
        return false;
    }

    char name[g_hlaSignalDataBufSize];
    char value[g_hlaSignalDataBufSize];
    char* nextNameValue;

    // Required fields.

    bool foundSizeField      = false;
    bool foundTimeoutField   = false;
    bool foundTimestampField = false;

    if (m_ifaceData->m_hla->debug) { cout << "FED: "; }

    while (IO_GetDelimitedToken(token, next, "\n", &next))
    {
        if (strcmp(token, "EOH") == 0)
        {
            if (m_ifaceData->m_hla->debug) { cout << endl; }
            break;
        }

        if (m_ifaceData->m_hla->debug) { cout << token << " "; }

        if (!IO_GetDelimitedToken(name, token, "=", &nextNameValue))
        {
            HlaReportWarning("Can't find attribute=value in message string");
            return false;
        }

        if (!IO_GetDelimitedToken(value, nextNameValue, "=", &nextNameValue))
        {
            HlaReportWarning("Can't find attribute=value in message string");
            return false;
        }

        if (strcmp(name, "receiver") == 0)
        {
            HlaEntityId dstEntityId;

            if (sscanf(value, "%hu.%hu.%hu",
                       &dstEntityId.siteId,
                       &dstEntityId.applicationId,
                       &dstEntityId.entityNumber) != 3)
            {
                HlaReportWarning("Can't read receiver field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            HlaEntityIdToEntityMap::iterator entityIt =
                m_ifaceData->m_hla->entityIdToEntity.find(dstEntityId);

            if (entityIt == m_ifaceData->m_hla->entityIdToEntity.end())
            {
                HlaReportWarning(
                    "Can't map receiver EntityID in message string to entity",
                    __FILE__, __LINE__);
                return false;
            }

            dstEntityPtr = entityIt->second;
        }
        else
        if (strcmp(name, "size") == 0)
        {
            double sizeValue;
            char unitString[g_hlaSignalDataBufSize];

            if (sscanf(value, "%lf %s", &sizeValue, unitString) != 2
                || sizeValue < 0.0
                || sizeValue > (double) UINT_MAX)
            {
                HlaReportWarning("Bad size field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            // TODO:  Fractional sizes (either bytes or seconds) are rounded
            // down here.  Perhaps fractional values should be retained,
            // especially for seconds.

            if (strcmp(unitString, "bytes") == 0)
            {
                dataMsgSize = (unsigned) sizeValue;

                isVoice = false;
            }
            else
            if (strcmp(unitString, "seconds") == 0)
            {
                voiceMsgDuration = HlaConvertDoubleToClocktype(sizeValue);

                isVoice = true;
            }
            else
            {
                HlaReportWarning("Unrecognized units in size field",
                                 __FILE__, __LINE__);
                return false;
            }

            foundSizeField = true;
        }
        else
        if (!strcmp(name, "timeout"))
        {
            char* endPtr = NULL;
            errno = 0;
            double double_timeoutDelay = strtod(value, &endPtr);
            if (endPtr == value || errno != 0)
            {
                HlaReportWarning("Bad timeout field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            timeoutDelay = HlaConvertDoubleToClocktype(double_timeoutDelay);
            foundTimeoutField = true;
        }
        else
        if (!strcmp(name, "timestamp"))
        {
            // 0x34567890, 10 bytes.

            const unsigned timestampStringLength = 10;
            if (strlen(value) != timestampStringLength
                || !(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                || sscanf(&value[2], "%x", &timestamp) != 1)
            {
                HlaReportWarning("Bad timestamp field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            foundTimestampField = true;
        }
    }//while//

    if (!(foundSizeField && foundTimeoutField && foundTimestampField))
    {
        HlaReportWarning("All essential fields not present in message string",
                         __FILE__, __LINE__);
        return false;
    }

    return true;
}

void
StartSendSimulatedMsgUsingMessenger(
    Node* srcNode,
    NodeAddress srcNetworkAddress,
    NodeAddress destNodeId,
    const HlaSimulatedMsgInfo& smInfo,
    double requestedDataRate,
    unsigned dataMsgSize,
    clocktype voiceMsgDuration,
    bool isVoice,
    clocktype timeoutDelay,
    bool unicast,
    clocktype sendDelay)
{
    NodeAddress srcNodeId = srcNode->nodeId;

    // The variable below will be retrieved as an info field in the Messenger
    // result function.

    MessengerPktHeader info;

    info.pktNum = 0;
    info.initialPrDelay = sendDelay;
    assert(info.initialPrDelay >= 0);

    info.srcAddr
       = MAPPING_GetDefaultInterfaceAddressFromNodeId(srcNode,
                                                      srcNode->nodeId);

    if (unicast)
    {
        info.destAddr
           = MAPPING_GetDefaultInterfaceAddressFromNodeId(
               srcNode, destNodeId);
    }
    else
    {
        assert(srcNetworkAddress != 0);
        info.destAddr = srcNetworkAddress;
    }

    info.msgId = (int) smInfo.msgId;
    info.lifetime = timeoutDelay;

    // Add the size of one double variable in case the compiler adds padding
    // between the Messenger data and the custom data.

    unsigned minBytes
        = sizeof(MessengerPktHeader) + sizeof(smInfo) + sizeof(double);

    // This has to be done at real source (messenger use only)
#ifdef MILITARY_RADIOS_LIB
    if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK11
        || srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK16)
    {
        info.transportType = TRANSPORT_TYPE_MAC;
    }
    else
    {
#endif /* MILITARY_RADIOS_LIB */
        info.transportType = TRANSPORT_TYPE_UNRELIABLE;
#ifdef MILITARY_RADIOS_LIB
    }
#endif /* MILITARY_RADIOS_LIB */

    // Perform different modeling depending on if the message is a data
    // message or a voice message.
    if (!isVoice)
    {
        // Data message.

        info.appType = GENERAL;

#ifdef MILITARY_RADIOS_LIB
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK11
            || srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK16)
        {
            // For Link-11 or Link-16, just send one huge packet for this
            // data message.
            // The Link-11 model won't fragment the packet, but send the
            // whole thing in one shot, following real-world operation.
            // The Link-16 model will send the packet using a TDMA approach.

            info.freq = 0 * SECOND;

            if (dataMsgSize >= minBytes) { info.fragSize = dataMsgSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        {
#endif /* MILITARY_RADIOS_LIB */
            // For non-Link-11 network devices, chunk the data message into
            // packets, each packet containing 128-bytes payload.  This
            // size was arbitrarily chosen.  Since the data message is sent
            // via UDP, the payload size per packet is limited by QualNet's
            // UDP model.
            //
            // Dividing the payload-size of each packet by the ASRS
            // interaction's DataRate parameter, one obtains the packet
            // interval.
            //
            // Dividing dataMsgSize by the payload-size of each packet,
            // one obtains the number of packets.

            unsigned payloadSize = 128;
            double packetInterval = (double) payloadSize * 8.0
                                    / requestedDataRate;

            info.freq     = HlaConvertDoubleToClocktype(packetInterval);
            info.fragSize = payloadSize;

            info.numFrags = (unsigned short)
                            ceil(((double) dataMsgSize)
                                  / (double) payloadSize);

            unsigned minFragments
                = (unsigned) ceil((double) minBytes
                                  / (double) info.fragSize);
            if (info.numFrags < minFragments)
            {
                info.numFrags = minFragments;
            }
#ifdef MILITARY_RADIOS_LIB
        }
#endif /* MILITARY_RADIOS_LIB */
    }
    else
    {
        // Voice message.

        info.appType = VOICE_PACKET;

#ifdef MILITARY_RADIOS_LIB
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK11)
        {
            // For Link-11, the voice message is comprised of a single large
            // packet, just as with Link-11 data messages.
            //
            // Multiplying voiceMsgDuration with the PHY data rate, one
            // obtains the size of the message in bytes such that it should
            // take approximately the correct amount of time.
            //
            // Note:  The actual number of bytes should be less than the
            // value calculated as described, since there are probably
            // overhead bytes added by the Link-11 model.  This is assumed
            // to be negligible though.

            info.freq = 0 * SECOND;

            PhyDataAbstract* pda
                = (PhyDataAbstract*)
                  srcNode->phyData[MAC_DEFAULT_INTERFACE]->phyVar;

            unsigned packetSize
                = (unsigned)
                  (HlaConvertClocktypeToDouble(voiceMsgDuration)
                   * ((double) pda->dataRate / 8.0));

            if (packetSize >= minBytes) { info.fragSize = packetSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK16)
        {
            // For Link-16, the voice message is comprised of a single large
            // packet, just as with Link-16 data messages.
            //
            // An 8-kbps data rate for the voice message is assumed.
            // Multiplying the data rate by voiceMsgDuration, one obtains the
            // size of the voice message in bytes.
            //
            // Note:  This computation implies that the entire voice message
            // is available to be transmitted on the Link-16 network at the
            // time the ASRS interaction was received.  This means that the
            // entire voice message can be transmitted in a time much less
            // than the duration of the message, given a high Link-16 data
            // rate and a large proportion of slots assigned to the source
            // node.
            //
            // This is in contrast to the Link-11 and default approach for
            // voice messages, which assumes that the voice message is being
            // read off starting at the time the ASRS was received (in the
            // Link-11 and default approach for voice, if voiceMsgDuration is
            // 10 seconds, it should take ~ 10 seconds of simulation time
            // before QualNet sends a Process Message interaction).
            //
            // The differing approach for Link-16 is assumed to be okay
            // because Link-16 operates in a TDMA fashion anyway.

            info.freq = 0 * SECOND;

            PhyDataAbstract* pda
                = (PhyDataAbstract*)
                  srcNode->phyData[MAC_DEFAULT_INTERFACE]->phyVar;

            unsigned packetSize
                = (unsigned)
                  (HlaConvertClocktypeToDouble(voiceMsgDuration)
                   * (8000.0 / 8.0));

            if (packetSize >= minBytes) { info.fragSize = packetSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        {
#endif /* MILITARY_RADIOS_LIB */
            // For non-Link-11 network devices, an 8-kbps data rate for the
            // voice message is assumed, with a 250-byte payload per packet
            // at 4 packets per second.  (Real VoIP applications would send
            // smaller payloads at a more frequent rate to reach 8-kbps.
            // The values uses in the model are chosen to increase modeling
            // speed.)
            //
            // Dividing voiceMsgDuration by the packet interval, one obtains
            // the number of packets for the voice message.

            const double   packetInterval = 0.25;
            const unsigned payloadSize    = 250;

            info.freq     = HlaConvertDoubleToClocktype(packetInterval);
            info.fragSize = payloadSize;

            info.numFrags
                = (unsigned short)
                  ceil(HlaConvertClocktypeToDouble(voiceMsgDuration)
                       / packetInterval);

            unsigned minFragments
                = (unsigned) ceil((double) minBytes
                                  / (double) info.fragSize);
            if (info.numFrags < minFragments)
            {
                info.numFrags = minFragments;
            }
#ifdef MILITARY_RADIOS_LIB
        }
#endif /* MILITARY_RADIOS_LIB */
    }//if//


#ifdef MILITARY_RADIOS_LIB
    if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK16)
    {
        MacDataLink16* link16 =
            (MacDataLink16*) srcNode->macData[MAC_DEFAULT_INTERFACE]->macVar;

        assert(link16 != NULL);
        assert(link16->numNPG > 0);
        assert(link16->npgInfo != NULL);

        info.destNPGId = link16->npgInfo[0].npgAddress;
    }
#endif /* MILITARY_RADIOS_LIB */
    MessengerSendMessage(
        srcNode,
        info,
        (char*) &smInfo,
        sizeof(smInfo),
        &HlaAppMessengerResultFcn);
}
void
HlaFedAmb::SendSimulatedMsgUsingMessenger(
    const HlaSimulatedMsgInfo& smInfo,
    const HlaEntity* dstEntityPtr,
    double requestedDataRate,
    unsigned dataMsgSize,
    clocktype voiceMsgDuration,
    bool isVoice,
    clocktype timeoutDelay,
    bool unicast,
    const HlaRadio* dstRadioPtr,
    clocktype sendDelay)
{
    // TODO: Check what destNodeId of -1 means.

    NodeAddress destNodeId = (NodeAddress) -1;

    if (unicast)
    {
        assert(dstRadioPtr != NULL);
        destNodeId = dstRadioPtr->node->nodeId;
    }
    else
    {
        assert(smInfo.srcRadioPtr->networkPtr->ipAddress != 0);
        destNodeId = (NodeAddress) -1;
    }
    if (smInfo.srcRadioPtr->node->partitionId == 0)
    {
        // Call the core function
        StartSendSimulatedMsgUsingMessenger(
            smInfo.srcRadioPtr->node,
            smInfo.srcRadioPtr->networkPtr->ipAddress,
            destNodeId,
            smInfo,
            requestedDataRate,
            dataMsgSize,
            voiceMsgDuration,
            isVoice,
            timeoutDelay,
            unicast,
            sendDelay);
    }
    else
    {
        Node* srcNode = smInfo.srcRadioPtr->node;
        // Build an HLA-interface message so we can request the
        // correct partition start up a messenger app
        Message* startMessengerMsg = MESSAGE_Alloc(
            srcNode,
            EXTERNAL_LAYER,    // special layer
            EXTERNAL_HLA,         // EXTERNAL_HLA,           // protocol
            MSG_EXTERNAL_HLA_StartMessengerForwarded);
        EXTERNAL_HlaStartMessenegerForwardedInfo* startInfo =
            (EXTERNAL_HlaStartMessenegerForwardedInfo *)
            MESSAGE_InfoAlloc (srcNode, startMessengerMsg,
            sizeof (EXTERNAL_HlaStartMessenegerForwardedInfo));
        startInfo->isVoice = isVoice;
        startInfo->unicast = unicast;
        startInfo->destNodeId = destNodeId;
        startInfo->voiceMsgDuration = voiceMsgDuration;
        startInfo->timeoutDelay = timeoutDelay;
        startInfo->sendDelay = sendDelay;
        startInfo->srcNetworkAddress =
            smInfo.srcRadioPtr->networkPtr->ipAddress;
        startInfo->requestedDataRate = requestedDataRate;
        startInfo->srcNodeId = srcNode->nodeId;
        startInfo->dataMsgSize = dataMsgSize;
        memcpy (&(startInfo->smInfo), &smInfo, sizeof (HlaSimulatedMsgInfo));
        EXTERNAL_MESSAGE_RemoteSend
            (m_ifaceData->m_hla->iface, srcNode->partitionId,
            startMessengerMsg, 0, EXTERNAL_SCHEDULE_LOOSELY);

    }

    if (!m_ifaceData->m_hla->newEventScheduled)
    {
        m_ifaceData->m_hla->newEventScheduled = true;
    }
}

void
HlaFedAmb::ScheduleTimeout(
    const HlaSimulatedMsgInfo& smInfo,
    clocktype timeoutDelay,
    clocktype sendDelay)
{
    Node* srcNode = smInfo.srcRadioPtr->node;

    Message* timeoutMsg;
    timeoutMsg = MESSAGE_Alloc(srcNode,
                               EXTERNAL_LAYER,
                               EXTERNAL_HLA,
                               MSG_EXTERNAL_HLA_AckTimeout);

    MESSAGE_InfoAlloc(srcNode, timeoutMsg, sizeof(smInfo));
    HlaSimulatedMsgInfo& timeoutHeader
        = *((HlaSimulatedMsgInfo*) MESSAGE_ReturnInfo(timeoutMsg));

    timeoutHeader = smInfo;

    assert(timeoutDelay >= 0);
    assert(sendDelay >= 0);

    clocktype delay = sendDelay + timeoutDelay;

    MESSAGE_Send(srcNode, timeoutMsg, delay);

    if (!m_ifaceData->m_hla->newEventScheduled) { m_ifaceData->m_hla->newEventScheduled = true; }
}

void
HlaFedAmb::StoreOutstandingSimulatedMsgInfo(
    const HlaSimulatedMsgInfo& smInfo,
    const HlaEntity* dstEntityPtr,
    unsigned timestamp,
    clocktype sendTime)
{
    const HlaRadio& srcRadio = *smInfo.srcRadioPtr;
    Node* srcNode = srcRadio.node;

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        m_ifaceData->m_hla->nodeIdToPerNodeData.find(srcNode->nodeId);
    HlaData& hlaData = nodeIt->second;

    HlaOutstandingSimulatedMsgInfo& osmInfo
        = hlaData.outstandingSimulatedMsgInfo[smInfo.msgId ];

    osmInfo.timestamp = timestamp;
    osmInfo.sendTime = sendTime;

    osmInfo.numDstEntitiesProcessed = 0;
    osmInfo.smDstEntitiesInfos.clear();

    if (dstEntityPtr != NULL)
    {
        assert(srcRadio.entityPtr != dstEntityPtr);

        HlaSimulatedMsgDstEntityInfo smDstEntityInfo;

        smDstEntityInfo.dstEntityPtr = dstEntityPtr;
        smDstEntityInfo.processed    = false;
        smDstEntityInfo.success      = false;

        osmInfo.smDstEntitiesInfos.push_back(smDstEntityInfo);
    }
    else
    {
        const HlaNetwork& network = *srcRadio.networkPtr;

        unsigned i;
        for (i = 0; i < network.radios.size(); i++)
        {
            const HlaRadio& dstRadio = *network.radios[i];

            if (dstRadio.node->nodeId == srcRadio.node->nodeId) { continue; }

            HlaSimulatedMsgDstEntityInfo smDstEntityInfo;

            smDstEntityInfo.dstEntityPtr = dstRadio.entityPtr;
            smDstEntityInfo.processed    = false;
            smDstEntityInfo.success      = false;

            osmInfo.smDstEntitiesInfos.push_back(smDstEntityInfo);
        }

    }
}

void
HlaFedAmb::ReceiveDataIxn(
    const RTI::ParameterHandleValuePairSet& theParameters)
{
    if (m_ifaceData->m_hla->debug)
    {
        cout << "FED: Received Data interaction" << endl;
    }

    HlaDataIxnInfo dataIxnInfo;
    memset(&dataIxnInfo, 0, sizeof(dataIxnInfo));

    if (!ReadDataIxnParameters(theParameters, dataIxnInfo)) { return; }

    HlaVariableDatumInfo& vdInfo = dataIxnInfo.variableDatumSetInfo.variableDatumsInfo[0];

    const unsigned terminateCesNotificationDatumId = 66900;

    if (vdInfo.datumId == terminateCesNotificationDatumId)
    {
        ProcessTerminateCesRequest();
    }
    else
    {
        if (m_ifaceData->m_hla->debug2)
        {
            cout << "FED: Ignoring Data interaction with DatumID "
                    << vdInfo.datumId << endl;
        }
    }
}

bool
HlaFedAmb::ReadDataIxnParameters(
    const RTI::ParameterHandleValuePairSet& theParameters,
    HlaDataIxnInfo& dataIxnInfo)
{
    HlaVariableDatumSetInfo& vdsInfo = dataIxnInfo.variableDatumSetInfo;

    unsigned i;
    for (i = 0; i < theParameters.size(); i++)
    {
        RTI::ParameterHandle ph = theParameters.getHandle(i);
        RTI::ULong size = theParameters.getValueLength(i);

        if (ph == m_dataOriginatingEntityHandle)
        {
            // Not used.
        }
        else
        if (ph == m_dataReceivingEntityHandle)
        {
            // Not used.
        }
        else
        if (ph == m_dataRequestIdentifierHandle)
        {
            // Not used.
        }
        else
        if (ph == m_dataFixedDatumsHandle)
        {
            // Not used.
        }
        else
        if (ph == m_dataVariableDatumSetHandle)
        {
            if (size > g_hlaVariableDatumSetBufSize)
            {
                if (m_ifaceData->m_hla->debug)
                {
                    cout << "FED: Ignoring Data interaction containing"
                            " large VariableDatumSet parameter" << endl;
                }

                return false;
            }

            char variableDatumSet[g_hlaVariableDatumSetBufSize];
            theParameters.getValue(i, variableDatumSet, size);

            // NumberOfVariableDatums, DatumID, DatumLength.
            // Converted to host-byte-order.

            unsigned vdsOffset = 0;

            HlaCopyFromOffsetAndNtoh(
                &vdsInfo.numberOfVariableDatums,
                variableDatumSet,
                vdsOffset,
                sizeof(vdsInfo.numberOfVariableDatums));

            if (vdsInfo.numberOfVariableDatums != 1)
            {
                if (m_ifaceData->m_hla->debug)
                {
                    cout << "FED: Ignoring Data interaction where"
                            " NumberOfVariableDatums != 1" << endl;
                }

                return false;
            }

            HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

            HlaCopyFromOffsetAndNtoh(
                &vdInfo.datumId,
                variableDatumSet,
                vdsOffset,
                sizeof(vdInfo.datumId));

            HlaCopyFromOffsetAndNtoh(
                &vdInfo.datumLength,
                variableDatumSet,
                vdsOffset,
                sizeof(vdInfo.datumLength));

            if (vdInfo.datumLength % 8 != 0)
            {
                if (m_ifaceData->m_hla->debug)
                {
                    cout << "FED: Ignoring Data interaction where"
                            " DatumLength is not a multiple of 8" << endl;
                }

                return false;
            }

            unsigned datumLengthInBytes = vdInfo.datumLength / 8;

            if (datumLengthInBytes > g_hlaDatumValueBufSize)
            {
                if (m_ifaceData->m_hla->debug)
                {
                    cout << "FED: Ignoring Data interaction containing"
                            " large DatumValue field" << endl;
                }

                return false;
            }

            // DatumValue.
            // Left in network-byte-order.

            HlaCopyFromOffset(
                vdInfo.datumValue,
                variableDatumSet,
                vdsOffset,
                datumLengthInBytes);
        }
        else
        {
            // Ignore unknown parameter handle.
        }
    }//for//

    return true;
}

void
HlaFedAmb::PrintCommEffectsRequestProcessed(
    const HlaRadio& srcRadio,
    const HlaEntity* dstEntityPtr,
    bool unicast,
    const HlaRadio* dstRadioPtr,
    clocktype sendTime)
{
    cout << "FED: CREQ, "
            << srcRadio.node->nodeId << " ("
            << srcRadio.entityPtr->entityIdString << ", "
            << srcRadio.radioIndex << ") " << srcRadio.entityPtr->markingData;

    if (unicast)
    {
        assert(dstEntityPtr != NULL);
        assert(dstRadioPtr != NULL);

        cout << " unicast to " << dstRadioPtr->node->nodeId
                << " (" << dstEntityPtr->entityIdString << ") "
                << dstEntityPtr->markingData;
    }
    else
    {
        cout << " broadcast";

        if (dstEntityPtr != NULL)
        {
            cout << " to (" << dstEntityPtr->entityIdString << ") "
                    << dstEntityPtr->markingData;
        }
    }


    char sendTimeString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(sendTime, sendTimeString);

    cout << ", send time " << sendTimeString << endl;
}

void
HlaFedAmb::ProcessTerminateCesRequest()
{
    cout << endl
         << "FED: Received Terminate CES request." << endl;

    HlaEndSimulation(m_ifaceData);
}

void
HlaFedAmb::ScheduleChangeInterfaceState(
    const HlaEntity& entity,
    bool enableInterfaces,
    clocktype delay)
{
    int eventType;
    if (enableInterfaces) { eventType = MSG_MAC_EndFault; }
    else { eventType = MSG_MAC_StartFault; }

    assert(entity.numRadioPtrs > 0);

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        assert(entity.radioPtrs[i] != NULL);
        const HlaRadio& radio = *entity.radioPtrs[i];

        if (enableInterfaces
            && radio.txOperationalStatus == HLA_TX_OPERATIONAL_STATUS_OFF)
        {
            continue;
        }

        Node* node = radio.node;

        unsigned interfaceIndex;
        for (interfaceIndex = 0;
             interfaceIndex < (unsigned) node->numberInterfaces;
             interfaceIndex++)
        {
            Message* msg = MESSAGE_Alloc(node,
                                         MAC_LAYER,
                                         0,
                                         eventType);

            MESSAGE_SetInstanceId(msg, (short) interfaceIndex);

            MESSAGE_InfoAlloc(node, msg, sizeof(FaultType));
            FaultType& info = *((FaultType*) MESSAGE_ReturnInfo(msg));
            info = STATIC_FAULT;

            MESSAGE_Send(node, msg, delay);
        }//for//
    }//for//
}

void
HlaFedAmb::ScheduleChangeInterfaceState(
    const HlaRadio& radio, bool enableInterface)
{
    int eventType;
    if (enableInterface) { eventType = MSG_MAC_EndFault; }
    else { eventType = MSG_MAC_StartFault; }

    clocktype simTime = TIME_getSimTime(radio.node);
    clocktype delay
        = MAX(EXTERNAL_QueryExternalTime(m_ifaceData->m_hla->iface) - simTime, 0);

    Node* node = radio.node;

    unsigned interfaceIndex;
    for (interfaceIndex = 0;
         interfaceIndex < (unsigned) node->numberInterfaces;
         interfaceIndex++)
    {
        Message* msg = MESSAGE_Alloc(node,
                                     MAC_LAYER,
                                     0,
                                     eventType);

        MESSAGE_SetInstanceId(msg, (short) interfaceIndex);

        MESSAGE_InfoAlloc(node, msg, sizeof(FaultType));
        FaultType& info = *((FaultType*) MESSAGE_ReturnInfo(msg));
        info = STATIC_FAULT;

        MESSAGE_Send(node, msg, delay);
    }//for//
}

void
HlaFedAmb::ScheduleChangeMaxTxPower(
    Node *node,
    const HlaEntity& entity,
    unsigned damageState,
    clocktype delay)
{
    Message* msg = MESSAGE_Alloc(node,
                                 EXTERNAL_LAYER,
                                 EXTERNAL_HLA,
                                 MSG_EXTERNAL_HLA_ChangeMaxTxPower);

    MESSAGE_InfoAlloc(node, msg, sizeof(HlaChangeMaxTxPowerInfo));
    HlaChangeMaxTxPowerInfo& info
        = *((HlaChangeMaxTxPowerInfo*) MESSAGE_ReturnInfo(msg));
    info.entityPtr = &entity;
    info.damageState = damageState;

    MESSAGE_Send(node, msg, delay);
}
