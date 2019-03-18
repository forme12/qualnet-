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
// PACKAGE     :: TRANSPORT LAYER
// DESCRIPTION :: This file describes data structures and functions used by the Tansport
//                Layer.
// **/


#ifndef _TRANSPORT_H_
#define _TRANSPORT_H_

#ifdef ADDON_DB
struct StatsDBTransAggregate;
struct TransSrcDstData;
struct ltportpair;
struct ltaddrpair;
typedef std::map<std::pair<short, short>, TransSrcDstData *, ltportpair> MapTransSrcDstData;
#endif

// /**
// CONSTANT    :: TRANSPORT_DELAY : (1 * MICRO_SECOND)
// DESCRIPTION :: Delay to process a packet in transport layer
// **/
#define TRANSPORT_DELAY (1 * MICRO_SECOND)


// /**
// ENUM        :: TransportProtocol
// DESCRIPTION :: Enlisting different transport layer protocol
// **/
enum TransportProtocol
{
    TransportProtocol_UDP,
    TransportProtocol_TCP,
    //InsertPatch TRANSPORT_ENUMERATION
    TransportProtocol_RSVP
};


typedef struct TransportDataUdpStruct TransportDataUdp;
typedef struct TransportDataTcpStruct TransportDataTcp;
typedef struct TransportDataAbstractTcpStruct TransportDataAbstractTcp;

// Transport Reset functions used by NMS.
typedef void (*TransportSetFunctionPtr)(Node*,
                                const NodeInput*);
// Set rules defined
struct TransportResetFunction
{
    // Corresponding set function
    TransportSetFunctionPtr FuncPtr;

    // the next match command
    TransportResetFunction* next;
};

struct TransportResetFunctionList
{
    TransportResetFunction* first;
    TransportResetFunction* last;
};


// /**
// STRUCT      :: TransportData
// DESCRIPTION :: Main data structure of transport layer
// **/
struct TransportData {
    TransportDataUdp* udp;
    // TransportDataTcp* tcp;
    int tcpType; // It may be TCP_REGULAR or TCP_ABSTRACT
    void* tcp;
    BOOL rsvpProtocol;
    void *rsvpVariable;
#ifdef ADDON_NGCNMS
    TransportResetFunctionList* resetFunctionList;
#endif
};

void
TRANSPORT_Reset(Node *node, const NodeInput *nodeInput);

void 
TRANSPORT_AddResetFunctionList(Node* node, void *param);

#endif /* _TRANSPORT_H_ */

