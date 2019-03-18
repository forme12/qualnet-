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

// iostream.h is needed to support DMSO RTI 1.3v6 (non-NG).

#ifdef NOT_RTI_NG
#include <iostream.h>
#else /* NOT_RTI_NG */
#include <iostream>
using namespace std;
#endif /* NOT_RTI_NG */

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <csignal>
#include <cassert>
#include <cerrno>

#include "RTI.hh"

#include "hla_HashTable.h"
#include "hla_gcclatlon.h"
#include "hla_archspec.h"
#include "hla_rpr_fom.h"
#include "hla_shared.h"
#include "hla_hashfcns.h"
#include "hla_main.h"
#include "hla_FedAmb.h"
#include "hla.h"

double
HlaGetElapsedPhysicalTime()
{
    return HlaFtime() - g_hlaFedAmb->m_double_referencePhysicalTime;
}

unsigned
HlaGetTimestamp()
{
    return HlaConvertDoubleToTimestamp(GetNumSecondsPastHour(), 1);
}

void
HlaMakeEntityIdString(const HlaEntityId& entityId, char* entityIdString)
{
    sprintf(entityIdString, "%u.%u.%u",
            entityId.siteId,
            entityId.applicationId,
            entityId.entityNumber);
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

void
HlaCopyFromOffset(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    offset += size;
}

void
HlaCopyToOffset(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    offset += size;
}

void
HlaCopyFromOffsetAndNtoh(
    void* dst, const void* src, unsigned& offset, unsigned size)
{
    const unsigned char* uchar_src = (const unsigned char*) src;

    memcpy(dst, &uchar_src[offset], size);
    if (size > 1) { HlaNtoh(dst, size); }
    offset += size;
}

void
HlaCopyToOffsetAndHton(
    void* dst, unsigned& offset, const void* src, unsigned size)
{
    unsigned char* uchar_dst = (unsigned char*) dst;

    memcpy(&uchar_dst[offset], src, size);
    if (size > 1) { HlaHton(&uchar_dst[offset], size); }
    offset += size;
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

void
HlaNtohEntityId(HlaEntityId& entityId)
{
    HlaNtoh(&entityId.siteId, sizeof(entityId.siteId));
    HlaNtoh(&entityId.applicationId, sizeof(entityId.applicationId));
    HlaNtoh(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
HlaHtonEntityId(HlaEntityId& entityId)
{
    HlaHton(&entityId.siteId, sizeof(entityId.siteId));
    HlaHton(&entityId.applicationId, sizeof(entityId.applicationId));
    HlaHton(&entityId.entityNumber, sizeof(entityId.entityNumber));
}

void
HlaVerify(
    bool condition,
    char* errorString,
    const char* path,
    unsigned lineNumber)
{
    if (!condition)
    {
        HlaReportError(errorString, path, lineNumber);
    }
}

void
HlaCheckMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr == NULL)
    {
        HlaReportError("Out of memory", path, lineNumber);
    }
}

void
HlaCheckNoMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber)
{
    if (ptr != NULL)
    {
        HlaReportError(
            "Attempting to allocate memory for non-null pointer",
            path,
            lineNumber);
    }
}

void
HlaReportWarning(
    char* warningString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "HLA warning:";

    if (path != NULL)
    {
        cerr << path << ":";

        if (lineNumber > 0)
        {
            cerr << lineNumber << ":";
        }
    }

    cerr << " " << warningString << endl;
}
void
HlaReportError(
    char* errorString,
    const char* path,
    unsigned lineNumber)
{
    cerr << "HLA error:";

    if (path != NULL)
    {
        cerr << path << ":";

        if (lineNumber > 0)
        {
            cerr << lineNumber << ":";
        }
    }

    cerr << " " << errorString << endl;

    exit(EXIT_FAILURE);
}
