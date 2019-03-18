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

#include <iostream>
using namespace std;
#include <cstdio>
#include <climits>
#include <csignal>
#include <cerrno>

#include "dis_shared.h"

#include "phy_abstract.h"

#ifdef MILITARY_RADIOS_LIB
#include "mac_link16.h"
#endif /* MILITARY_RADIOS_LIB */

#include "app_messenger.h"

Dis g_dis;

void
DisInitDisVariable(EXTERNAL_Interface* iface)
{
    // g_dis is a global variable, so all variables begin initialized to 0.

    g_dis.iface = iface;
    g_dis.partitionData = iface->partition;

    g_dis.minLat
        = g_dis.partitionData->terrainData->getOrigin().latlonalt.latitude;
    g_dis.maxLat
        = g_dis.minLat
        + g_dis.partitionData->terrainData->getDimensions().latlonalt.latitude;
    g_dis.minLon
        = g_dis.partitionData->terrainData->getOrigin().latlonalt.longitude;
    g_dis.maxLon
        = g_dis.minLon
        + g_dis.partitionData->terrainData->getDimensions().latlonalt.longitude;

    RANDOM_SetSeed(g_dis.seed, iface->partition->seedVal, 0, 0);
    g_dis.slightDamageReduceTxPowerProbability = 0.25;
    g_dis.moderateDamageReduceTxPowerProbability = 0.25;
    g_dis.destroyedReduceTxPowerProbability = 0.50;
    g_dis.slightDamageReduceTxPowerMaxFraction = 0.75;
    g_dis.moderateDamageReduceTxPowerMaxFraction = 0.75;
    g_dis.destroyedReduceTxPowerMaxFraction = 0.75;

    g_dis.altitudeOffset = 0.0;
}

void
DisReadParameters(EXTERNAL_Interface* iface, const NodeInput* nodeInput)
{
    BOOL parameterFound;
    char buf[MAX_STRING_LENGTH];
    double altitudeOffset(0.0);

    IO_ReadDouble(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-ALTITUDE-OFFSET",
        &parameterFound,
        &altitudeOffset);

    cout << "DIS altitude offset (Position)           = ";

    if (parameterFound)
    {
      g_dis.altitudeOffset = altitudeOffset;
      cout << buf << endl;
    }
    else
    {
      g_dis.altitudeOffset = 0.0;
      cout << "0.0" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-COMMS",
        &parameterFound,
        buf);

    cout << "DIS debugging output (Comms)           = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintComms = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintComms = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-COMMS-2",
        &parameterFound,
        buf);

    cout << "DIS debugging output (Comms-2)         = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintComms2 = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintComms2 = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-MAPPING",
        &parameterFound,
        buf);

    cout << "DIS debugging output (mapping)         = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintMapping = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintMapping = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-DAMAGE",
        &parameterFound,
        buf);

    cout << "DIS debugging output (damage)          = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintDamage = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintDamage = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-TX-STATE",
        &parameterFound,
        buf);

    cout << "DIS debugging output (TX state)        = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintTxState = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintTxState = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-TX-POWER",
        &parameterFound,
        buf);

    cout << "DIS debugging output (TX power)        = ";

    if (parameterFound && strcmp(buf, "NO") == 0)
    {
        g_dis.debugPrintTxPower = false;
        cout << "Off" << endl;
    }
    else
    {
        g_dis.debugPrintTxPower = true;
        cout << "On" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-MOBILITY",
        &parameterFound,
        buf);

    cout << "DIS debugging output (mobility)        = ";

    if (parameterFound && strcmp(buf, "YES") == 0)
    {
        g_dis.debugPrintMobility = true;
        cout << "On" << endl;
    }
    else
    {
        g_dis.debugPrintMobility = false;
        cout << "Off" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-TRANSMITTER-PDU",
        &parameterFound,
        buf);

    cout << "DIS debugging output (Transmitter PDU) = ";

    if (parameterFound && strcmp(buf, "YES") == 0)
    {
        g_dis.debugPrintTransmitterPdu = true;
        cout << "On" << endl;
    }
    else
    {
        g_dis.debugPrintTransmitterPdu = false;
        cout << "Off" << endl;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-DEBUG-PRINT-PDUS",
        &parameterFound,
        buf);

    cout << "DIS debugging output (PDU hex dump)    = ";

    if (parameterFound && strcmp(buf, "YES") == 0)
    {
        g_dis.debugPrintPdus = true;
        cout << "On" << endl;
    }
    else
    {
        g_dis.debugPrintPdus = false;
        cout << "Off" << endl;
    }

    cout << endl;


    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-IP-ADDRESS",
        &parameterFound,
        buf);

    if (!parameterFound)
    {
        // Parameter was not specified.

        // Default to listening to local broadcast IP address.

        strcpy(buf, "255.255.255.255");

        g_dis.disIpAddress = 0xffffffff;
        g_dis.disIpAddressIsMulticast = false;
    }
    else
    {
        // Parameter was specified.

        DisVerify(
            DisConvertStringToIpAddress(buf, g_dis.disIpAddress),
            "DIS-IP-ADDRESS cannot be parsed");

        if (DisIsMulticastIpAddress(g_dis.disIpAddress))
        {
            g_dis.disIpAddressIsMulticast = true;
        }
        else
        {
            g_dis.disIpAddressIsMulticast = false;
        }
    }

    if (g_dis.disIpAddressIsMulticast)
    {
        cout << "Listening for DIS PDUs at multicast IP address "
                << buf << "." << endl;
    }
    else
    {
        cout << "Listening for DIS PDUs at local broadcast IP address "
                << buf << "." << endl;
    }

    int int_disPort;

    IO_ReadInt(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-PORT",
        &parameterFound,
        &int_disPort);

    if (parameterFound)
    {
        DisVerify(int_disPort > 0, "DIS port must be > 0");
        DisVerify(int_disPort <= USHRT_MAX, "DIS port too large");

        g_dis.disPort = (unsigned short) int_disPort;
    }
    else
    {
        g_dis.disPort = 3000;
    }

    cout << "Listening for DIS PDUs on UDP port " << g_dis.disPort << "."
            << endl;

    int int_exerciseIdentifier;

    IO_ReadInt(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-EXERCISE-ID",
        &parameterFound,
        &int_exerciseIdentifier);

    if (parameterFound)
    {
        DisVerify(int_exerciseIdentifier >= 0,
                  "DIS Exercise Identifier must be >= 0");
        DisVerify(int_exerciseIdentifier <= UCHAR_MAX,
                  "DIS Exercise Identifier too large");

        g_dis.filterOnExerciseIdentifier = true;
        g_dis.exerciseIdentifier = (unsigned char) int_exerciseIdentifier;

        cout << "Filtering for DIS PDUs with Exercise ID "
                 << (unsigned) g_dis.exerciseIdentifier << "."
                 << endl;
    }
    else
    {
        assert(g_dis.filterOnExerciseIdentifier == false);

        cout << "Accepting DIS PDUs for all Exercise ID values." << endl;
    }

    cout << endl;

    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-RECEIVE-DELAY",
        &parameterFound,
        &g_dis.receiveDelay);

    if (parameterFound)
    {
        DisVerify(
            g_dis.receiveDelay >= 0,
            "Can't use negative time");
    }
    else
    {
        g_dis.receiveDelay = 200 * MILLI_SECOND;
    }

    char receiveDelayString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(g_dis.receiveDelay, receiveDelayString);

    cout << "DIS interface receive delay         = " << receiveDelayString
            << " second(s)" << endl;

    EXTERNAL_SetReceiveDelay(iface, g_dis.receiveDelay);

    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-MAX-RECEIVE-DURATION",
        &parameterFound,
        &g_dis.maxReceiveDuration);

    if (parameterFound)
    {
        DisVerify(
            g_dis.maxReceiveDuration >= 0,
            "Can't use negative time");
    }
    else
    {
        g_dis.maxReceiveDuration = 5 * MILLI_SECOND;
    }

    char maxReceiveDurationString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(
        g_dis.maxReceiveDuration, maxReceiveDurationString);

    cout << "DIS interface max receive duration  = "
            << maxReceiveDurationString << " second(s)" << endl;

    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-MOBILITY-INTERVAL",
        &parameterFound,
        &g_dis.mobilityInterval);

    if (parameterFound)
    {
        DisVerify(
            g_dis.mobilityInterval >= 0.0,
            "Can't use negative time");
    }
    else
    {
        g_dis.mobilityInterval = 500 * MILLI_SECOND;
    }

    char mobilityIntervalString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(g_dis.mobilityInterval, mobilityIntervalString);

    cout << "DIS mobility interval               = " << mobilityIntervalString
            << " second(s)" << endl;

    cout << endl;

    IO_ReadDouble(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-XYZ-EPSILON",
        &parameterFound,
        &g_dis.xyzEpsilon);

    if (parameterFound)
    {
        DisVerify(
            g_dis.xyzEpsilon >= 0.0,
            "Can't use negative epsilon value");
    }
    else
    {
        g_dis.xyzEpsilon = 0.57735026918962576450914878050196;
    }

    cout << "GCC (x,y,z) epsilons are ("
            << g_dis.xyzEpsilon << ","
            << g_dis.xyzEpsilon << ","
            << g_dis.xyzEpsilon << " meter(s))" << endl
         << "  For movement to be reflected in QualNet, change in position"
            " must be" << endl
         << "  >= any one of these values." << endl;

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-ENTITIES-FILE-PATH",
        &parameterFound,
        g_dis.entitiesPath);

    DisVerify(
        parameterFound == TRUE, "DIS-ENTITIES-FILE-PATH must be specified");

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-RADIOS-FILE-PATH",
        &parameterFound,
        g_dis.radiosPath);

    DisVerify(
        parameterFound == TRUE, "DIS-RADIOS-FILE-PATH must be specified");

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS-NETWORKS-FILE-PATH",
        &parameterFound,
        g_dis.networksPath);

    DisVerify(
        parameterFound == TRUE, "DIS-NETWORKS-FILE-PATH must be specified");

    cout << endl;
}

void
DisReadEntitiesFile(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    assert(g_dis.numEntities == 0);

    cout << "DIS entities file = " << g_dis.entitiesPath << endl;

    // Determine number of lines in file.

    g_dis.numEntities = DisGetNumLinesInFile(g_dis.entitiesPath);
    DisVerify(g_dis.numEntities > 0, "File is empty", g_dis.entitiesPath);

    DisMallocEntities();

    // Open file.

    FILE* fpEntities = fopen(g_dis.entitiesPath, "r");
    DisVerify(fpEntities != NULL, "Can't open for reading", g_dis.entitiesPath);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    unsigned numBytesCopied;
    const char* p;
    const char* next;
    unsigned i;
    unsigned lineNumber;
    for (i = 0, lineNumber = 1;
         i < g_dis.numEntities;
         i++, lineNumber++)
    {
        DisVerify(fgets(line, lineBufSize, fpEntities) != NULL,
                 "Not enough lines",
                 g_dis.entitiesPath);

        DisVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 g_dis.entitiesPath);

        next = line;

        DisEntity& entity = g_dis.entities[i];

        // Entity Marking string.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(
            p != NULL,
            "Can't read Entity Marking string",
            g_dis.entitiesPath, lineNumber);

        DisTrimLeft(token);
        DisTrimRight(token);

        DisVerify(
            strlen(token) != 0,
            "Can't read Entity Marking string",
            g_dis.entitiesPath, lineNumber);

        DisVerify(
            strlen(token) < sizeof(entity.markingString),
            "Entity Marking string too large",
            g_dis.entitiesPath, lineNumber);
        strcpy(entity.markingString, token);

        // Update Entity Marking string to entity-pointer hash.

        DisVerify(
            g_dis.markingStringToEntity
                .find(entity.markingString) == g_dis.markingStringToEntity.end(),
            "Entity with duplicate Entity Marking string",
            g_dis.entitiesPath, lineNumber);

        g_dis.markingStringToEntity[entity.markingString] = &entity;

        // ForceID (skip).

        unsigned numBytesToCopy;

        p = DisGetToken(numBytesToCopy, next, ",");
        DisVerify(
            p != NULL, "Can't read Force ID", g_dis.entitiesPath, lineNumber);

        // Country (skip).

        p = DisGetToken(numBytesToCopy, next, ",");
        DisVerify(
            p != NULL, "Can't read Country", g_dis.entitiesPath, lineNumber);

        // Geodetic position.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(
            p != NULL, "Can't read latitude", g_dis.entitiesPath, lineNumber);

        char* endPtr = NULL;
        errno = 0;
        entity.lat = strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse latitude",
            g_dis.entitiesPath, lineNumber);

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL, "Can't read longitude", g_dis.entitiesPath, lineNumber);

        errno = 0;
        entity.lon = strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse longitude",
            g_dis.entitiesPath, lineNumber);

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL, "Can't read altitude", g_dis.entitiesPath, lineNumber);

        errno = 0;
        entity.alt = strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse altitude",
            g_dis.entitiesPath, lineNumber);

        DisVerify(
            entity.lat >= -90.0 && entity.lat <= 90.0
            && entity.lon >= -180.0 && entity.lon <= 180.0,
            "Invalid geodetic coordinates",
            g_dis.entitiesPath, lineNumber);

        entity.latScheduled = entity.lat;
        entity.lonScheduled = entity.lon;
        entity.altScheduled = entity.alt;

        // Geocentric Cartesian position.

        DisConvertLatLonAltToGcc(
            entity.lat, entity.lon, entity.alt, entity.x, entity.y, entity.z);

        entity.xScheduled = entity.x;
        entity.yScheduled = entity.y;
        entity.zScheduled = entity.z;

        // Miscellaneous.

        entity.azimuth = 0;
        entity.elevation = 0;
        entity.azimuthScheduled = entity.azimuth;
        entity.elevationScheduled = entity.elevation;

        entity.speed = 0.0;
        entity.speedScheduled = entity.speed;

        entity.damage = DIS_DAMAGE_NO_DAMAGE;
        entity.hierarchyIdExists = false;
        entity.numRadioPtrs = 0;
        entity.mappedToHandle = false;

        memset(&entity.entityId, 0, sizeof(entity.entityId));
        entity.entityIdString[0] = 0;

        const clocktype neverHappenedTime = -1;
        entity.lastScheduledMobilityEventTime = neverHappenedTime;
    }//for//

    fclose(fpEntities);
}

void
DisReadRadiosFile(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    assert(g_dis.numRadios == 0);

    cout << "DIS radios file   = " << g_dis.radiosPath << endl;

    // Determine number of lines in file.

    g_dis.numRadios = DisGetNumLinesInFile(g_dis.radiosPath);
    DisVerify(g_dis.numRadios > 0, "File is empty", g_dis.radiosPath);

    DisMallocRadios();

    // Open file.

    FILE* fpRadios = fopen(g_dis.radiosPath, "r");
    DisVerify(fpRadios != NULL, "Can't open for reading", g_dis.radiosPath);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    unsigned numBytesCopied;
    const char* p;
    const char* next;
    unsigned i;
    unsigned lineNumber;
    for (i = 0, lineNumber = 1;
         i < g_dis.numRadios;
         i++, lineNumber++)
    {
        DisVerify(fgets(line, lineBufSize, fpRadios) != NULL,
                 "Not enough lines",
                 g_dis.radiosPath);

        DisVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 g_dis.radiosPath);

        next = line;

        DisRadio& radio = g_dis.radios[i];

        // Node pointer <- nodeId.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL, "Can't read nodeId", g_dis.radiosPath, lineNumber);

        char* endPtr = NULL;
        errno = 0;
        unsigned nodeId = (unsigned) strtoul(token, &endPtr, 10);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse nodeId",
            g_dis.radiosPath, lineNumber);

        unsigned j;
        for (j = 0; j < i; j++)
        {
            DisVerify(
                nodeId != g_dis.radios[j].node->nodeId,
                "Radio with duplicate nodeId",
                g_dis.radiosPath, lineNumber);
        }

        radio.node
            = MAPPING_GetNodePtrFromHash(g_dis.partitionData->nodeIdHash,
                                         nodeId);
        DisVerify(radio.node != NULL,
                  "Can't get node pointer associated with nodeId",
                  g_dis.radiosPath, lineNumber);

        // Initialize per-node DIS data for this nodeId.
        DisData &disData = g_dis.nodeIdToPerNodeData[nodeId];

        disData.radioPtr = &radio;
        disData.nextMsgId = 0;

        // Entity Marking string.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(
            p != NULL,
            "Can't read Entity Marking string",
            g_dis.radiosPath, lineNumber);

        DisTrimLeft(token);
        DisTrimRight(token);

        DisVerify(
            strlen(token) != 0,
            "Can't read Entity Marking string",
            g_dis.radiosPath, lineNumber);

        DisVerify(
            strlen(token) < g_disMarkingStringBufSize,
            "MarkingData too large",
            g_dis.radiosPath, lineNumber);

        // Assign radio's entity pointer using Entity Marking string.

        DisMarkingStringToEntityMap::iterator it = g_dis.markingStringToEntity.find(token);

        DisVerify(
            it != g_dis.markingStringToEntity.end(),
            "Can't find entity with Entity Marking string",
            g_dis.radiosPath, lineNumber);

        DisEntity *entity = it->second;

        radio.entityPtr = entity;

        // Assign radio pointer to host entity.

        DisVerify(entity->numRadioPtrs < g_disMaxRadiosPerEntity,
                  "Exceeded max radios per entity");

        entity->radioPtrs[entity->numRadioPtrs] = &radio;
        entity->numRadioPtrs++;

        // Radio ID.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(
            p != NULL, "Can't read Radio ID", g_dis.radiosPath, lineNumber);

        errno = 0;
        radio.radioId = (unsigned short) strtoul(token, &endPtr, 10);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse Radio ID", g_dis.radiosPath, lineNumber);

        // Entity Marking string + Radio ID to radio pointer hash.

        DisRadioKey radioKey;
        strcpy(radioKey.markingString, entity->markingString);
        radioKey.radioId = radio.radioId;

        DisVerify(
            g_dis.radioKeyToRadio.find(radioKey) == g_dis.radioKeyToRadio.end(),
            "Radio with duplicate Entity Marking string and Radio ID",
            g_dis.radiosPath, lineNumber);

        g_dis.radioKeyToRadio[radioKey] = &radio;

        // Relative Antenna Location.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL,
                 "Can't read Relative Antenna Location (x)",
                 g_dis.radiosPath, lineNumber);

        errno = 0;
        radio.relativeAntennaLocationX = (float) strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse Relative Antenna Location (x)",
            g_dis.radiosPath,
            lineNumber);

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL,
                 "Can't read Relative Antenna Location (y)",
                 g_dis.radiosPath, lineNumber);

        errno = 0;
        radio.relativeAntennaLocationY = (float) strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse Relative Antenna Location (y)",
            g_dis.radiosPath,
            lineNumber);

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ",");
        DisVerify(p != NULL,
                 "Can't read Relative Antenna Location (z)",
                 g_dis.radiosPath, lineNumber);

        errno = 0;
        radio.relativeAntennaLocationZ = (float) strtod(token, &endPtr);
        DisVerify(
            endPtr != token && errno == 0,
            "Can't parse Relative Antenna Location (z)",
            g_dis.radiosPath,
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

        double radioX = entity->x + radio.relativeAntennaLocationX;
        double radioY = entity->y - radio.relativeAntennaLocationY;
        double radioZ = entity->z - radio.relativeAntennaLocationZ;

        DisConvertGccToLatLonAlt(
            radioX,
            radioY,
            radioZ,
            radio.latScheduled,
            radio.lonScheduled,
            radio.altScheduled);

        // Miscellaneous.

        radio.txState = DIS_TX_STATE_ON_BUT_NOT_TRANSMITTING;

        if (radio.node->numberPhys > 0)
        {
            radio.usesTxPower = true;

            PHY_GetTransmitPower(radio.node, 0, &radio.maxTxPower);
            radio.currentMaxTxPower = radio.maxTxPower;
        }
        else
        {
            radio.usesTxPower = false;
        }

        radio.mappedToEntityMarkingString = false;
        radio.mappedToHandle = false;
        radio.networkPtr = NULL;
        radio.defaultDstRadioPtr = NULL;

        // Make radio data accessible from node pointer.

        //DisData& disData = *((DisData*) radio.node->appData.disData);
        //disData.radioPtr = &radio;
    }//for//
}

void
DisReadNetworksFile(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    assert(g_dis.numNetworks == 0);

    cout << "DIS networks file = " << g_dis.networksPath << "." << endl;

    // Determine number of lines in file.

    g_dis.numNetworks = DisGetNumLinesInFile(g_dis.networksPath);
    DisVerify(g_dis.numNetworks > 0, "File is empty", g_dis.networksPath);

    DisMallocNetworks();

    // Open file.

    FILE* fpNetworks = fopen(g_dis.networksPath, "r");
    DisVerify(
        fpNetworks != NULL, "Can't open for reading", g_dis.networksPath);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    char nodeIdToken[lineBufSize];
    unsigned numBytesCopied;
    const char* p;
    const char* next;
    const char* nextNodeIdToken;
    unsigned i;
    unsigned lineNumber = 0;
    for (i = 0, lineNumber = 1;
         i < g_dis.numNetworks;
         i++, lineNumber++)
    {
        DisVerify(fgets(line, lineBufSize, fpNetworks) != NULL,
                  "Not enough lines",
                  g_dis.networksPath);

        DisVerify(strlen(line) < lineBufSize - 1,
                  "Exceeds permitted line length",
                  g_dis.networksPath);

        next = line;

        DisNetwork& network = g_dis.networks[i];

        // Name of network.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ";");
        DisVerify(
            p != NULL,
            "Can't read network name",
            g_dis.networksPath,
            lineNumber);

        DisVerify(
            strlen(token) < sizeof(network.name),
            "Network name too long",
            g_dis.networksPath, lineNumber);

        assert(sizeof(network.name) <= sizeof(token));

        strcpy(network.name, token);

        // Frequency.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ";");
        DisVerify(
            p != NULL,
            "Can't read network frequency",
            g_dis.networksPath,
            lineNumber);

        int retVal = atouint64(token, &network.frequency);
        DisVerify(
            retVal == 1,
            "Can't parse network frequency",
            g_dis.networksPath,
            lineNumber);

        // nodeIds.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ";");
        DisVerify(
            p != NULL,
            "Can't read list of nodeIds",
            g_dis.networksPath,
            lineNumber);

        network.numRadioPtrs = 0;

        nextNodeIdToken = token;
        while (DisGetToken(nodeIdToken,
                           sizeof(nodeIdToken),
                           numBytesCopied,
                           nextNodeIdToken, ","))
        {
            DisVerify(
                network.numRadioPtrs < g_disMaxMembersInNetwork,
                "Exceeded maximum number of nodes for network",
                g_dis.networksPath, lineNumber);

            errno = 0;
            unsigned nodeId = (unsigned) strtoul(nodeIdToken, NULL, 10);
            DisVerify(errno == 0,
                     "Couldn't parse nodeId",
                     g_dis.networksPath, lineNumber);

            assert(g_dis.numRadios > 0);
            unsigned j;
            for (j = 0; j < g_dis.numRadios; j++)
            {
                DisRadio& radio = g_dis.radios[j];
                if (radio.node->nodeId == nodeId) { break; }
            }

            DisVerify(
                j != g_dis.numRadios,
                "Can't find nodeId in list of radios",
                g_dis.networksPath, lineNumber);

            DisRadio& radio = g_dis.radios[j];

            network.radioPtrs[network.numRadioPtrs] = &radio;

            network.numRadioPtrs++;

            DisVerify(
                radio.networkPtr == NULL,
                "nodeId present in more than one network",
                g_dis.networksPath, lineNumber);

            radio.networkPtr = &network;
        }

        DisVerify(network.numRadioPtrs > 0,
                 "Network must have at least one node",
                 g_dis.networksPath, lineNumber);

        // IP address.

        p = DisGetToken(token, sizeof(token), numBytesCopied, next, ";\n\r");
        DisVerify(p != NULL,
                 "Can't read IP address",
                 g_dis.networksPath, lineNumber);

        int numHostBits;
        BOOL isNodeId;

        IO_ParseNodeIdHostOrNetworkAddress(
            token,
            &network.ipAddress,
            &numHostBits,
            &isNodeId);

        DisVerify(
            !isNodeId,
            "Can't parse IP address",
            g_dis.networksPath,
            lineNumber);

        network.unicast = (network.ipAddress == 0);
    }//for//
}

void
DisMapHierarchyIds(NodeInput* nodeInput)
{
    // Fill hash table of nodeId -> hierarchy IDs.

    assert(nodeInput->numLines >= 0);

    unsigned i;
    unsigned lineNumber;
    for (i = 0, lineNumber = 1;
         i < (unsigned) nodeInput->numLines;
         i++, lineNumber++)
    {
        const char* valueString = nodeInput->values[i];

        if (strcmp(nodeInput->variableNames[i], "COMPONENT") != 0)
        { continue;  }

        char* endPtr = NULL;
        errno = 0;
        int hierarchyId = (int) strtol(valueString, &endPtr, 10);
        DisVerify(
            endPtr != valueString && errno == 0,
            "Can't parse hierarchy ID",
            ".config file", lineNumber);

        if (hierarchyId == 0) { continue; }

        const char* p = strchr(valueString, '{');

        DisVerify(
            p != NULL,
            "Invalid COMPONENT value",
            ".config file", lineNumber);

        const char* next = p + 1;  // Points to byte after '{'.

        char nodeIdList[g_disLineBufSize];
        unsigned numBytesCopied;

        p = DisGetToken(
                nodeIdList, sizeof(nodeIdList), numBytesCopied, next, "}");

        DisVerify(
            p != NULL,
            "Invalid COMPONENT value",
            ".config file", lineNumber);

        if (strlen(nodeIdList) == 0) { continue; }

        DisVerify(
            numBytesCopied < sizeof(nodeIdList) - 1,
            "COMPONENT line too long",
            ".config file", lineNumber);

        char nodeIdString[g_disLineBufSize];

        next = nodeIdList;

        while (1)
        {
            p = DisGetAnyToken(
                    nodeIdString,
                    sizeof(nodeIdString),
                    numBytesCopied,
                    next,
                    " \t,");

            if (p == NULL) { break; }

            DisVerify(
                numBytesCopied < sizeof(nodeIdString) - 1,
                "nodeId string too long",
                ".config file", lineNumber);

            // Ignore Subnets, Hierarchies etc.
            if (!isdigit(nodeIdString[0]))
            {
                continue;
            }

            errno = 0;
            unsigned nodeId = (unsigned) strtoul(nodeIdString, &endPtr, 10);
            DisVerify(
                endPtr != nodeIdString && errno == 0,
                "Can't parse nodeId",
                ".config file", lineNumber);

            DisVerify(
                g_dis.nodeIdToHierarchyId.find(nodeId) == g_dis.nodeIdToHierarchyId.end(),
                "nodeId belongs to more than one hierachy",
                ".config file", lineNumber);

            g_dis.nodeIdToHierarchyId[nodeId] = hierarchyId;
        }//while//
    }//for//

    for (i = 0; i < g_dis.numEntities; i++)
    {
        assert(g_dis.entities != NULL);
        DisEntity& entity = g_dis.entities[i];

        assert(!entity.hierarchyIdExists);

        unsigned j;
        for (j = 0; j < entity.numRadioPtrs; j++)
        {
            assert(entity.radioPtrs[j] != NULL);
            const DisRadio& radio = *entity.radioPtrs[j];

            assert(radio.node != NULL);

            DisNodeIdToHierarchyIdMap::iterator it
                = g_dis.nodeIdToHierarchyId.find(radio.node->nodeId);

            if (it != g_dis.nodeIdToHierarchyId.end())
            {
                entity.hierarchyIdExists = true;
                entity.hierarchyId = it->second;

                break;
            }
        }//for//
    }//for//
}

void
DisInitMessenger(EXTERNAL_Interface* iface)
{
    Node* node = iface->partition->firstNode;

    while (node)
    {
        MessengerInit(node);
        node = node->nextNodeData;
    }
}

void
DisWaitForFirstEntityStatePdu(EXTERNAL_Interface* iface)
{
    cout << "Waiting for first Entity State PDU ... ";
    cout.flush();

    bool firstEntityStatePduReceived = false;

    char payload[g_maxUdpPayloadSize];
    unsigned payloadSize;

    while (!firstEntityStatePduReceived)
    {
        if (!DisAttemptRecv(
                 g_dis.sd,
                 payload,
                 sizeof(payload),
                 payloadSize))
        {
            // recv() reported no pending UDP datagrams, so sleep for 100
            // milliseconds and listen again.
            // This sleep time is actually occurring before QualNet simulation
            // time has begun increasing.
            // Sleeping here also allows time for the QualNet GUI to set up the
            // scenario.

            DisUsleep(100000);
            continue;
        }

        // Verify that the entire payload consists of one or more DIS PDUs
        // with a Protocol Version field indicating DIS 2.0.3 through
        // 2.0.6.
        // Use the Length field in the PDU Header record to help determine
        // whether there's extraneous/insufficient data.

        // Discard the entire payload if the above requirements aren't met.

        if (payloadSize > 0
            && DisPayloadContainsOnlyPdus(payload, payloadSize))
        {
            // Init an index to the current PDU.  Start at byte 0.
            // Declare a length variable for the current PDU.

            unsigned index = 0;
            unsigned short length;

            while (1)
            {
               length = DisGetPduLength(&payload[index]);

               // Only process PDUs received after the first Entity State PDU
               // is received.  (The first Entity State PDU is also processed.)

               if (!firstEntityStatePduReceived)
               {
                   if ((!g_dis.filterOnExerciseIdentifier
                        || DisGetExerciseIdentifier(&payload[index])
                             == g_dis.exerciseIdentifier)
                       && DisGetPduType(&payload[index])
                            == DIS_ENTITY_STATE_PDU)
                   {
                       firstEntityStatePduReceived = true;

                       cout << "received." << endl;
                       cout.flush();

                       DisProcessPdu(&payload[index], length);
                   }
               }
               else
               {
                   DisProcessPdu(&payload[index], length);
               }

               index += length;

               if (index == payloadSize) { break; }

               assert(index < payloadSize);
            }//while//
        }//if//
    }//while//
}

void
DisRegisterCtrlCHandler()
{
    signal(SIGINT, DisHandleSigInt);

    cout << "DIS: Press Ctrl-C to end program." << endl;
}

void
DisHandleSigInt(int a)
{
    // Ctrl-C pressed.

    cout << endl << "Ending program ... " << endl;

    // TODO:  Unverified method of ending simulation early, but it appears to
    // work.

    g_dis.partitionData->maxSimClock = g_dis.partitionData->theCurrentTime;
}


void
DisProcessPdu(const char* pduBuf, unsigned short length)
{
    if (g_dis.filterOnExerciseIdentifier
        && DisGetExerciseIdentifier(pduBuf) != g_dis.exerciseIdentifier)
    {
        // Don't process this PDU when we're filtering on Exercise Identifier
        // and it's not a match.

        return;
    }

    if (g_dis.debugPrintPdus)
    {
        DisPrintHex(pduBuf, length);
        cout << "--" << endl;
    }

    switch (DisGetPduType(pduBuf))
    {
        case DIS_ENTITY_STATE_PDU:
            DisProcessEntityStatePdu(pduBuf, length);
            break;
        case DIS_TRANSMITTER_PDU:
            DisProcessTransmitterPdu(pduBuf, length);
            break;
        case DIS_SIGNAL_PDU:
            DisProcessSignalPdu(pduBuf, length);
            break;
        default:
            // Don't do anything for other PDU types.
            ;
    }//switch//
}

void
DisProcessEntityStatePdu(const char* pduBuf, unsigned short length)
{
    if (length < sizeof(DisEntityStatePdu))
    {
        DisReportWarning("Entity State PDU too short");
        return;
    }

    DisEntityStatePdu pdu;

    // Ignore the optional Articular Parameters record, if any.

    memcpy(&pdu, pduBuf, sizeof(pdu));

    // Perform network-to-host byte-swapping.

    DisNtohEntityStatePdu(pdu);

    // Retrieve entity pointer using Entity Marking string.
    // Return early if no entity could be found.

    std::string marking((char *)(pdu.entityMarking+1));
    DisMarkingStringToEntityMap::iterator it =
        g_dis.markingStringToEntity.find(marking);

    if (it == g_dis.markingStringToEntity.end())
    {
        return;
    }

    // Update Entity ID to entity-pointer hash table.
    // need to check for a memory leak when a new entity is saved over a previous entity
    // who should delete the entity?
    DisEntity* entity = it->second;
    g_dis.entityIdToEntity[pdu.entityId] = entity;

    // Assign or re-assign a different Entity ID to the entity data structure.
    // (A re-assignment of a different Entity ID may occur if an entity was
    // deleted and another entity with the same Entity Marking string but a
    // different Entity ID was created.)

    if (entity->entityId != pdu.entityId)
    {
        entity->entityId = pdu.entityId;
        DisMakeEntityIdString(entity->entityId, entity->entityIdString);
    }

    DisProcessMobilityFields(pdu, *entity);
    DisScheduleMobilityEventIfNecessary(*entity);

    DisProcessDamageFields(pdu, *entity);
}

void
DisProcessMobilityFields(DisEntityStatePdu &pdu, DisEntity& entity)
{
    // Entity Location.

    if (pdu.entityLocation[0] != entity.x
        || pdu.entityLocation[1] != entity.y
        || pdu.entityLocation[2] != entity.z)
    {
        double lat;
        double lon;
        double alt;

        DisConvertGccToLatLonAlt(
            pdu.entityLocation[0],
            pdu.entityLocation[1],
            pdu.entityLocation[2],
            lat,
            lon,
            alt);

        if (lat >= g_dis.minLat && lat < g_dis.maxLat
            && lon >= g_dis.minLon && lon <= g_dis.maxLon)
        {
            entity.lat = lat;
            entity.lon = lon;
            entity.alt = alt;

            entity.x = pdu.entityLocation[0];
            entity.y = pdu.entityLocation[1];
            entity.z = pdu.entityLocation[2];
        }
    }//if//

    // Entity Orientation.

    DisConvertDisOrientationToQualNetOrientation(
        entity.lat,
        entity.lon,
        pdu.entityOrientation[0],
        pdu.entityOrientation[1],
        pdu.entityOrientation[2],
        entity.azimuth,
        entity.elevation);

    // Entity Linear Velocity.

    entity.speed = sqrt(pow(pdu.entityLinearVelocity[0], 2)
                        + pow(pdu.entityLinearVelocity[1], 2)
                        + pow(pdu.entityLinearVelocity[2], 2));
}

void
DisScheduleMobilityEventIfNecessary(DisEntity& entity)
{
    clocktype simTime = g_dis.partitionData->theCurrentTime;

    clocktype mobilityEventTime;

    if (g_dis.iface->horizon == 0)
    {
        // During initialization schedule hierarchy mobility at 1ns and node mobility at 2ns
        mobilityEventTime = 2 * NANO_SECOND;
    }
    else
    {
        mobilityEventTime = MAX(EXTERNAL_QueryExternalTime(g_dis.iface), simTime + 1);
    }

    if (entity.lastScheduledMobilityEventTime > 0 &&
        mobilityEventTime < entity.lastScheduledMobilityEventTime + g_dis.mobilityInterval)
    {
        return;
    }

    bool scheduleLocation      = false;
    bool scheduleOrientation   = false;
    bool scheduleSpeed         = false;

    if (fabs(entity.x - entity.xScheduled) >= g_dis.xyzEpsilon
        || fabs(entity.y - entity.yScheduled) >= g_dis.xyzEpsilon
        || fabs(entity.z - entity.zScheduled) >= g_dis.xyzEpsilon)
    { scheduleLocation = true; }

    if (entity.azimuth != entity.azimuthScheduled
        || entity.elevation != entity.elevationScheduled)
    { scheduleOrientation = true; }

    if (entity.speed != entity.speedScheduled)
    { scheduleSpeed = true; }

    if (!scheduleLocation && !scheduleOrientation && !scheduleSpeed)
    { return; }

    double entityX;
    double entityY;
    double entityZ;

    double entityLat;
    double entityLon;
    double entityAlt;

    if (scheduleLocation)
    {
        entityX = entity.x;
        entityY = entity.y;
        entityZ = entity.z;

        entityLat = entity.lat;
        entityLon = entity.lon;
        entityAlt = entity.alt;
    }
    else
    {
        entityX = entity.xScheduled;
        entityY = entity.yScheduled;
        entityZ = entity.zScheduled;

        entityLat = entity.latScheduled;
        entityLon = entity.lonScheduled;
        entityAlt = entity.altScheduled;
    }

    Orientation orientation;

    if (scheduleOrientation)
    {
        orientation.azimuth   = entity.azimuth;
        orientation.elevation = entity.elevation;
    }
    else
    {
        orientation.azimuth   = entity.azimuthScheduled;
        orientation.elevation = entity.elevationScheduled;
    }

    double speed;

    if (scheduleSpeed)
    {
        speed = entity.speed;
    }
    else
    {
        speed = entity.speedScheduled;
    }

    // Move entity (hierarchy).

    if (g_dis.partitionData->guiOption
        && entity.hierarchyIdExists
        && (scheduleOrientation || scheduleLocation))
    {
        Node* node;

        if (entity.numRadioPtrs > 0)
        {
            DisRadio& radio = *entity.radioPtrs[0];
            node = radio.node;
        }
        else
        {
            node = g_dis.partitionData->firstNode;
        }

        Coordinates coordinates;
        coordinates.latlonalt.latitude  = entityLat;
        coordinates.latlonalt.longitude = entityLon;
        coordinates.latlonalt.altitude  = entityAlt;

        Message* msg = MESSAGE_Alloc(node,
                                     EXTERNAL_LAYER,
                                     EXTERNAL_DIS,
                                     MSG_EXTERNAL_DIS_HierarchyMobility);

        MESSAGE_InfoAlloc(node, msg, sizeof(DisHierarchyMobilityInfo));
        DisHierarchyMobilityInfo& info
            = *((DisHierarchyMobilityInfo*) MESSAGE_ReturnInfo(msg));
        info.hierarchyId = entity.hierarchyId;
        info.coordinates = coordinates;
        info.orientation = orientation;

        // Execute the hierarchy move one nanosecond before the node move
        // time.  This is a workaround for a GUI issue where the screen will
        // get all messed up if the GUI command to move the node is sent
        // before the GUI command to move the hierarchy -- even if both GUI
        // commands are sent with the same timestamp.

        clocktype delay = (mobilityEventTime - 1) - simTime;
        assert(delay >= 0);

        MESSAGE_Send(node, msg, delay);
    }//if//

    // Move radios (nodes).

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        DisRadio& radio = *entity.radioPtrs[i];

        // Schedule QualNet node movement.

        Coordinates coordinates;

        double radioLat;
        double radioLon;
        double radioAlt;

        if (scheduleLocation)
        {
            // When calculating positions for hosted radios, QualNet assumes
            // the host entity is pointed in the same direction as the GCC
            // x-axis, with no roll with respect to the earth's equatorial
            // plane, and with the top of the entity facing the same direction
            // as the north pole.
            //
            // Relative positions for the entity y- and z-axes are flipped with
            // respect to the GCC y- and z-axes.

            double radioX = entityX + radio.relativeAntennaLocationX;
            double radioY = entityY - radio.relativeAntennaLocationY;
            double radioZ = entityZ - radio.relativeAntennaLocationZ;

            DisConvertGccToLatLonAlt(
                radioX, radioY, radioZ, radioLat, radioLon, radioAlt);

            coordinates.latlonalt.latitude  = radioLat;
            coordinates.latlonalt.longitude = radioLon;
            coordinates.latlonalt.altitude  = radioAlt;
        }
        else
        {
            coordinates.latlonalt.latitude  = radio.latScheduled;
            coordinates.latlonalt.longitude = radio.lonScheduled;
            coordinates.latlonalt.altitude  = radio.altScheduled;
        }

        EXTERNAL_ChangeNodePositionOrientationAndSpeedAtTime(
            g_dis.iface,
            radio.node,
            mobilityEventTime,
            coordinates.latlonalt.latitude,
            coordinates.latlonalt.longitude,
            coordinates.latlonalt.altitude,
            orientation.azimuth,
            orientation.elevation,
            speed);

        if (scheduleLocation)
        {
            radio.latScheduled = radioLat;
            radio.lonScheduled = radioLon;
            radio.altScheduled = radioAlt;
        }
    }//for//

    if (scheduleLocation)
    {
        entity.xScheduled = entity.x;
        entity.yScheduled = entity.y;
        entity.zScheduled = entity.z;

        entity.latScheduled = entity.lat;
        entity.lonScheduled = entity.lon;
        entity.altScheduled = entity.alt;
    }

    if (scheduleOrientation)
    {
        entity.azimuthScheduled   = entity.azimuth;
        entity.elevationScheduled = entity.elevation;
    }

    if (scheduleSpeed)
    {
        entity.speedScheduled = entity.speed;
    }

    entity.lastScheduledMobilityEventTime = mobilityEventTime;

    if (g_dis.debugPrintMobility)
    {
        double entityAltDisplayed = entityAlt;

        if (entityAlt < 0 && entityAlt > -0.999999)
        { entityAltDisplayed = 0.0; }

        double dbl_mobilityEventTime
            = DisConvertClocktypeToDouble(mobilityEventTime);

        printf(
            "DIS: Moving %s to (%.4f,%.4f,%.0f) (%u,%u) (%u),"
            " time %.3f\n",
            entity.markingString,
            entityLat,
            entityLon,
            entityAltDisplayed,
            orientation.azimuth,
            orientation.elevation,
            (unsigned) speed,
            dbl_mobilityEventTime);
    }
}

void
DisProcessDamageFields(DisEntityStatePdu& pdu, DisEntity& entity)
{
    unsigned damage = (((unsigned) pdu.entityAppearance) & 24) >> 3;

    DisVerify(damage >= DIS_DAMAGE_MIN
              && damage <= DIS_DAMAGE_MAX,
              "Unexpected attribute value",
              __FILE__, __LINE__);

    assert(entity.damage >= DIS_DAMAGE_MIN
           && entity.damage <= DIS_DAMAGE_MAX);

    if (entity.damage == damage) { return; }

    if (g_dis.debugPrintDamage)
    {
        cout << "DIS: " << entity.markingString << " ("
                << entity.entityIdString << ") Damage = ";

        switch (damage)
        {
            case DIS_DAMAGE_NO_DAMAGE:
                cout << "No damage";
                break;
            case DIS_DAMAGE_SLIGHT_DAMAGE:
                cout << "Slight damage";
                break;
            case DIS_DAMAGE_MODERATE_DAMAGE:
                cout << "Moderate damage";
                break;
            case DIS_DAMAGE_DESTROYED:
                cout << "Destroyed";
                break;
            default:
                assert(0);
        }

        cout << endl;
    }//if//

    if (entity.numRadioPtrs > 0)
    {
        // Get node pointer and delay.

        assert(entity.radioPtrs[0] != NULL);
        const DisRadio& radio = *entity.radioPtrs[0];

        Node* node = radio.node;

        clocktype simTime = TIME_getSimTime(node);
        clocktype delay;

        if (g_dis.iface->horizon == 0)
        {
            // During initialization schedule interface changes at 1ns
            delay = 1 * NANO_SECOND;
        }
        else
        {
            delay = EXTERNAL_QueryExternalTime(g_dis.iface) - simTime;
        }

        // Turn off/on interfaces.

        if (entity.damage != DIS_DAMAGE_DESTROYED
            && damage == DIS_DAMAGE_DESTROYED)
        {
            DisScheduleChangeInterfaceState(entity, false, delay);
        }
        else
        if (entity.damage == DIS_DAMAGE_DESTROYED
            && damage != DIS_DAMAGE_DESTROYED)
        {
            DisScheduleChangeInterfaceState(entity, true, delay);
        }

        // Schedule check for reducing TX power.

        if (entity.damage < DIS_DAMAGE_SLIGHT_DAMAGE
            && damage == DIS_DAMAGE_SLIGHT_DAMAGE)
        {
            // Less-damaged state -> Slight damage.

            DisScheduleChangeMaxTxPower(
                node, entity, DIS_DAMAGE_SLIGHT_DAMAGE, delay);
        }
        else
        if (entity.damage < DIS_DAMAGE_MODERATE_DAMAGE
            && damage == DIS_DAMAGE_MODERATE_DAMAGE)
        {
            // Less-damaged state -> Moderate damage.

            DisScheduleChangeMaxTxPower(
                node, entity, DIS_DAMAGE_MODERATE_DAMAGE, delay);
        }
        else
        if (entity.damage < DIS_DAMAGE_DESTROYED
            && damage == DIS_DAMAGE_DESTROYED)
        {
            // Less-damaged state -> Destroyed.

            DisScheduleChangeMaxTxPower(
                node, entity, DIS_DAMAGE_DESTROYED, delay);
        }
        else
        if (entity.damage != DIS_DAMAGE_NO_DAMAGE
            && damage == DIS_DAMAGE_NO_DAMAGE)
        {
            // More-damaged state -> No damage.

            DisScheduleChangeMaxTxPower(
                node, entity, DIS_DAMAGE_NO_DAMAGE, delay);
        }
        else
        if (entity.damage > damage)
        {
            // More-damaged state -> Less-damaged state (but not NoDamage).
            // Don't change max TX power.
        }
        else
        {
            assert(0);
        }
    }//if//

    entity.damage = damage;
}

void
DisScheduleChangeInterfaceState(
    const DisEntity& entity,
    bool enableInterfaces,
    clocktype delay)
{
    int eventType;
    if (enableInterfaces) { eventType = MSG_MAC_EndFault; }
    else { eventType = MSG_MAC_StartFault; }

    assert(entity.numRadioPtrs > 0);

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        assert(entity.radioPtrs[i] != NULL);
        const DisRadio& radio = *entity.radioPtrs[i];

        if (enableInterfaces
            && radio.txState == DIS_TX_STATE_OFF)
        { continue; }

        Node* node = radio.node;

        unsigned interfaceIndex;
        for (interfaceIndex = 0;
             interfaceIndex < (unsigned) node->numberInterfaces;
             interfaceIndex++)
        {
            Message* msg = MESSAGE_Alloc(node,
                                         MAC_LAYER,
                                         0,
                                         eventType);

            MESSAGE_SetInstanceId(msg, (short) interfaceIndex);

            MESSAGE_InfoAlloc(node, msg, sizeof(FaultType));
            FaultType& info = *((FaultType*) MESSAGE_ReturnInfo(msg));
            info = STATIC_FAULT;

            MESSAGE_Send(node, msg, delay);
        }//for//
    }//for//
}

void
DisScheduleChangeInterfaceState(
    const DisRadio& radio, bool enableInterface)
{
    int eventType;
    if (enableInterface) { eventType = MSG_MAC_EndFault; }
    else { eventType = MSG_MAC_StartFault; }

    clocktype simTime = TIME_getSimTime(radio.node);
    clocktype delay;

    if (g_dis.iface->horizon == 0)
    {
        // During initialization schedule interface changes at 1ns
        delay = 1 * NANO_SECOND;
    }
    else
    {
        delay = EXTERNAL_QueryExternalTime(g_dis.iface) - simTime;
    }

    Node* node = radio.node;

    unsigned interfaceIndex;
    for (interfaceIndex = 0;
         interfaceIndex < (unsigned) node->numberInterfaces;
         interfaceIndex++)
    {
        Message* msg = MESSAGE_Alloc(node,
                                     MAC_LAYER,
                                     0,
                                     eventType);

        MESSAGE_SetInstanceId(msg, (short) interfaceIndex);

        MESSAGE_InfoAlloc(node, msg, sizeof(FaultType));
        FaultType& info = *((FaultType*) MESSAGE_ReturnInfo(msg));
        info = STATIC_FAULT;

        MESSAGE_Send(node, msg, delay);
    }//for//
}

void
DisScheduleChangeMaxTxPower(
    Node *node,
    const DisEntity& entity,
    unsigned damage,
    clocktype delay)
{
    Message* msg = MESSAGE_Alloc(node,
                                 EXTERNAL_LAYER,
                                 EXTERNAL_DIS,
                                 MSG_EXTERNAL_DIS_ChangeMaxTxPower);
    MESSAGE_InfoAlloc(node, msg, sizeof(DisChangeMaxTxPowerInfo));
    DisChangeMaxTxPowerInfo& info
        = *((DisChangeMaxTxPowerInfo*) MESSAGE_ReturnInfo(msg));
    info.entityPtr = &entity;
    info.damage = damage;

    MESSAGE_Send(node, msg, delay);
}

void
DisProcessHierarchyMobilityEvent(Node* node, Message* msg)
{
    DisHierarchyMobilityInfo& info
        = *((DisHierarchyMobilityInfo*) MESSAGE_ReturnInfo(msg));

    GUI_MoveHierarchy(
        info.hierarchyId,
        info.coordinates,
        info.orientation,
        TIME_getSimTime(node));
}

void
DisProcessChangeMaxTxPowerEvent(Node* node, Message* msg)
{
    DisChangeMaxTxPowerInfo& info
        = *((DisChangeMaxTxPowerInfo*) MESSAGE_ReturnInfo(msg));

    const  DisEntity& entity = *info.entityPtr;

    bool   restoreMaxTxPower = false;
    double probability;
    double maxFraction;

    switch (info.damage)
    {
        case DIS_DAMAGE_NO_DAMAGE:
            restoreMaxTxPower = true;
            break;
        case DIS_DAMAGE_SLIGHT_DAMAGE:
            probability
                = g_dis.slightDamageReduceTxPowerProbability;
            maxFraction
                = g_dis.slightDamageReduceTxPowerMaxFraction;
            break;
        case DIS_DAMAGE_MODERATE_DAMAGE:
            probability
                = g_dis.moderateDamageReduceTxPowerProbability;
            maxFraction
                = g_dis.moderateDamageReduceTxPowerMaxFraction;
            break;
        case DIS_DAMAGE_DESTROYED:
            probability
                = g_dis.destroyedReduceTxPowerProbability;
            maxFraction
                = g_dis.destroyedReduceTxPowerMaxFraction;
            break;
        default:
            assert(0);
    }

    assert(entity.numRadioPtrs > 0);

    unsigned i;
    for (i = 0; i < entity.numRadioPtrs; i++)
    {
        assert(entity.radioPtrs[i] != NULL);
        DisRadio& radio = *entity.radioPtrs[i];

        if (!radio.usesTxPower) { continue; }

        Node* node = radio.node;

        double newTxPower;

        if (restoreMaxTxPower)
        {
            newTxPower = radio.maxTxPower;
        }
        else
        if (RANDOM_erand(g_dis.seed) > probability)
        {
            continue;
        }
        else
        {
            newTxPower = radio.currentMaxTxPower
                         * (maxFraction * RANDOM_erand(g_dis.seed));
        }

        if (g_dis.debugPrintTxPower)
        {
            cout << "DIS: " << entity.markingString << " ("
                    << node->nodeId << ","
                    << radio.radioId << ")"
                    << " Old TX power = " << radio.currentMaxTxPower
                    << " New TX power = " << newTxPower << " mW" << endl;
        }

        const int phyIndex = 0;

        PHY_SetTransmitPower(
            node,
            phyIndex,
            newTxPower);

        radio.currentMaxTxPower = newTxPower;
    }//for//
}

void
DisProcessTransmitterPdu(const char* pduBuf, unsigned short length)
{
    // In this function, the Transmitter PDU will be silently dropped unless
    // the corresponding Entity State PDU has been received first.  This is a
    // QualNet quirk, which would take some effort to update.

    if (length < sizeof(DisTransmitterPdu))
    {
        DisReportWarning("Transmitter PDU too short");
        return;
    }

    // Any variable-length part of the Transmitter PDU is not copied
    // (the variable part is not currently used).

    DisTransmitterPdu pdu;
    memcpy(&pdu, pduBuf, sizeof(pdu));

    DisNtohTransmitterPdu(pdu);

    if (g_dis.debugPrintTransmitterPdu)
    {
        char entityIdString[g_disEntityIdStringBufSize];
        DisMakeEntityIdString(pdu.entityId, entityIdString);

        cout << "DIS: Transmitter PDU (" << entityIdString << ", "
                << pdu.radioId << ")" << endl;
    }

    DisRadio* radioPtr = DisGetRadioPtr(pdu.entityId, pdu.radioId);

    if (radioPtr == NULL) { return;}
    DisRadio& radio = *radioPtr;

    if (g_dis.debugPrintMapping && !radio.mappedToEntityMarkingString)
    {
        const DisEntity& entity = *radio.entityPtr;

        cout << "DIS: Assigned " << entity.markingString << " ("
                << radio.node->nodeId << ","
                << radio.radioId
                << ") to " << entity.entityIdString << endl;
    }

    if (!radio.mappedToEntityMarkingString)
    {
#ifdef MILITARY_RADIOS_LIB
        if (radio.node->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK11
            || radio.node->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK16)
        {
            // Schedule the RTSS notification to be sent 1 nanosecond from
            // the current simulation time.
            // Why 1 nanosecond and not 0?
            // For a delay of 0, the RTSS will be sent immediately, i.e.,
            // while still executing a reflectAttributeValues() callback,
            // which is illegal in HLA.
            // (The DIS code is left this way to maintain consistency.)

            clocktype delay = 1 * NANO_SECOND;

            DisScheduleRtssNotification(radio, delay);
        }
#endif /* MILITARY_RADIOS_LIB */

        radio.mappedToEntityMarkingString = true;
    }

    DisProcessTransmitStateField(pdu, radio);
}

void
DisProcessTransmitStateField(const DisTransmitterPdu& pdu, DisRadio& radio)
{
    unsigned char newTxState = pdu.transmitState;

    assert(DIS_TX_STATE_MIN == 0);

    DisVerify(newTxState <= DIS_TX_STATE_MAX,
              "Unexpected field value",
              __FILE__, __LINE__);

    assert(radio.txState <= DIS_TX_STATE_MAX);

    if (newTxState == radio.txState) { return; }

    assert(radio.entityPtr != NULL);
    const DisEntity& entity = *radio.entityPtr;

    if (g_dis.debugPrintTxState)
    {
        cout << "DIS: " << entity.markingString << " ("
                << radio.node->nodeId << ","
                << radio.radioId
                << ") Transmit State = ";

        switch (newTxState)
        {
            case DIS_TX_STATE_OFF:
                cout << "Off";
                break;
            case DIS_TX_STATE_ON_BUT_NOT_TRANSMITTING:
                cout << "On but not transmitting";
                break;
            case DIS_TX_STATE_ON_AND_TRANSMITTING:
                cout << "On and transmitting";
                break;
            default:
                assert(0);
        }

        cout << endl;
    }

    if (newTxState != DIS_TX_STATE_OFF
        && radio.txState != DIS_TX_STATE_OFF)
    {
        // Radio was on, and is still on, but just a different mode.
    }
    else
    if (newTxState == DIS_TX_STATE_OFF)
    {
        // Radio has been turned off.

        if (entity.damage != DIS_DAMAGE_DESTROYED)
        {
            DisScheduleChangeInterfaceState(radio, false);
        }
    }
    else
    {
        // Radio has been turned on.

        if (entity.damage != DIS_DAMAGE_DESTROYED)
        {
            DisScheduleChangeInterfaceState(radio, true);
        }
    }

    radio.txState = newTxState;
}

#ifdef MILITARY_RADIOS_LIB
void
DisScheduleRtssNotification(
    const DisRadio& radio,
    clocktype delay)
{
    Node* node = radio.node;

    Message* msg = MESSAGE_Alloc(node,
                                 EXTERNAL_LAYER,
                                 EXTERNAL_DIS,
                                 MSG_EXTERNAL_DIS_SendRtss);

    assert(delay >= 0);

    MESSAGE_Send(node, msg, delay);
}

void
DisProcessSendRtssEvent(Node* node, Message* msg)
{
    DisSendRtssNotification(node);
}

void
DisSendRtssNotification(Node* node)
{
    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    const DisData& disData = it->second;

    // All fields except Variable Datum Value are copied in host-byte-order
    // and swapped in a function call.

    DisDataPdu pdu;
    memset(&pdu, 0, sizeof(pdu));

    DisPreparePduHeader(
        pdu.pduHeader,
        DIS_DATA_PDU,
        DIS_PROTOCOL_FAMILY_SIMULATION_MANAGEMENT);

    const DisRadio& radio = *disData.radioPtr;

    pdu.originatingEntityId = radio.entityPtr->entityId;

    pdu.numberOfVariableDatumRecords = 1;
    pdu.variableDatumId = 60010;  // RTSS

    unsigned char* vdv = pdu.variableDatumValue;
    unsigned vdvOffset = 0;

    // DatumValue.
    // Converted to network-byte-order here.

    DisCopyToOffsetAndHtonEntityId(
        vdv,
        vdvOffset,
        &pdu.originatingEntityId);

    DisCopyToOffsetAndHton(
        vdv,
        vdvOffset,
        &radio.radioId,
        sizeof(radio.radioId));

    DisCopyToOffsetAndHton(
        vdv,
        vdvOffset,
        &pdu.pduHeader.timestamp,
        sizeof(pdu.pduHeader.timestamp));

    const float windowTime = 0.0;

    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &windowTime, sizeof(windowTime));

    assert(vdvOffset <= g_disMaxOutgoingDataPduVariableDatumsSize);

    pdu.variableDatumLength = vdvOffset * 8;

    pdu.pduHeader.length  = sizeof(DisHeader)
                            + 36
                            + vdvOffset;
    unsigned pduSize = pdu.pduHeader.length;

    DisHtonDataPdu(pdu);

    bool sendVal = DisSendDatagram(
                       g_dis.sd,
                       g_dis.disIpAddress,
                       g_dis.disPort,
                       (char*) &pdu,
                       pduSize);

    if (!sendVal)
    {
        DisReportWarning("Could not send RTSS");
    }

    if (g_dis.debugPrintComms)
    {
        DisPrintSentRtssNotification(node);
    }
}

void
DisPrintSentRtssNotification(Node* node)
{
    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    const DisData& disData = it->second;

    const DisRadio& radio = *disData.radioPtr;

    char clocktypeString[g_disClocktypeStringBufSize];
    TIME_PrintClockInSecond(TIME_getSimTime(node), clocktypeString);

    if (g_dis.debugPrintComms)
    {
        cout << "DIS: RTSS, "
                << radio.entityPtr->markingString << " ("
                << radio.node->nodeId << ","
                << radio.radioId
                << "), time " << clocktypeString << endl;
    }
}
#endif /* MILITARY_RADIOS_LIB */

void
DisProcessSignalPdu(const char* pduBuf, unsigned short length)
{
    // Minimum-size PDU has a Data field with four bytes.
    // (Application-Specific Data with four bytes indicating a type.)

    if (length < sizeof(DisSignalPdu) - g_disSignalDataBufSize + 4)
    {
        DisReportWarning("Signal PDU too small");
        return;
    }

    if (length > sizeof(DisSignalPdu))
    {
        DisReportWarning("Signal PDU too big");
        return;
    }

    DisSignalPdu pdu;
    memcpy(&pdu, pduBuf, length);

    DisNtohSignalPdu(pdu);

    unsigned char encodingClass = DisGetEncodingClass(pdu);
    unsigned userProtocolId = DisGetUserProtocolId(pdu);

    if (g_dis.debugPrintComms2)
    {
        cout << "DIS: Received Signal PDU (";

        if (encodingClass == DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA
            && userProtocolId == DIS_USER_PROTOCOL_ID_QUALNET_CER)
        {
            cout << "QualNet Comm Effects Request";
        }
        else
        {
            cout << (unsigned) encodingClass;

            if (encodingClass == DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA)
            {
                cout << ", " << userProtocolId;
            }
        }

        cout << ")" << endl;
    }

    // Verify Encoding Class indicates Application-Specific Data, or drop.

    if (encodingClass != DIS_ENCODING_CLASS_APPLICATION_SPECIFIC_DATA)
    { return; }

    // Verify User Protocol indicates QualNet Comm Effects Request, or drop.

    if (userProtocolId != DIS_USER_PROTOCOL_ID_QUALNET_CER)
    { return; }

    // Service PDU.

    DisProcessCommEffectsRequest(pdu);
}

void
DisProcessCommEffectsRequest(const DisSignalPdu& pdu)
{
    DisSimulatedMsgInfo smInfo;

    double              requestedDataRate;
    const DisEntity*    dstEntityPtr = NULL;
    unsigned            dataMsgSize;
    clocktype           voiceMsgDuration;
    bool                isVoice;
    clocktype           timeoutDelay;
    unsigned            timestamp;

    bool                unicast;
    const DisRadio*     dstRadioPtr = NULL;

    DisRadio* srcRadioPtr = DisGetRadioPtr(pdu.entityId, pdu.radioId);

    if (srcRadioPtr == NULL)
    {
        char warningString[MAX_STRING_LENGTH];
        char entityIdString[g_disEntityIdStringBufSize];
        DisMakeEntityIdString(pdu.entityId, entityIdString);

        sprintf(
            warningString,
            "Can't map to QualNet radio using Entity ID %s, Radio ID = %hu",
            entityIdString, pdu.radioId);
        DisReportWarning(warningString);

        return;
    }

    const DisRadio& srcRadio = *srcRadioPtr;

    smInfo.srcRadioPtr = &srcRadio;

#ifndef MILITARY_RADIOS_LIB
    if (pdu.sampleRate == 0
        && (srcRadio.node->macData[0]->macProtocol
              != MAC_PROTOCOL_TADIL_LINK11
            && srcRadio.node->macData[0]->macProtocol
              != MAC_PROTOCOL_TADIL_LINK16))
    {
        DisReportWarning("Sample Rate field is 0 for non-Link-11/16 radio");
        return;
    }
#endif /* MILITARY_RADIOS_LIB */

    requestedDataRate = (double) pdu.sampleRate;

    if (!DisParseMsgString(
             (char*) &pdu.data[4],
             dstEntityPtr,
             dataMsgSize,
             voiceMsgDuration,
             isVoice,
             timeoutDelay,
             timestamp))
    { return; }

    if (dstEntityPtr == srcRadio.entityPtr)
    {
        DisReportWarning("Can't send message to self");
        return;
    }

    if (srcRadio.networkPtr == NULL)
    {
        DisReportWarning("Can't find associated network");
        return;
    }

    if (srcRadio.networkPtr->unicast)
    {
        unicast = true;
        if (dstEntityPtr == NULL)
        {
            ERROR_Assert(srcRadio.defaultDstRadioPtr != NULL,
                         "DIS network is configured for unicast messages but "
                         "the external application wants to send a broadcast.");
            dstRadioPtr = srcRadio.defaultDstRadioPtr;
            dstEntityPtr = dstRadioPtr->entityPtr;
        }
        else
        {
            // Find dst radio in same network as src radio.

            const DisNetwork& network = *srcRadio.networkPtr;

            unsigned i;
            for (i = 0; i < network.numRadioPtrs; i++)
            {
                const DisRadio& dstRadio = *network.radioPtrs[i];

                if (dstRadio.node == srcRadio.node) { continue; }

                if (dstRadio.entityPtr->entityId == dstEntityPtr->entityId)
                { break; }
            }

            if (i == network.numRadioPtrs)
            {
                DisReportWarning("Dst EntityID is not in same network");
                return;
            }

            dstRadioPtr = network.radioPtrs[i];
        }
    }
    else
    {
        unicast = false;
    }

    if (!unicast)
    {
        // This is a broadcast, so check that there is at least one receiver.

        const DisNetwork& network = *srcRadio.networkPtr;

        unsigned i;
        for (i = 0; i < network.numRadioPtrs; i++)
        {
            const DisRadio& dstRadio = *network.radioPtrs[i];

            if (dstRadio.node->nodeId == srcRadio.node->nodeId) { continue; }

            break;
        }

        if (i == network.numRadioPtrs)
        {
            if (g_dis.debugPrintComms)
            {
                DisReportWarning(
                    "Ignoring broadcast to network with no potential"
                    " receivers");
            }

            return;
        }//if//
    }//if//

    DisNodeIdToPerNodeDataMap::iterator it =
        g_dis.nodeIdToPerNodeData.find(srcRadio.node->nodeId);

    DisData &srcDisData = it->second;

    smInfo.msgId = srcDisData.nextMsgId;
    srcDisData.nextMsgId++;

    if (srcDisData.nextMsgId > g_disMaxMsgId)
    {
        DisReportError("Maximum msgId reached", __FILE__, __LINE__);
    }

    Node* srcNode = srcRadio.node;

    clocktype simTime = TIME_getSimTime(srcNode);
    clocktype sendTime;

    if (g_dis.iface->horizon == 0)
    {
        // During initialization schedule transmissions at 1ns
        sendTime = 1 * NANO_SECOND;
    }
    else
    {
        sendTime = EXTERNAL_QueryExternalTime(g_dis.iface);
    }

    const clocktype sendDelay = sendTime - TIME_getSimTime(srcNode);

    DisSendSimulatedMsgUsingMessenger(
        smInfo,
        dstEntityPtr,
        requestedDataRate,
        dataMsgSize,
        voiceMsgDuration,
        isVoice,
        timeoutDelay,
        unicast,
        dstRadioPtr,
        sendDelay);

    DisScheduleTimeout(smInfo, timeoutDelay, sendDelay);

    DisStoreOutstandingSimulatedMsgInfo(
        smInfo,
        dstEntityPtr,
        timestamp,
        sendTime);

    if (g_dis.debugPrintComms)
    {
        DisPrintCommEffectsRequestProcessed(
            srcRadio, dstEntityPtr, unicast, dstRadioPtr, sendTime);
    }
}

bool
DisParseMsgString(
    char*             msgString,
    const DisEntity*& dstEntityPtr,
    unsigned&         dataMsgSize,
    clocktype&        voiceMsgDuration,
    bool&             isVoice,
    clocktype&        timeoutDelay,
    unsigned&         timestamp)
{
    char token[g_disSignalDataBufSize];
    char* next = msgString;

    IO_GetDelimitedToken(token, next, "\n", &next);
    if (strcmp(token, "HEADER") != 0)
    {
        DisReportWarning("Message string did not start with HEADER");
        return false;
    }

    char name[g_disSignalDataBufSize];
    char value[g_disSignalDataBufSize];
    char* nextNameValue;

    // Required fields.

    bool foundSizeField      = false;
    bool foundTimeoutField   = false;
    bool foundTimestampField = false;

    if (g_dis.debugPrintComms2) { cout << "DIS: "; }

    while (IO_GetDelimitedToken(token, next, "\n", &next))
    {
        if (strcmp(token, "EOH") == 0)
        {
            if (g_dis.debugPrintComms2) { cout << endl; }
            break;
        }

        if (g_dis.debugPrintComms2) { cout << token << " "; }

        if (!IO_GetDelimitedToken(name, token, "=", &nextNameValue))
        {
            DisReportWarning("Can't find attribute=value in message string");
            return false;
        }

        if (!IO_GetDelimitedToken(value, nextNameValue, "=", &nextNameValue))
        {
            DisReportWarning("Can't find attribute=value in message string");
            return false;
        }

        if (strcmp(name, "receiver") == 0)
        {
            DisEntityId dstEntityId;

            if (sscanf(value, "%hu.%hu.%hu",
                       &dstEntityId.siteId,
                       &dstEntityId.applicationId,
                       &dstEntityId.entityNumber) != 3)
            {
                DisReportWarning("Can't read receiver field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            DisEntityIdToEntityMap::iterator it
                = g_dis.entityIdToEntity.find(dstEntityId);

            if (it == g_dis.entityIdToEntity.end())
            {
                DisReportWarning(
                    "Can't map receiver EntityID in message string to entity",
                    __FILE__, __LINE__);
                return false;
            }

            dstEntityPtr = it->second;
        }
        else
        if (strcmp(name, "size") == 0)
        {
            double sizeValue;
            char unitString[g_disSignalDataBufSize];

            if (sscanf(value, "%lf %s", &sizeValue, unitString) != 2
                || sizeValue < 0.0
                || sizeValue > (double) UINT_MAX)
            {
                DisReportWarning("Bad size field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            // TODO:  Fractional sizes (either bytes or seconds) are rounded
            // down here.  Perhaps fractional values should be retained,
            // especially for seconds.

            if (strcmp(unitString, "bytes") == 0)
            {
                dataMsgSize = (unsigned) sizeValue;

                isVoice = false;
            }
            else
            if (strcmp(unitString, "seconds") == 0)
            {
                voiceMsgDuration = DisConvertDoubleToClocktype(sizeValue);

                isVoice = true;
            }
            else
            {
                DisReportWarning("Unrecognized units in size field",
                                 __FILE__, __LINE__);
                return false;
            }

            foundSizeField = true;
        }
        else
        if (!strcmp(name, "timeout"))
        {
            char* endPtr = NULL;
            errno = 0;
            double double_timeoutDelay = strtod(value, &endPtr);
            if (endPtr == value || errno != 0)
            {
                DisReportWarning("Bad timeout field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            timeoutDelay = DisConvertDoubleToClocktype(double_timeoutDelay);
            foundTimeoutField = true;
        }
        else
        if (!strcmp(name, "timestamp"))
        {
            // 0x34567890, 10 bytes.

            const unsigned timestampStringLength = 10;
            if (strlen(value) != timestampStringLength
                || !(value[0] == '0' && (value[1] == 'x' || value[1] == 'X'))
                || sscanf(&value[2], "%x", &timestamp) != 1)
            {
                DisReportWarning("Bad timestamp field in message string",
                                 __FILE__, __LINE__);
                return false;
            }

            foundTimestampField = true;
        }
    }//while//

    if (!(foundSizeField && foundTimeoutField && foundTimestampField))
    {
        DisReportWarning("All essential fields not present in message string",
                         __FILE__, __LINE__);
        return false;
    }

    return true;
}

void
DisSendSimulatedMsgUsingMessenger(
    const DisSimulatedMsgInfo& smInfo,
    const DisEntity* dstEntityPtr,
    double           requestedDataRate,
    unsigned         dataMsgSize,
    clocktype        voiceMsgDuration,
    bool             isVoice,
    clocktype        timeoutDelay,
    bool             unicast,
    const DisRadio*  dstRadioPtr,
    clocktype        sendDelay)
{
    const DisRadio& srcRadio = *smInfo.srcRadioPtr;
    Node* srcNode = srcRadio.node;

    // The variable below will be retrieved as an info field in the Messenger
    // result function.

    MessengerPktHeader info;

    info.pktNum = 0;
    info.initialPrDelay = sendDelay;
    assert(info.initialPrDelay >= 0);

    info.srcAddr
       = MAPPING_GetDefaultInterfaceAddressFromNodeId(srcNode,
                                                      srcNode->nodeId);

    if (unicast)
    {
        assert(dstRadioPtr != NULL);

        info.destAddr
           = MAPPING_GetDefaultInterfaceAddressFromNodeId(
               srcNode, dstRadioPtr->node->nodeId);
    }
    else
    {
        assert(srcRadio.networkPtr->ipAddress != 0);
        info.destAddr = srcRadio.networkPtr->ipAddress;
    }

    info.msgId = (int) smInfo.msgId;
    info.lifetime = timeoutDelay;

    // Add the size of one double variable in case the compiler adds padding
    // between the Messenger data and the custom data.

    unsigned minBytes
        = sizeof(MessengerPktHeader) + sizeof(smInfo) + sizeof(double);

#ifdef MILITARY_RADIOS_LIB
    if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK11
        || srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK16)
    {
        info.transportType = TRANSPORT_TYPE_MAC;
    }
    else
    {
#endif /* MILITARY_RADIOS_LIB */
        info.transportType = TRANSPORT_TYPE_UNRELIABLE;
#ifdef MILITARY_RADIOS_LIB
    }
#endif /* MILITARY_RADIOS_LIB */

    // Perform different modeling depending on if the message is a data
    // message or a voice message.

    if (!isVoice)
    {
        // Data message.

        info.appType = GENERAL;

#ifdef MILITARY_RADIOS_LIB
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK11
            || srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK16)
        {
            // For Link-11 or Link-16, just send one huge packet for this
            // data message.
            // The Link-11 model won't fragment the packet, but send the
            // whole thing in one shot, following real-world operation.
            // The Link-16 model will send the packet using a TDMA approach.

            info.freq = 0 * SECOND;

            if (dataMsgSize >= minBytes) { info.fragSize = dataMsgSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        {
#endif /* MILITARY_RADIOS_LIB */
            // For non-Link-11 network devices, chunk the data message into
            // packets, each packet containing 128-bytes payload.  This
            // size was arbitrarily chosen.  Since the data message is sent
            // via UDP, the payload size per packet is limited by QualNet's
            // UDP model.
            //
            // Dividing the payload-size of each packet by the ASRS
            // interaction's DataRate parameter, one obtains the packet
            // interval.
            //
            // Dividing dataMsgSize by the payload-size of each packet,
            // one obtains the number of packets.

            unsigned payloadSize = 128;
            double packetInterval = (double) payloadSize * 8.0
                                    / requestedDataRate;

            info.freq     = DisConvertDoubleToClocktype(packetInterval);
            info.fragSize = payloadSize;

            info.numFrags = (unsigned short)
                            ceil(((double) dataMsgSize)
                                  / (double) payloadSize);

            unsigned minFragments
                = (unsigned) ceil((double) minBytes
                                  / (double) info.fragSize);
            if (info.numFrags < minFragments)
            {
                info.numFrags = minFragments;
            }
#ifdef MILITARY_RADIOS_LIB
        }
#endif /* MILITARY_RADIOS_LIB */
    }
    else
    {
        // Voice message.

        info.appType = VOICE_PACKET;

#ifdef MILITARY_RADIOS_LIB
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK11)
        {
            // For Link-11, the voice message is comprised of a single large
            // packet, just as with Link-11 data messages.
            //
            // Multiplying voiceMsgDuration with the PHY data rate, one
            // obtains the size of the message in bytes such that it should
            // take approximately the correct amount of time.
            //
            // Note:  The actual number of bytes should be less than the
            // value calculated as described, since there are probably
            // overhead bytes added by the Link-11 model.  This is assumed
            // to be negligible though.

            info.freq = 0 * SECOND;

            PhyDataAbstract* pda
                = (PhyDataAbstract*)
                  srcNode->phyData[MAC_DEFAULT_INTERFACE]->phyVar;

            unsigned packetSize
                = (unsigned)
                  (DisConvertClocktypeToDouble(voiceMsgDuration)
                   * ((double) pda->dataRate / 8.0));

            if (packetSize >= minBytes) { info.fragSize = packetSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
            == MAC_PROTOCOL_TADIL_LINK16)
        {
            // For Link-16, the voice message is comprised of a single large
            // packet, just as with Link-16 data messages.
            //
            // An 8-kbps data rate for the voice message is assumed.
            // Multiplying the data rate by voiceMsgDuration, one obtains the
            // size of the voice message in bytes.
            //
            // Note:  This computation implies that the entire voice message
            // is available to be transmitted on the Link-16 network at the
            // time the ASRS interaction was received.  This means that the
            // entire voice message can be transmitted in a time much less
            // than the duration of the message, given a high Link-16 data
            // rate and a large proportion of slots assigned to the source
            // node.
            //
            // This is in contrast to the Link-11 and default approach for
            // voice messages, which assumes that the voice message is being
            // read off starting at the time the ASRS was received (in the
            // Link-11 and default approach for voice, if voiceMsgDuration is
            // 10 seconds, it should take ~ 10 seconds of simulation time
            // before QualNet sends a Process Message interaction).
            //
            // The differing approach for Link-16 is assumed to be okay
            // because Link-16 operates in a TDMA fashion anyway.

            info.freq = 0 * SECOND;

            PhyDataAbstract* pda
                = (PhyDataAbstract*)
                  srcNode->phyData[MAC_DEFAULT_INTERFACE]->phyVar;

            unsigned packetSize
                = (unsigned)
                  (DisConvertClocktypeToDouble(voiceMsgDuration)
                   * (8000.0 / 8.0));

            if (packetSize >= minBytes) { info.fragSize = packetSize; }
            else { info.fragSize = minBytes; }

            info.numFrags = 1;
        }
        else
        {
#endif /* MILITARY_RADIOS_LIB */
            // For non-Link-11 network devices, an 8-kbps data rate for the
            // voice message is assumed, with a 250-byte payload per packet
            // at 4 packets per second.  (Real VoIP applications would send
            // smaller payloads at a more frequent rate to reach 8-kbps.
            // The values uses in the model are chosen to increase modeling
            // speed.)
            //
            // Dividing voiceMsgDuration by the packet interval, one obtains
            // the number of packets for the voice message.

            const double   packetInterval = 0.25;
            const unsigned payloadSize    = 250;

            info.freq     = DisConvertDoubleToClocktype(packetInterval);
            info.fragSize = payloadSize;

            info.numFrags
                = (unsigned short)
                  ceil(DisConvertClocktypeToDouble(voiceMsgDuration)
                       / packetInterval);

            unsigned minFragments
                = (unsigned) ceil((double) minBytes
                                  / (double) info.fragSize);
            if (info.numFrags < minFragments)
            {
                info.numFrags = minFragments;
            }
#ifdef MILITARY_RADIOS_LIB
        }
#endif /* MILITARY_RADIOS_LIB */
    }//if//

    if (g_dis.debugPrintComms2)
    {
        unsigned oldPrecision = cout.precision();
        cout.precision(3);

        cout << "DIS: " << info.numFrags << " packet(s), "
                << info.fragSize << " bytes per packet, "
                << DisConvertClocktypeToDouble(info.freq) << " sec interval"
                << endl;

        cout.precision(oldPrecision);
    }

#ifdef MILITARY_RADIOS_LIB
    if (srcNode->macData[MAC_DEFAULT_INTERFACE]->macProtocol
        == MAC_PROTOCOL_TADIL_LINK16)
    {
        MacDataLink16* link16 =
            (MacDataLink16*) srcNode->macData[MAC_DEFAULT_INTERFACE]->macVar;

        assert(link16 != NULL);
        assert(link16->numNPG > 0);
        assert(link16->npgInfo != NULL);

        info.destNPGId = link16->npgInfo[0].npgAddress;
    }
#endif /* MILITARY_RADIOS_LIB */

    MessengerSendMessage(
        srcNode,
        info,
        (char*) &smInfo,
        sizeof(smInfo),
        &DisMessengerResultFcn);

    if (!g_dis.newEventScheduled) { g_dis.newEventScheduled = true; }
}

void
DisScheduleTimeout(
    const DisSimulatedMsgInfo& smInfo,
    clocktype timeoutDelay,
    clocktype sendDelay)
{
    Node* srcNode = smInfo.srcRadioPtr->node;

    Message* timeoutMsg;
    timeoutMsg = MESSAGE_Alloc(srcNode,
                               EXTERNAL_LAYER,
                               EXTERNAL_DIS,
                               MSG_EXTERNAL_DIS_SimulatedMsgTimeout);

    MESSAGE_InfoAlloc(srcNode, timeoutMsg, sizeof(smInfo));
    DisSimulatedMsgInfo& timeoutHeader
        = *((DisSimulatedMsgInfo*) MESSAGE_ReturnInfo(timeoutMsg));

    timeoutHeader = smInfo;

    assert(timeoutDelay >= 0);
    assert(sendDelay >= 0);

    clocktype delay = sendDelay + timeoutDelay;

    MESSAGE_Send(srcNode, timeoutMsg, delay);

    if (!g_dis.newEventScheduled) { g_dis.newEventScheduled = true; }
}

void
DisStoreOutstandingSimulatedMsgInfo(
    const DisSimulatedMsgInfo& smInfo,
    const DisEntity* dstEntityPtr,
    unsigned timestamp,
    clocktype sendTime)
{
    const DisRadio& srcRadio = *smInfo.srcRadioPtr;
    Node* srcNode = srcRadio.node;

    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(srcNode->nodeId);
    DisData& disData = it->second;

    DisOutstandingSimulatedMsgInfo& osmInfo
        = disData.outstandingSimulatedMsgInfo[smInfo.msgId];

    osmInfo.timestamp = timestamp;
    osmInfo.sendTime = sendTime;

    osmInfo.numDstEntities = 0;
    osmInfo.numDstEntitiesProcessed = 0;

    if (dstEntityPtr != NULL)
    {
        assert(srcRadio.entityPtr != dstEntityPtr);

        DisSimulatedMsgDstEntityInfo &smDstEntityInfo
            = osmInfo.smDstEntitiesInfo[osmInfo.numDstEntities];

        smDstEntityInfo.dstEntityPtr = dstEntityPtr;
        smDstEntityInfo.processed    = false;
        smDstEntityInfo.success      = false;

        osmInfo.numDstEntities++;
    }
    else
    {
        const DisNetwork& network = *srcRadio.networkPtr;

        unsigned i;
        for (i = 0; i < network.numRadioPtrs; i++)
        {
            const DisRadio& dstRadio = *network.radioPtrs[i];

            if (dstRadio.node->nodeId == srcRadio.node->nodeId) { continue; }

            DisSimulatedMsgDstEntityInfo &smDstEntityInfo
                = osmInfo.smDstEntitiesInfo[osmInfo.numDstEntities];

            smDstEntityInfo.dstEntityPtr = dstRadio.entityPtr;
            smDstEntityInfo.processed    = false;
            smDstEntityInfo.success      = false;

            osmInfo.numDstEntities++;
        }

        assert(osmInfo.numDstEntities > 0);
    }
}

void
DisMessengerResultFcn(Node* node, Message* msg, BOOL success)
{
    // Messenger has returned a result for a given message.

    DisMessengerResultFcnBody(node, msg, success);

    MESSAGE_Free(node, msg);
}

void
DisMessengerResultFcnBody(Node* node, Message* msg, BOOL success)
{
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

    DisNodeIdToPerNodeDataMap::iterator dstIt
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    if (dstIt == g_dis.nodeIdToPerNodeData.end())
    {
        // this node does not map to a DIS entity
        return;
    }
    const DisData& dstDisData = dstIt->second;

    const DisRadio& dstRadio = *dstDisData.radioPtr;

    DisSimulatedMsgInfo& smInfo
        = *((DisSimulatedMsgInfo*) MESSAGE_ReturnPacket(msg));

    const DisRadio& srcRadio = *smInfo.srcRadioPtr;

    DisNodeIdToPerNodeDataMap::iterator srcIt
        = g_dis.nodeIdToPerNodeData.find(srcRadio.node->nodeId);
    DisData& srcDisData = srcIt->second;

    DisOutstandingSimulatedMsgInfoMap::iterator msgIt
        = srcDisData.outstandingSimulatedMsgInfo.find(smInfo.msgId);

    if (msgIt == srcDisData.outstandingSimulatedMsgInfo.end())
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

        //if (g_dis.debug)
        //{
        //    DisPrintCommEffectsResult(node, smInfo, "IGNR");
        //}

        return;
    }

    DisOutstandingSimulatedMsgInfo& osmInfo = msgIt->second;

    assert(osmInfo.numDstEntities <= g_disMaxMembersInNetwork);
    assert(osmInfo.numDstEntitiesProcessed < osmInfo.numDstEntities);

    unsigned i;
    for (i = 0; i < osmInfo.numDstEntities; i++)
    {
        const DisSimulatedMsgDstEntityInfo &smDstEntityInfo
            = osmInfo.smDstEntitiesInfo[i];

        if (smDstEntityInfo.dstEntityPtr == dstRadio.entityPtr) { break; }
    }

    if (i == osmInfo.numDstEntities)
    {
        // The result function has been called with successful delivery to this
        // node, but this node isn't in the source node's list of destination
        // nodes.

        return;
    }

    DisSimulatedMsgDstEntityInfo &smDstEntityInfo = osmInfo.smDstEntitiesInfo[i];

    if (smDstEntityInfo.processed)
    {
        // This destination has already been processed.

        return;
    }

    smDstEntityInfo.processed = true;
    smDstEntityInfo.success = (success == TRUE);

    osmInfo.numDstEntitiesProcessed++;

    DisSendProcessMsgNotification(node, smInfo, osmInfo, 1);

    if (osmInfo.numDstEntitiesProcessed == osmInfo.numDstEntities)
    {
        DisSendTimeoutNotification(node, smInfo, osmInfo);

        srcDisData.outstandingSimulatedMsgInfo.erase(msgIt);
    }
}

void
DisSendProcessMsgNotification(
    Node* node,
    const DisSimulatedMsgInfo& smInfo,
    const DisOutstandingSimulatedMsgInfo& osmInfo,
    unsigned short success)
{
    // All fields except Variable Datum Value are copied in host-byte-order
    // and swapped in a function call.

    DisDataPdu pdu;
    memset(&pdu, 0, sizeof(pdu));

    DisPreparePduHeader(
        pdu.pduHeader,
        DIS_DATA_PDU,
        DIS_PROTOCOL_FAMILY_SIMULATION_MANAGEMENT);

    const DisRadio& srcRadio = *smInfo.srcRadioPtr;

    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    const DisData& dstDisData = it->second;

    const DisRadio& dstRadio = *dstDisData.radioPtr;

    pdu.originatingEntityId = srcRadio.entityPtr->entityId;

    pdu.numberOfVariableDatumRecords = 1;
    pdu.variableDatumId = 60001;  // Process Message notification

    unsigned char* vdv = pdu.variableDatumValue;
    unsigned vdvOffset = 0;

    // DatumValue.
    // Converted to network-byte-order here.

    DisCopyToOffsetAndHtonEntityId(
        vdv,
        vdvOffset,
        &pdu.originatingEntityId);

    DisCopyToOffsetAndHton(
        vdv,
        vdvOffset,
        &srcRadio.radioId,
        sizeof(srcRadio.radioId));

    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &osmInfo.timestamp, sizeof(osmInfo.timestamp));

    const unsigned numEntityIds = 1;
    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &numEntityIds, sizeof(numEntityIds));

    DisCopyToOffsetAndHtonEntityId(
        vdv,
        vdvOffset,
        &dstRadio.entityPtr->entityId);
    pdu.receivingEntityId = dstRadio.entityPtr->entityId;

    const clocktype delay = TIME_getSimTime(node) - osmInfo.sendTime;
    const double double_delay = DisConvertClocktypeToDouble(delay);
    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &double_delay, sizeof(double_delay));

    assert(vdvOffset <= g_disMaxOutgoingDataPduVariableDatumsSize);

    pdu.variableDatumLength = vdvOffset * 8;

    pdu.pduHeader.length  = sizeof(DisHeader)
                            + 36
                            + vdvOffset;
    unsigned pduSize = pdu.pduHeader.length;

    DisHtonDataPdu(pdu);

    bool sendVal = DisSendDatagram(
                       g_dis.sd,
                       g_dis.disIpAddress,
                       g_dis.disPort,
                       (char*) &pdu,
                       pduSize);

    if (!sendVal)
    {
        DisReportWarning("Could not send Process Message notification");
    }

    if (g_dis.debugPrintComms)
    {
        DisPrintCommEffectsResult(node, smInfo, "SUCC");
    }
}

void
DisProcessTimeoutEvent(Node* node, Message* msg)
{
    DisSimulatedMsgInfo& smInfo
        = *((DisSimulatedMsgInfo*) MESSAGE_ReturnInfo(msg));

    DisNodeIdToPerNodeDataMap::iterator disIt
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    DisData& disData = disIt->second;

    DisOutstandingSimulatedMsgInfoMap::iterator msgIt
        = disData.outstandingSimulatedMsgInfo.find(smInfo.msgId);

    if (msgIt == disData.outstandingSimulatedMsgInfo.end())
    {
        // Timeout notification already sent.

        if (g_dis.debugPrintComms)
        {
            DisPrintCommEffectsResult(node, smInfo, "CANC");
        }

        return;
    }

    const DisOutstandingSimulatedMsgInfo& osmInfo = msgIt->second;

    DisSendTimeoutNotification(node, smInfo, osmInfo);

    disData.outstandingSimulatedMsgInfo.erase(msgIt);
}

void
DisSendTimeoutNotification(
    Node* node,
    const DisSimulatedMsgInfo& smInfo,
    const DisOutstandingSimulatedMsgInfo& osmInfo)
{
    // All fields except Variable Datum Value are copied in host-byte-order
    // and swapped in a function call.

    DisDataPdu pdu;
    memset(&pdu, 0, sizeof(pdu));

    DisPreparePduHeader(
        pdu.pduHeader,
        DIS_DATA_PDU,
        DIS_PROTOCOL_FAMILY_SIMULATION_MANAGEMENT);

    const DisRadio& srcRadio = *smInfo.srcRadioPtr;

    pdu.originatingEntityId = srcRadio.entityPtr->entityId;

    pdu.numberOfVariableDatumRecords = 1;
    pdu.variableDatumId = 60002;  // Timeout notification

    unsigned char* vdv = pdu.variableDatumValue;
    unsigned vdvOffset = 0;

    // DatumValue.
    // Converted to network-byte-order here.

    DisCopyToOffsetAndHtonEntityId(
        vdv, vdvOffset, &pdu.originatingEntityId);

    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &srcRadio.radioId, sizeof(srcRadio.radioId));

    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &osmInfo.timestamp, sizeof(osmInfo.timestamp));

    const unsigned numPackets = 1;
    DisCopyToOffsetAndHton(
        vdv, vdvOffset, &numPackets, sizeof(numPackets));

    unsigned numEntityIds = 0;
    unsigned numEntityIdsOffset = vdvOffset;
    vdvOffset += sizeof(numEntityIds);

    assert(osmInfo.numDstEntities <= g_disMaxMembersInNetwork);
    assert(osmInfo.numDstEntities
           <= g_disMaxDstEntityIdsInTimeoutNotification);

    unsigned i;
    for (i = 0; i < osmInfo.numDstEntities; i++)
    {
        const DisSimulatedMsgDstEntityInfo& smDstEntityInfo
            = osmInfo.smDstEntitiesInfo[i];

        assert(smDstEntityInfo.dstEntityPtr != srcRadio.entityPtr);

        DisCopyToOffsetAndHtonEntityId(
            vdv, vdvOffset, &smDstEntityInfo.dstEntityPtr->entityId);

        DisCopyToOffsetAndHton(
            vdv,
            vdvOffset,
            &smDstEntityInfo.success,
            sizeof(smDstEntityInfo.success));

        numEntityIds++;
    }
    if (osmInfo.numDstEntities == 1)
    {
        pdu.receivingEntityId 
            = osmInfo.smDstEntitiesInfo[0].dstEntityPtr->entityId;
    }
    else
    {
        pdu.receivingEntityId.siteId = DIS_ALL_SITES;
        pdu.receivingEntityId.applicationId = DIS_ALL_APPLIC;
        pdu.receivingEntityId.entityNumber = DIS_ALL_ENTITIES;
    }

    assert(vdvOffset <= g_disMaxOutgoingDataPduVariableDatumsSize);

    DisCopyToOffsetAndHton(
        vdv, numEntityIdsOffset, &numEntityIds, sizeof(numEntityIds));

    pdu.variableDatumLength = vdvOffset * 8;

    pdu.pduHeader.length  = sizeof(DisHeader)
                            + 36
                            + vdvOffset;
    unsigned pduSize = pdu.pduHeader.length;

    DisHtonDataPdu(pdu);

    bool sendVal = DisSendDatagram(
                       g_dis.sd,
                       g_dis.disIpAddress,
                       g_dis.disPort,
                       (char*) &pdu,
                       pduSize);

    if (!sendVal)
    {
        DisReportWarning("Could not send Timeout notification");
    }

    if (g_dis.debugPrintComms)
    {
        DisPrintCommEffectsResult(srcRadio.node, smInfo, "TIME");
    }
}

void
DisPrintCommEffectsRequestProcessed(
    const DisRadio&  srcRadio,
    const DisEntity* dstEntityPtr,
    bool             unicast,
    const DisRadio*  dstRadioPtr,
    clocktype        sendTime)
{
    cout << "DIS: CREQ, "
            << srcRadio.entityPtr->markingString << " ("
            << srcRadio.node->nodeId << ","
            << srcRadio.radioId << ")";

    if (unicast)
    {
        assert(dstEntityPtr != NULL);
        assert(dstRadioPtr != NULL);

        cout << " unicast to " << dstEntityPtr->markingString  << " ("
                << dstRadioPtr->node->nodeId << ","
                << dstRadioPtr->radioId << ")";
    }
    else
    {
        cout << " broadcast";

        if (dstEntityPtr != NULL)
        {
            cout << " to " << dstEntityPtr->markingString;
        }
    }


    char sendTimeString[MAX_STRING_LENGTH];
    TIME_PrintClockInSecond(sendTime, sendTimeString);

    cout << ", send time " << sendTimeString << endl;
}

void
DisPrintCommEffectsResult(
    Node* node, const DisSimulatedMsgInfo& smInfo, const char* resultString)
{
    const DisRadio& srcRadio = *smInfo.srcRadioPtr;

    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);
    DisData& dstDisData = it->second;

    const DisRadio& dstRadio = *dstDisData.radioPtr;

    char clocktypeString[g_disClocktypeStringBufSize];
    TIME_PrintClockInSecond(TIME_getSimTime(node), clocktypeString);

    cout << "FED: " << resultString << ", "
            << srcRadio.entityPtr->markingString << " ("
            << srcRadio.node->nodeId << ","
            << srcRadio.radioId
            << ") to "
            << dstRadio.entityPtr->markingString << " ("
            << dstRadio.node->nodeId << ","
            << dstRadio.radioId
            << "), time " << clocktypeString << endl;
}

DisRadio*
DisGetRadioPtr(const DisEntityId& entityId, unsigned short radioId)
{
    // Retrieve entity.

    DisEntityIdToEntityMap::iterator entityIt
        = g_dis.entityIdToEntity.find(entityId);

    if (entityIt == g_dis.entityIdToEntity.end()) { return NULL; }

    DisEntity *entity = entityIt->second;

    // Retrieve radio.

    DisRadioKey radioKey;
    strcpy(radioKey.markingString, entity->markingString);
    radioKey.radioId = radioId;

    DisRadioKeyToRadioMap::iterator radioIt
        = g_dis.radioKeyToRadio.find(radioKey);
    if (radioIt == g_dis.radioKeyToRadio.end()) { return NULL; }

    return radioIt->second;
}

void
DisPreparePduHeader(
    DisHeader& pduHeader,
    unsigned char pduType,
    unsigned char protocolFamily)
{
    pduHeader.protocolVersion    = 4;
    // TODO:  Exercise ID should be defineable in .config file.
    pduHeader.exerciseIdentifier = 1;
    pduHeader.pduType            = pduType;

    pduHeader.protocolFamily     = protocolFamily;
    pduHeader.timestamp          = DisGetTimestamp();
}

void
DisMallocEntities()
{
    DisCheckNoMalloc(g_dis.entities, __FILE__, __LINE__);
    g_dis.entities = new DisEntity [g_dis.numEntities];
    DisCheckMalloc(g_dis.entities, __FILE__, __LINE__);
}

void
DisFreeEntities()
{
    if (g_dis.entities)
    {
        delete [] g_dis.entities;
        g_dis.entities = NULL;
    }
}

void
DisMallocRadios()
{
    DisCheckNoMalloc(g_dis.radios, __FILE__, __LINE__);
    g_dis.radios = new DisRadio [g_dis.numRadios];
    DisCheckMalloc(g_dis.radios, __FILE__, __LINE__);
}

void
DisFreeRadios()
{
    if (g_dis.radios)
    {
        delete [] g_dis.radios;
        g_dis.radios = NULL;
    }
}

void
DisMallocNetworks()
{
    DisCheckNoMalloc(g_dis.networks, __FILE__, __LINE__);
    g_dis.networks = new DisNetwork [g_dis.numNetworks];
    DisCheckMalloc(g_dis.networks, __FILE__, __LINE__);
}

void
DisFreeNetworks()
{
    if (g_dis.networks)
    {
        delete [] g_dis.networks;
        g_dis.networks = NULL;
    }
}
