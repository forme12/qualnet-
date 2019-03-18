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

#include <cstdlib>
#include <cassert>
#include <cerrno>

#include "hla_fileio.h"

#include "RTI.hh"

#include "hla_HashTable.h"
#include "hla_gcclatlon.h"
#include "hla_orientation.h"
#include "hla_archspec.h"
#include "hla_rpr_fom.h"
#include "hla_shared.h"
#include "hla_hashfcns.h"
#include "hla_FedAmb.h"
#include "hla_main.h"
#include "hla.h"

const double HLA_PI = 3.14159265358979323846264338328;

const double HLA_RADIANS_PER_DEGREE = (HLA_PI / 180.0);
const double HLA_DEGREES_PER_RADIAN = (180.0 / HLA_PI);

int
main(int argc, char* argv[])
{
    HlaValidateConstants();

    HlaMallocFederateAmbassador();
    HlaMallocRtiAmbassador();

    ProcessCommandLineArguments(argc, argv);

    HlaReadEntitiesFile();
    HlaReadRadiosFile();

    HlaReadNetworksFile();

    HlaCreateFederation();
    HlaSleep(1);
    HlaJoinFederation();
    HlaGetObjectAndInteractionClassHandles();
    HlaSubscribeAndPublish();

    EnterCommandLoop();

    HlaLeaveFederation();
    HlaDestroyFederation();

    HlaFreeFederateAmbassador();
    HlaFreeRtiAmbassador();

    return 0;
}

void
ProcessCommandLineArguments(unsigned argc, char* argv[])
{
    g_hlaFedAmb->m_debug = false;
    g_hlaFedAmb->m_debug2 = false;
    strcpy(g_hlaFedAmb->m_federationName, "RPR-FOM");
    strcpy(g_hlaFedAmb->m_fedFilePath, "RPR-FOM.fed");
    strcpy(g_hlaFedAmb->m_federateName, "testfed");
    g_hlaFedAmb->m_rprFomVersion = 1.0;

    g_hlaFedAmb->m_scenarioName[0] = 0;

    unsigned i;
    for (i = 1; i < argc; i++)
    {
        char* arg = argv[i];

        if (arg[0] == '-' || arg[0] == '/')
        {
            // This argument has a - or / as the first character.
            // Skip over this character and process the rest of the argument.

            arg++;

            if (arg[0] == 'h' || arg[0] == '?')
            {
                ShowUsage();
                exit(EXIT_SUCCESS);
            }
            else
            if (arg[0] == 'd')
            {
                g_hlaFedAmb->m_debug = true;
            }
            else
            if (arg[0] == 'f')
            {
                i++;

                if (i == argc)
                {
                    ShowUsage();
                    exit(EXIT_FAILURE);
                }

                arg = argv[i];

                HlaVerify(
                    strlen(arg) + 1 <= sizeof(g_hlaFedAmb->m_federationName),
                    "Federation name too long");

                strcpy(g_hlaFedAmb->m_federationName, arg);
            }
            else
            if (arg[0] == 'F')
            {
                i++;

                if (i == argc)
                {
                    ShowUsage();
                    exit(EXIT_FAILURE);
                }

                arg = argv[i];

                HlaVerify(
                    strlen(arg) + 1 <= sizeof(g_hlaFedAmb->m_fedFilePath),
                    "FED-file path too long");

                strcpy(g_hlaFedAmb->m_fedFilePath, arg);
            }
            else
            if (arg[0] == 'r')
            {
                g_hlaFedAmb->m_rprFomVersion = 0.5;
            }
            else
            {
                ShowUsage();
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            HlaVerify(
                strlen(arg) + 1 <= sizeof(g_hlaFedAmb->m_scenarioName),
                "Scenario name too long");

            strcpy(g_hlaFedAmb->m_scenarioName, arg);
        }//if//
    }//for//

    if (g_hlaFedAmb->m_scenarioName[0] == 0)
    {
        ShowUsage();
        exit(EXIT_FAILURE);
    }
}

void
ShowUsage()
{
    cout << "HLA RPR-FOM 1.0 test federate" << endl
         << endl
         << "Syntax:" << endl
         << endl
         << "    testfed [options] base-filename" << endl
         << endl
         << "    (Default values in parentheses)" << endl
         << "    -h" << endl
         << "\tHelp" << endl
         << "    -d" << endl
         << "\tDebug mode" << endl
         << "    -f federation-name" << endl
         << "\tSet federation name (RPR-FOM)" << endl
         << "    -F FED-file" << endl
         << "\tSet FED file (RPR-FOM.fed)" << endl
         << "    -r" << endl
         << "\tUse RPR FOM 0.5" << endl;
}

void
EnterCommandLoop()
{
    ShowCommands();
    cout << endl;

    //ListNetworks();
    //cout << endl;

    char inputLine[g_hlaUserInputLineBufSize];
    char firstToken[g_hlaUserInputLineBufSize];
    char* next;
    char command;

    while (1)
    {
        cout << "> ";
        cin.getline(inputLine, sizeof(inputLine));
        firstToken[0] = 0;
        IO_GetToken(firstToken, inputLine, &next);
        command = firstToken[0];

        if (command == 0)
        {
            // No input.
        }
        else
        if (command == '?' || tolower(command) == 'h')
        {
            ShowCommands();
        }
        else
        if (command == 'r')
        {
            RegisterObjects();
        }
        else
        if (command == 'm')
        {
            MoveEntity(next);
        }
        else
        if (command == 'o')
        {
            ChangeEntityOrientation(next);
        }
        else
        if (command == 'v')
        {
            ChangeEntityVelocity(next);
        }
        else
        if (command == 'l')
        {
            //ListNetworks();
        }
        else
        if (command == 's')
        {
            SendCommEffectsRequest(next);
        }
        else
        if (command == 'd')
        {
            ChangeDamageState(next);
        }
        else
        if (command == 't')
        {
            ChangeTxOperationalStatus(next);
        }
        else
        if (command == 'T')
        {
            //SendTerminate();
        }
        else
        if (command == 'z')
        {
            //ShowEntities(next);
        }
        else
        if (command == 'q')
        {
            //SendTerminate();
            break;
        }
        else
        {
            cout << "Command ignored." << endl;
        }
        g_hlaRtiAmb->tick();
    }//while//
}

void
ShowCommands()
{
    cout << "Commands" << endl
         << endl
         << "?  This list" << endl
         << "r  Register objects" << endl
         << "m  Move entity (moves the entity that hosts the radio)" << endl
         << "     m nodeId lat lon z" << endl
         << "o  Change entity orientation" << endl
         << "   (changes orientation for all radios hosted by entity)" << endl
         << "     o nodeId psi theta phi (in degrees)" << endl
         << "v  Change entity velocity (affects the entity that hosts the"
            " radio" << endl
         << "     v nodeId xVelocity yVelocity zVelociy (meters / sec)" << endl
         //<< "l  List networks" << endl
         << "s  Send Comm Effects Request" << endl
         << "     s srcNodeId [[V]msgSize] [timeoutDelay] [dstNodeId]" << endl
         << "       (V = voice traffic)" << endl
         << "     Defaults to 100 bytes, 10 seconds, no dstNodeId" << endl
         << "d  Change entity DamageState" << endl
         << "     d nodeId 0-3 (0 = Not Damaged, ..., 3 = Destroyed)" << endl
         << "t  Change radio TransmitterOperationalStatus" << endl
         << "     t nodeId 0-2 (0 = Off, 1 = OnButNotTransmitting,"
                               " 2 = OnAndTransmitting)" << endl
         //<< "T  Terminate QualNet" << endl
         //<< "z  Status" << endl
         //<< "     z [startNodeId[-endNodeId]]" << endl
         << "q  Exit program" << endl;
}

void
RegisterObjects()
{
    if (g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects already registered." << endl;
        return;
    }

    HlaRegisterObjects();

    // Sleep for one second to let the RTI perform necessary processing.
    // This shouldn't be necessary, but DMSO RTI-NG 1.3v6 needs this time
    // or it will drop the subsequent attribute updates.

    HlaSleep(1);

    HlaUpdateAttributes();

    HlaAssignObjectNames();
}

void
MoveEntity(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned nodeId;
    double lat;
    double lon;
    double alt;

    if (!ParseMoveEntityCommand(next, nodeId, lat, lon, alt))
    { return; }

    HlaHashItem* radioPtrItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&nodeId);

    if (radioPtrItemPtr == NULL)
    {
        cout << "Unknown nodeId." << endl;
        return;
    }

    const HlaRadio& radio = **((HlaRadio**) radioPtrItemPtr->GetDataPtr());

    assert(radio.entityPtr != NULL);

    HlaEntity& entity = *radio.entityPtr;

    entity.lat = lat;
    entity.lon = lon;
    entity.alt = alt;

    HlaConvertLatLonAltToGcc(
        entity.lat, entity.lon, entity.alt,
        entity.x, entity.y, entity.z);

    try
    {
        RTI::AttributeHandleValuePairSet* set
            = RTI::AttributeSetFactory::create(1);

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

        g_hlaRtiAmb->updateAttributeValues(entity.handle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Moved (" << entity.entityIdString << ") "
            << entity.markingData << " to ("
            << entity.lat << ", "
            << entity.lon << ", "
            << entity.alt << ")." << endl;
}

bool
ParseMoveEntityCommand(
    char* next,
    unsigned& nodeId,
    double& lat,
    double& lon,
    double& alt)
{
    char token[g_hlaUserInputLineBufSize];

    token[0] = 0;
    IO_GetToken(token, next, &next);

    char* endPtr = NULL;
    errno = 0;
    nodeId = (unsigned) strtoul(token, &endPtr, 10);

    if (token[0] == 0)
    {
        cout << "Expecting nodeId." << endl;
        return false;
    }

    if (endPtr == token || errno != 0)
    {
        cout << "Bad nodeId." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting latitude." << endl;
        return false;
    }

    errno = 0;
    lat = strtod(token, &endPtr);

    if (endPtr == token || errno != 0 || lat < -90.0 || lat > 90.0)
    {
        cout << "Bad latitude." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting longitude." << endl;
        return false;
    }

    errno = 0;
    lon = strtod(token, &endPtr);

    if (endPtr == token || errno != 0 || lon < -180.0 || lon > 180.0)
    {
        cout << "Bad longitude." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting altitude." << endl;
        return false;
    }

    errno = 0;
    alt = strtod(token, &endPtr);

    if (endPtr == token || errno != 0)
    {
        cout << "Bad altitude." << endl;
        return false;
    }

    return true;
}

void
ChangeEntityOrientation(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned nodeId;
    double orientationPsiDegrees;
    double orientationThetaDegrees;
    double orientationPhiDegrees;

    if (!ParseChangeEntityOrientationCommand(
             next,
             nodeId,
             orientationPsiDegrees,
             orientationThetaDegrees,
             orientationPhiDegrees))
    { return; }

    float orientationPsi
        = (float) (orientationPsiDegrees * HLA_RADIANS_PER_DEGREE);
    float orientationTheta
        = (float) (orientationThetaDegrees * HLA_RADIANS_PER_DEGREE);
    float orientationPhi
        = (float) (orientationPhiDegrees * HLA_RADIANS_PER_DEGREE);

    HlaHashItem* radioPtrItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&nodeId);

    if (radioPtrItemPtr == NULL)
    {
        cout << "Unknown nodeId." << endl;
        return;
    }

    const HlaRadio& radio = **((HlaRadio**) radioPtrItemPtr->GetDataPtr());

    assert(radio.entityPtr != NULL);

    HlaEntity& entity = *radio.entityPtr;

    short azimuth;
    short elevation;

    HlaConvertRprFomOrientationToQualNetOrientation(
        entity.lat,
        entity.lon,
        orientationPsi,
        orientationTheta,
        orientationPhi,
        azimuth,
        elevation);

    entity.orientationPsi   = orientationPsi;
    entity.orientationTheta = orientationTheta;
    entity.orientationPhi   = orientationPhi;

    entity.azimuth   = azimuth;
    entity.elevation = elevation;

    try
    {
        RTI::AttributeHandleValuePairSet* set
            = RTI::AttributeSetFactory::create(1);

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

        g_hlaRtiAmb->updateAttributeValues(entity.handle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Changed (" << entity.entityIdString << ") "
            << entity.markingData << " orientation to ("
            << entity.orientationPsi << ","
            << entity.orientationTheta << ","
            << entity.orientationPhi << ") (radians)." << endl;
}

bool
ParseChangeEntityOrientationCommand(
    char* next,
    unsigned& nodeId,
    double& orientationPsiDegrees,
    double& orientationThetaDegrees,
    double& orientationPhiDegrees)
{
    char token[g_hlaUserInputLineBufSize];

    token[0] = 0;
    IO_GetToken(token, next, &next);

    char* endPtr = NULL;
    errno = 0;
    nodeId = (unsigned) strtoul(token, &endPtr, 10);

    if (token[0] == 0)
    {
        cout << "Expecting nodeId." << endl;
        return false;
    }

    if (endPtr == token || errno != 0)
    {
        cout << "Bad nodeId." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting psi." << endl;
        return false;
    }

    errno = 0;
    orientationPsiDegrees = (float) strtod(token, &endPtr);

    if (endPtr == token
        || errno != 0
        || orientationPsiDegrees < -180.0
        || orientationPsiDegrees > 180.0)
    {
        cout << "Bad psi." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting theta." << endl;
        return false;
    }

    errno = 0;
    orientationThetaDegrees = strtod(token, &endPtr);

    if (endPtr == token
        || errno != 0
        || orientationThetaDegrees < -180.0
        || orientationThetaDegrees > 180.0)
    {
        cout << "Bad theta." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting phi." << endl;
        return false;
    }

    errno = 0;
    orientationPhiDegrees = strtod(token, &endPtr);

    if (endPtr == token
        || errno != 0
        || orientationPhiDegrees < -180.0
        || orientationPhiDegrees > 180.0)
    {
        cout << "Bad phi." << endl;
        return false;
    }

    return true;
}

void
ChangeEntityVelocity(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned nodeId;
    float xVelocity;
    float yVelocity;
    float zVelocity;

    if (!ParseChangeEntityVelocityCommand(
             next,
             nodeId,
             xVelocity,
             yVelocity,
             zVelocity))
    { return; }

    HlaHashItem* radioPtrItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&nodeId);

    if (radioPtrItemPtr == NULL)
    {
        cout << "Unknown nodeId." << endl;
        return;
    }

    const HlaRadio& radio = **((HlaRadio**) radioPtrItemPtr->GetDataPtr());

    assert(radio.entityPtr != NULL);
    HlaEntity& entity = *radio.entityPtr;

    entity.xVelocity = xVelocity;
    entity.yVelocity = yVelocity;
    entity.zVelocity = zVelocity;

    try
    {
        RTI::AttributeHandleValuePairSet* set
            = RTI::AttributeSetFactory::create(1);

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

        g_hlaRtiAmb->updateAttributeValues(entity.handle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Changed (" << entity.entityIdString << ") "
            << entity.markingData << " velocity to ("
            << entity.xVelocity << ","
            << entity.yVelocity << ","
            << entity.zVelocity << ") (m/s)." << endl;
}

bool
ParseChangeEntityVelocityCommand(
    char* next,
    unsigned& nodeId,
    float& xVelocity,
    float& yVelocity,
    float& zVelocity)
{
    char token[g_hlaUserInputLineBufSize];

    token[0] = 0;
    IO_GetToken(token, next, &next);

    char* endPtr = NULL;
    errno = 0;
    nodeId = (unsigned) strtoul(token, &endPtr, 10);

    if (token[0] == 0)
    {
        cout << "Expecting nodeId." << endl;
        return false;
    }

    if (endPtr == token || errno != 0)
    {
        cout << "Bad nodeId." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting x velocity." << endl;
        return false;
    }

    errno = 0;
    xVelocity = (float) strtod(token, &endPtr);

    if (endPtr == token || errno != 0)
    {
        cout << "Can't parse x velocity." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting y velocity." << endl;
        return false;
    }

    errno = 0;
    yVelocity = (float) strtod(token, &endPtr);

    if (endPtr == token || errno != 0)
    {
        cout << "Can't parse y velocity." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetDelimitedToken(token, next, " ,()", &next);

    if (token[0] == 0)
    {
        cout << "Expecting z velocity." << endl;
        return false;
    }

    errno = 0;
    zVelocity = (float) strtod(token, &endPtr);

    if (endPtr == token || errno != 0)
    {
        cout << "Can't parse z velocity." << endl;
        return false;
    }

    return true;
}

void
SendCommEffectsRequest(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned srcNodeId;
    bool     isData;
    unsigned msgSize;
    double   timeoutDelay;
    bool     dstNodeIdPresent;
    unsigned dstNodeId;

    if (!ParseSendCommEffectsRequestCommand(
            next,
            srcNodeId,
            isData,
            msgSize,
            timeoutDelay,
            dstNodeIdPresent,
            dstNodeId))
    { return; }

    HlaHashItem* srcRadioItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&srcNodeId);

    if (srcRadioItemPtr == NULL)
    {
        cout << "Unknown src nodeId." << endl;
        return;
    }

    const HlaRadio& srcRadio = **((HlaRadio**) srcRadioItemPtr->GetDataPtr());

    const HlaEntity* dstEntityPtr = NULL;

    if (dstNodeIdPresent)
    {
        HlaHashItem* dstRadioItemPtr
            = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&dstNodeId);

        if (dstRadioItemPtr == NULL)
        {
            cout << "Unknown dst nodeId." << endl;
            return;
        }

        const HlaRadio& dstRadio = **((HlaRadio**) dstRadioItemPtr->GetDataPtr());

        assert(dstRadio.entityPtr != NULL);

        dstEntityPtr = dstRadio.entityPtr;
    }

    HlaApplicationSpecificRadioSignalIxnInfo asrsIxnInfo;

    strcpy(asrsIxnInfo.hostRadioIndex, srcRadio.objectName);

    asrsIxnInfo.dataRate = 1000000;

    HlaPrepareMessageString(
        asrsIxnInfo,
        dstEntityPtr,
        isData,
        msgSize,
        timeoutDelay);

    asrsIxnInfo.signalDataLength = 8 * (strlen(asrsIxnInfo.signalData) + 1);

    const unsigned cerUserProtocolId = 10000;

    asrsIxnInfo.userProtocolId = cerUserProtocolId;

    HlaSendApplicationSpecificRadioSignalIxn(asrsIxnInfo);
}

bool
ParseSendCommEffectsRequestCommand(
    char*       next,
    unsigned&   srcNodeId,
    bool&       isData,
    unsigned&   msgSize,
    double&     timeoutDelay,
    bool&       dstNodeIdPresent,
    unsigned&   dstNodeId)
{
    char token[g_hlaUserInputLineBufSize];

    // srcNodeId.

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        cout << "Expecting srcNodeId." << endl;
        return false;
    }

    char* endPtr = NULL;
    errno = 0;
    srcNodeId = (unsigned) strtoul(token, &endPtr, 10);
    if (endPtr == token || errno != 0)
    {
        cout << "Bad srcNodeId." << endl;
        return false;
    }

    // isData, msgSize.

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        const bool defaultIsData = true;
        const unsigned defaultMsgSize = 100;

        isData = defaultIsData;
        msgSize = defaultMsgSize;
    }
    else
    {
        if (isalpha(token[0]))
        {
            if (toupper(token[0]) != 'V' || token[1] == 0)
            {
                cout << "Bad msgSize for data (use DmsgSize format)."
                        << endl;
                return false;
            }

            isData = false;

            errno = 0;
            msgSize = (unsigned) strtoul(&token[1], &endPtr, 10);

            if (endPtr == &token[1] || errno != 0)
            {
                cout << "Bad msgSize for data (use [V]msgSize format)."
                        << endl;
                return false;
            }
        }
        else
        {
            isData = true;

            errno = 0;
            msgSize = (unsigned) strtoul(token, &endPtr, 10);

            if (endPtr == token || errno != 0)
            {
                cout << "Bad msgSize." << endl;
                return false;
            }
        }
    }

    // timeoutDelay.

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        const double defaultTimeoutDelay = 10.0;

        timeoutDelay = defaultTimeoutDelay;
    }
    else
    {
        errno = 0;
        timeoutDelay = strtod(token, &endPtr);

        if (endPtr == token || errno != 0 || timeoutDelay < 0.0)
        {
            cout << "Bad timeoutDelay." << endl;
            return false;
        }
    }

    // dstNodeId.

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        dstNodeIdPresent = false;
    }
    else
    {
        dstNodeIdPresent = true;

        errno = 0;
        dstNodeId = (unsigned) strtoul(token, &endPtr, 10);

        if (endPtr == token || errno != 0)
        {
            cout << "Bad dstNodeId." << endl;
            return false;
        }
    }

    return true;
}

void
ChangeDamageState(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned nodeId;
    unsigned damageState;

    if (!ParseChangeDamageStateCommand(next, nodeId, damageState))
    { return; }

    HlaHashItem* radioItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&nodeId);

    if (radioItemPtr == NULL)
    {
        cout << "Unknown nodeId." << endl;
        return;
    }

    const HlaRadio& radio = **((HlaRadio**) radioItemPtr->GetDataPtr());
    assert(radio.entityPtr != NULL);
    HlaEntity& entity = *radio.entityPtr;

    if (entity.damageState == damageState) { return; }
    entity.damageState = damageState;

    try
    {
        RTI::AttributeHandleValuePairSet* set
            = RTI::AttributeSetFactory::create(1);

        unsigned nboDamageState = damageState;
        HlaHton(&nboDamageState, sizeof(nboDamageState));

        set->add(g_hlaFedAmb->m_peDamageStateHandle,
                 (char*) &nboDamageState,
                 sizeof(nboDamageState));

        g_hlaRtiAmb->updateAttributeValues(entity.handle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    cout << "Set (" << entity.entityIdString
            << ") " << entity.markingData
            << " DamageState to " << damageState << "." << endl;
}

bool
ParseChangeDamageStateCommand(
    char* next,
    unsigned& nodeId,
    unsigned& damageState)
{
    char token[g_hlaUserInputLineBufSize];

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        cout << "Expecting nodeId." << endl;
        return false;
    }

    char* endPtr = NULL;
    errno = 0;
    nodeId = (unsigned) strtoul(token, &endPtr, 10);
    if (endPtr == token || errno != 0)
    {
        cout << "Bad nodeId." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        cout << "Expecting DamageState." << endl;
        return false;
    }

    errno = 0;
    damageState = (unsigned) strtoul(token, &endPtr, 10);
    if (endPtr == token || errno != 0 || damageState > HLA_DAMAGE_STATE_MAX)
    {
        cout << "Bad DamageState." << endl;
        return false;
    }

    return true;
}

void
ChangeTxOperationalStatus(char* next)
{
    if (!g_hlaFedAmb->m_objectsRegistered)
    {
        cout << "Objects need to be registered first." << endl;
        return;
    }

    unsigned nodeId;
    unsigned char txOperationalStatus;

    if (!ParseChangeTxOperationalStatusCommand(
             next, nodeId, txOperationalStatus))
    { return; }

    HlaHashItem* radioItemPtr
        = g_hlaFedAmb->m_nodeIdToRadioPtrHash.GetItem(&nodeId);

    if (radioItemPtr == NULL)
    {
        cout << "Unknown nodeId." << endl;
        return;
    }

    HlaRadio& radio = **((HlaRadio**) radioItemPtr->GetDataPtr());

    if (radio.txOperationalStatus == txOperationalStatus) { return; }
    radio.txOperationalStatus = txOperationalStatus;

    try
    {
        RTI::AttributeHandleValuePairSet* set
            = RTI::AttributeSetFactory::create(1);

        set->add(g_hlaFedAmb->m_rtTransmitterOperationalStatusHandle,
                 (char*) &txOperationalStatus,
                 sizeof(txOperationalStatus));

        g_hlaRtiAmb->updateAttributeValues(radio.handle, *set, NULL);

        delete set;
        set = NULL;
    }
    catch (RTI::Exception& e)
    {
        cerr << "FED: " << &e << endl;
        exit(EXIT_FAILURE);
    }

    assert(radio.entityPtr != NULL);
    HlaEntity& entity = *radio.entityPtr;

    cout << "Set (" << entity.entityIdString
            << ", " << radio.radioIndex << ") " << entity.markingData
            << " TransmitterOperationalStatus to "
            << (unsigned) txOperationalStatus << "." << endl;
}

bool
ParseChangeTxOperationalStatusCommand(
    char* next,
    unsigned& nodeId,
    unsigned char& txOperationalStatus)
{
    char token[g_hlaUserInputLineBufSize];

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        cout << "Expecting nodeId." << endl;
        return false;
    }

    char* endPtr = NULL;
    errno = 0;
    nodeId = (unsigned) strtoul(token, &endPtr, 10);
    if (endPtr == token || errno != 0)
    {
        cout << "Bad nodeId." << endl;
        return false;
    }

    token[0] = 0;
    IO_GetToken(token, next, &next);

    if (token[0] == 0)
    {
        cout << "Expecting TransmitterOperationalStatus." << endl;
        return false;
    }

    errno = 0;
    txOperationalStatus = (unsigned char) strtoul(token, &endPtr, 10);
    if (endPtr == token
        || errno != 0
        || txOperationalStatus > HLA_TX_OPERATIONAL_STATUS_MAX)
    {
        cout << "Bad TransmitterOperationalStatus." << endl;
        return false;
    }

    return true;
}
