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

#include "hla_shared.h"
#include "external_socket.h"

// (2^31 - 1) / 3600, is used to compute or decode a DIS timestamp.
// The figure is rounded to a value just before further digits would be
// ignored.

const double g_hlaTimestampRatio = 596523.235277778;

bool
operator ==(const HlaEntityId& op1, const HlaEntityId& op2)
{
    return (   op1.siteId        == op2.siteId
            && op1.applicationId == op2.applicationId
            && op1.entityNumber  == op2.entityNumber);
}

bool
operator !=(const HlaEntityId& op1, const HlaEntityId& op2)
{
    return !(   op1.siteId        == op2.siteId
             && op1.applicationId == op2.applicationId
             && op1.entityNumber  == op2.entityNumber);
}

void
HlaMakeEntityIdString(const HlaEntityId& entityId, char* entityIdString)
{
    sprintf(entityIdString, "%u.%u.%u",
            entityId.siteId,
            entityId.applicationId,
            entityId.entityNumber);
}

void
HlaNtohEntityId(HlaEntityId& entityId)
{
    EXTERNAL_ntoh(&entityId.siteId, sizeof(entityId.siteId));
    EXTERNAL_ntoh(&entityId.applicationId, sizeof(entityId.applicationId));
    EXTERNAL_ntoh(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
HlaHtonEntityId(HlaEntityId& entityId)
{
    EXTERNAL_hton(&entityId.siteId, sizeof(entityId.siteId));
    EXTERNAL_hton(&entityId.applicationId, sizeof(entityId.applicationId));
    EXTERNAL_hton(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
HlaCopyToOffsetAndHtonEntityId(
    void* dst, unsigned& offset, const void* src)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    HlaCopyToOffsetAndHton(
        dst, offset, &uchar_src[0], sizeof(unsigned short));
    HlaCopyToOffsetAndHton(
        dst, offset, &uchar_src[2], sizeof(unsigned short));
    HlaCopyToOffsetAndHton(
        dst, offset, &uchar_src[4], sizeof(unsigned short));
}

unsigned
HlaGetTimestamp()
{
    return HlaConvertDoubleToTimestamp(HlaGetNumSecondsPastHour(), 1);
}

double
HlaConvertTimestampToDouble(unsigned timestamp)
{
    // Convert DIS timestamp to seconds.

    // It is NOT guaranteed that converting from one type to the other,
    // then back to the original type, will provide the original value.

    timestamp >>= 1;
    return ((double) timestamp) / g_hlaTimestampRatio;
}

unsigned
HlaConvertDoubleToTimestamp(double double_timestamp, bool absolute)
{
    // Convert seconds to DIS timestamp.

    // It is NOT guaranteed that converting from one type to the other,
    // then back to the original type, will provide the original value.

    // DIS timestamps are flipped to 0 after one hour (3600 seconds).

    assert(double_timestamp < 3600.0);
    unsigned timestamp = (unsigned) (double_timestamp * g_hlaTimestampRatio);

    // Check high-order bit is still 0.

    assert((timestamp & 0x80000000) == 0);
    timestamp <<= 1;

    if (absolute) { return timestamp |= 1; }
    else { return timestamp; }
}

bool HlaEntityId::less::operator()(const HlaEntityId &id1, const HlaEntityId &id2) const
{
    if (id1.siteId < id2.siteId)
    {
        return true;
    }
    else if (id1.siteId > id2.siteId)
    {
        return false;
    }
    if (id1.applicationId < id2.applicationId)
    {
        return true;
    }
    else if (id1.applicationId > id2.applicationId)
    {
        return false;
    }
    if (id1.entityNumber < id2.entityNumber)
    {
        return true;
    }
    else
    {
        return false;
    }
}

