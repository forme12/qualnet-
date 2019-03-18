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

#include "hla_types.h"
#include "hla_gcclatlon.h"

namespace SNT_HLA
{

template <> unsigned char fromNet<unsigned char>(const unsigned char* net_data)
{
    return *net_data;
}
template <> unsigned short fromNet<unsigned short>(const unsigned char* net_data)
{
    unsigned char data[2];
    data[0] = net_data[1];
    data[1] = net_data[0];
    return *(unsigned short*) data;
}
template <> unsigned int fromNet<unsigned int>(const unsigned char* net_data)
{
    unsigned char data[4];
    data[0] = net_data[3];
    data[1] = net_data[2];
    data[2] = net_data[1];
    data[3] = net_data[0];
    return *(unsigned int*) data;
}
template <> float fromNet<float>(const unsigned char* net_data)
{
    unsigned char data[4];
    data[0] = net_data[3];
    data[1] = net_data[2];
    data[2] = net_data[1];
    data[3] = net_data[0];
    return *(float*) data;
}
template <> double fromNet<double>(const unsigned char* net_data)
{
    unsigned char data[8];
    data[0] = net_data[7];
    data[1] = net_data[6];
    data[2] = net_data[5];
    data[3] = net_data[4];
    data[4] = net_data[3];
    data[5] = net_data[2];
    data[6] = net_data[1];
    data[7] = net_data[0];
    return *(double*) data;
}
template <> unsigned long long fromNet<unsigned long long>(const unsigned char* net_data)
{
    unsigned char data[8];
    data[0] = net_data[7];
    data[1] = net_data[6];
    data[2] = net_data[5];
    data[3] = net_data[4];
    data[4] = net_data[3];
    data[5] = net_data[2];
    data[6] = net_data[1];
    data[7] = net_data[0];
    return *(unsigned long long*) data;
}

template <>
void Attribute<unsigned short>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[2];
    copyData(theAttributes, idx, data, sizeof(data));
    value = fromNet<unsigned short>(data+0);
}

template <>
void Attribute<unsigned long long>::reflectAttribute(const RTI::AttributeHandleValuePairSet& theAttributes, int idx) throw (RTI::FederateInternalError)
{
    unsigned char data[8];
    copyData(theAttributes, idx, data, sizeof(data));
    value = fromNet<unsigned long long>(data+0);
}

};

