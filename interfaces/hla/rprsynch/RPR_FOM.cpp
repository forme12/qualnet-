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

#include <RTI.hh>
#include <iostream>
#include "Config.h"
#include "HLAbase.h"
#include "hla_types.h"
#include "rpr_types.h"
#include "RPR_FOM.h"
#include <sstream>

namespace SNT_HLA
{

BaseEntity::BaseEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
          HLAObject(theObject, theObjectName, cls)
{
    attributes[cls->attributes["EntityType"]] = new Attribute<EntityTypeStruct>(EntityType);
    attributes[cls->attributes["EntityIdentifier"]] = new Attribute<EntityIdentifierStruct>(EntityIdentifier);
    attributes[cls->attributes["Orientation"]] = new Attribute<OrientationStruct>(Orientation);
    attributes[cls->attributes["WorldLocation"]] = new Attribute<WorldLocationStruct>(WorldLocation);
}

template <>
void Factory<BaseEntity>::defineClass()
{
    static const char* name = "BaseEntity";
    static const char* attrs[] = {
        "EntityType",
        "EntityIdentifier",
        "Orientation",
        "WorldLocation",
        0};
    theClass = new HLAClass(name, attrs, *this);
}

AggregateEntity::AggregateEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
          BaseEntity(theObject, theObjectName, cls->baseClass)
{
    attributes[cls->attributes["AggregateMarking"]] = new Attribute<AggregateMarkingStruct>(AggregateMarking);
//    Marking
    attributes[cls->attributes["AggregateState"]] = new Attribute<AggregateStateEnum8>(AggregateState);
    attributes[cls->attributes["Dimensions"]] = new Attribute<DimensionStruct>(Dimensions);
    attributes[cls->attributes["EntityIDs"]] = new Attribute<RTIObjectIdArrayStruct>(EntityIDs);
    attributes[cls->attributes["ForceID"]] = new Attribute<ForceIdentifierEnum8>(ForceID);
    attributes[cls->attributes["Formation"]] = new Attribute<FormationEnum32>(Formation);
    attributes[cls->attributes["NumberOfEntities"]] = new Attribute<unsigned short>(NumberOfEntities);
//    attributes[cls->attributes["SilentAggregates"]] = new Attribute<std::vector<SilentAggregateStruct> >(SilentAggregates);
//    attributes[cls->attributes["SilentEntities"]] = new Attribute<std::vector<SilentEntityStruct> >(SilentEntities);
    attributes[cls->attributes["SubAggregateIDs"]] = new Attribute<RTIObjectIdArrayStruct>(SubAggregateIDs);
//    attributes[cls->attributes["VariableDatums"]] = new Attribute<std::vector<VariableDatumStruct> >(VariableDatums);
}

template <>
void Factory<AggregateEntity>::defineClass()
{
    static const char* name = "BaseEntity.AggregateEntity";
    static const char* attrs[] = {
        "AggregateMarking",
        "AggregateState",
        "Dimensions",
        "EntityIDs",
        "ForceID",
        "Formation",
        "NumberOfEntities",
        "SilentAggregates",
        "SilentEntities",
        "SubAggregateIDs",
        "VariableDatums",
        0};
    theClass = new HLAClass(name, attrs, *this, Factory<BaseEntity>::instance().getClass());
}

PhysicalEntity::PhysicalEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
            BaseEntity(theObject, theObjectName, cls->baseClass)
{
    myClass = cls;
    DamageState = DamageStatusEnum::NoDamage;
    ForceIdentifier = ForceIdentifierEnum::Other;

    attributes[cls->attributes["DamageState"]] = new Attribute<DamageStatusEnum32>(DamageState);
    attributes[cls->attributes["ForceIdentifier"]] = new Attribute<ForceIdentifierEnum8>(ForceIdentifier);
    attributes[cls->attributes["Marking"]] = new Attribute<MarkingStruct>(Marking);
}

template <>
void Factory<PhysicalEntity>::defineClass()
{
    static const char* name = "BaseEntity.PhysicalEntity";
    static const char* attrs[] = {
        "DamageState",
        "ForceIdentifier",
        "Marking",
        0};
    theClass = new HLAClass(name, attrs, *this, Factory<BaseEntity>::instance().getClass());
}

EmbeddedSystem::EmbeddedSystem(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
          HLAObject(theObject, theObjectName, cls)
{
    attributes[cls->attributes["EntityIdentifier"]] = new Attribute<EntityIdentifierStruct>(EntityIdentifier);
    attributes[cls->attributes["RelativePosition"]] = new Attribute<RelativePositionStruct>(RelativePosition);
}

template <>
void Factory<EmbeddedSystem>::defineClass()
{
    static const char* name = "EmbeddedSystem";
    static const char* attrs[] = {
        "EntityIdentifier",
        "RelativePosition",
        0};
    theClass = new HLAClass(name, attrs, *this);
}

RadioTransmitter::RadioTransmitter(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls) :
          EmbeddedSystem(theObject, theObjectName, cls->baseClass)
{
    myClass = cls;
    attributes[cls->attributes["Frequency"]] = new Attribute<UnsignedInteger64BE>(Frequency);
    attributes[cls->attributes["RadioIndex"]] = new Attribute<unsigned short>(RadioIndex);
    attributes[cls->attributes["RadioSystemType"]] = new Attribute<RadioTypeStruct>(RadioSystemType);
    attributes[cls->attributes["TransmitterOperationalStatus"]] = new Attribute<TransmitterOperationalStatusEnum8>(TransmitterOperationalStatus);
}

template <>
void Factory<RadioTransmitter>::defineClass()
{
    static const char* name = "EmbeddedSystem.RadioTransmitter";
    static const char* attrs[] = {
        "Frequency",
        "RadioIndex",
        "RadioSystemType",
        "TransmitterOperationalStatus",
        0};
    theClass = new HLAClass(name, attrs, *this, Factory<EmbeddedSystem>::instance().getClass());
}

};