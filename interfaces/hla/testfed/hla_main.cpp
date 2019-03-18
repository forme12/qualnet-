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

#include "hla_fileio.h"

#include "RTI.hh"

#include "hla_HashTable.h"
#include "hla_gcclatlon.h"
#include "hla_archspec.h"
#include "hla_rpr_fom.h"
#include "hla_shared.h"
#include "hla_hashfcns.h"
#include "hla_FedAmb.h"
#include "hla.h"
#include "hla_main.h"

HlaFedAmb*          g_hlaFedAmb = NULL;
RTI::RTIambassador* g_hlaRtiAmb = NULL;

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

void
HlaMallocFederateAmbassador()
{
    HlaCheckNoMalloc(g_hlaFedAmb, __FILE__, __LINE__);
    g_hlaFedAmb = new HlaFedAmb;
    HlaCheckMalloc(g_hlaFedAmb, __FILE__, __LINE__);
}

void
HlaMallocRtiAmbassador()
{
    HlaCheckNoMalloc(g_hlaRtiAmb, __FILE__, __LINE__);
    g_hlaRtiAmb = new RTI::RTIambassador;
    HlaCheckMalloc(g_hlaRtiAmb, __FILE__, __LINE__);
}

void
HlaFreeFederateAmbassador()
{
    if (g_hlaFedAmb)
    {
        delete g_hlaFedAmb;
        g_hlaFedAmb = NULL;
    }
}

void
HlaFreeRtiAmbassador()
{
    if (g_hlaRtiAmb)
    {
        delete g_hlaRtiAmb;
        g_hlaRtiAmb = NULL;
    }
}

void
HlaReadEntitiesFile()
{
    // Determine path of .hla-entities file.

    char path[g_hlaPathBufSize];

    sprintf(path, "%s.hla-entities", g_hlaFedAmb->m_scenarioName);

    cout << ".hla-entities file = " << path << "." << endl;

    // Determine number of lines in file.

    g_hlaFedAmb->m_numEntities = HlaGetNumLinesInFile(path);
    HlaVerify(g_hlaFedAmb->m_numEntities > 0, "File is empty", path);

    g_hlaFedAmb->MallocEntities();

    // Open file.

    FILE* fpEntities = fopen(path, "r");
    HlaVerify(fpEntities != NULL, "Can't open for reading", path);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    char* p;
    char* next;
    unsigned lineNumber;
    unsigned i;
    for (i = 0, lineNumber = 1;
         i < g_hlaFedAmb->m_numEntities;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, lineBufSize, fpEntities) != NULL,
                 "Not enough lines",
                 path);

        HlaVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 path);

        next = line;

        HlaEntity& entity = g_hlaFedAmb->m_entities[i];

        // EntityID.

        entity.entityId.siteId        = g_hlaFedAmb->m_siteId;
        entity.entityId.applicationId = g_hlaFedAmb->m_applicationId;
        entity.entityId.entityNumber  = lineNumber;

        HlaMakeEntityIdString(entity.entityId, entity.entityIdString);

        // EntityID to entity pointer hash.

        const HlaEntityId& entityKey = entity.entityId;

        HlaVerify(
            g_hlaFedAmb->m_entityIdToEntityPtrHash.GetItem(&entityKey) == NULL,
            "Entity with duplicate EntityID",
            path, lineNumber);

        HlaHashItem& entityPtrItem
             = g_hlaFedAmb->m_entityIdToEntityPtrHash.AddItem(&entityKey);
        HlaEntity*& entityPtr = *((HlaEntity**) entityPtrItem.GetDataPtr());
        entityPtr = &entity;

        // MarkingData.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read MarkingData", path, lineNumber);

        IO_TrimLeft(token);
        IO_TrimRight(token);

        HlaVerify(strlen(token) < sizeof(entity.markingData),
                 "MarkingData too large",
                 path, lineNumber);
        strcpy(entity.markingData, token);

        // MarkingData to entity pointer hash.

        HlaVerify(
            g_hlaFedAmb->m_markingDataToEntityPtrHash
                .GetItem(entity.markingData) == NULL,
            "Entity with duplicate MarkingData",
            path, lineNumber);

        HlaHashItem& entityPtrItem2
            = g_hlaFedAmb->m_markingDataToEntityPtrHash.AddItem(entity.markingData);
        HlaEntity*& entityPtr2 = *((HlaEntity**) entityPtrItem2.GetDataPtr());
        entityPtr2 = &entity;

        // ForceID.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read ForceID", path, lineNumber);

        IO_TrimLeft(token);
        IO_TrimRight(token);

        HlaVerify(strlen(token) == 1, "Invalid ForceID", path, lineNumber);

        switch(token[0])
        {
            case 'F':
                entity.forceId = HLA_FORCEID_FRIENDLY;
                break;
            case 'O':
                entity.forceId = HLA_FORCEID_OPPOSING;
                break;
            case 'N':
                entity.forceId = HLA_FORCEID_NEUTRAL;
                break;
            default:
                HlaReportError("Invalid ForceID", path, lineNumber);
        }

        // Country string (skip, since the numeric value is in EntityType).

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read country string", path, lineNumber);

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

        HlaVerify(entity.lat >= -90.0 && entity.lat <= 90.0
                 && entity.lon >= -180.0 && entity.lon <= 180.0,
                 "Invalid geodetic coordinates",
                 path, lineNumber);

        // Geocentric Cartesian position.

        HlaConvertLatLonAltToGcc(
            entity.lat,
            entity.lon,
            entity.alt,
            entity.x,
            entity.y,
            entity.z);

        // The seven fields of the EntityType attribute follow.

        // EntityKind.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read EntityKind", path, lineNumber);

        errno = 0;
        entity.entityType.entityKind = (unsigned char) 
                                       strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse EntityKind",
            path,
            lineNumber);

        // Domain.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Domain", path, lineNumber);

        errno = 0;
        entity.entityType.domain = (unsigned char) 
                                   strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Domain",
            path,
            lineNumber);

        // CountryCode.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read CountryCode", path, lineNumber);

        errno = 0;
        entity.entityType.countryCode = (unsigned short)
                                        strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse CountryCode",
            path,
            lineNumber);

        // Category.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Category", path, lineNumber);

        errno = 0;
        entity.entityType.category = (unsigned char)
                                     strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Category",
            path,
            lineNumber);

        // Subcategory.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Subcategory", path, lineNumber);

        errno = 0;
        entity.entityType.subcategory = (unsigned char)
                                        strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Subcategory",
            path,
            lineNumber);

        // Specific.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Specific", path, lineNumber);

        errno = 0;
        entity.entityType.specific = (unsigned char)
                                     strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Specific",
            path,
            lineNumber);

        // Extra.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Extra", path, lineNumber);

        errno = 0;
        entity.entityType.extra = (unsigned char)
                                  strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Extra",
            path,
            lineNumber);

        // Miscellaneous.

        entity.orientationPsi   = 0.0;
        entity.orientationTheta = 0.0;
        entity.orientationPhi   = 0.0;

        entity.azimuth   = 0;
        entity.elevation = 0;

        entity.xVelocity = 0.0;
        entity.yVelocity = 0.0;
        entity.zVelocity = 0.0;

        entity.damageState = HLA_DAMAGE_STATE_NO_DAMAGE;
        entity.numRadioPtrs = 0;
    }//for//
}

void
HlaReadRadiosFile()
{
    // Determine path of .hla-radios file.

    char path[g_hlaPathBufSize];

    sprintf(path, "%s.hla-radios", g_hlaFedAmb->m_scenarioName);

    cout << ".hla-radios file   = " << path << "." << endl;

    // Determine number of lines in file.

    g_hlaFedAmb->m_numRadios = HlaGetNumLinesInFile(path);
    if (g_hlaFedAmb->m_numRadios == 0) { return; }

    g_hlaFedAmb->MallocRadios();

    // Open file.

    FILE* fpRadios = fopen(path, "r");
    HlaVerify(fpRadios != NULL, "Can't open for reading", path);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    char* p;
    char* next;
    unsigned i;
    unsigned lineNumber;
    for (i = 0, lineNumber = 1;
         i < g_hlaFedAmb->m_numRadios;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, lineBufSize, fpRadios) != NULL,
                  "Not enough lines",
                  path);

        HlaVerify(strlen(line) < lineBufSize - 1,
                  "Exceeds permitted line length",
                  path);

        next = line;

        HlaRadio& radio = g_hlaFedAmb->m_radios[i];

        // nodeId.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read nodeId", path, lineNumber);

        char* endPtr = NULL;
        errno = 0;
        radio.nodeId = (unsigned) strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse nodeId",
            path,
            lineNumber);

        HlaVerify(
            g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&radio.nodeId) == NULL,
            "Radio with duplicate nodeId",
            path, lineNumber);

        HlaHashItem& radioPtrItem
            = g_hlaFedAmb->m_nodeIdToRadioPtrHash.AddItem(&radio.nodeId);

        HlaRadio*& radioPtr = *((HlaRadio**) radioPtrItem.GetDataPtr());
        radioPtr = &radio;

        // MarkingData.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read MarkingData", path, lineNumber);

        IO_TrimLeft(token);
        IO_TrimRight(token);

        HlaVerify(strlen(token) < g_hlaMarkingDataBufSize,
                 "MarkingData too large",
                 path, lineNumber);

        // Assign radio's entity pointer using MarkingData.

        HlaHashItem* entityPtrItemPtr
            = g_hlaFedAmb->m_markingDataToEntityPtrHash.GetItem(token);

        HlaVerify(entityPtrItemPtr != NULL,
                  "Can't find entity with MarkingData",
                  path, lineNumber);

        HlaEntity& entity
            = **((HlaEntity**) entityPtrItemPtr->GetDataPtr());

        radio.entityPtr = &entity;

        // Assign radio pointer to host entity.

        HlaVerify(entity.numRadioPtrs < g_hlaMaxRadiosPerEntity,
                  "Exceeded max radios per entity");

        entity.radioPtrs[entity.numRadioPtrs] = &radio;
        entity.numRadioPtrs++;

        // RadioIndex.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read RadioIndex", path, lineNumber);

        errno = 0;
        radio.radioIndex = (unsigned short) strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RadioIndex",
            path,
            lineNumber);

        // MarkingData + RadioIndex to radio pointer hash.

        HlaRadioKey radioKey;
        strcpy(radioKey.markingData, entity.markingData);
        radioKey.radioIndex = radio.radioIndex;

        HlaVerify(
            g_hlaFedAmb->m_radioKeyToRadioPtrHash.GetItem(&radioKey) == NULL,
            "Radio with duplicate MarkingData and RadioIndex",
            path, lineNumber);

        HlaHashItem& radioPtrItem2
            = g_hlaFedAmb->m_radioKeyToRadioPtrHash.AddItem(&radioKey);
        HlaRadio*& radioPtr2 = *((HlaRadio**) radioPtrItem2.GetDataPtr());
        radioPtr2 = &radio;

        // RelativePosition.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(
            p != NULL,
            "Can't read RelativePosition (x)",
            path,
            lineNumber);

        errno = 0;
        radio.relativePositionX = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (x)",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(
            p != NULL,
            "Can't read RelativePosition (y)",
            path,
            lineNumber);

        errno = 0;
        radio.relativePositionY = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (y)",
            path,
            lineNumber);

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(
            p != NULL,
            "Can't read RelativePosition (z)",
            path,
            lineNumber);

        errno = 0;
        radio.relativePositionZ = (float) strtod(token, &endPtr);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse RelativePosition (z)",
            path,
            lineNumber);

        // The seven fields of the RadioSystemType attribute follow.

        // EntityKind.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read EntityKind", path, lineNumber);

        errno = 0;
        radio.radioSystemType.entityKind = (unsigned char)
                                           strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse EntityKind",
            path,
            lineNumber);

        // Domain.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Domain", path, lineNumber);

        errno = 0;
        radio.radioSystemType.domain = (unsigned char)
                                       strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Domain",
            path,
            lineNumber);

        // CountryCode.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read CountryCode", path, lineNumber);

        errno = 0;
        radio.radioSystemType.countryCode = (unsigned short)
                                            strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse CountryCode",
            path,
            lineNumber);

        // Category.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Category", path, lineNumber);

        errno = 0;
        radio.radioSystemType.category = (unsigned char)
                                         strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Category",
            path,
            lineNumber);

        // NomenclatureVersion.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read NomenclatureVersion",
                  path, lineNumber);

        errno = 0;
        radio.radioSystemType.nomenclatureVersion 
            = (unsigned char) strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse NomenclatureVersion",
            path,
            lineNumber);

        // Nomenclature.

        p = IO_GetDelimitedToken(token, next, ",", &next);
        HlaVerify(p != NULL, "Can't read Nomenclature", path, lineNumber);

        errno = 0;
        radio.radioSystemType.nomenclature = (unsigned char)
                                             strtoul(token, &endPtr, 10);
        HlaVerify(
            endPtr != token && errno == 0,
            "Can't parse Nomenclature",
            path,
            lineNumber);

        // Miscellaneous.

        radio.txOperationalStatus
            = HLA_TX_OPERATIONAL_STATUS_ON_BUT_NOT_TRANSMITTING;

        radio.networkPtr = NULL;
    }//for//
}

void
HlaReadNetworksFile()
{
    // Determine path of .hla-networks file.

    char path[g_hlaPathBufSize];

    sprintf(path, "%s.hla-networks", g_hlaFedAmb->m_scenarioName);

    cout << ".hla-networks file = " << path << "." << endl
         << endl;

    // Determine number of lines in file.

    g_hlaFedAmb->m_numNetworks = HlaGetNumLinesInFile(path);
    if (g_hlaFedAmb->m_numNetworks == 0) { return; }

    g_hlaFedAmb->MallocNetworks();

    // Open file.

    FILE* fpNetworks = fopen(path, "r");
    HlaVerify(fpNetworks != NULL, "Can't open for reading", path);

    // Read file.

    const unsigned lineBufSize = 512;
    char line[lineBufSize];
    char token[lineBufSize];
    char nodeIdToken[lineBufSize];
    char* p;
    char* next;
    char* nextNodeIdToken;
    unsigned i;
    unsigned lineNumber = 0;
    for (i = 0, lineNumber = 1;
         i < g_hlaFedAmb->m_numNetworks;
         i++, lineNumber++)
    {
        HlaVerify(fgets(line, lineBufSize, fpNetworks) != NULL,
                 "Not enough lines",
                 path);

        HlaVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 path);

        HlaNetwork& network = g_hlaFedAmb->m_networks[i];

        // Name of network.

        p = IO_GetDelimitedToken(token, line, ";\n", &next);
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
            HlaVerify(
                network.numRadioPtrs < g_hlaMaxMembersInNetwork,
                "Exceeded maximum number of nodes for network",
                path,
                lineNumber);

            char* endPtr = NULL;
            errno = 0;
            unsigned nodeId = (unsigned) strtoul(nodeIdToken, &endPtr, 10);
            HlaVerify(
                endPtr != token && errno == 0,
                "Couldn't parse nodeId",
                path,
                lineNumber);

            assert(g_hlaFedAmb->m_numRadios > 0);
            unsigned j;
            for (j = 0; j < g_hlaFedAmb->m_numRadios; j++)
            {
                HlaRadio& radio = g_hlaFedAmb->m_radios[j];
                if (radio.nodeId == nodeId) { break; }
            }

            HlaVerify(
                j != g_hlaFedAmb->m_numRadios,
                "Can't find nodeId in list of radios",
                path, lineNumber);

            HlaRadio& radio = g_hlaFedAmb->m_radios[j];

            network.radioPtrs[network.numRadioPtrs] = &radio;

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

#if 0
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
#endif /* 0 */
    }//for//
}

unsigned
HlaGetNumLinesInFile(const char* path)
{
    FILE* fp = fopen(path, "r");
    HlaVerify(fp != NULL, "Can't open for reading", path);

    const unsigned lineBufSize = 1024;
    char line[lineBufSize];
    unsigned numLines = 0;
    while (1)
    {
        if (fgets(line, lineBufSize, fp) == NULL) { break; }

        numLines++;

        HlaVerify(strlen(line) < lineBufSize - 1,
                 "Exceeds permitted line length",
                 path);
    }

    fclose(fp);

    return numLines;
}

void
HlaCreateFederation()
{
    cout << "FED: Trying to create federation "
            << g_hlaFedAmb->m_federationName << " ... " << endl;

    try
    {
        g_hlaRtiAmb->createFederationExecution(g_hlaFedAmb->m_federationName,
                                            g_hlaFedAmb->m_fedFilePath);
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
HlaJoinFederation()
{
    cout << "FED: Trying to join federate " << g_hlaFedAmb->m_federateName
            << " to federation " << g_hlaFedAmb->m_federationName
            << " ... " << endl;

    while (1)
    {
        try
        {
            g_hlaRtiAmb->joinFederationExecution(
                g_hlaFedAmb->m_federateName,
                g_hlaFedAmb->m_federationName,
                g_hlaFedAmb);
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
HlaGetObjectAndInteractionClassHandles()
{
    try
    {
        // BaseEntity object.

        g_hlaFedAmb->m_baseEntityHandle
            = g_hlaRtiAmb->getObjectClassHandle("BaseEntity");


        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_beEntityIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "EntityIdentifier", g_hlaFedAmb->m_baseEntityHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_beEntityIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "EntityID", g_hlaFedAmb->m_baseEntityHandle);
        }
        else { assert(0); }

        g_hlaFedAmb->m_beEntityTypeHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "EntityType", g_hlaFedAmb->m_baseEntityHandle);
        g_hlaFedAmb->m_beOrientationHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "Orientation", g_hlaFedAmb->m_baseEntityHandle);
        g_hlaFedAmb->m_beVelocityVectorHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "VelocityVector", g_hlaFedAmb->m_baseEntityHandle);

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_beWorldLocationHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "WorldLocation", g_hlaFedAmb->m_baseEntityHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_beWorldLocationHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "Position", g_hlaFedAmb->m_baseEntityHandle);
        }
        else { assert(0); }

        // BaseEntity.PhysicalEntity object (RPR FOM 1.0),
        // BaseEntity.MilitaryEntity object (RPR FOM 0.5)

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_physicalEntityHandle
                = g_hlaRtiAmb->getObjectClassHandle(
                      "BaseEntity.PhysicalEntity");
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_physicalEntityHandle
                = g_hlaRtiAmb->getObjectClassHandle(
                    "BaseEntity.PhysicalEntity.MilitaryEntity");
        }
        else { assert(0); }

        g_hlaFedAmb->m_peDamageStateHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "DamageState", g_hlaFedAmb->m_physicalEntityHandle);

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_peForceIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "ForceIdentifier", g_hlaFedAmb->m_physicalEntityHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_peForceIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "ForceID", g_hlaFedAmb->m_physicalEntityHandle);
        }
        else { assert(0); }

        g_hlaFedAmb->m_peMarkingHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "Marking", g_hlaFedAmb->m_physicalEntityHandle);

        // EmbeddedSystem object.

        g_hlaFedAmb->m_embeddedSystemHandle
            = g_hlaRtiAmb->getObjectClassHandle("EmbeddedSystem");

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_esEntityIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "EntityIdentifier", g_hlaFedAmb->m_embeddedSystemHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_esEntityIdentifierHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "EntityID", g_hlaFedAmb->m_embeddedSystemHandle);
        }
        else { assert(0); }

        g_hlaFedAmb->m_esRelativePositionHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "RelativePosition", g_hlaFedAmb->m_embeddedSystemHandle);

        // EmbeddedSystem.RadioTransmitter object.

        g_hlaFedAmb->m_radioTransmitterHandle
            = g_hlaRtiAmb->getObjectClassHandle(
                  "EmbeddedSystem.RadioTransmitter");
        g_hlaFedAmb->m_rtFrequencyHandle
            = g_hlaRtiAmb->getAttributeHandle(
                  "Frequency", g_hlaFedAmb->m_radioTransmitterHandle);

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_rtRadioIndexHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "RadioIndex", g_hlaFedAmb->m_radioTransmitterHandle);
            g_hlaFedAmb->m_rtRadioSystemTypeHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "RadioSystemType",
                      g_hlaFedAmb->m_radioTransmitterHandle);
            g_hlaFedAmb->m_rtTransmitterOperationalStatusHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "TransmitterOperationalStatus",
                      g_hlaFedAmb->m_radioTransmitterHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_rtRadioIndexHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "RadioNumber", g_hlaFedAmb->m_radioTransmitterHandle);
            g_hlaFedAmb->m_rtRadioSystemTypeHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "RadioType",
                      g_hlaFedAmb->m_radioTransmitterHandle);
            g_hlaFedAmb->m_rtTransmitterOperationalStatusHandle
                = g_hlaRtiAmb->getAttributeHandle(
                      "TransmitState",
                      g_hlaFedAmb->m_radioTransmitterHandle);
        }
        else { assert(0); }

        // RadioSignal.ApplicationSpecificRadioSignal interaction.

        g_hlaFedAmb->m_applicationSpecificRadioSignalHandle
            = g_hlaRtiAmb->getInteractionClassHandle(
                  "RadioSignal.ApplicationSpecificRadioSignal");

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_asrsHostRadioIndexHandle
                = g_hlaRtiAmb->getParameterHandle(
                      "HostRadioIndex",
                      g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_asrsHostRadioIndexHandle
                = g_hlaRtiAmb->getParameterHandle(
                      "HostRadioID",
                      g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        }
        else { assert(0); }

        g_hlaFedAmb->m_asrsDataRateHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "DataRate",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        g_hlaFedAmb->m_asrsSignalDataLengthHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "SignalDataLength",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        g_hlaFedAmb->m_asrsSignalDataHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "SignalData",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        g_hlaFedAmb->m_asrsTacticalDataLinkTypeHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "TacticalDataLinkType",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        g_hlaFedAmb->m_asrsTdlMessageCountHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "TDLMessageCount",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);
        g_hlaFedAmb->m_asrsUserProtocolIdHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "UserProtocolID",
                  g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);

        // Data interaction.

        g_hlaFedAmb->m_dataHandle
            = g_hlaRtiAmb->getInteractionClassHandle("Data");
        g_hlaFedAmb->m_dataOriginatingEntityHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "OriginatingEntity", g_hlaFedAmb->m_dataHandle);
        g_hlaFedAmb->m_dataReceivingEntityHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "ReceivingEntity", g_hlaFedAmb->m_dataHandle);

        if (g_hlaFedAmb->m_rprFomVersion == 1.0)
        {
            g_hlaFedAmb->m_dataRequestIdentifierHandle
                = g_hlaRtiAmb->getParameterHandle(
                      "RequestIdentifier", g_hlaFedAmb->m_dataHandle);
        }
        else
        if (g_hlaFedAmb->m_rprFomVersion == 0.5)
        {
            g_hlaFedAmb->m_dataRequestIdentifierHandle
                = g_hlaRtiAmb->getParameterHandle(
                      "RequestID", g_hlaFedAmb->m_dataHandle);
        }
        else { assert(0); }

        g_hlaFedAmb->m_dataFixedDatumsHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "FixedDatums", g_hlaFedAmb->m_dataHandle);
        g_hlaFedAmb->m_dataVariableDatumSetHandle
            = g_hlaRtiAmb->getParameterHandle(
                  "VariableDatumSet", g_hlaFedAmb->m_dataHandle);

    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaSubscribeAndPublish()
{
    // Objects.

    try
    {
        RTI::AttributeHandleSet* set
            = RTI::AttributeHandleSetFactory::create(8);

        set->add(g_hlaFedAmb->m_beEntityIdentifierHandle);
        set->add(g_hlaFedAmb->m_beEntityTypeHandle);
        set->add(g_hlaFedAmb->m_beOrientationHandle);
        set->add(g_hlaFedAmb->m_beVelocityVectorHandle);
        set->add(g_hlaFedAmb->m_beWorldLocationHandle);
        set->add(g_hlaFedAmb->m_peDamageStateHandle);
        set->add(g_hlaFedAmb->m_peForceIdentifierHandle);
        set->add(g_hlaFedAmb->m_peMarkingHandle);

        g_hlaRtiAmb->publishObjectClass(
            g_hlaFedAmb->m_physicalEntityHandle, *set);

        delete set;
        set = NULL;

        set = RTI::AttributeHandleSetFactory::create(6);

        set->add(g_hlaFedAmb->m_esEntityIdentifierHandle);
        set->add(g_hlaFedAmb->m_esRelativePositionHandle);
        set->add(g_hlaFedAmb->m_rtFrequencyHandle);
        set->add(g_hlaFedAmb->m_rtRadioIndexHandle);
        set->add(g_hlaFedAmb->m_rtRadioSystemTypeHandle);
        set->add(g_hlaFedAmb->m_rtTransmitterOperationalStatusHandle);

        g_hlaRtiAmb->publishObjectClass(
            g_hlaFedAmb->m_radioTransmitterHandle, *set);

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
        g_hlaRtiAmb->publishInteractionClass(
            g_hlaFedAmb->m_applicationSpecificRadioSignalHandle);

        g_hlaRtiAmb->subscribeInteractionClass(g_hlaFedAmb->m_dataHandle);
        g_hlaRtiAmb->publishInteractionClass(g_hlaFedAmb->m_dataHandle);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaLeaveFederation()
{
    try
    {
        g_hlaRtiAmb->resignFederationExecution(
            RTI::DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

void
HlaDestroyFederation()
{
    try
    {
        g_hlaRtiAmb->destroyFederationExecution(g_hlaFedAmb->m_federationName);
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
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}

// TODO-HIGH:  Sort.

void
HlaRegisterObjects()
{
    // PhysicalEntity objects.

    unsigned i;
    for (i = 0; i < g_hlaFedAmb->m_numEntities; i++)
    {
        HlaEntity& entity = g_hlaFedAmb->m_entities[i];

        try
        {
            entity.handle = g_hlaRtiAmb->registerObjectInstance(
                               g_hlaFedAmb->m_physicalEntityHandle);
        }
        catch (RTI::Exception& e)
        {
            cerr << "FED: " << &e << endl;
            exit(EXIT_FAILURE);
        }
    }

    // RadioTransmitter objects.

    for (i = 0; i < g_hlaFedAmb->m_numRadios; i++)
    {
        HlaRadio& radio = g_hlaFedAmb->m_radios[i];

        try
        {
            radio.handle = g_hlaRtiAmb->registerObjectInstance(
                               g_hlaFedAmb->m_radioTransmitterHandle);
        }
        catch (RTI::Exception& e)
        {
            cerr << "FED: " << &e << endl;
            exit(EXIT_FAILURE);
        }
    }

    g_hlaFedAmb->m_objectsRegistered = true;
    cout << "FED: Objects registered." << endl;
}

void
HlaUpdateAttributes()
{
    // PhysicalEntity objects.

    RTI::AttributeHandleValuePairSet* set = NULL;

    try
    {
        set = RTI::AttributeSetFactory::create(8);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    unsigned i;
    for (i = 0; i < g_hlaFedAmb->m_numEntities; i++)
    {
        const HlaEntity& entity = g_hlaFedAmb->m_entities[i];

        try
        {
            // EntityID.

            HlaEntityId nboEntityId = entity.entityId;
            HlaHtonEntityId(nboEntityId);

            set->add(g_hlaFedAmb->m_beEntityIdentifierHandle,
                     (char*) &nboEntityId,
                     sizeof(nboEntityId));

            // EntityType.

            HlaEntityType nboEntityType = entity.entityType;

            HlaHton(&nboEntityType.countryCode, sizeof(nboEntityType.countryCode));

            set->add(g_hlaFedAmb->m_beEntityTypeHandle,
                     (char*) &nboEntityType,
                     sizeof(nboEntityType));

            // Orientation.

            float nboOrientation[3];

            nboOrientation[0] = entity.orientationPsi;
            nboOrientation[1] = entity.orientationTheta;
            nboOrientation[2] = entity.orientationPhi;

            HlaHton(&nboOrientation[0], sizeof(nboOrientation[0]));
            HlaHton(&nboOrientation[1], sizeof(nboOrientation[1]));
            HlaHton(&nboOrientation[2], sizeof(nboOrientation[2]));

            set->add(g_hlaFedAmb->m_beOrientationHandle,
                     (char*) nboOrientation,
                     sizeof(nboOrientation));

            // VelocityVector.

            float nboVelocityVector[3];

            nboVelocityVector[0] = entity.xVelocity;
            nboVelocityVector[1] = entity.yVelocity;
            nboVelocityVector[2] = entity.zVelocity;

            HlaHton(&nboVelocityVector[0], sizeof(nboVelocityVector[0]));
            HlaHton(&nboVelocityVector[1], sizeof(nboVelocityVector[1]));
            HlaHton(&nboVelocityVector[2], sizeof(nboVelocityVector[2]));

            set->add(g_hlaFedAmb->m_beVelocityVectorHandle,
                     (char*) nboVelocityVector,
                     sizeof(nboVelocityVector));

            // WorldLocation.

            double nboWorldLocation[3];

            nboWorldLocation[0] = entity.x;
            nboWorldLocation[1] = entity.y;
            nboWorldLocation[2] = entity.z;

            HlaHton(&nboWorldLocation[0], sizeof(nboWorldLocation[0]));
            HlaHton(&nboWorldLocation[1], sizeof(nboWorldLocation[1]));
            HlaHton(&nboWorldLocation[2], sizeof(nboWorldLocation[2]));

            set->add(g_hlaFedAmb->m_beWorldLocationHandle,
                     (char*) nboWorldLocation,
                     sizeof(nboWorldLocation));

            // DamageState.

            unsigned nboDamageState = entity.damageState;
            HlaHton(&nboDamageState, sizeof(nboDamageState));

            set->add(g_hlaFedAmb->m_peDamageStateHandle,
                     (char*) &nboDamageState,
                     sizeof(nboDamageState));

            // ForceID.

            unsigned char nboForceId = entity.forceId;

            set->add(g_hlaFedAmb->m_peForceIdentifierHandle,
                     (char*) &nboForceId,
                      sizeof(nboForceId));

            // Marking.

            char nboMarking[g_hlaMarkingBufSize];

            const char asciiMarkingEncodingType = 1;
            nboMarking[0] = asciiMarkingEncodingType;
            strcpy(&nboMarking[1], entity.markingData);

            set->add(g_hlaFedAmb->m_peMarkingHandle,
                     nboMarking,
                     sizeof(nboMarking));

            // RTI-ambassador call.

            g_hlaRtiAmb->updateAttributeValues(entity.handle, *set, NULL);

            set->empty();
        }
        catch (RTI::Exception& e)
        {
            cerr << "FED: " << &e << endl;
            exit(EXIT_FAILURE);
        }
    }//for//

    delete set;
    set = NULL;

    // RadioTransmitter objects.

    try
    {
        set = RTI::AttributeSetFactory::create(6);
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    for (i = 0; i < g_hlaFedAmb->m_numRadios; i++)
    {
        const HlaRadio& radio = g_hlaFedAmb->m_radios[i];

        try
        {
            // EntityID.

            assert(radio.entityPtr != NULL);
            const HlaEntity& entity = *radio.entityPtr;

            HlaEntityId nboEntityId;
            memcpy(&nboEntityId, &entity.entityId, sizeof(nboEntityId));
            HlaHtonEntityId(nboEntityId);

            set->add(g_hlaFedAmb->m_esEntityIdentifierHandle,
                     (char*) &nboEntityId,
                     sizeof(nboEntityId));

            // RelativePosition.

            float nboRelativePosition[3] =
                { radio.relativePositionX,
                  radio.relativePositionY,
                  radio.relativePositionZ };

            HlaHton(&nboRelativePosition[0], sizeof(nboRelativePosition[0]));
            HlaHton(&nboRelativePosition[1], sizeof(nboRelativePosition[1]));
            HlaHton(&nboRelativePosition[2], sizeof(nboRelativePosition[2]));

            set->add(g_hlaFedAmb->m_esRelativePositionHandle,
                     (char*) nboRelativePosition,
                     sizeof(nboRelativePosition));

            // Frequency.

            uint64 nboFrequency;
            assert(radio.networkPtr != NULL);
            memcpy(&nboFrequency,
                   &radio.networkPtr->frequency,
                   sizeof(nboFrequency));
            HlaHton(&nboFrequency, sizeof(nboFrequency));

            set->add(g_hlaFedAmb->m_rtFrequencyHandle,
                     (char*) &nboFrequency,
                     sizeof(nboFrequency));

            // RadioIndex.

            unsigned short nboRadioIndex;
            memcpy(&nboRadioIndex, &radio.radioIndex, sizeof(nboRadioIndex));
            HlaHton(&nboRadioIndex, sizeof(nboRadioIndex));

            set->add(g_hlaFedAmb->m_rtRadioIndexHandle,
                     (char*) &nboRadioIndex,
                     sizeof(nboRadioIndex));

            // RadioSystemType.

            HlaRadioSystemType nboRadioSystemType = radio.radioSystemType;

            HlaHton(&nboRadioSystemType.countryCode,
                    sizeof(nboRadioSystemType.countryCode));
            HlaHton(&nboRadioSystemType.nomenclature,
                    sizeof(nboRadioSystemType.nomenclature));

            set->add(g_hlaFedAmb->m_rtRadioSystemTypeHandle,
                     (char*) &nboRadioSystemType,
                     sizeof(nboRadioSystemType));

            // TransmitterOperationalStatus.

            set->add(g_hlaFedAmb->m_rtTransmitterOperationalStatusHandle,
                     (char*) &radio.txOperationalStatus,
                     sizeof(radio.txOperationalStatus));

            // RTI-ambassador call.

            g_hlaRtiAmb->updateAttributeValues(radio.handle, *set, NULL);

            set->empty();
        }
        catch (RTI::Exception& e)
        {
            cerr << "FED: " << &e << endl;
            exit(EXIT_FAILURE);
        }
    }//for//

    delete set;
    set = NULL;

    cout << "FED: Object attributes updated." << endl;
}

void
HlaAssignObjectNames()
{
    unsigned i;
    for (i = 0; i < g_hlaFedAmb->m_numRadios; i++)
    {
        HlaRadio& radio = g_hlaFedAmb->m_radios[i];

        char* objectName = g_hlaRtiAmb->getObjectInstanceName(radio.handle);

        assert(objectName != NULL);

        HlaVerify(strlen(objectName) + 1 <= sizeof(radio.objectName),
                  "Object name too big for buffer",
                  __FILE__, __LINE__);

        strcpy(radio.objectName, objectName);
        delete [] objectName;
    }
}

void
HlaPrepareMessageString(
    HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo,
    const HlaEntity* dstEntityPtr,
    bool isData,
    unsigned msgSize,
    double timeoutDelay)
{
    const char newLine = '\n';
    const char nullTerminator = 0;

    char s[MAX_STRING_LENGTH];

    unsigned sdOffset = 0;
    char *signalData = asrsIxnInfo.signalData;

    HlaCopyToOffset(
        signalData,
        sdOffset,
        "HEADER\n",
        strlen("HEADER\n"));

    // Receiver.

    if (dstEntityPtr != NULL)
    {
        HlaCopyToOffset(
            signalData,
            sdOffset,
            "receiver=",
            strlen("receiver="));

        HlaCopyToOffset(
            signalData,
            sdOffset,
            dstEntityPtr->entityIdString,
            strlen(dstEntityPtr->entityIdString));

        HlaCopyToOffset(
            signalData,
            sdOffset,
            &newLine,
            sizeof(newLine));
    }

    // Size.

    HlaCopyToOffset(
        signalData,
        sdOffset,
        "size=",
        strlen("size="));

    sprintf(s, "%u ", msgSize);

    HlaCopyToOffset(
        signalData,
        sdOffset,
        s,
        strlen(s));

    if (isData)
    {
        HlaCopyToOffset(
            signalData,
            sdOffset,
            "bytes",
            strlen("bytes"));
    }
    else
    {
        HlaCopyToOffset(
            signalData,
            sdOffset,
            "seconds",
            strlen("seconds"));
    }

    HlaCopyToOffset(
        signalData,
        sdOffset,
        &newLine,
        sizeof(newLine));

    // Timeout.

    HlaCopyToOffset(
        signalData,
        sdOffset,
        "timeout=",
        strlen("timeout="));

    sprintf(s, "%.3f", timeoutDelay);

    HlaCopyToOffset(
        signalData,
        sdOffset,
        s,
        strlen(s));

    HlaCopyToOffset(
        signalData,
        sdOffset,
        &newLine,
        sizeof(newLine));

    // Timestamp.

    HlaCopyToOffset(
        signalData,
        sdOffset,
        "timestamp=",
        strlen("timestamp="));

    sprintf(s, "0x%08x", HlaGetTimestamp());

    HlaCopyToOffset(
        signalData,
        sdOffset,
        s,
        strlen(s));

    HlaCopyToOffset(
        signalData,
        sdOffset,
        &newLine,
        sizeof(newLine));

    HlaCopyToOffset(
        signalData,
        sdOffset,
        "EOH\nDATA\nEOD\n",
        strlen("EOH\nDATA\nEOD\n"));

    HlaCopyToOffset(
        signalData,
        sdOffset,
        &nullTerminator,
        sizeof(nullTerminator));
}

void
HlaSendApplicationSpecificRadioSignalIxn(
    const HlaApplicationSpecificRadioSignalIxnInfo& asrsIxnInfo)
{
    try
    {
        RTI::ParameterHandleValuePairSet* set
            = RTI::ParameterSetFactory::create(7);

        // HostRadioIndex.

        set->add(
            g_hlaFedAmb->m_asrsHostRadioIndexHandle,
            asrsIxnInfo.hostRadioIndex,
            strlen(asrsIxnInfo.hostRadioIndex) + 1);

        // DataRate.
        // Converted to network-byte-order.

        unsigned nboDataRate;
        nboDataRate = asrsIxnInfo.dataRate;

        HlaHton(&nboDataRate, sizeof(nboDataRate));

        set->add(
            g_hlaFedAmb->m_asrsDataRateHandle,
            (char*) &nboDataRate,
            sizeof(nboDataRate));

        // SignalDataLength.
        // Converted to network-byte-order.

        unsigned short nboSignalDataLength;
        nboSignalDataLength = asrsIxnInfo.signalDataLength;

        HlaHton(&nboSignalDataLength, sizeof(nboSignalDataLength));

        set->add(
            g_hlaFedAmb->m_asrsSignalDataLengthHandle,
            (char*) &nboSignalDataLength,
            sizeof(nboSignalDataLength));

        // SignalData.

        assert(asrsIxnInfo.signalDataLength % 8 == 0);

        // (Prevent warning with gcc 3.0.4.)
        // Cast to unsigned (int) before comparison.

        assert((unsigned) (asrsIxnInfo.signalDataLength / 8)
               == strlen(asrsIxnInfo.signalData) + 1);

        set->add(
            g_hlaFedAmb->m_asrsSignalDataHandle,
            asrsIxnInfo.signalData,
            asrsIxnInfo.signalDataLength / 8);

        // TacticalDataLinkType.

        unsigned short nboTacticalDataLinkType = 0;

        HlaHton(&nboTacticalDataLinkType, sizeof(nboTacticalDataLinkType));

        set->add(
            g_hlaFedAmb->m_asrsTacticalDataLinkTypeHandle,
            (char*) &nboTacticalDataLinkType,
            sizeof(nboTacticalDataLinkType));

        // TDLMessageCount.

        unsigned short nboTdlMessageCount = 0;

        HlaHton(&nboTdlMessageCount, sizeof(nboTdlMessageCount));

        set->add(
            g_hlaFedAmb->m_asrsTdlMessageCountHandle,
            (char*) &nboTdlMessageCount,
            sizeof(nboTdlMessageCount));

        // UserProtocolID.

        unsigned nboUserProtocolId;
        nboUserProtocolId = asrsIxnInfo.userProtocolId;

        HlaHton(&nboUserProtocolId, sizeof(nboUserProtocolId));

        set->add(
            g_hlaFedAmb->m_asrsUserProtocolIdHandle,
            (char*) &nboUserProtocolId,
            sizeof(nboUserProtocolId));

        g_hlaRtiAmb->sendInteraction(
            g_hlaFedAmb->m_applicationSpecificRadioSignalHandle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }
}
