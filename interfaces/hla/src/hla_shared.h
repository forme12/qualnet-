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

#include <vector>
#include <map>

#include "api.h"
#include "partition.h"
#include "app_messenger.h"

#include "RTI.hh"

#include "hla.h"
#include "hla_archspec.h"
#include "hla_gcclatlon.h"
#include "hla_orientation.h"

#include "hla_rpr_fom.h"

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

const unsigned g_hlaLineBufSize            = 32768;
const unsigned g_hlaClocktypeStringBufSize = 21;

const unsigned g_hlaMetricUpdateBufSize
    = g_hlaMaxOutgoingCommentIxnDatumValueSize;

const unsigned g_hlaMaxRadiosPerEntity     = 64;

const unsigned g_hlaNetworkNameBufSize     = 64;

struct HlaEntity;
struct HlaNetwork;
struct Hla;
class HlaFedAmb;

struct Node;

struct HlaRadio
{
    Node*               node;

    HlaEntityId         entityId;
    unsigned short      radioIndex;

    float               relativePositionX;
    float               relativePositionY;
    float               relativePositionZ;

    double              latScheduled;
    double              lonScheduled;
    double              altScheduled;

    unsigned char       txOperationalStatus;

    bool                usesTxPower;
    double              maxTxPower;
    double              currentMaxTxPower;

    bool                mappedToHandle;
    const HlaEntity*    entityPtr;
    const HlaNetwork*   networkPtr;
    const HlaRadio*     defaultDstRadioPtr;
};

struct HlaEntity
{
    HlaEntityId         entityId;

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

    float               orientationPsi;
    float               orientationTheta;
    float               orientationPhi;

    short               azimuth;
    short               elevation;
    short               azimuthScheduled;
    short               elevationScheduled;

    double              speed;
    double              speedScheduled;

    clocktype           lastScheduledMobilityEventTime;

    unsigned            damageState;

    bool                hierarchyIdExists;
    int                 hierarchyId;

    unsigned            numRadioPtrs;
    HlaRadio*           radioPtrs[g_hlaMaxRadiosPerEntity];

    bool                mappedToHandle;
    char                entityIdString[g_hlaEntityIdStringBufSize];
    char                markingData[g_hlaMarkingDataBufSize];
};

struct HlaNetwork
{
    char                name[g_hlaNetworkNameBufSize];
    uint64              frequency;

    unsigned            numRadioPtrs;
    std::vector< const HlaRadio*> radios;

    unsigned            ipAddress;
    bool                unicast;
};

struct HlaEntityObject
{
    bool                entityIdExists;
    HlaEntityId         entityId;

    bool                markingDataExists;
    char                markingData[g_hlaMarkingDataBufSize];

    HlaEntity*          entityPtr;
};

struct HlaRadioKey
{
    class less
    {
      public:
        bool operator()(const HlaRadioKey &key1, const HlaRadioKey &key2) const;
    };
    char                markingData[g_hlaMarkingDataBufSize];
    unsigned short      radioIndex;
};

struct HlaRadioObject
{
    bool                entityIdExists;
    bool                radioIndexExists;

    HlaEntityId         entityId;
    unsigned short      radioIndex;

    HlaRadio*           radioPtr;

    char                objectName[g_hlaObjectNameBufSize];
};

struct HlaSimulatedMsgInfo
{
    const HlaRadio*     srcRadioPtr;
    unsigned            msgId;
};

struct HlaSimulatedMsgDstEntityInfo
{
    const HlaEntity*    dstEntityPtr;
    bool                processed;
    bool                success;
};

struct HlaOutstandingSimulatedMsgInfo
{
    unsigned    timestamp;
    clocktype   sendTime;

    unsigned    numDstEntitiesProcessed;

    std::vector<HlaSimulatedMsgDstEntityInfo>
        smDstEntitiesInfos;
};

typedef std::map<unsigned, HlaOutstandingSimulatedMsgInfo> HlaOutstandingSimulatedMsgInfoMap;

struct HlaData
{
    const HlaRadio*     radioPtr;

    unsigned            nextMsgId;
    HlaOutstandingSimulatedMsgInfoMap    outstandingSimulatedMsgInfo;
};

typedef std::map<std::string, HlaEntity*> HlaMarkingDataToEntityMap;
typedef std::map<HlaRadioKey, HlaRadio*, HlaRadioKey::less> HlaRadioKeyToRadioMap;
typedef std::map<unsigned, int> HlaNodeIdToHierarchyIdMap;
typedef std::map<std::string, int> HlaObjectNameToHandleMap;
typedef std::map<HlaEntityId, HlaEntity*, HlaEntityId::less> HlaEntityIdToEntityMap;
typedef std::map<unsigned, HlaData> HlaNodeIdToPerNodeDataMap;
typedef std::map<RTI::ObjectHandle, RTI::ObjectClassHandle>
    HlaHandleToObjectClassMap;

typedef std::map<RTI::ObjectHandle, HlaEntityObject> HlaHandleToEntityObjectMap;
typedef std::map<RTI::ObjectHandle, HlaRadioObject> HlaHandleToRadioObjectMap;

struct Hla
{
    EXTERNAL_Interface* iface;
    PartitionData*      partitionData;

    bool                debug;
    bool                debug2;
#ifdef MILITARY_RADIOS_LIB
    bool                debugPrintRtss;
#endif /* MILITARY_RADIOS_LIB */

    double              rprFomVersion;
    bool                nawcGatewayCompatibility;
    clocktype           tickInterval;
    clocktype           mobilityInterval;

    bool                hlaDynamicStats;
    bool                verboseMetricUpdates;
    bool                sendNodeIdDescriptions;
    bool                sendMetricDefinitions;
    char                metricUpdateBuf[g_hlaMetricUpdateBufSize];
    unsigned            metricUpdateSize;
    clocktype           checkMetricUpdateInterval;

    double              dbl_referencePhysicalTime;
    bool                newEventScheduled;
    bool                endProgram;

    unsigned            maxMsgId;

    clocktype           attributeUpdateRequestDelay;
    clocktype           attributeUpdateRequestInterval;
    unsigned            maxAttributeUpdateRequests;
    unsigned            numAttributeUpdateRequests;

    unsigned            numEntities;
    HlaEntity*          entities;

    unsigned            numRadios;
    HlaRadio*           radios;

    unsigned            numNetworks;
    HlaNetwork*         networks;

    double              minLat;
    double              maxLat;
    double              minLon;
    double              maxLon;

    double              xyzEpsilon;

    RandomSeed          seed;
    double              slightDamageReduceTxPowerProbability;
    double              moderateDamageReduceTxPowerProbability;
    double              destroyedReduceTxPowerProbability;
    double              slightDamageReduceTxPowerMaxFraction;
    double              moderateDamageReduceTxPowerMaxFraction;
    double              destroyedReduceTxPowerMaxFraction;

    HlaMarkingDataToEntityMap     markingDataToEntity;
    HlaRadioKeyToRadioMap         radioKeyToRadio;
    HlaNodeIdToHierarchyIdMap     nodeIdToHierarchyId;
    HlaObjectNameToHandleMap      objectNameToHandle;
    HlaEntityIdToEntityMap        entityIdToEntity;
    HlaNodeIdToPerNodeDataMap     nodeIdToPerNodeData;
    HlaHandleToObjectClassMap     handleToObjectClass;
    HlaHandleToEntityObjectMap    handleToEntityObject;
    HlaHandleToRadioObjectMap     handleToRadioObject;
};

struct HlaChangeMaxTxPowerInfo
{
    const HlaEntity*    entityPtr;
    unsigned            damageState;
};

struct HlaHierarchyMobilityInfo
{
    unsigned            hierarchyId;
    Coordinates         coordinates;
    Orientation         orientation;
};

struct RtssForwardedInfo
{
    NodeAddress         nodeId;
};

struct EXTERNAL_HlaStartMessenegerForwardedInfo
{
    HlaSimulatedMsgInfo smInfo;
    double requestedDataRate;
    NodeAddress srcNodeId;
    unsigned dataMsgSize;
    clocktype voiceMsgDuration;
    clocktype timeoutDelay;
    clocktype sendDelay;

    NodeAddress srcNetworkAddress;
    NodeAddress destNodeId;
    bool unicast;
    bool isVoice;
};

void
StartSendSimulatedMsgUsingMessenger(
    Node* srcNode,
    NodeAddress srcNetworkAddress,
    NodeAddress destNodeId,
    const HlaSimulatedMsgInfo& smInfo,
    double requestedDataRate,
    unsigned dataMsgSize,
    clocktype voiceMsgDuration,
    bool isVoice,
    clocktype timeoutDelay,
    bool unicast,
    clocktype sendDelay);

struct EXTERNAL_HlaCompletedMessenegerForwardedInfo
{
    HlaSimulatedMsgInfo smInfo;
    NodeAddress     destNodeId;
    BOOL            success;
};

void
HlaAppMessengerResultCompleted(
    Node* destNode,
    const HlaSimulatedMsgInfo& smInfo,
    BOOL success);

struct EXTERNAL_Interface;

class HlaInterfaceData
{
public:
    HlaInterfaceData();

    Hla*                m_hla;
    HlaFedAmb*          m_hlaFedAmb;
    RTI::RTIambassador* m_hlaRtiAmb;
    EXTERNAL_Interface* m_interface;
};

HlaInterfaceData*
HlaGetIfaceDataFromNode(Node* node);

HlaInterfaceData*
HlaGetIfaceDataFromNodeId(NodeId nodeId);

clocktype
HlaConvertDoubleToClocktype(double doubleValue);

double
HlaConvertClocktypeToDouble(clocktype clocktypeValue);

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

char*
HlaGetTokenHelper(
    bool skipEmptyFields,
    bool& foundEmptyField,
    bool& overflowed,
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesToCopyOrCopied,
    const char*& src,
    const char* delimiters);

char*
HlaGetTokenOrEmptyField(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    bool& overflowed,
    const char*& src,
    const char* delimiters,
    bool& foundEmptyField);

char*
HlaGetToken(
    char* token,
    unsigned tokenBufSize,
    unsigned& numBytesCopied,
    bool& overflowed,
    const char*& src,
    const char* delimiters);

const char*
HlaLocateTokenOrEmptyField(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters,
    bool& foundEmptyField);

const char*
HlaLocateToken(
    unsigned& numBytesToCopy,
    const char*& src,
    const char* delimiters);

void
HlaTrimLeft(char *s);

void
HlaTrimRight(char *s);

unsigned
HlaGetNumLinesInFile(const char* path);

void
HlaVerify(
    bool condition,
    const char* errorString,
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
    const char* warningString,
    const char* path = NULL,
    unsigned lineNumber = 0);

void
HlaReportError(
    const char* errorString,
    const char* path = NULL,
    unsigned lineNumber = 0);

#endif /* HLA_SHARED_H */
