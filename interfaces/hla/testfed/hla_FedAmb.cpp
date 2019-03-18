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

#include "RTI.hh"
#include "fedtime.hh"

#include "hla_HashTable.h"
#include "hla_gcclatlon.h"
#include "hla_archspec.h"
#include "hla_rpr_fom.h"
#include "hla_shared.h"
#include "hla_hashfcns.h"
#include "hla_main.h"
#include "hla.h"
#include "hla_FedAmb.h"


std::ostream &operator<<( std::ostream &out, const RTI::FedTime &theTime );


HlaFedAmb::HlaFedAmb()
    : m_siteId(1),
      m_applicationId(1),
      m_objectsRegistered(false),
      m_numNetworks(0),
      m_networks(NULL),
      m_numRadios(0),
      m_radios(NULL),
      m_numEntities(0),
      m_entities(NULL)
{
    ValidateConstants();

    const unsigned numBuckets = 100;
    const unsigned numItemsPerBucket = 1;

    m_entityIdToEntityPtrHash.Init(
        numBuckets,
        numItemsPerBucket,
        sizeof(HlaEntityId),
        sizeof(HlaEntity*),
        HlaEntityKeyHashFcn,
        HlaEntityKeysEqualFcn);

    m_markingDataToEntityPtrHash.Init(
        numBuckets,
        numItemsPerBucket,
        g_hlaMarkingDataBufSize,
        sizeof(HlaEntity*),
        HlaStringKeyHashFcn,
        HlaStringKeysEqualFcn);

    m_nodeIdToRadioPtrHash.Init(
        numBuckets,
        numItemsPerBucket,
        sizeof(unsigned),
        sizeof(HlaRadio*),
        HlaUnsignedKeyHashFcn,
        HlaUnsignedKeysEqualFcn);

    m_radioKeyToRadioPtrHash.Init(
        numBuckets,
        numItemsPerBucket,
        sizeof(HlaRadioKey),
        sizeof(HlaRadio*),
        HlaRadioKeyHashFcn,
        HlaRadioKeysEqualFcn);
}

HlaFedAmb::~HlaFedAmb()
throw(RTI::FederateInternalError)
{
    FreeRadios();
    FreeEntities();
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
    //cout << "FED: startRegistrationForObjectClass" << endl;
}

void
HlaFedAmb::stopRegistrationForObjectClass(
    RTI::ObjectClassHandle theClass)
throw (
    RTI::ObjectClassNotPublished,
    RTI::FederateInternalError)
{
    //cout << "FED: stopRegistrationForObjectClass" << endl;
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
    cout << "FED: discoverObjectInstance" << endl;
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
    cout << "FED: reflectAttributeValues (RO)" << endl;
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

    if (theInteraction == m_dataHandle)
    {
        ReceiveDataIxn(theParameters);
    }
    else
    {
        HlaReportError("Unknown interaction class", __FILE__, __LINE__);
    }
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
    cout << "FED: removeObjectInstance (RO)" << endl;
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

// Functions not defined in the base class ------------------------------------

void
HlaFedAmb::ValidateConstants()
{
}

void
HlaFedAmb::MallocRadios()
{
    HlaCheckNoMalloc(m_radios, __FILE__, __LINE__);
    m_radios = new HlaRadio [m_numRadios];
    HlaCheckMalloc(m_radios, __FILE__, __LINE__);
}

void
HlaFedAmb::FreeRadios()
{
    if (m_radios)
    {
        delete [] m_radios;
        m_radios = NULL;
    }
}

void
HlaFedAmb::MallocEntities()
{
    HlaCheckNoMalloc(m_entities, __FILE__, __LINE__);
    m_entities = new HlaEntity [m_numEntities];
    HlaCheckMalloc(m_entities, __FILE__, __LINE__);
}

void
HlaFedAmb::FreeEntities()
{
    if (m_entities)
    {
        delete [] m_entities;
        m_entities = NULL;
    }
}

void
HlaFedAmb::MallocNetworks()
{
    HlaCheckNoMalloc(m_networks, __FILE__, __LINE__);
    m_networks = new HlaNetwork [m_numNetworks];
    HlaCheckMalloc(m_networks, __FILE__, __LINE__);
}

void
HlaFedAmb::FreeNetworks()
{
    if (m_networks)
    {
        delete [] m_networks;
        m_networks = NULL;
    }
}

void
HlaFedAmb::ReceiveDataIxn(
    const RTI::ParameterHandleValuePairSet& theParameters)
{
    cout << "FED: Received Data interaction" << endl;

    HlaDataIxnInfo dataIxnInfo;
    memset(&dataIxnInfo, 0, sizeof(dataIxnInfo));

    if (!ReadDataIxnParameters(theParameters, dataIxnInfo)) { return; }

#if 0
    HlaVariableDatumInfo& vdInfo = dataIxnInfo.variableDatumSetInfo.variableDatumsInfo[0];

    const unsigned terminateCesNotificationDatumId = 66900;

    if (vdInfo.datumId == terminateCesNotificationDatumId)
    {
        ProcessTerminateCesRequest();
    }
    else
    {
        if (m_debug2)
        {
            cout << "FED: Ignoring Data interaction with DatumID "
                    << vdInfo.datumId << endl;
        }
    }
#endif /* 0 */
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
                if (m_debug)
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
                if (m_debug)
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
                if (m_debug)
                {
                    cout << "FED: Ignoring Data interaction where"
                            " DatumLength is not a multiple of 8" << endl;
                }

                return false;
            }

            unsigned datumLengthInBytes = vdInfo.datumLength / 8;

            if (datumLengthInBytes > g_hlaDatumValueBufSize)
            {
                if (m_debug)
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
            HlaReportError("Unknown parameter handle", __FILE__, __LINE__);
        }
    }//for//

    return true;
}
