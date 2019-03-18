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

#ifndef HLA_RPR_FOM_H
#define HLA_RPR_FOM_H

// 12345.78901.34567 plus one byte for the null terminator.
// Each number is represented as an unsigned short, so can never be negative.

const unsigned g_hlaEntityIdStringBufSize = 18;

// The Marking attribute consists of the MarkingEncodingType field (1 byte),
// followed by the MarkingData field (11 bytes).
// MarkingData should include a null terminator, which leaves a maximum of
// 10 useful characters.

const unsigned g_hlaMarkingBufSize     = 12;
const unsigned g_hlaMarkingDataBufSize = 11;

// Arbitrary size for buffer containing object-instance name.

const unsigned g_hlaObjectNameBufSize = 64;

// SignalDataLength, as a two-byte unsigned integer, has a maximum value of
// 2^16 - 1 = 65535, in units of bits.  ceil(65535.0 / 8.0) is equal to the
// value below.

const unsigned g_hlaSignalDataBufSize = 8192;

// Must be large enough to satisfy ICD requirements for Data interactions
// sent by QualNet.
// The ICD does not specify a size limit on Comm Effects Requests received by
// QualNet, so the value can be increased if desired.
// This value must be a multiple of 8, to satisfy RPR FOM 1.0 padding
// requirements.

const unsigned g_hlaDatumValueBufSize = 1024;

// ICD draft 6 value.

const unsigned g_hlaMaxOutgoingDataIxnVariableDatumsSize = 1000;

// QualNet only sends one HlaVariableDatum per Data interaction, so the value
// below is calculated correctly.

const unsigned g_hlaMaxOutgoingDataIxnDatumValueSize
    = g_hlaMaxOutgoingDataIxnVariableDatumsSize
      - sizeof(unsigned)   // DatumId
      - sizeof(unsigned);  // DatumLength

// HLA Dynamic Statistics ICD draft 1 value.

const unsigned g_hlaMaxOutgoingCommentIxnVariableDatumsSize = 1000;

// QualNet only sends one HlaVariableDatum per Comment interaction, so the
// valuebelow is calculated correctly.

const unsigned g_hlaMaxOutgoingCommentIxnDatumValueSize
    = g_hlaMaxOutgoingCommentIxnVariableDatumsSize
      - sizeof(unsigned)   // DatumId
      - sizeof(unsigned);  // DatumLength

// QualNet only handles one VariableDatum in the VariableDatums field.

const unsigned g_hlaMaxVariableDatums = 1;

const unsigned g_hlaVariableDatumSetBufSize
    = sizeof(unsigned)        // NumberOfVariableDatums
      + ((sizeof(unsigned)    // DatumID
          + sizeof(unsigned)  // DatumLength
          + g_hlaDatumValueBufSize)
         * g_hlaMaxVariableDatums);

const unsigned HLA_DAMAGE_STATE_MIN             = 0;
const unsigned HLA_DAMAGE_STATE_NO_DAMAGE       = 0;
const unsigned HLA_DAMAGE_STATE_SLIGHT_DAMAGE   = 1;
const unsigned HLA_DAMAGE_STATE_MODERATE_DAMAGE = 2;
const unsigned HLA_DAMAGE_STATE_DESTROYED       = 3;
const unsigned HLA_DAMAGE_STATE_MAX             = 3;

const unsigned char HLA_TX_OPERATIONAL_STATUS_MIN                     = 0;
const unsigned char HLA_TX_OPERATIONAL_STATUS_OFF                     = 0;
const unsigned char HLA_TX_OPERATIONAL_STATUS_ON_BUT_NOT_TRANSMITTING = 1;
const unsigned char HLA_TX_OPERATIONAL_STATUS_ON_AND_TRANSMITTING     = 2;
const unsigned char HLA_TX_OPERATIONAL_STATUS_MAX                     = 2;

struct HlaEntityId
{
    class less
    {
      public:
        bool operator()(const HlaEntityId &id1, const HlaEntityId &id2) const;
    };
    unsigned short siteId;
    unsigned short applicationId;
    unsigned short entityNumber;
};

struct HlaVariableDatumInfo
{
    unsigned      datumId;
    unsigned      datumLength;
    unsigned char datumValue[g_hlaDatumValueBufSize];
};

struct HlaVariableDatumSetInfo
{
    unsigned numberOfVariableDatums;

    HlaVariableDatumInfo variableDatumsInfo[g_hlaMaxVariableDatums];
};

struct HlaCommentIxnInfo
{
    HlaVariableDatumSetInfo variableDatumSetInfo;

    // No storage for unused parameters:
    // OriginatingEntity
    // ReceivingEntity
};

struct HlaDataIxnInfo
{
    HlaEntityId originatingEntity;
    HlaVariableDatumSetInfo variableDatumSetInfo;

    // No storage for unused parameters:
    // ReceivingEntity
    // RequestIdentifier
    // FixedDatums
};

struct HlaApplicationSpecificRadioSignalIxnInfo
{
    char                hostRadioIndex[g_hlaObjectNameBufSize];
    unsigned            dataRate;
    unsigned short      signalDataLength;
    char                signalData[g_hlaSignalDataBufSize];
    unsigned            userProtocolId;

    // No storage for unused parameters:
    // TacticalDataLinkType
    // TDLMessageCount
};

bool
operator ==(const HlaEntityId& op1, const HlaEntityId& op2);

bool
operator !=(const HlaEntityId& op1, const HlaEntityId& op2);

void
HlaMakeEntityIdString(const HlaEntityId& entityId, char* entityIdString);

void
HlaNtohEntityId(HlaEntityId& entityId);

void
HlaHtonEntityId(HlaEntityId& entityId);

void
HlaCopyToOffsetAndHtonEntityId(
    void* dst, unsigned& offset, const void* src);

unsigned
HlaGetTimestamp();

double
HlaConvertTimestampToDouble(unsigned timestamp);

unsigned
HlaConvertDoubleToTimestamp(double double_timestamp, bool absolute);

#endif /* HLA_RPR_FOM_H */
