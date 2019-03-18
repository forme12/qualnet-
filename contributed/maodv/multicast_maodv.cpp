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

//----------------------------------------------------------------------------//
// Name: maodv.cpp
// Purpose: To simulate Multicast AODV
// Author: Venkatesh Rajendran (venkat@cse.ucsc.edu)
// Univeristy of California, Santa Cruz.

// This implementation is as per the specification of
// multicast operation of AODV in IETF draft. Some
// improvizations has been made to ensure protocol
// correctness and performance.
//----------------------------------------------------------------------------//

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>

#include "api.h"
#include "network_ip.h"
#include "multicast_maodv.h"
#include "buffer.h"

#define INF 10000

#define DEBUG_PACKET 0
#define DEBUG_ROUTE_DISCOVERY 0
#define DEBUG_HELLO 0
#define DEBUG_FAILURE 0
#define DEBUG_GROUP_HELLO 0
#define DEBUG_LEADER 0

//-----------------------------------------------------------------------------
// MACRO        MaodvInitNeighborTable()
// PURPOSE      Initalizes neighborTable
// PARAMETERS   neighborTable
//                  Pointer to neighbor table
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitNeighborTable(neighborTable) \
    (neighborTable)->head = NULL; \
    (neighborTable)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitSeenTable()
// PURPOSE      Initalizes SeenTable
// PARAMETERS   seenTable
//                  Pointer to seen table
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitSeenTable(seenTable) \
    (seenTable)->front = NULL; \
    (seenTable)->rear = NULL; \
    (seenTable)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitBuffer()
// PURPOSE      Initalizes Buffer
// PARAMETERS   buffer
//                  Pointer to buffer
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitBuffer(buffer) \
    (buffer)->head = NULL; \
    (buffer)->size = 0; \
    (buffer)->numByte = 0;
//-----------------------------------------------------------------------------
// MACRO        MaodvInitSent()
// PURPOSE      Initalizes sent list
// PARAMETERS   sent
//                  Pointer to sent list
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitSent(sent) \
    (sent)->head = NULL; \
    (sent)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitNextHopsList()
// PURPOSE      Initalizes nexthop list
// PARAMETERS   nestHops
//                  Pointer to next list
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitNextHopsList(nextHops) \
    (nextHops)->head = NULL; \
    (nextHops)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitMembership()
// PURPOSE      Initalizes membership list
// PARAMETERS   memberFlag
//                  Pointer to member
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitMembership(memberFlag) \
    (memberFlag)->head = NULL; \
    (memberFlag)->size = 0;
//-----------------------------------------------------------------------------
// MACRO        MaodvInitRouteTable()
// PURPOSE      Initalizes routing Table
// PARAMETERS   routeTable
//                  Pointer to routeTable
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitRouteTable(routeTable) \
    (routeTable)->rtEntry = NULL; \
    (routeTable)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitRouteTable()
// PURPOSE      Initalizes multicast route Table
// PARAMETERS   mrouteTable
//                  Pointer to multicast route Table
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitMrouteTable(mrouteTable) \
    (mrouteTable)->mrtEntry = NULL; \
    (mrouteTable)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitRequestTable()
// PURPOSE      Initalizes multicast request Table
// PARAMETERS   requestTable
//                  Pointer to multicast request Table
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitRequestTable(requestTable) \
    (requestTable)->requestEntry = NULL; \
    (requestTable)->size = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitStats()
// PURPOSE      Initalizes statistics
// PARAMETERS   x
//                  Pointer to MaodvStats
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitStats(x) \
    memset(&(x->stats), 0, sizeof(MaodvStats));

//-----------------------------------------------------------------------------
// MACRO        MaodvInitSeq()
// PURPOSE      Initalizes sequence no
// PARAMETERS   maodv
//                  maodv data pointer
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitSeq(maodv) \
    (maodv)->seqNumber = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvInitFloodingId()
// PURPOSE      Initalizes flooding Id
// PARAMETERS   maodv
//                  maodv data pointer
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvInitFloodingId(maodv) \
    (maodv)->floodingId = 0;

//-----------------------------------------------------------------------------
// MACRO        MaodvIncreaseSeq()
// PURPOSE      Increase sequenceNo.
// PARAMETERS   maodv
//                  maodv data pointer
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvIncreaseSeq(maodv) \
    (maodv)->seqNumber++;

//-----------------------------------------------------------------------------
// MACRO        MaodvGetMySeq()
// PURPOSE      Get owm sequenceNo.
// PARAMETERS   maodv
//                  maodv data pointer
// RETURNS      None
//-----------------------------------------------------------------------------
#define MaodvGetMySeq(maodv) \
    (maodv)->seqNumber;

//---------------------------------------------------------------------------
// FUNCTION: MaodvPrintMroutingTable
//
// PURPOSE: Printing the different fields of the multicast routing
//          table of Aodv
//
// ARGUMENTS: node, The node printing the multicast routing table
//            mrouteTable, Aodv routing table
//
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvPrintMroutingTable(Node *node, MaodvMroutingTable *mrouteTable)
{
    MaodvMrouteEntry *mrtEntry;
    printf("\n \n MRT %u is:\n", node->nodeId);
    printf("Group        Seq    Leader  HCLeader  HCMember  "
        "isValid  isMember\n");
    printf("-------------------------------------------------------------"
        "----------\n");
    for (mrtEntry = mrouteTable->mrtEntry;
        mrtEntry != NULL;
        mrtEntry = mrtEntry->next)
    {
        char address[100];
        IO_ConvertIpAddressToString(mrtEntry->destination.address, address);
        printf("%s   %5u   %5u  %5d   %5d  Valid:%s Member:%s\n",
            address,
            mrtEntry->destination.seqNum,
            mrtEntry->mcastLeader,
            mrtEntry->hopCountLeader,
            mrtEntry->hopCount, mrtEntry->isValid ? "TRUE" : "FALSE",
            mrtEntry->isMember ? "TRUE" : "FALSE");

        if (mrtEntry->nextHops.size == 0)
        {
            printf("No next HOP \n");
        }
        else
        {
            MaodvNextHopNode *nextHopNode = NULL;
            for (nextHopNode = mrtEntry->nextHops.head;
                nextHopNode != NULL;
                nextHopNode = nextHopNode->next)
            {
                IO_ConvertIpAddressToString(
                    nextHopNode->nextHopAddr,
                    address);

                printf("\n \t %s  %d   %s  %s",
                    address,
                    nextHopNode->interface,
                    (nextHopNode->direction == DOWNSTREAM_DIRECTION) ?
                        "DOWNSTREAM_DIRECTION" : "UPSTREAM_DIRECTION",
                    nextHopNode->isEnabled ?  "TRUE" : "FALSE");
            }
            printf("\n");
        }
    }
    printf("-------------------------------------------------------------"
        "----------\n");
    printf("\n\n");
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvPrintRoutingTable
//
// PURPOSE: Printing the different fields of the routing table of Aodv
//
// ARGUMENTS: node, The node printing the routing table
//            routeTable, Aodv routing table
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvPrintRoutingTable(Node *node, MaodvRoutingTable *routeTable)
{
    MaodvRouteEntry *rtEntry;
    char nextHop[25];
    char dest[25];
    printf("The Routing Table of node %u is:\n", node->nodeId);
    printf("Dest  DestSeq  HopCount  Intf  NextHop  activated  lifetime\n");
    printf("-------------------------------------------------------------\n");

    for (rtEntry = routeTable->rtEntry; rtEntry != NULL; rtEntry = rtEntry->
        next)
    {
        char time[20];

        TIME_PrintClockInSecond(rtEntry->lifetime, time);

        IO_ConvertIpAddressToString(rtEntry->destination.address, dest);
        IO_ConvertIpAddressToString(rtEntry->nextHop, nextHop);

        printf("%s   %5u   %5d   %5d   %s   %9s  %5s\n",
            dest,
            rtEntry->destination.seqNum,
            rtEntry->hopCount,
            rtEntry->interface,
            nextHop,
            rtEntry->activated ? "TRUE" : "FALSE",
            time);
    }
    printf("-------------------------------------------------------------\n");
    printf("\n\n");
}

//---------------------------------------------------------------------------
// FUNCTION:  MaodvGetValidNextHops
//
// PURPOSE: Returns the number of valid next hops
//
// ARGUMENTS:
//
// RETURN: the number of valid next hops
//---------------------------------------------------------------------------
static
int MaodvGetValidNextHops(MaodvNextHops *nextHops)
{
    if (nextHops->size == 0)
    {
        // No entry found
        return(0);
    }
    else
    {
        MaodvNextHopNode *current = NULL;
        int count = 0;
        for (current = nextHops->head;
            current != NULL;
            current = current->next)
        {
            if (current->isEnabled == TRUE)
            {
                count++;
            }
        }

        return (count);
    }
}

//---------------------------------------------------------------------------
// FUNCTION:  AddCustomMaodvIpOptionFields
//
// PURPOSE:
//
// ARGUMENTS:  node ,msg.
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void AddCustomMaodvIpOptionFields(Node* node, Message* msg)
{
    AddIpOptionField(node, msg, IPOPT_MAODV, sizeof(MaodvIpOptionType));
}

//---------------------------------------------------------------------------
// FUNCTION:  GetMaodvIpOptionField
//
// PURPOSE: Extract the IP Option field from the
//              packet.
//
// Parameters:
//                     ms : Pointer to message structure
// RETURN:    None
//---------------------------------------------------------------------------

static
MaodvIpOptionType GetMaodvIpOptionField(const Message* msg)
{
    IpOptionsHeaderType* ipOption =
        FindAnIpOptionField((IpHeaderType*) msg->packet, IPOPT_MAODV);
    MaodvIpOptionType MaodvIpOption;
    memcpy(&MaodvIpOption, ((char*)ipOption + sizeof(IpOptionsHeaderType)),
        sizeof(MaodvIpOptionType));
    return MaodvIpOption;
}

//---------------------------------------------------------------------------
// FUNCTION:  SetMaodvIpOptionField
// PURPOSE      Set IP Option field of the packet
//
// Parameters:
//  msg  : Pointer to message structure
//  MaodvIpOptionType : Pointer to IP Option field
//---------------------------------------------------------------------------

static
void SetMaodvIpOptionField(
    Message* msg,
    const MaodvIpOptionType* MaodvIpOption)
{
    IpOptionsHeaderType* ipOption =
        FindAnIpOptionField((IpHeaderType*)msg->packet, IPOPT_MAODV);
    memcpy(((char*)ipOption + sizeof(IpOptionsHeaderType)), MaodvIpOption,
        sizeof(MaodvIpOptionType));
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvSetTimer
//
// PURPOSE:  Set timers for protocol events
//
// ARGUMENTS: node, The node which is scheduling an event
//            eventType, The event type of the message
//            destAddr, Destination for which the event has been sent (if
//            necessary)
//            delay,Time after which the event will expire
//
//---------------------------------------------------------------------------
static
void MaodvSetTimer(
    Node *node,
    Message *newMsg,
    Int32 eventType,
    NodeAddress destAddr,
    clocktype delay)
{
    NodeAddress *info = NULL;

    if (newMsg == NULL)
    {
        // Allocate message for the timer
        newMsg = MESSAGE_Alloc(
            node,
            NETWORK_LAYER,
            MULTICAST_PROTOCOL_MAODV,
            eventType);
    }
    else
    {
        MESSAGE_SetEvent(newMsg, (short)eventType);
    }


    // Assign the address for which the timer is meant for
    MESSAGE_InfoAlloc(node, newMsg, sizeof(NodeAddress));

    info = (NodeAddress *) MESSAGE_ReturnInfo(newMsg);
    *info = destAddr;
    // Schdule the timer after the specified delay
    MESSAGE_Send(node, newMsg, delay);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvSetMulticastTimer
//
// PURPOSE:  Set timers for protocol events
//
// ARGUMENTS: node, The node which is scheduling an event
//            eventType, The event type of the message
//            mcastAddr, Group Destination for which the event has been sent
//            (if necessary)
//            nextHopAddr, address of the next hop node in the tree
//            delay,    Time after which the event will expire
//---------------------------------------------------------------------------

static
void MaodvSetMulticastTimer(
    Node *node,
    Int32 eventType,
    NodeAddress mcastAddr,
    NodeAddress nextHopAddr,
    clocktype delay)
{
    Message *newMsg = NULL;
    NextHopInfo *info = NULL;

    // Allocate message for the timer
    newMsg = MESSAGE_Alloc(
                 node,
                 NETWORK_LAYER,
                 MULTICAST_PROTOCOL_MAODV,
                 eventType);

    // Assign the address for which the timer is meant for
    MESSAGE_InfoAlloc(node, newMsg, sizeof(NextHopInfo));

    info = (NextHopInfo *) MESSAGE_ReturnInfo(newMsg);
    info->mcastAddr = mcastAddr;
    info->nextHopAddr = nextHopAddr;
    // Schdule the timer after the specified delay
    MESSAGE_Send(node, newMsg, delay);
}


//---------------------------------------------------------------------------
// FUNCTION:  MaodvGetFloodingId
//
// PURPOSE:   Obtains the broadcast ID for the outgoing packet
//
// ARGUMENTS: aodv, aodv main data structure.
//
// RETURN:    The broadcast Id
//
// NOTE:      This function increments the broadcast id by 1
//---------------------------------------------------------------------------

static
int MaodvGetFloodingId(MaodvData *maodv)
{
    int bcast;
    bcast = maodv->floodingId;
    maodv->floodingId++;
    return (bcast);
}

//---------------------------------------------------------------------------
// FUNCTION:  MaodvUpdateInsertNeighbor
//
// PURPOSE:
//
// ARGUMENTS: node, neighborAddr, neighborTable
//
// RETURN:   none
//---------------------------------------------------------------------------
static
void MaodvUpdateInsertNeighbor(
    Node *node,
    NodeAddress neighborAddr,
    MaodvNeighborTable *neighborTable)
{
    MaodvNeighborNode *current = neighborTable->head;
    MaodvNeighborNode *previous =  NULL;
    MaodvNeighborNode *newNode = NULL;

    if (neighborAddr == ANY_IP)
    {
        // Own IP address so don't need to insert it.
        return;
    }

    while ((current != NULL) && (current->destAddr < neighborAddr))
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL || current->destAddr != neighborAddr)
    {
        // adding a new entry  here
        ++(neighborTable->size);
        newNode = (MaodvNeighborNode *) MEM_malloc(sizeof(MaodvNeighborNode));
        newNode->destAddr = neighborAddr;

        if (previous == NULL)
        {
            newNode->next = neighborTable->head;
            neighborTable->head = newNode;
        }
        else
        {
            previous->next = newNode;
            newNode->next = current;
        }
    }
    else if (current->destAddr == neighborAddr)
    {
        newNode = current;
    }

    newNode->lastHeared = getSimTime(node);
}

//---------------------------------------------------------------------------
// FUNCTION:  MaodvGetLastHearedTime
//
// PURPOSE:   To get the last heardTime node
//
// ARGUMENTS: neighborAddr, neighborTable
//
// RETURN:   none
//---------------------------------------------------------------------------
static
clocktype MaodvGetLastHearedTime(
    NodeAddress neighborAddr,
    MaodvNeighborTable *neighborTable)
{
    MaodvNeighborNode *current = neighborTable->head;

    while ((current != NULL) && (current->destAddr <= neighborAddr))
    {
        if (current->destAddr == neighborAddr)
        {
            return (current->lastHeared);
        }
        current = current->next;
    }
    ERROR_Assert(FALSE, "MaodvGetLastHearedTime: Neighbor entry not found");
    return 0;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInsertSeenTable
//
// PURPOSE:  Insert an entry into the seen table
//
// ARGUMENTS: node, the node which is inserting into the table
//            srcAddr, the source address of RREQ packet
//            floodingId, the flooding Id in the RREQ from the source
//            seenTable, table to store source and broadcast id pair
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvInsertSeenTable(
    Node *node,
    NodeAddress srcAddr,
    int floodingId,
    NodeAddress leader,
    MaodvRreqSeenTable *seenTable)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node, MULTICAST_PROTOCOL_MAODV);

    // Always add in the rear of the list and send one timer for expire.
    // In time of deletion, it will be always from the front

    if (seenTable->size == 0)
    {
        seenTable->rear = (MaodvRreqSeenNode *)
            MEM_malloc(sizeof(MaodvRreqSeenNode));
        seenTable->front = seenTable->rear;
    }
    else
    {
        seenTable->rear->next = (MaodvRreqSeenNode *)
            MEM_malloc(sizeof(MaodvRreqSeenNode));
        seenTable->rear = seenTable->rear->next;
    }

    seenTable->rear->srcAddr = srcAddr;
    seenTable->rear->floodingId = floodingId;
    seenTable->rear->leader = leader;
    seenTable->rear->next = NULL;
    ++(seenTable->size);

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_FlushTables,
        ANY_IP,
        (clocktype) MAODV_FLOOD_RECORD_TIME);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvLookupSeenTable
//
// PURPOSE:  Returns TRUE if the broadcast packet is processed before
//
// ARGUMENTS: srcAddr, source of RREQ
//            floodingId, floodingId id in the received RREQ
//            seenTable, table where information of seen RREQ's has been stored
//
// RETURN:    None
//---------------------------------------------------------------------------

static
BOOL MaodvLookupSeenTable(
    NodeAddress srcAddr,
    int floodingId,
    NodeAddress leader,
    MaodvRreqSeenTable *seenTable)
{
    MaodvRreqSeenNode *current;

    if (seenTable->size == 0)
    {
        return (FALSE);
    }

    for (current = seenTable->front; current != NULL; current = current->next)
    {
        if (current->srcAddr == srcAddr
            && current->floodingId == floodingId
            && current->leader == leader)
        {
            return (TRUE);
        }
    }
    return (FALSE);
}


//---------------------------------------------------------------------------
//FUNCTION: MaodvDeleteSeenTable
//
//PURPOSE:  Remove an entry from the seen table, deletion will be always from
//           the front of the table and insertion fron the rear.
//
// ARGUMETS: seenTable
//
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvDeleteSeenTable(MaodvRreqSeenTable *seenTable)
{
    MaodvRreqSeenNode *toFree;

    toFree = seenTable->front;
    seenTable->front = toFree->next;
    MEM_free(toFree);
    --(seenTable->size);

    if (seenTable->size == 0)
    {
        seenTable->rear = NULL;
    }
}

//---------------------------------------------------------------------------
// BUFFFER RELATED FUNCTIONS
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// FUNCTION: MaodvLookupBuffer
//
// PURPOSE:  Returns TRUE if any packet is buffered to the destination
//
// ARGUMENTS: destAddress, the destination address of the message
//            buffer,      buffer to store the messages
//
// RETURN:  true, if there is buffered packet towards the destination
//          false, otherwise
//---------------------------------------------------------------------------

static
BOOL MaodvLookupBuffer(
    NodeAddress destAddr,
    MaodvMessageBuffer *buffer)
{
    MaodvBufferNode *current;

    if (buffer->size == 0)
    {
        // There is no packet in the buffer so return false
        return (FALSE);
    }

    // There are packets in the buffer, check for the destination addresses
    for (current = buffer->head; current != NULL
        && current->destAddr <= destAddr; current = current->next)
    {
        if (current->destAddr == destAddr)
        {
            // The destination address matched so return true
            return(TRUE);
        }
    }

    // No match for the destination so return false
    return (FALSE);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInsertBuffer
//
// PURPOSE: Insert a packet into the buffer if no route is available
//
// ARGUMENTS: msg, The message waiting for a route to destination
//            destAddr, The destination of the packet
//            buffer,   The buffer to store the message
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvInsertBuffer(
    Node *node,
    Message *msg,
    NodeAddress destAddr,
    NodeAddress previousHop,
    MaodvMessageBuffer *buffer)
{
    MaodvBufferNode *current, *previous, *newNode;
    MaodvData *maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(node,MULTICAST_PROTOCOL_MAODV);

    // if the buffer exceeds silently drop the packet
    // if no buffer size is specified in bytes it will only check for
    // number of packet.

    if (maodv->bufferSizeInByte == 0)
    {
        if (buffer->size == maodv->bufferSizeInNumPacket)
        {
            MESSAGE_Free(node, msg);
            maodv->stats.numDataDroppedForOverlimit++;
            return;
        }
    }
    else
    {
        if ((buffer->numByte + MESSAGE_ReturnPacketSize(msg))
            > maodv->bufferSizeInByte)
        {
            MESSAGE_Free(node, msg);
            maodv->stats.numDataDroppedForOverlimit++;
            return;
        }
    }

    // Allocate space for the new message
    newNode = (MaodvBufferNode *) MEM_malloc(sizeof(MaodvBufferNode));

    // Store the allocate message along with the destination number and the
    // the time at which the packet has been inserted
    newNode->destAddr = destAddr;
    newNode->previousHop = previousHop;
    newNode->msg = msg;
    newNode->timestamp = getSimTime(node);
    newNode->next = NULL;
    // Increase the size of the buffer
    ++(buffer->size);
    buffer->numByte += MESSAGE_ReturnPacketSize(msg);

    // Find Insertion point.  Insert after all address matches.
    // This is to maintain a sorted list in ascending order of the destination
    // address
    previous = NULL;
    current = buffer->head;

    while ((current != NULL) && (current->destAddr <= destAddr))
    {
        previous = current;
        current = current->next;
    }

    // Got the insertion point
    if (previous == NULL)
    {
        // The is the first message in the buffer
        newNode->next = buffer->head;
        buffer->head = newNode;
    }
    else
    {
        // This is an intermediate node in the list
        newNode->next = previous->next;
        previous->next = newNode;
    }
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvGetBufferedPacket
//
// PURPOSE:  Extract the packet that was buffered
//
// ARGUMENTS: destAddr, the destination address of the packet to be retrieved
//            buffer,   the message buffer
//
// RETURN:   The mesage for this destination
//---------------------------------------------------------------------------

static
Message *MaodvGetBufferedPacket(
    NodeAddress destAddr,
    NodeAddress *previousHop,
    MaodvMessageBuffer *buffer)
{
    MaodvBufferNode *current;

    // Search for the destination
    for (current = buffer->head;current != NULL
        && current->destAddr <= destAddr;
        current = current->next)
    {
        if (current->destAddr == destAddr)
        {
            // Got the matched destination so return the packet
            *previousHop = current->previousHop;
            return(current->msg);
        }
    }

    // No packet for the specified destination
    return NULL;
}


//---------------------------------------------------------------------------
// FUNCTION:    RoutingModvDeleteBuffer
//
// PURPOSE: Remove a packet from the buffer;
//          Return TRUE if deleted
//
// ARGUMENTS: destAddr, the destination to which destined packet should be
//                      deleted.
//            buffer,   the buffer to store packets
//
// RETURN:    true, if deletion is successful
//            false, otherwise
//---------------------------------------------------------------------------

static
BOOL MaodvDeleteBuffer(
    NodeAddress destAddr,
    MaodvMessageBuffer *buffer)
{
    MaodvBufferNode *toFree, *current;
    BOOL deleted;

    if (buffer->size == 0)
    {
        // No packet in the buffer so deletion failed
        deleted = FALSE;
    }
    else if (buffer->head->destAddr == destAddr)
    {
        // The first packet is the desired packet
        toFree = buffer->head;
        buffer->head = toFree->next;
        buffer->numByte -= MESSAGE_ReturnPacketSize(toFree->msg);
        MEM_free(toFree);
        --(buffer->size);
        deleted = TRUE;
    }
    else
    {
        // Search for the packet
        for (current = buffer->head; current->next != NULL
            && current->next->destAddr < destAddr;current = current->next)
        {
        }

        if (current->next != NULL && current->next->destAddr == destAddr)
        {
            // Got the desired packet, deletion successful
            toFree = current->next;
            current->next = toFree->next;
            buffer->numByte -= MESSAGE_ReturnPacketSize(toFree->msg);
            MEM_free(toFree);
            --(buffer->size);
            deleted = TRUE;
        }
        else
        {
            // Don't have the desired packet, deletion unsuccessful
            deleted = FALSE;
        }
    }
    return (deleted);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCheckSent
//
// PURPOSE:  Check if RREQ has been sent; return TRUE if sent
//
// ARGUMENTS: destAddress, destination address of the packet
//            sent,        the structure to mark the packets for which RREQ
//                         has been sent
// RETURN:    pointer to the sent node if exists, null otherwise
//            NULL otherwise
//            same used for multicast
//---------------------------------------------------------------------------

static
MaodvRreqSentNode *MaodvCheckSent(
    NodeAddress destAddr,
    MaodvRreqSentTable *sent)
{
    MaodvRreqSentNode *current;
    if (sent->size == 0)
    {
        // The sent table is empty
        return (NULL);
    }

    for (current = sent->head; current != NULL
        && current->destAddr <= destAddr;
        current = current->next)
    {
        if (current->destAddr == destAddr)
        {
            return(current);
        }
    }
    return (NULL);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvInsertSent
//
// PURPOSE:  Insert an entry into the sent table if RREQ is sent
//
// ARGUMENTS: destAddr, The destination address for which the rreq has been
//                      sent
//            ttl,      The time to leave of the rreq
//            sent,     The structure to store information about the
//                      destinations for which rreq has been sent
//
// RETURN:   The node just inserted
//---------------------------------------------------------------------------

static
MaodvRreqSentNode *MaodvInsertSent(
    NodeAddress destAddr,
    int ttl,
    RreqType type,
    clocktype timestamp,
    MaodvRreqSentTable *sent)
{
    MaodvRreqSentNode *current, *previous;
    MaodvRreqSentNode *newNode = (MaodvRreqSentNode *)
        MEM_malloc(sizeof(MaodvRreqSentNode));

    newNode->destAddr = destAddr;
    newNode->ttl = ttl;
    newNode->times = 0;
    newNode->next = NULL;
    newNode->type = type;
    newNode->timestamp = timestamp;
    newNode->isReplied = FALSE;

    (sent->size)++;

    // Find Insertion point.  Insert after all address matches.
    // To make the list sorted in ascending order
    previous = NULL;
    current = sent->head;

    while ((current != NULL) && (current->destAddr <= destAddr))
    {
        previous = current;
        current = current->next;
    }

    if (previous == NULL)
    {
        newNode->next = sent->head;
        sent->head = newNode;
    }
    else
    {
        newNode->next = previous->next;
        previous->next = newNode;
    }

    return (newNode);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvDeleteSent
//
// PURPOSE:  Remove an entry from the sent table
//
// ARGUMENTS: destAddr, address to be deleted from sent table
//            sent,     The structure to store information about the
//                      destinations for which rreq has been sent
//
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvDeleteSent(
    NodeAddress destAddr,
    MaodvRreqSentTable *sent)
{
    MaodvRreqSentNode *toFree, *current;

    if (sent->size == 0)
    {
        return;
    }
    else if (sent->head->destAddr == destAddr)
    {
        toFree = sent->head;
        sent->head = toFree->next;
        MEM_free(toFree);
        --(sent->size);
    }
    else
    {
        for (current = sent->head; current->next != NULL
            && current->next->destAddr < destAddr; current = current->next)
        {
        }
        if (current->next != NULL && current->next->destAddr == destAddr)
        {
            toFree = current->next;
            current->next = toFree->next;
            MEM_free(toFree);
            --(sent->size);
        }
    }
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvSetMembershipType
//
// PURPOSE:  Sets membership type
//
// ARGUMENTS: mcastAddr: multicast address
//            type:     Type of membership
//            memberFlag: Membership flag
//
// RETURN:   None
//---------------------------------------------------------------------------
static
MAODV_M_Node *MaodvSetMembershipType(
    NodeAddress mcastAddr,
    MaodvMemberType type,
    MAODV_Membership *memberFlag)
{
    MAODV_M_Node *current;
    if (memberFlag->size == 0)
    {
        ERROR_Assert(FALSE, "\n Error in setting membership type..");
    }

    for (current = memberFlag->head;current != NULL;current = current->next)
    {
        if (current->mcastAddr == mcastAddr)
        {
            current->type = type;
            return(current);
        }
    }
    ERROR_Assert(FALSE, "Error in setting membership type.. ");
    return NULL;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvSetMemFlagInOrder
//
// PURPOSE:  Sets membership flag in order
//
// ARGUMENTS: mcastAddr: multicast address
//            type:     Type of membership
//            memberFlag: Membership flag
//
// RETURN:   None
//---------------------------------------------------------------------------
static
MAODV_M_Node *MaodvSetMemFlagInOrder(
    Node* node,
    NodeAddress mcastAddr,
    MaodvMemberType type,
    MAODV_M_Node *old)
{
    MAODV_M_Node *newOne;

    if (old == NULL)
    {
        newOne = (MAODV_M_Node *) MEM_malloc(sizeof(MAODV_M_Node));
        newOne->mcastAddr = mcastAddr;
        newOne->next = NULL;
        newOne->type = type;
    }
    else if (old->mcastAddr > mcastAddr)
    {
        newOne = (MAODV_M_Node *) MEM_malloc(sizeof(MAODV_M_Node));
        newOne->mcastAddr = mcastAddr;
        newOne->type = type;
        newOne->next = old;
    }
    else
    {
        newOne = old;
        newOne->next = MaodvSetMemFlagInOrder(
            node,
            mcastAddr,
            type,
            old->next);
    }
    newOne->isUpdate = FALSE;
    return (newOne);
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvSetMemberFlag
// PURPOSE      Set membership flag for a particular multicast group.
//
// Parameters:
//     mcastAddr:     Multicast group.
//     memberFlag:    Membership flag.
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvSetMemberFlag(
    Node* node,
    NodeAddress mcastAddr,
    MaodvMemberType type,
    MAODV_Membership *memberFlag)
{
    ++(memberFlag->size);
    memberFlag->head = MaodvSetMemFlagInOrder(
        node,
        mcastAddr,
        type,
        memberFlag->head);
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvResetMemberFlag
// PURPOSE      Reset membership flag for a particular multicast group.
//
// Parameters:
//     mcastAddr:     Multicast group.
//     memberFlag: Membership flag.
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvResetMemberFlag(NodeAddress mcastAddr,
                                 MAODV_Membership *memberFlag)
{
    MAODV_M_Node *toFree;
    MAODV_M_Node *current;

    if (memberFlag->size == 0)
    {
        return;
    }
    else if (memberFlag->head->mcastAddr == mcastAddr)
    {
        toFree = memberFlag->head;
        memberFlag->head = toFree->next;
        MEM_free(toFree);
        --(memberFlag->size);
    }
    else
    {
        for (current = memberFlag->head;
            current->next != NULL && current->next->mcastAddr < mcastAddr;
            current = current->next)
        {
        }

        if (current->next != NULL && current->next->mcastAddr == mcastAddr)
        {
            toFree = current->next;
            current->next = toFree->next;
            MEM_free(toFree);
            --(memberFlag->size);
        }
    }
} /* MaodvResetMemberFlag */

//---------------------------------------------------------------------------
// FUNCTION     MaodvInitMessageCache
// PURPOSE      Initialize the message cache.
//
// Parameters:
//     messageCache:  Message cache.
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvInitMessageCache(MAODV_MC *messageCache)
{
    messageCache->front = NULL;
    messageCache->rear = NULL;
    messageCache->size = 0;
}


//---------------------------------------------------------------------------
// FUNCTION     MaodvInsertMessageCache
// PURPOSE      Insert new entry into message cache.
//
// Parameters:
//     node:            Node that is inserting new entry into message cache.
//     sourceAddr:      Packet originator.
//     seqNumber:       Packet sequence number.
//     messageCache: Message cache.
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvInsertMessageCache(
    Node *node,
    NodeAddress srcAddr,
    MAODV_MC *messageCache,
    unsigned int pktId)
{
    if (messageCache->size == 0)
    {
        messageCache->rear = (MAODV_MC_Node *)
            MEM_malloc(sizeof(MAODV_MC_Node));
        messageCache->front = messageCache->rear;
    }
    else
    {
        messageCache->rear->next = (MAODV_MC_Node *)
            MEM_malloc(sizeof(MAODV_MC_Node));
        messageCache->rear = messageCache->rear->next;
    }

    messageCache->rear->srcAddr = srcAddr;
    messageCache->rear->sent = FALSE;
    messageCache->rear->next = NULL;
    messageCache->rear->pktId = pktId;

    ++(messageCache->size);

    MaodvSetTimer(
        node,
        NULL,
        MSG_ROUTING_MaodvFlushMessageCache,
        ANY_DEST,
        MAODV_FLUSH_INTERVAL);
}


//---------------------------------------------------------------------------
// FUNCTION     MaodvDeleteMsgCache
// PURPOSE      Remove an entry from the message cache
//
// Parameters:
//     messageCache:  Message cache table.
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvDeleteMsgCache(MAODV_MC *messageCache)
{
    MAODV_MC_Node *toFree;
    toFree = messageCache->front;
    messageCache->front = toFree->next;
    MEM_free(toFree);
    --(messageCache->size);

    if (messageCache->size == 0)
    {
        messageCache->rear = NULL;
    }
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLookupMessageCache
// PURPOSE      Check if the join query/data packet is seen before.
//
// Parameters:
//     sourceAddr:      Originating node of the packet.
//     seqNumber:       Sequece number of the packet.
//     messageCache: Message cache table.
//
// Return: TRUE if seen before; FALSE otherwise.
//---------------------------------------------------------------------------

BOOL MaodvLookupMessageCache(
    NodeAddress srcAddr,
    MAODV_MC *messageCache,
    unsigned int pktId)
{
    MAODV_MC_Node *current;

    if (messageCache->size == 0)
    {
        return (FALSE);
    }

    for (current = messageCache->front;
        current != NULL;
        current = current->next)
    {
        if (current->srcAddr == srcAddr && current->pktId == pktId)
        {
            return (TRUE);
        }
    }

    return (FALSE);
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLookupMemberFlag
// PURPOSE      Looks for message flags.
//
// Parameters:
//     mcastAddr:      multicast address
//     memberFlag:     member flag to lookup.
//
// Return: TRUE if seen before; FALSE otherwise.
//---------------------------------------------------------------------------
static
MAODV_M_Node *MaodvLookupMemberFlag(
    NodeAddress mcastAddr,
    MAODV_Membership *memberFlag)
{
    MAODV_M_Node *current;
    if (memberFlag->size == 0)
    {
        return (NULL);
    }

    for (current = memberFlag->head; current != NULL; current = current->next)
    {
        if (current->mcastAddr == mcastAddr)
        {
            return(current);
        }
    }

    return (NULL);
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLookupTreeMembership
// PURPOSE      Check if the node is a member of the multicast group.
//
// Parameters:
//     mcastAddr:     Multicast group to check.
//     memberFlag: Membership flag.
//
// Return: TRUE if member; FALSE otherwise.
//---------------------------------------------------------------------------

static
BOOL MaodvLookupTreeMembership(
    NodeAddress mcastAddr,
    MAODV_Membership *memberFlag)
{
    MAODV_M_Node *current = MaodvLookupMemberFlag(
        mcastAddr,
        memberFlag);

    if (current != NULL)
    {
        return(TRUE);
    }
    else
    {
        return(FALSE);
    }
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLookupGroupMembership
// PURPOSE      Check if the node is a member of the multicast group.
//
// Parameters:
//     mcastAddr:  Multicast group to check.
//     memberFlag:pointer to  MAODV_Membership
//
// Return: TRUE if member; FALSE otherwise.
//---------------------------------------------------------------------------
static
BOOL MaodvLookupGroupMembership(
    NodeAddress mcastAddr,
    MAODV_Membership *memberFlag)
{
    MAODV_M_Node *current = MaodvLookupMemberFlag(
        mcastAddr,
        memberFlag);

    if (current != NULL)
    {
        if (current->type == LEADER || current->type == MEMBER)
        {
            return(TRUE);
        }
    }
    return(FALSE);
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLookupLeadership
// PURPOSE      Check if the node is a Leader of the multicast group.
//
// Parameters:
//     mcastAddr:  Multicast group to check.
//     memberFlag:pointer to  MAODV_Membership
//
// Return: TRUE if member; FALSE otherwise.
//---------------------------------------------------------------------------
static
BOOL MaodvLookupLeadership(
    NodeAddress mcastAddr,
    MAODV_Membership *memberFlag)
{
    MAODV_M_Node *current = MaodvLookupMemberFlag(
        mcastAddr,
        memberFlag);

    if (current != NULL)
    {
        if (current->type == LEADER)
        {
            return(TRUE);
        }
    }

    return(FALSE);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvReplaceInsertRouteTable
//
// PURPOSE:  Insert/Update an entry into the route table
//
// ARGUMENTS: node,       current node.
//            maodv,       Maodv information base
//            destAddr,   Destination Address
//            destSeq,    Destination sequence number
//            hopCount,   Number of hops to the destination
//            nextHop,    Immediate forwarding node towards the destination
//            lifetime,   Life time of the route
//            activated,  Whether this is an active route
//            interfaceIndex, The interface through the message has been
//                            received (ie. the interface in which to direct
//                            packet to reach the destination)
//            routeTable, Routing table
//
// RETURN:    The route just modified or created
//---------------------------------------------------------------------------

static
MaodvRouteEntry *MaodvReplaceInsertRouteTable(
    Node *node,
    MaodvData *maodv,
    NodeAddress destAddr,
    int destSeq,
    int hopCount,
    NodeAddress nextHop,
    clocktype lifetime,
    BOOL activated,
    int  interfaceIndex,
    MaodvRoutingTable *routeTable)
{
    MaodvRouteEntry *theNode = NULL;
    MaodvRouteEntry *current;
    MaodvRouteEntry *previous;

    // Find Insertion point.
    previous = NULL;
    current = routeTable->rtEntry;

    while ((current != NULL) && (current->destination.address < destAddr))
    {
        previous = current;
        current = current->next;
    }

    if ((current == NULL) || (current->destination.address != destAddr))
    {
        ++(routeTable->size);
        // Adding a new Entry here
        theNode = (MaodvRouteEntry *) MEM_malloc(sizeof(MaodvRouteEntry));
        theNode->lifetime = lifetime;
        theNode->activated = activated;
        theNode->destination.address = destAddr;
        theNode->lastHopCount = hopCount;

        ERROR_Assert(theNode->lastHopCount > 0, "Last hop count can't be < 0");

        if (previous == NULL)
        {
            theNode->next = routeTable->rtEntry;
            routeTable->rtEntry = theNode;
        }
        else
        {
            theNode->next = previous->next;
            previous->next = theNode;
        }
    }
    else
    {
        current->lifetime = MAX(lifetime, current->lifetime);

        if (!current->activated)
        {
            current->activated = activated;
        }
        theNode = current;
    }

    theNode->destination.seqNum = destSeq;
    theNode->hopCount = hopCount;
    theNode->nextHop = nextHop;
    theNode->interface = interfaceIndex;

    return(theNode);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvDeleteRouteTable
//
// PURPOSE:  Remove an entry from the route table
//
// ARGUMENTS: node, The node deleting the route entry
//            destAddr, The destination address to be deleted
//            routeTable, Aodv routing table
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvDeleteRouteTable(
    Node *node,
    NodeAddress destAddr,
    MaodvRoutingTable *routeTable)
{
    MaodvRouteEntry *toFree;
    MaodvRouteEntry *current;

    if (routeTable->size == 0 || routeTable->rtEntry == NULL)
    {
        return;
    }
    else if ((routeTable->rtEntry->destination.address == destAddr) &&
        (routeTable->rtEntry->activated == FALSE))
    {
        if (routeTable->rtEntry->lifetime <= getSimTime(node))
        {
            toFree = routeTable->rtEntry;
            routeTable->rtEntry = toFree->next;
            MEM_free(toFree);
            --(routeTable->size);
        }
    }
    else
    {
        for (current = routeTable->rtEntry;
            current->next != NULL
            && current->next->destination.address < destAddr;
            current = current->next)
        {
        }

        if (current->next != NULL && current->next->destination.address ==
            destAddr && current->next->lifetime <= getSimTime(node))
        {
            toFree = current->next;
            current->next = toFree->next;

            MEM_free(toFree);
            --(routeTable->size);
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCheckRouteExist
//
// PURPOSE:  To check whether route to a particular destination exist.
//           this function serves dual purpose, in case of invalid route it
//           return the pointer to the route with setting isValid flag to
//           FALSE. And in case of valid routes it returns the valid route
//           pointer with setting the isValid flag to TRUE
//
// ARGUMENTS: node, the node searching for a route
//            destAddr,   destination address of the packet
//            routeTable, aodv routing table to store possible routes
//            isValid,    to return if the route is a valid route or invalid
//                        route
//
// RETURN:    pointer to the route if it exists in the routing table,
//            NULL otherwise
//---------------------------------------------------------------------------

static
MaodvRouteEntry *MaodvCheckRouteExist(
    Node *node,
    NodeAddress destAddr,
    MaodvRoutingTable *routeTable,
    BOOL *isValid)
{
    MaodvRouteEntry *current;
    *isValid = FALSE;

    if (routeTable->size == 0)
    {
        // No entry in the routing table so return NULL
        return (NULL);
    }

    // Search for the entry
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddr;
        current = current->next)
    {
        if (current->destination.address == destAddr)
        {
            // Found the entry
            if (current->activated == TRUE)
            {
                // The entry is a valid route
                *isValid = TRUE;
            }
            return(current);
        }
    }
    // The entry doesn't exists
    return (NULL);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvGetNextHop
//
// PURPOSE:  Looks up the routing table to obtain next hop to the destinaton
//
// ARGUMENTS: destAddr, The destination address for which next hop is wanted
//            routeTable, Maodv routing table
//            nextHop, pointer to be assigned the next hop address
//            interfaceIndex, The interface index through which the message is
//                            to be sent
//
//
// RETURN:    TRUE, If one route is found
//            FALSE, otherwise
//---------------------------------------------------------------------------

static
BOOL MaodvGetNextHop(
    NodeAddress destAddr,
    MaodvRoutingTable *routeTable,
    NodeAddress *nextHop,
    int *interfaceIndex)
{
    MaodvRouteEntry *current;

    // Search for the route in the routing table
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddr;
        current = current->next)
    {
        if (current->destination.address == destAddr
            && current->activated ==TRUE)
        {
            // Got the route and this is a valid route
            // Assign next hop and interface index
            *nextHop = current->nextHop;
            *interfaceIndex = current->interface;
            return(TRUE);
        }
    }
    // The entry doesn't exist
    return(FALSE);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvUpdateLifetime
//
// PURPOSE: Update the lifetime field of the destination entry in the route
//          table
//
// ARGUMENTS: maodv, Data structure for Aodv internal variables
//            destAddr, The destination for which the life time to be updated
//            routeTable, Maodv routing table
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvUpdateLifetime(
    Node *node,
    MaodvData *maodv,
    NodeAddress destAddr,
    MaodvRoutingTable *routeTable)
{
    MaodvRouteEntry *current;

    // Search for the entry
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddr;
        current = current->next)
    {
        if (current->destination.address == destAddr)
        {
            // update lifetime to current time + active route timeout
            if (current->lifetime < getSimTime(node) +
                MAODV_ACTIVE_ROUTE_TIMEOUT)
            {
                current->lifetime = getSimTime(node) +
                    MAODV_ACTIVE_ROUTE_TIMEOUT;

                MaodvSetTimer(node, NULL, MSG_NETWORK_CheckRouteTimeout,
                    destAddr, (clocktype) MAODV_ACTIVE_ROUTE_TIMEOUT);
                return;
            }
        }
    }
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvDisableRoute
//
// PURPOSE:  disabling an active route
//
// ARGUMENTS: node, the node disabling the route
//            destAddress, address to be disabled
//            routeTable, maodv routing table
//
// RETURN:   The destination sequence number, 0 if the route doesn't exist
//---------------------------------------------------------------------------
static
unsigned int MaodvDisableRoute(
    Node *node,
    NodeAddress destAddress,
    MaodvRoutingTable *routeTable)
{
    MaodvRouteEntry *current = NULL;
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    // Search for the specified destination
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddress;
        current = current->next)
    {
        // Make sure that the route is not already activated.
        if (current->destination.address == destAddress)
        {
            // Got the destination disable it by making the hop count
            // infinity
            ERROR_Assert(current->activated == TRUE,
                "MAODV:  Route should be activated, but it is not.\n");
            // Copy the hop count field in the last hop count
            current->lastHopCount = current->hopCount;
            current->hopCount = MAODV_INFINITY;
            current->activated  = FALSE;
            // Set timer to delete the route after delete period
            current->lifetime   = getSimTime(node) + MAODV_DELETE_PERIOD;

            // An expired routing table entry SHOULD NOT be expunged before
            // (current_time + DELETE_PERIOD) (see section 8.13).  Otherwise,
            // the soft state corresponding to the route (e.g., Last Hop
            // Count) will be lost. Sec: 8.4
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_DeleteRoute,
                destAddress,
                (clocktype) MAODV_DELETE_PERIOD);

            return(++current->destination.seqNum);
        }
    }

    // Don't have the destination in the routing table.
    // Return destination sequence 0
    return (0);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvReplaceInsertNextHopsList
//
// PURPOSE:  Insert or modify nexthops list in multicast entry
//
// ARGUMENTS: maodv, The data structure to store maodv information base
//            mcastAddr, multicast address of the destination
//            nextHopAddr, The address to insert/modify
//            direction,   link direction
//            active,      activate flag
//
//            interfaceIndex, The interface through the message has been
//                            received (ie. the interface in which to direct
//                            packet to reach the destination)
//            nextHops, The  nextHops list
//            isEnabled, flag used to enable the next hop
//
// RETURN:   None
//---------------------------------------------------------------------------

static
MaodvNextHopNode *MaodvReplaceInsertNextHopsList(
    Node *node,
    NodeAddress mcastAddr,
    NodeAddress nextHopAddr,
    LinkDirection direction,
    int  interfaceIndex,
    BOOL isEnabled,
    MaodvNextHops *nextHops)
{
    MaodvNextHopNode *current = NULL;
    MaodvNextHopNode *previous = NULL;
    MaodvNextHopNode *newNode = NULL;
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    // Find Insertion point.  Insert after all address matches.
    previous = NULL;
    current = nextHops->head;

    while ((current != NULL) && (current->nextHopAddr < nextHopAddr))
    {
        previous = current;
        current = current->next;
    }

    if ((current == NULL) || (current->nextHopAddr != nextHopAddr))
    {
        ++(nextHops->size);
        // Adding a new Entry here
        newNode = (MaodvNextHopNode *) MEM_malloc(sizeof(MaodvNextHopNode));

        newNode->nextHopAddr = nextHopAddr;

        if (previous == NULL)
        {
            newNode->next = nextHops->head;
            nextHops->head = newNode;
        }
        else
        {
            newNode->next = previous->next;
            previous->next = newNode;
        }
        // setting timer
        newNode->isEnabled = isEnabled;

        if (isEnabled)
        {
            newNode->lifetime = getSimTime(node) + MAODV_HELLO_LIFE;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvTreeUtilizationTimer,
                mcastAddr,
                nextHopAddr,
                (clocktype) MAODV_HELLO_LIFE);

            if (direction == UPSTREAM_DIRECTION)
            {
                newNode->lifetime = getSimTime(node)
                    + MAODV_MDATA_FORWARD_TIMEOUT;
                MaodvSetMulticastTimer(
                    node,
                    MSG_ROUTING_MaodvCheckMroute,
                    mcastAddr,
                    nextHopAddr,
                    (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);
            }

        }
        else
        {
            newNode->lifetime = getSimTime(node) + MAODV_MTREE_BUILD;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvCheckNextHopTimeout,
                mcastAddr,
                nextHopAddr,
                (clocktype) MAODV_MTREE_BUILD);
        }
    }
    else
    {
        newNode = current;
        if (!isEnabled)
        {
            newNode->lifetime = getSimTime(node) + MAODV_MTREE_BUILD;
            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvCheckNextHopTimeout,
                mcastAddr,
                nextHopAddr,
                (clocktype) MAODV_MTREE_BUILD);
        }
        else
        {
            // enabling a valid nexthop
            newNode->lifetime = getSimTime(node) + MAODV_HELLO_LIFE;
            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvTreeUtilizationTimer,
                mcastAddr,
                nextHopAddr,
                (clocktype) MAODV_HELLO_LIFE);

            if (direction == UPSTREAM_DIRECTION)
            {
                newNode->lifetime = getSimTime(node)
                    + MAODV_MDATA_FORWARD_TIMEOUT;

                MaodvSetMulticastTimer(
                    node,
                    MSG_ROUTING_MaodvCheckMroute,
                    mcastAddr,
                    nextHopAddr,
                    (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);
            }
        }
    }

    newNode->direction = direction;

    if (newNode->isEnabled && !isEnabled)
    {
        newNode->lifetime = getSimTime(node) + MAODV_MTREE_BUILD;
        MaodvSetMulticastTimer(
            node,
            MSG_ROUTING_MaodvCheckNextHopTimeout,
            mcastAddr,
            nextHopAddr,
            (clocktype) MAODV_MTREE_BUILD);
    }

    newNode->isEnabled = isEnabled;
    newNode->interface = interfaceIndex;
    return(newNode);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCheckNextHopsList
//
// PURPOSE:  Checks Nexthop address in the list
//
// ARGUMENTS: nextHopAddr: next hop adddress
//            nextHops:    next hop node pointer
//
// RETURN:   if found next hop node is returned else null
//---------------------------------------------------------------------------
static
MaodvNextHopNode *MaodvCheckNextHopsList(
    NodeAddress nextHopAddr,
    MaodvNextHops *nextHops)
{
    MaodvNextHopNode *current;

    for (current = nextHops->head; current != NULL; current = current->next)
    {
        if (current->nextHopAddr == nextHopAddr)
        {
            return (current);
        }
    }
    return (NULL);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvFindUpstreamNextHop
//
// PURPOSE:  Checks Upstream Nexthop address in the list
//
// ARGUMENTS: nextHops: next hop node pointer
//
// RETURN:   if found next hop node is returned else null
//---------------------------------------------------------------------------
static
MaodvNextHopNode *MaodvFindUpstreamNextHop(MaodvNextHops *nextHops)
{
    MaodvNextHopNode *current = NULL;

    for (current = nextHops->head; current != NULL; current = current->next)
    {
        if (current->direction == UPSTREAM_DIRECTION)
        {
            return (current);
        }
    }
    return (NULL);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvDeleteNextHop
//
// PURPOSE:  Delets Nexthop address from the list
//
// ARGUMENTS: nextHopAddr: next hop adddress
//            nextHops:    next hop node pointer
//            lastEntry:   last Entry boolean flag
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvDeleteNextHop(
    NodeAddress nextHopAddr,
    MaodvNextHops *nextHops,
    BOOL *lastEntry)
{
    MaodvNextHopNode *current;
    MaodvNextHopNode *toFree;
    *lastEntry = FALSE;

    if (nextHops->size == 0)
    {
        *lastEntry = TRUE;
        return;
    }
    else if (nextHops->head->nextHopAddr == nextHopAddr)
    {
        toFree = nextHops->head;
        nextHops->head = toFree->next;
        MEM_free(toFree);
        --(nextHops->size);
    }
    else
    {
        for (current =  nextHops->head;
            current->next != NULL && current->next->nextHopAddr < nextHopAddr;
            current = current->next)
        {
        }

        if (current->next != NULL && current->next->nextHopAddr == nextHopAddr)
        {
            toFree = current->next;
            current->next = toFree->next;
            MEM_free(toFree);
            --(nextHops->size);
        }
    }
    if (nextHops->size == 0)
    {
        *lastEntry = TRUE;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvFreeNextHops
//
// PURPOSE:  Frees next hops from the lists
//
// ARGUMENTS: nextHops:pointer to MaodvNextHops
//
// RETURN: None
//---------------------------------------------------------------------------
static
void MaodvFreeNextHops(MaodvNextHops *nextHops)
{
    MaodvNextHopNode *current = nextHops->head;
    MaodvNextHopNode *next;

    while (current != NULL)
    {
        next = current->next;
        MEM_free(current);
        current = next;
    }
    nextHops->head = NULL;
    nextHops->size = 0;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvReplaceInsertMrouteTable
//
// PURPOSE:  Insert/Update an entry into the multicast route table
//
// ARGUMENTS: node,        current node.
//            maodv,       Maodv information base
//            mcastAddr,   Multicast address of the group
//            mcastSeq,    sequence number
//            mcastLeader, address of the multicast group leader
//            hopCount,    hop count to the next multicast group member
//            hopCountLeader, hop count to the multicast leader
//
//            nextHop,    address to be added to the nextHop list
//            direction,  direction relative to the group leader
//            interfaceIndex, The interface through the message has been
//                            received (ie. the interface in which to direct
//                            packet to reach the destination)
//
//            isEnabled     enable flag of the nexthop
//            isMember     whether the  node is a member of multicast tree
//            mrouteTable, Multicast Routing table
//
// RETURN:    The route just modified or created
//---------------------------------------------------------------------------

static
MaodvMrouteEntry *MaodvReplaceInsertMrouteTable(
    Node *node,
    MaodvData *maodv,
    NodeAddress mcastAddr,
    int mcastSeq,
    NodeAddress mcastLeader,
    int hopCount,
    int hopCountLeader,
    clocktype lifetime,
    NodeAddress nextHop,
    LinkDirection direction,
    int  interfaceIndex,
    BOOL isEnabled,
    BOOL isValid,
    MaodvMroutingTable *mrouteTable)
{
    MaodvMrouteEntry *theNode = NULL;
    MaodvMrouteEntry *current;
    MaodvMrouteEntry *previous;

    // Find Insertion point.
    previous = NULL;
    current = mrouteTable->mrtEntry;

    while ((current != NULL) && (current->destination.address < mcastAddr))
    {
        previous = current;
        current = current->next;
    }

    if ((current == NULL) || (current->destination.address != mcastAddr))
    {
        ++(mrouteTable->size);
        // Adding a new Entry here
        theNode = (MaodvMrouteEntry *) MEM_malloc(sizeof(MaodvMrouteEntry));
        theNode->destination.address = mcastAddr;
        theNode->isMember = FALSE;
        MaodvInitNextHopsList(&theNode->nextHops);

        if (previous == NULL)
        {
            theNode->next = mrouteTable->mrtEntry;
            mrouteTable->mrtEntry = theNode;
        }
        else
        {
            theNode->next = previous->next;
            previous->next = theNode;
        }
    }
    else
    {
        theNode = current;
    }

    MaodvReplaceInsertNextHopsList(
        node,
        mcastAddr,
        nextHop,
        direction,
        interfaceIndex,
        isEnabled,
        &theNode->nextHops);

    theNode->destination.seqNum = mcastSeq;
    theNode->hopCount = hopCount;
    theNode->hopCountLeader = hopCountLeader;
    theNode->isValid = isValid;
    theNode->mcastLeader = mcastLeader;
    theNode->lifetime = lifetime;
    return(theNode);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCreateMrouteEntry
//
// PURPOSE:  Create an entry into the multicast route table
//
// ARGUMENTS: node,        current node.
//            maodv,       Maodv information base
//            mcastAddr,   Multicast address of the group
//            mcastSeq,    sequence number
//            mcastLeader, address of the multicast group leader
//            hopCount,    hop count to the next multicast group member
//            hopCountLeader, hop count to the multicast leader
//
//            isValid,    Validity flag
//            lifetime,   Route life time
//            mrouteTable, Multicast Routing table
//
// RETURN:    The route just created
//---------------------------------------------------------------------------
static
MaodvMrouteEntry *MaodvCreateMrouteEntry(
    Node *node,
    MaodvData *maodv,
    NodeAddress mcastAddr,
    int mcastSeq,
    NodeAddress mcastLeader,
    int hopCount,
    int hopCountLeader,
    BOOL isValid,
    clocktype lifetime,
    MaodvMroutingTable *mrouteTable)
{
    MaodvMrouteEntry *theNode = NULL;
    MaodvMrouteEntry *current;
    MaodvMrouteEntry *previous;

    // Find Insertion point.
    previous = NULL;
    current = mrouteTable->mrtEntry;

    while ((current != NULL) && (current->destination.address < mcastAddr))
    {
        previous = current;
        current = current->next;
    }

    if ((current == NULL) || (current->destination.address != mcastAddr))
    {
        ++(mrouteTable->size);

        // Adding a new Entry here
        theNode = (MaodvMrouteEntry *) MEM_malloc(sizeof(MaodvMrouteEntry));
        theNode->destination.address = mcastAddr;
        theNode->isMember = FALSE; // FALSE by default
        MaodvInitNextHopsList(&theNode->nextHops);

        if (previous == NULL)
        {
            theNode->next = mrouteTable->mrtEntry;
            mrouteTable->mrtEntry = theNode;
        }
        else
        {
            theNode->next = previous->next;
            previous->next = theNode;
        }
    }
    else
    {
        // The entry already exists so point to the current entry
        theNode = current;
    }

    theNode->destination.seqNum = mcastSeq;
    theNode->hopCount = hopCount;
    theNode->hopCountLeader = hopCountLeader;
    theNode->isValid = isValid;
    theNode->mcastLeader = mcastLeader;
    theNode->lifetime = lifetime;
    return(theNode);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCheckMrouteExist
//
// PURPOSE:  Checks route exists or not into the multicast route table
//
// ARGUMENTS: node,        current node.
//            mcastAddr,   Multicast address of the group
//            mrouteTable, Multicast Routing table
//            isValid,    Validity flag
//
// RETURN:    existing route jor null
//---------------------------------------------------------------------------
static
MaodvMrouteEntry *MaodvCheckMrouteExist(
    Node *node,
    NodeAddress mcastAddr,
    MaodvMroutingTable *mrouteTable,
    BOOL *isValid)
{
    MaodvMrouteEntry *current;
    *isValid = FALSE;

    if (mrouteTable->size == 0)
    {
        // No entry in the routing table so return NULL
        return (NULL);
    }

    // Search for the entry
    for (current = mrouteTable->mrtEntry;
        current != NULL && current->destination.address <= mcastAddr;
        current = current->next)
    {
        if (current->destination.address == mcastAddr)
        {
            // Found the entry
            if (current->isValid == TRUE)
            {
                // The entry is a valid route
                *isValid = TRUE;
            }
            return(current);
        }
    }
    // The entry doesn't exists
    return (NULL);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvDeleteMroute
//
// PURPOSE:  Deletes  existing route from the multicast route table
//
// ARGUMENTS: node,        current node.
//            mcastAddr,   Multicast address of the group
//            mrouteTable, Multicast Routing table
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvDeleteMroute(
    Node *node,
    NodeAddress mcastAddr,
    MaodvMroutingTable *mrtTable)
{
    MaodvMrouteEntry * current = mrtTable->mrtEntry;
    MaodvMrouteEntry * toFree = NULL;

    if (mrtTable->size == 0 || mrtTable->mrtEntry == NULL)
    {
        return;
    }
    else if (mrtTable->mrtEntry->destination.address == mcastAddr)
    {
        toFree = mrtTable->mrtEntry;
        mrtTable->mrtEntry = toFree->next;
        MaodvFreeNextHops(&toFree->nextHops);
        MEM_free(toFree);
        --(mrtTable->size);
    }
    else
    {
        for (current =  mrtTable->mrtEntry;
            current->next != NULL
            && current->next->destination.address < mcastAddr;
            current = current->next)
        {
        }

        if (current->next != NULL
            && current->next->destination.address == mcastAddr)
        {
            toFree = current->next;
            current->next = toFree->next;
            MaodvFreeNextHops(&toFree->nextHops);
            MEM_free(toFree);
            --(mrtTable->size);
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvReplaceInsertRequestTable
//
// PURPOSE:  Delets  existing route from the multicast route table
//
// ARGUMENTS: node,        current node.
//            mcastAddr,   Multicast address of the group
//            mrouteTable, Multicast Routing table
//
// RETURN:    Request entry
//---------------------------------------------------------------------------
static
MaodvRequestEntry *MaodvReplaceInsertRequestTable(
    NodeAddress mcastAddr,
    NodeAddress leaderAddr,
    MaodvRequestTable *requestTable)
{
    MaodvRequestEntry *current;
    MaodvRequestEntry *previous;
    MaodvRequestEntry *newNode = NULL;

    // Find Insertion point.  Insert after all address matches.
    // To make the list sorted in ascending order
    previous = NULL;
    current = requestTable->requestEntry;

    while ((current != NULL) && (current->mcastAddr < mcastAddr))
    {
        previous = current;
        current = current->next;
    }

    if (current == NULL || current->mcastAddr != mcastAddr)
    {
        // adding a new entry here
        newNode = (MaodvRequestEntry *) MEM_malloc(sizeof(MaodvRequestEntry));
        newNode->mcastAddr = mcastAddr;
        (requestTable->size)++;
        if (previous == NULL)
        {
            newNode->next = requestTable->requestEntry;
            requestTable->requestEntry = newNode;
        }
        else
        {
            newNode->next = previous->next;
            previous->next = newNode;
        }
    }
    else
    {
        newNode = current;
    }

    newNode->leaderAddr = leaderAddr;
    return (newNode);
}

static
MaodvRequestEntry *MaodvCheckRequestTable(
    NodeAddress mcastAddr,
    MaodvRequestTable *requestTable)
{
    MaodvRequestEntry *current = requestTable->requestEntry;

    if (requestTable->size == 0)
    {
        return(NULL);
    }

    while ((current != NULL) && (current->mcastAddr <= mcastAddr))
    {
        if (current->mcastAddr == mcastAddr)
        {
            // entry found
            return(current);
        }
        current = current->next;
    }
    return(NULL);
}

static
void MaodvDeleteRequestTable(
    NodeAddress mcastAddr,
    MaodvRequestTable *requestTable)
{
    MaodvRequestEntry *toFree;
    MaodvRequestEntry *current = requestTable->requestEntry;

    if (requestTable->size == 0)
    {
        return;
    }
    else if (current->mcastAddr == mcastAddr)
    {
        toFree = current;
        current = toFree->next;
        --(requestTable->size);
        MEM_free(toFree);
        return;
    }
    else
    {
        for (current = requestTable->requestEntry;
            current->next != NULL && current->next->mcastAddr < mcastAddr;
            current = current->next)
        {
        }

        if (current->next != NULL && current->next->mcastAddr == mcastAddr)
        {
            toFree = current->next;
            current->next = toFree->next;
            MEM_free(toFree);
            --(requestTable->size);
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvIncreaseTtl
//
// PURPOSE:  Increase the TTL value of a destination, to which rreq has
//           been sent
//
// ARGUMENTS: maodv, Aodv main structure
//            current, the entry in sent, for which the ttl to be incremented
//
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvIncreaseTtl(
    MaodvData *maodv,
    MaodvRreqSentNode *current)
{
    current->ttl += MAODV_TTL_INCREMENT;

    if (current->ttl > MAODV_TTL_THRESHOLD)
    {
        // over ttl threshold ttl will be net diameter
        current->ttl = MAODV_NET_DIAMETER;
    }
    return;
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvGetLastHopCount
//
// PURPOSE:  Obtains the last hop count known to the destination node
//
// ARGUMENTS: destAddr, for which the next hop is wanted
//            mrouteTable, Maodv multicast routing table
//             routeTable  routing table
//
// RETURN:    The last hop count if found, TTL_START otherwise.
//---------------------------------------------------------------------------

static
int MaodvGetLastHopCount(
    Node *node,
    NodeAddress destAddr,
    MaodvRoutingTable *routeTable,
    MaodvMroutingTable *mrouteTable)
{
    MaodvMrouteEntry *mcurrent;
    MaodvRouteEntry *current;

    // Go through all the entries of the routing table to get a matching
    // for the specified destination
    for (mcurrent = mrouteTable->mrtEntry;
        mcurrent != NULL && mcurrent->destination.address <= destAddr;
        mcurrent = mcurrent->next) {

        if (mcurrent->destination.address == destAddr)
        {
            ERROR_Assert(mcurrent->hopCount >= 0,
                "MAODV: invalid last hop count for the group.\n");
            // Got the matching destination so return the hopcount
            return(mcurrent->hopCount);
        }
    }

    // Go through all the entries of the routing table to get a matching
    // for the specified destination
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddr;
        current = current->next)
    {
        if (current->destination.address == destAddr)
        {
            ERROR_Assert(current->lastHopCount > 0,
                "MAODV: invalid last hop count.\n");
            // Got the matching destination so return the hopcount
            return(current->lastHopCount);
        }
    }
    // No match found for the destination so return TTL_START.
    return (MAODV_TTL_START);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvGetSeq
//
// PURPOSE:  Obtains the sequence number of the destination node, if there is
//           any entry in the routing table for the destination. If there is
//           is no entry for the destination a sequence number 0 is returned
//
// ARGUMENTS: destAddr, The destination address for which a sequence number
//                      is wanted.
//            routeTable, maodv routing table.
//
// RETURN:    sequence number if there is one existing expired or invalid
//            route 0 otherwise.
//---------------------------------------------------------------------------

static
int MaodvGetSeq(
    NodeAddress destAddress,
    MaodvRoutingTable *routeTable,
    MaodvMroutingTable *mrouteTable)
{
    MaodvRouteEntry *current;
    MaodvMrouteEntry *mcurrent;
    // search thro the multicast table
    for (mcurrent = mrouteTable->mrtEntry;
        mcurrent != NULL && mcurrent->destination.address <= destAddress;
        mcurrent = mcurrent->next)
    {
        if (mcurrent->destination.address == destAddress)
        {
            // Got the desired destination
            return(mcurrent->destination.seqNum);
        }
    }
    // Search through all the entries to match the destination.
    for (current = routeTable->rtEntry;
        current != NULL && current->destination.address <= destAddress;
        current = current->next)
    {
        if (current->destination.address == destAddress)
        {
            // Got the desired destination
            return(current->destination.seqNum);
        }
    }

    // No entry for the destination so return o
    return (0);
}

//---------------------------------------------------------------------------
// FUNCTION: RoutingModvTransmitMdata
//
// PURPOSE:  Forward the data packet to the next hop
//
// ARGUMENTS: node, The node which is transmitting or forwarding data
//            msg,  The packet to be forwarded
//            destAddr, The destination to which the packet is to be sent
//
// RETURN:    None
//
//---------------------------------------------------------------------------

static
void MaodvTransmitMdata(
    Node *node,
    Message *msg,
    MaodvMrouteEntry *mrtEntryToDest,
    NodeAddress previousHopAddress,
    int incomingInterface)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    int validNextHops = 0;
    int i;
    NodeAddress nextHopsToRx[20];

    MaodvNextHopNode *nextHopNode = NULL;

    MESSAGE_SetLayer(msg, MAC_LAYER, 0);
    MESSAGE_SetEvent(msg, MSG_MAC_FromNetwork);

    if (previousHopAddress == ANY_IP)
    {
        maodv->stats.numMdataInitiated++;
    }
    else
    {
        maodv->stats.numMdataForwarded++;
    }

    for (nextHopNode = mrtEntryToDest->nextHops.head;
        nextHopNode != NULL;
        nextHopNode = nextHopNode->next)
    {
        if (nextHopNode->isEnabled
            && nextHopNode->nextHopAddr != previousHopAddress)
        {
            nextHopsToRx[validNextHops++] = nextHopNode->nextHopAddr;
            i = nextHopNode->interface;
        }
    }

    if (validNextHops > 0)
    {
        for (i = 0; i < node->numberInterfaces; i++)
        {
            clocktype delay = (clocktype)
                (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);

            NetworkIpSendPacketToMacLayerWithDelay(
                node,
                MESSAGE_Duplicate(node, msg),
                i,
                ANY_DEST,
                delay);
         }

        maodv->lastBroadcastSent = getSimTime(node);

        // Setting up timers
        for (i = 0; i < validNextHops; i++)
        {
            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvRetransmitTimer,
                mrtEntryToDest->destination.address,
                nextHopsToRx[i],
                (clocktype) MAODV_RETRANSMIT_TIME);
        }
        }

    MESSAGE_Free(node,msg);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvTransmitData
//
// PURPOSE:  Forward the data packet to the next hop
//
// ARGUMENTS: node, The node which is transmitting or forwarding data
//            msg,  The packet to be forwarded
//            destAddr, The destination to which the packet is to be sent
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvTransmitData(
    Node *node,
    Message *msg,
    MaodvRouteEntry *rtEntryToDest,
    NodeAddress previousHopAddress)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    IpHeaderType *ipHeader = (IpHeaderType *) MESSAGE_ReturnPacket(msg);

    MESSAGE_SetLayer(msg, MAC_LAYER, 0);

    MESSAGE_SetEvent(msg, MSG_MAC_FromNetwork);

    NetworkIpSendPacketToMacLayer(
        node,
        msg,
        rtEntryToDest->interface,
        rtEntryToDest->nextHop);

    // Each time a route is used to forward a data packet, its Lifetime
    // field is updated to be no less than the current time plus
    // ACTIVE_ROUTE_TIMEOUT. Sec: 8.2

    rtEntryToDest->lifetime = getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT;

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_CheckRouteTimeout,
        rtEntryToDest->destination.address,
        (clocktype) MAODV_ACTIVE_ROUTE_TIMEOUT);

    // Since the route between each source and
    // destination pair are expected to be symmetric, the Lifetime
    // for the previous hop, along the reverse path back to the IP
    // source, is also updated to be no less than the current time plus
    // ACTIVE_ROUTE_TIMEOUT. Sec 8.2

    if (previousHopAddress != ANY_IP)
    {
        MaodvUpdateLifetime(
            node,
            maodv,
            previousHopAddress,
            &maodv->routeTable);
    }

    // Update lifetime of source
    if (previousHopAddress != ipHeader->ip_src)
    {
        MaodvUpdateLifetime(node, maodv, ipHeader->ip_src, &maodv->routeTable);
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvBroadcastHelloMessage
//
// PURPOSE:  Function to advertize hello message if a node wants to
//
// ARGUMENTS: node, The node which is transmitting or forwarding data
//            msg,  The packet to be forwarded
//            destAddr, The destination to which the packet is to be sent
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvBroadcastHelloMessage(
    Node *node,
    MaodvData *maodv,
    NodeAddress nextHop)
{
    Message *newMsg;
    RrepPacket *rrepPkt;
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;

    char *pktPtr;
    int pktSize = sizeof(RrepPacket);
    int i= 0;

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    rrepPkt = (RrepPacket *) pktPtr;

    // Section 8.4 of draft-ietf-manet-aodv-08.txt
    // Allocate the message and then broadcast to all interfaces

    RrepPacketSetType(&(rrepPkt->Rreppkt), MAODV_RREP);
    RrepPacketSetR(&(rrepPkt->Rreppkt), 0);
    RrepPacketSetResv(&(rrepPkt->Rreppkt), 0);
    RrepPacketSetPrefixSz(&(rrepPkt->Rreppkt), 0);

    // No specification in the draft for source address!
    rrepPkt->sourceAddr = ANY_IP;

    //MaodvGetMySeq(maodv);

    rrepPkt->destination.seqNum = MaodvGetMySeq(maodv);
    RrepPacketSetHopCount(&(rrepPkt->Rreppkt), 0);
    rrepPkt->lifetime = (unsigned int) (MAODV_ALLOWED_HELLO_LOSS *
        MAODV_HELLO_INTERVAL) / MILLI_SECOND;

    for (i = 0; i < node->numberInterfaces; i++)
    {
        IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];
        if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
        {
            clocktype delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) *
                MAODV_BROADCAST_JITTER);

            //rrepPkt = (RrepPacket *) MESSAGE_ReturnPacket(newMsg);
            rrepPkt->destination.address = intfInfo->ipAddress;

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                MESSAGE_Duplicate(node, newMsg),
                intfInfo->ipAddress,
                nextHop,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                1,
                i,
                nextHop,
                delay);
        }
    }

    if (nextHop == ANY_DEST)
    {
        maodv->stats.numHelloSent++;
    }
    MESSAGE_Free(node, newMsg);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvBroadcastGroupHelloMessage
//
// PURPOSE:  Function to advertize group hello message
//
// ARGUMENTS: node, The node which is transmitting or forwarding data
//            mcastAddr,  Multicast address
//            seqNum, Sequence Number
//            leaderAddr, Group leader address
//            maodv, node's maodv data
//            hopCount, hop count
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvBroadcastGroupHelloMessage(
    Node *node,
    NodeAddress mcastAddr,
    unsigned int seqNum,
    NodeAddress leaderAddr,
    BOOL uflag,
    MaodvData *maodv,
    int hopCount)
{
    Message *newMsg;
    GrphPacket *gpHelloPkt;

    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;

    char *pktPtr;
    int pktSize = sizeof(GrphPacket);
    int i= 0;

    if (DEBUG_GROUP_HELLO)
    {
        char time[100];
        char grpAddrStr[25];
        char leaderStr[25];

        TIME_PrintClockInSecond(getSimTime(node), time);
        IO_ConvertIpAddressToString(mcastAddr, grpAddrStr);
        IO_ConvertIpAddressToString(leaderAddr, leaderStr);

        printf("\nMaodvBroadcastGroupHelloMessage: Node %u, time %s\n",
            node->nodeId, time);
        printf("\tSending group hello for address %s, seq %u, leader %s\n",
            grpAddrStr, seqNum, leaderStr);
    }

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    gpHelloPkt = (GrphPacket *) pktPtr;

    // Section 8.4 of draft-ietf-manet-aodv-08.txt
    // Allocate the message and then broadcast to all interfaces
    GrphPacketSetType(&(gpHelloPkt->GrphPkt), MAODV_GRPH);

    if (uflag)
    {
        GrphPacketSetU(&(gpHelloPkt->GrphPkt), 1);
    }
    else
    {
        GrphPacketSetU(&(gpHelloPkt->GrphPkt), 0);
    }

    GrphPacketSetM(&(gpHelloPkt->GrphPkt), 0);
    GrphPacketSetHopCount(&(gpHelloPkt->GrphPkt), hopCount);
    gpHelloPkt->leaderAddr = leaderAddr;
    gpHelloPkt->mcastGroup.address = mcastAddr;
    gpHelloPkt->mcastGroup.seqNum = seqNum;

    for (i = 0; i < node->numberInterfaces; i++)
    {
        IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];

        if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
        {
            clocktype delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) *
                MAODV_BROADCAST_JITTER);

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                MESSAGE_Duplicate(node, newMsg),
                intfInfo->ipAddress,
                ANY_DEST,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                1,
                i,
                ANY_DEST,
                delay);
        }
    }
    maodv->stats.numGroupHelloSent++;

    MaodvInsertSeenTable(
        node,
        mcastAddr,
        seqNum,
        leaderAddr,
        &maodv->seenTable);

    MESSAGE_Free(node, newMsg);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayGroupHello
//
// PURPOSE:  Function to reply group hello message
//
// ARGUMENTS: node, The node which is repying
//            maodv, node's maodv data
//            msg, The reply message
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvRelayGroupHello(Node *node, MaodvData *maodv, Message *msg)
{
    Message *newMsg = NULL;
    GrphPacket *newGrph;
    GrphPacket *oldGrph;
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
    int pktSize = sizeof(GrphPacket);
    char *pktPtr;
    int i;
    BOOL isValidRt = FALSE;

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    oldGrph = (GrphPacket *) MESSAGE_ReturnPacket(msg);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newGrph = (GrphPacket *) pktPtr;
    GrphPacketSetType(&(newGrph->GrphPkt),
        GrphPacketGetType(oldGrph->GrphPkt));
    GrphPacketSetU(&(newGrph->GrphPkt), GrphPacketGetU(oldGrph->GrphPkt));
    if (GrphPacketGetM(oldGrph->GrphPkt) == 1)
    {
        GrphPacketSetM(&(newGrph->GrphPkt), 1);
    }
    else
    {
        if (isValidRt)
        {
            GrphPacketSetM(&(newGrph->GrphPkt), 0);
        }
        else
        {
            GrphPacketSetM(&(newGrph->GrphPkt), 1);
        }
    }

    GrphPacketSetResv(&(newGrph->GrphPkt), 0);
    newGrph->mcastGroup.address = oldGrph->mcastGroup.address;
    newGrph->mcastGroup.seqNum = oldGrph->mcastGroup.seqNum;
    newGrph->leaderAddr = oldGrph->leaderAddr;
    GrphPacketSetHopCount(&(newGrph->GrphPkt),
        (GrphPacketGetHopCount(oldGrph->GrphPkt) + 1));

    for (i = 0; i < node->numberInterfaces; i++)
    {
        IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];
        if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
        {
            clocktype delay = (clocktype)
                (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                MESSAGE_Duplicate(node, newMsg),
                intfInfo->ipAddress,
                ANY_DEST,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                1,
                i,
                ANY_DEST,
                delay);
        }
    }
    maodv->lastBroadcastSent = getSimTime(node);
    maodv->stats.numGroupHelloRelayed++;
    MESSAGE_Free(node, newMsg);
    return;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvDeclareAsLeader
//
// PURPOSE:  Function to declare as group leader
//
// ARGUMENTS: node, The node which declares
//            mcastAddr, multicast address
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvDeclareAsLeader(Node *node, NodeAddress mcastAddr)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    // create route table entry
    MaodvMrouteEntry *mrtToDest;
    MaodvRequestEntry *rqEntry;
    MaodvNextHopNode *upstreamNode;
    BOOL isValidRt;
    MAODV_M_Node *member;


    if (DEBUG_LEADER)
    {
        char time[100], addrStr[25];
        IO_ConvertIpAddressToString(mcastAddr, addrStr);
        TIME_PrintClockInSecond(getSimTime(node), time);
        printf("\nNode %u is declaring itself a Group leader for %s at %s\n",
            node->nodeId, addrStr, time);
    }
    mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValidRt);

    if (mrtToDest == NULL)
    {
        mrtToDest = MaodvCreateMrouteEntry(
            node,
            maodv,
            mcastAddr,
            1,
            NetworkIpGetInterfaceAddress(node, 0), // Default intf address
            0,
            0,
            TRUE,
            getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
            &maodv->mrouteTable);
    }

    upstreamNode = MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

    if (upstreamNode != NULL)
    {
        BOOL lastEntry;

        MaodvDeleteNextHop(
            upstreamNode->nextHopAddr,
            &mrtToDest->nextHops,
            &lastEntry);
    }

    mrtToDest->isMember = TRUE;
    mrtToDest->isValid = TRUE;
    mrtToDest->hopCount = 0;
    mrtToDest->hopCountLeader = 0;
    mrtToDest->mcastLeader = NetworkIpGetInterfaceAddress(node, 0);

    // increase sequence numbers
    MaodvIncreaseSeq(maodv);

    // update request table
    rqEntry = MaodvReplaceInsertRequestTable(
        mcastAddr,
        NetworkIpGetInterfaceAddress(node, 0),
        &maodv->requestTable);

    // change status to leader

    member = MaodvSetMembershipType(mcastAddr, LEADER, &maodv->memberFlag);

    ERROR_Assert(member != NULL, "MaodvDeclareAsLeader: Not a member!!");

    member->isUpdate = FALSE;

    // start sending group hello messages
    MaodvSetTimer(node, NULL, MSG_ROUTING_MaodvSendGroupHello, mcastAddr, 0);

    // get the buffered packets to the group and start sending them
    // note: since in this case there are no receivers in the group
    // the packets will eventuallly be discarded at the Router

    while (MaodvLookupBuffer(mcastAddr, &maodv->msgBuffer))
    {
        NodeAddress previousHop;

        Message *newMsg = MaodvGetBufferedPacket(
            mcastAddr,
            &previousHop,
            &maodv->msgBuffer);

        MaodvTransmitMdata(
            node,
            MESSAGE_Duplicate(node, newMsg),
            mrtToDest,
            previousHop,
            0); // incomming interface

        MaodvDeleteBuffer(mrtToDest->destination.address, &maodv->msgBuffer);
        MESSAGE_Free(node, newMsg);
    }
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateMGIRREP
//
// PURPOSE:  Initiating route reply message with MGI
//
// ARGUMENTS: node, The node sending the route reply
//            aodv, Aodv main data structure
//            msg,  Received Route request message
//            interfaceIndex, The interface through which the RREP should
//                            be sent
//            nextHop, The node to which Route reply should be sent
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvInitiateMGIRREP(
    Node *node,
    MaodvData *maodv,
    Message *msg,
    int interfaceIndex,
    NodeAddress nextHop,
    MaodvMrouteEntry *mrtToDest)
{
    Message *newMsg;
    RreqPacket *rreqPkt;
    MgiextPacket *mgiextPkt;
    char *pktPtr;
    int pktSize = sizeof(MgiextPacket);

    rreqPkt = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_AODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    mgiextPkt = (MgiextPacket *) pktPtr;

    // Section 8.4 of draft-ietf-manet-aodv-08.txt
    MgiextPacketSetType(&(mgiextPkt->MgiextPkt), MAODV_MGIEXT);
    MgiextPacketSetR(&(mgiextPkt->MgiextPkt), 0);
    MgiextPacketSetResv(&(mgiextPkt->MgiextPkt), 0);
    MgiextPacketSetPrefixSz(&(mgiextPkt->MgiextPkt), 0);

    mgiextPkt->sourceAddr = rreqPkt->source.address;
    mgiextPkt->destination.address = rreqPkt->destination.address;

    mgiextPkt->destination.seqNum = mrtToDest->destination.seqNum;

    MgiextPacketSetHopCount(&(mgiextPkt->MgiextPkt), 0);
    mgiextPkt->lifetime = (unsigned int) (MAODV_MY_ROUTE_TIMEOUT /
        MILLI_SECOND);

    mgiextPkt->groupHopCount = mrtToDest->hopCountLeader;
    mgiextPkt->leader = mrtToDest->mcastLeader;

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        nextHop);

    maodv->stats.numReplyInitiatedAsDest++;
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateRREP
//
// PURPOSE:  Initiating route reply message
//
// ARGUMENTS: node, The node sending the route reply
//            aodv, Aodv main data structure
//            msg,  Received Route request message
//            interfaceIndex, The interface through which the RREP should
//                            be sent
//            nextHop, The node to which Route reply should be sent
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvInitiateRREP(
    Node *node,
    MaodvData *maodv,
    Message *msg,
    int interfaceIndex,
    NodeAddress nextHop,
    MaodvMrouteEntry *mrtToDest)
{
    Message *newMsg;
    RreqPacket *rreqPkt;
    RrepPacket *rrepPkt;
    char *pktPtr;
    int pktSize = sizeof(RrepPacket);

    rreqPkt = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    rrepPkt = (RrepPacket *) pktPtr;

    // Section 8.4 of draft-ietf-manet-aodv-08.txt
    RrepPacketSetType(&(rrepPkt->Rreppkt), MAODV_RREP);
    RrepPacketSetR(&(rrepPkt->Rreppkt), 0);
    RrepPacketSetResv(&(rrepPkt->Rreppkt), 0);
    RrepPacketSetPrefixSz(&(rrepPkt->Rreppkt), 0);

    rrepPkt->sourceAddr = rreqPkt->source.address;
    rrepPkt->destination.address = rreqPkt->destination.address;

    rrepPkt->destination.seqNum = mrtToDest->destination.seqNum;

    RrepPacketSetHopCount(&(rrepPkt->Rreppkt), 0);
    rrepPkt->lifetime = (unsigned int) (MAODV_MY_ROUTE_TIMEOUT / MILLI_SECOND);

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        nextHop);

    maodv->stats.numReplyInitiatedAsDest++;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateRREPbyIN
//
// PURPOSE:  An intermediate node that knows the route to the destination
//           sends the RREP
//
// ARGUMENTS: node, the node generating rrep.
//            maodv, maodv main data structure
//            msg,  rreq received
//            lastHopAddress, last hop address in routing table for the
//                            destination
//            interfaceIndex, the interface from which rreq received
//
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvInitiateRREPbyIN(
    Node *node,
    MaodvData *maodv,
    Message *msg,
    NodeAddress lastHopAddress,
    int interfaceIndex,
    MaodvRouteEntry *rtToDest)
{
    Message *newMsg = NULL;
    RreqPacket *rreqPkt = NULL;
    RrepPacket *rrepPkt = NULL;
    char *pktPtr = NULL;
    int pktSize = sizeof(RrepPacket);

    // we don't want to send RREP if life time becomes 0
    if ((unsigned int) ((getSimTime(node) - rtToDest->lifetime) /
        MILLI_SECOND) == 0)
    {
        return;
    }

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    rrepPkt = (RrepPacket *) pktPtr;

    rreqPkt = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    RrepPacketSetType(&(rrepPkt->Rreppkt), MAODV_RREP);
    rrepPkt->sourceAddr = rreqPkt->source.address;
    rrepPkt->destination.address = rreqPkt->destination.address;

    rrepPkt->destination.seqNum = rtToDest->destination.seqNum;

    rrepPkt->lifetime = (unsigned int)
        ((rtToDest->lifetime - getSimTime(node)) / MILLI_SECOND);

    RrepPacketSetHopCount(&(rrepPkt->Rreppkt), rtToDest->hopCount);

    if ((int)RrepPacketGetHopCount(rrepPkt->Rreppkt) > maodv->netDiameter)
    {
        ERROR_ReportError("MAODV: Please increase AODV_NET_DIAMETER.\n");
    }

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        lastHopAddress,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        lastHopAddress);

    maodv->stats.numReplyInitiatedAsIntermediate++;

    // Also, at each node the (reverse) route used to forward a RREP has
    // its lifetime changed to current time plus ACTIVE_ROUTE_TIMEOUT.
    MaodvUpdateLifetime(node, maodv, lastHopAddress, &maodv->routeTable);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayMGIEXT
//
// PURPOSE:  Reply an multicast GI extension
//
// ARGUMENTS: node, the node.
//            mrtToDest, Multicast route entry
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvRelayMGIEXT(Node *node,Message *msg,MaodvMrouteEntry *mrtToDest)
{
    Message *newMsg = NULL;
    MgiextPacket *newMgiext = NULL;
    char *pktPtr = NULL;
    BOOL isRtToSrcExist = FALSE;
    int pktSize = sizeof(MgiextPacket);
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MgiextPacket *oldMgiext = (MgiextPacket *) MESSAGE_ReturnPacket(msg);
    MaodvRouteEntry *rtToSource = MaodvCheckRouteExist(
        node,
        oldMgiext->sourceAddr,
        &maodv->routeTable,
        &isRtToSrcExist);

    // If the current node is not the source node as indicated by the Source
    // IP Address in the MGIEXT message AND a forward route has been created
    // or updated as described before, the node consults its route table
    // entry for the source node to determine the next hop for the MGIEXT
    // packet, and then forwards the MGIEXT towards the source with its Hop
    // Count incremented by one. Sec: 8.7

    if (!isRtToSrcExist)
    {
        return;
    }

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newMgiext = (MgiextPacket *) pktPtr;

    MgiextPacketSetType(&(newMgiext->MgiextPkt), MAODV_MGIEXT);
    MgiextPacketSetR(&(newMgiext->MgiextPkt),
        MgiextPacketGetR(oldMgiext->MgiextPkt));

    MgiextPacketSetResv(&(newMgiext->MgiextPkt), 0);
    MgiextPacketSetPrefixSz(&(newMgiext->MgiextPkt), 0);

    newMgiext->sourceAddr  = oldMgiext->sourceAddr;
    newMgiext->destination.address = oldMgiext->destination.address;
    newMgiext->destination.seqNum  = oldMgiext->destination.seqNum;
    MgiextPacketSetHopCount(&(newMgiext->MgiextPkt),
        (MgiextPacketGetHopCount(oldMgiext->MgiextPkt) + 1));
    newMgiext->groupHopCount = oldMgiext->groupHopCount;
    newMgiext->leader = oldMgiext->leader;

    if ((int)MgiextPacketGetHopCount(newMgiext->MgiextPkt) >
        maodv->netDiameter){
        ERROR_ReportError("MAODV: Please increase MAODV_NET_DIAMETER.\n");
    }

    newMgiext->lifetime = oldMgiext->lifetime;

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, rtToSource->interface),
        rtToSource->nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        rtToSource->interface,
        rtToSource->nextHop);

    maodv->stats.numMreplyForwarded++;

    // Also, at each node the (reverse) route used to forward a MGIEXT has
    // its lifetime changed to current time plus ACTIVE_ROUTE_TIMEOUT.
    MaodvUpdateLifetime(node, maodv, rtToSource->nextHop, &maodv->routeTable);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvPrintPacket
//
// PURPOSE:  Prints MAODv packets
//
// ARGUMENTS: node, the node for which it is printed
//            msg, The message containing the packet to be printed.
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvPrintPacket(Node *node, Message *msg)
{
    char address[100];
    //char *packetType = MESSAGE_ReturnPacket(msg);//original code

    RreqPacket *Pkttype = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    printf("\n ****************************************************** \n");
    //switch (*packetType)//original code

    switch (RreqPacketGetType(Pkttype->RreqPkt))
    {
    case MAODV_RREQ:
        {
            RreqPacket *rreqPkt = (RreqPacket *) MESSAGE_ReturnPacket(msg);
            printf("\t Type:\t RREQ");
            if (RreqPacketGetJ(rreqPkt->RreqPkt) == 1)
            {
                printf("-JOIN Request\n");
            }
            else
            {
                printf("-NON-JOIN Request\n");
            }
            printf("\t HopCount %u \n",
                RreqPacketGetHopCount(rreqPkt->RreqPkt));
            IO_ConvertIpAddressToString(rreqPkt->source.address, address);
            printf("\t Source address  = %s\n", address);
            printf("\t Source Seq Num %u \n", rreqPkt->source.seqNum);
            IO_ConvertIpAddressToString(
                rreqPkt->destination.address,
                address);

            printf("\t Destination address  = %s\n", address);
            printf("\t Destination Seq Num %u \n",
                rreqPkt->destination.seqNum);
            printf("\n Flooding ID %d \n", rreqPkt->floodingId);

            break;
        }
    case MAODV_MGLEXT:
        {
            MglextPacket *rreqPkt = (MglextPacket *)
                MESSAGE_ReturnPacket(msg);
            printf("\t Type:\t RREQ Join with MGL");
            printf("\t HopCount %u \n",
                MglextPacketGetHopCount(rreqPkt->MglextPkt));
            IO_ConvertIpAddressToString(rreqPkt->source.address, address);
            printf("\t Source address  = %s\n", address);
            printf("\t Source Seq Num %d \n", rreqPkt->source.seqNum);
            IO_ConvertIpAddressToString(
                rreqPkt->destination.address,
                address);

            printf("\t Destination address  = %s\n", address);
            printf("\t Destination Seq Num %d \n",
                rreqPkt->destination.seqNum);
            IO_ConvertIpAddressToString(rreqPkt->leader, address);
            printf("\t Leader address  = %s\n", address);
            printf("\n Flooding ID %d \n", rreqPkt->floodingId);

            break;
        }
    case MAODV_MGREXT:
        {
            MgrextPacket *rreqPkt = (MgrextPacket *)
                MESSAGE_ReturnPacket(msg);
            printf("\t Type:\t REPAIR RREQ");
            printf("\t HopCount %u \n",
                MgrextPacketGetHopCount(rreqPkt->MgrextPkt));
            IO_ConvertIpAddressToString(rreqPkt->source.address, address);
            printf("\t Source address  = %s\n", address);
            printf("\t Source Seq Num %d \n", rreqPkt->source.seqNum);
            IO_ConvertIpAddressToString(
                rreqPkt->destination.address,
                address);

            printf("\t Destination address  = %s\n", address);
            printf("\t Destination Seq Num %d \n",
                rreqPkt->destination.seqNum);

            printf("\t Group Hop count %d \n", rreqPkt->groupHopCount);
            printf("\n Flooding ID %d \n", rreqPkt->floodingId);
            printf("\n leader address %d \n", rreqPkt->leader);
            break;
        }
    case MAODV_MACT:
        {
            MactPacket *mactPkt = (MactPacket *) MESSAGE_ReturnPacket(msg);
            printf("\t Type:\t MACT");
            IO_ConvertIpAddressToString(mactPkt->mcastDest.address, address);
            printf("\t Group address  = %s\n", address);
            printf("\t source address = %d\n", mactPkt->srcAddr);
            printf("\t Group Seq Num %d \n", mactPkt->mcastDest.seqNum);
            if (MactPacketGetU(mactPkt->MactPkt))
            {
                printf("\t Hop count = %d \n",
                    MactPacketGetHopCount(mactPkt->MactPkt));
            }
            printf("\n Join %d \t Prune %d \t Group %d \t Update %d \t "
                "Repair %d \n",
                MactPacketGetJ(mactPkt->MactPkt),
                MactPacketGetP(mactPkt->MactPkt),
                MactPacketGetG(mactPkt->MactPkt),
                MactPacketGetU(mactPkt->MactPkt),
                MactPacketGetR(mactPkt->MactPkt));

            break;
        }
    case MAODV_RREP:
        {
            RrepPacket *rrepPkt = (RrepPacket *) MESSAGE_ReturnPacket(msg);
            if (rrepPkt->sourceAddr == ANY_DEST)
            {
                printf("\t Type: HELLO \n");
            }
            else
            {
                printf("\t Type:\t RREP \n");
                IO_ConvertIpAddressToString(rrepPkt->sourceAddr, address);
                printf("\t Source address  = %s\n", address);
            }
            IO_ConvertIpAddressToString(
                rrepPkt->destination.address,
                address);

            printf("\t Destination address  = %s\n", address);
            printf("\t Destination Seq Num %d \n",
                rrepPkt->destination.seqNum);
            break;
        }
    case MAODV_MGIEXT:
        {
            MgiextPacket *rrepPkt = (MgiextPacket *)
                MESSAGE_ReturnPacket(msg);

            if (MgiextPacketGetR(rrepPkt->MgiextPkt) == 1)
            {
                printf("\n Type: \t RREP with MGIEXT for MGL REQ \n");
            }
            else
            {
                printf("\t Type:\t RREP with MGIEXT \n");
            }

            IO_ConvertIpAddressToString(rrepPkt->sourceAddr, address);
            printf("\t Source address  = %s\n", address);
            IO_ConvertIpAddressToString(
                rrepPkt->destination.address,
                address);

            printf("\t Destination address  = %s\n", address);
            printf("\t Destination Seq Num %d \n",
                rrepPkt->destination.seqNum);
            printf("\t Leader %u \n", rrepPkt->leader);
            printf("\t Hop count to Leader %d \n", rrepPkt->groupHopCount);
            break;
        }
    case MAODV_GRPH:
        {
            GrphPacket *gphPkt = (GrphPacket *) MESSAGE_ReturnPacket(msg);
            printf("\t Type:\t GROUP HELLO \n");
            printf("\t HopCount %d \n",
                GrphPacketGetHopCount(gphPkt->GrphPkt));
            IO_ConvertIpAddressToString(gphPkt->mcastGroup.address, address);
            printf("\t Group address  = %s\n", address);
            printf("\t Group Seq Num %d \n", gphPkt->mcastGroup.seqNum);
            printf("\t Leader address %u \n", gphPkt->leaderAddr);
            printf("\t U = %d \n", GrphPacketGetU(gphPkt->GrphPkt));
            printf("\t M = %d \n", GrphPacketGetM(gphPkt->GrphPkt));
            break;
        }
    default:
        ERROR_Assert(FALSE, "\n unknown packet..\n");
    }
    printf("\n ****************************************************** \n\n");
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayMGLEXT
//
// PURPOSE:  Relays GL's ext.
//
// ARGUMENTS: node, the node
//            msg, The message containing the packet
//            srcAddr, Source Address
//            nextHop, NextHop Address
//            interfaceIndex, Interface index
//            seqNum, Sequence number
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvRelayMGLEXT(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    NodeAddress nextHop,
    unsigned int interfaceIndex,
    int seqNum)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg = NULL;
    MglextPacket *oldRreq;
    MglextPacket *newRreq;
    char *pktPtr;
    int pktSize = sizeof(MglextPacket);

    oldRreq = (MglextPacket *) MESSAGE_ReturnPacket(msg);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);

    newRreq = (MglextPacket *) pktPtr;

    MglextPacketSetType(&(newRreq->MglextPkt),
        MglextPacketGetType(oldRreq->MglextPkt));
    MglextPacketSetJ(&(newRreq->MglextPkt),
        MglextPacketGetJ(oldRreq->MglextPkt));
    MglextPacketSetR(&(newRreq->MglextPkt),
        MglextPacketGetR(oldRreq->MglextPkt));
    MglextPacketSetG(&(newRreq->MglextPkt),
        MglextPacketGetG(oldRreq->MglextPkt));

    newRreq->floodingId = oldRreq->floodingId;
    newRreq->destination.address = oldRreq->destination.address;
    newRreq->destination.seqNum = seqNum;
    newRreq->source.address = oldRreq->source.address;
    newRreq->source.seqNum = oldRreq->source.seqNum;
    MglextPacketSetHopCount(&(newRreq->MglextPkt),
        (MglextPacketGetHopCount(oldRreq->MglextPkt) + 1));
    newRreq->leader = oldRreq->leader;

    if ((int)MglextPacketGetHopCount(newRreq->MglextPkt) > maodv->netDiameter)
    {
        ERROR_ReportError("MAODV: Please increase MAODV_NET_DIAMETER.\n");
    }

    NetworkIpSendRawMessageToMacLayer(
        node,
        MESSAGE_Duplicate(node, newMsg),
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        nextHop);

    MESSAGE_Free(node,newMsg);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayMglReply
//
// PURPOSE:  Relays GL's reply.
//
// ARGUMENTS: node, the node
//            msg, The message containing the packet
//            nextHop, NextHop Address
//            interfaceIndex, Interface index
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvRelayMglReply(
    Node *node,
    Message *msg,
    NodeAddress nextHop,
    unsigned int interfaceIndex)
{
    Message *newMsg = NULL;
    MgiextPacket *newMgiext = NULL;
    char *pktPtr = NULL;
    int pktSize = sizeof(MgiextPacket);
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MgiextPacket *oldMgiext = (MgiextPacket *) MESSAGE_ReturnPacket(msg);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newMgiext = (MgiextPacket *) pktPtr;

    MgiextPacketSetType(&(newMgiext->MgiextPkt), MAODV_MGIEXT);
    MgiextPacketSetR(&(newMgiext->MgiextPkt),
        MgiextPacketGetR(oldMgiext->MgiextPkt));

    MgiextPacketSetResv(&(newMgiext->MgiextPkt), 0);
    MgiextPacketSetPrefixSz(&(newMgiext->MgiextPkt), 0);

    newMgiext->sourceAddr  = oldMgiext->sourceAddr;
    newMgiext->destination.address = oldMgiext->destination.address;
    newMgiext->destination.seqNum  = oldMgiext->destination.seqNum;
    MgiextPacketSetHopCount(&(newMgiext->MgiextPkt), (MgiextPacketGetHopCount
        (oldMgiext->MgiextPkt) + 1));
    newMgiext->groupHopCount = oldMgiext->groupHopCount;
    newMgiext->leader = oldMgiext->leader;

    if ((int)MgiextPacketGetHopCount(newMgiext->MgiextPkt) >
        maodv->netDiameter)
    {
        ERROR_ReportError("MAODV: Please increase MAODV_NET_DIAMETER.\n");
    }

    newMgiext->lifetime = oldMgiext->lifetime;

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        nextHop);

    maodv->stats.numMreplyForwarded++;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateRepairRREQ
//
// PURPOSE:  Repair route request.
//
// ARGUMENTS: node, the node
//            msg, The message containing the packet
//            destAddr, Destination Address
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvInitiateRepairRREQ(
    Node *node,
    NodeAddress destAddr)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message* newMsg = NULL;
    MaodvRreqSentNode* sentNode = NULL;
    MgrextPacket* rreqPkt = NULL;
    char* pktPtr = NULL;
    int pktSize = sizeof(MgrextPacket);
    int ttl;
    int i;
    NetworkDataIp* ip = (NetworkDataIp *) node->networkData.networkVar;
    MaodvMrouteEntry* mrtToDest = NULL;
    BOOL isValidRoute;

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);

    mrtToDest = MaodvCheckMrouteExist(
        node,
        destAddr,
        &maodv->mrouteTable,
        &isValidRoute);

    ERROR_Assert(mrtToDest != NULL, "MaodvInitiateRepairRREQ: For repair "
        "the node should have a mroute entry\n");

    rreqPkt = (MgrextPacket *) pktPtr;
    MgrextPacketSetType(&(rreqPkt->MgrextPkt), MAODV_MGREXT);
    MgrextPacketSetR(&(rreqPkt->MgrextPkt), 1);
    MgrextPacketSetJ(&(rreqPkt->MgrextPkt), 1);
    MgrextPacketSetResv(&(rreqPkt->MgrextPkt), 0);
    rreqPkt->floodingId = MaodvGetFloodingId(maodv);
    rreqPkt->destination.address = destAddr;
    rreqPkt->destination.seqNum = mrtToDest->destination.seqNum;
    rreqPkt->source.seqNum = ++maodv->seqNumber;
    rreqPkt->groupHopCount = mrtToDest->hopCountLeader;
    rreqPkt->leader = mrtToDest->mcastLeader;
    MgrextPacketSetHopCount(&(rreqPkt->MgrextPkt), 0);

    // In multicast there is no need for setting up a bi-directional connection
    // so unset the G flag.

    MgrextPacketSetG(&(rreqPkt->MgrextPkt), FALSE);

    // Check if rreq has already been sent
    sentNode = MaodvCheckSent(destAddr, &maodv->sent);

    if (sentNode != NULL)
    {
        ttl = sentNode->ttl;
    }
    else
    {
        ttl = mrtToDest->hopCount + MAODV_TTL_INCREMENT;

        if (ttl > MAODV_TTL_THRESHOLD)
        {
            ttl = MAODV_NET_DIAMETER;
        }

        sentNode = MaodvInsertSent(
            destAddr,
            ttl,
            REPAIR,
            getSimTime(node),
            &maodv->sent);
    }

    MaodvIncreaseTtl(maodv, sentNode);

    if (ttl >= MAODV_NET_DIAMETER)
    {
                sentNode->times++;
        }

    for (i = 0; i < node->numberInterfaces; i++)
    {
        IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];

        if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
        {
            clocktype delay = (clocktype)
                (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);

            Message* duplicateMsg = MESSAGE_Duplicate(node, newMsg);
            pktPtr = (char *) MESSAGE_ReturnPacket(duplicateMsg);
            rreqPkt = (MgrextPacket *) pktPtr;
            rreqPkt->source.address = intfInfo->ipAddress;

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                duplicateMsg,
                intfInfo->ipAddress,
                ANY_DEST,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                ttl,
                i,
                ANY_DEST,
                delay);

            MaodvInsertSeenTable(
                node,
                rreqPkt->source.address,
                rreqPkt->floodingId,
                0,
                &maodv->seenTable);
        }
    }

    MESSAGE_Free(node, newMsg);
    maodv->stats.numRepairReqInitiated++;
    maodv->lastBroadcastSent = getSimTime(node);

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_CheckReplied,
        destAddr,
        (clocktype) (2 * ttl * MAODV_NODE_TRAVERSAL_TIME));
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateRREQ
//
// PURPOSE:  Initiating a RREQ for a multicast group
//
// ARGUMENTS:
//      node, Node initiating the route request
//      destAddr, The multicast group
//      isBidirectional, Whether want to open a bidirectional connection
//      reqType, Whether JOIN, or NON_JOIN, REPAIR request
//
// RETURN: None
//---------------------------------------------------------------------------
static
void MaodvInitiateRREQ(
    Node *node,
    NodeAddress destAddr,
    RreqType reqType)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg = NULL;
    MaodvRreqSentNode *sentNode;
    RreqPacket *rreqPkt;

    char *pktPtr;
    int pktSize = sizeof(RreqPacket);
    int ttl;
    int i;
    clocktype delay;

    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;

    if (reqType == REPAIR)
    {
        MaodvInitiateRepairRREQ(node, destAddr);
        return;
    }

    // Allocate the route request packet
    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    rreqPkt = (RreqPacket *) pktPtr;

    RreqPacketSetType(&(rreqPkt->RreqPkt), MAODV_RREQ);

    if (reqType == JOIN)
    {
        // This is a join request so set the join flag.
        RreqPacketSetJ(&(rreqPkt->RreqPkt), 1);
    }
    else if (reqType == NONJOIN)
    {
        // This is not a join request to unset the join flag
        RreqPacketSetJ(&(rreqPkt->RreqPkt), 0);
    }

    RreqPacketSetR(&(rreqPkt->RreqPkt), 0);

    // In case of multicast packet there is not need for Gratuitous reply
    // as flow of packet will be in one way only.
    RreqPacketSetG(&(rreqPkt->RreqPkt), FALSE);

    RreqPacketSetResv(&(rreqPkt->RreqPkt), 0);
    rreqPkt->floodingId = MaodvGetFloodingId(maodv);
    rreqPkt->destination.address = destAddr;

    // The tree nodes should get the sequence number from the multicast
    // routing table, but the nodes which are not tree member (either a
    // group member or a router which forward packets between two members)
    // should not have an entry in the multicast routing table. So they will
    // search the group sequence number from their routing table.

    rreqPkt->destination.seqNum  = MaodvGetSeq(
        destAddr,
        &maodv->routeTable,
        &maodv->mrouteTable);

    rreqPkt->source.seqNum = ++maodv->seqNumber;
    RreqPacketSetHopCount(&(rreqPkt->RreqPkt), 0);

    // Check if a rreq has already been sent
    sentNode = MaodvCheckSent(destAddr, &maodv->sent);

    if (sentNode != NULL)
    {
        ttl = sentNode->ttl;

        MaodvIncreaseTtl(maodv, sentNode);

        if (ttl >= MAODV_NET_DIAMETER)
        {
            sentNode->times++;
        }

        if (reqType == JOIN)
        {
            maodv->stats.numJoinRequestResent++;
        }
        else
        {
            maodv->stats.numNonJoinRequestResent++;
        }
    }
    else
    {
        ttl = MAODV_TTL_START;

        sentNode = MaodvInsertSent(
            destAddr,
            ttl,
            reqType,
            getSimTime(node),
            &maodv->sent);

        MaodvIncreaseTtl(maodv, sentNode);

        if (reqType == JOIN)
        {
            maodv->stats.numJoinReqInitiated++;
        }
        else
        {
            maodv->stats.numNonJoinReqInitiated++;
        }
    }

    // Allocation and formation of the RREQ is complete so now send the
    // RREQ to all available interfaces.

    for (i = 0; i < node->numberInterfaces; i++)
    {
        IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];

        if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
        {
            delay = (clocktype)
                (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);

            Message* duplicateMsg = MESSAGE_Duplicate(node, newMsg);
            pktPtr = (char *) MESSAGE_ReturnPacket(duplicateMsg);
            rreqPkt = (RreqPacket *) pktPtr;

            rreqPkt->source.address = intfInfo->ipAddress;

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                duplicateMsg,
                intfInfo->ipAddress,
                ANY_DEST,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                ttl,
                i,
                ANY_DEST,
                delay);

            MaodvInsertSeenTable(
                node,
                rreqPkt->source.address,
                rreqPkt->floodingId,
                0,                       // Group leaders address
                &maodv->seenTable);
        }
    }

    MESSAGE_Free(node, newMsg);

    maodv->lastBroadcastSent = getSimTime(node);

    // set timer to check for reply
    delay = (clocktype) (2 * ttl * MAODV_NODE_TRAVERSAL_TIME +
        RANDOM_erand(maodv->jitterSeed) * MAODV_JITTER);

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_CheckReplied,
        destAddr,
        delay);

    if (DEBUG_ROUTE_DISCOVERY)
    {
        char time[100];
        char addr[25];
        TIME_PrintClockInSecond(getSimTime(node), time);
        IO_ConvertIpAddressToString(rreqPkt->destination.address, addr);
        printf("\nMaodvInitiateRREQ: Node %u\n", node->nodeId);
        printf("\tSending Route Request at %s\n", time);
        printf("\tDest %s, Dest Seq %u, ttl %u\n",
            addr,
            rreqPkt->destination.seqNum,
            ttl);

        TIME_PrintClockInSecond(delay, time);

        printf("\tCheck replied after %sS\n", time);
    }

}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateTreeCombineRequest
//
// PURPOSE:  Initiating a tree combine request
//
// ARGUMENTS:
//      node: Node initiating the route request
//      mcastAddr: The multicast address
//      leaderAddr: Group leader address
//      prevHop: Previous Hop
//      interfaceIndex: Interface index
//      hopCount: Hop count
//
// RETURN: None
//---------------------------------------------------------------------------
static
void  MaodvInitiateTreeCombineRequest(
    Node *node,
    NodeAddress mcastAddr,
    NodeAddress leaderAddr,
    NodeAddress prevHop,
    unsigned int interfaceIndex,
    int hopCount)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg = NULL;
    MaodvRreqSentNode *sentNode;
    MglextPacket *rreqPkt = NULL;
    BOOL isValidRt;

    MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValidRt);

    char *pktPtr;
    int pktSize = sizeof(MglextPacket);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);

    if (mrtToDest == NULL)
    {
        ERROR_Assert(FALSE, "Only a group leader with valid route can \n"
            "generate tree combine requests. \n");
    }

    rreqPkt = (MglextPacket *) pktPtr;
    MglextPacketSetType(&(rreqPkt->MglextPkt), MAODV_MGLEXT);
    MglextPacketSetJ(&(rreqPkt->MglextPkt), 1);
    MglextPacketSetR(&(rreqPkt->MglextPkt), 1);
    MglextPacketSetResv(&(rreqPkt->MglextPkt), 0);
    rreqPkt->floodingId = MaodvGetFloodingId(maodv);
    rreqPkt->destination.address = mcastAddr;
    rreqPkt->destination.seqNum  = mrtToDest->destination.seqNum;

    rreqPkt->source.seqNum  = ++maodv->seqNumber;
    rreqPkt->source.address = NetworkIpGetInterfaceAddress(
        node,
        interfaceIndex);

    rreqPkt->leader = leaderAddr;
    MglextPacketSetHopCount(&(rreqPkt->MglextPkt), 0);

    // Check if rreq has already been sent
    sentNode = MaodvCheckSent(mcastAddr, &maodv->sent);

    if (sentNode == NULL)
    {
        // add to the sent node
        sentNode = MaodvInsertSent(
            mcastAddr,
            hopCount,
            COMBINE,
            getSimTime(node),
            &maodv->sent);

        MaodvIncreaseTtl(maodv,sentNode);
    }

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        prevHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        prevHop);

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_CheckReplied,
        mcastAddr,
        (clocktype) (2 * hopCount * MAODV_NODE_TRAVERSAL_TIME));
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayRREQ
//
// PURPOSE:  Forward (re-broadcast) the RREQ
//
// ARGUMENTS: node, The node forwarding the Route Request
//            msg,  The Rreq packet
//            ttl,  time to leave of the message
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvRelayRREQ(
    Node *node,
    Message *msg,
    int ttl,
    int seqNum,
    MaodvNextHopNode *upstreamNode,
    int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg = NULL;
    RreqPacket *oldRreq;
    RreqPacket *newRreq;
    char *pktPtr;
    int pktSize = sizeof(RreqPacket);
    clocktype delay;
    int i;
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
    oldRreq = (RreqPacket *) MESSAGE_ReturnPacket(msg);
    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newRreq = (RreqPacket *) pktPtr;

    RreqPacketSetType(&(newRreq->RreqPkt),
        RreqPacketGetType(oldRreq->RreqPkt));
    RreqPacketSetJ(&(newRreq->RreqPkt), RreqPacketGetJ(oldRreq->RreqPkt));
    RreqPacketSetR(&(newRreq->RreqPkt), RreqPacketGetR(oldRreq->RreqPkt));
    RreqPacketSetG(&(newRreq->RreqPkt), RreqPacketGetG(oldRreq->RreqPkt));
    newRreq->floodingId = oldRreq->floodingId;
    newRreq->destination.address = oldRreq->destination.address;
    newRreq->destination.seqNum = seqNum;
    newRreq->source.address = oldRreq->source.address;
    newRreq->source.seqNum = oldRreq->source.seqNum;
    RreqPacketSetHopCount(&(newRreq->RreqPkt),
        (RreqPacketGetHopCount(oldRreq->RreqPkt) + 1));

    //the ttl should be decreased before relaying the packet
    ttl = ttl - IP_TTL_DEC;

    // Relay the Request to all interfaces except from which it received the
    // request. In case of single interface (mainly in wireless scenarios) the
    // node will forward the request though.

    if (upstreamNode == NULL)
    {
        for (i = 0; i < node->numberInterfaces; i++)
        {
            IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];
            if ((intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
                && (((node->numberInterfaces == 1) || (i != interfaceIndex))))
            {
                delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) *
                    MAODV_BROADCAST_JITTER);

                if (DEBUG_ROUTE_DISCOVERY)
                {
                    char clockStr[MAX_STRING_LENGTH];
                    char ipSrcStr[25], srcStr[25], destStr[25];

                    TIME_PrintClockInSecond(getSimTime(node), clockStr);
                    printf("\nNode %u relaying RREQ at %s\n",
                        node->nodeId,
                        clockStr);

                    IO_ConvertIpAddressToString(
                        newRreq->source.address,
                        srcStr);

                    IO_ConvertIpAddressToString(
                        newRreq->destination.address,
                        destStr);

                    IO_ConvertIpAddressToString(
                        intfInfo->ipAddress,
                        ipSrcStr);

                    printf("\tIP src %s, Request Source %s, Request Dest %s\n",
                        ipSrcStr, srcStr, destStr);
                }

                NetworkIpSendRawMessageToMacLayerWithDelay(
                    node,
                    MESSAGE_Duplicate(node, newMsg),
                    intfInfo->ipAddress,
                    ANY_DEST,
                    IPTOS_PREC_INTERNETCONTROL,
                    IPPROTO_MAODV,
                    ttl,
                    i,
                    ANY_DEST,
                    delay);
            }
        }
        MESSAGE_Free(node, newMsg);
        maodv->lastBroadcastSent = getSimTime(node);
    }
    else
    {
        // Forward to the upstream.
        NetworkIpSendRawMessageToMacLayer(
            node,
            newMsg,
            NetworkIpGetInterfaceAddress(node, upstreamNode->interface),
            upstreamNode->nextHopAddr,
            IPTOS_PREC_INTERNETCONTROL,
            IPPROTO_MAODV,
            ttl,
            upstreamNode->interface,
            upstreamNode->nextHopAddr);
    }
    if (RreqPacketGetJ(oldRreq->RreqPkt))
    {
        maodv->stats.numJoinRequestRelayed++;
    }
    else
    {
        maodv->stats.numNonJoinRequestRelayed++;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleRequest
//
// PURPOSE:  Processing procedure when RREQ is received
//
// ARGUMENTS: node, The node which has received the RREQ
//            msg,  The message containg the RREQ packet
//            srcAddr, previous hop
//            ttl,  The ttl of the message
//            interfaceIndex, The interface index through which the RREQ has
//                            been received.
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvHandleRequest(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    int ttl,
    int interfaceIndex,
    BOOL isUnicast)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    RreqPacket *rreqPkt = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    if (RreqPacketGetJ(rreqPkt->RreqPkt) == 1)
    {
        maodv->stats.numJoinReqRecd++;
    }
    else
    {
        maodv->stats.numNonJoinReqRecd++;
    }

    // check for duplicate
    if (!MaodvLookupSeenTable(
        rreqPkt->source.address,
        rreqPkt->floodingId,
        0,
        &maodv->seenTable))
    {
        clocktype revRtLifetime = MAODV_REV_ROUTE_LIFE;
        BOOL isValidSrc = FALSE;
        BOOL replyByIntermediate = FALSE;
        MaodvRouteEntry *rtToSrc = NULL;
        MaodvRouteEntry *rtEntryToDest = NULL;
        MaodvMrouteEntry *mrtEntryToDest = NULL;
        BOOL isValidRt = FALSE;
        int seqNum = rreqPkt->destination.seqNum;

        if (DEBUG_ROUTE_DISCOVERY)
        {
            printf("\tNot a duplicate RREQ so processing it\n");
        }

        // This is not a duplicate packet so process the request

        // Insert the source and the broadcast id in seen table to protect
        // processing duplicates
        MaodvInsertSeenTable(
            node,
            rreqPkt->source.address,
            rreqPkt->floodingId,
            0,
            &maodv->seenTable);

        // create reverse path entry in the route table for the destination
        // and also for the previous hop.

        // Check if there is a path for the source of the Request.
        rtToSrc = MaodvCheckRouteExist(
            node,
            rreqPkt->source.address,
            &maodv->routeTable,
            &isValidSrc);

        // Create the reverse path route entry if there is no route in the routing
        // table or if the route in the request is more recent.
        if (!isValidSrc || rtToSrc->destination.seqNum
            < rreqPkt->source.seqNum
            || ((rtToSrc->destination.seqNum == rreqPkt->source.seqNum)
                    && ((int) rtToSrc->hopCount >
                   (int)RreqPacketGetHopCount(rreqPkt->RreqPkt) + 1)))
        {
            rtToSrc = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                rreqPkt->source.address,
                rreqPkt->source.seqNum,
                (RreqPacketGetHopCount(rreqPkt->RreqPkt) + 1),
                srcAddr,
                getSimTime(node) + revRtLifetime,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (rtToSrc->lifetime == getSimTime(node) + revRtLifetime)
            {
                // Set timer for the new lifetime
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    rreqPkt->source.address,
                    (clocktype) revRtLifetime);
            }
        }

        // Update route to the previous hop if previous hop is not the source
        if (rreqPkt->source.address != srcAddr)
        {
            MaodvRouteEntry *ptrToPrevHop = NULL;

            ptrToPrevHop = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                srcAddr,
                0,
                1,
                srcAddr,
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            // Update lifetime ofthe previous hop address
            if (ptrToPrevHop->lifetime == getSimTime(node) +
                MAODV_MY_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    srcAddr,
                    (clocktype) MAODV_MY_ROUTE_TIMEOUT);
            }
        }

        // Check if an entry exists in the Multicast routing table for the dest
        mrtEntryToDest = MaodvCheckMrouteExist(
            node,
            rreqPkt->destination.address,
            &maodv->mrouteTable,
            &isValidRt);

        if (RreqPacketGetJ(rreqPkt->RreqPkt) == 1)
        {
            if (DEBUG_ROUTE_DISCOVERY)
            {
                printf("\tProcessing join RREQ\n");
            }

            // JOIN request -- only active tree member can reply
            if (mrtEntryToDest != NULL && isValidRt)
            {
                // A valid route exists for the multicast group. An entry in
                // the Multicast routing table means either the node is a
                // group member or memeber of the active tree for the group.
                MaodvNextHopNode *downstreamNode = NULL;
                MaodvNextHopNode *upstreamNode = NULL;

                // Check whether the previous hop is listed in the next hop
                // list of the route.
                downstreamNode = MaodvCheckNextHopsList(
                    srcAddr,
                    &mrtEntryToDest->nextHops);

                if (DEBUG_ROUTE_DISCOVERY)
                {
                    printf("\tHas a valid route in Mroute table\n");
                }

                if (downstreamNode != NULL)
                {
                    if (downstreamNode->direction == UPSTREAM_DIRECTION)
                    {
                        // The node from which the RREQ has been received is
                        // listed as a UPSTREAM_DIRECTION node so discard the request

                        if (DEBUG_ROUTE_DISCOVERY)
                        {
                            printf("\tIgnore as from UPSTREAM_DIRECTION\n");
                        }

                        return;
                    }
                }

                if (mrtEntryToDest->isMember)
                {
                    // If the sequence is greater than or equal to the sequence
                    // in the RREQ or the node is leader of the group

                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tIs a member of the tree\n");
                    }

                    if (mrtEntryToDest->destination.seqNum >=
                        rreqPkt->destination.seqNum
                        || MaodvLookupLeadership(
                               rreqPkt->destination.address,
                               &maodv->memberFlag))
                    {

                        if (DEBUG_ROUTE_DISCOVERY)
                        {
                            printf("\tSeq is up to date or grp leader\n");
                            printf("\tInitiating MGIRREP\n");
                        }

                        downstreamNode = MaodvReplaceInsertNextHopsList(
                            node,
                            rreqPkt->destination.address,
                            srcAddr,
                            DOWNSTREAM_DIRECTION,
                            interfaceIndex,
                            FALSE,
                            &mrtEntryToDest->nextHops);

                        // In case of intermediate nodes destination seq number
                        // in the multicast table is always greater. But if the
                        // group leaders sequence number is smaller it need to
                        // update its own sequence number.

                        mrtEntryToDest->destination.seqNum =
                            MAX(
                                mrtEntryToDest->destination.seqNum,
                                rreqPkt->destination.seqNum);

                        MaodvInitiateMGIRREP(
                            node,
                            maodv,
                            msg,
                            interfaceIndex,
                            srcAddr,
                            mrtEntryToDest);

                        return;
                    }
                    else
                    {
                        if (DEBUG_ROUTE_DISCOVERY)
                        {
                            printf("\tSequence number is not up to date\n");
                            printf("\tSeq Num in Mroute %u, seq in RREQ %u\n",
                                mrtEntryToDest->destination.seqNum,
                                rreqPkt->destination.seqNum);
                        }

                        //ttl comparision should be done with 1
                        if (ttl <= 1)
                        {
                            // TTL expired so can't relay the packet.
                            maodv->stats.numRequestTtlExpired++;

                            if (DEBUG_ROUTE_DISCOVERY)
                            {
                                printf("\tTTL expired so dropping RREQ\n");
                            }

                            return;
                        }

                        upstreamNode = MaodvFindUpstreamNextHop(
                            &mrtEntryToDest->nextHops);

                        if (upstreamNode != NULL)
                        {
                            if (upstreamNode == downstreamNode)
                            {
                                ERROR_Assert(FALSE, "Request from UPSTREAM_DIRECTION not "
                                    "possible \n");
                            }
                        }

                        downstreamNode = MaodvReplaceInsertNextHopsList(
                            node,
                            rreqPkt->destination.address,
                            srcAddr,
                            DOWNSTREAM_DIRECTION,
                            interfaceIndex,
                            FALSE,
                            &mrtEntryToDest->nextHops);

                        seqNum =
                            MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);

                        if (DEBUG_ROUTE_DISCOVERY)
                        {
                            printf("\tRelaying RREQ\n");
                        }

                        MaodvRelayRREQ(node, msg, ttl, seqNum, upstreamNode,
                            interfaceIndex);
                        return;
                    }
                }
                else
                {
                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tNot a member\n");
                    }

                    // create downstream entry and forward
                    //ttl comparision should be done with 1
                    if (ttl <= 1)
                    {
                        // Relay the packet only if TTL is not one
                        if (DEBUG_ROUTE_DISCOVERY)
                        {
                            printf("\tTTL expired so drop the packet\n");
                        }
                        maodv->stats.numRequestTtlExpired++;
                        return;
                    }

                    if (isUnicast && downstreamNode != NULL)
                    {
                        if (!downstreamNode->isEnabled)
                        {
                            downstreamNode->lifetime =
                                getSimTime(node) + MAODV_MTREE_BUILD;

                            MaodvSetMulticastTimer(
                                node,
                                MSG_ROUTING_MaodvCheckNextHopTimeout,
                                rreqPkt->destination.address,
                                downstreamNode->nextHopAddr,
                                (clocktype) MAODV_MTREE_BUILD);
                        }
                    }
                    else
                    {
                        downstreamNode = MaodvReplaceInsertNextHopsList(
                            node,
                            rreqPkt->destination.address,
                            srcAddr,
                            DOWNSTREAM_DIRECTION,
                            interfaceIndex,
                            FALSE,
                            &mrtEntryToDest->nextHops);
                    }

                    // forward the request
                    seqNum = MAX( seqNum, (int) mrtEntryToDest->destination.seqNum);

                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tRelaying the request\n");
                    }

                    MaodvRelayRREQ(
                        node,
                        msg,
                        ttl,
                        seqNum,
                        NULL,
                        interfaceIndex);
                    return;
                }
            }// if validRt
            else
            {
                // No route exists for the group so the node is not a group
                // member or a tree member.
                int hopCountLeader = -1;

                MaodvNextHopNode *downstreamNode = NULL;
                MaodvNextHopNode *upstreamNode;

                if (DEBUG_ROUTE_DISCOVERY)
                {
                    printf("\tNot a group member or tree member\n");
                }
                //ttl comparision should be done with 1
                if (ttl <= 1)
                {
                    // Relay the packet only if TTL is not one
                    maodv->stats.numRequestTtlExpired++;

                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tTTL expired so drop the packet\n");
                    }
                    return;
                }
                // JOIN request, hence create downstream nexthop
                if (mrtEntryToDest == NULL)
                {
                    mrtEntryToDest = MaodvReplaceInsertMrouteTable(
                        node,
                        maodv,
                        rreqPkt->destination.address,
                        rreqPkt->destination.seqNum,
                        MAODV_UNKNOWN_LEADER,
                        -1,
                        hopCountLeader,
                        getSimTime(node) + revRtLifetime,
                        srcAddr,
                        DOWNSTREAM_DIRECTION,
                        interfaceIndex,
                        FALSE,
                        FALSE,
                        &maodv->mrouteTable);

                    downstreamNode = MaodvCheckNextHopsList(
                        srcAddr,
                        &mrtEntryToDest->nextHops);
                }
                else
                {
                    if (downstreamNode != NULL)
                    {
                        if (downstreamNode->isEnabled == TRUE)
                        {
                            ERROR_Assert(FALSE,"trying to add ENABLED NODE \n");
                        }
                        if (downstreamNode->direction == UPSTREAM_DIRECTION)
                        {
                            return;
                        }
                    }

                    upstreamNode = MaodvFindUpstreamNextHop(
                        &mrtEntryToDest->nextHops);

                    if (upstreamNode != NULL)
                    {
                        if (upstreamNode->nextHopAddr == srcAddr)
                        {
                            return;
                        }
                    }

                    downstreamNode = MaodvReplaceInsertNextHopsList(
                        node,
                        rreqPkt->destination.address,
                        srcAddr,
                        DOWNSTREAM_DIRECTION,
                        interfaceIndex,
                        FALSE,
                        &mrtEntryToDest->nextHops);

                    seqNum = MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);
                }

                MaodvRelayRREQ(node, msg, ttl, seqNum, NULL, interfaceIndex);
                return;
            }
        } // intermediate node
        else
        {
            // NON-JOIN Request
            // The node is not the multicast tree or group member and the
            // request is non-join
            // check if it has an active route to the destination

            BOOL isValidDest = FALSE;
            int destSeq = rreqPkt->destination.seqNum;

            if (DEBUG_ROUTE_DISCOVERY)
            {
                printf("\tProcessing non-join RREQ\n");
            }

            // Check if a route exists in the multicast table.
            if (mrtEntryToDest != NULL && isValidRt)
            {
                if (mrtEntryToDest->destination.seqNum >=
                    rreqPkt->destination.seqNum
                    || MaodvLookupLeadership(
                           rreqPkt->destination.address,
                           &maodv->memberFlag))
                {
                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tSending reply as tree member for "
                            "non-join RREQ\n");
                    }

                    // generate rrep
                    MaodvInitiateRREP(
                        node,
                        maodv,
                        msg,
                        interfaceIndex,
                        srcAddr,
                        mrtEntryToDest);

                    return;
                }
            }

            // No route in the multicast routing table so check the unicast
            // table if any unicast path exists to a group member.
            rtEntryToDest = MaodvCheckRouteExist(
                node,
                rreqPkt->destination.address,
                &maodv->routeTable,
                &isValidDest);

            if (rtEntryToDest != NULL && isValidDest)
            {
                if (rtEntryToDest->destination.seqNum >=
                    rreqPkt->destination.seqNum)
                {
                    replyByIntermediate = TRUE;

                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("\tIntermediate node Sending reply for "
                            "non-join request\n");
                    }

                    // Send a Route Reply
                    MaodvInitiateRREPbyIN(
                        node,
                        maodv,
                        msg,
                        srcAddr,
                        interfaceIndex,
                        rtEntryToDest);

                    return;
                }
            }

            // Can't send a reply so relay the request
            if (rtEntryToDest)
            {
                destSeq = MAX((int) rtEntryToDest->destination.seqNum, destSeq);
            }

            //ttl comparision should be done with 1
            if (ttl > 1)
            {
                if (DEBUG_ROUTE_DISCOVERY)
                {
                    printf("\tRelaying Non-join RREQ\n");
                }

                MaodvRelayRREQ(
                    node,
                    msg,
                    ttl,
                    destSeq,
                    NULL,
                    interfaceIndex);
            }
            else
            {
                // TTL expired

                if (DEBUG_ROUTE_DISCOVERY)
                {
                    printf("\tTTL expired so dropping RREQ\n");
                }
                maodv->stats.numRequestTtlExpired++;
            }
        }
    }
    else
    {
        // If this RREQ already has been processed then silently discard the
        // message
        maodv->stats.numRequestDuplicate++;

        if (DEBUG_ROUTE_DISCOVERY)
        {
            printf("\tDuplicate RREQ so discarding it\n");
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayMGREXT
//
// PURPOSE:  Relay to multicast group extension
//
// ARGUMENTS: node, The node which has received the packet
//            msg,  The message containg the packet
//            ttl,  The ttl of the message
//            seqNum, Sequence number
//            upstreamNode, Upstrem node
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvRelayMGREXT(
    Node *node,
    Message *msg,
    int ttl,
    int seqNum,
    MaodvNextHopNode *upstreamNode)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg = NULL;
    MgrextPacket *oldMgrext, *newMgrext;
    char *pktPtr;
    int pktSize = sizeof(MgrextPacket), i;
    clocktype delay;
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;

    oldMgrext = (MgrextPacket *) MESSAGE_ReturnPacket(msg);

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newMgrext = (MgrextPacket *) pktPtr;

    MgrextPacketSetType(&(newMgrext->MgrextPkt),
        MgrextPacketGetType(oldMgrext->MgrextPkt));
    MgrextPacketSetJ(&(newMgrext->MgrextPkt),
        MgrextPacketGetJ(oldMgrext->MgrextPkt));
    MgrextPacketSetR(&(newMgrext->MgrextPkt),
        MgrextPacketGetR(oldMgrext->MgrextPkt));
    MgrextPacketSetG(&(newMgrext->MgrextPkt),
        MgrextPacketGetG(oldMgrext->MgrextPkt));
    newMgrext->floodingId = oldMgrext->floodingId;
    newMgrext->destination.address = oldMgrext->destination.address;
    newMgrext->destination.seqNum = seqNum;
    newMgrext->source.address = oldMgrext->source.address;
    newMgrext->source.seqNum = oldMgrext->source.seqNum;
    MgrextPacketSetHopCount(&(newMgrext->MgrextPkt),
        (MgrextPacketGetHopCount(oldMgrext->MgrextPkt) + 1));
    newMgrext->groupHopCount = oldMgrext->groupHopCount;
    newMgrext->leader = oldMgrext->leader;

    //ttl should be decreased before relaying the packet
    ttl = ttl - IP_TTL_DEC;
    if (upstreamNode == NULL)
    {
        if ((int)MgrextPacketGetHopCount(newMgrext->MgrextPkt) >
            maodv->netDiameter)
        {
            ERROR_ReportError("MAODV: Please increase MAODV_NET_DIAMETER.\n");
        }

        for (i = 0; i < node->numberInterfaces; i++)
        {
            IpInterfaceInfoType *intfInfo = ip->interfaceInfo[i];

            if (intfInfo->multicastProtocolType == MULTICAST_PROTOCOL_MAODV)
            {
                delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);
                NetworkIpSendRawMessageToMacLayerWithDelay(
                    node,
                    MESSAGE_Duplicate(node, newMsg),
                    intfInfo->ipAddress,
                    ANY_DEST,
                    IPTOS_PREC_INTERNETCONTROL,
                    IPPROTO_MAODV,
                    ttl,
                    i,
                    ANY_DEST,
                    delay);
            }
        }
        maodv->lastBroadcastSent = getSimTime(node);
    }

    else
    {
        NetworkIpSendRawMessageToMacLayer(
            node,
            MESSAGE_Duplicate(node, newMsg),
            NetworkIpGetInterfaceAddress(node, upstreamNode->interface),
            upstreamNode->nextHopAddr,
            IPTOS_PREC_INTERNETCONTROL,
            IPPROTO_MAODV,
            ttl,
            upstreamNode->interface,
            upstreamNode->nextHopAddr);
    }

    if (MgrextPacketGetJ(oldMgrext->MgrextPkt))
    {
        maodv->stats.numJoinRequestRelayed++;
    }
    else
    {
        maodv->stats.numNonJoinRequestRelayed++;
    }
    MESSAGE_Free(node, newMsg);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleMGREXT
//
// PURPOSE:  Process to multicast group extension
//
// ARGUMENTS: node, The node which has received the packet
//            msg,  The message containg the packet
//            srcAddr, Source address
//            ttl,  The ttl of the message
//            interfaceIndex, Interface index
//            isUnicast, unicast type flag
//
// RETURN:    None
//---------------------------------------------------------------------------
void MaodvHandleMGREXT(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    int ttl,
    int interfaceIndex,
    BOOL isUnicast)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MgrextPacket *rreqPkt = (MgrextPacket *) MESSAGE_ReturnPacket(msg);
    maodv->stats.numRepairReqRecd++;

    // check for duplicate
    if (!MaodvLookupSeenTable(
        rreqPkt->source.address,
        rreqPkt->floodingId,
        0,
        &maodv->seenTable))
    {
        clocktype revRtLifetime = MAODV_ACTIVE_ROUTE_TIMEOUT;
        BOOL isValidSrc = FALSE;
        MaodvRouteEntry *rtToSrc = NULL;
        MaodvMrouteEntry *mrtEntryToDest = NULL;
        BOOL isValidRt = FALSE;
        NodeAddress leader;
        MaodvRequestEntry *reqEntry = NULL;
        int seqNum = rreqPkt->destination.seqNum;

        revRtLifetime = MAODV_ACTIVE_ROUTE_TIMEOUT;
        // This is not a duplicate packet so process the request
        // Insert the source and the broadcast id in seen table to protect
        // processing duplicates

        MaodvInsertSeenTable(
            node,
            rreqPkt->source.address,
            rreqPkt->floodingId,
            0,
            &maodv->seenTable);

        // create reverse path entry in the route table for the destination
        // and also for the previous hop.
        rtToSrc = MaodvCheckRouteExist(
            node,
            rreqPkt->source.address,
            &maodv->routeTable,
            &isValidSrc);

        if (!isValidSrc
            || rtToSrc->destination.seqNum < rreqPkt->source.seqNum
            || ((rtToSrc->destination.seqNum == rreqPkt->source.seqNum)
                && ((int) rtToSrc->hopCount >
                (int)(MgrextPacketGetHopCount(rreqPkt->MgrextPkt) + 1))))
        {
            rtToSrc = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                rreqPkt->source.address,
                rreqPkt->source.seqNum,
                (MgrextPacketGetHopCount(rreqPkt->MgrextPkt) + 1),
                srcAddr,
                getSimTime(node) + revRtLifetime,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (rtToSrc->lifetime == getSimTime(node) +  revRtLifetime)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    rreqPkt->source.address,
                    (clocktype) revRtLifetime);
            }
        }

        if (rreqPkt->source.address != srcAddr)
        {
            MaodvRouteEntry *ptrToPrevHop;

            ptrToPrevHop = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                srcAddr,
                0,
                1,
                srcAddr,
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (ptrToPrevHop->lifetime ==
                getSimTime(node) +  MAODV_MY_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    srcAddr,
                    (clocktype) MAODV_MY_ROUTE_TIMEOUT);
            }
        }

        mrtEntryToDest = MaodvCheckMrouteExist(
            node,
            rreqPkt->destination.address,
            &maodv->mrouteTable,
            &isValidRt);

        if (mrtEntryToDest != NULL && isValidRt)
        {
            // the node is a tree member
            // can generate RREP if valid
            if (mrtEntryToDest->isMember)
            {
                // active tree member
                MaodvNextHopNode *upstreamNode =  MaodvFindUpstreamNextHop(
                    &mrtEntryToDest->nextHops);

                if (!MaodvLookupLeadership(
                    rreqPkt->destination.address,
                    &maodv->memberFlag))
                {
                    ERROR_Assert(upstreamNode != NULL,
                        "\n active member with no upstream \n");

                    if (upstreamNode->nextHopAddr == srcAddr)
                    {
                        return;
                    }
                }
                // active member of the tree..
                if ((mrtEntryToDest->destination.seqNum >=
                        rreqPkt->destination.seqNum && mrtEntryToDest->hopCountLeader
                    <= rreqPkt->groupHopCount)
                    || MaodvLookupLeadership(
                           rreqPkt->destination.address,
                           &maodv->memberFlag))
                {
                    MaodvNextHopNode *downstreamNode;
                    downstreamNode = MaodvReplaceInsertNextHopsList(
                        node,
                        rreqPkt->destination.address,
                        srcAddr,DOWNSTREAM_DIRECTION,
                        interfaceIndex,
                        FALSE,
                        &mrtEntryToDest->nextHops);

                    mrtEntryToDest->destination.seqNum =
                        MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);

                    MaodvInitiateMGIRREP(
                        node,
                        maodv,
                        msg,
                        interfaceIndex,
                        srcAddr,
                        mrtEntryToDest);
                    return;
                }
                else
                {
                    MaodvNextHopNode *downstreamNode =
                        MaodvCheckNextHopsList(
                            srcAddr,
                            &mrtEntryToDest->nextHops);

                    //ttl comparision should be done with 1
                    if (ttl <= 1)
                    {
                        // Relay the packet only if TTL is not one
                        maodv->stats.numRequestTtlExpired++;
                        return;
                    }

                    if (downstreamNode != NULL)
                    {
                        if (!downstreamNode->isEnabled)
                        {
                            downstreamNode->lifetime =
                                getSimTime(node) + MAODV_MTREE_BUILD;

                            MaodvSetMulticastTimer(
                                node,
                                MSG_ROUTING_MaodvCheckNextHopTimeout,
                                mrtEntryToDest->destination.address,
                                srcAddr,
                                (clocktype) MAODV_MTREE_BUILD);
                        }
                    }
                    else
                    {
                        downstreamNode =
                            MaodvReplaceInsertNextHopsList(
                                node,
                                rreqPkt->destination.address,
                                srcAddr,
                                DOWNSTREAM_DIRECTION,
                                interfaceIndex,
                                FALSE,
                                &mrtEntryToDest->nextHops);
                    }

                    // forward the request
                    seqNum = MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);

                    MaodvRelayMGREXT(node,  msg, ttl, seqNum, upstreamNode);
                    return;
                }
            }
            else
            {
                // not an active member of tree -- i.e.
                // trying to repair its upstream
                // cannot reply, just ignore
                // create downstream entry and forward
                MaodvNextHopNode *downstreamNode =
                    MaodvCheckNextHopsList(
                        srcAddr,
                        &mrtEntryToDest->nextHops);
                //ttl comparision should be done with 1
                if (ttl <= 1)
                {
                    // Relay the packet only if TTL is not one
                    maodv->stats.numRequestTtlExpired++;
                    return;
                }

                if (isUnicast && downstreamNode != NULL)
                {
                }
                else
                {
                    downstreamNode =
                        MaodvReplaceInsertNextHopsList(
                            node,
                            rreqPkt->destination.address,
                            srcAddr,DOWNSTREAM_DIRECTION,
                            interfaceIndex,
                            FALSE,
                            &mrtEntryToDest->nextHops);
                }

                // forward the request
                seqNum = MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);
                MaodvRelayMGREXT(node,  msg, ttl, seqNum, NULL);
                return;
            }
        }  // isValid
        else
        {
            // not a member previously
            int hopCountLeader = -1;
            //ttl comparision should be done with 1
            if (ttl <= 1)
            {
                // Relay the packet only if TTL is not one
                maodv->stats.numRequestTtlExpired++;
                return;
            }

            // cannot generate RREP
            // relay the repair request
            reqEntry = MaodvCheckRequestTable(
                rreqPkt->destination.address,
                &maodv->requestTable);

            if (reqEntry != NULL)
            {
                leader = reqEntry->leaderAddr;
            }
            else
            {
                leader = rreqPkt->leader;
                reqEntry = MaodvReplaceInsertRequestTable(
                    rreqPkt->destination.address,
                    leader,
                    &maodv->requestTable);
                hopCountLeader = -1;
            }
            //  create downstream nexthop
            if (mrtEntryToDest == NULL)
            {
                mrtEntryToDest = MaodvReplaceInsertMrouteTable(
                    node,
                    maodv,
                    rreqPkt->destination.address,
                    rreqPkt->destination.seqNum,
                    leader,
                    MgrextPacketGetHopCount((rreqPkt->MgrextPkt) + 1),
                    hopCountLeader,
                    getSimTime(node) + revRtLifetime,
                    srcAddr,
                    DOWNSTREAM_DIRECTION,
                    interfaceIndex,
                    FALSE,
                    FALSE,
                    &maodv->mrouteTable);
            }
            else
            {
                MaodvNextHopNode *downstreamNode, *upstreamNode;
                upstreamNode = MaodvFindUpstreamNextHop(
                    &mrtEntryToDest->nextHops);

                if (upstreamNode != NULL)
                {
                    if (upstreamNode->nextHopAddr == srcAddr)
                    {
                        return;
                    }
                }
                downstreamNode = MaodvReplaceInsertNextHopsList(
                    node,
                    rreqPkt->destination.address,
                    srcAddr,
                    DOWNSTREAM_DIRECTION,
                    interfaceIndex,
                    FALSE,
                    &mrtEntryToDest->nextHops);

                seqNum = MAX(seqNum, (int) mrtEntryToDest->destination.seqNum);
                mrtEntryToDest->lifetime =
                    MAX(
                        mrtEntryToDest->lifetime,
                        getSimTime(node) + revRtLifetime);

                mrtEntryToDest->destination.seqNum = seqNum;
            }
            MaodvRelayMGREXT(node,  msg, ttl, seqNum, NULL);
        }
    }
    else
    {
        // already processed RREQ
        // silently discard it
        return;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvSendMact
//
// PURPOSE:  Sends multicast packets
//
// ARGUMENTS: node, The node which sends the packet
//            msg,  The message containg the packet
//            mcastAddr, Multicast address
//            destAddr, Destination address
//            interface, Interface index
//            type, multicast packet type flag
//            hopCount, Hop count flag
//            seqNum, Sequence Number
//            leaderAddr, group leader address
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvSendMact(
    Node *node,
    NodeAddress mcastAddr,
    NodeAddress destAddr,
    unsigned int interface,
    MactType type,
    unsigned int hopCount,
    unsigned int seqNum,
    NodeAddress leaderAddr)
{
    MaodvData* maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    Message *newMsg;
    MactPacket *mactPkt;

    char *pktPtr;
    int pktSize = sizeof(MactPacket), ttl = 1;

    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);

    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    mactPkt = (MactPacket *) pktPtr;

    MactPacketSetType(&(mactPkt->MactPkt), MAODV_MACT);
    switch (type)
    {
    case J:
        {
            MactPacketSetJ(&(mactPkt->MactPkt), 1);
            MactPacketSetP(&(mactPkt->MactPkt), 0);
            MactPacketSetU(&(mactPkt->MactPkt), 0);
            MactPacketSetG(&(mactPkt->MactPkt), 0);
            MactPacketSetR(&(mactPkt->MactPkt), 0);
            break;
        }
    case P:
        {
            MactPacketSetJ(&(mactPkt->MactPkt), 0);
            MactPacketSetP(&(mactPkt->MactPkt), 1);
            MactPacketSetU(&(mactPkt->MactPkt), 0);
            MactPacketSetG(&(mactPkt->MactPkt), 0);
            MactPacketSetR(&(mactPkt->MactPkt), 0);
            break;
        }
    case U:
        {
            MactPacketSetJ(&(mactPkt->MactPkt), 0);
            MactPacketSetP(&(mactPkt->MactPkt), 0);
            MactPacketSetU(&(mactPkt->MactPkt), 1);
            MactPacketSetG(&(mactPkt->MactPkt), 0);
            MactPacketSetR(&(mactPkt->MactPkt), 0);
            break;
        }
    case G:
        {
            MactPacketSetJ(&(mactPkt->MactPkt), 0);
            MactPacketSetP(&(mactPkt->MactPkt), 0);
            MactPacketSetU(&(mactPkt->MactPkt), 0);
            MactPacketSetG(&(mactPkt->MactPkt), 1);
            MactPacketSetR(&(mactPkt->MactPkt), 0);
            break;
        }
    case R:
        {
            MactPacketSetJ(&(mactPkt->MactPkt), 0);
            MactPacketSetP(&(mactPkt->MactPkt), 0);
            MactPacketSetU(&(mactPkt->MactPkt), 0);
            MactPacketSetG(&(mactPkt->MactPkt), 0);
            MactPacketSetR(&(mactPkt->MactPkt), 1);
            break;
        }
    default:
        {
            ERROR_Assert(FALSE, "Unknown MACT type......\n");
        }
    }

    MactPacketSetHopCount(&(mactPkt->MactPkt), hopCount);
    mactPkt->srcAddr = NetworkIpGetInterfaceAddress(node, interface);
    mactPkt->mcastDest.address = mcastAddr;
    mactPkt->mcastDest.seqNum = seqNum;
    mactPkt->leaderAddr = leaderAddr;

    if (DEBUG_ROUTE_DISCOVERY)
    {
        char mcastAddrStr[25];
        char destStr[25];
        char time[100];

        TIME_PrintClockInSecond(getSimTime(node), time);

        IO_ConvertIpAddressToString(mcastAddr, mcastAddrStr);
        IO_ConvertIpAddressToString(destAddr, destStr);

        printf("\nMaodvSendMact: Node %u, time %s\n",
            node->nodeId,
            time);
        printf("\tSending MACT: dest %s, Multicast address %s\n",
            destStr,
            mcastAddrStr);

        IO_ConvertIpAddressToString(mactPkt->srcAddr, mcastAddrStr);
        IO_ConvertIpAddressToString(mactPkt->leaderAddr, destStr);

        printf("\tMACT leader %s, Reply Dest Seq %u, MACT Src %s\n",
            destStr, mactPkt->mcastDest.seqNum, mcastAddrStr);

    }

    if (destAddr == ANY_DEST)
    {
        for (int i = 0; i < node->numberInterfaces; i++)
        {
            Message *mact = MESSAGE_Duplicate(node, newMsg);
            clocktype delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) *
                MAODV_BROADCAST_JITTER);

            pktPtr = MESSAGE_ReturnPacket(mact);
            mactPkt = (MactPacket *) pktPtr;

            mactPkt->srcAddr = NetworkIpGetInterfaceAddress(node, i);

            NetworkIpSendRawMessageToMacLayerWithDelay(
                node,
                mact,
                NetworkIpGetInterfaceAddress(node,i),
                destAddr,
                IPTOS_PREC_INTERNETCONTROL,
                IPPROTO_MAODV,
                ttl,
                i,
                destAddr,
                delay);
        }
    }
    else
    {
        NetworkIpSendRawMessageToMacLayer(
            node,
            MESSAGE_Duplicate(node, newMsg),
            NetworkIpGetInterfaceAddress(node,interface),
            destAddr,
            IPTOS_PREC_INTERNETCONTROL,
            IPPROTO_MAODV,
            ttl,
            interface,
            destAddr);
    }

    maodv->stats.numMactSent++;
    MESSAGE_Free(node, newMsg);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleMACT
//
// PURPOSE:  Handles multicast packets
//
// ARGUMENTS: node, The node which has received the packet
//            msg,  The message containg the packet
//            srcAddr, Source address
//            interfaceIndex, Interface index
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvHandleMACT(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    unsigned int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MactPacket *mactPkt = (MactPacket *) MESSAGE_ReturnPacket(msg);
    NodeAddress mcastAddr = mactPkt->mcastDest.address;
    MaodvMrouteEntry *mrtToDest = NULL;
    MaodvNextHopNode *nextHop = NULL, *upstreamNode = NULL;
    BOOL isValid;

    // find the entry
    mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValid);

    if (mrtToDest == NULL)
    {
        // This node should not get this MACT
        return;
    }
    nextHop = MaodvCheckNextHopsList(mactPkt->srcAddr, &mrtToDest->nextHops);

    if (nextHop == NULL)
    {
        // Next hop list should not be empty
        return;
    }

    if (MactPacketGetJ(mactPkt->MactPkt) == 1)
    {
        // JOIN flag set ==> graft the tree
        if (nextHop->isEnabled == TRUE)
        {
            // MACT received from a tree member joining the group
            return;
        }
        // THE NODE IS A TREE MEMBER
        if (isValid)
        {
            // make DOWNSTREAM_DIRECTION valid and set the timer
            nextHop->isEnabled = TRUE;
            nextHop->lifetime = getSimTime(node) + MAODV_HELLO_LIFE;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvTreeUtilizationTimer,
                mcastAddr,
                mactPkt->srcAddr,
                (clocktype) MAODV_HELLO_LIFE);

            mrtToDest->lifetime =
                MAX(mrtToDest->lifetime,
                    getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT);

            return;
        }
        else
        {
            // NOT A TREE MEMBER
            upstreamNode = MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

            if (upstreamNode == NULL)
            {
                return;
            }

            // make DOWNSTREAM_DIRECTION valid
            nextHop->isEnabled = TRUE;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvTreeUtilizationTimer,
                mcastAddr,
                mactPkt->srcAddr,
                (clocktype) MAODV_HELLO_LIFE);

            // make UPSTREAM_DIRECTION valid
            upstreamNode->isEnabled = TRUE;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvTreeUtilizationTimer,
                mcastAddr,
                upstreamNode->nextHopAddr,
                (clocktype) MAODV_HELLO_LIFE);

            upstreamNode->lifetime = getSimTime(node) +
                MAODV_MDATA_FORWARD_TIMEOUT;

            MaodvSetMulticastTimer(
                node,
                MSG_ROUTING_MaodvCheckMroute,
                mcastAddr,
                upstreamNode->nextHopAddr,
                (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);

            // make the route valid AND JOIN THE NODE TO THE TREE
            mrtToDest->isValid = TRUE;
            mrtToDest->isMember = TRUE;

            // propagate the JOIN message
            MaodvSendMact(
                node,
                mcastAddr,
                upstreamNode->nextHopAddr,
                upstreamNode->interface,
                J,
                0,
                mrtToDest->destination.seqNum,
                0);

            // node added to the tree
            mrtToDest->lifetime =
                MAX(
                    mrtToDest->lifetime,
                    getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT);

            return;
        }
    }
    // PRUNE MACT - PROCESSING
    else if (MactPacketGetP(mactPkt->MactPkt) == 1)
    {
        BOOL lastEntry = FALSE;
        MAODV_M_Node *member = NULL;

        if (!isValid)
        {
            return;
        }

        // PRUNE FROM UPSTREAM_DIRECTION
        if (nextHop->direction == UPSTREAM_DIRECTION)
        {
            if (!nextHop->isEnabled)
            {
                MaodvDeleteNextHop(
                    mactPkt->srcAddr,
                    &mrtToDest->nextHops,
                    &lastEntry);

                return;
            }

            MaodvDeleteNextHop(
                mactPkt->srcAddr,
                &mrtToDest->nextHops,
                &lastEntry);

            // If the node is a member then make it the group leader
            if (MaodvLookupGroupMembership(
                mactPkt->mcastDest.address,
                &maodv->memberFlag))
            {
                member = MaodvSetMembershipType(
                    mactPkt->mcastDest.address,
                    LEADER,
                    &maodv->memberFlag);

                member->isUpdate = TRUE;
                mrtToDest->mcastLeader = NetworkIpGetInterfaceAddress(node, 0);
                mrtToDest->hopCountLeader = 0;
                mrtToDest->destination.seqNum =
                    MAX(
                        mrtToDest->destination.seqNum,
                        mactPkt->mcastDest.seqNum);

                // SEND MACT UPDATE
                MaodvSendMact(
                    node,
                    mactPkt->mcastDest.address,
                    ANY_DEST,
                    DEFAULT_INTERFACE,
                    U,
                    mrtToDest->hopCountLeader,
                    mrtToDest->destination.seqNum,
                    mrtToDest->mcastLeader);

                // start sending group hello
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_ROUTING_MaodvSendGroupHello,
                    mcastAddr,
                    0);
            }
            else
            {
                // Node is not a group member. Propagate leadership
                MaodvNextHopNode *selectedNextHop = NULL;

                for (selectedNextHop = mrtToDest->nextHops.head;
                    selectedNextHop != NULL;
                    selectedNextHop = selectedNextHop->next)
                {
                    if (selectedNextHop->isEnabled
                        && selectedNextHop->direction == DOWNSTREAM_DIRECTION)
                    {
                        break;
                    }
                }

                if (selectedNextHop == NULL)
                {
                    MaodvDeleteMroute(
                        node,
                        mactPkt->mcastDest.address,
                        &maodv->mrouteTable);
                    return;
                }

                MaodvSendMact(
                    node,
                    mactPkt->mcastDest.address,
                    selectedNextHop->nextHopAddr,
                    selectedNextHop->interface,
                    G,
                    0,
                    mrtToDest->destination.seqNum,
                    0);

                ERROR_Assert(selectedNextHop->direction == DOWNSTREAM_DIRECTION,
                    "MACT with G flag PROPAGED THRO UPSTREAM_DIRECTION NODE \n");
                // set the direction of the nextHop to upstream
                selectedNextHop->direction = UPSTREAM_DIRECTION;
            }

            return;
        }
        MaodvDeleteNextHop(mactPkt->srcAddr,&mrtToDest->nextHops,&lastEntry);

        // Check if it can prune itself
        if (!MaodvLookupGroupMembership(
            mactPkt->mcastDest.address,
            &maodv->memberFlag))
        {
            // node is not a member of multicast group
            if (mrtToDest->nextHops.size == 1)
            {
                // leaf node
                if (mrtToDest->nextHops.head->isEnabled)
                {
                    MaodvSendMact(
                        node,
                        mactPkt->mcastDest.address,
                        mrtToDest->nextHops.head->nextHopAddr,
                        mrtToDest->nextHops.head->interface,
                        P,
                        0,
                        mrtToDest->destination.seqNum,
                        0);
                }

                MaodvDeleteMroute(
                    node,
                    mactPkt->mcastDest.address,
                    &maodv->mrouteTable);
            }
        }
        return;
    }

    // UPDATE MACT
    else if (MactPacketGetU(mactPkt->MactPkt) == 1)
    {
        // some upstream treemember has repaired a broken tree link and is
        // at new distance from the leader
        if (nextHop->direction != UPSTREAM_DIRECTION)
        {
            return;
        }

        mrtToDest->hopCountLeader =
            MactPacketGetHopCount(mactPkt->MactPkt) + 1;

        mrtToDest->destination.seqNum =
            MAX(
                mactPkt->mcastDest.seqNum,
                mrtToDest->destination.seqNum);

        mrtToDest->mcastLeader = mactPkt->leaderAddr;

        if (mrtToDest->nextHops.size > 1)
        {
            // propagate the update to all DOWNSTREAM_DIRECTION nodes
            MaodvSendMact(
                node,
                mactPkt->mcastDest.address,
                ANY_DEST,
                DEFAULT_INTERFACE,
                U,
                mrtToDest->hopCountLeader,
                mrtToDest->destination.seqNum,
                mrtToDest->mcastLeader);
        }
        return;
    }
    // GROUP LEADER TRANSFER MACT
    else if (MactPacketGetG(mactPkt->MactPkt) == 1)
    {
        MAODV_M_Node *member;
        // failed to repair the tree.. and asking to be the leader
        // change the direction  of the next hop from which mact received
        mrtToDest->destination.seqNum =
            MAX(
                mactPkt->mcastDest.seqNum,
                mrtToDest->destination.seqNum);

        // check if it is a group member
        if (MaodvLookupGroupMembership(
            mactPkt->mcastDest.address,
            &maodv->memberFlag))
        {
            // member of the group, hence declare as leader
            member = MaodvSetMembershipType(
                mactPkt->mcastDest.address,
                LEADER,
                &maodv->memberFlag);

            member->isUpdate = TRUE;
            mrtToDest->mcastLeader = NetworkIpGetInterfaceAddress(node, 0);
            mrtToDest->hopCountLeader = 0;
            // start sending group hello
            MaodvSetTimer(
                node,
                NULL,
                MSG_ROUTING_MaodvSendGroupHello,
                mcastAddr,
                0);
            nextHop->direction = DOWNSTREAM_DIRECTION;
        }
        else
        {
            MaodvNextHopNode *selectedNextHop = NULL;

            for (selectedNextHop = mrtToDest->nextHops.head;
                selectedNextHop != NULL;
                selectedNextHop = selectedNextHop->next)
            {
                if (selectedNextHop->isEnabled
                    && selectedNextHop->direction == DOWNSTREAM_DIRECTION)
                {
                    break;
                }
            }
            if (selectedNextHop == NULL)
            {
                MaodvDeleteSent(mactPkt->mcastDest.address, &maodv->sent);
                MaodvDeleteMroute(
                    node,
                    mactPkt->mcastDest.address,
                    &maodv->mrouteTable);

                return;
            }

            MaodvSendMact(
                node,
                mactPkt->mcastDest.address,
                selectedNextHop->nextHopAddr,
                selectedNextHop->interface,
                G,
                0,
                mrtToDest->destination.seqNum,
                0);

            // set the direction of the nextHop to upstream
            selectedNextHop->direction = UPSTREAM_DIRECTION;
            nextHop->direction = DOWNSTREAM_DIRECTION;
        }
        return;
    }
    else if (MactPacketGetR(mactPkt->MactPkt) == 1)
    {
        // REBOOT
        // add code here
        // NOT YET IMPLEMENTED
        //
        return;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleMglReply
//
// PURPOSE:  Handles multicast group leader reply
//
// ARGUMENTS: node, The node which has received the packet
//            msg,  The message containg the packet
//            srcAddr, Source address
//            interfaceIndex, Interface index
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvHandleMglReply(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    int interfaceIndex)
{
    MgiextPacket *rrepPkt = (MgiextPacket *) MESSAGE_ReturnPacket(msg);
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    clocktype lifetime;
    MaodvMrouteEntry *mrtToDest = NULL;

    BOOL isValidRt = FALSE;

    // clocktype must be copied to access the field of that type
    lifetime = (clocktype) (rrepPkt->lifetime * MILLI_SECOND);

    if (lifetime == 0)
    {
        return;
    }

    if (rrepPkt->destination.address != srcAddr
        && MaodvCheckRouteExist(
            node,
            srcAddr,
            &maodv->routeTable,
            &isValidRt)
        && !isValidRt)
    {
        MaodvRouteEntry *ptrToPrevHop = NULL;

        ptrToPrevHop = MaodvReplaceInsertRouteTable(
            node,
            maodv,
            srcAddr,
            0,
            1,
            srcAddr,
            getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
            TRUE,
            interfaceIndex,
            &maodv->routeTable);

        if (ptrToPrevHop->lifetime == getSimTime(node) +
            MAODV_MY_ROUTE_TIMEOUT)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_CheckRouteTimeout,
                srcAddr,
                (clocktype) MAODV_MY_ROUTE_TIMEOUT);
        }
    }
    mrtToDest = MaodvCheckMrouteExist(
        node,
        rrepPkt->destination.address,
        &maodv->mrouteTable,
        &isValidRt);

    if (NetworkIpIsMyIP(node, rrepPkt->sourceAddr))
    {
        // destination of mgl reply
        MaodvRreqSentNode *reqSent = NULL;
        MaodvNextHopNode *upstreamNode = NULL;
        MaodvNextHopNode *newNextHop = NULL;
        reqSent = MaodvCheckSent(rrepPkt->destination.address, &maodv->sent);

        if (reqSent == NULL)
        {
            if (mrtToDest->isMember && isValidRt)
            {
                return;
            }
            ERROR_Assert(FALSE, "MaodvHandleMglReply: No req entry and "
                "no valid route");
        }

        if (rrepPkt->destination.seqNum < mrtToDest->destination.seqNum)
        {
            return;
        }

        if (mrtToDest == NULL || !isValidRt)
        {
            return;
        }
        upstreamNode = MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

        if (upstreamNode != NULL)
        {
            BOOL notUsed;
            ERROR_Assert(!upstreamNode->isEnabled,
                "valid upstream entry found for mgl reply..\n");
            MaodvDeleteNextHop(
                upstreamNode->nextHopAddr,
                &mrtToDest->nextHops,
                &notUsed);
        }

        newNextHop = MaodvReplaceInsertNextHopsList(
            node,
            rrepPkt->destination.address,
            srcAddr,
            UPSTREAM_DIRECTION,
            interfaceIndex,
            TRUE,
            &mrtToDest->nextHops);

        reqSent->isReplied = TRUE;
        reqSent->type = COMBINE;

        mrtToDest->isMember = TRUE;
        mrtToDest->destination.seqNum = rrepPkt->destination.seqNum;
        mrtToDest->mcastLeader = rrepPkt->leader;
        mrtToDest->hopCount = MgiextPacketGetHopCount(rrepPkt->MgiextPkt)
            + 1;
        mrtToDest->hopCountLeader =
            MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;

        // send mact to inform its downstream about the leader update
        MaodvSendMact(
            node,
            rrepPkt->destination.address,
            ANY_DEST,
            DEFAULT_INTERFACE,
            U,
            mrtToDest->hopCountLeader,
            mrtToDest->destination.seqNum,
            mrtToDest->mcastLeader);

        // Release buffer and send the buffered data
        while (MaodvLookupBuffer(
            rrepPkt->destination.address,
            &maodv->msgBuffer))
        {
            NodeAddress previousHop;
            Message *newMsg = MaodvGetBufferedPacket(
                rrepPkt->destination.address,
                &previousHop,
                &maodv->msgBuffer);

            MaodvTransmitMdata(
                node,
                MESSAGE_Duplicate(node,msg),
                mrtToDest,
                previousHop,
                interfaceIndex);

            MaodvDeleteBuffer(
                mrtToDest->destination.address,
                &maodv->msgBuffer);

            MESSAGE_Free(node, newMsg);
        }
        return;
    }
    else if (isValidRt && mrtToDest != NULL)
    {
        if (mrtToDest->mcastLeader == rrepPkt->sourceAddr)
        {
            // node belongs to the lower partition
            // enable the downstream and pass thro upstream
            MaodvNextHopNode *upstreamNode = NULL;
            MaodvNextHopNode *newNextHop = NULL, *newNextHop1 = NULL;

            if (rrepPkt->destination.seqNum < mrtToDest->destination.seqNum)
            {
                return;
            }

            upstreamNode =   MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

            if (upstreamNode == NULL)
            {
                if (mrtToDest->isMember)
                {
                    ERROR_Assert(FALSE, "MaodvHandleMglReply: Already a"
                        "member!!");
                }
                else
                {
                    return;
                }
            }
            if (upstreamNode->nextHopAddr == srcAddr)
            {
                // this could occur due to inconsistence in leader info
                // just ignore the reply
                return;
            }
            mrtToDest->mcastLeader = rrepPkt->leader;
            mrtToDest->hopCountLeader =
                MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;

            newNextHop1 = MaodvReplaceInsertNextHopsList(
                node,
                rrepPkt->destination.address,
                upstreamNode->nextHopAddr,
                DOWNSTREAM_DIRECTION,
                upstreamNode->interface,
                TRUE,
                &mrtToDest->nextHops);

            newNextHop = MaodvReplaceInsertNextHopsList(
                node,
                rrepPkt->destination.address,
                srcAddr,
                UPSTREAM_DIRECTION,
                interfaceIndex,
                TRUE,
                &mrtToDest->nextHops);

            MaodvRelayMglReply(
                node,
                msg,
                upstreamNode->nextHopAddr,
                upstreamNode->interface);

            MaodvSendMact(
                node,
                rrepPkt->destination.address,
                ANY_DEST,
                DEFAULT_INTERFACE,
                U,
                mrtToDest->hopCountLeader,
                mrtToDest->destination.seqNum,
                mrtToDest->mcastLeader);

            return;
        }
        else if (mrtToDest->mcastLeader == rrepPkt->leader)
        {
            MaodvRouteEntry *rtEntry = NULL;
            BOOL isValidRtToSrc = FALSE;
            MaodvNextHopNode *upstreamNode = NULL;
            MaodvNextHopNode *newNextHop1 = NULL;

            rtEntry = MaodvCheckRouteExist(
                node,
                rrepPkt->sourceAddr,
                &maodv->routeTable,
                &isValidRtToSrc);

            if (rtEntry == NULL || !isValidRtToSrc)
            {
                // does not have valid route to the source
                return;
            }
            upstreamNode =  MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

            if (upstreamNode != NULL)
            {
                if (upstreamNode->nextHopAddr == rtEntry->nextHop
                    || upstreamNode->nextHopAddr != srcAddr)
                {
                    return;
                }
            }

            ERROR_Assert(
                rrepPkt->destination.seqNum >= mrtToDest->destination.seqNum,
                "old seq no at mgl reply higher partition \n");

            // update downstream
            newNextHop1 = MaodvReplaceInsertNextHopsList(
                node,
                rrepPkt->destination.address,
                rtEntry->nextHop,
                DOWNSTREAM_DIRECTION,
                rtEntry->interface,
                TRUE,
                &mrtToDest->nextHops);

            mrtToDest->destination.seqNum = rrepPkt->destination.seqNum;

            mrtToDest->lifetime =
                MAX(
                    mrtToDest->lifetime,
                    getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT);

            mrtToDest->hopCountLeader =
                MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;
            MaodvRelayMglReply(
                node,
                msg,
                rtEntry->nextHop,
                rtEntry->interface);
            return;
        }
        else
        {
            // some other intermediate member..
            return;
        }
    }
    else  if (mrtToDest == NULL || !isValidRt)
    {
        MaodvRouteEntry *rtEntry = NULL;
        BOOL isValidRtToSrc = FALSE;
        MaodvNextHopNode *upstreamNode = NULL;
        MaodvNextHopNode *newNextHop = NULL, *newNextHop1;
        rtEntry = MaodvCheckRouteExist(
            node,
            rrepPkt->sourceAddr,
            &maodv->routeTable,
            &isValidRtToSrc);

        if (rtEntry == NULL || !isValidRtToSrc)
        {
            // does not have valid route to the source
            return;
        }
        if (mrtToDest == NULL)
        {
            mrtToDest = MaodvCreateMrouteEntry(
                node,
                maodv,
                rrepPkt->destination.address,
                rrepPkt->destination.seqNum,
                rrepPkt->leader,
                MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1,
                MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1,
                TRUE,
                getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
                &maodv->mrouteTable);
        }

        if (rrepPkt->destination.seqNum < mrtToDest->destination.seqNum)
        {
            return;
        }
        mrtToDest->destination.seqNum = rrepPkt->destination.seqNum;
        mrtToDest->mcastLeader = rrepPkt->leader;
        mrtToDest->hopCount = MgiextPacketGetHopCount(rrepPkt->MgiextPkt)
            + 1;
        mrtToDest->hopCountLeader =
            MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;
        mrtToDest->isValid = TRUE;
        mrtToDest->isMember = TRUE;
        mrtToDest->lifetime =
            MAX(getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
                mrtToDest->lifetime);

        upstreamNode = MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

        if (upstreamNode != NULL)
        {
            BOOL last;

            MaodvDeleteNextHop(
                upstreamNode->nextHopAddr,
                &mrtToDest->nextHops,
                &last);
        }

        MaodvReplaceInsertRequestTable(
            rrepPkt->destination.address,
            rrepPkt->leader,
            &maodv->requestTable);

        newNextHop1 = MaodvReplaceInsertNextHopsList(
            node,
            rrepPkt->destination.address,
            rtEntry->nextHop,
            DOWNSTREAM_DIRECTION,
            rtEntry->interface,
            TRUE,
            &mrtToDest->nextHops);

        newNextHop = MaodvReplaceInsertNextHopsList(
            node,
            rrepPkt->destination.address,
            srcAddr,
            UPSTREAM_DIRECTION,
            interfaceIndex,
            TRUE,
            &mrtToDest->nextHops);

        MaodvRelayMglReply(node, msg, rtEntry->nextHop,rtEntry->interface);
        return;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleMGIExtension
//
// PURPOSE:  Handles multicast group information extension
//
// ARGUMENTS: node, The node which has the packet
//            msg,  The message containg the packet
//            srcAddr, Source address
//            interfaceIndex, Interface index
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvHandleMGIExtension(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    unsigned int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MgiextPacket *rrepPkt = (MgiextPacket *) MESSAGE_ReturnPacket(msg);

    MaodvMrouteEntry *prevRtPtr = NULL;

    // When a node receives a RREP message, it first increments the hop
    // count value in the RREP by one, to account for the new hop through
    // the intermediate node. Sec: 8.7

    int hopCount = MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;

    BOOL isValidRt = FALSE;
    BOOL lastEntry;
    BOOL isSource = FALSE;
    MaodvRreqSentNode *sentNode = NULL;

    // clocktype must be copied to access the field of that type
    clocktype  lifetime = (clocktype) (rrepPkt->lifetime * MILLI_SECOND);
    MaodvNextHopNode *srcHop;

    if (lifetime == 0)
    {
        // Lifetime is zero, so the route is not usable.
        return;
    }

    maodv->stats.numReplyRecved++;

    if (MgiextPacketGetR(rrepPkt->MgiextPkt) == 1)
    {
        // The repair flag is on. So handle repair

        if (DEBUG_FAILURE)
        {
            printf("Repair flag is on so handling repair\n");
        }

        MaodvHandleMglReply(node, msg, srcAddr, interfaceIndex);
        return;
    }

    // Update route to the previous hop node
    if (rrepPkt->destination.address != srcAddr
        && MaodvCheckRouteExist(
               node,
               srcAddr,
               &maodv->routeTable,
               &isValidRt)
        && !isValidRt)
    {
        MaodvRouteEntry *ptrToPrevHop = NULL;

        ptrToPrevHop = MaodvReplaceInsertRouteTable(
            node,
            maodv,
            srcAddr,
            0,
            1,
            srcAddr,
            getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
            TRUE,
            interfaceIndex,
            &maodv->routeTable);

        if (ptrToPrevHop->lifetime == getSimTime(node) +
            MAODV_MY_ROUTE_TIMEOUT)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_CheckRouteTimeout,
                srcAddr,
                (clocktype) MAODV_MY_ROUTE_TIMEOUT);
        }
    }


    prevRtPtr = MaodvCheckMrouteExist(
        node,
        rrepPkt->destination.address,
        &maodv->mrouteTable,
        &isValidRt);

    isSource = NetworkIpIsMyIP(node,rrepPkt->sourceAddr);

    if (prevRtPtr == NULL)
    {
        // This node should not receive this reply

        if (DEBUG_ROUTE_DISCOVERY)
        {
            printf("discarding as no previous mroute entry");
        }
        return;
    }

    // IF IT IS THE LEADER OF THE GROUP AND RECEIVES A RREP, if the other
    // node IP is greater initiate tree combine else ignore

    if (MaodvLookupLeadership(
            rrepPkt->destination.address,
            &maodv->memberFlag))
    {
        if (DEBUG_ROUTE_DISCOVERY)
        {
            printf("Itself a Leader, initiate tree combine if own IP"
                "is small\n");
        }

        if (NetworkIpGetInterfaceAddress(node, 0) < rrepPkt->leader)
        {
            // This nodes IP address is less than the group leaders IP address
            // mentioned in the Route Reply. So it needs to accept the other
            // node as group leader.

            MaodvSetMembershipType(
                rrepPkt->destination.address,
                MEMBER,
                &maodv->memberFlag);

            prevRtPtr->isMember = FALSE;

            MaodvReplaceInsertRequestTable(
                rrepPkt->destination.address,
                rrepPkt->leader,
                &maodv->requestTable);

            MaodvInitiateTreeCombineRequest(
                node,
                rrepPkt->destination.address,
                rrepPkt->leader,
                srcAddr,
                interfaceIndex,
                rrepPkt->groupHopCount +
                (MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1));
        }
        return;
    }

    srcHop = MaodvCheckNextHopsList(srcAddr, &prevRtPtr->nextHops);


    // Accept only if fresh enough route
    if (rrepPkt->destination.seqNum >= prevRtPtr->destination.seqNum)
    {
        MaodvNextHopNode *upstreamNode = MaodvFindUpstreamNextHop(
            &prevRtPtr->nextHops);

        if (DEBUG_ROUTE_DISCOVERY)
        {
            printf("Fresh enough route\n");
        }

        if (upstreamNode != NULL)
        {
            // already forwarded an rrep for this destination.. hence
            // forward now only if fresh enough
            if (upstreamNode->isEnabled)
            {
                if (DEBUG_ROUTE_DISCOVERY)
                {
                    printf("Upstream enabled\n");
                }

                if ((upstreamNode->nextHopAddr != srcAddr)
                    && ((rrepPkt->destination.seqNum >
                        prevRtPtr->destination.seqNum)
                        || (rrepPkt->destination.seqNum ==
                            prevRtPtr->destination.seqNum
                            && (hopCount + rrepPkt->groupHopCount) <
                                prevRtPtr->hopCountLeader)))
                {
                    // DELETE THE CURRENT UPSTREAM_DIRECTION AND ACTIVATE THE NEW
                    // UPSTREAM_DIRECTION
                    BOOL lastEntry;
                    MaodvDeleteNextHop(
                        upstreamNode->nextHopAddr,
                        &prevRtPtr->nextHops,
                        &lastEntry);

                    upstreamNode = MaodvReplaceInsertNextHopsList(
                                       node,
                                       rrepPkt->destination.address,
                                       srcAddr,
                                       UPSTREAM_DIRECTION,
                                       interfaceIndex,
                                       TRUE,
                                       &prevRtPtr->nextHops);
                }
                else
                {
                    return;
                }

                prevRtPtr->destination.seqNum = rrepPkt->destination.seqNum;
                prevRtPtr->hopCount =
                    MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1;
                prevRtPtr->mcastLeader = rrepPkt->leader;
                prevRtPtr->hopCountLeader =
                    MgiextPacketGetHopCount(rrepPkt->MgiextPkt) + 1
                    + rrepPkt->groupHopCount;

                MaodvReplaceInsertRequestTable(
                    prevRtPtr->destination.address,
                    rrepPkt->leader,
                    &maodv->requestTable);

                // send mact to enable the upstream
                MaodvSendMact(
                    node,
                    rrepPkt->destination.address,
                    upstreamNode->nextHopAddr,
                    upstreamNode->interface,
                    J,
                    0,
                    prevRtPtr->destination.seqNum,
                    0);

                MaodvRelayMGIEXT(node, msg, prevRtPtr);
                return;
            }

            // forward only if seq no higher or hop count lower
            else if (rrepPkt->destination.seqNum >
                    prevRtPtr->destination.seqNum
                || (rrepPkt->destination.seqNum ==
                    prevRtPtr->destination.seqNum
                && (hopCount + rrepPkt->groupHopCount) <
                    prevRtPtr->hopCountLeader))
            {
                MaodvNextHopNode *newNextHop = NULL;
                MaodvDeleteNextHop(
                    upstreamNode->nextHopAddr,
                    &prevRtPtr->nextHops,
                    &lastEntry);

                prevRtPtr->destination.seqNum = rrepPkt->destination.seqNum;

                prevRtPtr->lifetime =
                    MAX(
                        prevRtPtr->lifetime,
                        getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT);

                newNextHop = MaodvReplaceInsertNextHopsList(
                    node,
                    rrepPkt->destination.address,
                    srcAddr,
                    UPSTREAM_DIRECTION,
                    interfaceIndex,
                    FALSE,
                    &prevRtPtr->nextHops);

                prevRtPtr->hopCount = hopCount;
                prevRtPtr->hopCountLeader = hopCount + rrepPkt->groupHopCount;
                prevRtPtr->mcastLeader = rrepPkt->leader;

                MaodvReplaceInsertRequestTable(
                    prevRtPtr->destination.address,
                    rrepPkt->leader,
                    &maodv->requestTable);

                if (isSource)
                {
                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("Can't forward as this is the source\n");
                    }
                    return;
                }
                else
                {
                    if (DEBUG_ROUTE_DISCOVERY)
                    {
                        printf("Relaying MGIEXT\n");
                    }

                    MaodvRelayMGIEXT(node, msg, prevRtPtr);
                    return;
                }
            }
            else
            {
                return;
            }
        }
        else
        {
            MaodvNextHopNode *newNextHop = NULL;
            // first reply - forward now

            if (DEBUG_ROUTE_DISCOVERY)
            {
                printf("No upstream and first reply\n");
            }

            if (!(prevRtPtr->isMember) && isValidRt)
            {
                sentNode = MaodvCheckSent(
                    rrepPkt->destination.address,
                    &maodv->sent);

                if (sentNode == NULL)
                {
                    ERROR_Assert(FALSE, "Sent node NULL!!");
                    return;
                }
                if (sentNode->type == COMBINE)
                {
                    return;
                }
                else
                {
                    // MaodvDeleteSent(
                    //     rrepPkt->destination.address,
                    //     &maodv->sent);
                    sentNode->isReplied = TRUE;
                }
            }

            prevRtPtr->lifetime =
                MAX(prevRtPtr->lifetime, getSimTime(node) +
                    MAODV_REV_ROUTE_LIFE);

            newNextHop = MaodvReplaceInsertNextHopsList(
                node,
                rrepPkt->destination.address,
                srcAddr,
                UPSTREAM_DIRECTION,
                interfaceIndex,
                FALSE,
                &prevRtPtr->nextHops);

            prevRtPtr->destination.seqNum = rrepPkt->destination.seqNum;
            prevRtPtr->mcastLeader = rrepPkt->leader;
            prevRtPtr->hopCount = hopCount;
            prevRtPtr->hopCountLeader = hopCount + rrepPkt->groupHopCount;

            MaodvReplaceInsertRequestTable(
                prevRtPtr->destination.address,
                rrepPkt->leader,
                &maodv->requestTable);

            // forward the MGIEXT

            if (!isSource)
            {
                MaodvRelayMGIEXT(node, msg, prevRtPtr);
                return;
            }
        }
    }
    else
    {
         return;
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleTreePartition
//
// PURPOSE:  Handles tree partitioning
//
// ARGUMENTS: node, The node which is partitioning
//            mcastAddr, Multicast address
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvHandleTreePartition(Node *node, NodeAddress mcastAddr)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    BOOL isValidRt;
    MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValidRt);

    int validNextHops;

    if (mrtToDest == NULL)
    {
                return;
    }

    validNextHops = MaodvGetValidNextHops(&mrtToDest->nextHops);
    if (MaodvLookupGroupMembership(mcastAddr, &maodv->memberFlag))
    {
        MAODV_M_Node *member = NULL;
        MaodvNextHopNode *upstreamNode = NULL;
        // change status to leader
        member = MaodvSetMembershipType(
            mcastAddr,
            LEADER,
            &maodv->memberFlag);

        ERROR_Assert(member != NULL,
            "membership info not found -- NULL \n");

        // the node is a group member, hence declare as leader
        // and send GroupHello message with update
        member->isUpdate = TRUE;

        mrtToDest = MaodvCheckMrouteExist(
            node,
            mcastAddr,
            &maodv->mrouteTable,
            &isValidRt);

        mrtToDest->mcastLeader = NetworkIpGetInterfaceAddress(node, 0);
        mrtToDest->hopCountLeader = 0;
        mrtToDest->isMember = TRUE;

        // update request table
        MaodvReplaceInsertRequestTable(
            mcastAddr,
            NetworkIpGetInterfaceAddress(node, 0),
            &maodv->requestTable);

        // increment the sequence number
        mrtToDest->destination.seqNum++;
        upstreamNode = MaodvFindUpstreamNextHop(&mrtToDest->nextHops);

        if (upstreamNode != NULL)
        {
            BOOL last;
            ERROR_Assert(!upstreamNode->isEnabled,
                "upstream enabled and declaring as leader \n");

            MaodvDeleteNextHop(
                upstreamNode->nextHopAddr,
                &mrtToDest->nextHops,
                &last);
        }

        MaodvIncreaseSeq(maodv);

        // start sending group hello messages
        MaodvSetTimer(node,
                      NULL,
                      MSG_ROUTING_MaodvSendGroupHello,
                      mcastAddr,
                      0);

        // get the buffered packets to the group and start sending them
        while (MaodvLookupBuffer(mcastAddr,&maodv->msgBuffer))
        {
            NodeAddress previousHop;
            Message *newMsg = MaodvGetBufferedPacket(
                mcastAddr,
                &previousHop,
                &maodv->msgBuffer);

            MaodvTransmitMdata(
                node,
                MESSAGE_Duplicate(node,newMsg),
                mrtToDest,
                previousHop,
                0); // incomming interface

            MaodvDeleteBuffer(
                mrtToDest->destination.address,
                &maodv->msgBuffer);

            MESSAGE_Free(node, newMsg);
        }
    }
    else if (mrtToDest->nextHops.size == 1)
    {
        MaodvNextHopNode *selectedNextHop = NULL;
        mrtToDest->isMember = TRUE;

        // get the buffered packets to the group and start sending them
        while (MaodvLookupBuffer(mcastAddr,&maodv->msgBuffer))
        {
            NodeAddress previousHop;
            Message *newMsg = MaodvGetBufferedPacket(
                mcastAddr,
                &previousHop,
                &maodv->msgBuffer);

            MaodvTransmitMdata(
                node,
                MESSAGE_Duplicate(node,newMsg),
                mrtToDest,
                previousHop,
                0); // incoming interface

            MaodvDeleteBuffer(
                mrtToDest->destination.address,
                &maodv->msgBuffer);

            MESSAGE_Free(node, newMsg);
        }

        // select a nextHop and unicast a MACT with P flag
        for (selectedNextHop = mrtToDest->nextHops.head;
            selectedNextHop != NULL;
            selectedNextHop = selectedNextHop->next)
        {
            if (selectedNextHop->isEnabled
                && selectedNextHop->direction == DOWNSTREAM_DIRECTION)
            {
                break;
            }
        }

        if (validNextHops == 1)
        {
            if (selectedNextHop != NULL)
            {
                MaodvSendMact(
                    node,
                    mcastAddr,
                    selectedNextHop->nextHopAddr,
                    selectedNextHop->interface,
                    P,
                    0,
                    mrtToDest->destination.seqNum,
                    0);
            }
        }

        // reset member flag
        // delete route entry
        MaodvDeleteMroute(
            node,
            mrtToDest->destination.address,
            &maodv->mrouteTable);
    }
    else
    {
        MaodvNextHopNode *selectedNextHop = NULL;
        // not a group member
        // select a nextHop and unicast a MACT with G flag
        for (selectedNextHop = mrtToDest->nextHops.head;
            selectedNextHop != NULL;
            selectedNextHop = selectedNextHop->next)
        {
            if (selectedNextHop->isEnabled
                && selectedNextHop->direction == DOWNSTREAM_DIRECTION)
            {
                break;
            }
        }

        if (selectedNextHop == NULL)
        {
            while (MaodvLookupBuffer(mcastAddr,&maodv->msgBuffer))
            {
                NodeAddress previousHop;
                Message *newMsg = MaodvGetBufferedPacket(
                    mcastAddr,
                    &previousHop,
                    &maodv->msgBuffer);

                maodv->stats.numMdataDropped++;

                MaodvDeleteBuffer(
                    mrtToDest->destination.address,
                    &maodv->msgBuffer);

                MESSAGE_Free(node, newMsg);
            }

            MaodvDeleteMroute(
                node,
                mrtToDest->destination.address,
                &maodv->mrouteTable);

            return;
        }

        MaodvSendMact(
            node,
            mcastAddr,
            selectedNextHop->nextHopAddr,
            selectedNextHop->interface,
            G,
            0,
            mrtToDest->destination.seqNum,
            0);

        // set the direction of the nextHop to upstream
        selectedNextHop->direction = UPSTREAM_DIRECTION;
        mrtToDest->isMember = TRUE;

        // get the buffered packets to the group and start sending them
        while (MaodvLookupBuffer(mcastAddr,&maodv->msgBuffer))
        {
            NodeAddress previousHop;

            Message *newMsg = MaodvGetBufferedPacket(
                mcastAddr,
                &previousHop,
                &maodv->msgBuffer);

            MaodvTransmitMdata(
                node,
                MESSAGE_Duplicate(node,newMsg),
                mrtToDest,
                previousHop,
                0); // incoming interface

            MaodvDeleteBuffer(
                mrtToDest->destination.address,
                &maodv->msgBuffer);

            MESSAGE_Free(node, newMsg);
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvLeaveGroup
// PURPOSE      Leave a multicast group
//
// Parameters:
//     node:          Node that is leaving the group.
//     mcastAddr:     Multicast group to leave.
// RETURN: None
//---------------------------------------------------------------------------
static
void MaodvLeaveGroup(Node *node, NodeAddress mcastAddr)
{
    MaodvData* maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MaodvMrouteEntry *mrtToDest;
    BOOL isValid;

    // Leave group if a member
    mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValid);

    ERROR_Assert(mrtToDest != NULL,
        "NULL multicast route entry for leave group\n");
    ERROR_Assert(MaodvLookupGroupMembership(mcastAddr, &maodv->memberFlag),
        "non-group member trying to leave \n");

    if (MaodvLookupLeadership(mcastAddr, &maodv->memberFlag))
    {
        MaodvNextHopNode *selectedNextHop = NULL;

        // send mact prune
        for (selectedNextHop = mrtToDest->nextHops.head;
            selectedNextHop != NULL;
            selectedNextHop = selectedNextHop->next)
        {
            if (selectedNextHop->isEnabled)
            {
                break;
            }
        }

        if (selectedNextHop == NULL)
        {
            // no valid nexthop to transfer leadership
            MaodvResetMemberFlag(mcastAddr, &maodv->memberFlag);
            MaodvDeleteMroute(node, mcastAddr,&maodv->mrouteTable);
            return;
        }

        ERROR_Assert(selectedNextHop->direction == DOWNSTREAM_DIRECTION,
            "g mact from leader to upstream not possible\n");

        if (mrtToDest->nextHops.size == 1)
        {
            MaodvSendMact(
                node,
                mcastAddr,
                selectedNextHop->nextHopAddr,
                selectedNextHop->interface,
                P,
                0,
                mrtToDest->destination.seqNum,
                0);

            MaodvResetMemberFlag(mcastAddr, &maodv->memberFlag);
            MaodvDeleteMroute(node, mcastAddr,&maodv->mrouteTable);
            return;
        }
        else
        {
            MaodvSendMact(
                node,
                mcastAddr,
                selectedNextHop->nextHopAddr,
                selectedNextHop->interface,
                G,
                0,
                mrtToDest->destination.seqNum,
                0);

            selectedNextHop->direction = UPSTREAM_DIRECTION;
            MaodvResetMemberFlag(mcastAddr, &maodv->memberFlag);
            return;
        }
    }
    else
    {
        MaodvNextHopNode *selectedNextHop = NULL;
        if (mrtToDest->nextHops.size == 1)
        {
            for (selectedNextHop = mrtToDest->nextHops.head;
                selectedNextHop != NULL;
                selectedNextHop = selectedNextHop->next)
            {
                if (selectedNextHop->isEnabled)
                {
                    MaodvSendMact(
                        node,
                        mcastAddr,
                        selectedNextHop->nextHopAddr,
                        selectedNextHop->interface,
                        P,
                        0,
                        mrtToDest->destination.seqNum,
                        0);

                    MaodvResetMemberFlag(mcastAddr, &maodv->memberFlag);
                    MaodvDeleteMroute(node, mcastAddr,&maodv->mrouteTable);
                    return;
                }
            }
        }
    }
} /* MaodvLeaveGroup */


//---------------------------------------------------------------------------
// FUNCTION: MaodvRelayRREP
//
// PURPOSE:  Forward the RREP packet
//
// ARGUMENTS: node, the node relaying reply
//            msg,  message containing received route reply packet
//            destRouteEntry, pointer to the destination route
//
// RETURN:   none
//---------------------------------------------------------------------------
static
void MaodvRelayRREP(
    Node *node,
    Message *msg,
    MaodvRouteEntry *destRouteEntry)
{
    Message *newMsg = NULL;
    RrepPacket *newRrep = NULL;
    char *pktPtr = NULL;
    BOOL isRtToSrcExist = FALSE;
    int pktSize = sizeof(RrepPacket);

    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    RrepPacket *oldRrep = (RrepPacket *) MESSAGE_ReturnPacket(msg);

    MaodvRouteEntry *rtToSource = MaodvCheckRouteExist(
        node,
        oldRrep->sourceAddr,
        &maodv->routeTable,
        &isRtToSrcExist);

    // If the current node is not the source node as indicated by the Source
    // IP Address in the RREP message AND a forward route has been created
    // or updated as described before, the node consults its route table
    // entry for the source node to determine the next hop for the RREP
    // packet, and then forwards the RREP towards the source with its Hop
    // Count incremented by one. Sec: 8.7

    if (!isRtToSrcExist)
    {
        return;
    }
    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    newRrep = (RrepPacket *) pktPtr;
    RrepPacketSetType(&(newRrep->Rreppkt), MAODV_RREP);
    RrepPacketSetR(&(newRrep->Rreppkt), 0);
    RrepPacketSetResv(&(newRrep->Rreppkt), 0);
    RrepPacketSetPrefixSz(&(newRrep->Rreppkt), 0);
    newRrep->sourceAddr  = oldRrep->sourceAddr;
    newRrep->destination.address = oldRrep->destination.address;
    newRrep->destination.seqNum  = oldRrep->destination.seqNum;
    RrepPacketSetHopCount(&(newRrep->Rreppkt),
        (RrepPacketGetHopCount(oldRrep->Rreppkt) + 1));

    if ((int)RrepPacketGetHopCount(newRrep->Rreppkt) > maodv->netDiameter)
    {
        ERROR_ReportError("MAODV: Please increase MAODV_NET_DIAMETER.\n");
    }

    newRrep->lifetime = oldRrep->lifetime;

    NetworkIpSendRawMessageToMacLayer(
        node,
        newMsg,
        NetworkIpGetInterfaceAddress(node, rtToSource->interface),
        rtToSource->nextHop,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        rtToSource->interface,
        rtToSource->nextHop);

    maodv->stats.numReplyForwarded++;

    // Also, at each node the (reverse) route used to forward a RREP has
    // its lifetime changed to current time plus ACTIVE_ROUTE_TIMEOUT.
    MaodvUpdateLifetime(node, maodv, rtToSource->nextHop, &maodv->routeTable);
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleReply
//
// PURPOSE:  Processing procedure when RREP is received, applicable only for
//           those nodes which are not part of the multicast tree, and just want
//           send packets to the multicast group members.
//           Multicast group members or forwarding nodes will not receive reply
//           through this function.
//
// ARGUMENTS: node, the node received reply
//            msg,  Message containing rreq packet
//            srcAddr, source address in the ip header
//            destAddr, destination address in the ip header
//            interfaceIndex, the interface through which reply has been
//                            received
//
// RETURN:   None
//---------------------------------------------------------------------------

static
void MaodvHandleReply(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    int interfaceIndex)
{
    MaodvData* maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    RrepPacket* rrepPkt = (RrepPacket *) MESSAGE_ReturnPacket(msg);
    clocktype lifetime;

    MaodvRouteEntry* prevRtPtr = NULL;
    MaodvRouteEntry* newRtPtr = NULL;
    MaodvRouteEntry* rtToDest = NULL;

    // When a node receives a RREP message, it first increments the hop
    // count value in the RREP by one, to account for the new hop through
    // the intermediate node. Sec: 8.7 draft-ietf-manet-aodv-09.txt

    int hopCount = RrepPacketGetHopCount(rrepPkt->Rreppkt) + 1;

    BOOL newRtAdded = FALSE;
    BOOL routeUpdated = FALSE;
    BOOL isValidRt = FALSE;

    // clocktype must be copied to access the field of that type
    lifetime = (clocktype) (rrepPkt->lifetime * MILLI_SECOND);

    if (lifetime == 0)
    {
        return;
    }

    // HELLO packet processing
    if (rrepPkt->sourceAddr == ANY_IP)
    {

        if (DEBUG_HELLO)
        {
            char time[100];
            TIME_PrintClockInSecond(getSimTime(node), time);
            printf("Node %u:", node->nodeId);
            printf("\tReceived reply at %s\n", time);
            printf("\tDest %x, Dest Seq %u, Source %x\n",
                rrepPkt->destination.address,
                rrepPkt->destination.seqNum,
                rrepPkt->sourceAddr);
        }

        if (NetworkIpIsMyIP(node, rrepPkt->destination.address))
        {
            // Own hello paket
            return;
        }

        // Whenever a node receives a HELLO packet from a neighbor, the node
        // SHOULD make sure that it has an active route to the neighbor,
        // and create one if necessary.  If a route already exists, then the
        // Lifetime for the route should be increased if necessary to be at
        // least ACTIVE_ROUTE_TIMEOUT. In any case, the route to the neighbor
        // should be updated to contain the latest Destination Sequence Number
        // from the HELLO message. Sec: 8.9

        rtToDest = MaodvReplaceInsertRouteTable(
            node,
            maodv,
            rrepPkt->destination.address,
            rrepPkt->destination.seqNum,
            RrepPacketGetHopCount(rrepPkt->Rreppkt) + 1,
            srcAddr,
            getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
            TRUE,
            interfaceIndex,
            &maodv->routeTable);

        if (rtToDest->lifetime == getSimTime(node) +
            MAODV_ACTIVE_ROUTE_TIMEOUT)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_CheckRouteTimeout,
                rrepPkt->destination.address,
                (clocktype) MAODV_ACTIVE_ROUTE_TIMEOUT);
        }
        maodv->stats.numHelloRecved++;
        return;
    }

    maodv->stats.numReplyRecved++;

    if (DEBUG_ROUTE_DISCOVERY)
    {
        char srcStr[25];
        char destStr[25];
        char time[100];

        TIME_PrintClockInSecond(getSimTime(node), time);

        IO_ConvertIpAddressToString(srcAddr, srcStr);
        IO_ConvertIpAddressToString(destAddr, destStr);

        printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
            node->nodeId,
            time);
        printf("\tReceived RREP: dest %s, prev hop %s\n",
            destStr,
            srcStr);

        IO_ConvertIpAddressToString(rrepPkt->sourceAddr, srcStr);
        IO_ConvertIpAddressToString(rrepPkt->destination.address, destStr);

        printf("\tReply Dest %s, Reply Dest Seq %u, Reply Src %s\n",
            destStr, rrepPkt->destination.seqNum, srcStr);

    }

    if (rrepPkt->destination.address != srcAddr
        && MaodvCheckRouteExist(
               node,
               srcAddr,
               &maodv->routeTable,
               &isValidRt)
        && !isValidRt)
    {
        MaodvRouteEntry *ptrToPrevHop = NULL;

        ptrToPrevHop = MaodvReplaceInsertRouteTable(
            node,
            maodv,
            srcAddr,
            0,
            1,
            srcAddr,
            getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
            TRUE,
            interfaceIndex,
            &maodv->routeTable);

        if (ptrToPrevHop->lifetime == getSimTime(node) +
            MAODV_MY_ROUTE_TIMEOUT)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_CheckRouteTimeout,
                srcAddr,
                (clocktype) MAODV_MY_ROUTE_TIMEOUT);
        }
    }

    prevRtPtr = MaodvCheckRouteExist(
        node,
        rrepPkt->destination.address,
        &maodv->routeTable,
        &isValidRt);

    rtToDest = prevRtPtr;

    // The forward route for this destination is created or updated only if
    // (i) the Destination Sequence Number in the RREP is greater than the
    //     node's copy of the destination sequence number, or
    // (ii) the sequence numbers are the same, but the route is no longer
    //      active or the incremented Hop Count in RREP is smaller than
    //      the hop count in route table entry. Sec: 8.7

    if ((!isValidRt)
        || (prevRtPtr->destination.seqNum < rrepPkt->destination.seqNum)
        || ((prevRtPtr->destination.seqNum == rrepPkt->destination.seqNum)
                && (prevRtPtr->hopCount > hopCount)))
    {
        // Add the forward route entry in the routing table
        newRtPtr = MaodvReplaceInsertRouteTable(
            node,maodv,
            rrepPkt->destination.address,
            rrepPkt->destination.seqNum,
            hopCount,
            srcAddr,
            getSimTime(node) + lifetime,
            TRUE,
            interfaceIndex,
            &maodv->routeTable);

        if (newRtPtr->lifetime == getSimTime(node) + lifetime)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_CheckRouteTimeout,
                rrepPkt->destination.address,
                lifetime);
        }

        if (!prevRtPtr)
        {
            newRtAdded = TRUE;
            rtToDest = newRtPtr;
        }
        else
        {
            routeUpdated = TRUE;
            rtToDest     = prevRtPtr;
        }
    }

    // Source of the route
    if (NetworkIpIsMyIP(node, rrepPkt->sourceAddr))
    {
        MaodvDeleteSent(rrepPkt->destination.address, &maodv->sent);

        maodv->stats.numReplyRecvedAsSource++;

        // Send any buffered packets to the destination
        while (MaodvLookupBuffer(
            rrepPkt->destination.address,
            &maodv->msgBuffer))
        {
            NodeAddress previousHop;

            Message *newMsg = MaodvGetBufferedPacket(
                rrepPkt->destination.address,
                &previousHop,
                &maodv->msgBuffer);

            if (previousHop == ANY_IP)
            {
                maodv->stats.numMdataInitiated++;
            }

            MaodvTransmitData(node, newMsg, rtToDest, previousHop);
            MaodvDeleteBuffer(rrepPkt->destination.address, &maodv->msgBuffer);
        } // while
    } // if source
    // Intermediate node of the route
    else
    {
        if (newRtAdded || routeUpdated)
        {
            // Forward the packet to the upstream of the route
            MaodvRelayRREP(node, msg, rtToDest);
        } // if new route
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInitiateMglReply
//
// PURPOSE:  Initates Multicast GLreply
//
// ARGUMENTS: node, the node initates reply
//            maodv, maodv data pointer
//            msg,  Message containing packet
//            mrtToDest, multicast route to destination
//            srcAddr, source address in the ip header
//            interfaceIndex, the interface index
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvInitiateMglReply(
    Node *node,
    MaodvData *maodv,
    Message *msg,
    MaodvMrouteEntry *mrtToDest,
    NodeAddress srcAddr,
    unsigned int interfaceIndex)
{
    Message *newMsg;
    MgiextPacket *rrepPkt;
    char *pktPtr;
    int pktSize = sizeof(MgiextPacket);

    MglextPacket *rreqPkt = (MglextPacket *) MESSAGE_ReturnPacket(msg);
    MAODV_M_Node *member = NULL;

    rrepPkt = (MgiextPacket *) MESSAGE_ReturnPacket(msg);
    newMsg = MESSAGE_Alloc(node, MAC_LAYER, 0, MSG_MAC_FromNetwork);
    MESSAGE_PacketAlloc(node, newMsg, pktSize, TRACE_MAODV);
    pktPtr = (char *) MESSAGE_ReturnPacket(newMsg);
    rrepPkt = (MgiextPacket *) pktPtr;

    MgiextPacketSetType(&(rrepPkt->MgiextPkt), MAODV_MGIEXT);
    MgiextPacketSetR(&(rrepPkt->MgiextPkt), 1);
    MgiextPacketSetResv(&(rrepPkt->MgiextPkt), 0);
    MgiextPacketSetPrefixSz(&(rrepPkt->MgiextPkt), 0);
    rrepPkt->sourceAddr = rreqPkt->source.address;
    rrepPkt->destination.address = rreqPkt->destination.address;
    rrepPkt->destination.seqNum = mrtToDest->destination.seqNum;
    MgiextPacketSetHopCount(&(rrepPkt->MgiextPkt), 0);
    rrepPkt->lifetime = (unsigned int) (MAODV_MY_ROUTE_TIMEOUT / MILLI_SECOND);

    // Always 0th interface address has been assigned as the group leader
    // IP address.
    rrepPkt->leader = NetworkIpGetInterfaceAddress(node, 0);
    rrepPkt->groupHopCount = 0;

    NetworkIpSendRawMessageToMacLayer(
        node,
        MESSAGE_Duplicate(node,newMsg),
        NetworkIpGetInterfaceAddress(node, interfaceIndex),
        srcAddr,
        IPTOS_PREC_INTERNETCONTROL,
        IPPROTO_MAODV,
        1,
        interfaceIndex,
        srcAddr);

    MESSAGE_Free(node,newMsg);

    member = MaodvSetMembershipType(
        rreqPkt->destination.address,
        LEADER,
        &maodv->memberFlag);

    member->isUpdate = TRUE;
    return;
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleMGLEXT
//
// PURPOSE:  Handles Multicast GL extension
//
// ARGUMENTS: node, the node initates reply
//            msg,  Message containing packet
//            srcAddr, source address in the ip header
//            interfaceIndex, the interface index
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void  MaodvHandleMGLEXT(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    unsigned int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    MglextPacket *rreqPkt = (MglextPacket *) MESSAGE_ReturnPacket(msg);

    if (!MaodvLookupSeenTable(
        rreqPkt->source.address,
        rreqPkt->floodingId,
        0,
        &maodv->seenTable))
    {
        BOOL isValidSrc = FALSE;
        MaodvRouteEntry *rtToSrc;
        MaodvRouteEntry *rtEntryToDest;
        MaodvMrouteEntry *mrtToDest;
        BOOL isValidRt = FALSE;
        int seqNum;

        // This is not a duplicate packe so process the request
        // Insert the source and the broadcast id in seen table to protect
        // processing duplicates
        MaodvInsertSeenTable(
            node,
            rreqPkt->source.address,
            rreqPkt->floodingId,
            0,
            &maodv->seenTable);

        rtToSrc = MaodvCheckRouteExist(
            node,
            rreqPkt->source.address,
            &maodv->routeTable,
            &isValidSrc);

        if (!isValidSrc
            || rtToSrc->destination.seqNum < rreqPkt->source.seqNum
            || ((rtToSrc->destination.seqNum == rreqPkt->source.seqNum)
                && ( (int) rtToSrc->hopCount >
                (int)(MglextPacketGetHopCount(rreqPkt->MglextPkt) + 1))))
        {
            rtToSrc = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                rreqPkt->source.address,
                rreqPkt->source.seqNum,
                (MglextPacketGetHopCount(rreqPkt->MglextPkt) + 1),
                srcAddr,
                getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (rtToSrc->lifetime == getSimTime(node) +
                MAODV_ACTIVE_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    rreqPkt->source.address,
                    (clocktype) MAODV_ACTIVE_ROUTE_TIMEOUT);
            }
        }

        if (rreqPkt->source.address != srcAddr)
        {
            MaodvRouteEntry *ptrToPrevHop;
            ptrToPrevHop = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                srcAddr,
                0,
                1,
                srcAddr,
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (ptrToPrevHop->lifetime ==
                getSimTime(node) +  MAODV_MY_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    srcAddr,
                    (clocktype) MAODV_MY_ROUTE_TIMEOUT);
            }
        }

        mrtToDest = MaodvCheckMrouteExist(
            node,
            rreqPkt->destination.address,
            &maodv->mrouteTable,
            &isValidRt);

        seqNum = rreqPkt->destination.seqNum;

        if (isValidRt && mrtToDest != NULL)
        {
            // tree member
            if (mrtToDest->isMember)
            {
                // active tree node
                if (NetworkIpIsMyIP(node, rreqPkt->leader))
                {
                    MaodvNextHopNode *newNextHop = NULL;
                    // destination of mgl
                    if (MaodvLookupLeadership(
                        rreqPkt->destination.address,
                        &maodv->memberFlag))
                    {
                        MAODV_M_Node *memberInfo = MaodvLookupMemberFlag(
                            rreqPkt->destination.address,
                            &maodv->memberFlag);

                        if (memberInfo == NULL)
                        {
                            ERROR_Assert(FALSE,
                                "member info null not possible");
                        }

                        // leader so generate rrep
                        newNextHop = MaodvReplaceInsertNextHopsList(
                            node,
                            rreqPkt->destination.address,
                            srcAddr,
                            DOWNSTREAM_DIRECTION,
                            interfaceIndex,
                            TRUE,
                            &mrtToDest->nextHops);

                        mrtToDest->destination.seqNum =
                            MAX(
                                mrtToDest->destination.seqNum,
                                rreqPkt->destination.seqNum) + 1;
                        MaodvInitiateMglReply(
                            node,
                            maodv,
                            msg,
                            mrtToDest,
                            srcAddr,
                            interfaceIndex);
                        return;
                    }
                    else
                    {
                        // not a leader anymore - ignore
                        return;
                    }
                }
                else
                {
                    // not the destination of mgl
                    if (mrtToDest->mcastLeader == rreqPkt->leader)
                    {
                        // forward thro the upstream
                        MaodvNextHopNode *upstreamNode =
                            MaodvFindUpstreamNextHop(&mrtToDest->nextHops);
                        MaodvNextHopNode *downstreamNode = NULL;

                        if (upstreamNode == NULL)
                        {
                            ERROR_Assert(FALSE, "no upstream node found\n");
                        }

                        seqNum = MAX(seqNum, (int) mrtToDest->destination.seqNum);

                        if (srcAddr == upstreamNode->nextHopAddr)
                        {
                            return;
                        }

                        downstreamNode = MaodvCheckNextHopsList(
                            srcAddr,
                            &mrtToDest->nextHops);

                        if (downstreamNode == NULL)
                        {
                            downstreamNode = MaodvReplaceInsertNextHopsList(
                                node,
                                rreqPkt->destination.address,
                                srcAddr,
                                DOWNSTREAM_DIRECTION,
                                interfaceIndex,
                                FALSE,
                                &mrtToDest->nextHops);
                        }

                        MaodvRelayMGLEXT(
                            node,
                            msg,
                            srcAddr,
                            upstreamNode->nextHopAddr,
                            upstreamNode->interface,
                            seqNum);
                        return;
                    } // higher partition
                    else
                    {
                        //else if (mrtToDest->mcastLeader ==
                        //    rreqPkt->source.address){

                        MaodvNextHopNode *downstreamNode;

                        // else forward thro the unicast route table
                        rtEntryToDest = MaodvCheckRouteExist(
                            node,
                            rreqPkt->leader,
                            &maodv->routeTable,
                            &isValidRt);

                        if (rtEntryToDest == NULL || !isValidRt)
                        {
                            return;
                        }

                        downstreamNode = MaodvCheckNextHopsList(
                            srcAddr,
                            &mrtToDest->nextHops);

                        if (downstreamNode == NULL)
                        {
                            downstreamNode = MaodvReplaceInsertNextHopsList(
                                node,
                                rreqPkt->destination.address,
                                srcAddr,
                                DOWNSTREAM_DIRECTION,
                                interfaceIndex,
                                FALSE,
                                &mrtToDest->nextHops);
                        }

                        seqNum = MAX(seqNum, (int) mrtToDest->destination.seqNum);
                        MaodvRelayMGLEXT(
                            node,
                            msg,
                            srcAddr,
                            rtEntryToDest->nextHop,
                            rtEntryToDest->interface,
                            seqNum);
                        return;
                    } // lower partition
                }
            }
            else
            {
                // not an active tree node
                return;
            }
        }
        else
        {
             MaodvNextHopNode *downstreamNode;

             MaodvReplaceInsertRequestTable(
                 rreqPkt->destination.address,
                 rreqPkt->leader,
                 &maodv->requestTable);

             rtEntryToDest = MaodvCheckRouteExist(
                 node,
                 rreqPkt->leader,
                 &maodv->routeTable,
                 &isValidRt);

             if (rtEntryToDest!=NULL && isValidRt)
             {
                 if (mrtToDest != NULL)
                 {
                     seqNum = MAX(seqNum, (int) mrtToDest->destination.seqNum);
                 }
                 else
                 {
                     mrtToDest = MaodvCreateMrouteEntry(
                         node,
                         maodv,
                         rreqPkt->destination.address,
                         rreqPkt->destination.seqNum,
                         rreqPkt->leader,
                         -1,
                         -1,
                         FALSE,
                         getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT,
                         &maodv->mrouteTable);
                }

                 downstreamNode = MaodvReplaceInsertNextHopsList(
                     node,
                     rreqPkt->destination.address,
                     srcAddr,
                     DOWNSTREAM_DIRECTION,
                     interfaceIndex,
                     FALSE,
                     &mrtToDest->nextHops);

                 MaodvRelayMGLEXT(
                     node,
                     msg,
                     srcAddr,
                     rtEntryToDest->nextHop,
                     rtEntryToDest->interface,
                     seqNum);
            } // node has valid route to the leader
            return;
        }
    }
    else
    {
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvCheckForPrune
//
// PURPOSE:  Checks for pruning
//
// ARGUMENTS: node, the node initates reply
//            mrouteTable,  Multicast routing table
//            mrtToDest, multicast route
//
// RETURN:   None
//---------------------------------------------------------------------------
static
void MaodvCheckForPrune(
    Node *node,
    MaodvMroutingTable *mrouteTable,
    MaodvMrouteEntry *mrtToDest)
{
    NodeAddress mcastAddr = mrtToDest->destination.address;

    MaodvData *maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(node, MULTICAST_PROTOCOL_MAODV);

    MaodvRreqSentNode *sentNode = NULL;
    sentNode = MaodvCheckSent(mcastAddr, &maodv->sent);

    if (sentNode)
    {
        if (sentNode->type == REPAIR)
        {
            //do not prune while repair is underway
            return;
        }
    }

    if (mrtToDest->nextHops.size == 1)
    {
        // leaf node
        if (mrtToDest->nextHops.head->isEnabled)
        {
            MaodvSendMact(
                node,
                mcastAddr,
                mrtToDest->nextHops.head->nextHopAddr,
                mrtToDest->nextHops.head->interface,
                P,
                0,
                mrtToDest->destination.seqNum,
                0);

            MaodvDeleteMroute(node, mcastAddr, mrouteTable);
            mrtToDest = NULL;
        }
        else if (mrtToDest->lifetime < getSimTime(node))
        {
            MaodvDeleteMroute(node, mcastAddr, mrouteTable);
            mrtToDest = NULL;
        }
    }
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvInitializeConfigurableParameters
//
// PURPOSE: To initialyze the user configurable parameters or initialize
//          the corresponding variables with the default values as specified
//          in draft-ietf-manet-aodv-08.txt
//
// PARAMETERS: node, the node pointer, which is running aodv as its routing
//                   protocol
//             Maodv, Maodv internal structure
//
// RETURN:  None
//---------------------------------------------------------------------------

static
void MaodvInitializeConfigurableParameters(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex,
    MaodvData *maodv)
{
    BOOL wasFound;
    NodeAddress interfaceAddress;
    interfaceAddress = NetworkIpGetInterfaceAddress(node,interfaceIndex);

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-NET-DIAMETER",
        &wasFound,
        &maodv->netDiameter);

    if (!wasFound)
    {
        maodv->netDiameter = MAODV_DEFAULT_NET_DIAMETER;
    }

    IO_ReadTime(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-NODE-TRAVERSAL-TIME",
        &wasFound,
        &maodv->nodeTraversalTime);

    if (!wasFound)
    {
        maodv->nodeTraversalTime = MAODV_DEFAULT_NODE_TRAVERSAL_TIME;
    }

    IO_ReadTime(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-ACTIVE-ROUTE-TIMEOUT",
        &wasFound,
        &maodv->activeRouteTimeout);

    if (!wasFound)
    {
        maodv->activeRouteTimeout = MAODV_DEFAULT_ACTIVE_ROUTE_TIMEOUT;
    }

    IO_ReadTime(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-MY-ROUTE-TIMEOUT",
        &wasFound,
        &maodv->myRouteTimeout);

    if (!wasFound)
    {
        maodv->myRouteTimeout = MAODV_DEFAULT_MY_ROUTE_TIMEOUT;
    }

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-ALLOWED-HELLO-LOSS",
        &wasFound,
        &maodv->allowedHelloLoss);

    if (!wasFound)
    {
        maodv->allowedHelloLoss = MAODV_DEFAULT_ALLOWED_HELLO_LOSS;
    }

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-RREQ-RETRIES",
        &wasFound,
        &maodv->rreqRetries);

    if (!wasFound)
    {
        maodv->rreqRetries = MAODV_DEFAULT_RREQ_RETRIES;
    }

    IO_ReadTime(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-HELLO-INTERVAL",
        &wasFound,
        &maodv->helloInterval);

    if (!wasFound)
    {
        maodv->helloInterval = MAODV_DEFAULT_HELLO_INTERVAL;
    }

    IO_ReadTime(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-GROUP-HELLO-INTERVAL",
        &wasFound,
        &maodv->groupHelloInterval);

    if (!wasFound)
    {
        maodv->groupHelloInterval = MAODV_DEFAULT_GROUP_HELLO_INTERVAL;
    }

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-ROUTE-DELETION-CONSTANT",
        &wasFound,
        &maodv->rtDeletionConstant);

    if (!wasFound)
    {
        maodv->rtDeletionConstant = MAODV_DEFAULT_ROUTE_DELETE_CONST;
    }

    maodv->processHello = TRUE;

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-BUFFER-MAX-PACKET",
        &wasFound,
        &maodv->bufferSizeInNumPacket);

    if (wasFound == FALSE)
    {
        maodv->bufferSizeInNumPacket = MAODV_DEFAULT_MESSAGE_BUFFER_IN_PKT;
    }

    if (maodv->bufferSizeInNumPacket <= 0)
    {
        ERROR_ReportError("MAODV-BUFFER-MAX-PACKET needs to be a positive "
            "number\n");
    }

    IO_ReadInt(
        node->nodeId,
        interfaceAddress,
        nodeInput,
        "MAODV-BUFFER-MAX-BYTE",
        &wasFound,
        &maodv->bufferSizeInByte);

    if (wasFound == FALSE)
    {
        maodv->bufferSizeInByte = 0;
    }

    if (maodv->bufferSizeInByte < 0)
    {
        ERROR_ReportError("MAODV-BUFFER-MAX-BYTE needs to be a positive "
            "number\n");
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleData
//
// PURPOSE:  Processing procedure when data is received from another node.
//           this node is either intermediate hop or destination of the data
//
// ARGUMENTS: node, The node which has received data
//            msg,  The message received
//            destAddr, The destination for the packet
//            interfaceIndex, The incoming interface
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvHandleData(
    Node *node,
    Message *msg,
    NodeAddress sourceAddress,
    NodeAddress destAddr,
    NodeAddress previousHopAddress,
    int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    IpHeaderType *ipHeader = (IpHeaderType *) MESSAGE_ReturnPacket(msg);

    MaodvRouteEntry *rtToDest = NULL;

    BOOL isValidRoute = FALSE;
    BOOL isMember = FALSE;
    unsigned char ipProtocolNumber = ipHeader->ip_p;
    unsigned char IpProtocol;
    Message *newMsg = NULL;
    NodeAddress srcAddress;
    NodeAddress destAddress;
    unsigned int ttl;
    QueuePriorityType priority;
    MaodvMrouteEntry *mrtToDest;

    // Handle Multicast data
    mrtToDest = MaodvCheckMrouteExist(
        node,
        destAddr,
        &maodv->mrouteTable,
        &isValidRoute);

    if (isValidRoute && mrtToDest != NULL)
    {
        // member of the tree

        isMember = MaodvLookupGroupMembership(destAddr, &maodv->memberFlag);

        if (isMember)
        {
            // pass to the higher layer
            newMsg = MESSAGE_Duplicate(node, msg);

            NetworkIpRemoveIpHeader(
                node,
                newMsg,
                &srcAddress,
                &destAddress,
                &priority,
                &IpProtocol,
                &ttl);

            switch (ipProtocolNumber)
            {
            // Delivery to transport layer protocols.
            case IPPROTO_UDP:
                {
                    SendToUdp(
                        node,
                        newMsg,
                        priority,
                        sourceAddress,
                        destAddress,
                        interfaceIndex);
                    maodv->stats.numMdataDelivered++;
                    break;
                }
            default:
                {
                    ERROR_ReportError("MAODV currently only support UDP\n");
                }
            }
        }

        if (mrtToDest->isMember)
        {
            // is active
            MaodvTransmitMdata(
                node,
                msg,
                mrtToDest,
                previousHopAddress,
                interfaceIndex);
        }
        else
        {
            // buffer the data and send an acknowledgement for the
            // previous hop
            MaodvBroadcastHelloMessage(
                node,
                maodv,
                previousHopAddress);

            MaodvInsertBuffer(
                node,
                msg,
                destAddr,
                previousHopAddress,
                &maodv->msgBuffer);
        }
    }
    else
    {
        // Not a member of the multicast tree
        // check if it in forwarding path of the unicast route to

        if (DEBUG_PACKET)
        {
            printf("node %u: not a member of mcast tree for %x, "
                "check for ucast path\n", node->nodeId, destAddr);
        }

        rtToDest = MaodvCheckRouteExist(
            node,
            destAddr,
            &maodv->routeTable,
            &isValidRoute);

        if (!isValidRoute)
        {
            // Received a Multicast packet but don't know the route
            // and also not a tree member.

            if (DEBUG_PACKET)
            {
                printf("node %u: no valid path for %x, drop the packet \n",
                    node->nodeId, destAddr);
            }

            MESSAGE_Free(node,msg);
            return;
        }
        else
        {
            // transmit the packet
            MaodvTransmitData(node, msg, rtToDest, previousHopAddress);
            maodv->stats.numMdataForwarded++;
            return;
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleGRPH
//
// PURPOSE:  Processing Group Hello
//
// ARGUMENTS: node, The node which has received data
//            msg,  The message received
//            previousHop, previous hop address
//            interfaceIndex, The incoming interface
//
// RETURN:    None
//---------------------------------------------------------------------------

static
void MaodvHandleGRPH(
    Node *node,
    Message *msg,
    NodeAddress previousHop,
    unsigned int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    GrphPacket *oldGrph = NULL;
    MaodvMrouteEntry *mrtToDest = NULL;
    BOOL isValidRt = FALSE, isValidEntry = FALSE;
    int hopCount;
    MaodvRouteEntry *rtToLeader = NULL;

    oldGrph = (GrphPacket *) MESSAGE_ReturnPacket(msg);
    hopCount = GrphPacketGetHopCount(oldGrph->GrphPkt) + 1;

    // update neighbor info

    // check if already seen
    if (!MaodvLookupSeenTable(
        oldGrph->mcastGroup.address,
        oldGrph->mcastGroup.seqNum,
        oldGrph->leaderAddr,
        &maodv->seenTable))
    {
        // new group hello
        // add it to the seen table
        MaodvInsertSeenTable(
            node,
            oldGrph->mcastGroup.address,
            oldGrph->mcastGroup.seqNum,
            oldGrph->leaderAddr,
            &maodv->seenTable);

        // update route to the previous hop
        if (oldGrph->leaderAddr != previousHop
            && MaodvCheckRouteExist(
                   node,
                   previousHop,
                   &maodv->routeTable,
                   &isValidEntry)
            && !isValidEntry)
        {
            MaodvRouteEntry *ptrToPrevHop = NULL;
            ptrToPrevHop = MaodvReplaceInsertRouteTable(
                node,
                maodv,
                previousHop,
                0,
                1,
                previousHop,
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (ptrToPrevHop->lifetime ==
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    previousHop,
                    (clocktype) MAODV_MY_ROUTE_TIMEOUT);
            }
        }

        // update route to leader
        rtToLeader =  MaodvCheckRouteExist(
            node,
            oldGrph->leaderAddr,
            &maodv->routeTable,
            &isValidEntry);

        if (rtToLeader == NULL || !isValidEntry )
        {
            rtToLeader =  MaodvReplaceInsertRouteTable(
                node,
                maodv,
                oldGrph->leaderAddr,
                0,
                (GrphPacketGetHopCount(oldGrph->GrphPkt) + 1),
                previousHop,
                getSimTime(node) + MAODV_MY_ROUTE_TIMEOUT,
                TRUE,
                interfaceIndex,
                &maodv->routeTable);

            if (rtToLeader->lifetime ==  getSimTime(node) +
                MAODV_MY_ROUTE_TIMEOUT)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_CheckRouteTimeout,
                    oldGrph->leaderAddr,
                    (clocktype) MAODV_MY_ROUTE_TIMEOUT);
            }
        }
        mrtToDest = MaodvCheckMrouteExist(
            node,
            oldGrph->mcastGroup.address,
            &maodv->mrouteTable,
            &isValidRt);

        if (mrtToDest != NULL && isValidRt)
        {
            // tree member
            if (MaodvLookupLeadership(
                    oldGrph->mcastGroup.address,
                    &maodv->memberFlag)
                && NetworkIpGetInterfaceAddress(node, 0) !=
                    oldGrph->leaderAddr)
            {
                if (NetworkIpGetInterfaceAddress(node, 0) < oldGrph->leaderAddr)
                {
                    MaodvSetMembershipType(
                        oldGrph->mcastGroup.address,
                        MEMBER,
                        &maodv->memberFlag);

                    mrtToDest->isMember = FALSE;

                    MaodvReplaceInsertRequestTable(
                        oldGrph->mcastGroup.address,
                        oldGrph->leaderAddr,
                        &maodv->requestTable);

                    MaodvInitiateTreeCombineRequest(
                        node,
                        oldGrph->mcastGroup.address,
                        oldGrph->leaderAddr,
                        previousHop,
                        interfaceIndex,
                        hopCount);

                    return;
                }
                else
                {
                    // Other node has to initiate tree combine, so do nothing
                    return;
                }
            }
            else
            {
                // update the seq num and leader info if U flag set
                if (oldGrph->mcastGroup.seqNum <=
                    mrtToDest->destination.seqNum)
                {
                    MaodvRelayGroupHello(node, maodv, msg);
                    return;
                }
                else
                {
                    if (GrphPacketGetU(oldGrph->GrphPkt) == 0)
                    {
                        if (mrtToDest->mcastLeader !=  oldGrph->leaderAddr)
                        {
                            MaodvRelayGroupHello(node, maodv, msg);
                            return;
                        }

                        mrtToDest->destination.seqNum =
                            MAX(
                                mrtToDest->destination.seqNum,
                                oldGrph->mcastGroup.seqNum);
                    }
                    else if ((GrphPacketGetU(oldGrph->GrphPkt)== 1)
                        && mrtToDest->isMember)
                    {
                        MaodvNextHopNode *prevHopNode =
                            MaodvCheckNextHopsList(
                                previousHop,
                                &mrtToDest->nextHops);

                        if (prevHopNode != NULL)
                        {
                            if (prevHopNode->direction == UPSTREAM_DIRECTION
                                || (GrphPacketGetM(oldGrph->GrphPkt) == 0
                                    && mrtToDest->hopCountLeader ==
                                        (int) (GrphPacketGetHopCount
                                        (oldGrph->GrphPkt) + 1))
                                || mrtToDest->mcastLeader ==
                                    oldGrph->leaderAddr)
                            {
                                mrtToDest->destination.seqNum =
                                    MAX(
                                        mrtToDest->destination.seqNum,
                                        oldGrph->mcastGroup.seqNum);

                                mrtToDest->mcastLeader = oldGrph->leaderAddr;

                                MaodvReplaceInsertRequestTable(
                                    oldGrph->mcastGroup.address,
                                    oldGrph->leaderAddr,
                                    &maodv->requestTable);
                            }
                            else
                            {
                            }
                        }
                    }
                    if (GrphPacketGetM(oldGrph->GrphPkt) == 0)
                    {
                        if (mrtToDest->hopCountLeader !=
                            (int) (GrphPacketGetHopCount(
                            oldGrph->GrphPkt) + 1))
                        {
                        }
                    }
                    MaodvRelayGroupHello(node, maodv, msg);
                }
            }
        }
        else
        {
            MaodvReplaceInsertRequestTable(
                oldGrph->mcastGroup.address,
                oldGrph->leaderAddr,
                &maodv->requestTable);

            MaodvInsertSeenTable(
                node,
                oldGrph->mcastGroup.address,
                oldGrph->mcastGroup.seqNum,
                oldGrph->leaderAddr,
                &maodv->seenTable);

            MaodvRelayGroupHello(node, maodv, msg);
        }
    }
    else
    {
    }
}

//---------------------------------------------------------------------------
// FUNCTION     MaodvJoinGroup
// PURPOSE      Join a multicast group
//
// Parameters:
//     node:          Node that is joining the group.
//     mcastAddr:     Multicast group to join.
//
// RETURN: None
//---------------------------------------------------------------------------
static
void MaodvJoinGroup(Node *node, NodeAddress mcastAddr)
{
    MaodvData* maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(node, MULTICAST_PROTOCOL_MAODV);

    MaodvMrouteEntry *mrtToDest = NULL;
    BOOL isValidRt = FALSE;

    mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValidRt);

    if (mrtToDest != NULL && isValidRt)
    {
        MaodvSetMemberFlag(node, mcastAddr, MEMBER, &maodv->memberFlag);

        MaodvIncreaseSeq(maodv);
    }
    else
    {
        // No valid route exists in the routing table for the group. So either
        // this node was not a group member or tree member or the route to the
        // group is expired.

        // Check whether already sent a request for the group or not

        MaodvRequestEntry *rqEntry = MaodvCheckRequestTable(
            mcastAddr,
            &maodv->requestTable);

        NodeAddress leader;
        int hopCountLeader = -1;

        if (rqEntry != NULL)
        {
            leader = rqEntry->leaderAddr;
        }
        else
        {
            // No previous request found so insert one
            MaodvReplaceInsertRequestTable(
                mcastAddr,
                NetworkIpGetInterfaceAddress(node, 0), // Group leader's addr
                &maodv->requestTable);

            // Group leader's address is always set to the 0th interface
            // IP address.
            leader = NetworkIpGetInterfaceAddress(node, 0);
            hopCountLeader = 0;
        }

        if (mrtToDest == NULL)
        {
            // Create an entry for the Multicast address in the Multicast
            // routing table.
            mrtToDest = MaodvCreateMrouteEntry(
                node,
                maodv,
                mcastAddr,
                0,               // Multicast sequence number
                leader,
                -1,              // Hop count to next multicast group member
                hopCountLeader,  // Hop count to the leader
                TRUE,            // Is a valid route
                getSimTime(node) + MAODV_ACTIVE_ROUTE_TIMEOUT, // life time
                &maodv->mrouteTable);
        }
        else
        {
            // The node was a member of the multicast group or member of the
            // multicast tree but now the route is invalid. In this case the
            // node is not sending any request but just activating the previous
            // route.

            MaodvNextHopNode *upstreamNode = MaodvFindUpstreamNextHop(
                &mrtToDest->nextHops);

            mrtToDest->isValid = TRUE;

            if (upstreamNode != NULL)
            {
                MaodvSetMemberFlag(
                    node,
                    mcastAddr,
                    MEMBER,
                    &maodv->memberFlag);

                mrtToDest->isMember = TRUE;

                // enable the upstream
                upstreamNode->isEnabled = TRUE;

                MaodvSetMulticastTimer(
                    node,
                    MSG_ROUTING_MaodvTreeUtilizationTimer,
                    mrtToDest->destination.address,
                    upstreamNode->nextHopAddr,
                    (clocktype) MAODV_HELLO_LIFE);

                upstreamNode->lifetime =
                    getSimTime(node) + MAODV_MDATA_FORWARD_TIMEOUT;

                MaodvSetMulticastTimer(
                    node,
                    MSG_ROUTING_MaodvCheckMroute,
                    mrtToDest->destination.address,
                    upstreamNode->nextHopAddr,
                    (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);

                // send MACT to graft the tree
                MaodvSendMact(
                    node,
                    mrtToDest->destination.address,
                    upstreamNode->nextHopAddr,
                    upstreamNode->interface,
                    J,
                    0,
                    mrtToDest->destination.seqNum,
                    0);
                return;
            }
        }

        MaodvSetMemberFlag(node, mcastAddr, MEMBER, &maodv->memberFlag);
        MaodvInitiateRREQ(node, mcastAddr, JOIN);
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvProcessNextHopFailure
//
// PURPOSE:  Processing Next hop failure
//
// ARGUMENTS: node, The node which has received data
//            mcastAddr,  The multicast address
//            nextHopAddr, next hop address
//
// RETURN:    None
//---------------------------------------------------------------------------
static
void MaodvProcessNextHopFailure(
    Node *node,
    NodeAddress mcastAddr,
    NodeAddress nextHopAddr)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    BOOL isValidRt = FALSE;
    BOOL lastEntry = FALSE;

    MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
        node,
        mcastAddr,
        &maodv->mrouteTable,
        &isValidRt);

    MaodvNextHopNode *nextHop = NULL;

    if (DEBUG_FAILURE)
    {
        char addr[25];
        IO_ConvertIpAddressToString(nextHopAddr, addr);
        printf("MaodvProcessNextHopFailure: node %u\n"
            "\ttransmission failed to nextHopAddr = %s\n",
            node->nodeId,
            addr);

        IO_ConvertIpAddressToString(mcastAddr, addr);
        printf("\tDestination %s\n", addr);
        MaodvPrintMroutingTable(node, &maodv->mrouteTable);
        MaodvPrintRoutingTable(node, &maodv->routeTable);
    }

    if (!mrtToDest || !isValidRt)
    {
        // This is node is not a tree member
        // Check the unicast routing table and handle next hop failure
        // As there is no specification in the draft just delete the entries
        // all route which are using this next hop
        MaodvRoutingTable *routeTable = &maodv->routeTable;
        MaodvRouteEntry *rtEntry = routeTable->rtEntry;

        NetworkIpDeleteOutboundPacketsToANode(
            node,
            nextHopAddr,
            ANY_IP,
            FALSE);

        while (rtEntry)
        {
            if (rtEntry->activated && rtEntry->nextHop == nextHopAddr)
            {
                // Disable the route and then delete it after delete period
                MaodvDisableRoute(
                    node,
                    rtEntry->destination.address,
                    &maodv->routeTable);
            }
            rtEntry = rtEntry->next;
        }
        return;
    }

    nextHop = MaodvCheckNextHopsList(nextHopAddr, &mrtToDest->nextHops);

    if (nextHop == NULL)
    {
        ERROR_Assert(FALSE, "ProcessNextHopFailure: Next hop can't be NULL\n");
    }

    if (nextHop->direction == UPSTREAM_DIRECTION)
    {
        MAODV_M_Node *member = MaodvLookupMemberFlag(
            mcastAddr,
            &maodv->memberFlag);

        if (member != NULL)
        {
            member->isUpdate = TRUE;
        }

        // this node should initiate the repair
        // InitiateRepair
        // delete the nexthop
        MaodvDeleteNextHop(nextHopAddr, &mrtToDest->nextHops, &lastEntry);
        mrtToDest->isMember = FALSE;
        MaodvInitiateRREQ(node, mcastAddr, REPAIR);
    }
    else
    {
        // downstream failure
        // just delete the entry
        MaodvDeleteNextHop(nextHopAddr,&mrtToDest->nextHops, &lastEntry);

        if (!MaodvLookupGroupMembership(
            mcastAddr,
            &maodv->memberFlag)
            && mrtToDest->nextHops.size == 1)
        {
            // the node is the leaf node and non-member
            // hence it could be leader or a just a treemember
            MaodvSetTimer(
                node,
                NULL,
                MSG_ROUTING_MaodvPruneTimeout,
                mcastAddr,
                (clocktype) MAODV_PRUNE_TIMEOUT);
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvMacLayerStatusHandler
//
// PURPOSE:  Processing MAC layer status handler
//
// ARGUMENTS: node, The node which has received data
//            msg, The message containing the packet
//            nextHopAddress, next hop address
//            incommingInterfaceIndex, Incoming interface index
//
// RETURN:    None
//---------------------------------------------------------------------------
void
MaodvMacLayerStatusHandler(
    Node *node,
    const Message* msg,
    const NodeAddress nextHopAddress,
    const int incommingInterfaceIndex)
{
    IpHeaderType *ipHeader = NULL;
    NodeAddress destAddr;

    ERROR_Assert(MESSAGE_GetEvent(msg) == MSG_NETWORK_PacketDropped,
        "MAODV: Unexpected event in MAC layer status handler. \n");

    ipHeader = (IpHeaderType *) MESSAGE_ReturnPacket(msg);

    // Get the next hop towards the destination
    destAddr = ipHeader->ip_dst;

    if (ipHeader->ip_p  == IPPROTO_MAODV || destAddr == ANY_ADDRESS
        || nextHopAddress == ANY_ADDRESS)
    {
        return;
    }

    if (DEBUG_FAILURE)
    {
        char addr[25];
        IO_ConvertIpAddressToString(nextHopAddress, addr);
        printf("Node: %u, MacLayerStatusHandler\n"
                "\ttransmission failed to %s\n",
            node->nodeId,
            addr);
    }
    // process nexthop link failure
    MaodvProcessNextHopFailure(node, destAddr, nextHopAddress);
}

//---------------------------------------------------------------------------
// FUNCTION: RoutingIsMAODVEnabledInterface
//
// PURPOSE:  Determine whether a specific interface is maodv enabled or not
//
// ARGUMENTS: node,     The node received message
//            interfaceId, receiving interface
//
// RETURN:   BOOL
//---------------------------------------------------------------------------
BOOL RoutingIsMAODVEnabledInterface(Node* node,
        int interfaceId)
{
    NetworkDataIp* ipLayer = (NetworkDataIp*) node->networkData.networkVar;

    if ((ipLayer->interfaceInfo[interfaceId]->multicastEnabled == FALSE) ||
        (ipLayer->interfaceInfo[interfaceId]->multicastProtocolType !=
             MULTICAST_PROTOCOL_MAODV))
    {
        return (FALSE);
    }

    return (TRUE);
}


//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleProtocolPacket
//
// PURPOSE:  Called when Aodv packet is received from MAC, the packets may be
//           of following types, Route Request, Route Reply, Route Error,
//           Route Acknowledgement
//
// ARGUMENTS: node,     The node received message
//            msg,      The message received
//            srcAddr,  Source Address of the message
//            destAddr, Destination Address of the message
//            ttl,      time to leave
//            interfaceIndex, receiving interface
//
// RETURN:   None
//---------------------------------------------------------------------------

void MaodvHandleProtocolPacket(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    int ttl,
    int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(
            node,
            MULTICAST_PROTOCOL_MAODV);

    if (!RoutingIsMAODVEnabledInterface(node, interfaceIndex))
    {
        MESSAGE_Free(node, msg);
        return;
    }

    MaodvUpdateInsertNeighbor(
            node,
            srcAddr,
            &maodv->neighborTable);

    RreqPacket *Pkttype = (RreqPacket *) MESSAGE_ReturnPacket(msg);

    switch (RreqPacketGetType(Pkttype->RreqPkt))
    {
        case MAODV_RREQ:
        {
            BOOL isUnicast = FALSE;

            if (destAddr != ANY_DEST)
            {
                isUnicast = TRUE;
            }

            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived RREQ: dest %s, prev hop %s ttl %u\n",
                    destStr,
                    srcStr,
                    ttl);
            }

            MaodvHandleRequest(
                node,
                msg,
                srcAddr,
                ttl,
                interfaceIndex,
                isUnicast);

            MESSAGE_Free(node, msg);
            break;
        }

    case MAODV_RREP:
        {
            MaodvHandleReply(node, msg, srcAddr, destAddr, interfaceIndex);

            MESSAGE_Free(node, msg);
            break;
        }
    case MAODV_MGLEXT:
        {
            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived MGLEXT: dest %s, prev hop %s\n",
                    destStr,
                    srcStr);
            }

            MaodvHandleMGLEXT(node, msg, srcAddr, interfaceIndex);

            MESSAGE_Free(node, msg);
            break;
        }

    case MAODV_MGREXT:
        {
            BOOL isUnicast =FALSE;

            if (destAddr != ANY_DEST)
            {
                isUnicast = TRUE;
            }

            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived MGREXT: dest %s, prev hop %s\n",
                    destStr,
                    srcStr);
            }

            MaodvHandleMGREXT(
                node,
                msg,
                srcAddr,
                ttl,
                interfaceIndex,
                isUnicast);

            MESSAGE_Free(node, msg);
            break;
        }

    case MAODV_MGIEXT:
        {
            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived MGIEXT: dest %s, prev hop %s\n",
                    destStr,
                    srcStr);
            }

            MaodvHandleMGIExtension(node, msg, srcAddr, interfaceIndex);

            MESSAGE_Free(node, msg);
            break;
        }

    case MAODV_MACT:
        {
            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived MACT: dest %s, prev hop %s\n",
                    destStr,
                    srcStr);
            }

            MaodvHandleMACT(node, msg, srcAddr, interfaceIndex);

            MESSAGE_Free(node, msg);
            break;
        }
    case MAODV_GRPH:
        {
            if (DEBUG_ROUTE_DISCOVERY)
            {
                char srcStr[25];
                char destStr[25];
                char time[100];

                TIME_PrintClockInSecond(getSimTime(node), time);

                IO_ConvertIpAddressToString(srcAddr, srcStr);
                IO_ConvertIpAddressToString(destAddr, destStr);

                printf("\nMaodvHandleProtocolPacket: Node %u, time %s\n",
                    node->nodeId,
                    time);
                printf("\tReceived GRPH: dest %s, prev hop %s\n",
                    destStr,
                    srcStr);
            }
            MaodvHandleGRPH(node, msg, srcAddr, interfaceIndex);
            MESSAGE_Free(node, msg);
            break;
        }
    default:
        {
            char address[100];
            printf("\n NODE %d received unknown packet \n", node->nodeId);
            IO_ConvertIpAddressToString(srcAddr, address);
            printf("    srcAddr = %s\n", address);
            IO_ConvertIpAddressToString(destAddr, address);
            printf("    destAddr = %s\n", address);
            ERROR_Assert(FALSE, "Unknown packet type for Maodv");
            MESSAGE_Free(node, msg);
            break;
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvHandleProtocolEvent
//
// PURPOSE: Handles all the protocol events
//
// ARGUMENTS: node, the node received the event
//            msg,  msg containing the event type
//---------------------------------------------------------------------------

void MaodvHandleProtocolEvent(
    Node *node,
    Message *msg)
{
    MaodvData *maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(node, MULTICAST_PROTOCOL_MAODV);

    switch (MESSAGE_GetEvent(msg))
    {
        // Remove an entry from the RREQ Seen Table
    case MSG_NETWORK_FlushTables:
        {
            MaodvDeleteSeenTable(&maodv->seenTable);
            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_ROUTING_MaodvPruneTimeout:
        {
            NodeAddress *mcastAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            BOOL isValidRt = FALSE;
            MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
                node,
                *mcastAddr,
                &maodv->mrouteTable,
                &isValidRt);

            if (mrtToDest == NULL)
            {
                MESSAGE_Free(node,msg);
                return;
            }
            if (mrtToDest->nextHops.size <= 1
                && !MaodvLookupGroupMembership(
                        *mcastAddr,
                        &maodv->memberFlag))
             {
                MaodvNextHopNode *nextHop = mrtToDest->nextHops.head;
                if (nextHop != NULL)
                {
                    if (nextHop->isEnabled)
                    {
                        MaodvSendMact(
                            node,
                            *mcastAddr,
                            nextHop->nextHopAddr,
                            nextHop->interface,
                            P,
                            0,
                            mrtToDest->destination.seqNum,
                            0);

                        MaodvDeleteMroute(
                            node,
                            *mcastAddr,
                            &maodv->mrouteTable);
                    }
                    else if (mrtToDest->lifetime <= getSimTime(node))
                    {
                        MaodvDeleteMroute(
                            node,
                            *mcastAddr,
                            &maodv->mrouteTable);
                    }
                }
                else
                {
                    MaodvDeleteMroute(node, *mcastAddr, &maodv->mrouteTable);
                }
            }
            MESSAGE_Free(node,msg);
            break;
        }
    case MSG_ROUTING_MaodvFlushMessageCache:
        {
            MaodvDeleteMsgCache(&maodv->messageCache);
            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_NETWORK_JoinGroup:
        {
            // code goes here
            NodeAddress *mcastAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            MaodvJoinGroup(node, *mcastAddr);
            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_NETWORK_LeaveGroup:
        {
            // code goes here
            NodeAddress *mcastAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            MaodvLeaveGroup(node, *mcastAddr);
            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_ROUTING_MaodvCheckNextHopTimeout:
        {
            NextHopInfo *nextHopInfo = (NextHopInfo *)
                MESSAGE_ReturnInfo(msg);
            NodeAddress nextHopAddr = nextHopInfo->nextHopAddr;
            NodeAddress mcastAddr = nextHopInfo->mcastAddr;
            MaodvNextHopNode *nextHopNode;
            BOOL isValidRt;
            MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
                node,
                mcastAddr,
                &maodv->mrouteTable,
                &isValidRt);

            if (mrtToDest == NULL)
            {
            }
            else
            {
                nextHopNode = MaodvCheckNextHopsList(
                    nextHopAddr,
                    &mrtToDest->nextHops);

                if (nextHopNode == NULL)
                {
                }
                else if (!nextHopNode->isEnabled
                    && (nextHopNode->lifetime <= getSimTime(node)))
                {
                    // nexthop failed
                    BOOL lastEntry = FALSE;

                    MaodvDeleteNextHop(
                        nextHopAddr,
                        &mrtToDest->nextHops,
                        &lastEntry);

                    if (!MaodvLookupGroupMembership(
                        mcastAddr,
                        &maodv->memberFlag))
                    {
                        MaodvCheckForPrune(
                            node,
                            &maodv->mrouteTable,
                            mrtToDest);
                    }
                }
            }
            MESSAGE_Free(node,msg);
            break;
        }
    case MSG_ROUTING_MaodvTreeUtilizationTimer:
        {
            NextHopInfo *nextHopInfo = (NextHopInfo *)
                MESSAGE_ReturnInfo(msg);
            NodeAddress nextHopAddr = nextHopInfo->nextHopAddr;
            NodeAddress mcastAddr = nextHopInfo->mcastAddr;
            BOOL isValidRt;

            MaodvMrouteEntry *mrtToDest = MaodvCheckMrouteExist(
                node,
                mcastAddr,
                &maodv->mrouteTable,
                &isValidRt);

            clocktype lastHeared = MaodvGetLastHearedTime(
                nextHopAddr,
                &maodv->neighborTable);

            if (mrtToDest == NULL || !isValidRt)
            {
                // Corresponding group entry not there so do nothing
                MESSAGE_Free(node,msg);
            }
            else
            {
                MaodvNextHopNode *nextHopNode =
                    MaodvCheckNextHopsList(
                        nextHopAddr,
                        &mrtToDest->nextHops);

                if (nextHopNode == NULL)
                {
                    // No next hop entry in the multicast routing table.
                    // the next hop already expired
                    MESSAGE_Free(node, msg);
                }
                else if (nextHopNode->isEnabled)
                {
                    if (lastHeared + MAODV_HELLO_LIFE <= getSimTime(node))
                    {
                        if (DEBUG_FAILURE)
                        {
                            char addr[MAX_STRING_LENGTH];
                            IO_ConvertIpAddressToString(nextHopAddr, addr);
                            printf("Node: %u, "
                                   "MSG_ROUTING_MaodvTreeUtilizationTimer "
                                   "expired to %s\n",
                                   node->nodeId,
                                   addr);
                        }

                        MaodvProcessNextHopFailure(
                            node,
                            mcastAddr,
                            nextHopAddr);

                        MESSAGE_Free(node, msg);
                    }
                    else
                    {
                        MESSAGE_Send(node, msg, (clocktype) MAODV_HELLO_LIFE);
                    }
                }
                else
                {
                    // Next hop is disabled so don't need to process the timer
                    MESSAGE_Free(node, msg);
                }
            }
            break;
        }
    case MSG_ROUTING_MaodvRetransmitTimer:
        {
            NextHopInfo *nextHopInfo = (NextHopInfo *)
                MESSAGE_ReturnInfo(msg);
            NodeAddress nextHopAddr = nextHopInfo->nextHopAddr;
            NodeAddress mcastAddr = nextHopInfo->mcastAddr;

            BOOL isValidRt;
            MaodvMrouteEntry *mrtToDest =
                MaodvCheckMrouteExist(
                    node,
                    mcastAddr,
                    &maodv->mrouteTable,
                    &isValidRt);

            clocktype lastHeared = MaodvGetLastHearedTime(
                nextHopAddr,
                &maodv->neighborTable);

            if (mrtToDest == NULL || !isValidRt)
            {
            }
            else
            {
                MaodvNextHopNode *nextHopNode = MaodvCheckNextHopsList(
                    nextHopAddr,
                    &mrtToDest->nextHops);

                if (nextHopNode == NULL)
                {
                }
                else
                {
                    if (lastHeared + MAODV_RETRANSMIT_TIME <= getSimTime(node))
                    {
                        if (DEBUG_FAILURE)
                        {
                            char addr[25];
                            IO_ConvertIpAddressToString(nextHopAddr, addr);
                            printf("Node: %u, "
                                   "MSG_ROUTING_MaodvRetransmitTimer "
                                   "expired to %s\n",
                                   node->nodeId,
                                   addr);
                        }
                        MaodvProcessNextHopFailure(
                            node,
                            mcastAddr,
                            nextHopAddr);
                    }
                }
            }
            MESSAGE_Free(node,msg);
            break;
        }
    case MSG_ROUTING_MaodvCheckMroute:
        {
            NextHopInfo *nextHopInfo = (NextHopInfo *)
                MESSAGE_ReturnInfo(msg);
            NodeAddress nextHopAddr = nextHopInfo->nextHopAddr;
            NodeAddress mcastAddr = nextHopInfo->mcastAddr;
            BOOL isValidRt = FALSE;

            MaodvMrouteEntry *mrtToDest =
                MaodvCheckMrouteExist(
                    node,
                    mcastAddr,
                    &maodv->mrouteTable,
                    &isValidRt);

            if (mrtToDest == NULL || !isValidRt)
            {
                MESSAGE_Free(node,msg);
            }
            else
            {
                MaodvNextHopNode *nextHopNode =
                    MaodvCheckNextHopsList(
                        nextHopAddr,
                        &mrtToDest->nextHops);

                if (nextHopNode == NULL)
                {
                    MESSAGE_Free(node,msg);
                }
                else
                {
                    if (!nextHopNode->isEnabled)
                    {
                        MESSAGE_Free(node,msg);
                    }
                    else if (nextHopNode->lifetime <= getSimTime(node))
                    {
                        if (DEBUG_FAILURE)
                        {
                            char addr[25];
                            IO_ConvertIpAddressToString(nextHopAddr, addr);
                            printf("Node: %u, MSG_ROUTING_MaodvCheckMroute "
                                   "expired to %s\n",
                                   node->nodeId,
                                   addr);
                        }
                        MaodvProcessNextHopFailure(
                            node,
                            mcastAddr,
                            nextHopAddr);

                        MESSAGE_Free(node,msg);
                    }
                    else
                    {
                        MESSAGE_Send(
                            node,
                            msg,
                            (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);
                    }
                }
            }
            break;
        }
        // Remove the route that has not been used for awhile
    case MSG_NETWORK_CheckRouteTimeout:
        {
            NodeAddress *destAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            BOOL isValidRt = FALSE;
            MaodvRouteEntry *rtPtr = MaodvCheckRouteExist(node, *destAddr,
                &maodv->routeTable, &isValidRt);

            if (isValidRt && (rtPtr->lifetime <= getSimTime(node)))
            {
                // Disable the route and then delete it after delete period
                MaodvDisableRoute(node, *destAddr, &maodv->routeTable);

                // flag in the route table MUST be reset when the route times
                // out (i.e., after the route has been not been active for
                // ACTIVE_ROUTE_TIMEOUT). Sec: 8.12
            }

            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_NETWORK_DeleteRoute:
        {
            NodeAddress *destAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);

            MaodvDeleteRouteTable(node, *destAddr, &maodv->routeTable);
            MESSAGE_Free(node, msg);
            break;
        }
    case MSG_ROUTING_MaodvDeleteMroute:
        {
            NodeAddress *mcastAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            MaodvDeleteMroute(node, *mcastAddr, &maodv->mrouteTable);
            MESSAGE_Free(node, msg);
            break;
        }
    // Check if RREP is received after sending RREQ
    case MSG_NETWORK_CheckReplied:
        {
            NodeAddress *destAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            MaodvRreqSentNode *sentNode = NULL;
            BOOL isValidRt = FALSE;
            MaodvRouteEntry *rtEntry = NULL;
            MaodvMrouteEntry *mrtEntry = NULL;
            MaodvNextHopNode *upstreamNode = NULL;
            BOOL isValidUcastRt = FALSE;
            sentNode = MaodvCheckSent(*destAddr, &maodv->sent);

            // A route exists for the multicast destination.
            if (!sentNode)
            {
                // Already got the route
                MESSAGE_Free(node, msg);
                break;
            }

            if (sentNode->type == NONJOIN)
            {
                // MaodvPrintRoutingTable(node, &maodv->routeTable);

                // In case of Non-Join request multicast group entry should
                // be stored in the routing table instead of Multicast routing
                // table
                rtEntry = MaodvCheckRouteExist(
                    node,
                    *destAddr,
                    &maodv->routeTable,
                    &isValidUcastRt);
            }
            else
            {
                mrtEntry = MaodvCheckMrouteExist(
                    node,
                    *destAddr,
                    &maodv->mrouteTable,
                    &isValidRt);
            }

            if (mrtEntry)
            {
                upstreamNode = MaodvFindUpstreamNextHop(&mrtEntry->nextHops);
            }

            if (((sentNode->type == NONJOIN) && !isValidUcastRt) ||
                ((sentNode->type != NONJOIN) && (upstreamNode == NULL)))
            {
                // No upstream node has been found so no route exists for the
                // group. Retry request if not
                if (sentNode->times < maodv->rreqRetries)
                {
                    // MAX_RETRY not reached so retry RREQ with increased ttl

                    if (sentNode->type == COMBINE)
                    {
                        sentNode->type = REPAIR;
                    }

                    MaodvInitiateRREQ(
                        node,
                        *destAddr,
                        sentNode->type);
                }
                else
                {
                    Message *messageToDelete = NULL;
                    NodeAddress previousHop;

                    if (sentNode->type == NONJOIN)
                    {
                        // Remove all the messages destined to the multicast
                        // group and drop the session
                        while ((messageToDelete =
                            MaodvGetBufferedPacket(
                                *destAddr,
                                &previousHop,
                                &maodv->msgBuffer)))
                        {
                            maodv->stats.numDataDroppedForNoRoute++;
                            MaodvDeleteBuffer(*destAddr, &maodv->msgBuffer);
                            MESSAGE_Free(node, messageToDelete);
                        }
                    }
                    else if (sentNode->type == JOIN)
                    {
                        // No reply received for JOIN request
                        // Declare leader and start sending group hello msg
                        MaodvDeclareAsLeader(node, *destAddr);
                    }
                    else if (sentNode->type == REPAIR)
                    {
                        MaodvHandleTreePartition(node, *destAddr);
                    }

                    // Remove from sent table.
                    MaodvDeleteSent(*destAddr, &maodv->sent);
                }
            }
            else
            {
                // ROUTE has been obtained
                // Remove from sent table and validate for multicast join
                // or repair requests
                if (sentNode->type == NONJOIN)
                {
                    MaodvDeleteSent(*destAddr, &maodv->sent);
                }
                else if (sentNode->type == COMBINE)
                {
                    MaodvDeleteSent(*destAddr, &maodv->sent);
                }
                else
                {
                    // Join or Repair request
                    // activate the route
                    mrtEntry->isMember = TRUE;
                    // enable the upstream
                    upstreamNode->isEnabled = TRUE;

                    MaodvSetMulticastTimer(
                        node,
                        MSG_ROUTING_MaodvTreeUtilizationTimer,
                        mrtEntry->destination.address,
                        upstreamNode->nextHopAddr,
                        (clocktype) MAODV_HELLO_LIFE);

                    upstreamNode->lifetime =
                        getSimTime(node) + MAODV_MDATA_FORWARD_TIMEOUT;

                    MaodvSetMulticastTimer(
                        node,
                        MSG_ROUTING_MaodvCheckMroute,
                        mrtEntry->destination.address,
                        upstreamNode->nextHopAddr,
                        (clocktype) MAODV_MDATA_FORWARD_TIMEOUT);

                    if (DEBUG_FAILURE)
                    {
                        printf("Node %u: Sending MACT to graft the tree\n",
                            node->nodeId);
                    }

                    // send MACT to graft the tree
                    MaodvSendMact(
                        node,
                        mrtEntry->destination.address,
                        upstreamNode->nextHopAddr,
                        upstreamNode->interface,
                        J,
                        0,
                        mrtEntry->destination.seqNum,
                        0);

                    while (MaodvLookupBuffer(
                        mrtEntry->destination.address,
                        &maodv->msgBuffer))
                    {
                        NodeAddress previousHop;

                        Message *newMsg = MaodvGetBufferedPacket(
                            mrtEntry->destination.address,
                            &previousHop,
                            &maodv->msgBuffer);

                        MaodvTransmitMdata(
                            node,
                            newMsg,
                            mrtEntry,
                            previousHop,
                            0); // incoming interface

                        MaodvDeleteBuffer(
                            mrtEntry->destination.address,
                            &maodv->msgBuffer);

                    } // while

                    if (sentNode->type == REPAIR)
                    {
                        MaodvSendMact(
                            node,
                            mrtEntry->destination.address,
                            ANY_DEST,
                            DEFAULT_INTERFACE,
                            U,
                            mrtEntry->hopCountLeader,
                            mrtEntry->destination.seqNum,
                            mrtEntry->mcastLeader);
                    }
                    MaodvDeleteSent(
                        mrtEntry->destination.address,
                        &maodv->sent);
                }
            }

            MESSAGE_Free(node, msg);
            break;
      }
    case MSG_NETWORK_SendHello:
        {
            clocktype delay = (clocktype)
                (RANDOM_erand(maodv->broadcastJitterSeed) * MAODV_BROADCAST_JITTER);

            if (maodv->lastBroadcastSent
                <= (getSimTime(node) - MAODV_HELLO_INTERVAL)
                || maodv->lastBroadcastSent == 0)
            {
                MaodvBroadcastHelloMessage(node, maodv, ANY_DEST);
                maodv->lastBroadcastSent = getSimTime(node);
            }

            MESSAGE_Send(node, msg, (clocktype) (MAODV_HELLO_INTERVAL + delay));
            break;
        }
    case MSG_ROUTING_MaodvSendGroupHello:
        {
            clocktype delay = (clocktype) (RANDOM_erand(maodv->broadcastJitterSeed) *
                MAODV_BROADCAST_JITTER);

            NodeAddress *mcastAddr = (NodeAddress *) MESSAGE_ReturnInfo(msg);
            MaodvMrouteEntry *mrtToDest = NULL;
            BOOL isValidRoute = FALSE;

            if (MaodvLookupLeadership(*mcastAddr, &maodv->memberFlag))
            {
                MAODV_M_Node *memberInfo = MaodvLookupMemberFlag(
                    *mcastAddr,
                    &maodv->memberFlag);

                if (memberInfo == NULL)
                {
                    ERROR_Assert(FALSE, "\n member info null not possible");
                }

                mrtToDest = MaodvCheckMrouteExist(
                    node,
                    *mcastAddr,
                    &maodv->mrouteTable,
                    &isValidRoute);

                if (!isValidRoute)
                {
                    ERROR_Assert(FALSE, "Error in group hello ..\n");
                }

                mrtToDest->destination.seqNum++;

                MaodvBroadcastGroupHelloMessage(
                    node,
                    *mcastAddr,
                    mrtToDest->destination.seqNum,
                    NetworkIpGetInterfaceAddress(node, 0),
                    memberInfo->isUpdate,
                    maodv,
                    mrtToDest->hopCountLeader);

                memberInfo->isUpdate = FALSE;
                maodv->lastBroadcastSent = getSimTime(node);

                MESSAGE_Send(
                    node,
                    msg,
                    (clocktype) (MAODV_GROUP_HELLO_INTERVAL + delay));
            }
            else
            {
                // not the leader anymore, discard the timer
                MESSAGE_Free(node, msg);
            }
            break;
        }
    default:
        ERROR_Assert(FALSE, "Maodv: Unknown MSG type!\n");
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvRouterFunction
//
// PURPOSE:  To route packet
//
// ARGUMENTS: node, The node which has received data
//            msg, The message containing the packet
//            destAddr, Destination address
//            interfaceIndex, Interface index
//            packetWasRouted, Packet routed flag
//            previousHopAddress, Previous Hop address
//
// RETURN:    None
//---------------------------------------------------------------------------
void MaodvRouterFunction(
    Node *node,
    Message *msg,
    NodeAddress destAddr,
    int interfaceIndex,
    BOOL *packetWasRouted,
    NodeAddress previousHopAddress)
{
    MaodvData *maodv = (MaodvData *) NetworkIpGetMulticastRoutingProtocol(
        node,
        MULTICAST_PROTOCOL_MAODV);

    IpHeaderType *ipHeader = (IpHeaderType *) MESSAGE_ReturnPacket(msg);
    MaodvMrouteEntry *mrtToDest = NULL;
    BOOL isValidRt = FALSE;
    BOOL isMember = FALSE;
    BOOL isSource = FALSE;

    MaodvIpOptionType option;

    NodeAddress srcId;
    unsigned int pktId;

    if (DEBUG_PACKET)
    {
        char time[100];
        char dest[25];
        char prevHop[25];
        TIME_PrintClockInSecond(getSimTime(node), time);
        IO_ConvertIpAddressToString(destAddr, dest);
        IO_ConvertIpAddressToString(previousHopAddress, prevHop);

        printf("\nRouter Function: Node %u\n"
            "Dest Addr %s, Previous Hop %s\n"
            "Time %s",
            node->nodeId,
            dest,
            prevHop,
            time);
    }

    // no need for IP to process packet forwarding
    *packetWasRouted = TRUE;

    if (previousHopAddress != ANY_IP)
    {
        // Update neighbor table with previos hop only unless
        // this is the source node.
        MaodvUpdateInsertNeighbor(
            node,
            previousHopAddress,
            &maodv->neighborTable);
    }

    if (ipHeader->ip_p == IPPROTO_MAODV)
    {
        if (DEBUG_PACKET)
        {
            printf("\tThis is a MAODV control packet\n");
        }

        // This is a MAODV control packet. Process the packet.
        MaodvHandleProtocolPacket(
            node,
            msg,
            ipHeader->ip_src,
            ipHeader->ip_dst,
            ipHeader->ip_ttl,
            interfaceIndex);

        return;
    }

    if (DEBUG_PACKET)
    {
        printf("\tThis is a data packet\n");
    }

    // This is a data packet
    if (FindAnIpOptionField(ipHeader, IPOPT_MAODV) == NULL)
    {
        // Possible source of the packet. For any other nodes there should
        // be an option field added with the IP header.
        // The source node should not have a option header already
        // added in it.

        pktId = maodv->pktId;
        srcId = ipHeader->ip_src;

        if (DEBUG_PACKET)
        {
            printf("\tSource of the data packet, adding IP option\n");
        }

        // Source should add the IP option field. This field is needed for
        // duplicate data packet detection.

        AddCustomMaodvIpOptionFields(node, msg);
        option.pktId = maodv->pktId++;
        option.srcId = ipHeader->ip_src;

        SetMaodvIpOptionField(msg, &option);
        isSource = TRUE;
    }
    else
    {
        option = GetMaodvIpOptionField(msg);

        pktId = option.pktId;
        srcId = option.srcId;

        if (DEBUG_PACKET)
        {
            char src[25];
            IO_ConvertIpAddressToString(srcId, src);
            printf("\tReceived packet with Source %s, Pkt Id %u\n",
                src,
                pktId);
        }

        if (NetworkIpIsMyIP(node, srcId))
        {
            // Should be a duplicate packet as the option header is there
            // and the source id is same to this node's id

            // So drop the packet

            if (DEBUG_PACKET)
            {
                printf("\tOwn duplicate packet!!\n");
            }

            MESSAGE_Free(node, msg);
            return;
        }
        else
        {
            if (DEBUG_PACKET)
            {
                printf("\tReceived packet to forward\n");
            }
            isSource = FALSE;
        }
    }

    // update the MCAST_TIMER whenever it receives a packet from the nexthop
    isMember = MaodvLookupGroupMembership(destAddr, &maodv->memberFlag);

    mrtToDest = MaodvCheckMrouteExist(
        node,
        destAddr,
        &maodv->mrouteTable,
        &isValidRt);

    if (!isSource)
    {
        if (mrtToDest != NULL && isValidRt)
        {
            // Has a valid route to the destination.
            if (mrtToDest->isMember)
            {
                // This is a member of the multicast group. If the neighbor
                // exists in its neighbor table update the lifetime of the
                // neighbor.
                MaodvNextHopNode *prevHop =  MaodvCheckNextHopsList(
                    previousHopAddress,
                    &mrtToDest->nextHops);

                if (prevHop != NULL)
                {
                    if (prevHop->isEnabled)
                    {
                        prevHop->lifetime =
                            MAX(prevHop->lifetime,
                                getSimTime(node) + MAODV_MDATA_FORWARD_TIMEOUT);
                    }
                }
            }
        }
    }

    // Check the packet for duplicity by its source address and packet id.
    // If this is a duplicate packet then there is no need to process the
    // packet further. So drop the packet.

    if (!(MaodvLookupMessageCache(srcId, &maodv->messageCache, pktId)))
    {
        // The packet is not a duplicate so process the packet. Insert the
        // packet into the message cache for further checking of duplicity

        MaodvInsertMessageCache(node, srcId,  &maodv->messageCache, pktId);
        isMember =  MaodvLookupGroupMembership(destAddr, &maodv->memberFlag);

        if (DEBUG_PACKET)
        {
            printf("\tThis is not a duplicate packet\n");
        }

        if (!isSource)
        {
            // Intermediate node or destination of the route

            if (DEBUG_PACKET)
            {
                printf("\tIntermediate node processing data packet\n");
            }

            MaodvHandleData(
                node,
                msg,
                srcId,
                destAddr,
                previousHopAddress,
                interfaceIndex);
        }
        else if (mrtToDest != NULL && isValidRt)
        {
            // This is the source of the packet and has a valid route for the
            // multicast group.

            if (DEBUG_PACKET)
            {
                printf("\tSource of the packet processing the packet\n");
            }

            if (!mrtToDest->isMember)
            {
                // Source is not an active tree node still
                // Join/Repair request has been sent. Buffer the packet..
                MaodvInsertBuffer(
                    node,
                    msg,
                    destAddr,
                    previousHopAddress,
                    &maodv->msgBuffer);

                if (DEBUG_PACKET)
                {
                    printf("\tSource: sent request to be a tree member\n");
                    printf("\t\tBuffer the packet\n");
                }

            }
            else
            {
                // source has a valid route to the multicast destination

                if (DEBUG_PACKET)
                {
                    printf("\tSource: a tree member\n");
                    printf("\t\tSend the packet\n");
                }

                MaodvTransmitMdata(
                    node,
                    msg,
                    mrtToDest,
                    previousHopAddress,
                    interfaceIndex);
            }
        }
        else
        {
            // Not a multicast group member or tree member but source of the
            // data to multicast group

            MaodvRouteEntry *rtToDest = NULL;

            // check whether unicast path exists to the multicast group
            // Why is is searching the Unicast table. Multicast group addresses
            // should be stored in multicast routing table.

            if (DEBUG_PACKET)
            {
                printf("\tSource:not a tree member, neither sent a "
                    "JOIN request\n");
            }

            rtToDest = MaodvCheckRouteExist(
                node,
                destAddr,
                &maodv->routeTable,
                &isValidRt);

            if (isValidRt && rtToDest != NULL)
            {
                // valid route to the multicast group exists
                // transmit data

                if (DEBUG_PACKET)
                {
                    printf("\tSource: Has a valid unicast route to group\n");
                    printf("\t\tSend the packet\n");
                }

                MaodvTransmitData(node, msg, rtToDest, previousHopAddress);
                maodv->stats.numMdataInitiated++;
            }
            else if ((!isValidRt || rtToDest == NULL)
                && MaodvCheckSent(destAddr, &maodv->sent))
            {
                // RREQ sent buffer the packet...
                if (DEBUG_PACKET)
                {
                    printf("\tSource: send a requst for unicast route to "
                        "group\n");
                    printf("\t\tBuffer the packet\n");
                }

                MaodvInsertBuffer(
                    node,
                    msg,
                    destAddr,
                    previousHopAddress,
                    &maodv->msgBuffer);
            }
            else if (!MaodvCheckSent(destAddr, &maodv->sent))
            {
                // Increase sequence number and Initiate RREQ
                MaodvInsertBuffer(
                    node,
                    msg,
                    destAddr,
                    previousHopAddress,
                    &maodv->msgBuffer);

                if (DEBUG_PACKET)
                {
                    printf("\tSource: Initiating RREQ\n");
                    printf("\t\tBuffer the packet\n");
                }

                // Initiate a non-join request
                MaodvInitiateRREQ(node, destAddr, NONJOIN);
            }
        }
    }
    else
    {
        // received a duplicate packet, silently discard it
        if (DEBUG_PACKET)
        {
            printf("\tThis is a duplicate data packet, so dropping\n");
        }

        maodv->stats.numDupDataRecd++;
        MESSAGE_Free(node, msg);
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvInit
//
// PURPOSE:  Initialization function for MAODV protocol
//
// ARGUMENTS: node, Aodv router which is initializing itself
//            aodvPtr, data space to store aodv informations
//            routerInfo, struture to store all the routing informations of
//                        network layer
//            nodeInput,  The configuration file
//
// RETURN:    None
//---------------------------------------------------------------------------

void MaodvInit(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex)
{
    MaodvData *maodv = (MaodvData *) MEM_malloc(sizeof(MaodvData));
    BOOL retVal;
    char buf[MAX_STRING_LENGTH];
    NodeInput memberInput;
    int i;

    char joinTimeStr[MAX_STRING_LENGTH];
    char leaveTimeStr[MAX_STRING_LENGTH];
    clocktype joinTime;
    clocktype leaveTime;
    NodeAddress srcAddr;
    NodeAddress mcastAddr;

    char mcastString[MAX_STRING_LENGTH];
    BOOL isNodeId;
    int numSubnetBits;

    if (maodv == NULL)
    {
        ERROR_ReportError("MAODV: Cannot alloc memory for MAODV struct!\n");
    }

    RANDOM_SetSeed(maodv->broadcastJitterSeed,
                   node->globalSeed,
                   node->nodeId,
                   MULTICAST_PROTOCOL_MAODV,
                   interfaceIndex);
    RANDOM_SetSeed(maodv->helloIntervalSeed,
                   node->globalSeed,
                   node->nodeId,
                   MULTICAST_PROTOCOL_MAODV,
                   interfaceIndex + 1);
    RANDOM_SetSeed(maodv->jitterSeed,
                   node->globalSeed,
                   node->nodeId,
                   MULTICAST_PROTOCOL_MAODV,
                   interfaceIndex + 2);

    NetworkIpSetMulticastRoutingProtocol(node, maodv, interfaceIndex);

    NetworkIpSetMulticastRouterFunction(
        node,
        (MulticastRouterFunctionType)MaodvRouterFunction,
        interfaceIndex);

    // Read whether statistics needs to be collected for the protocol
    IO_ReadString(
        node->nodeId,
        ANY_ADDRESS,
        nodeInput,
        "ROUTING-STATISTICS",
        &retVal,
        buf);

    if ((retVal == FALSE) || (strcmp(buf, "NO") == 0))
    {
        maodv->statsCollected = FALSE;
    }
    else if (strcmp(buf, "YES") == 0)
    {
        maodv->statsCollected = TRUE;
    }
    else
    {
        ERROR_ReportError("Needs YES/NO against STATISTICS");
    }

    // Read user configurable parameters from the cofiguration file or
    // initialize them with the default value.
    MaodvInitializeConfigurableParameters(node, nodeInput,interfaceIndex,
        maodv);

    // Initialize statistical variables
    MaodvInitStats(maodv);
    maodv->statsPrinted = FALSE;
    maodv->pktId = 0;

    // Initialize aodv routing table
    MaodvInitRouteTable(&maodv->routeTable);
    MaodvInitMrouteTable(&maodv->mrouteTable);
    MaodvInitNeighborTable(&maodv->neighborTable);
    MaodvInitRequestTable(&maodv->requestTable);
    MaodvInitMessageCache(&maodv->messageCache);
    MaodvInitMembership(&maodv->memberFlag);

    // Initialize aodv structure to store RREQ information
    MaodvInitSeenTable(&maodv->seenTable);

    // Initialize buffer to store packets which don't have any route
    MaodvInitBuffer(&maodv->msgBuffer);

    // Initialize buffer to store information about the destinations for
    // which RREQ has been sent
    MaodvInitSent(&maodv->sent);

    // Initialize Aodv sequence number
    MaodvInitSeq(maodv);

    // Initialize Aodv Broadcast id
    MaodvInitFloodingId(maodv);

    NetworkIpSetMacLayerStatusEventHandlerFunction(
        node, &MaodvMacLayerStatusHandler, interfaceIndex);
    // Initialize Last Broadcast sent
    maodv->lastBroadcastSent = (clocktype) 0;

    MaodvSetTimer(
        node,
        NULL,
        MSG_NETWORK_SendHello,
        ANY_IP,
        (clocktype) (MAODV_HELLO_INTERVAL * RANDOM_erand(maodv->helloIntervalSeed)));

    IO_ReadCachedFile(
        ANY_NODEID,
        NetworkIpGetInterfaceAddress(node,interfaceIndex),
        nodeInput,
        "MULTICAST-GROUP-FILE",
        &retVal,
        &memberInput);

    if (retVal == FALSE)
    {
        ERROR_ReportError("MAODV: Needs MULTICAST-GROUP-FILE.\n");
    }

    for (i = 0; i < memberInput.numLines; i++)
    {
        retVal = sscanf(memberInput.inputStrings[i],"%d %s %s %s",
            &srcAddr,
            mcastString,
            joinTimeStr,
            leaveTimeStr);

        if (retVal != 4)
        {
            ERROR_ReportError("Application: Wrong configuration format!\n");
        }

        IO_ParseNodeIdHostOrNetworkAddress(
            mcastString,
            &mcastAddr,
            &numSubnetBits,
            &isNodeId);

        joinTime = TIME_ConvertToClock(joinTimeStr);
        leaveTime = TIME_ConvertToClock(leaveTimeStr);

        if (node->nodeId == srcAddr)
        {
            MaodvSetTimer(
                node,
                NULL,
                MSG_NETWORK_JoinGroup,
                mcastAddr,
                joinTime);

            if (leaveTime)
            {
                MaodvSetTimer(
                    node,
                    NULL,
                    MSG_NETWORK_LeaveGroup,
                    mcastAddr,
                    leaveTime);
            }
        }
    }
}

//---------------------------------------------------------------------------
// FUNCTION: MaodvFinalize
//
// PURPOSE:  Called at the end of the simulation to collect the results
//
// ARGUMENTS: node, The node for which the statistics are to be printed
//
// RETURN:    None
//---------------------------------------------------------------------------

void MaodvFinalize(Node *node)
{
    MaodvData *maodv = (MaodvData *)
        NetworkIpGetMulticastRoutingProtocol(
            node,
            MULTICAST_PROTOCOL_MAODV);

    char buf[MAX_STRING_LENGTH];

    if (maodv->statsCollected)
    {
        if (maodv->statsPrinted)
        {
            return;
        }
        else
        {
            maodv->statsPrinted = TRUE;
        }

        sprintf(buf, "Number of Join RREQ Initiated = %d",
            maodv->stats.numJoinReqInitiated);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Repair RREQ Initiated = %d",
            maodv->stats.numRepairReqInitiated);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Non-join RREQ Initiated = %d",
            maodv->stats.numNonJoinReqInitiated);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Join RREQ Retried = %d",
            maodv->stats.numJoinRequestResent);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Non-Join RREQ Retried = %d",
            maodv->stats.numNonJoinRequestResent);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Join RREQ Forwarded = %d",
            maodv->stats.numJoinRequestRelayed);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Non-Join RREQ Forwarded = %d",
            maodv->stats.numNonJoinRequestRelayed);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Join RREQ received = %d",
            maodv->stats.numJoinReqRecd);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Repair RREQ received = %d",
            maodv->stats.numRepairReqRecd);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of NonJoin RREQ received = %d",
            maodv->stats.numNonJoinReqRecd);
        IO_PrintStat(node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);



        sprintf(buf, "Number of Duplicate RREQ received = %d",
            maodv->stats.numRequestDuplicate);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number RREQ ttl expired = %d",
            maodv->stats.numRequestTtlExpired);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number RREQ received by Dest = %d",
            maodv->stats.numRequestRecvedAsDest);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of RREP Initiated as Dest = %d",
            maodv->stats.numReplyInitiatedAsDest);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of RREP Initiated as intermediate node = %d",
            maodv->stats.numReplyInitiatedAsIntermediate);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Multicast RREP Forwarded = %d",
            maodv->stats.numMreplyForwarded);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of RREP Received = %d",
            maodv->stats.numReplyRecved);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of RREP Received as Source = %d",
            maodv->stats.numReplyRecvedAsSource);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Hello message sent = %d",
            maodv->stats.numHelloSent);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Hello message received = %d",
            maodv->stats.numHelloRecved);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);


        sprintf(buf, "Number of Multicast Data packets sent as Source = %d",
            maodv->stats.numMdataInitiated);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Multicast Data packets "
            "dropped as Source = %d",
            maodv->stats.numMdataDroppedAsSource);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Multicast Data packets delivered "
            "to the node = %d",
            maodv->stats.numMdataDelivered);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Multicast Data packets Forwarded "
            "as a tree member = %d",
            maodv->stats.numMdataForwarded);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Multicast Data packets Dropped = %d",
            maodv->stats.numMdataDropped);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);


        sprintf(buf, "Number of MACT sent = %d",
            maodv->stats.numMactSent);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);


        sprintf(buf, "Number of Data Packets Dropped for no route = %d",
            maodv->stats.numDataDroppedForNoRoute);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);

        sprintf(buf, "Number of Data Packets Dropped for buffer "
            "overflow = %d",
            maodv->stats.numDataDroppedForOverlimit);
        IO_PrintStat(
            node,
            "Network",
            "MAODV",
            ANY_DEST,
            -1 /* instance Id */,
            buf);
    }
}

// MAODV END
