// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.
#ifndef CONTROL_MESSAGES_H
#define CONTROL_MESSAGES_H

enum ControlMessage {
    /* 0x0 - 0x999 reserved for Simulation Layer */
    CtrlMsg_Sim_PauseSimulation = 0x0,
    CtrlMsg_Sim_ResumeSimulation,
    CtrlMsg_Sim_FastForward,
    CtrlMsg_Sim_StopSimulation,
    
    /* 0x1000 - 0x1999 reserved for Application  Layer */
    
    /* 0x2000 - 0x2999 reserved for Transport  Layer */

    /* 0x3000 - 0x3999 reserved for Network  Layer */
    CtrlMsg_Network_GetRoute = 0x3000,
    CtrlMsg_Network_SetRoute,
    CtrlMsg_Network_DeleteRoute,
    CtrlMsg_Network_SetBufferSize,
    CtrlMsg_Network_GetBufferSize,

    
    /* 0x4000 - 0x4999 reserved for MAC  Layer */
    CtrlMsg_MAC_GetInterfaceList = 0x4000,
    CtrlMsg_MAC_GetInterfaceAddress,
    CtrlMsg_MAC_SetInterfaceAddress,
    CtrlMsg_MAC_GetBroadcastAddress,
    CtrlMsg_MAC_SetBroadcastAddress,
    CtrlMsg_MAC_SetPhyscialAddress,
    CtrlMsg_MAC_GetPhyscialAddress,
    CtrlMsg_MAC_GetInterfaceName,
    CtrlMsg_MAC_SetInterfaceName,
    CtrlMsg_MAC_GetNetmask,
    CtrlMsg_MAC_SetNetmask,

    
    /* 0x5000 - 0x5999 reserved for Physical Layer */
    CtrlMsg_PHY_GetInterfaceStats = 0x5000,
    CtrlMsg_PHY_SetHWAddress,
    CtrlMsg_PHY_GetHWAddress,
    CtrlMsg_PHY_SetTxPower,
    CtrlMsg_PHY_GetTxPower,
    CtrlMsg_PHY_SetDataRate,
    CtrlMsg_PHY_GetDataRate,
    CtrlMsg_PHY_SetTxChannel,
    CtrlMsg_PHY_GetTxChannel,
    CtrlMsg_PHY_SetInterfaceFlags,
    CtrlMsg_PHY_GetInterfaceFlags,
    CtrlMsg_PHY_SetInterfaceMetric,
    CtrlMsg_PHY_GetInterfaceMetric,
    CtrlMsg_PHY_SetInterfaceMTU,
    CtrlMsg_PHY_GetInterfaceMTU,
    
    /* 0x6000 - 0x6999 reserved for Propagation Layer */
    
    /* 0x7000 - 0x7999 reserved for UPA */
    CtrlMsg_UPA_SetUpaSocket = 0x7000,
    CtrlMsg_UPA_SetPhysicalAddress,
    CtrlMsg_UPA_GetPhysicalAddress,
};

BOOL 
NodeControlInterface(
    Node *node,
    ControlMessage cmd,
    unsigned long arg);



#endif
