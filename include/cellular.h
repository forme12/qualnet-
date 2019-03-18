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

// /**
// PACKAGE     :: CELLULAR
// DESCRIPTION :: Defines the data structures used in all layers of the
//                cellualr model the data structure start with Cellular***.
// **/

#ifndef _CELLULAR_H_
#define _CELLULAR_H_

// /**
// ENUM        :: CellularNodeType
// DESCRIPTION :: Enlisted different types of cellular nodes
// **/
typedef enum
{
    CELLULAR_MS=0,
    CELLULAR_BS=1,
    CELLULAR_SC=2,
    CELLULAR_GATEWAY=3,
    CELLULAR_AGGREGATED_NODE=4,

    //for UMTS or GPRS
    CELLULAR_UE,
    CELLULAR_NODEB,
    CELLULAR_RNC,

    CELLULAR_SGSN,
    CELLULAR_GGSN,

    //for all
    CELLULAR_VLR,
    CELLULAR_HLR,
    CELLULAR_Auc
}CellularNodeType;

// /**
// ENUM        :: CellularProtocolDiscriminator
// DESCRIPTION :: Identifies the protocol discriminator
// **/
// REFERENCE: GSM 04.08, sec 10.2
typedef enum
{
    CELLULAR_PD_CC = 0x03,   // Call Control
    CELLULAR_PD_MM = 0x05,   // Mobility Management
    CELLULAR_PD_RR = 0x06,   // Radio Resource Management
    CELLULAR_PD_SM = 0x07,   // Session Management
    CELLULAR_PD_APP =0x08,   // application data, for abstract cellular only
    CELLULAR_PD_INVALID      // last one

}CellularProtocolDiscriminator;
#endif /* _CELLULAR_H_ */
