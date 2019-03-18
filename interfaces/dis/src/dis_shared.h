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

#ifndef DIS_SHARED_H
#define DIS_SHARED_H

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

#include <limits.h>
#include "api.h"
#include "partition.h"
#include "external_util.h"
#include "app_messenger.h"

#include "dis_sockets.h"

#include "dis.h"
#include "dis_archspec.h"
#include "dis_gcclatlon.h"
#include "dis_orientation.h"

#include "dis_pdu.h"

#include <map>

const unsigned g_disPathBufSize            = 256;
const unsigned g_disLineBufSize            = 16384;
const unsigned g_disClocktypeStringBufSize = 21;

const unsigned g_disMaxRadiosPerEntity     = 64;

const unsigned g_disMaxMembersInNetwork    = 254;
const unsigned g_disNetworkNameBufSize     = 64;

// For the maximum msgId, INT_MAX - 1 is used instead of UINT_MAX - 1
// because the msgId values are passed into the Messenger application,
// and Messenger uses signed int for msgId.

const unsigned g_disMaxMsgId = INT_MAX - 1;

const int MSG_EXTERNAL_DIS_SimulatedMsgTimeout = 1902;
const int MSG_EXTERNAL_DIS_SendRtss            = 1903;

struct DisEntity;
struct DisNetwork;

struct DisRadio
{
    Node*               node;

    DisEntityId         entityId;
    unsigned short      radioId;

    float               relativeAntennaLocationX;
    float               relativeAntennaLocationY;
    float               relativeAntennaLocationZ;

    double              latScheduled;
    double              lonScheduled;
    double              altScheduled;

    unsigned char       txState;

    bool                usesTxPower;
    double              maxTxPower;
    double              currentMaxTxPower;

    bool                mappedToEntityMarkingString;
    bool                mappedToHandle;
    const DisEntity*    entityPtr;
    const DisNetwork*   networkPtr;
    const DisRadio*     defaultDstRadioPtr;
};

struct DisRadioKey
{
    class less
    {
      public:
        bool operator()(const DisRadioKey &key1, const DisRadioKey &key2) const;
    };

    char                markingString[g_disMarkingStringBufSize];
    unsigned short      radioId;
};

struct DisEntity
{
    DisEntityId         entityId;

    double              lat;
    double              lon;
    double              alt;
    double              latScheduled;
    double              lonScheduled;
    double              altScheduled;

    double              x;
    double              y;
    double              z;
    double              xScheduled;
    double              yScheduled;
    double              zScheduled;

    short               azimuth;
    short               elevation;
    short               azimuthScheduled;
    short               elevationScheduled;

    double              speed;
    double              speedScheduled;

    clocktype           lastScheduledMobilityEventTime;

    unsigned            damage;

    bool                hierarchyIdExists;
    int                 hierarchyId;

    unsigned            numRadioPtrs;
    DisRadio*           radioPtrs[g_disMaxRadiosPerEntity];

    bool                mappedToHandle;
    char                entityIdString[g_disEntityIdStringBufSize];
    char                markingString[g_disMarkingStringBufSize];
};

struct DisEntityObject
{
    bool                entityIdExists;
    DisEntityId         entityId;

    bool                markingStringExists;
    char                markingString[g_disMarkingStringBufSize];

    DisEntity*          entityPtr;
};

struct DisNetwork
{
    char                name[g_disNetworkNameBufSize];
    uint64              frequency;

    unsigned            numRadioPtrs;
    const DisRadio*     radioPtrs[g_disMaxMembersInNetwork];

    unsigned            ipAddress;
    bool                unicast;
};

struct DisSimulatedMsgInfo
{
    const DisRadio*     srcRadioPtr;
    unsigned            msgId;
};

struct DisSimulatedMsgDstEntityInfo
{
    const DisEntity*    dstEntityPtr;
    bool                processed;
    bool                success;
};

struct DisOutstandingSimulatedMsgInfo
{
    unsigned    timestamp;
    clocktype   sendTime;

    unsigned    numDstEntities;
    unsigned    numDstEntitiesProcessed;

    DisSimulatedMsgDstEntityInfo
        smDstEntitiesInfo[g_disMaxMembersInNetwork];
};

typedef std::map<unsigned, DisOutstandingSimulatedMsgInfo> DisOutstandingSimulatedMsgInfoMap;

struct DisData
{
    const DisRadio*     radioPtr;

    unsigned            nextMsgId;
    DisOutstandingSimulatedMsgInfoMap      outstandingSimulatedMsgInfo;
};

typedef std::map<std::string, DisEntity*> DisMarkingStringToEntityMap;
typedef std::map<DisRadioKey, DisRadio*, DisRadioKey::less> DisRadioKeyToRadioMap;
typedef std::map<DisEntityId, DisEntity*, DisEntityId::less> DisEntityIdToEntityMap;
typedef std::map<unsigned, int> DisNodeIdToHierarchyIdMap;
typedef std::map<unsigned, DisData> DisNodeIdToPerNodeDataMap;

struct Dis
{
    EXTERNAL_Interface* iface;
    PartitionData*      partitionData;

    double              altitudeOffset;

    bool                debugPrintComms;
    bool                debugPrintComms2;
    bool                debugPrintMapping;
    bool                debugPrintDamage;
    bool                debugPrintTxState;
    bool                debugPrintTxPower;
    bool                debugPrintMobility;
    bool                debugPrintTransmitterPdu;
    bool                debugPrintPdus;

    char                entitiesPath[g_disPathBufSize];
    char                radiosPath[g_disPathBufSize];
    char                networksPath[g_disPathBufSize];

    bool                disIpAddressIsMulticast;
    unsigned            disIpAddress;
    unsigned short      disPort;
    SOCKET              sd;

    bool                filterOnExerciseIdentifier;
    unsigned char       exerciseIdentifier;

    clocktype           receiveDelay;
    clocktype           maxReceiveDuration;
    clocktype           mobilityInterval;

    double              xyzEpsilon;

    bool                newEventScheduled;

    unsigned            numEntities;
    DisEntity*          entities;

    unsigned            numRadios;
    DisRadio*           radios;

    unsigned            numNetworks;
    DisNetwork*         networks;

    DisMarkingStringToEntityMap  markingStringToEntity;
    DisRadioKeyToRadioMap        radioKeyToRadio;
    DisEntityIdToEntityMap       entityIdToEntity;
    DisNodeIdToHierarchyIdMap    nodeIdToHierarchyId;
    DisNodeIdToPerNodeDataMap    nodeIdToPerNodeData;

    double              minLat;
    double              maxLat;
    double              minLon;
    double              maxLon;

    RandomSeed          seed;
    double              slightDamageReduceTxPowerProbability;
    double              moderateDamageReduceTxPowerProbability;
    double              destroyedReduceTxPowerProbability;
    double              slightDamageReduceTxPowerMaxFraction;
    double              moderateDamageReduceTxPowerMaxFraction;
    double              destroyedReduceTxPowerMaxFraction;
};

struct DisChangeMaxTxPowerInfo
{
    const DisEntity*    entityPtr;
    unsigned            damage;
};

struct DisHierarchyMobilityInfo
{
    int                 hierarchyId;
    Coordinates         coordinates;
    Orientation         orientation;
};

#include "dis_main.h"

void
DisPrintHex(const char* buf, unsigned length);

bool
DisConvertStringToIpAddress(const char* s, unsigned& ipAddress);

bool
DisIsMulticastIpAddress(unsigned ipAddress);

clocktype
DisConvertDoubleToClocktype(double doubleValue);

double
DisConvertClocktypeToDouble(clocktype clocktypeValue);

void
DisCopyFromOffset(
    void* dst, const void* src, unsigned& offset, unsigned size);

void
DisCopyToOffset(
    void* dst, unsigned& offset, const void* src, unsigned size);

void
DisCopyFromOffsetAndNtoh(
    void* dst, const void* src, unsigned& offset, unsigned size);

void
DisCopyToOffsetAndHton(
    void* dst, unsigned& offset, const void* src, unsigned size);

char*
DisGetAnyToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesToCopy,
    const char*& src);

char*
DisGetAnyToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters);

const char*
DisGetAnyToken(
    unsigned& numBytesToCopy,
    const char*& src);

const char*
DisGetAnyToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters);

char*
DisGetToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    const char*& src,
    const char* delimiters);

const char*
DisGetToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters);

void
DisCopyAndNullTerminate(char* dst, const char* src, unsigned numBytesToCopy);

void
DisTrimLeft(char *s);

void
DisTrimRight(char *s);

unsigned
DisGetNumLinesInFile(const char* path);

void
DisVerify(
    bool condition,
    const char* errorString,
    const char* path = NULL,
    unsigned lineNumber = 0);

void
DisCheckMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber);

void
DisCheckNoMalloc(
    const void* ptr,
    const char* path,
    unsigned lineNumber);

void
DisReportWarning(
    const char* warningString,
    const char* path = NULL,
    unsigned lineNumber = 0);

void
DisReportError(
    const char* errorString,
    const char* path = NULL,
    unsigned lineNumber = 0);

#endif /* DIS_SHARED_H */
