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

#ifndef _RPR_FOM_H_
#define _RPR_FOM_H_

#include "HLAbase.h"
#include "hla_types.h"
#include "rpr_types.h"
#include "FedAmb.h"
#include "Config.h"
#include <set>
#include <math.h>

namespace SNT_HLA
{


class BaseEntity : public HLAObject
{
    public:
        EntityIdentifierStruct EntityIdentifier;
        EntityTypeStruct EntityType;
        OrientationStruct Orientation;
        WorldLocationStruct WorldLocation;

        BaseEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls);
};

class AggregateEntity : public BaseEntity
{
    public:
        class less
        {
            public:
                bool operator()(const AggregateEntity* a1, const AggregateEntity* a2) const
                {
                    return a1->EntityIdentifier < a2->EntityIdentifier;
                }
        };
        AggregateMarkingStruct AggregateMarking;
        AggregateStateEnum8 AggregateState;
        DimensionStruct Dimensions;
        RTIObjectIdArrayStruct EntityIDs;
        ForceIdentifierEnum8 ForceID;
        FormationEnum32 Formation;
        unsigned short NumberOfEntities;
        std::vector<SilentAggregateStruct> SilentAggregates;
        std::vector<SilentEntityStruct> SilentEntities;
        RTIObjectIdArrayStruct SubAggregateIDs;
        std::vector<VariableDatumStruct> VariableDatums;

        AggregateEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls);
};

class PhysicalEntity : public BaseEntity
{
    public:
        DamageStatusEnum32 DamageState;
        ForceIdentifierEnum8 ForceIdentifier;
        MarkingStruct Marking;
        PhysicalEntity(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls);
};

class EmbeddedSystem : public HLAObject
{
    public:
        EntityIdentifierStruct EntityIdentifier;
        RelativePositionStruct RelativePosition;

        EmbeddedSystem(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls);
};

class RadioTransmitter : public EmbeddedSystem
{
    public:
        UnsignedInteger64BE Frequency;
        unsigned short RadioIndex;
        RadioTypeStruct RadioSystemType;
        TransmitterOperationalStatusEnum8 TransmitterOperationalStatus;

        RadioTransmitter(RTI::ObjectHandle theObject, const char * theObjectName, HLAClass* cls);
};

};


#endif
