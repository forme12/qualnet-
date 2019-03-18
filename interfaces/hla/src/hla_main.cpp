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
#include <climits>
#include <csignal>
#include <cassert>
#include <cerrno>

#include "hla_shared.h"
#include "hla_FedAmb.h"
#include "hla_main.h"
#include "WallClock.h"
#include "external_util.h"

// This global is two ways:
// (1) To support the signal() call on Ctrl-C (SIGINT), because the
//     signal() handler only takes a single integer argument.
// (2) To support HlaGetIfaceDataFromNodeId().

HlaInterfaceData* g_ifaceData = 0;

void
HlaValidateConstants()
{
    assert(sizeof(bool) == 1);
    assert(sizeof(char) == 1);
    assert(sizeof(short) == 2);
    assert(sizeof(int) == 4);
    assert(sizeof(double) == 8);

    assert(g_hlaDatumValueBufSize % 8 == 0);

    assert(g_hlaMaxVariableDatums == 1);

    assert(g_hlaMaxOutgoingDataIxnDatumValueSize <= g_hlaDatumValueBufSize);

    assert(g_hlaMaxRadiosPerEntity > 0);
}

HlaInterfaceData *
HlaMallocHla(PartitionData* partitionData)
{
    HlaInterfaceData * newIfaceData = new HlaInterfaceData ();
    HlaCheckMalloc(newIfaceData, __FILE__, __LINE__);

    Hla * newHla = new Hla;
    HlaCheckMalloc(newHla, __FILE__, __LINE__);
    newIfaceData->m_hla = newHla;

    if (partitionData->partitionId == 0)
    {
        assert(g_ifaceData == NULL);
        g_ifaceData = newIfaceData;
    }

    return newIfaceData;
}

void
HlaMallocFederateAmbassador(HlaInterfaceData * ifaceData)
{
    HlaCheckNoMalloc(ifaceData->m_hlaFedAmb, __FILE__, __LINE__);
    ifaceData->m_hlaFedAmb = new HlaFedAmb (ifaceData);
    HlaCheckMalloc(ifaceData->m_hlaFedAmb, __FILE__, __LINE__);
}

void
HlaMallocRtiAmbassador(HlaInterfaceData* ifaceData)
{
        HlaCheckNoMalloc(ifaceData->m_hlaRtiAmb, __FILE__, __LINE__);
    ifaceData->m_hlaRtiAmb = new RTI::RTIambassador;
    HlaCheckMalloc(ifaceData->m_hlaRtiAmb, __FILE__, __LINE__);
}

void
HlaMallocRadios(HlaInterfaceData* ifaceData)
{
    HlaCheckNoMalloc(ifaceData->m_hla->radios, __FILE__, __LINE__);
    ifaceData->m_hla->radios = new HlaRadio [ifaceData->m_hla->numRadios];
    HlaCheckMalloc(ifaceData->m_hla->radios, __FILE__, __LINE__);
}

void
HlaMallocEntities(HlaInterfaceData * ifaceData)
{
    HlaCheckNoMalloc(ifaceData->m_hla->entities, __FILE__, __LINE__);
    ifaceData->m_hla->entities = new HlaEntity [ifaceData->m_hla->numEntities];
    HlaCheckMalloc(ifaceData->m_hla->entities, __FILE__, __LINE__);
}

void
HlaMallocNetworks(HlaInterfaceData * ifaceData)
{
    HlaCheckNoMalloc(ifaceData->m_hla->networks, __FILE__, __LINE__);
    ifaceData->m_hla->networks = new HlaNetwork [ifaceData->m_hla->numNetworks];
    HlaCheckMalloc(ifaceData->m_hla->networks, __FILE__, __LINE__);
}

void
HlaFreeHla(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hla)
    {
        delete ifaceData->m_hla;
        ifaceData->m_hla = NULL;
    }
}

void
HlaFreeFederateAmbassador(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hlaFedAmb)
    {
        delete ifaceData->m_hlaFedAmb;
        ifaceData->m_hlaFedAmb = NULL;
    }
}

void
HlaFreeRtiAmbassador(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hlaRtiAmb)
    {
        delete ifaceData->m_hlaRtiAmb;
        ifaceData->m_hlaRtiAmb = NULL;
    }
}

void
HlaFreeRadios(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hla->radios)
    {
        delete [] ifaceData->m_hla->radios;
        ifaceData->m_hla->radios = NULL;
    }
}

void
HlaFreeEntities(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hla->entities)
    {
        delete [] ifaceData->m_hla->entities;
        ifaceData->m_hla->entities = NULL;
    }
}

void
HlaFreeNetworks(HlaInterfaceData * ifaceData)
{
    if (ifaceData->m_hla->networks)
    {
        delete [] ifaceData->m_hla->networks;
        ifaceData->m_hla->networks = NULL;
    }
}

void
HlaInitHlaVariable(EXTERNAL_Interface* iface, HlaInterfaceData * ifaceData)
{
    ifaceData->m_hla->iface = iface;
    ifaceData->m_hla->partitionData = iface->partition;

    ifaceData->m_hla->metricUpdateSize = 1;
    ifaceData->m_hla->checkMetricUpdateInterval = 1 * SECOND;
    ifaceData->m_hla->metricUpdateBuf[0] = 0;

    ifaceData->m_hla->dbl_referencePhysicalTime = 0.0;
    ifaceData->m_hla->newEventScheduled = false;
    ifaceData->m_hla->endProgram = false;

    // For the maximum msgId, INT_MAX - 1 is used instead of UINT_MAX - 1
    // because the msgId values are passed into the Messenger application,
    // and Messenger uses the (signed) int type for msgId.

    ifaceData->m_hla->maxMsgId = INT_MAX - 1;

    ifaceData->m_hla->numAttributeUpdateRequests = 0;

    ifaceData->m_hla->numEntities = 0;
    ifaceData->m_hla->entities    = NULL;

    ifaceData->m_hla->numRadios   = 0;
    ifaceData->m_hla->radios      = NULL;

    ifaceData->m_hla->numNetworks = 0;
    ifaceData->m_hla->networks    = NULL;

    RANDOM_SetSeed(ifaceData->m_hla->seed, iface->partition->seedVal, 0, 0);
    ifaceData->m_hla->slightDamageReduceTxPowerProbability = 0.25;
    ifaceData->m_hla->moderateDamageReduceTxPowerProbability = 0.25;
    ifaceData->m_hla->destroyedReduceTxPowerProbability = 0.50;
    ifaceData->m_hla->slightDamageReduceTxPowerMaxFraction = 0.75;
    ifaceData->m_hla->moderateDamageReduceTxPowerMaxFraction = 0.75;
    ifaceData->m_hla->destroyedReduceTxPowerMaxFraction = 0.75;
}

void
HlaReadParameters(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput)
{
    BOOL retVal;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-DEBUG",
        &retVal,
        buf);

    cout << endl
         << "HLA debugging output is ";

    if (retVal && strcmp(buf, "YES") == 0)
    {
                ifaceData->m_hla->debug = true;
        cout << "on." << endl;
    }
    else
    {
        ifaceData->m_hla->debug = false;
        cout << "off." << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-DEBUG-2",
        &retVal,
        buf);

    cout << "HLA debugging output level 2 is ";

    if (retVal && strcmp(buf, "YES") == 0)
    {
        ifaceData->m_hla->debug2 = true;
        cout << "on." << endl;
    }
    else
    {
        ifaceData->m_hla->debug2 = false;
        cout << "off." << endl;
    }

#ifdef MILITARY_RADIOS_LIB
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-DEBUG-PRINT-RTSS",
        &retVal,
        buf);

    cout << "HLA debugging output (Ready To Send Signal interaction) is ";

    if (retVal && strcmp(buf, "NO") == 0)
    {
        ifaceData->m_hla->debugPrintRtss = false;
        cout << "off." << endl;
    }
    else
    {
        ifaceData->m_hla->debugPrintRtss = true;
        cout << "on." << endl;
    }

    ifaceData->m_hla->debugPrintRtss = true;
#endif /* MILITARY_RADIOS_LIB */

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-FEDERATION-NAME",
        &retVal,
        buf);

    HlaVerify(retVal == TRUE,
             "Can't find HLA-FEDERATION-NAME parameter",
             ".config file");

    strcpy(ifaceData->m_hlaFedAmb->m_federationName, buf);

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-FED-FILE-PATH",
        &retVal,
        buf);

    HlaVerify(retVal == TRUE,
             "Can't find HLA-FED-FILE-PATH parameter",
             ".config file");

    strcpy(ifaceData->m_hlaFedAmb->m_fedFilePath, buf);

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-FEDERATE-NAME",
        &retVal,
        buf);

    HlaVerify(retVal == TRUE,
             "Can't find HLA-FEDERATE-NAME parameter",
             ".config file");

    strcpy(ifaceData->m_hlaFedAmb->m_federateName, buf);

    IO_ReadDouble(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-RPR-FOM-VERSION",
        &retVal,
        &ifaceData->m_hla->rprFomVersion);

    if (retVal == FALSE)
    {
        ifaceData->m_hla->rprFomVersion = 1.0;
    }

    HlaVerify(
        ifaceData->m_hla->rprFomVersion == 0.5
        || ifaceData->m_hla->rprFomVersion == 1.0,
        "Invalid RPR-FOM version");

    cout.precision(1);

    cout << "Federation Object Model is RPR FOM "
            << ifaceData->m_hla->rprFomVersion << "." << endl;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-NAWC-GATEWAY-COMPATIBILITY",
        &retVal,
        buf);

    cout << "NAWC Gateway compatibility is ";

    if (retVal && strcmp(buf, "YES") == 0)
    {
        cout << "on." << endl;

        ifaceData->m_hla->nawcGatewayCompatibility       = true;

        ifaceData->m_hla->attributeUpdateRequestDelay    = 10 * SECOND;
        ifaceData->m_hla->attributeUpdateRequestInterval = 10 * SECOND;
        ifaceData->m_hla->maxAttributeUpdateRequests     = 3;
    }
    else
    {
        cout << "off." << endl;

        ifaceData->m_hla->nawcGatewayCompatibility       = false;

        ifaceData->m_hla->attributeUpdateRequestDelay    = 2 * SECOND;
        ifaceData->m_hla->attributeUpdateRequestInterval = 2 * SECOND;
        ifaceData->m_hla->maxAttributeUpdateRequests     = 3;
    }

    IO_ReadDouble(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-XYZ-EPSILON",
        &retVal,
        &ifaceData->m_hla->xyzEpsilon);

    if (retVal == FALSE)
    {
        ifaceData->m_hla->xyzEpsilon = 0.5;
    }

    HlaVerify(
        ifaceData->m_hla->xyzEpsilon >= 0.0,
        "Can't use negative epsilon value");

    cout << "GCC (x,y,z) epsilons are ("
            << ifaceData->m_hla->xyzEpsilon << ","
            << ifaceData->m_hla->xyzEpsilon << ","
            << ifaceData->m_hla->xyzEpsilon << " meter(s))" << endl
         << "  For movement to be reflected in QualNet, change in position"
            " must be" << endl
         << "  >= any one of these values." << endl;

    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-TICK-INTERVAL",
        &retVal,
        &ifaceData->m_hla->tickInterval);

    if (retVal == FALSE)
    {
        ifaceData->m_hla->tickInterval = 200 * MILLI_SECOND;
    }

    HlaVerify(
        ifaceData->m_hla->tickInterval >= 0,
        "Can't use negative time");

    char tickIntervalString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(ifaceData->m_hla->tickInterval, tickIntervalString);

    cout << "HLA tick() interval = " << tickIntervalString << " second(s)"
            << endl;

    EXTERNAL_SetReceiveDelay(ifaceData->m_hla->iface, ifaceData->m_hla->tickInterval);

    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-MOBILITY-INTERVAL",
        &retVal,
        &ifaceData->m_hla->mobilityInterval);

    if (retVal == FALSE)
    {
        ifaceData->m_hla->mobilityInterval = 500 * MILLI_SECOND;
    }

    HlaVerify(
        ifaceData->m_hla->mobilityInterval >= 0,
        "Can't use negative time");

    char mobilityIntervalString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(ifaceData->m_hla->mobilityInterval, mobilityIntervalString);

    cout << "HLA mobility interval = " << mobilityIntervalString << " second(s)"
            << endl;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-DYNAMIC-STATISTICS",
        &retVal,
        buf);

    cout << endl
         << "HLA dynamic statistics (via Comment interaction)         = ";

    if (retVal && strcmp(buf, "NO") == 0)
    {
        ifaceData->m_hla->hlaDynamicStats = false;
        cout << "Off" << endl;
    }
    else
    {
        ifaceData->m_hla->hlaDynamicStats = true;
        cout << "On" << endl;
    }

    if (ifaceData->m_hla->hlaDynamicStats)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "HLA-DYNAMIC-STATISTICS-METRIC-UPDATE-MODE",
            &retVal,
            buf);

        cout << "HLA dynamic statistics, Metric Update notifications      = ";

        if (!retVal || strcmp(buf, "BRIEF") != 0)
        {
            ifaceData->m_hla->verboseMetricUpdates = true;
            cout << "Verbose" << endl;
        }
        else
        {
            ifaceData->m_hla->verboseMetricUpdates = false;
            cout << "Brief" << endl;
        }

        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "HLA-DYNAMIC-STATISTICS-SEND-NODEID-DESCRIPTIONS",
            &retVal,
            buf);

        cout << "HLA dynamic statistics, nodeId Description notifications = ";

        if (retVal && strcmp(buf, "YES") == 0)
        {
            ifaceData->m_hla->sendNodeIdDescriptions = true;
            cout << "On" << endl;
        }
        else
        {
            ifaceData->m_hla->sendNodeIdDescriptions = false;
            cout << "Off" << endl;
        }

        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "HLA-DYNAMIC-STATISTICS-SEND-METRIC-DEFINITIONS",
            &retVal,
            buf);

        cout << "HLA dynamic statistics, Metric Definition notifications  = ";

        if (retVal && strcmp(buf, "YES") == 0)
        {
            ifaceData->m_hla->sendMetricDefinitions = true;
            cout << "On" << endl;
        }
        else
        {
            ifaceData->m_hla->sendMetricDefinitions = false;
            cout << "Off" << endl;
        }
    }//if//

    cout << endl;

    // Store terrain boundaries in federate ambassador object for easy
    // retrieval.

    ifaceData->m_hla->minLat = partitionData->terrainData->getOrigin().latlonalt.latitude;
    ifaceData->m_hla->maxLat
        = ifaceData->m_hla->minLat
          + partitionData->terrainData->getDimensions().latlonalt.latitude;
    ifaceData->m_hla->minLon = partitionData->terrainData->getOrigin().latlonalt.longitude;
    ifaceData->m_hla->maxLon
        = ifaceData->m_hla->minLon
          + partitionData->terrainData->getDimensions().latlonalt.longitude;
}

void
HlaReadEntitiesFile(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput)
{
    // Determine path of .hla-entities file.

    char path[g_hlaPathBufSize];
    BOOL retVal;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-ENTITIES-FILE-PATH",
        &retVal,
        path);

    if (retVal == FALSE)
    {
        const char* defaultPath = "iitsec.hla-entities";
        strcpy(path, defaultPath);
    }

    cout << ".hla-entities file = " << path << "." << endl;

    // Determine number of lines in file.

    ifaceData->m_hla->numEntities = HlaGetNumLinesInFile(path);
    HlaVerify(ifaceData->m_hla->numEntities > 0, "File is empty", path);

    HlaMallocEntities(ifaceData);

    // Open file.

    FILE* fpEntities = fopen(path, "r");
    HlaVerify(fpEntities != NULL, "Can't open for reading", path);

    // Read file.

    char line[g_hlaLineBufSize];
    char token[g_hlaLineBufSize];
    char* p;
    char* next;
    unsigned lineNumber;
    unsigned i;
    for (i = 0, lineNumber = 1;
         i < ifaceData->m_hla->numEntities;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, g_hlaLineBufSize, fpEntities) != NULL,
                 "Not enough lines",
                 path);

        HlaVerify(strlen(line) < g_hlaLineBufSize - 1,
                 "Exceeds permitted line length",
                 path);

        next = line;

        HlaEntity& entity = ifaceData->m_hla->entities[i];

        // MarkingData.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read MarkingData", path, lineNumber);

        HlaTrimLeft(token);
        HlaTrimRight(token);

        HlaVerify(strlen(token) < sizeof(entity.markingData),
                 "MarkingData too large",
                 path, lineNumber);
        strcpy(entity.markingData, token);

        // MarkingData to entity pointer hash.

        HlaMarkingDataToEntityMap::iterator markIt =
            ifaceData->m_hla->markingDataToEntity.find(entity.markingData);
        HlaVerify( markIt == ifaceData->m_hla->markingDataToEntity.end(),
            "Entity with duplicate MarkingData",
            path, lineNumber);

        ifaceData->m_hla->markingDataToEntity[entity.markingData] = &entity;

        // ForceID (skip).

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read ForceID", path, lineNumber);

        // Country (skip).

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Country", path, lineNumber);

        // Geodetic position.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read latitude", path, lineNumber);

        char* endPtr = NULL;
        errno = 0;
        entity.lat = strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse latitude",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read longitude", path, lineNumber);

        errno = 0;
        entity.lon = strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse longitude",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read altitude", path, lineNumber);

        errno = 0;
        entity.alt = strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse altitude",
            path,
            lineNumber);

        entity.latScheduled = entity.lat;
        entity.lonScheduled = entity.lon;
        entity.altScheduled = entity.alt;

        HlaVerify(entity.lat >= -90.0 && entity.lat <= 90.0
                 && entity.lon >= -180.0 && entity.lon <= 180.0,
                 "Invalid geodetic coordinates",
                 path, lineNumber);

        // Geocentric Cartesian position.

        HlaConvertLatLonAltToGcc(
            entity.lat, entity.lon, entity.alt, entity.x, entity.y, entity.z);

        entity.xScheduled = entity.x;
        entity.yScheduled = entity.y;
        entity.zScheduled = entity.z;

        // Miscellaneous.

        entity.azimuth = 0;
        entity.elevation = 0;
        entity.azimuthScheduled = 0;
        entity.elevationScheduled = 0;

        entity.speed = 0.0;
        entity.speedScheduled = 0.0;

        entity.damageState = HLA_DAMAGE_STATE_NO_DAMAGE;
        entity.hierarchyIdExists = false;
        entity.numRadioPtrs = 0;
        entity.mappedToHandle = false;

        const clocktype neverHappenedTime = -1;
        entity.lastScheduledMobilityEventTime = neverHappenedTime;
    }//for//
}

void
HlaReadRadiosFile(PartitionData* partitionData, HlaInterfaceData* ifaceData, NodeInput* nodeInput)
{
    // Determine path of .hla-radios file.

    char path[g_hlaPathBufSize];
    BOOL retVal;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-RADIOS-FILE-PATH",
        &retVal,
        path);

    if (retVal == FALSE)
    {
        const char* defaultPath = "iitsec.hla-radios";
        strcpy(path, defaultPath);
    }

    cout << ".hla-radios file   = " << path << "." << endl;

    // Determine number of lines in file.

    ifaceData->m_hla->numRadios = HlaGetNumLinesInFile(path);
        HlaVerify(ifaceData->m_hla->numRadios > 0, "File is empty", path);


    HlaMallocRadios(ifaceData);

    // Open file.

    FILE* fpRadios = fopen(path, "r");
    HlaVerify(fpRadios != NULL, "Can't open for reading", path);

    // Read file.

    char line[g_hlaLineBufSize];
    char token[g_hlaLineBufSize];
    char* p;
    char* next;
    unsigned i;
    unsigned lineNumber;
    for (i = 0, lineNumber = 1;
         i < ifaceData->m_hla->numRadios;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, g_hlaLineBufSize, fpRadios) != NULL,
                 "Not enough lines",
                 path);

        HlaVerify(strlen(line) < g_hlaLineBufSize - 1,
                 "Exceeds permitted line length",
                 path);

        next = line;

        HlaRadio& radio = ifaceData->m_hla->radios[i];

        // Node pointer <- nodeId.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read nodeId", path, lineNumber);

        errno = 0;
        unsigned nodeId = (unsigned) strtoul(token, NULL, 10);
        HlaVerify(errno == 0, "Can't parse nodeId", path, lineNumber);

        unsigned j;
        for (j = 0; j < i; j++)
        {
            HlaVerify(nodeId != ifaceData->m_hla->radios[j].node->nodeId,
                     "Radio with duplicate nodeId",
                     path, lineNumber);
        }

        BOOL remoteOk = TRUE;
        if (PARTITION_ReturnNodePointer (partitionData, &radio.node,
            nodeId, remoteOk) == FALSE)
        {
            HlaVerify(FALSE,
                 "Can't get node pointer associated with nodeId",
                 path, lineNumber);
        }

        // Initialize per-node HLA data for this nodeId.
        HlaData& hlaData = ifaceData->m_hla->nodeIdToPerNodeData[nodeId];

        hlaData.radioPtr = &radio;
        hlaData.nextMsgId = 0;

        // MarkingData.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read MarkingData", path, lineNumber);

        HlaTrimLeft(token);
        HlaTrimRight(token);

        HlaVerify(strlen(token) < g_hlaMarkingDataBufSize,
                 "MarkingData too large",
                 path, lineNumber);

        // Assign radio's entity pointer using MarkingData.

        HlaMarkingDataToEntityMap::iterator markIt =
            ifaceData->m_hla->markingDataToEntity.find(token);

        HlaVerify(markIt != ifaceData->m_hla->markingDataToEntity.end(),
                  "Can't find entity with MarkingData",
                  path, lineNumber);

        HlaEntity *entity = markIt->second;

        radio.entityPtr = entity;

        // Assign radio pointer to host entity.

        HlaVerify(entity->numRadioPtrs < g_hlaMaxRadiosPerEntity,
                  "Exceeded max radios per entity");

        entity->radioPtrs[entity->numRadioPtrs] = &radio;
        entity->numRadioPtrs++;

        // RadioIndex.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read RadioIndex", path, lineNumber);

        errno = 0;
        radio.radioIndex = (unsigned short) strtoul(token, NULL, 10);
        HlaVerify(errno == 0, "Can't parse RadioIndex", path, lineNumber);

        // MarkingData + RadioIndex to radio pointer hash.

        HlaRadioKey radioKey;
        strcpy(radioKey.markingData, entity->markingData);
        radioKey.radioIndex = radio.radioIndex;

        HlaRadioKeyToRadioMap::iterator radioIt =
            ifaceData->m_hla->radioKeyToRadio.find(radioKey);
        HlaVerify( radioIt == ifaceData->m_hla->radioKeyToRadio.end(),
            "Radio with duplicate MarkingData and RadioIndex",
            path, lineNumber);

        ifaceData->m_hla->radioKeyToRadio[radioKey] = &radio;

        // RelativePosition.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL,
                 "Can't read RelativePosition (x)",
                 path, lineNumber);

        char* endPtr = NULL;
        errno = 0;
        radio.relativePositionX = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (x)",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL,
                 "Can't read RelativePosition (y)",
                 path, lineNumber);

        errno = 0;
        radio.relativePositionY = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (y)",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL,
                 "Can't read RelativePosition (z)",
                 path, lineNumber);

        errno = 0;
        radio.relativePositionZ = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (z)",
            path,
            lineNumber);

        // Geodetic position.

        // When calculating positions for hosted radios, QualNet assumes
        // the host entity is pointed in the same direction as the GCC
        // x-axis, with no roll with respect to the earth's equatorial
        // plane, and with the top of the entity facing the same direction
        // as the north pole.
        //
        // Relative positions for the entity y- and z-axes are flipped with
        // respect to the GCC y- and z-axes.

        double radioX = entity->x + radio.relativePositionX;
        double radioY = entity->y - radio.relativePositionY;
        double radioZ = entity->z - radio.relativePositionZ;

        HlaConvertGccToLatLonAlt(
            radioX,
            radioY,
            radioZ,
            radio.latScheduled,
            radio.lonScheduled,
            radio.altScheduled);

        // Miscellaneous.

        radio.txOperationalStatus
            = HLA_TX_OPERATIONAL_STATUS_ON_BUT_NOT_TRANSMITTING;

        // If this is a proxy for a node that is on a different partiont,
        // we use dynamic hierarchy to obtain the node's transmitPower.
        // Internal Doc: 9-07: Dynamic hierarchy can be used
        // here because the other partitions won't invoke
        // HlaInitNodes () - which is calling us - because
        // only partition 0 performs HlaInitNodes (). So all the
        // other partitions will have completed
        // EXTERNAL_InitializeExternalInterfaces () and thus will be in
        // their event loops and able to respond to dyn hierarchy reqs.
        double txPower;
        EXTERNAL_PHY_GetTxPower (radio.node, 0, &txPower);
        if (txPower > 0.0)
        {
            radio.usesTxPower = true;
            radio.maxTxPower = txPower;
            radio.currentMaxTxPower = txPower;
        }
        else
        {
            // Radio has no physicals
            radio.usesTxPower = false;
        }

        radio.mappedToHandle = false;
        radio.networkPtr = NULL;
        radio.defaultDstRadioPtr = NULL;
    }//for//
}

void
HlaReadNetworksFile(PartitionData* partitionData, HlaInterfaceData * ifaceData,
    NodeInput* nodeInput)
{
    // Determine path of .hla-networks file.

    char path[g_hlaPathBufSize];
    BOOL retVal;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "HLA-NETWORKS-FILE-PATH",
        &retVal,
        path);

    if (retVal == FALSE)
    {
        const char* defaultPath = "iitsec.hla-networks";
        strcpy(path, defaultPath);
    }

    cout << ".hla-networks file = " << path << "." << endl
         << endl;

    // Determine number of lines in file.

    ifaceData->m_hla->numNetworks = HlaGetNumLinesInFile(path);
    HlaVerify(ifaceData->m_hla->numNetworks > 0, "File is empty", path);

    HlaMallocNetworks(ifaceData);

    // Open file.

    FILE* fpNetworks = fopen(path, "r");
    HlaVerify(fpNetworks != NULL, "Can't open for reading", path);

    // Read file.

    char line[g_hlaLineBufSize];
    char token[g_hlaLineBufSize];
    char nodeIdToken[g_hlaLineBufSize];
    char* p;
    char* next;
    char* nextNodeIdToken;
    unsigned i;
    unsigned lineNumber = 0;
    for (i = 0, lineNumber = 1;
         i < ifaceData->m_hla->numNetworks;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, g_hlaLineBufSize, fpNetworks) != NULL,
                 "Not enough lines",
                 path);

        HlaVerify(strlen(line) < g_hlaLineBufSize - 1,
                 "Exceeds permitted line length",
                 path);

        next = line;

        HlaNetwork& network = ifaceData->m_hla->networks[i];

        // Name of network.

        p = IO_GetDelimitedToken(token, next, ";\n", &next);
        HlaVerify(p != NULL, "Can't read network name", path, lineNumber);
        HlaVerify(strlen(token) < sizeof(network.name),
                 "Network name too long",
                 path, lineNumber);

        assert(sizeof(network.name) <= sizeof(token));

        strcpy(network.name, token);

        // Frequency.

        p = IO_GetDelimitedToken(token, next, ";\n", &next);
        HlaVerify(p != NULL, "Can't read network frequency", path, lineNumber);

        int retVal = atouint64(token, &network.frequency);
        HlaVerify(retVal == 1,
                 "Can't parse network frequency",
                 path, lineNumber);

        // nodeIds.

        p = IO_GetDelimitedToken(token, next, ";\n", &next);
        HlaVerify(p != NULL, "Can't read list of nodeIds", path, lineNumber);

        network.numRadioPtrs = 0;

        nextNodeIdToken = token;
        while (IO_GetDelimitedToken(nodeIdToken, nextNodeIdToken, ",",
                                    &nextNodeIdToken))
        {
            errno = 0;
            unsigned nodeId = (unsigned) strtoul(nodeIdToken, NULL, 10);
            HlaVerify(errno == 0,
                     "Couldn't parse nodeId",
                     path, lineNumber);

            assert(ifaceData->m_hla->numRadios > 0);
            unsigned j;
            for (j = 0; j < ifaceData->m_hla->numRadios; j++)
            {
                HlaRadio& radio = ifaceData->m_hla->radios[j];
                if (radio.node->nodeId == nodeId) { break; }
            }

            HlaVerify(
                j != ifaceData->m_hla->numRadios,
                "Can't find nodeId in list of radios",
                path, lineNumber);

            HlaRadio& radio = ifaceData->m_hla->radios[j];

            network.radios.push_back(&radio);

            network.numRadioPtrs++;

            HlaVerify(
                radio.networkPtr == NULL,
                "nodeId present in more than one network",
                path, lineNumber);

            radio.networkPtr = &network;
        }

        HlaVerify(network.numRadioPtrs > 0,
                 "Network must have at least one node",
                 path, lineNumber);

        // IP address.

        p = IO_GetDelimitedToken(token, next, ";\n", &next);
        HlaVerify(p != NULL,
                 "Can't read IP address",
                 path, lineNumber);

        int numHostBits;
        BOOL isNodeId;

        IO_ParseNodeIdHostOrNetworkAddress(
            token,
            &network.ipAddress,
            &numHostBits,
            &isNodeId);

        HlaVerify(!isNodeId, "Can't parse IP address", path, lineNumber);

        network.unicast = (network.ipAddress == 0);
    }//for//
}

void
HlaMapDefaultDstRadioPtrsToRadios(HlaInterfaceData * ifaceData)
{
    // This function assigns the first node in the same network as the source
    // node as the default destination.  This default is used when the Comm
    // Effects Request does not specify a receiver.
    //
    // TODO:  The default destination can be specified as a nodeId in the
    // .hla-radios file.

    assert(ifaceData->m_hla->numRadios > 0);

    unsigned i;
    for (i = 0; i < ifaceData->m_hla->numRadios; i++)
    {
        HlaRadio& srcRadio = ifaceData->m_hla->radios[i];

        if (srcRadio.networkPtr == NULL)
        {
            char warningString[MAX_STRING_LENGTH];
            sprintf(warningString,
                    "node %u does not have an associated network",
                    srcRadio.node->nodeId);
            HlaReportWarning(warningString, __FILE__, __LINE__);
            continue;
        }

        const HlaNetwork& network = *srcRadio.networkPtr;

        if (!network.unicast)
        {
            assert(srcRadio.defaultDstRadioPtr == NULL);
            continue;
        }

        assert(srcRadio.defaultDstRadioPtr == NULL);
        assert(network.numRadioPtrs > 0);

        unsigned j;
        for (j = 0; j < network.radios.size(); j++)
        {
            const HlaRadio& dstRadio = *network.radios[j];

            if (dstRadio.node == srcRadio.node) { continue; }

            srcRadio.defaultDstRadioPtr = &dstRadio;
            break;
        }

        if (j == network.numRadioPtrs)
        {
            char errorString[MAX_STRING_LENGTH];
            sprintf(errorString,
                    "HLA: Can't determine default destination for node %u",
                    srcRadio.node->nodeId);
            HlaReportError(errorString);
            continue;

        }
    }//for//
}

void
HlaMapHierarchyIds(HlaInterfaceData * ifaceData)
{
    assert(ifaceData->m_hla->partitionData != NULL);
    assert(ifaceData->m_hla->partitionData->nodeInput != NULL);

    // Fill hash table of nodeId -> hierarchy IDs.

    const NodeInput& nodeInput = *ifaceData->m_hla->partitionData->nodeInput;

    assert(nodeInput.numLines >= 0);

    unsigned i;
    for (i = 0; i < (unsigned) nodeInput.numLines; i++)
    {
        const char* valueString = nodeInput.values[i];

        if (strcmp(nodeInput.variableNames[i], "COMPONENT") != 0)
        { continue;  }

        errno = 0;
        int hierarchyId = (int) strtol(valueString, NULL, 10);
        HlaVerify(
            errno == 0,
            "Can't parse hierarchy ID",
            ".config file");

        if (hierarchyId == 0) { continue; }

        const char* p = strchr(valueString, '{');

        HlaVerify(
            p != NULL && p[1] != 0,
            "Invalid COMPONENT value",
            ".config file");

        const char* next = p + 1;  // Points to character after '{'.
        char nodeIdList[g_hlaLineBufSize];
        unsigned numBytesCopied;
        bool foundEmptyField;
        bool overflowed;

        p = HlaGetTokenOrEmptyField(
                nodeIdList,
                sizeof(nodeIdList),
                numBytesCopied,
                overflowed,
                next,
                "}",
                foundEmptyField);

        if (numBytesCopied == 0)
        {
            // Hierarchy has no associated nodeIds.

            continue;
        }

        HlaVerify(
            p != NULL
              && numBytesCopied + 1 < sizeof(nodeIdList),
            "Invalid COMPONENT value",
            ".config file");

        char nodeIdString[g_hlaLineBufSize];
        next = nodeIdList;

        while (1)
        {
            p = HlaGetToken(
                    nodeIdString,
                    sizeof(nodeIdString),
                    numBytesCopied,
                    overflowed,
                    next,
                    " \t,");

            if (p == NULL) { break; }

            HlaVerify(
                numBytesCopied + 1 < sizeof(nodeIdString),
                "nodeId string too long",
                ".config file");

            if (!isdigit(nodeIdString[0])) { continue; }

            errno = 0;
            unsigned nodeId = (unsigned) strtoul(nodeIdString, NULL, 10);
            HlaVerify(
                errno == 0,
                "Can't parse nodeId",
                ".config file");

            HlaNodeIdToHierarchyIdMap::iterator hierIt =
                ifaceData->m_hla->nodeIdToHierarchyId.find(nodeId);
            HlaVerify( hierIt == ifaceData->m_hla->nodeIdToHierarchyId.end(),
                "nodeId belongs to more than one hierachy",
                ".config file");

            ifaceData->m_hla->nodeIdToHierarchyId[nodeId] = hierarchyId;
        }//while//
    }//for//

    for (i = 0; i < ifaceData->m_hla->numEntities; i++)
    {
        assert(ifaceData->m_hla->entities != NULL);
        HlaEntity& entity = ifaceData->m_hla->entities[i];

        assert(!entity.hierarchyIdExists);

        unsigned j;
        for (j = 0; j < entity.numRadioPtrs; j++)
        {
            assert(entity.radioPtrs[j] != NULL);
            const HlaRadio& radio = *entity.radioPtrs[j];

            assert(radio.node != NULL);

            HlaNodeIdToHierarchyIdMap::iterator hierIt =
                ifaceData->m_hla->nodeIdToHierarchyId.find(radio.node->nodeId);

            if (hierIt != ifaceData->m_hla->nodeIdToHierarchyId.end())
            {
                int hierarchyId = hierIt->second;

                entity.hierarchyIdExists = true;
                entity.hierarchyId = hierarchyId;

                break;
            }
        }//for//
    }//for//
}

void
HlaInitMessenger(PartitionData* partitionData, NodeInput* nodeInput)
{
    Node* node = partitionData->firstNode;

    while (node)
    {
        MessengerInit(node);
        node = node->nextNodeData;
    }
}

void
HlaCreateFederation(HlaInterfaceData * ifaceData)
{
    cout << "FED: Trying to create federation "
            << ifaceData->m_hlaFedAmb->m_federationName << " ... " << endl;

    try
    {
        ifaceData->m_hlaRtiAmb->createFederationExecution(ifaceData->m_hlaFedAmb->m_federationName,
                                            ifaceData->m_hlaFedAmb->m_fedFilePath);
    }
    catch (RTI::FederationExecutionAlreadyExists)
    {
        cout << "FED: Federation execution already exists." << endl;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaJoinFederation(HlaInterfaceData * ifaceData)
{
    cout << "FED: Trying to join federate " << ifaceData->m_hlaFedAmb->m_federateName
            << " to federation " << ifaceData->m_hlaFedAmb->m_federationName
            << " ... " << endl;

    while (1)
    {
        try
        {
            ifaceData->m_hlaRtiAmb->joinFederationExecution(
                ifaceData->m_hlaFedAmb->m_federateName,
                ifaceData->m_hlaFedAmb->m_federationName,
                ifaceData->m_hlaFedAmb);
            break;
        }
        catch (RTI::FederationExecutionDoesNotExist&)
        {
            // Federation execution may not be ready to accept joins yet, so
            // sleep for 1 second and try again.

            HlaSleep(1);
        }
        catch (RTI::Exception& e)
        {
            cerr << "FED: " << &e << endl;
            exit(EXIT_FAILURE);
        }
    }

    cout << "FED: Successfully joined federation." << endl;
}

void
HlaRegisterCtrlCHandler()
{
    signal(SIGINT, HlaHandleSigInt);

    cout << "FED: Press Ctrl-C to end program." << endl
         << endl;
}

void
HlaHandleSigInt(int a)
{
    // Ctrl-C pressed.
    // sigval sigev_value
    // sigval.sival_ptr

    // NYI, have to store the partitionData in the siginfo state
    HlaEndSimulation(NULL);
}

void
HlaEndSimulation(HlaInterfaceData * ifaceData)
{
    if (ifaceData == NULL)
    {
        ifaceData = g_ifaceData;
    }

    cout << endl << "Ending program ... " << endl;
    ifaceData->m_hla->endProgram = true;

    EXTERNAL_SetSimulationEndTime(ifaceData->m_interface,
        ifaceData->m_hla->partitionData->theCurrentTime);
}

void
HlaGetObjectAndInteractionClassHandles(HlaInterfaceData * ifaceData)
{
    try
    {
        // BaseEntity object.

        ifaceData->m_hlaFedAmb->m_baseEntityHandle
            = ifaceData->m_hlaRtiAmb->getObjectClassHandle("BaseEntity");

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_beEntityIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "EntityIdentifier", ifaceData->m_hlaFedAmb->m_baseEntityHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_beEntityIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "EntityID", ifaceData->m_hlaFedAmb->m_baseEntityHandle);
        }
        else { assert(0); }

        ifaceData->m_hlaFedAmb->m_beOrientationHandle
            = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                  "Orientation", ifaceData->m_hlaFedAmb->m_baseEntityHandle);

        ifaceData->m_hlaFedAmb->m_beVelocityVectorHandle
            = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                  "VelocityVector", ifaceData->m_hlaFedAmb->m_baseEntityHandle);

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_beWorldLocationHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "WorldLocation", ifaceData->m_hlaFedAmb->m_baseEntityHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_beWorldLocationHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "Position", ifaceData->m_hlaFedAmb->m_baseEntityHandle);
        }
        else { assert(0); }

        // BaseEntity.PhysicalEntity object.

        ifaceData->m_hlaFedAmb->m_physicalEntityHandle
            = ifaceData->m_hlaRtiAmb->getObjectClassHandle(
                  "BaseEntity.PhysicalEntity");
        ifaceData->m_hlaFedAmb->m_peDamageStateHandle
            = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                  "DamageState", ifaceData->m_hlaFedAmb->m_physicalEntityHandle);
        ifaceData->m_hlaFedAmb->m_peMarkingHandle
            = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                  "Marking", ifaceData->m_hlaFedAmb->m_physicalEntityHandle);

        // EmbeddedSystem object.

        ifaceData->m_hlaFedAmb->m_embeddedSystemHandle
            = ifaceData->m_hlaRtiAmb->getObjectClassHandle("EmbeddedSystem");

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_esEntityIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "EntityIdentifier", ifaceData->m_hlaFedAmb->m_embeddedSystemHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_esEntityIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "EntityID", ifaceData->m_hlaFedAmb->m_embeddedSystemHandle);
        }
        else { assert(0); }

        // EmbeddedSystem.RadioTransmitter object.

        ifaceData->m_hlaFedAmb->m_radioTransmitterHandle
            = ifaceData->m_hlaRtiAmb->getObjectClassHandle(
                  "EmbeddedSystem.RadioTransmitter");

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_rtRadioIndexHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "RadioIndex", ifaceData->m_hlaFedAmb->m_radioTransmitterHandle);
            ifaceData->m_hlaFedAmb->m_rtTransmitterOperationalStatusHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "TransmitterOperationalStatus",
                      ifaceData->m_hlaFedAmb->m_radioTransmitterHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_rtRadioIndexHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "RadioNumber", ifaceData->m_hlaFedAmb->m_radioTransmitterHandle);
            ifaceData->m_hlaFedAmb->m_rtTransmitterOperationalStatusHandle
                = ifaceData->m_hlaRtiAmb->getAttributeHandle(
                      "TransmitState",
                      ifaceData->m_hlaFedAmb->m_radioTransmitterHandle);
        }
        else { assert(0); }

        // RadioSignal.ApplicationSpecificRadioSignal interaction.

        ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle
            = ifaceData->m_hlaRtiAmb->getInteractionClassHandle(
                  "RadioSignal.ApplicationSpecificRadioSignal");

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_asrsHostRadioIndexHandle
                = ifaceData->m_hlaRtiAmb->getParameterHandle(
                      "HostRadioIndex",
                      ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_asrsHostRadioIndexHandle
                = ifaceData->m_hlaRtiAmb->getParameterHandle(
                      "HostRadioID",
                      ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        }
        else { assert(0); }

        ifaceData->m_hlaFedAmb->m_asrsDataRateHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "DataRate",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaFedAmb->m_asrsSignalDataLengthHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "SignalDataLength",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaFedAmb->m_asrsSignalDataHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "SignalData",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaFedAmb->m_asrsTacticalDataLinkTypeHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "TacticalDataLinkType",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaFedAmb->m_asrsTdlMessageCountHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "TDLMessageCount",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaFedAmb->m_asrsUserProtocolIdHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "UserProtocolID",
                  ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);

        // Comment interaction.

        ifaceData->m_hlaFedAmb->m_commentHandle
            = ifaceData->m_hlaRtiAmb->getInteractionClassHandle("Comment");
        ifaceData->m_hlaFedAmb->m_commentOriginatingEntityHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "OriginatingEntity", ifaceData->m_hlaFedAmb->m_commentHandle);
        ifaceData->m_hlaFedAmb->m_commentReceivingEntityHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "ReceivingEntity", ifaceData->m_hlaFedAmb->m_commentHandle);
        ifaceData->m_hlaFedAmb->m_commentVariableDatumSetHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "VariableDatumSet", ifaceData->m_hlaFedAmb->m_commentHandle);

        // Data interaction.

        ifaceData->m_hlaFedAmb->m_dataHandle
            = ifaceData->m_hlaRtiAmb->getInteractionClassHandle("Data");
        ifaceData->m_hlaFedAmb->m_dataOriginatingEntityHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "OriginatingEntity", ifaceData->m_hlaFedAmb->m_dataHandle);
        ifaceData->m_hlaFedAmb->m_dataReceivingEntityHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "ReceivingEntity", ifaceData->m_hlaFedAmb->m_dataHandle);

        if (ifaceData->m_hla->rprFomVersion == 1.0)
        {
            ifaceData->m_hlaFedAmb->m_dataRequestIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getParameterHandle(
                      "RequestIdentifier", ifaceData->m_hlaFedAmb->m_dataHandle);
        }
        else
        if (ifaceData->m_hla->rprFomVersion == 0.5)
        {
            ifaceData->m_hlaFedAmb->m_dataRequestIdentifierHandle
                = ifaceData->m_hlaRtiAmb->getParameterHandle(
                      "RequestID", ifaceData->m_hlaFedAmb->m_dataHandle);
        }
        else { assert(0); }

        ifaceData->m_hlaFedAmb->m_dataFixedDatumsHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "FixedDatums", ifaceData->m_hlaFedAmb->m_dataHandle);
        ifaceData->m_hlaFedAmb->m_dataVariableDatumSetHandle
            = ifaceData->m_hlaRtiAmb->getParameterHandle(
                  "VariableDatumSet", ifaceData->m_hlaFedAmb->m_dataHandle);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaSubscribeAndPublish(HlaInterfaceData * ifaceData)
{
    // Objects.

    try
    {
        RTI::AttributeHandleSet* set
            = RTI::AttributeHandleSetFactory::create(6);

        set->add(ifaceData->m_hlaFedAmb->m_beEntityIdentifierHandle);
        set->add(ifaceData->m_hlaFedAmb->m_beOrientationHandle);
        set->add(ifaceData->m_hlaFedAmb->m_beVelocityVectorHandle);
        set->add(ifaceData->m_hlaFedAmb->m_beWorldLocationHandle);
        set->add(ifaceData->m_hlaFedAmb->m_peDamageStateHandle);
        set->add(ifaceData->m_hlaFedAmb->m_peMarkingHandle);

        ifaceData->m_hlaRtiAmb->subscribeObjectClassAttributes(
            ifaceData->m_hlaFedAmb->m_physicalEntityHandle, *set);

        delete set;
        set = NULL;

        set = RTI::AttributeHandleSetFactory::create(3);

        set->add(ifaceData->m_hlaFedAmb->m_esEntityIdentifierHandle);
        set->add(ifaceData->m_hlaFedAmb->m_rtRadioIndexHandle);
        set->add(ifaceData->m_hlaFedAmb->m_rtTransmitterOperationalStatusHandle);

        ifaceData->m_hlaRtiAmb->subscribeObjectClassAttributes(
            ifaceData->m_hlaFedAmb->m_radioTransmitterHandle, *set);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    // Interactions.

    try
    {
        ifaceData->m_hlaRtiAmb->subscribeInteractionClass(
            ifaceData->m_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        ifaceData->m_hlaRtiAmb->publishInteractionClass(ifaceData->m_hlaFedAmb->m_commentHandle);
        ifaceData->m_hlaRtiAmb->publishInteractionClass(ifaceData->m_hlaFedAmb->m_dataHandle);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaWaitForFirstObjectDiscovery(HlaInterfaceData * ifaceData)
{
    cout << "Waiting for first HLA discoverObjectInstance callback ... ";
    cout.flush();

    while (1)
    {
        if (ifaceData->m_hla->endProgram)
        {
            // User pressed Ctrl-C.

            HlaFinalize(ifaceData->m_hla->iface);

            exit(EXIT_FAILURE);
        }

        ifaceData->m_hlaRtiAmb->tick();

        if (ifaceData->m_hla->dbl_referencePhysicalTime != 0.0)
        {
            cout << "received." << endl;
            break;
        }

        HlaUsleep(100000);
    }//while//
}

void
HlaCheckRequestAttributeUpdates(HlaInterfaceData * ifaceData)
{
    // Solicit attribute updates.  This is helpful if the QualNet federate
    // joins the federation after HLA objects have already been created (since
    // the RTI only propagates attribute values to other federates only on
    // calls to the updateAttributeValues() service).

    clocktype elapsedPhysicalTime
        = EXTERNAL_QueryExternalTime(ifaceData->m_hla->iface);

    clocktype nextRequestTime
        = ifaceData->m_hla->attributeUpdateRequestDelay
          + (ifaceData->m_hla->numAttributeUpdateRequests
             * ifaceData->m_hla->attributeUpdateRequestInterval);

    if (ifaceData->m_hla->numAttributeUpdateRequests
        < ifaceData->m_hla->maxAttributeUpdateRequests
        && elapsedPhysicalTime >= nextRequestTime)
    {
        ifaceData->m_hla->numAttributeUpdateRequests++;

        cout << "FED: Requesting object-attribute updates #"
                << ifaceData->m_hla->numAttributeUpdateRequests << endl;

        //HlaRequestAttributeUpdatesForClass();
        HlaRequestAttributeUpdates(ifaceData);
    }
}

#ifdef OLD_HLA_REQUEST_ATTRIBUTEUPDATEFORCLASS
void
HlaRequestAttributeUpdatesForClass()
{
    // When attribute updates for all objects in a class a desired, the
    // requestClassAttributeValueUpdate() service is more efficient than
    // calling requestObjectAttributeValueUpdate() for each object.
    //
    // However, some federates may not support this service.

    try
    {
        RTI::AttributeHandleSet* set
            = RTI::AttributeHandleSetFactory::create(4);

        set->add(ifaceData->m_hlaFedAmb->m_beEntityIdentifierHandle);
        set->add(ifaceData->m_hlaFedAmb->m_beOrientationHandle);
        set->add(ifaceData->m_hlaFedAmb->m_beWorldLocationHandle);
        set->add(ifaceData->m_hlaFedAmb->m_peMarkingHandle);

        ifaceData->m_hlaRtiAmb->requestClassAttributeValueUpdate(
            ifaceData->m_hlaFedAmb->m_physicalEntityHandle, *set);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}
#endif

void
HlaRequestAttributeUpdates(HlaInterfaceData * ifaceData)
{
    // This function requests attribute updates for objects which have not
    // been mapped to an entity or radio yet, and also for which certain
    // attributes have not been received.

    // PhysicalEntity objects.

    RTI::AttributeHandleSet* set
        = RTI::AttributeHandleSetFactory::create(4);

    set->add(ifaceData->m_hlaFedAmb->m_beEntityIdentifierHandle);
    set->add(ifaceData->m_hlaFedAmb->m_beOrientationHandle);
    set->add(ifaceData->m_hlaFedAmb->m_beWorldLocationHandle);
    set->add(ifaceData->m_hlaFedAmb->m_peMarkingHandle);

    HlaHandleToEntityObjectMap::iterator entityIt =
        ifaceData->m_hla->handleToEntityObject.begin();
    while (entityIt != ifaceData->m_hla->handleToEntityObject.end())
    {
        const HlaEntityObject& eo = entityIt->second;

        if (eo.entityPtr == NULL && !eo.entityIdExists)
        {
            try
            {
                ifaceData->m_hlaRtiAmb->requestObjectAttributeValueUpdate(
                    entityIt->first, *set);
            }
            catch (RTI::Exception& e)
            {
                cerr << "FED: " << &e << endl;
                exit(EXIT_FAILURE);
            }
        }//if//
        entityIt++;
    }//while//

    delete set;
    set = NULL;

    // RadioTransmitter objects.

    set = RTI::AttributeHandleSetFactory::create(2);

    set->add(ifaceData->m_hlaFedAmb->m_esEntityIdentifierHandle);
    set->add(ifaceData->m_hlaFedAmb->m_rtRadioIndexHandle);

    HlaHandleToRadioObjectMap::iterator radioIt =
        ifaceData->m_hla->handleToRadioObject.begin();
    while (radioIt != ifaceData->m_hla->handleToRadioObject.end())
    {
        const HlaRadioObject& ro = radioIt->second;

        if (ro.radioPtr == NULL
            && (!ro.entityIdExists || !ro.radioIndexExists))
        {
            try
            {
                ifaceData->m_hlaRtiAmb->requestObjectAttributeValueUpdate(
                    radioIt->first, *set);
            }
            catch (RTI::Exception& e)
            {
                cerr << "FED: " << &e << endl;
                exit(EXIT_FAILURE);
            }
        }//if//
        radioIt++;
    }//while//

    delete set;
    set = NULL;
}

double
HlaGetElapsedPhysicalTime (HlaInterfaceData * ifaceData)
{
    return (ifaceData->m_hla->partitionData->wallClock->getRealTimeAsDouble ()
           - ifaceData->m_hla->dbl_referencePhysicalTime);
}

void
HlaLeaveFederation(HlaInterfaceData * ifaceData)
{
    try
    {
        ifaceData->m_hlaRtiAmb->resignFederationExecution(
            RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaDestroyFederation(HlaInterfaceData * ifaceData)
{
    try
    {
        ifaceData->m_hlaRtiAmb->destroyFederationExecution(ifaceData->m_hlaFedAmb->m_federationName);
    }
    catch (RTI::FederatesCurrentlyJoined&)
    {
        // Do nothing.
    }
    catch (RTI::FederationExecutionDoesNotExist&)
    {
        // Do nothing.
    }
    catch (RTI::Exception& e)
    {
        // Tell the user in case it's a RTI misconfiguration.
        // We can ignore the failure since we're exiting.
        cerr << "FED: " << &e << endl;
    }
}

#ifdef MILITARY_RADIOS_LIB
void
HlaProcessSendRtssEvent(Node* node, Message* msg)
{
    if (node->partitionData->partitionId != 0)
    {
        // This node is not on partition 0, so
        // forward the processing of the ready to send event to p0
        // We'd like to use MESSAGE_RemoteSend (node, destNodeId, msg, -1);
        // but we can't becuase that looks up which partition the node
        // lives on, and we actually need to send to p0 proxy node.
        // We'd like to use EXTERNAL_MESSAGE_SendAnyNode () but again we can't
        // because that schedules on the partition where the node really
        // lives.
        // 1.) Allocate an RtssEventNotification message
        Message * rtssForwardedMsg = MESSAGE_Alloc(
            node,
            EXTERNAL_LAYER,    // special layer
            EXTERNAL_HLA,         // EXTERNAL_HLA,           // protocol
            MSG_EXTERNAL_HLA_SendRtssForwarded);
        // 2.) Set info - info is the node's nodeId
        MESSAGE_InfoAlloc (node, rtssForwardedMsg, sizeof (RtssForwardedInfo));
        RtssForwardedInfo * rtssForwardedInfo = (RtssForwardedInfo *)
            MESSAGE_ReturnInfo (rtssForwardedMsg);
        rtssForwardedInfo->nodeId = node->nodeId;
        // 3.) Forward the message to p0 - HLA fed on p0 will then
        // be able complete handling via HlaSendRtssNotification()
        int destPartition = 0;
        EXTERNAL_Interface * iface =
            node->partitionData->interfaceTable [EXTERNAL_HLA];
        EXTERNAL_MESSAGE_RemoteSend (iface, destPartition,
            rtssForwardedMsg, 0, EXTERNAL_SCHEDULE_LOOSELY);
    }
    else
    {
        HlaSendRtssNotification(node);
    }
}
#endif /* MILITARY_RADIOS_LIB */

void
HlaAppMessengerResultFcn(Node* node, Message* msg, BOOL success)
{
    // Messenger has returned a result for a given message.
    // This will be called for nodes on remote partitions (partition != 0)

    HlaAppMessengerResultFcnBody(node, msg, success);

    MESSAGE_Free(node, msg);

}

void
HlaAppMessengerResultCompleted (Node* node,
    const HlaSimulatedMsgInfo& smInfo, BOOL success)
{
    // This is the core function that is invoked on partition 0
    // so that the HLA federation can indicate that the messenger
    // copmleted.
    assert (node->partitionData->partitionId == 0);

    if (!success)
    {
        // Do nothing if result function indicates failure.
        //
        // Note that, for failure, the Messenger result function (this
        // function) is called with the source-node pointer.
        //
        // TODO:  Code can be written to check if the destination address maps
        // to a single node (and thus qualifies as unicast).  If unicast, then
        // code can be written to handle failure.
        //
        // TODO:  It should be determined whether Messenger-indicated failures
        // occur for non-unicast destinations, and if so, what such a failure
        // indicates.

        return;
    }

    HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);
    const HlaRadio& srcRadio = *smInfo.srcRadioPtr;

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    if (nodeIt == ifaceData->m_hla->nodeIdToPerNodeData.end())
    {
        // this node does not map to a HLA entity
        return;
    }

    HlaData& dstHlaData = nodeIt->second;

    const HlaRadio& dstRadio = *dstHlaData.radioPtr;

    nodeIt = ifaceData->m_hla->nodeIdToPerNodeData.find(srcRadio.node->nodeId);
    HlaData& srcHlaData = nodeIt->second;

    HlaOutstandingSimulatedMsgInfoMap::iterator msgIt =
        srcHlaData.outstandingSimulatedMsgInfo.find(smInfo.msgId);

    if (msgIt == srcHlaData.outstandingSimulatedMsgInfo.end())
    {
        // msgId doesn't exist in hash table.  The assumption is that this
        // msgId has been added and already removed from the hash table,
        // which also means that a Timeout notification has already been sent
        // for this msgId.
        //
        // This can occur in two situations:
        //
        // While this node has successfully received the simulated message,
        //
        // (1) All destinations indicated in the initial request have already
        //     been processed, and this node is not one of the destinations.
        // (2) A timeout occurred prior to this node receiving the simulated
        //     message.
        //
        // Return early (do not send another Timeout notification).

        //if (ifaceData->m_hla->debug)
        //{
        //    HlaPrintCommEffectsResult(node, smInfo, "IGNR");
        //}

        return;
    }

    HlaOutstandingSimulatedMsgInfo& osmInfo = msgIt->second;

    assert(osmInfo.numDstEntitiesProcessed < osmInfo.smDstEntitiesInfos.size());

    unsigned i;
    for (i = 0; i < osmInfo.smDstEntitiesInfos.size(); i++)
    {
        const HlaSimulatedMsgDstEntityInfo &smDstEntityInfo
            = osmInfo.smDstEntitiesInfos[i];

        if (smDstEntityInfo.dstEntityPtr == dstRadio.entityPtr) { break; }
    }

    if (i == osmInfo.smDstEntitiesInfos.size())
    {
        // The result function has been called with successful delivery to this
        // node, but this node isn't in the source node's list of destination
        // nodes.

        return;
    }

    HlaSimulatedMsgDstEntityInfo &smDstEntityInfo = osmInfo.smDstEntitiesInfos[i];

    if (smDstEntityInfo.processed)
    {
        // This destination has already been processed.

        return;
    }

    smDstEntityInfo.processed = true;
    smDstEntityInfo.success = (success == TRUE);

    osmInfo.numDstEntitiesProcessed++;

    HlaSendProcessMsgNotification(node, smInfo, osmInfo, 1);

    if (osmInfo.numDstEntitiesProcessed == osmInfo.smDstEntitiesInfos.size())
    {
        HlaSendTimeoutNotification(node, smInfo, osmInfo);

        srcHlaData.outstandingSimulatedMsgInfo.erase(msgIt);
    }
}

void
HlaAppMessengerResultFcnBody(Node* node, Message* msg, BOOL success)
{
    // This is the callback function that is invoked by the messenger
    // application when a message has completed.

    if (!success)
    {
        // Do nothing if result function indicates failure.
        //
        // Note that, for failure, the Messenger result function (this
        // function) is called with the source-node pointer.
        //
        // TODO:  Code can be written to check if the destination address maps
        // to a single node (and thus qualifies as unicast).  If unicast, then
        // code can be written to handle failure.
        //
        // TODO:  It should be determined whether Messenger-indicated failures
        // occur for non-unicast destinations, and if so, what such a failure
        // indicates.

        return;
    }

    NodeAddress destNodeId = node->nodeId;
    HlaSimulatedMsgInfo& smInfo
        = *((HlaSimulatedMsgInfo*) MESSAGE_ReturnPacket(msg));
        HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);

    if (node->partitionData->partitionId != 0)
    {
        // Forward this message back to partition 0 to
        // indicate the comm request was
        // successful.
        Message * completedMessengerMsg = MESSAGE_Alloc(
            node,
            EXTERNAL_LAYER,    // special layer
            EXTERNAL_HLA,         // EXTERNAL_HLA,           // protocol
            MSG_EXTERNAL_HLA_CompletedMessengerForwarded);
        EXTERNAL_HlaCompletedMessenegerForwardedInfo * completedInfo =
            (EXTERNAL_HlaCompletedMessenegerForwardedInfo *)
            MESSAGE_InfoAlloc (node, completedMessengerMsg,
            sizeof (EXTERNAL_HlaCompletedMessenegerForwardedInfo));
        completedInfo->destNodeId = node->nodeId;    // will be a proxy on p0
    //    completedInfo.srcNodeId = smInfo.srcNodeId;
        completedInfo->smInfo = smInfo;
        completedInfo->success = TRUE;
        EXTERNAL_Interface * iface =
            node->partitionData->interfaceTable [EXTERNAL_HLA];
        EXTERNAL_MESSAGE_RemoteSend (iface, 0 /* p0 */,
            completedMessengerMsg, 0, EXTERNAL_SCHEDULE_LOOSELY);
        return;
    }
    else
    {
        HlaAppMessengerResultCompleted (node, smInfo, success);
        return;
    }
}

void
HlaSendProcessMsgNotification(
    Node* node,
    const HlaSimulatedMsgInfo& smInfo,
    const HlaOutstandingSimulatedMsgInfo& osmInfo,
    unsigned short success)
{
    HlaDataIxnInfo dataIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = dataIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    const HlaRadio& srcRadio = *smInfo.srcRadioPtr;

    HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    HlaData& dstHlaData = nodeIt->second;

    const HlaRadio& dstRadio = *dstHlaData.radioPtr;

    // OriginatingEntity, NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    dataIxnInfo.originatingEntity = srcRadio.entityPtr->entityId;

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned processMsgNotificationDatumId = 60001;
    vdInfo.datumId = processMsgNotificationDatumId;

    // DatumValue.
    // Converted to network-byte-order here.

    unsigned char* datumValue = vdInfo.datumValue;
    unsigned dvOffset = 0;

    HlaCopyToOffsetAndHtonEntityId(
        datumValue,
        dvOffset,
        &dataIxnInfo.originatingEntity);

    HlaCopyToOffsetAndHton(
        datumValue,
        dvOffset,
        &srcRadio.radioIndex,
        sizeof(srcRadio.radioIndex));

    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &osmInfo.timestamp, sizeof(osmInfo.timestamp));

    const unsigned numEntityIds = 1;
    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &numEntityIds, sizeof(numEntityIds));

    HlaCopyToOffsetAndHtonEntityId(
        datumValue,
        dvOffset,
        &dstRadio.entityPtr->entityId);

    const clocktype delay = TIME_getSimTime(node) - osmInfo.sendTime;
    const double double_delay = HlaConvertClocktypeToDouble(delay);
    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &double_delay, sizeof(double_delay));

    assert(dvOffset <= g_hlaMaxOutgoingDataIxnDatumValueSize);

    vdInfo.datumLength = dvOffset * 8;

    HlaSendDataIxn(ifaceData, dataIxnInfo);

    if (ifaceData->m_hla->debug)
    {
        HlaPrintCommEffectsResult(ifaceData, node, smInfo, "SUCC");
    }
}

void
HlaAppProcessTimeoutEvent(Node* node, Message* msg)
{
    HlaSimulatedMsgInfo& smInfo
        = *((HlaSimulatedMsgInfo*) MESSAGE_ReturnInfo(msg));

    HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);
    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    HlaData& hlaData = nodeIt->second;

    HlaOutstandingSimulatedMsgInfoMap::iterator msgIt =
        hlaData.outstandingSimulatedMsgInfo.find(smInfo.msgId);

    if (msgIt == hlaData.outstandingSimulatedMsgInfo.end())
    {
        // Timeout notification already sent.

        if (ifaceData->m_hla->debug)
        {
            HlaPrintCommEffectsResult(ifaceData, node, smInfo, "CANC");
        }

        return;
    }

    const HlaOutstandingSimulatedMsgInfo& osmInfo = msgIt->second;

    HlaSendTimeoutNotification(node, smInfo, osmInfo);

    hlaData.outstandingSimulatedMsgInfo.erase(msgIt);
}

void
HlaSendTimeoutNotification(
    Node* node,
    const HlaSimulatedMsgInfo& smInfo,
    const HlaOutstandingSimulatedMsgInfo& osmInfo)
{
    HlaDataIxnInfo dataIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = dataIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    const HlaRadio& srcRadio = *smInfo.srcRadioPtr;

    // OriginatingEntity, NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    dataIxnInfo.originatingEntity = srcRadio.entityPtr->entityId;

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned timeoutNotificationDatumId = 60002;
    vdInfo.datumId = timeoutNotificationDatumId;

    // DatumValue.
    // Converted to network-byte-order here.

    unsigned char* datumValue = vdInfo.datumValue;
    unsigned dvOffset = 0;

    HlaCopyToOffsetAndHtonEntityId(
        datumValue, dvOffset, &dataIxnInfo.originatingEntity);

    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &srcRadio.radioIndex, sizeof(srcRadio.radioIndex));

    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &osmInfo.timestamp, sizeof(osmInfo.timestamp));

    const unsigned numPackets = 1;
    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &numPackets, sizeof(numPackets));

    unsigned numEntityIds = 0;
    unsigned numEntityIdsOffset = dvOffset;
    dvOffset += sizeof(numEntityIds);

    unsigned infoOffset = dvOffset;
    unsigned i;
    for (i = 0; i < osmInfo.smDstEntitiesInfos.size(); )
    {
        dvOffset = infoOffset;
        unsigned j;
        for (j = 0; i < osmInfo.smDstEntitiesInfos.size() && j < g_hlaMaxDstEntityIdsInTimeoutNotification; j++, i++)
        {
            const HlaSimulatedMsgDstEntityInfo& smDstEntityInfo
                = osmInfo.smDstEntitiesInfos[i];

            assert(smDstEntityInfo.dstEntityPtr != srcRadio.entityPtr);

            HlaCopyToOffsetAndHtonEntityId(
                datumValue, dvOffset, &smDstEntityInfo.dstEntityPtr->entityId);

            HlaCopyToOffsetAndHton(
                datumValue,
                dvOffset,
                &smDstEntityInfo.success,
                sizeof(smDstEntityInfo.success));

            numEntityIds++;
        }

        assert(dvOffset <= g_hlaMaxOutgoingDataIxnDatumValueSize);

        HlaCopyToOffsetAndHton(
            datumValue, numEntityIdsOffset, &numEntityIds, sizeof(numEntityIds));

        vdInfo.datumLength = dvOffset * 8;

        HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);
        HlaSendDataIxn(ifaceData, dataIxnInfo);

        if (ifaceData->m_hla->debug)
        {
            HlaPrintCommEffectsResult(ifaceData, srcRadio.node, smInfo, "TIME");
        }
    }
}

#ifdef MILITARY_RADIOS_LIB
void
HlaSendRtssNotification(Node* node)
{
    // You may only call this function if you are on partition 0.

    HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);
    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    const HlaData& hlaData = nodeIt->second;

    HlaDataIxnInfo dataIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = dataIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    const HlaRadio& radio = *hlaData.radioPtr;

    // OriginatingEntity, NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    dataIxnInfo.originatingEntity = radio.entityPtr->entityId;

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned readyToSendSignalNotificationDatumId = 60010;
    vdInfo.datumId = readyToSendSignalNotificationDatumId;

    // DatumValue.
    // Converted to network-byte-order here.

    unsigned char* datumValue = vdInfo.datumValue;
    unsigned dvOffset = 0;

    HlaCopyToOffsetAndHtonEntityId(
        datumValue,
        dvOffset,
        &dataIxnInfo.originatingEntity);

    HlaCopyToOffsetAndHton(
        datumValue,
        dvOffset,
        &radio.radioIndex,
        sizeof(radio.radioIndex));

    const unsigned timestamp = HlaGetTimestamp();

    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &timestamp, sizeof(timestamp));

    const float windowTime = 0.0;

    HlaCopyToOffsetAndHton(
        datumValue, dvOffset, &windowTime, sizeof(windowTime));

    assert(dvOffset <= g_hlaMaxOutgoingDataIxnDatumValueSize);

    vdInfo.datumLength = dvOffset * 8;

    HlaSendDataIxn(ifaceData, dataIxnInfo);

    if (ifaceData->m_hla->debugPrintRtss)
    {
        HlaPrintSentRtssNotification(node);
    }
}

void
HlaPrintSentRtssNotification(Node* node)
{
    HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    const HlaData& hlaData = nodeIt->second;

    const HlaRadio& radio = *hlaData.radioPtr;

    char clocktypeString[g_hlaClocktypeStringBufSize];
    TIME_PrintClockInSecond(TIME_getSimTime(node), clocktypeString);

    cout << "FED: RTSS, "
            << radio.node->nodeId << " ("
            << radio.entityPtr->entityIdString
            << ") " << radio.entityPtr->markingData
            << ", time " << clocktypeString << endl;
}
#endif /* MILITARY_RADIOS_LIB */

void
HlaSendDataIxn(HlaInterfaceData * ifaceData,
    const HlaDataIxnInfo& dataIxnInfo)
{
    // The Data interaction sent contains only the parameters specified in the
    // ICD and those parameters required for RPR FOM 1.0.

    const HlaVariableDatumSetInfo& vdsInfo = dataIxnInfo.variableDatumSetInfo;

    assert(vdsInfo.numberOfVariableDatums == 1);

    RTI::ParameterHandleValuePairSet* set
        = RTI::ParameterSetFactory::create(4);

    // OriginatingEntity parameter.
    // Converted to network-byte-order.

    HlaEntityId nboOriginatingEntity;
    memcpy(&nboOriginatingEntity,
           &dataIxnInfo.originatingEntity,
           sizeof(nboOriginatingEntity));

    HlaHtonEntityId(nboOriginatingEntity);

    set->add(
        ifaceData->m_hlaFedAmb->m_dataOriginatingEntityHandle,
        (char*) &nboOriginatingEntity,
        sizeof(nboOriginatingEntity));

    // ReceivingEntity parameter.
    // Zero values are sent, since this parameter is not used in the ICD.
    // Not converted to network-byte-order since it's all zeroes.
    // This field is nonetheless required for RPR FOM 1.0.

    HlaEntityId nboReceivingEntity;
    memset(&nboReceivingEntity,
           0,
           sizeof(nboReceivingEntity));

    set->add(
        ifaceData->m_hlaFedAmb->m_dataReceivingEntityHandle,
        (char*) &nboReceivingEntity,
        sizeof(nboReceivingEntity));

    // RequestIdentifier parameter.
    // Zero values are sent, since this parameter is not used in the ICD.
    // Not converted to network-byte-order since it's all zeroes.
    // This field is nonetheless required for RPR FOM 1.0.

    unsigned nboRequestIdentifier = 0;

    set->add(
        ifaceData->m_hlaFedAmb->m_dataRequestIdentifierHandle,
        (char*) &nboRequestIdentifier,
        sizeof(nboRequestIdentifier));

    // VariableDatumSet parameter.
    // NumberofVariableDatums, DatumId, DatumLength.
    // Converted to network-byte-order.
    // DatumValue is already in network-byte-order.

    char variableDatumSet[g_hlaVariableDatumSetBufSize];
    unsigned vdsOffset = 0;

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdsInfo.numberOfVariableDatums,
        sizeof(vdsInfo.numberOfVariableDatums));

    const HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdInfo.datumId,
        sizeof(vdInfo.datumId));

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdInfo.datumLength,
        sizeof(vdInfo.datumLength));

    assert(vdInfo.datumLength % 8 == 0);
    unsigned datumLengthInBytes = vdInfo.datumLength / 8;
    assert(datumLengthInBytes <= g_hlaMaxOutgoingDataIxnDatumValueSize);

    HlaCopyToOffset(
        variableDatumSet,
        vdsOffset,
        vdInfo.datumValue,
        datumLengthInBytes);

    unsigned paddingSize = 8 - (datumLengthInBytes % 8);
    if (paddingSize == 8) { paddingSize = 0; }

    if (paddingSize != 0)
    {
        memset(&variableDatumSet[vdsOffset], 0, paddingSize);
        vdsOffset += paddingSize;
    }

    set->add(
        ifaceData->m_hlaFedAmb->m_dataVariableDatumSetHandle,
        variableDatumSet,
        vdsOffset);

    ifaceData->m_hlaRtiAmb->sendInteraction(ifaceData->m_hlaFedAmb->m_dataHandle, *set, NULL);

    delete set;
    set = NULL;
}

void
HlaPrintCommEffectsResult(HlaInterfaceData * ifaceData,
    Node* node, const HlaSimulatedMsgInfo& smInfo, const char* resultString)
{
    const HlaRadio& srcRadio = *smInfo.srcRadioPtr;

    HlaNodeIdToPerNodeDataMap::iterator nodeIt =
        ifaceData->m_hla->nodeIdToPerNodeData.find(node->nodeId);
    HlaData& dstHlaData = nodeIt->second;

    const HlaRadio& dstRadio = *dstHlaData.radioPtr;

    char clocktypeString[g_hlaClocktypeStringBufSize];
    TIME_PrintClockInSecond(TIME_getSimTime(node), clocktypeString);

    cout << "FED: " << resultString << ", "
            << srcRadio.node->nodeId << " ("
            << srcRadio.entityPtr->entityIdString
            << ") " << srcRadio.entityPtr->markingData
            << " to "
            << dstRadio.node->nodeId << " ("
            << dstRadio.entityPtr->entityIdString
            << ") " << dstRadio.entityPtr->markingData
            << ", time " << clocktypeString << endl;
}

void
HlaAppProcessChangeMaxTxPowerEvent(Node* node, Message* msg)
{
    HlaChangeMaxTxPowerInfo& info
        = *((HlaChangeMaxTxPowerInfo*) MESSAGE_ReturnInfo(msg));
        HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);

    const HlaEntity& entity = *info.entityPtr;

    bool   restoreMaxTxPower = false;
    double probability;
    double maxFraction;

    switch (info.damageState)
    {
        case HLA_DAMAGE_STATE_NO_DAMAGE:
            restoreMaxTxPower = true;
            break;
        case HLA_DAMAGE_STATE_SLIGHT_DAMAGE:
            probability
                = ifaceData->m_hla->slightDamageReduceTxPowerProbability;
            maxFraction
                = ifaceData->m_hla->slightDamageReduceTxPowerMaxFraction;
            break;
        case HLA_DAMAGE_STATE_MODERATE_DAMAGE:
            probability
                = ifaceData->m_hla->moderateDamageReduceTxPowerProbability;
            maxFraction
                = ifaceData->m_hla->moderateDamageReduceTxPowerMaxFraction;
            break;
        case HLA_DAMAGE_STATE_DESTROYED:
            probability
                = ifaceData->m_hla->destroyedReduceTxPowerProbability;
            maxFraction
                = ifaceData->m_hla->destroyedReduceTxPowerMaxFraction;
            break;
        default:
            assert(0);
    }

    assert(entity.numRadioPtrs > 0);

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        assert(entity.radioPtrs[i] != NULL);
        HlaRadio& radio = *entity.radioPtrs[i];

        if (!radio.usesTxPower) { continue; }

        Node* node = radio.node;

        double newTxPower;

        if (restoreMaxTxPower)
        {
            newTxPower = radio.maxTxPower;
        }
        else
        if (RANDOM_erand(ifaceData->m_hla->seed) > probability)
        {
            continue;
        }
        else
        {
            newTxPower = radio.currentMaxTxPower
                         * (maxFraction * RANDOM_erand(ifaceData->m_hla->seed));
        }

        cout << "FED: (" << entity.entityIdString
                << ", " << radio.radioIndex << ") " << entity.markingData
                << " Old TX power = " << radio.currentMaxTxPower
                << " New TX power = " << newTxPower << " mW" << endl;

        const int phyIndex = 0;

        // Ask EXTERNAL to change the phy tx power - this will become
        // a cross-partition message if needed (if node to change is remote)
        // Internal note: We could instead have ScheduleChangeMaxTxPower ()
        // perform the probability for each radio and then build an info
        // for each radio's node. That way the sceduled event would
        // get onto the correct partition a tiny bit sooner
        // (wall clock time) and then receiving partition would be
        // able to directly use PHY_SetTransmitPower (). However, it's
        // more work, not a big time diffs, and posisbly less efficent.
        EXTERNAL_PHY_SetTxPower(
            radio.node,
            phyIndex,
            newTxPower);

        radio.currentMaxTxPower = newTxPower;
    }//for//
}

void
HlaSendNodeIdDescriptionNotifications(HlaInterfaceData * ifaceData)
{
    assert(ifaceData->m_hla->numRadios > 0);
    assert(ifaceData->m_hla->radios != NULL);

    HlaCommentIxnInfo commentIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    // NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned nodeIdDescriptionNotificationDatumId = 60101;
    vdInfo.datumId = nodeIdDescriptionNotificationDatumId;

    char line[g_hlaMaxOutgoingCommentIxnDatumValueSize];

    // Initial size of DatumValue field includes the space for a terminating
    // null character.

    unsigned dvOffset = 0;
    unsigned currentDatumValueSizeInBytes = 1;
    unsigned i;
    for (i = 0; i < ifaceData->m_hla->numRadios; i++)
    {
        HlaRadio& radio = ifaceData->m_hla->radios[i];

        assert(radio.entityPtr != NULL);

        sprintf(
            line,
            "%u \"%s\" %u\n",
            radio.node->nodeId,
            radio.entityPtr->markingData,
            radio.radioIndex);

        unsigned lineLength = strlen(line);
        assert(lineLength < sizeof(line));

        unsigned potentialDatumValueSizeInBytes
            = currentDatumValueSizeInBytes + lineLength;

        // At least one record should fit in the notification.

        if (currentDatumValueSizeInBytes == 1)
        {
            assert(potentialDatumValueSizeInBytes
                   <= g_hlaMaxOutgoingCommentIxnDatumValueSize);
        }

        if (potentialDatumValueSizeInBytes
            > g_hlaMaxOutgoingCommentIxnDatumValueSize)
        {
            // Null-terminate string, send notification, and reset indexes to
            // DatumValue.

            vdInfo.datumValue[dvOffset] = 0;

            HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
                commentIxnInfo);

            dvOffset = 0;
            currentDatumValueSizeInBytes = 1;

            potentialDatumValueSizeInBytes
                = currentDatumValueSizeInBytes + lineLength;

            assert(potentialDatumValueSizeInBytes
                   <= g_hlaMaxOutgoingCommentIxnDatumValueSize);
        }//if//

        HlaCopyToOffset(
            vdInfo.datumValue,
            dvOffset,
            line,
            lineLength);

        currentDatumValueSizeInBytes = potentialDatumValueSizeInBytes;
    }//for//

    // Done loading all records.
    // If there's untransmitted data in DatumValue, transmit it now.

    if (currentDatumValueSizeInBytes > 1)
    {
        // Null-terminate string, send notification, and reset indexes to
        // DatumValue.

        vdInfo.datumValue[dvOffset] = 0;

        HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
            commentIxnInfo);

        dvOffset = 0;
        currentDatumValueSizeInBytes = 1;
    }
}

void
HlaSendNodeIdDescriptionNotification(HlaInterfaceData * ifaceData,
    const HlaRadio& radio)
{
    HlaCommentIxnInfo commentIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    // NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned nodeIdDescriptionNotificationDatumId = 60101;
    vdInfo.datumId = nodeIdDescriptionNotificationDatumId;

    // DatumValue.
    // Would be converted to network-byte-order here, but the entire value is a
    // character string.

    assert(radio.entityPtr != NULL);

    sprintf(
        (char*) vdInfo.datumValue,
        "%u \"%s\" %u\n",
        radio.node->nodeId,
        radio.entityPtr->markingData,
        radio.radioIndex);

    HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
        commentIxnInfo);
}

void
HlaSendMetricDefinitionNotifications(HlaInterfaceData * ifaceData)
{
    HlaCommentIxnInfo commentIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    // NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned metricDefinitionNotificationDatumId = 60102;
    vdInfo.datumId = metricDefinitionNotificationDatumId;

    char line[g_hlaMaxOutgoingCommentIxnDatumValueSize];

    // Initial size of DatumValue field includes the space for a terminating
    // null character.

    unsigned dvOffset = 0;
    unsigned currentDatumValueSizeInBytes = 1;
    unsigned i;
    for (i = 0; i < MAX_LAYERS; i++)
    {
        assert(g_metricData != NULL);
        const MetricLayerData& metricDataForLayer = g_metricData[i];

        assert(metricDataForLayer.numberUsed >= 0);

        unsigned j;
        for (j = 0; j < (unsigned) metricDataForLayer.numberUsed; j++)
        {
            assert(metricDataForLayer.metricList != NULL);
            MetricData& metric = metricDataForLayer.metricList[j];

            sprintf(
                line,
                "%d \"%s\" %d %d\n",
                metric.metricID,
                metric.metricName,
                (int) i,
                metric.metricDataType);

            unsigned lineLength = strlen(line);
            assert(lineLength < sizeof(line));

            unsigned potentialDatumValueSizeInBytes
                = currentDatumValueSizeInBytes + lineLength;

            // At least one record should fit in the notification.

            if (currentDatumValueSizeInBytes == 1)
            {
                assert(potentialDatumValueSizeInBytes
                       <= g_hlaMaxOutgoingCommentIxnDatumValueSize);
            }

            if (potentialDatumValueSizeInBytes
                > g_hlaMaxOutgoingCommentIxnDatumValueSize)
            {
                // Null-terminate string, send notification, and reset indexes
                // to DatumValue.

                vdInfo.datumValue[dvOffset] = 0;

                HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
                    commentIxnInfo);

                dvOffset = 0;
                currentDatumValueSizeInBytes = 1;

                potentialDatumValueSizeInBytes
                    = currentDatumValueSizeInBytes + lineLength;

                assert(potentialDatumValueSizeInBytes
                       <= g_hlaMaxOutgoingCommentIxnDatumValueSize);
            }//if//

            HlaCopyToOffset(
                vdInfo.datumValue,
                dvOffset,
                line,
                lineLength);

            currentDatumValueSizeInBytes = potentialDatumValueSizeInBytes;
        }//for//
    }//for//

    // Done loading all records.
    // If there's untransmitted data in DatumValue, transmit it now.

    if (currentDatumValueSizeInBytes > 1)
    {
        // Null-terminate string, send notification, and reset indexes to
        // DatumValue.

        vdInfo.datumValue[dvOffset] = 0;

        HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
            commentIxnInfo);

        dvOffset = 0;
        currentDatumValueSizeInBytes = 1;
    }
}

void
HlaSendMetricDefinitionNotification(HlaInterfaceData * ifaceData,
    const MetricData& metric, int metricLayer)
{
    HlaCommentIxnInfo commentIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    // NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned metricDefinitionNotificationDatumId = 60102;
    vdInfo.datumId = metricDefinitionNotificationDatumId;

    sprintf(
        (char*) vdInfo.datumValue,
        "%d \"%s\" %d %d\n",
        metric.metricID,
        metric.metricName,
        metricLayer,
        metric.metricDataType);

    HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
        commentIxnInfo);
}

void
HlaScheduleCheckMetricUpdate(HlaInterfaceData * ifaceData)
{
    Node* node = ifaceData->m_hla->partitionData->firstNode;

    Message* newMsg = MESSAGE_Alloc(node,
                                    EXTERNAL_LAYER,
                                    EXTERNAL_HLA,
                                    MSG_EXTERNAL_HLA_CheckMetricUpdate);

    MESSAGE_Send(
        node,
        newMsg,
        ifaceData->m_hla->checkMetricUpdateInterval);
}

void
HlaAppProcessCheckMetricUpdateEvent(Node* node, Message* msg)
{
        HlaInterfaceData * ifaceData = HlaGetIfaceDataFromNode (node);
    if (ifaceData->m_hla->metricUpdateSize > 1)
        {
        HlaSendMetricUpdateNotification(ifaceData);
    }

    HlaScheduleCheckMetricUpdate(ifaceData);
}

void
HlaSendMetricUpdateNotification(HlaInterfaceData* ifaceData)
{
    HlaCommentIxnInfo commentIxnInfo;
    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    // NumberOfVariableDatums, DatumID.
    // Remains in host-byte-order until HlaSendDataIxn().

    vdsInfo.numberOfVariableDatums = 1;

    const unsigned metricUpdateNotificationDatumId = 60103;
    vdInfo.datumId = metricUpdateNotificationDatumId;

    assert(strlen(ifaceData->m_hla->metricUpdateBuf) < sizeof(vdInfo.datumValue));
    strcpy((char*) vdInfo.datumValue, ifaceData->m_hla->metricUpdateBuf);

    HlaDetermineStringDatumLengthThenSendCommentIxn(ifaceData,
        commentIxnInfo);

    ifaceData->m_hla->metricUpdateBuf[0] = 0;
    ifaceData->m_hla->metricUpdateSize = 1;
}

void
HlaAppProcessMoveHierarchy(Node* node, Message* msg)
{
    HlaHierarchyMobilityInfo& info
        = *((HlaHierarchyMobilityInfo*) MESSAGE_ReturnInfo(msg));

    GUI_MoveHierarchy(
        info.hierarchyId,
        info.coordinates,
        info.orientation,
        TIME_getSimTime(node));
}

void
HlaDetermineStringDatumLengthThenSendCommentIxn(
    HlaInterfaceData * ifaceData,
    HlaCommentIxnInfo& commentIxnInfo)
{
    // The passed in Comment data is almost ready, except the DatumLength
    // field still needs to be set.  Set it, and transmit the Comment.

    HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;
    HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    unsigned datumLengthInBytes = strlen((char*) vdInfo.datumValue) + 1;
    assert(datumLengthInBytes <= g_hlaMaxOutgoingCommentIxnDatumValueSize);

    vdInfo.datumLength = datumLengthInBytes * 8;

    HlaSendCommentIxn(ifaceData, commentIxnInfo);

    //cout << "HLA dynamic stats " << vdInfo.datumId << endl
    //     << vdInfo.datumValue << endl;
}

void
HlaSendCommentIxn(HlaInterfaceData * ifaceData,
    const HlaCommentIxnInfo& commentIxnInfo)
{
    // The Comment interaction sent contains only the parameters specified in the
    // ICD and those parameters required for RPR FOM 1.0.

    const HlaVariableDatumSetInfo& vdsInfo = commentIxnInfo.variableDatumSetInfo;

    assert(vdsInfo.numberOfVariableDatums == 1);

    RTI::ParameterHandleValuePairSet* set
        = RTI::ParameterSetFactory::create(3);

    // OriginatingEntity parameter.
    // Zero values are sent, since this parameter is not used in the ICD.
    // Not converted to network-byte-order since it's all zeroes.
    // This field is nonetheless required for RPR FOM 1.0.

    HlaEntityId nboOriginatingEntity;
    memset(&nboOriginatingEntity,
           0,
           sizeof(nboOriginatingEntity));

    set->add(
        ifaceData->m_hlaFedAmb->m_commentOriginatingEntityHandle,
        (char*) &nboOriginatingEntity,
        sizeof(nboOriginatingEntity));

    // ReceivingEntity parameter.
    // Zero values are sent, since this parameter is not used in the ICD.
    // Not converted to network-byte-order since it's all zeroes.
    // This field is nonetheless required for RPR FOM 1.0.

    HlaEntityId nboReceivingEntity;
    memset(&nboReceivingEntity,
           0,
           sizeof(nboReceivingEntity));

    set->add(
        ifaceData->m_hlaFedAmb->m_commentReceivingEntityHandle,
        (char*) &nboReceivingEntity,
        sizeof(nboReceivingEntity));

    // VariableDatumSet parameter.
    // NumberofVariableDatums, DatumId, DatumLength.
    // Converted to network-byte-order.
    // DatumValue is already in network-byte-order.

    char variableDatumSet[g_hlaVariableDatumSetBufSize];
    unsigned vdsOffset = 0;

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdsInfo.numberOfVariableDatums,
        sizeof(vdsInfo.numberOfVariableDatums));

    const HlaVariableDatumInfo& vdInfo = vdsInfo.variableDatumsInfo[0];

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdInfo.datumId,
        sizeof(vdInfo.datumId));

    HlaCopyToOffsetAndHton(
        variableDatumSet,
        vdsOffset,
        &vdInfo.datumLength,
        sizeof(vdInfo.datumLength));

    assert(vdInfo.datumLength % 8 == 0);
    unsigned datumLengthInBytes = vdInfo.datumLength / 8;
    assert(datumLengthInBytes <= g_hlaMaxOutgoingCommentIxnDatumValueSize);

    HlaCopyToOffset(
        variableDatumSet,
        vdsOffset,
        vdInfo.datumValue,
        datumLengthInBytes);

    unsigned paddingSize = 8 - (datumLengthInBytes % 8);
    if (paddingSize == 8) { paddingSize = 0; }

    if (paddingSize != 0)
    {
        memset(&variableDatumSet[vdsOffset], 0, paddingSize);
        vdsOffset += paddingSize;
    }

    set->add(
        ifaceData->m_hlaFedAmb->m_commentVariableDatumSetHandle,
        variableDatumSet,
        vdsOffset);

    ifaceData->m_hlaRtiAmb->sendInteraction(ifaceData->m_hlaFedAmb->m_commentHandle, *set, NULL);

    delete set;
    set = NULL;
}
