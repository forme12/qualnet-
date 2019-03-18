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

#include "api.h"
#include "partition.h"
#include "external_util.h"
#include "interfacetutorial.h"

//---------------------------------------------------------------------------
// External Interface API Functions
//---------------------------------------------------------------------------

void InterfaceTutorialInitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput)
{
    InterfaceTutorialData *data;
    EXTERNAL_SocketErrorType err;

    // Allocate memory for interface-specific data.  The allocated memory
    // is verified by MEM_malloc.  Set the iface->data variable to the
    // newly allocated data for future use.
    data = (InterfaceTutorialData*)
           MEM_malloc(sizeof(InterfaceTutorialData));
    iface->data = (void*) data;

    // Initialize a listening socket and a data socket
    EXTERNAL_SocketInit(&data->listenSocket, FALSE);
    EXTERNAL_SocketInit(&data->s, FALSE);

    // Listen for a socket connection on port 5132.  The newly opened socket
    // connection will be returned in the data->s socket structure.
    printf("Listening for socket connection on port 5132...\n");
    err = EXTERNAL_SocketListen(
        &data->listenSocket,
        5132,
        &data->s);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportError("Error listening for socket connection");
    }

    printf("InitializeNodes function called\n");
}

void InterfaceTutorialReceive(EXTERNAL_Interface *iface)
{
    EXTERNAL_SocketErrorType err;
    InterfaceTutorialData *data;
    char error[MAX_STRING_LENGTH];
    char in[256];
    NodeAddress srcNodeId;
    NodeAddress destNodeId;
    NodeAddress srcAddr;
    NodeAddress destAddr;
    char payload[256];
    unsigned int size;

    // Extract the interface-specific data
    data = (InterfaceTutorialData*) iface->data;

    printf("Receive function called\n");

    // Keep receiving data while availabile
    do
    {
        // Attempt to receive up to 256 bytes from the data socket.
        // Store the results in the "in" variable.  The FALSE parameter
        // specifies that this recv operation will not block.
        err = EXTERNAL_SocketRecv(
            &data->s,
            in,
            256,
            &size,
            FALSE);
        if (err != EXTERNAL_NoSocketError)
        {
            ERROR_ReportError("Error receiving data from socket");
        }

        // If data was received
        if (size > 0)
        {
            // Parse the command in the form:
            // <src node id> <dest node id> <payload>
            // Invalid node ids will be checked in the
            // EXTERNAL_SendDataAppLayerUDP function.  Invalid payloads
            // will be checked in the APP_INTERFACETUTORIAL functions.
            memset(payload, 0, 256);
            sscanf(in, "%d %d %255c", &srcNodeId, &destNodeId, payload);

            // Get node addresses
            srcAddr = MAPPING_GetDefaultInterfaceAddressFromNodeId(
                iface->partition->firstNode,
                srcNodeId);
            destAddr = MAPPING_GetDefaultInterfaceAddressFromNodeId(
                iface->partition->firstNode,
                destNodeId);

            // Verify valid pointers
            if (srcAddr == INVALID_MAPPING || destAddr == INVALID_MAPPING)
            {
                ERROR_ReportWarning("Invalid address for interfacetutorial");
                continue;
            }

            // Send the payload from src to dest node.  The "0" specifies
            // that this packet should be sent immediately.  The destination
            // application is APP_INTERFACETUTORIAL.
            EXTERNAL_SendDataAppLayerUDP(
                iface,
                srcAddr,
                destAddr,
                payload,
                strlen(payload) + 1,
                0,
                APP_INTERFACETUTORIAL,
                TRACE_INTERFACETUTORIAL);
        }
    } while (size > 0);
}

void InterfaceTutorialForward(
    EXTERNAL_Interface *iface,
    Node* node,
    void *forwardData,
    int forwardSize)
{
    EXTERNAL_SocketErrorType err;
    InterfaceTutorialData *data;

    printf("Forward function called\n");
    printf("    forwarding \"%s\"\n", (char*) forwardData);

    // Extract interface-specific data
    data = (InterfaceTutorialData*) iface->data;

    // Send forwarded information on the data socket
    err = EXTERNAL_SocketSend(
        &data->s, 
        (char*) forwardData,
        forwardSize);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportError("Error sending data on socket");
    }
}

void InterfaceTutorialFinalize(EXTERNAL_Interface *iface)
{
    InterfaceTutorialData *data;
    EXTERNAL_SocketErrorType err;

    // Extract interface-specific data
    data = (InterfaceTutorialData*) iface->data;
    
    // Close the data socket
    err = EXTERNAL_SocketClose(&data->s);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportError("Error closing socket");
    }

    // Close the listening socket
    err = EXTERNAL_SocketClose(&data->listenSocket);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportError("Error closing socket");
    }
}
