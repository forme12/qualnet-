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

#include <iostream>
#include <vector>
#include <string>
#include <map>

#include "FedAmb.h"
#include "HLAbase.h"
#include "archspec.h"

using namespace SNT_HLA;

void FedAmb::Init(const std::string& federationName, const std::string& fedPath, const std::string& federateName) throw (RTI::Exception)
{
//    allInteractionClasses.clear();
//    allObjectClasses.clear();
//    allObjects.clear();
    numUpdatesReceived = 0;
    federationCreated = false;
    federationJoined = false;
    rti = new RTI::RTIambassador;
    try
    {
        std::cout << "trying to create federation " << federationName << " using " << fedPath << std::endl << std::flush;
        rti->createFederationExecution(federationName.c_str(), fedPath.c_str());
    }
    catch (RTI::FederationExecutionAlreadyExists)
    {
        std::cout << "federation " << federationName << " already exists." << std::endl << std::flush;
    }
    catch(RTI::Exception &e)
    {
        std::cout << "operation RTI::RTIambassador::createFederationExecution failed " << e._reason << std::endl << std::flush;
        throw;
    }
    federationCreated = true;

    while (1)
    {
        try
        {
            std::cout << "trying to join federation " << federationName << std::endl << std::flush;
            rti->joinFederationExecution(federateName.c_str(), federationName.c_str(), this);
            break;
        }
        catch (RTI::FederationExecutionDoesNotExist&)
        {
            Sleep(1000);
        }
        catch(RTI::Exception &e)
        {
            std::cout << "operation RTI::RTIambassador::joinFederationExecution failed " << e._reason << std::endl << std::flush;
            throw;
        }
    }
    federationJoined = true;
    std::cout << "federation " << federationName << " joined" << std::endl << std::flush;
}

void FedAmb::Cleanup(const std::string& federationName) throw (RTI::Exception)
{
    if( !rti )
        return;
    try
    {
        if( federationJoined )
        {
            rti->resignFederationExecution(
                RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
        }
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "resignFederationExecution() produced " << &e;
        throw;
    }
    catch (...)
    {
        std::cerr << "resignFederationExecution() produced unknown exception.";
        throw;
    }
    try
    {
        if( federationCreated )
        {
            rti->destroyFederationExecution(federationName.c_str());
        }
    }
    catch (RTI::FederatesCurrentlyJoined&)
    {
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "destroyFederationExecution() produced >" << &e << '<';
        throw;
    }
    catch (...)
    {
        std::cerr << "destroyFederationExecution() produced unknown exception.";
        throw;
    }
    delete rti;
    rti = 0;
}

void FedAmb::Subscribe(HLAClass* cls) throw (RTI::Exception)
{
    std::cout << "Subscribing to " << cls->className << std::endl;
    cls->Subscribe(rti);
    allObjectClasses[cls->handle()] = cls;
}
void FedAmb::Collect(int timeout)
{
    int timeSinceUpdate = 0;
    while(timeout > timeSinceUpdate)
    {
        Tick();
        Sleep(1000);
        if(update_occurred)
            timeSinceUpdate = 0;
        else
            timeSinceUpdate++;
    }
}

void FedAmb::Tick()
{
    update_occurred = false;
    rti->tick();
}

Interaction* FedAmb::findInteractionClass(RTI::InteractionClassHandle theHandle)
{
    std::map<RTI::InteractionClassHandle, Interaction*>::iterator it = allInteractionClasses.find(theHandle);
    if( it == allInteractionClasses.end() )
        return 0;
    else
        return (*it).second;
}
HLAClass* FedAmb::findObjectClass(RTI::ObjectClassHandle theClass)
{
    std::map<RTI::ObjectClassHandle, HLAClass*>::iterator it = allObjectClasses.find(theClass);
    if( it ==  allObjectClasses.end() )
        return 0;
    else
        return (*it).second;
}
HLAClass* FedAmb::findObjectClass(const std::string &className)
{
    std::map<RTI::ObjectClassHandle, HLAClass*>::iterator it = allObjectClasses.begin();
    while( it !=  allObjectClasses.end() )
    {
        if( (*it).second->className == className )
            return (*it).second;
        it++;
    }
    return 0;
}
HLAObject* FedAmb::findObject(RTI::ObjectHandle theObject)
{
    std::map<RTI::ObjectHandle, HLAObject*>::iterator it =  allObjects.find(theObject);
    if( it ==  allObjects.end() )
        return 0;
    else
        return (*it).second;
}

void FedAmb::synchronizationPointRegistrationSucceeded(const char *label) throw (RTI::FederateInternalError) {}
void FedAmb::synchronizationPointRegistrationFailed(const char *label) throw (RTI::FederateInternalError) {}
void FedAmb::announceSynchronizationPoint(const char *label, const char *tag) throw (RTI::FederateInternalError) {}
void FedAmb::federationSynchronized(const char *label) throw (RTI::FederateInternalError) {}
void FedAmb::initiateFederateSave(const char *label) throw (RTI::UnableToPerformSave, RTI::FederateInternalError) {}
void FedAmb::federationSaved() throw (RTI::FederateInternalError) {}
void FedAmb::federationNotSaved() throw (RTI::FederateInternalError) {}
void FedAmb::requestFederationRestoreSucceeded(const char *label) throw (RTI::FederateInternalError) {}
void FedAmb::requestFederationRestoreFailed(const char *label, const char *reason) throw (RTI::FederateInternalError) {}
void FedAmb::federationRestoreBegun() throw (RTI::FederateInternalError) {}
void FedAmb::initiateFederateRestore(const char *label, RTI::FederateHandle handle) throw (RTI::SpecifiedSaveLabelDoesNotExist, RTI::CouldNotRestore, RTI::FederateInternalError) {}
void FedAmb::federationRestored() throw (RTI::FederateInternalError) {}
void FedAmb::federationNotRestored() throw (RTI::FederateInternalError) {} 
void FedAmb::startRegistrationForObjectClass(RTI::ObjectClassHandle theClass) throw (RTI::ObjectClassNotPublished, RTI::FederateInternalError) {}
void FedAmb::stopRegistrationForObjectClass(RTI::ObjectClassHandle theClass) throw (RTI::ObjectClassNotPublished, RTI::FederateInternalError) {}

void FedAmb::turnInteractionsOn(RTI::InteractionClassHandle theHandle)
        throw (RTI::InteractionClassNotPublished, RTI::FederateInternalError)
{
    Interaction* I = findInteractionClass(theHandle);
    if( !I )
        throw RTI::InteractionClassNotPublished("Unknown Interaction");
    else
        I->turnOn();
}
void FedAmb::turnInteractionsOff(RTI::InteractionClassHandle theHandle)
        throw (RTI::InteractionClassNotPublished, RTI::FederateInternalError)
{
    Interaction* I = findInteractionClass(theHandle);
    if( !I )
        throw RTI::InteractionClassNotPublished("Unknown Interaction");
    else
        I->turnOff();
}
void FedAmb::discoverObjectInstance(RTI::ObjectHandle theObject,
                                       RTI::ObjectClassHandle theObjectClass,
                                       const char * theObjectName)
                                throw (RTI::CouldNotDiscover,
                                       RTI::ObjectClassNotKnown,
                                       RTI::FederateInternalError)
{
    std::cout << __FUNCTION__ << " thread " << GetCurrentThreadId() << std::endl << std::flush;
    HLAClass* cls = findObjectClass(theObjectClass);
    if( !cls )
        throw RTI::ObjectClassNotKnown("Unknown class");

    HLAObject* obj = findObject(theObject);
    if( obj )
        throw RTI::CouldNotDiscover("Object already exists");

    obj = cls->newObject(theObject, theObjectName);
    if( !obj )
        throw RTI::CouldNotDiscover("Could not create Object");

    allObjects[theObject] = obj;

    std::cout << "New HLA object discovered - name " << theObjectName;
    std::cout << ", handle " << theObject;
    std::cout << ", class " << cls->className;
    std::cout << std::endl << std::flush;

    RTI::AttributeHandleSet* set = cls->getAttributeHandleSet();
    rti->requestObjectAttributeValueUpdate(theObject, *set);
    delete set;
    update_occurred = true;
}
void FedAmb::reflectAttributeValues(RTI::ObjectHandle theObject,
                                       const RTI::AttributeHandleValuePairSet& theAttributes,
                                       const RTI::FedTime& theTime,
                                       const char *theTag,
                                       RTI::EventRetractionHandle theHandle)
                                throw (RTI::ObjectNotKnown,
                                       RTI::AttributeNotKnown,
                                       RTI::FederateOwnsAttributes,
                                       RTI::InvalidFederationTime,
                                       RTI::FederateInternalError)
{
    reflectAttributeValues(theObject, theAttributes, theTag);
}
void FedAmb::reflectAttributeValues(RTI::ObjectHandle theObject,
                                       const RTI::AttributeHandleValuePairSet& theAttributes,
                                       const char *theTag)
                                throw (RTI::ObjectNotKnown,
                                       RTI::AttributeNotKnown,
                                       RTI::FederateOwnsAttributes,
                                       RTI::FederateInternalError)
{
    numUpdatesReceived++;
    HLAObject* obj = findObject(theObject);
    if( !obj )
        throw RTI::ObjectNotKnown("Unknown Object");
    std::cout << "Setting attributes - object name " << obj->myName <<
        ", class " << obj->myClass->className;
    std::cout << std::endl;
    bool alreadyUpdated = obj->updated;
    obj->reflectAttributeValues(theAttributes, theTag);
    if( !alreadyUpdated && obj->updated)
        obj->myClass->numUpdated++;
    std::cout << std::endl << std::flush;
    update_occurred = true;
}
void FedAmb::receiveInteraction(RTI::InteractionClassHandle theInteraction, const RTI::ParameterHandleValuePairSet& theParameters, const RTI::FedTime& theTime, const char *theTag, RTI::EventRetractionHandle theHandle) throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError) {}
void FedAmb::receiveInteraction(RTI::InteractionClassHandle theInteraction, const RTI::ParameterHandleValuePairSet& theParameters, const char *theTag) throw (RTI::InteractionClassNotKnown, RTI::InteractionParameterNotKnown, RTI::FederateInternalError) {}
void FedAmb::removeObjectInstance(RTI::ObjectHandle theObject, const RTI::FedTime& theTime, const char *theTag, RTI::EventRetractionHandle theHandle) throw (RTI::ObjectNotKnown, RTI::InvalidFederationTime, RTI::FederateInternalError) {}

void FedAmb::removeObjectInstance(RTI::ObjectHandle theObject,
                                    const char *theTag)
                            throw (RTI::ObjectNotKnown,
                                    RTI::FederateInternalError)
{
    std::map<RTI::ObjectHandle, HLAObject*>::iterator it;
    it = allObjects.find(theObject);
    if( it == allObjects.end() )
        throw RTI::ObjectNotKnown("Unknown Object");
    HLAObject* obj = it->second;
    HLAClass* cls = obj->myClass;
    std::cout << "Removing HLA object - name " << obj->myName;
    std::cout << ", handle " << theObject;
    std::cout << ", class " << cls->className;
    std::cout << std::endl << std::flush;

    allObjects.erase(it);
    std::set<HLAObject*>::iterator cit = cls->allObjects.find(obj);
    if( cit != cls->allObjects.end() )
        cls->allObjects.erase(cit);

    delete obj;
}

void FedAmb::attributesInScope(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError) {}
void FedAmb::attributesOutOfScope(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError) {}
void FedAmb::provideAttributeValueUpdate(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError) {}
void FedAmb::turnUpdatesOnForObjectInstance(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError) {} 
void FedAmb::turnUpdatesOffForObjectInstance(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError) {} 
void FedAmb::requestAttributeOwnershipAssumption(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& offeredAttributes, const char *theTag) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError) {} 
void FedAmb::attributeOwnershipDivestitureNotification(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& releasedAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::AttributeDivestitureWasNotRequested, RTI::FederateInternalError) {}
void FedAmb::attributeOwnershipAcquisitionNotification(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& securedAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeAcquisitionWasNotRequested, RTI::AttributeAlreadyOwned, RTI::AttributeNotPublished, RTI::FederateInternalError) {} 
void FedAmb::attributeOwnershipUnavailable(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotDefined, RTI::AttributeAlreadyOwned, RTI::AttributeAcquisitionWasNotRequested, RTI::FederateInternalError) {} 
void FedAmb::requestAttributeOwnershipRelease(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& candidateAttributes, const char *theTag) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotOwned, RTI::FederateInternalError) {} 
void FedAmb::confirmAttributeOwnershipAcquisitionCancellation(RTI::ObjectHandle theObject, const RTI::AttributeHandleSet& theAttributes) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::AttributeNotDefined, RTI::AttributeAlreadyOwned, RTI::AttributeAcquisitionWasNotCanceled, RTI::FederateInternalError) {} 
void FedAmb::informAttributeOwnership(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute, RTI::FederateHandle theOwner) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError) {} 
void FedAmb::attributeIsNotOwned(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError) {}
void FedAmb::attributeOwnedByRTI(RTI::ObjectHandle theObject, RTI::AttributeHandle theAttribute) throw (RTI::ObjectNotKnown, RTI::AttributeNotKnown, RTI::FederateInternalError) {}
void FedAmb::timeRegulationEnabled(const RTI::FedTime& theFederateTime) throw (RTI::InvalidFederationTime, RTI::EnableTimeRegulationWasNotPending, RTI::FederateInternalError) {}
void FedAmb::timeConstrainedEnabled(const RTI::FedTime& theFederateTime) throw (RTI::InvalidFederationTime, RTI::EnableTimeConstrainedWasNotPending, RTI::FederateInternalError) {}
void FedAmb::timeAdvanceGrant(const RTI::FedTime& theTime) throw (RTI::InvalidFederationTime, RTI::TimeAdvanceWasNotInProgress, RTI::FederationTimeAlreadyPassed, RTI::FederateInternalError) {}
void FedAmb::requestRetraction(RTI::EventRetractionHandle theHandle) throw (RTI::EventNotKnown, RTI::FederateInternalError) {}


