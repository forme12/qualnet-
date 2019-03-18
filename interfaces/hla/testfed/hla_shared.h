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

#ifndef HLA_SHARED_H
#define HLA_SHARED_H

// 64-bit unsigned integer type.

#ifdef _WIN32
typedef unsigned __int64 uint64;
#define UINT64_MAX 0xffffffffffffffffui64

#define uint64toa(value, string) sprintf(string, "%I64u", value)
#define atouint64(string, value) sscanf(string, "%I64u", value)
#define UINT64_C(value)          value##ui64
#else /* _WIN32 */
typedef unsigned long long uint64;
#define UINT64_MAX 0xffffffffffffffffULL

#define uint64toa(value, string) sprintf(string, "%llu", value)
#define atouint64(string, value) sscanf(string, "%llu", value)
#define UINT64_C(value)          value##ULL
#endif /* _WIN32 */

const unsigned MAX_STRING_LENGTH = 512;

const unsigned g_hlaPathBufSize = 256;
const unsigned g_hlaClocktypeStringBufSize = 21;

const unsigned g_hlaMaxRadiosPerEntity = 64;

const unsigned g_hlaMaxMembersInNetwork = 254;
const unsigned g_hlaNetworkNameBufSize = 64;

// (2^31 - 1) / 3600, is used to compute or decode a DIS timestamp.
// The figure is rounded to a value just before further digits would be
// ignored.

const double g_hlaTimestampRatio = 596523.235277778;

struct HlaEntity;
struct HlaNetwork;

struct HlaRadio
{
    unsigned            handle;
    unsigned            nodeId;
    unsigned short      radioIndex;

    float               relativePositionX;
    float               relativePositionY;
    float               relativePositionZ;

    HlaRadioSystemType  radioSystemType;
    unsigned char       txOperationalStatus;

    HlaEntity*          entityPtr;
    const HlaNetwork*   networkPtr;

    char                objectName[g_hlaObjectNameBufSize];
};

struct HlaEntity
{
    unsigned            handle;

    HlaEntityId         entityId;
    unsigned char       forceId;

    double              lat;
    double              lon;
    double              alt;

    double              x;
    double              y;
    double              z;

    float               orientationPsi;
    float               orientationTheta;
    float               orientationPhi;

    short               azimuth;
    short               elevation;

    float               xVelocity;
    float               yVelocity;
    float               zVelocity;

    HlaEntityType       entityType;

    unsigned            damageState;

    unsigned            numRadioPtrs;
    const HlaRadio*     radioPtrs[g_hlaMaxRadiosPerEntity];

    char                entityIdString[g_hlaEntityIdStringBufSize];
    char                markingData[g_hlaMarkingDataBufSize];
};

struct HlaNetwork
{
    char                name[g_hlaNetworkNameBufSize];
    uint64              frequency;

    unsigned            numRadioPtrs;
    const HlaRadio*     radioPtrs[g_hlaMaxMembersInNetwork];

    unsigned            ipAddress;
    bool                unicast;
};

struct HlaRadioKey
{
    char                markingData[g_hlaMarkingDataBufSize];
    unsigned short      radioIndex;
};

double
HlaGetElapsedPhysicalTime();

unsigned
HlaGetTimestamp();

void
HlaMakeEntityIdString(const HlaEntityId& entityId, char* entityIdString);

double
HlaConvertTimestampToDouble(unsigned timestamp);

unsigned
HlaConvertDoubleToTimestamp(double double_timestamp, bool absolute);

void
HlaCopyFromOffset(
    void* dst, const void* src, unsigned& offset, unsigned size);

void
HlaCopyToOffset(
    void* dst, unsigned& offset, const void* src, unsigned size);

void
HlaCopyFromOffsetAndNtoh(
    void* dst, const void* src, unsigned& offset, unsigned size);

void
HlaCopyToOffsetAndHton(
    void* dst, unsigned& offset, const void* src, unsigned size);

void
HlaCopyToOffsetAndHtonEntityId(
    void* dst, unsigned& offset, const void* src);

void
HlaNtohEntityId(HlaEntityId& entityId);

void
HlaHtonEntityId(HlaEntityId& entityId);

void
HlaVerify(
    bool condition,
    char* errorString,
    const char* path = NULL,
    unsigned lineNumber = 0);

void
HlaCheckMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber);

void
HlaCheckNoMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber);

void
HlaReportWarning(
    char* warningString,
    const char* path = NULL,
    unsigned lineNumber = 0);

void
HlaReportError(
    char* errorString,
    const char* path = NULL,
    unsigned lineNumber = 0);

#endif /* HLA_SHARED_H */
