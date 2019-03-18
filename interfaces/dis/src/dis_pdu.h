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

#ifndef DIS_PDU_H
#define DIS_PDU_H

const unsigned g_maxUdpPayloadSize = 65527;

// 12345.78901.34567 plus one byte for the null terminator.
// Each number is represented as an unsigned short, so can never be negative.

const unsigned g_disEntityIdStringBufSize = 18;

// The Entity Marking record consists of one byte indicating the character set,
// followed by an 11-byte character string. We add a null terminator for
// printing and comparing which gives a total of 13 bytes.

const unsigned g_disMarkingBufSize       = 13;
const unsigned g_disMarkingStringBufSize = 12;

// The Signal PDU Data Length field, as a two-byte unsigned integer, has a
// maximum value of 2^16 - 1 = 65535, in units of bits.  ceil(65535.0 / 8.0)
// is equal to the value below.

const unsigned g_disSignalDataBufSize = 8192;

// Same value used for HLA.

const unsigned g_disVariableDatumValueBufSize = 1024;

// ICD draft 6 value.

const unsigned g_disMaxOutgoingDataPduVariableDatumsSize = 1000;

// DIS enumerations.

const unsigned char DIS_ENTITY_STATE_PDU = 1;
const unsigned char DIS_DATA_PDU         = 20;
const unsigned char DIS_TRANSMITTER_PDU  = 25;
const unsigned char DIS_SIGNAL_PDU       = 26;

const unsigned char DIS_PROTOCOL_FAMILY_RADIO_COMMUNICATION = 4;
const unsigned char DIS_PROTOCOL_FAMILY_SIMULATION_MANAGEMENT = 5;

const unsigned short DIS_ALL_APPLIC = 0xFFFF;
const unsigned short DIS_ALL_ENTITIES = 0xFFFF;
const unsigned short DIS_ALL_SITES = 0xFFFF;

const unsigned DIS_DAMAGE_MIN             = 0;
const unsigned DIS_DAMAGE_NO_DAMAGE       = 0;
const unsigned DIS_DAMAGE_SLIGHT_DAMAGE   = 1;
const unsigned DIS_DAMAGE_MODERATE_DAMAGE = 2;
const unsigned DIS_DAMAGE_DESTROYED       = 3;
const unsigned DIS_DAMAGE_MAX             = 3;

const unsigned char DIS_TX_STATE_MIN                     = 0;
const unsigned char DIS_TX_STATE_OFF                     = 0;
const unsigned char DIS_TX_STATE_ON_BUT_NOT_TRANSMITTING = 1;
const unsigned char DIS_TX_STATE_ON_AND_TRANSMITTING     = 2;
const unsigned char DIS_TX_STATE_MAX                     = 2;

const unsigned char DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA
                      = 128; // 1000 0000

const unsigned DIS_USER_PROTOCOL_ID_QUALNET_CER = 10000;

struct DisEntityId
{
    class less
    {
      public:
        bool operator()(const DisEntityId &id1, const DisEntityId &id2) const;
    };

    unsigned short siteId;
    unsigned short applicationId;
    unsigned short entityNumber;
};

struct DisEntityType
{
    unsigned char  entityKind;
    unsigned char  domain;
    unsigned short country;
    unsigned char  category;
    unsigned char  subcategory;
    unsigned char  specific;
    unsigned char  extra;
};

struct DisHeader
{
    unsigned char       protocolVersion;
    unsigned char       exerciseIdentifier;
    unsigned char       pduType;
    unsigned char       protocolFamily;

    unsigned            timestamp;

    unsigned short      length;
    unsigned short      padding;
};

struct DisEntityStatePdu
{
    DisHeader           pduHeader;

    DisEntityId         entityId;
    unsigned char       forceId;
    unsigned char       numberOfArticulationParameters;
    DisEntityType       entityType;
    DisEntityType       alternateEntityType;

    float               entityLinearVelocity[3];
    double              entityLocation[3];
    float               entityOrientation[3];

    unsigned            entityAppearance;
    unsigned char       deadReckoningParameters[40];
    unsigned char       entityMarking[12];
    unsigned            entityCapabilities;
};

struct DisTransmitterPdu
{
    // Fields below separated on 8-byte boundaries.

    DisHeader           pduHeader;
    DisEntityId         entityId;
    unsigned short      radioId;
    char                radioEntityType[8];
    unsigned char       transmitState;
    unsigned char       inputSource;
    unsigned short      padding;

    double              antennaLocation[3];

    float               relativeAntennaLocation[3];
    unsigned short      antennaPatternType;
    unsigned short      antennaPatternLength;

    uint64              transmissionFrequency;

    float               transmitFrequencyBandwidth;
    float               power;

    unsigned short      modulationType[4];
    unsigned short      cryptoSystem;
    unsigned short      cryptoKeyId;
    unsigned char       lengthOfModulationParameters;
    char                padding2[3];

    // Ignored:  Modulation Parameters and Antenna Pattern Parameters.

    //char                modulationParameters[?];
    //char                antennaPatternParameters[32];
};

struct DisDataPdu
{
    // This data structure stores a Data PDU with exactly one variable datum
    // and no fixed datums.

    // Fields below separated on 8-byte boundaries.

    DisHeader           pduHeader;
    DisEntityId         originatingEntityId;
    DisEntityId         receivingEntityId;

    unsigned            requestId;
    unsigned            padding;

    unsigned            numberOfFixedDatumRecords;
    unsigned            numberOfVariableDatumRecords;

    unsigned            variableDatumId;
    unsigned            variableDatumLength;

    unsigned char       variableDatumValue[g_disVariableDatumValueBufSize];
};

struct DisSignalPdu
{
    // Fields below separated on 4-byte boundaries.

    DisHeader           pduHeader;
    DisEntityId         entityId;
    unsigned short      radioId;

    unsigned short      encodingScheme;
    unsigned short      tdlType;

    unsigned            sampleRate;

    unsigned short      dataLength;
    unsigned short      samples;

    // Variable below is big enough to store maximum-size field.

    unsigned char       data[g_disSignalDataBufSize];
};

bool
operator ==(const DisEntityId& op1, const DisEntityId& op2);

bool
operator !=(const DisEntityId& op1, const DisEntityId& op2);

void
DisMakeEntityIdString(const DisEntityId& entityId, char* entityIdString);

void
DisNtohEntityId(DisEntityId& entityId);

void
DisHtonEntityId(DisEntityId& entityId);

void
DisCopyToOffsetAndHtonEntityId(
    void* dst, unsigned& offset, const void* src);

bool
DisPayloadContainsOnlyPdus(const char* payload, unsigned payloadSize);

unsigned char
DisGetExerciseIdentifier(const char* pduBuf);

unsigned char
DisGetPduType(const char* pduBuf);

unsigned short
DisGetPduLength(const char* pduBuf);

void
DisNtohPduHeader(DisHeader& pduHeader);

void
DisNtohEntityStatePdu(DisEntityStatePdu& pdu);

void
DisNtohTransmitterPdu(DisTransmitterPdu& pdu);

void
DisHtonDataPdu(DisDataPdu& pdu);

void
DisNtohSignalPdu(DisSignalPdu& pdu);

unsigned char
DisGetEncodingClass(const DisSignalPdu& pdu);

unsigned
DisGetUserProtocolId(const DisSignalPdu& pdu);

unsigned
DisGetTimestamp();

double
DisConvertTimestampToDouble(unsigned timestamp);

unsigned
DisConvertDoubleToTimestamp(double dbl_timestamp, bool absolute);

#endif /* DIS_PDU_H */
