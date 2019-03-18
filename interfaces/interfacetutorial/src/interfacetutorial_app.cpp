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
#include "app_util.h"
#include "interfacetutorial.h"
#include "interfacetutorial_app.h"

// #define DEBUG

/*
 * NAME:        AppInterfaceTutorialGet
 * PURPOSE:     search for an InterfaceTutorial data structure
 * PARAMETERS:  node - pointer to the node.
 * RETURN:      the pointer to the cbr client data structure,
 *              NULL if nothing found.
 */
AppDataInterfaceTutorial* AppInterfaceTutorialGet(Node *node)
{
    AppInfo *appList = node->appData.appPtr;

    // Search through all applications for APP_INTERFACETUTORIAL
    for (; appList != NULL; appList = appList->appNext)
    {
        if (appList->appType == APP_INTERFACETUTORIAL)
        {
            return (AppDataInterfaceTutorial*) appList->appDetail;
        }
    }

    // If not found return NULL
    return NULL;
}

/*
 * NAME:        AppInterfaceTutorialNew
 * PURPOSE:     create a new InterfaceTutorial data structure, place it
 *              at the beginning of the application list.
 * PARAMETERS:  node - pointer to the node
 *              data - the initial application data
 * RETURN:      the pointer to the created cbr client data structure,
 *              NULL if no data structure allocated.
 */
AppDataInterfaceTutorial* AppInterfaceTutorialNew(Node *node, char *data)
{
    char error[MAX_STRING_LENGTH];
    AppDataInterfaceTutorial *appData;

    // Allocate new memory for this application
    appData = (AppDataInterfaceTutorial*)
                MEM_malloc(sizeof(AppDataInterfaceTutorial));
    memset(appData, 0, sizeof(AppDataInterfaceTutorial));

    // Copy in the initial data
    strcpy(appData->data, data);

    // Lookup the InterfaceTutorial external interface
    appData->iface = EXTERNAL_GetInterfaceByName(
        &node->partitionData->interfaceList,
        "InterfaceTutorial");
    if (appData->iface == NULL)
    {
        sprintf(error, "Interface \"InterfaceTutorial\" not found");
        ERROR_ReportError(error);
    }

    // Register the application on the node
    APP_RegisterNewApp(node, APP_INTERFACETUTORIAL, appData);

    // Return the application data
    return appData;
}
/*
 * NAME:        AppLayerInterfaceTutorial
 * PURPOSE:     InterfaceTutorial message handling function
 * PARAMETERS:  node - pointer to the node which received the message.
 *              msg - message received by the layer
 * RETURN:      none.
 */
void AppLayerInterfaceTutorial(Node *node, Message *msg)
{
    char *newData;
    char buf[MAX_STRING_LENGTH];
    char error[MAX_STRING_LENGTH];
    AppDataInterfaceTutorial *appData;
    char *payload;
    int payloadSize;

    // Get the application on the node
    appData = AppInterfaceTutorialGet(node);
    if (appData == NULL)
    {
        sprintf(error, "No InterfaceTutorial app on node %d", node->nodeId);
#ifdef EXATA
        return;
#else
        ERROR_ReportError(error);
#endif
    }

    switch(msg->eventType)
    {
        // UDP data received
        case MSG_APP_FromTransport:
            payload = MESSAGE_ReturnPacket(msg);
            payloadSize = msg->packetSize;

            // Check that the payload is not empty
            if (strlen(payload) == 0)
            {
                sprintf(buf, "Node %d received empty payload",
                        node->nodeId);
                ERROR_ReportWarning(buf);
                break;
            }

            if (payload[0] == 's')
            {
                // If the payload starts with an 's' then this is a send
                // command.  First check that there is valid data after the
                // 's'.
                if (strlen(payload) <= 2)
                {
                    ERROR_ReportWarning("Send payload too small");
                    break;
                }

                // The new data will be the third character of the payload.
                // Ie, "s newdata"
                newData = &payload[2];

                // Set the new data
                printf("Node %d changed data from \"%s\" to \"%s\"\n",
                    node->nodeId,
                    appData->data,
                    newData);
                strcpy(appData->data, newData);
            }
            else if (payload[0] == 'g')
            {
                // If the payload begins with a 'g' this is a get command.
                // Forward this node's data to the external interface.
                sprintf(buf, "Node %d data is '%s'",
                        node->nodeId,
                        appData->data);
                EXTERNAL_ForwardData(appData->iface,
                                     node,
                                     buf,
                                     strlen(buf) + 1);
            }
            break;

        default:
           TIME_PrintClockInSecond(getSimTime(node), buf);
           sprintf(error, "InterfaceTutorial: at time %sS, node %d "
                   "received message of unknown type"
                   " %d\n", buf, node->nodeId, msg->eventType);
           ERROR_ReportError(error);
    }

    MESSAGE_Free(node, msg);
}

/*
 * NAME:        AppInterfaceTutorialInit
 * PURPOSE:     Initialize an InterfaceTutorial application
 * PARAMETERS:  node - pointer to the node,
 *              data - the initial data for this app
 * RETURN:      none.
 */
void AppInterfaceTutorialInit(
    Node *node,
    char *data)
{
    char error[MAX_STRING_LENGTH];
    AppDataInterfaceTutorial *appData;

    // Create a new InterfaceTutorial app
    appData = AppInterfaceTutorialNew(node, data);
    if (appData == NULL)
    {
        sprintf(error,
                "InterfaceTutorial: Node %d cannot allocate memory for "
                "new client\n", node->nodeId);
        ERROR_ReportError(error);
    }
}

/*
 * NAME:        AppInterfaceTutorialFinalize
 * PURPOSE:     Finalize an InterfaceTutorial application
 * PARAMETERS:  node - pointer to the node.
 *              appInfo - pointer to the application info data structure.
 * RETURN:      none.
 */
void AppInterfaceTutorialFinalize(Node *node, AppInfo* appInfo)
{
    // Nothing to do here
}
