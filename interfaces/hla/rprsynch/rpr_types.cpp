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

#include "rpr_types.h"
#include "hla_gcclatlon.h"
#include "Config.h"

#include <iostream>
#include <sstream>

namespace SNT_HLA
{

const unsigned char EntityTypeStruct::ANY_KIND = 0xff;
const unsigned char EntityTypeStruct::ANY_DOMAIN = 0xff;
const unsigned short EntityTypeStruct::ANY_COUNTRY = 0xffff;
const unsigned char EntityTypeStruct::ANY_CATEGORY = 0xff;
const unsigned char EntityTypeStruct::ANY_SUBCATEGORY = 0xff;
const unsigned char EntityTypeStruct::ANY_SPECIFIC = 0xff;
const unsigned char EntityTypeStruct::ANY_EXTRA = 0xff;

bool operator<(const EntityTypeStruct& e1, const EntityTypeStruct& e2)
{
    if( e1.entityKind < e2.entityKind )
        return true;
    else if( e1.entityKind > e2.entityKind )
        return false;
    else if( e1.domain < e2.domain )
        return true;
    else if( e1.domain > e2.domain )
        return false;
    else if( e1.countryCode < e2.countryCode )
        return true;
    else if( e1.countryCode > e2.countryCode )
        return false;
    else if( e1.category < e2.category )
        return true;
    else if( e1.category > e2.category )
        return false;
    else if( e1.subcategory < e2.subcategory )
        return true;
    else if( e1.subcategory > e2.subcategory )
        return false;
    else if( e1.specific < e2.specific )
        return true;
    else if( e1.specific > e2.specific )
        return false;
    else
        return e1.extra < e2.extra;
}
const unsigned char RadioTypeStruct::ANY_KIND = 0xff;
const unsigned char RadioTypeStruct::ANY_DOMAIN = 0xff;
const unsigned short RadioTypeStruct::ANY_COUNTRY = 0xffff;
const unsigned char RadioTypeStruct::ANY_CATEGORY = 0xff;

RadioTypeStruct::RadioTypeStruct()
{
    *this = Config::instance().defaultRadioSystemType;
    std::string test("7,1,225,3,1,1");
    RadioTypeStruct t2(test);
}
RadioTypeStruct::RadioTypeStruct(const std::string& str)
{
    unsigned char comma;
    unsigned int num;
    std::stringstream sss(str);
    sss >> num;
    EntityKind = num;
    sss >> comma;
    sss >> num;
    Domain = num;
    sss >> comma;
    sss >> num;
    CountryCode = num;
    sss >> comma;
    sss >> num;
    Category = num;
    sss >> comma;
    sss >> num;
    NomenclatureVersion = NomenclatureVersionEnum8(num);
    sss >> comma;
    sss >> num;
    Nomenclature = NomenclatureEnum16(num);

}

bool operator<(const RadioTypeStruct& r1, const RadioTypeStruct& r2)
{
    if( r1.EntityKind < r2.EntityKind )
        return true;
    else if( r1.EntityKind > r2.EntityKind )
        return false;
    else if( r1.Domain < r2.Domain )
        return true;
    else if( r1.Domain > r2.Domain )
        return false;
    else if( r1.CountryCode < r2.CountryCode )
        return true;
    else if( r1.CountryCode > r2.CountryCode )
        return false;
    else if( r1.Category < r2.Category )
        return true;
    else if( r1.Category > r2.Category )
        return false;
    else if( r1.NomenclatureVersion < r2.NomenclatureVersion )
        return true;
    else if( r1.NomenclatureVersion > r2.NomenclatureVersion )
        return false;
    else
        return r1.Nomenclature < r2.Nomenclature;
}

template <>
void Attribute<EntityIdentifierStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[6];
    copyData(theAttributes, idx, data, sizeof(data));
    value.siteId =            fromNet<unsigned short>(data+0);
    value.applicationId =    fromNet<unsigned short>(data+2);
    value.entityNumber =    fromNet<unsigned short>(data+4);
    std::cout << "Entity ID " << value.siteId << ":";
    std::cout << value.applicationId << ":";
    std::cout << value.entityNumber;
    std::cout << std::endl;
}

template <>
void Attribute<EntityTypeStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[8];
    copyData(theAttributes, idx, data, sizeof(data));
    value.entityKind =    fromNet<unsigned char>(data+0);
    value.domain =        fromNet<unsigned char>(data+1);
    value.countryCode =    fromNet<unsigned short>(data+2);
    value.category =    fromNet<unsigned char>(data+4);
    value.subcategory =    fromNet<unsigned char>(data+5);
    value.specific =    fromNet<unsigned char>(data+6);
    value.extra =        fromNet<unsigned char>(data+7);
    std::cout << "Entity type " << (int) value.entityKind << ".";
    std::cout << (int) value.domain << ".";
    std::cout << (int) value.countryCode << ".";
    std::cout << (int) value.category << ".";
    std::cout << (int) value.subcategory << ".";
    std::cout << (int) value.specific << ".";
    std::cout << (int) value.extra;
    std::cout << std::endl;
}

template <>
void Attribute<OrientationStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[12];
    copyData(theAttributes, idx, data, sizeof(data));
    value.Psi =        fromNet<float>(data+0);
    value.Theta =    fromNet<float>(data+4);
    value.Phi =        fromNet<float>(data+8);
    std::cout << "Entity orientation " << value.Psi;
    std::cout << " " << value.Theta;
    std::cout << " " << value.Phi;
    std::cout << std::endl;
}

template <>
void Attribute<WorldLocationStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[24];
    copyData(theAttributes, idx, data, sizeof(data));
    value.x = fromNet<double>(data+0);
    value.y = fromNet<double>(data+8);
    value.z = fromNet<double>(data+16);
    HlaConvertGccToLatLonAlt(value.x, value.y, value.z, value.lat, value.lon, value.alt);
    std::cout << "Entity location " << value.x;
    std::cout << " " << value.y;
    std::cout << " " << value.z;
    std::cout << "   " << value.lat;
    std::cout << " " << value.lon;
    std::cout << " " << value.alt;
    std::cout << std::endl;
}

template <>
void Attribute<DamageStatusEnum32>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[4];
    copyData(theAttributes, idx, data, sizeof(data));
    value = DamageStatusEnum32(fromNet<unsigned int>(data+0));
    std::cout << "Damage Status " << value;
    std::cout << std::endl;
}

template <>
void Attribute<ForceIdentifierEnum8>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[1];
    copyData(theAttributes, idx, data, sizeof(data));
    value = ForceIdentifierEnum8(fromNet<unsigned char>(data+0));
    std::cout << "Force id " << value;
    std::cout << std::endl;
}

template <>
void Attribute<MarkingStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[12];
    copyData(theAttributes, idx, data, sizeof(data));
    value.MarkingEncodingType = MarkingEncodingEnum8(fromNet<unsigned char>(data+0));
    memcpy(value.MarkingData, data+1, sizeof(value.MarkingData)-1);
    value.MarkingData[11] = 0;
    value.MarkingData[10] = 0;    // bug in qualnet simulator
    std::cout << "Marking data " << value.MarkingEncodingType << " ";
    for(size_t i=0; i<sizeof(value.MarkingData); i++)
        std::cout << value.MarkingData[i];
    std::cout << std::endl;
}

template <>
void Attribute<RelativePositionStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[12];
    copyData(theAttributes, idx, data, sizeof(data));
    value.BodyXDistance = fromNet<float>(data+0);
    value.BodyYDistance = fromNet<float>(data+4);
    value.BodyZDistance = fromNet<float>(data+8);
    std::cout << "Relative Position " << value.BodyXDistance;
    std::cout << " " << value.BodyYDistance;
    std::cout << " " << value.BodyZDistance;
    std::cout << std::endl;
}

template <>
void Attribute<RadioTypeStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[8];
    copyData(theAttributes, idx, data, sizeof(data));
    value.EntityKind =            fromNet<unsigned char>(data+0);
    value.Domain =                fromNet<unsigned char>(data+1);
    value.CountryCode =            fromNet<unsigned short>(data+2);
    value.Category =            fromNet<unsigned char>(data+4);
    value.NomenclatureVersion =    NomenclatureVersionEnum8(fromNet<unsigned char>(data+5));
    value.Nomenclature =        NomenclatureEnum16(fromNet<unsigned short>(data+6));
    std::cout << "Radio type " << (int) value.EntityKind << ".";
    std::cout << (int) value.Domain << ".";
    std::cout << (int) value.CountryCode << ".";
    std::cout << (int) value.Category << ".";
    std::cout << (int) value.NomenclatureVersion << ".";
    std::cout << (int) value.Nomenclature;
    std::cout << std::endl;
}

template <>
void Attribute<TransmitterOperationalStatusEnum8>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[1];
    copyData(theAttributes, idx, data, sizeof(data));
    value = TransmitterOperationalStatusEnum8(fromNet<unsigned char>(data+0));
    std::cout << "Transmitter Operational Status " << value;
    std::cout << std::endl;
}

template <>
void Attribute<AggregateMarkingStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[12];
    copyData(theAttributes, idx, data, sizeof(data));
    value.MarkingEncodingType = MarkingEncodingEnum8(fromNet<unsigned char>(data+0));
    memcpy(value.MarkingData, data+1, sizeof(value.MarkingData)-1);
    value.MarkingData[31] = 0;
    std::cout << "Aggregate Marking data " << value.MarkingEncodingType << " ";
    for(size_t i=0; i<sizeof(value.MarkingData); i++)
        std::cout << value.MarkingData[i];
    std::cout << std::endl;
}

template <>
void Attribute<AggregateStateEnum8>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[1];
    copyData(theAttributes, idx, data, sizeof(data));
    value = AggregateStateEnum8(fromNet<unsigned char>(data+0));
    std::cout << "Aggregate State " << value;
    std::cout << std::endl;
}

template <>
void Attribute<DimensionStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[12];
    copyData(theAttributes, idx, data, sizeof(data));
    value.XAxisLength = fromNet<float>(data+0);
    value.YAxisLength = fromNet<float>(data+4);
    value.ZAxisLength = fromNet<float>(data+8);
    std::cout << "Dimension " << value.XAxisLength;
    std::cout << " " << value.YAxisLength;
    std::cout << " " << value.ZAxisLength;
    std::cout << std::endl;
}

template <>
void Attribute<RTIObjectIdArrayStruct>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char *data = new unsigned char[theAttributes.getValueLength(idx)];
    copyData(theAttributes, idx, data, theAttributes.getValueLength(idx));
    unsigned int len = fromNet<unsigned short>(data+0);
    char* string = (char *)(data+2);
    unsigned int i = 0;
    while( *string && i<len )
    {
        value.push_back(std::string(string));
        while( *string && i<len )
        {
            string++;
            i++;
        }
        string++;
        i++;
    }
    delete [] data;
    std::cout << "Object IDs";
    for( i=0; i<value.size(); i++ )
    {
        std::cout << " " << value[i];
    }
    std::cout << std::endl;
}

template <>
void Attribute<FormationEnum32>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[4];
    copyData(theAttributes, idx, data, sizeof(data));
    value = FormationEnum32(fromNet<unsigned char>(data+0));
    std::cout << "Formation " << value;
    std::cout << std::endl;
}


bool operator<(const EntityIdentifierStruct& id1, const EntityIdentifierStruct& id2)
{
    if( id1.siteId < id2.siteId )
        return true;
    else if( id1.siteId > id2.siteId )
        return false;
    else if( id1.applicationId < id2.applicationId )
        return true;
    else if( id1.applicationId > id2.applicationId )
        return false;
    else if( id1.entityNumber < id2.entityNumber )
        return true;
    else
        return false;
}

std::ostream& operator<<(std::ostream& out, const MarkingStruct& mark)
{
    int len = strlen((char*)mark.MarkingData);
    if( len > 11 )
    {
        char buf[12];
        memcpy( buf, mark.MarkingData, 11 );
        buf[11] = 0;
        out << buf;
    }
    int leadingBlanks = sizeof(mark.MarkingData) - len - 1;
    for( int i=0; i<leadingBlanks; i++ )
        out << ' ';
    out << mark.MarkingData;
    return out;
}
std::ostream& operator<<(std::ostream& out, const WorldLocationStruct& loc)
{
    out << loc.lat << ", " << loc.lon << ", " << loc.alt;
    return out;
}
std::ostream& operator<<(std::ostream& out, const RelativePositionStruct& pos)
{
    out << pos.BodyXDistance << ", " << pos.BodyYDistance << ", " << pos.BodyZDistance;
    return out;
}
std::ostream& operator<<(std::ostream& out, const RadioTypeStruct& rad)
{
    if( rad.EntityKind != 255 )
        out << (unsigned int) rad.EntityKind << ", ";
    else
        out << "-1, ";
    if( rad.Domain!= 255 )
        out << (unsigned int) rad.Domain << ", ";
    else
        out << "-1, ";
    if( rad.CountryCode != 65535 )
        out << (unsigned int) rad.CountryCode << ", ";
    else
        out << "-1, ";
    if( rad.Category != 255 )
        out << (unsigned int) rad.Category << ", ";
    else
        out << "-1, ";
    if( rad.NomenclatureVersion != -1 )
        out << (unsigned int) rad.NomenclatureVersion << ", ";
    else
        out << "-1, ";
    if( rad.Nomenclature != -1 )
        out << (unsigned int) rad.Nomenclature;
    else
        out << "-1";
    return out;
}
std::ostream& operator<<(std::ostream& out, const EntityIdentifierStruct& ent)
{
    out << ent.siteId << ":"
        << ent.applicationId << ":"
        << ent.entityNumber;
    return out;
}
std::ostream& operator<<(std::ostream& out, const EntityTypeStruct& ent)
{
    out << (unsigned int) ent.entityKind << ", "
        << (unsigned int) ent.domain << ", "
        << (unsigned int) ent.countryCode << ", "
        << (unsigned int) ent.category << ", "
        << (unsigned int) ent.subcategory << ", "
        << (unsigned int) ent.specific << ", "
        << (unsigned int) ent.extra;
    return out;
}

};

std::ostream& operator<<(std::ostream& out, SNT_HLA::ForceIdentifierEnum8 forceId)
{
    switch(forceId)
    {
        case SNT_HLA::ForceIdentifierEnum::Other:
            out << "Other";
            break;
        case SNT_HLA::ForceIdentifierEnum::Friendly:
            out << "Friendly";
            break;
        case SNT_HLA::ForceIdentifierEnum::Opposing:
            out << "Opposing";
            break;
        case SNT_HLA::ForceIdentifierEnum::Neutral:
            out << "Neutral";
            break;
        default:
            unsigned int id = forceId;
            out << "Unknown (" << id << ")";
            break;
    }
    return out;
}


