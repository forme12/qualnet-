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

#include "dis_shared.h"
#include "external_socket.h"

// (2^31 - 1) / 3600, is used to compute or decode a DIS timestamp.
// The figure is rounded to a value just before further digits would be
// ignored.

const double g_disTimestampRatio = 596523.235277778;

bool
operator ==(const DisEntityId& op1, const DisEntityId& op2)
{
    return (   op1.siteId        == op2.siteId
            && op1.applicationId == op2.applicationId
            && op1.entityNumber  == op2.entityNumber);
}

bool
operator !=(const DisEntityId& op1, const DisEntityId& op2)
{
    return !(   op1.siteId        == op2.siteId
             && op1.applicationId == op2.applicationId
             && op1.entityNumber  == op2.entityNumber);
}

void
DisMakeEntityIdString(const DisEntityId& entityId, char* entityIdString)
{
    sprintf(entityIdString, "%u.%u.%u",
            entityId.siteId,
            entityId.applicationId,
            entityId.entityNumber);
}

void
DisNtohEntityId(DisEntityId& entityId)
{
    EXTERNAL_ntoh(&entityId.siteId, sizeof(entityId.siteId));
    EXTERNAL_ntoh(&entityId.applicationId, sizeof(entityId.applicationId));
    EXTERNAL_ntoh(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
DisHtonEntityId(DisEntityId& entityId)
{
    EXTERNAL_hton(&entityId.siteId, sizeof(entityId.siteId));
    EXTERNAL_hton(&entityId.applicationId, sizeof(entityId.applicationId));
    EXTERNAL_hton(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
DisCopyToOffsetAndHtonEntityId(
    void* dst, unsigned& offset, const void* src)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    DisCopyToOffsetAndHton(
        dst, offset, &uchar_src[0], sizeof(unsigned short));
    DisCopyToOffsetAndHton(
        dst, offset, &uchar_src[2], sizeof(unsigned short));
    DisCopyToOffsetAndHton(
        dst, offset, &uchar_src[4], sizeof(unsigned short));
}

bool
DisPayloadContainsOnlyPdus(const char* payload, unsigned payloadSize)
{
    unsigned index = 0;

    const unsigned short minNumBytesWithLengthField = 10;

    if (payloadSize < minNumBytesWithLengthField) { return false; }

    while (1)
    {
        unsigned char protocolVersion = payload[index];

        if (protocolVersion < 3 || protocolVersion > 6) { break; }

        unsigned short length = DisGetPduLength(&payload[index]);

        if (length < minNumBytesWithLengthField) { return false; }

        index += length;

        if (index < payloadSize) { continue; }
        if (index == payloadSize) { return true; }
        if (index > payloadSize) { break; }
    }//while//

    return false;
}

unsigned char
DisGetExerciseIdentifier(const char* pduBuf)
{
    const unsigned offset = 1;
    return  *((unsigned char*) &pduBuf[offset]);
}

unsigned char
DisGetPduType(const char* pduBuf)
{
    const unsigned offset = 2;
    return  *((unsigned char*) &pduBuf[offset]);
}

unsigned short
DisGetPduLength(const char* pduBuf)
{
    const unsigned offset = 8;
    unsigned short length = *((unsigned short*) &pduBuf[offset]);
    EXTERNAL_ntoh(&length, sizeof(length));

    return length;
}

void
DisNtohPduHeader(DisHeader& pduHeader)
{
    EXTERNAL_ntoh(&pduHeader.timestamp, sizeof(pduHeader.timestamp));
    EXTERNAL_ntoh(&pduHeader.length, sizeof(pduHeader.length));
}

void
DisNtohEntityStatePdu(DisEntityStatePdu& pdu)
{
    DisNtohPduHeader(pdu.pduHeader);

    DisNtohEntityId(pdu.entityId);

    // Don't convert Entity Type, Alternate Entity Type for now.

    //EXTERNAL_ntoh(&pdu.entityType.countryCode, sizeof(pdu.entityType.countryCode));
    //EXTERNAL_ntoh(&pdu.alternateEntityType.countryCode,
    //        sizeof(pdu.alternateEntityType.countryCode));

    EXTERNAL_ntoh(&pdu.entityLinearVelocity[0], sizeof(pdu.entityLinearVelocity[0]));
    EXTERNAL_ntoh(&pdu.entityLinearVelocity[1], sizeof(pdu.entityLinearVelocity[1]));
    EXTERNAL_ntoh(&pdu.entityLinearVelocity[2], sizeof(pdu.entityLinearVelocity[2]));

    EXTERNAL_ntoh(&pdu.entityLocation[0], sizeof(pdu.entityLocation[0]));
    EXTERNAL_ntoh(&pdu.entityLocation[1], sizeof(pdu.entityLocation[1]));
    EXTERNAL_ntoh(&pdu.entityLocation[2], sizeof(pdu.entityLocation[2]));

    EXTERNAL_ntoh(&pdu.entityOrientation[0], sizeof(pdu.entityOrientation[0]));
    EXTERNAL_ntoh(&pdu.entityOrientation[1], sizeof(pdu.entityOrientation[1]));
    EXTERNAL_ntoh(&pdu.entityOrientation[2], sizeof(pdu.entityOrientation[2]));

    EXTERNAL_ntoh(&pdu.entityAppearance, sizeof(pdu.entityAppearance));

    // Don't convert Dead Reckoning Parameteres for now.
    // Don't convert Entity Capabilities for now.
}

void
DisNtohTransmitterPdu(DisTransmitterPdu& pdu)
{
    DisNtohPduHeader(pdu.pduHeader);

    DisNtohEntityId(pdu.entityId);
    EXTERNAL_ntoh(&pdu.radioId, sizeof(pdu.radioId));

    // Don't convert antenna fields for now.

    //EXTERNAL_ntoh(&pdu.antennaLocation[0], sizeof(pdu.antennaLocation[0]));
    //EXTERNAL_ntoh(&pdu.antennaLocation[1], sizeof(pdu.antennaLocation[1]));
    //EXTERNAL_ntoh(&pdu.antennaLocation[2], sizeof(pdu.antennaLocation[2]));

    //EXTERNAL_ntoh(&pdu.relativeAntennaLocation[0],
    //        sizeof(pdu.relativeAntennaLocation[0]));
    //EXTERNAL_ntoh(&pdu.relativeAntennaLocation[1],
    //        sizeof(pdu.relativeAntennaLocation[1]));
    //EXTERNAL_ntoh(&pdu.relativeAntennaLocation[2],
    //        sizeof(pdu.relativeAntennaLocation[2]));

    //EXTERNAL_ntoh(&pdu.antennaPatternType, sizeof(pdu.antennaPatternType));
    //EXTERNAL_ntoh(&pdu.antennaPatternLength, sizeof(pdu.antennaPatternLength));

    EXTERNAL_ntoh(&pdu.transmissionFrequency, sizeof(pdu.transmissionFrequency));

    // Don't convert additional fields for now.
}

void
DisHtonDataPdu(DisDataPdu& pdu)
{
    EXTERNAL_hton(&pdu.pduHeader.timestamp, sizeof(pdu.pduHeader.timestamp));
    EXTERNAL_hton(&pdu.pduHeader.length, sizeof(pdu.pduHeader.length));

    DisHtonEntityId(pdu.originatingEntityId);
    DisHtonEntityId(pdu.receivingEntityId);
    EXTERNAL_hton(&pdu.requestId, sizeof(pdu.requestId));

    EXTERNAL_hton(&pdu.numberOfVariableDatumRecords,
            sizeof(pdu.numberOfVariableDatumRecords));

    EXTERNAL_hton(&pdu.variableDatumId, sizeof(pdu.variableDatumId));
    EXTERNAL_hton(&pdu.variableDatumLength, sizeof(pdu.variableDatumLength));
}

void
DisNtohSignalPdu(DisSignalPdu& pdu)
{
    DisNtohPduHeader(pdu.pduHeader);

    DisNtohEntityId(pdu.entityId);
    EXTERNAL_ntoh(&pdu.radioId, sizeof(pdu.radioId));

    EXTERNAL_ntoh(&pdu.encodingScheme, sizeof(pdu.encodingScheme));
    EXTERNAL_ntoh(&pdu.tdlType, sizeof(pdu.tdlType));

    EXTERNAL_ntoh(&pdu.sampleRate, sizeof(pdu.sampleRate));

    EXTERNAL_ntoh(&pdu.dataLength, sizeof(pdu.dataLength));
    EXTERNAL_ntoh(&pdu.samples, sizeof(pdu.samples));

    if (DisGetEncodingClass(pdu)
        == DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA)
    {
        EXTERNAL_hton(pdu.data, sizeof(unsigned));
    }
}

unsigned char
DisGetEncodingClass(const DisSignalPdu& pdu)
{
    unsigned char unfilteredEncodingSchemeByte
        = ((unsigned char*) &pdu.encodingScheme)[1];

    return unfilteredEncodingSchemeByte & 192; // 1100 0000
}

unsigned
DisGetUserProtocolId(const DisSignalPdu& pdu)
{
    assert(DisGetEncodingClass(pdu)
           == DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA);

    return *((unsigned*) pdu.data);
}

unsigned
DisGetTimestamp()
{
    return DisConvertDoubleToTimestamp(DisGetNumSecondsPastHour(), 1);
}

double
DisConvertTimestampToDouble(unsigned timestamp)
{
    // Convert DIS timestamp to seconds.

    // It is NOT guaranteed that converting from one type to the other,
    // then back to the original type, will provide the original value.

    timestamp >>= 1;
    return ((double) timestamp) / g_disTimestampRatio;
}

unsigned
DisConvertDoubleToTimestamp(double dbl_timestamp, bool absolute)
{
    // Convert seconds to DIS timestamp.

    // It is NOT guaranteed that converting from one type to the other,
    // then back to the original type, will provide the original value.

    // DIS timestamps are flipped to 0 after one hour (3600 seconds).

    assert(dbl_timestamp < 3600.0);
    unsigned timestamp = (unsigned) (dbl_timestamp * g_disTimestampRatio);

    // Check high-order bit is still 0.

    assert((timestamp & 0x80000000) == 0);
    timestamp <<= 1;

    if (absolute) { return timestamp |= 1; }
    else { return timestamp; }
}

bool DisEntityId::less::operator()(const DisEntityId &id1, const DisEntityId &id2) const
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

