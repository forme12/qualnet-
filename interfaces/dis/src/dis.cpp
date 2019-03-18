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

#ifdef _WIN32
#include <winsock2.h>

#define socklen_t      int
#else /* _WIN32 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BSD_COMP
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define ioctlsocket    ioctl

#ifndef INADDR_NONE
#define INADDR_NONE    -1
#endif /* INADDR_NONE */

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define SD_SEND        SHUT_WR
#endif /* _WIN32 */

#include "dis_shared.h"

bool g_disActive;

void
DisReadDisParameter(const NodeInput* nodeInput)
{
    BOOL retVal;
    char buf[MAX_STRING_LENGTH];

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "DIS",
        &retVal,
        buf);

    if (retVal && strcmp(buf, "YES") == 0)
    {
        g_disActive = true;

        cout << endl
             << "DIS support enabled." << endl;
    }
    else
    {
        g_disActive = false;
    }
}

bool
DisIsActive()
{
    return g_disActive;
}

void
DisInit(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    DisInitDisVariable(iface);

    cout << endl;

    DisReadParameters(iface, nodeInput);
}

void
DisInitNodes(EXTERNAL_Interface* iface, NodeInput* nodeInput)
{
    cout << endl;

    DisReadEntitiesFile(iface, nodeInput);
    DisReadRadiosFile(iface, nodeInput);
    DisReadNetworksFile(iface, nodeInput);

    DisMapHierarchyIds(nodeInput);

    DisInitMessenger(iface);

    // Open connection.

#ifdef _WIN32
    DisInitWin32Sockets();
#endif /* _WIN32 */

    DisCreateUdpSocket(g_dis.sd);

    DisSetSO_REUSEADDR(g_dis.sd);
    DisSetSO_BROADCAST(g_dis.sd);

    DisBindSocket(g_dis.sd, INADDR_ANY, g_dis.disPort);

    if (g_dis.disIpAddressIsMulticast)
    {
        DisJoinMulticastGroup(g_dis.sd, g_dis.disIpAddress, INADDR_ANY);
    }

    DisSetSocketToNonBlocking(g_dis.sd);

    cout << endl;

    DisWaitForFirstEntityStatePdu(iface);

    cout << endl;

    DisRegisterCtrlCHandler();
}

void
DisReceive(EXTERNAL_Interface* iface)
{
    const clocktype startTime = EXTERNAL_QueryExternalTime(iface);

    char payload[g_maxUdpPayloadSize];
    unsigned payloadSize;

    while (1)
    {
        if (!DisAttemptRecv(
                 g_dis.sd,
                 payload,
                 sizeof(payload),
                 payloadSize))
        {
            // recv() reported no pending UDP datagrams, so return now.

            return;
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

               DisProcessPdu(&payload[index], length);

               index += length;

               if (index == payloadSize) { break; }

               assert(index < payloadSize);
            }//while//
        }//if//

        // Check if time in recv() loop has been exceeded.

        clocktype timeInLoop = EXTERNAL_QueryExternalTime(iface) - startTime;
        assert(timeInLoop >= 0);

        if (timeInLoop >= g_dis.maxReceiveDuration)
        {
            // Spent too much time in recv() loop, so return now to let
            // QualNet do other things (besides polling the socket).

            return;
        }
    }//while//
}

void
DisProcessEvent(Node* node, Message* msg)
{
    switch(msg->eventType)
    {
        case MSG_EXTERNAL_DIS_HierarchyMobility:
            DisProcessHierarchyMobilityEvent(node, msg);
            break;
        case MSG_EXTERNAL_DIS_ChangeMaxTxPower:
            DisProcessChangeMaxTxPowerEvent(node, msg);
            break;
        case MSG_EXTERNAL_DIS_SimulatedMsgTimeout:
            DisProcessTimeoutEvent(node, msg);
            break;
#ifdef MILITARY_RADIOS_LIB
        case MSG_EXTERNAL_DIS_SendRtss:
            DisProcessSendRtssEvent(node, msg);
            break;
#endif /* MILITARY_RADIOS_LIB */
        default:
            break;
    }

    MESSAGE_Free(node, msg);
}

void
DisFinalize(EXTERNAL_Interface* iface)
{

    DisCloseUdpSocket(g_dis.sd);

#ifdef _WIN32
    DisCleanupWin32Sockets();
#endif /* _WIN32 */

    DisFreeRadios();
    DisFreeEntities();
}

#ifdef MILITARY_RADIOS_LIB
void
DisSendRtssNotificationIfNodeIsDisEnabled(Node* node)
{
    DisNodeIdToPerNodeDataMap::iterator it
        = g_dis.nodeIdToPerNodeData.find(node->nodeId);

    if (it == g_dis.nodeIdToPerNodeData.end())
    {
        // node does not map to DIS entity
        return;
    }

    DisSendRtssNotification(node);
}
#endif /* MILITARY_RADIOS_LIB */
