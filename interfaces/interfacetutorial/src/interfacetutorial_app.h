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

#ifndef _INTERFACE_TUTORIAL_APP_H_
#define _INTERFACE_TUTORIAL_APP_H_

struct AppDataInterfaceTutorial
{
    EXTERNAL_Interface *iface;
    char data[256];
};

/*
 * NAME:        AppLayerInterfaceTutorial
 * PURPOSE:     InterfaceTutorial message handling function
 * PARAMETERS:  node - pointer to the node which received the message.
 *              msg - message received by the layer
 * RETURN:      none.
 */
void AppLayerInterfaceTutorial(Node *node, Message *msg);

/*
 * NAME:        AppInterfaceTutorialInit
 * PURPOSE:     Initialize an InterfaceTutorial application
 * PARAMETERS:  node - pointer to the node,
 *              data - the initial data for this app
 * RETURN:      none.
 */
void AppInterfaceTutorialInit(
    Node *node,
    char *data);

/*
 * NAME:        AppInterfaceTutorialFinalize
 * PURPOSE:     Finalize an InterfaceTutorial application
 * PARAMETERS:  node - pointer to the node.
 *              appInfo - pointer to the application info data structure.
 * RETURN:      none.
 */
void AppInterfaceTutorialFinalize(Node *node, AppInfo* appInfo);

#endif /* _INTERFACE_TUTORIAL_APP_H_ */
