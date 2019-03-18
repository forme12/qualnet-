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

#ifndef _RPR_TYPES_H_
#define _RPR_TYPES_H_

#include "hla_types.h"
#include <map>
#include <vector>

namespace SNT_HLA
{

typedef struct EntityIdentifierStruct
{
    unsigned short siteId;
    unsigned short applicationId;
    unsigned short entityNumber;
    EntityIdentifierStruct() : siteId(0), applicationId(0), entityNumber(0) {}
} EntityIdentifierStruct;

class EntityTypeStruct
{
public:
        static const unsigned char ANY_KIND;
        static const unsigned char ANY_DOMAIN;
        static const unsigned short ANY_COUNTRY;
        static const unsigned char ANY_CATEGORY;
        static const unsigned char ANY_SUBCATEGORY;
        static const unsigned char ANY_SPECIFIC;
        static const unsigned char ANY_EXTRA;
        unsigned char  entityKind;
        unsigned char  domain;
        unsigned short countryCode;
        unsigned char  category;
        unsigned char  subcategory;
        unsigned char  specific;
        unsigned char  extra;
        EntityTypeStruct() :
            entityKind(0),
            domain(0),
            countryCode(0),
            category(0),
            subcategory(0),
            specific(0),
            extra(0) {}

};
bool operator<(const EntityTypeStruct& e1, const EntityTypeStruct& e2);

typedef struct OrientationStruct
{
    float Psi;
    float Theta;
    float Phi;
    float Azimuth;
    float Elevation;
    OrientationStruct() :
        Psi(0.0),
        Theta(0.0),
        Phi(0.0),
        Azimuth(0.0),
        Elevation(0.0) {}

} OrientationStruct;

typedef struct WorldLocationStruct
{
    double x;
    double y;
    double z;
    double lat;
    double lon;
    double alt;
    WorldLocationStruct() :
        x(0.0),
        y(0.0),
        z(0.0),
        lat(0.0),
        lon(0.0),
        alt(0.0) {}

} WorldLocationStruct;

namespace DamageStatusEnum
{
    typedef enum
    {
        NoDamage = 0,
        SlightDamage = 1,
        ModerateDamage = 2,
        Destroyed = 3
    } DamageStatusEnum32;
};
using namespace DamageStatusEnum;

namespace ForceIdentifierEnum
{
    typedef enum
    {
        Other = 0,
        Friendly = 1,
        Opposing = 2,
        Neutral = 3
    } ForceIdentifierEnum8;
};
using namespace ForceIdentifierEnum;

namespace  MarkingEncodingEnum
{
    typedef enum
    {
        Other = 0,
        ASCII = 1,
        ArmyMarkingCCTT = 2,
        DigitChevron = 3,
    } MarkingEncodingEnum8;
};
using namespace MarkingEncodingEnum;

typedef struct MarkingStruct
{
    MarkingEncodingEnum8 MarkingEncodingType;
    unsigned char MarkingData[12];
    MarkingStruct() : MarkingEncodingType(MarkingEncodingEnum::Other)
    {
        MarkingData[0] = 0;
    }
} MarkingStruct;

typedef struct AggregateMarkingStruct
{
    MarkingEncodingEnum8 MarkingEncodingType;
    unsigned char MarkingData[32];
} AggregateMarkingStruct;

typedef std::vector<std::string> RTIObjectIdArrayStruct;

typedef struct DimensionStruct
{
    float XAxisLength;
    float YAxisLength;
    float ZAxisLength;
} DimensionStruct;

namespace AggregateStateEnum
{
    typedef enum
    {
        Other = 0,
        Aggregated = 1,
        Disaggregated = 2,
        FullyDisaggregated = 3,
        PseudoDisaggregated = 4,
        PartiallyDisaggregated = 5
    } AggregateStateEnum8;
};
using namespace AggregateStateEnum;

namespace FormationEnum
{
    typedef enum
    {
        Other = 0,
        Assembly = 1,
        Vee = 2,
        Wedge = 3,
        Line = 4,
        Column = 5
    } FormationEnum32;
};
using namespace FormationEnum;

typedef struct SilentAggregateStruct
{
    EntityTypeStruct AggregateType;
    unsigned short NumberOfAggregatesOfThisType;
} SilentAggregateStruct;

typedef struct SilentEntityStruct
{
    unsigned short NumberOfEntitiesOfThisType;
    unsigned short NumberOfAppearanceRecords;
    EntityTypeStruct EntityType;
    unsigned int EntityAppearance;
} SilentEntityStruct;

namespace DatumIdentifierEnum
{
    typedef enum
    {
    } DatumIdentifierEnum32;
};
using namespace DatumIdentifierEnum;
typedef struct VariableDatumStruct
{
    DatumIdentifierEnum32 DatumID;
    unsigned int DatumLength;
    std::vector<unsigned char> DatumValue;
    std::vector<unsigned char> Padding;
} VariableDatumStruct;

typedef struct RelativePositionStruct
{
    float BodyXDistance;
    float BodyYDistance;
    float BodyZDistance;
} RelativePositionStruct;

namespace  NomenclatureVersionEnum
{
    typedef enum
    {
        ANY_VERSION = -1
    } NomenclatureVersionEnum8;
};
using namespace NomenclatureVersionEnum;

namespace  NomenclatureEnum
{
    typedef enum
    {
        ANY_NOMENCLATURE = -1
    } NomenclatureEnum16;
};
using namespace NomenclatureEnum;

class RadioTypeStruct
{
    public:
        static const unsigned char ANY_KIND;
        static const unsigned char ANY_DOMAIN;
        static const unsigned short ANY_COUNTRY;
        static const unsigned char ANY_CATEGORY;

        RadioTypeStruct();
        RadioTypeStruct(const std::string &str);
        unsigned char EntityKind;
        unsigned char Domain;
        unsigned short CountryCode;
        unsigned char Category;
        NomenclatureVersionEnum8 NomenclatureVersion;
        NomenclatureEnum16 Nomenclature;
};

bool operator<(const RadioTypeStruct& r1, const RadioTypeStruct& r2);

namespace TransmitterOperationalStatusEnum
{
typedef enum
{
    Off = 0,
    OnButNotTransmitting = 1,
    OnAndTransmitting = 2
} TransmitterOperationalStatusEnum8;
};
using namespace TransmitterOperationalStatusEnum;

bool operator<(const EntityIdentifierStruct& id1, const EntityIdentifierStruct& id2);

std::ostream& operator<<(std::ostream& out, const MarkingStruct& mark);
std::ostream& operator<<(std::ostream& out, const WorldLocationStruct& loc);
std::ostream& operator<<(std::ostream& out, const RelativePositionStruct& pos);
std::ostream& operator<<(std::ostream& out, const RadioTypeStruct& rad);
std::ostream& operator<<(std::ostream& out, const EntityTypeStruct& ent);
std::ostream& operator<<(std::ostream& out, const EntityIdentifierStruct& ent);


template <class T> class EntityTypeMapWithWildCards : public std::map<class EntityTypeStruct, T>
{
    public:
        typedef typename std::map<class EntityTypeStruct, T>::iterator _it;
        _it matchWC(EntityTypeStruct entityType)
        {
            _it it = this->find(entityType);
            if( it != this->end() )
                return it;
            it = this->lower_bound(entityType);
            if( it == this->end() )
                return it;
            if( it->first.entityKind != entityType.entityKind )
            {
                entityType.entityKind = EntityTypeStruct::ANY_KIND;
                return matchWC(entityType);
            }
            else if( it->first.domain != entityType.domain )
            {
                entityType.domain = EntityTypeStruct::ANY_DOMAIN;
                return matchWC(entityType);
            }
            else if( it->first.countryCode != entityType.countryCode )
            {
                entityType.countryCode = EntityTypeStruct::ANY_COUNTRY;
                return matchWC(entityType);
            }
            else if( it->first.category != entityType.category )
            {
                entityType.category = EntityTypeStruct::ANY_CATEGORY;
                return matchWC(entityType);
            }
            else if( it->first.subcategory != entityType.subcategory )
            {
                entityType.subcategory = EntityTypeStruct::ANY_SUBCATEGORY;
                return matchWC(entityType);
            }
            else if( it->first.specific != entityType.specific )
            {
                entityType.specific = EntityTypeStruct::ANY_SPECIFIC;
                return matchWC(entityType);
            }
            else if( it->first.extra != entityType.extra )
            {
                entityType.extra = EntityTypeStruct::ANY_EXTRA;
                return matchWC(entityType);
            }

            return it;
        }
};

template <class T> class RadioTypeMapWithWildCards : public std::map<class RadioTypeStruct, T>
{
    public:
        typedef typename std::map<class RadioTypeStruct, T>::iterator _it;
        _it matchWC(RadioTypeStruct radioType)
        {
            _it it = this->find(radioType);
            if( it != this->end() )
                return it;
            it = this->lower_bound(radioType);
            if( it == this->end() )
                return it;
            if( it->first.EntityKind != radioType.EntityKind )
            {
                radioType.EntityKind = RadioTypeStruct::ANY_KIND;
                return matchWC(radioType);
            }
            else if( it->first.Domain != radioType.Domain )
            {
                radioType.Domain = RadioTypeStruct::ANY_DOMAIN;
                return matchWC(radioType);
            }
            else if( it->first.CountryCode != radioType.CountryCode )
            {
                radioType.CountryCode = RadioTypeStruct::ANY_COUNTRY;
                return matchWC(radioType);
            }
            else if( it->first.Category != radioType.Category )
            {
                radioType.Category = RadioTypeStruct::ANY_CATEGORY;
                return matchWC(radioType);
            }
            else if( it->first.NomenclatureVersion != radioType.NomenclatureVersion )
            {
                radioType.NomenclatureVersion = NomenclatureVersionEnum::ANY_VERSION;
                return matchWC(radioType);
            }
            else if( it->first.Nomenclature != radioType.Nomenclature )
            {
                radioType.Nomenclature = NomenclatureEnum::ANY_NOMENCLATURE;
                return matchWC(radioType);
            }

            return it;
        }

};


};

std::ostream& operator<<(std::ostream& out, SNT_HLA::ForceIdentifierEnum8);

#endif

