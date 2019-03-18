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

#ifndef _INTERFACE_TUTORIAL_H_
#define _INTERFACE_TUTORIAL_H_

#include "external.h"
#include "external_socket.h"

// /**
// STRUCT      :: InterfaceTutorialData
// DESCRIPTION :: Data that is used for this external interface
// **/
struct InterfaceTutorialData
{
    EXTERNAL_Socket listenSocket;
    EXTERNAL_Socket s;
};

//---------------------------------------------------------------------------
// External Interface API Functions
//---------------------------------------------------------------------------

// /**
// API       :: InterfaceTutorialInitializeNodes
// PURPOSE   :: This will listen for a socket connection on port 5132
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The interface
// + nodeInput : NodeInput* : The configuration file data
// RETURN    :: void
// **/
void InterfaceTutorialInitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);

// /**
// API       :: InterfaceTutorialReceive
// PURPOSE   :: This function will receive packets through the opened socket
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The interface
// RETURN    :: void
// **/
void InterfaceTutorialReceive(EXTERNAL_Interface *iface);

// /**
// API       :: InterfaceTutorialForward
// PURPOSE   :: This function will send packets through the opened socket
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The interface
// + node : Node* : The node forwarding the data
// + forwardData : void* : A pointer to the data to forward
// + forwardSize : int : The size of the data
// RETURN    :: void
// **/
void InterfaceTutorialForward(
    EXTERNAL_Interface *iface,
    Node* node,
    void *forwardData,
    int forwardSize);

// /**
// API       :: InterfaceTutorialFinalize
// PURPOSE   :: This function will finalize this interface
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The interface
// RETURN    :: void
// **/
void InterfaceTutorialFinalize(EXTERNAL_Interface *iface);

#endif /* _INTERFACE_TUTORIAL_H_ */
