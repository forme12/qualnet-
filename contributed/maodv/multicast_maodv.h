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


//
// Name: maodv.h
// Purpose: To simulate Multicast AODV
// Author: Venkatesh Rajendran (venkat@cse.ucsc.edu)
// Univeristy of California, Santa Cruz.
//


#ifndef _MAODV_H_
#define _MAODV_H_

// /**
// CONSTANT    :: MAODV_MAX_GROUP  :  10
// DESCRIPTION :: Maximum number of multicast groups in simulation.
// **/
#define MAODV_MAX_GROUP                   10

// /**
// CONSTANT    :: MAODV_MAX_SOURCE  :  20
// DESCRIPTION :: Maximum number of sources for each multicast group.
// **/
#define MAODV_MAX_SOURCE                  20

// Aodv default timer and constant values ref:
// draft-ietf-manet-aodv-08.txt section: 12

// These default values are user configurable. If not configured by the
// user the protocol will run with these specified default values.
// /**
// CONSTANT    :: MAODV_HASH_BLOCK  :  1024
// DESCRIPTION :: Default hash block size
// **/
#define MAODV_HASH_BLOCK                    1024

// /**
// CONSTANT    :: MAODV_MDATA_FORWARD_TIMEOUT  :  (5000 * MILLI_SECOND)
// DESCRIPTION :: Mulicast data forward timeout
// **/
#define MAODV_MDATA_FORWARD_TIMEOUT         (5000 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_ACTIVE_ROUTE_TIMEOUT:  (3000 * MILLI_SECOND)
// DESCRIPTION :: Default active route tiemout
// **/
#define MAODV_DEFAULT_ACTIVE_ROUTE_TIMEOUT  (3000 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_ALLOWED_HELLO_LOSS:  (2)
// DESCRIPTION :: Default allowed hello loss
// **/
#define MAODV_DEFAULT_ALLOWED_HELLO_LOSS    (2)

// /**
// CONSTANT    :: MAODV_DEFAULT_HELLO_INTERVAL:  (1000 * MILLI_SECOND)
// DESCRIPTION :: Default hello interval
// **/
#define MAODV_DEFAULT_HELLO_INTERVAL        (1000 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_NET_DIAMETER:  (35)
// DESCRIPTION :: Default net diameter
// **/
#define MAODV_DEFAULT_NET_DIAMETER          (35)

// /**
// CONSTANT    :: MAODV_DEFAULT_NODE_TRAVERSAL_TIME:  (40 * MILLI_SECOND)
// DESCRIPTION :: Default node traversal time
// **/
#define MAODV_DEFAULT_NODE_TRAVERSAL_TIME   (40 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_RREQ_RETRIES:  (2)
// DESCRIPTION :: Default rotue request time
// **/
#define MAODV_DEFAULT_RREQ_RETRIES          (2)

// /**
// CONSTANT    :: MAODV_DEFAULT_ROUTE_DELETE_CONST:  (5)
// DESCRIPTION :: Default rotue delete time
// **/
#define MAODV_DEFAULT_ROUTE_DELETE_CONST    (5)

// /**
// CONSTANT    :: MAODV_DEFAULT_MESSAGE_BUFFER_IN_PKT:  (100)
// DESCRIPTION :: Default packet in message buffer
// **/
#define MAODV_DEFAULT_MESSAGE_BUFFER_IN_PKT (100)

// /**
// CONSTANT    :: MAODV_DEFAULT_GROUP_HELLO_INTERVAL:  (5000 * MILLI_SECOND)
// DESCRIPTION :: Default group hello interval
// **/
#define MAODV_DEFAULT_GROUP_HELLO_INTERVAL  (5000 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_PRUNE_TIMEOUT:(MAODV_DEFAULT_ACTIVE_ROUTE_TIMEOUT)
// DESCRIPTION :: Default prune timeout
// **/
#define MAODV_DEFAULT_PRUNE_TIMEOUT         (MAODV_DEFAULT_ACTIVE_ROUTE_TIMEOUT)

// /**
// MACRO       :: MAODV_ACTIVE_ROUTE_TIMEOUT:(maodv->activeRouteTimeout)
// DESCRIPTION :: Default route timeout
// **/
#define MAODV_ACTIVE_ROUTE_TIMEOUT          (maodv->activeRouteTimeout)

// /**
// CONSTANT    :: MAODV_RETRANSMIT_TIME:(750 * MILLI_SECOND)
// DESCRIPTION :: Default retransmit timeout
// **/
#define MAODV_RETRANSMIT_TIME               (750 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_DEFAULT_MY_ROUTE_TIMEOUT:(2 * MAODV_ACTIVE_ROUTE_TIMEOUT)
// DESCRIPTION :: Default route timeout
// **/
#define MAODV_DEFAULT_MY_ROUTE_TIMEOUT      (2 * MAODV_ACTIVE_ROUTE_TIMEOUT)

// /**
// MACRO       :: MAODV_ALLOWED_HELLO_LOSS:(maodv->allowedHelloLoss)
// DESCRIPTION :: Hello loss
// **/
#define MAODV_ALLOWED_HELLO_LOSS            (maodv->allowedHelloLoss)

// /**
// MACRO       :: MAODV_HELLO_INTERVAL:(maodv->helloInterval)
// DESCRIPTION :: Hello interval
// **/
#define MAODV_HELLO_INTERVAL                (maodv->helloInterval)

// /**
// MACRO       :: MAODV_HELLO_INTERVAL:(maodv->netDiameter)
// DESCRIPTION :: Net Diameters
// **/
#define MAODV_NET_DIAMETER                  (maodv->netDiameter)

// /**
// MACRO       :: MAODV_NODE_TRAVERSAL_TIME:(maodv->nodeTraversalTime)
// DESCRIPTION :: Node traversal time
// **/
#define MAODV_NODE_TRAVERSAL_TIME           (maodv->nodeTraversalTime)

// /**
// MACRO       :: MAODV_RREQ_RETRIES:(maodv->rreqRetries)
// DESCRIPTION :: Route request retry
// **/
#define MAODV_RREQ_RETRIES                  (maodv->rreqRetries)

// /**
// MACRO       :: MAODV_ROUTE_DELETE_CONST:(maodv->rtDeletionConstant)
// DESCRIPTION :: Route delete
// **/
#define MAODV_ROUTE_DELETE_CONST            (maodv->rtDeletionConstant)

// /**
// MACRO       :: MAODV_MY_ROUTE_TIMEOUT:(maodv->myRouteTimeout)
// DESCRIPTION :: My Route timeout
// **/
#define MAODV_MY_ROUTE_TIMEOUT              (maodv->myRouteTimeout)

// /**
// CONSTANT    :: MAODV_NEXT_HOP_WAIT:(NODE_TRAVERSAL_TIME + 10))
// DESCRIPTION :: Next hop wait time
// **/
#define MAODV_NEXT_HOP_WAIT                 (NODE_TRAVERSAL_TIME + 10)

// /**
// CONSTANT    :: MAODV_NET_TRAVERSAL_TIME:(3 * MAODV_NODE_TRAVERSAL_TIME *
//                                          MAODV_NET_DIAMETER / 2)
// DESCRIPTION :: Net traversal time
// **/
#define MAODV_NET_TRAVERSAL_TIME            (3 * MAODV_NODE_TRAVERSAL_TIME * \
                                             MAODV_NET_DIAMETER / 2)

// /**
// CONSTANT    :: MAODV_NET_TRAVERSAL_TIME:(MAODV_RREQ_RETRIES *
//                                          MAODV_NET_TRAVERSAL_TIME)
// DESCRIPTION :: Black list timeout
// **/
#define MAODV_BLACKLIST_TIMEOUT             (MAODV_RREQ_RETRIES * \
                                             MAODV_NET_TRAVERSAL_TIME)

// /**
// CONSTANT    :: MAODV_FLOOD_RECORD_TIME:(2 * MAODV_NET_TRAVERSAL_TIME)
// DESCRIPTION :: flood record timeout
// **/
#define MAODV_FLOOD_RECORD_TIME             (2 * MAODV_NET_TRAVERSAL_TIME)

// /**
// CONSTANT    :: MAODV_DELETE_PERIOD:
// DESCRIPTION :: route delete period
// **/
#define MAODV_DELETE_PERIOD                 (MAODV_ROUTE_DELETE_CONST * MAX(\
                                             MAODV_ACTIVE_ROUTE_TIMEOUT, \
                                             MAODV_ALLOWED_HELLO_LOSS \
                                             * MAODV_HELLO_INTERVAL))

// /**
// CONSTANT    :: MAODV_LOCAL_ADD_TTL:      (2)
// DESCRIPTION :: local add to ttl
// **/
#define MAODV_LOCAL_ADD_TTL                 (2)

// /**
// CONSTANT    :: MAODV_MAX_REPAIR_TTL:      (0.3 * MAODV_NET_DIAMETER)
// DESCRIPTION :: maximum repair ttl
// **/
#define MAODV_MAX_REPAIR_TTL                (0.3 * MAODV_NET_DIAMETER)

// /**
// CONSTANT    :: MAODV_MIN_REPAIR_TTL:      (lastHopCountToDest)
// DESCRIPTION :: minimum repair ttl
// **/
#define MAODV_MIN_REPAIR_TTL                lastHopCountToDest

// /**
// CONSTANT    :: MAODV_REV_ROUTE_LIFE:      (MAODV_NET_TRAVERSAL_TIME)
// DESCRIPTION :: reverse route life time
// **/
#define MAODV_REV_ROUTE_LIFE                (MAODV_NET_TRAVERSAL_TIME)

// /**
// CONSTANT    :: MAODV_TTL_START:      (1)
// DESCRIPTION :: TTL start
// **/
#define MAODV_TTL_START                     (1)

// /**
// CONSTANT    :: MAODV_TTL_INCREMENT:      (2)
// DESCRIPTION :: TTL increment
// **/
#define MAODV_TTL_INCREMENT                 (2)

// /**
// CONSTANT    :: MAODV_TTL_THRESHOLD:      (7)
// DESCRIPTION :: TTL thresold
// **/
#define MAODV_TTL_THRESHOLD                 (7)

// /**
// CONSTANT    :: MAODV_INFINITY:      (-1)
// DESCRIPTION :: Infinity
// **/
#define MAODV_INFINITY                      -1

// /**
// CONSTANT    :: MAODV_BROADCAST_JITTER:   (10 * MILLI_SECOND)
// DESCRIPTION :: Broadcast jitter
// **/
#define MAODV_BROADCAST_JITTER              (10 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_JITTER:   (10 * MILLI_SECOND)
// DESCRIPTION :: MAODV jitter
// **/
#define MAODV_JITTER                        (10 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_FLUSH_INTERVAL:   (1 * MINUTE)
// DESCRIPTION :: Flush interval
// **/
#define MAODV_FLUSH_INTERVAL                (1 * MINUTE)

// /**
// CONSTANT    :: MAODV_UNKNOWN_LEADER:   0
// DESCRIPTION :: Unknown leader
// **/
#define MAODV_UNKNOWN_LEADER                0

// /**
// CONSTANT    :: MAODV_HELLO_LIFE:   (3 * SECOND)
// DESCRIPTION :: Hello life
// **/
#define MAODV_HELLO_LIFE                    (3 * SECOND)

// Additional Maodv default timer and constant values ref:
// draft-ietf-manet-maodv-00.txt section: 11

// /**
// CONSTANT    :: MAODV_GROUP_HELLO_INTERVAL:   (5000 * MILLI_SECOND)
// DESCRIPTION :: Group hello interval
// **/
#define MAODV_GROUP_HELLO_INTERVAL          (5000 * MILLI_SECOND)

// /**
// CONSTANT    :: MAODV_MTREE_BUILD:   (2 * MAODV_REV_ROUTE_LIFE)
// DESCRIPTION :: Multicast tree build time
// **/
#define MAODV_MTREE_BUILD                   (2 * MAODV_REV_ROUTE_LIFE)

// /**
// CONSTANT    :: MAODV_PRUNE_TIMEOUT:   (MAODV_ACTIVE_ROUTE_TIMEOUT)
// DESCRIPTION :: Prune timeout
// **/
#define MAODV_PRUNE_TIMEOUT                 MAODV_ACTIVE_ROUTE_TIMEOUT

// /**
// CONSTANT    :: IPOPT_MAODV:   138
// DESCRIPTION :: Option header No.
// **/
#define IPOPT_MAODV  138

// /**
// STRUCT      :: MaodvIpOptionType
// DESCRIPTION :: Structure storing MAODV option fields for IP header
// **/
typedef struct
{
    unsigned int pktId;
    NodeAddress srcId;
} MaodvIpOptionType;

// /**
// ENUM        :: MaodvIpOptionType
// DESCRIPTION :: Route request type
// **/
enum rreq_type
{
    JOIN = 0, // Join request for a group
    REPAIR,   // Repair multicast tree
    NONJOIN,  // Nodes to send data to a multicast group
    COMBINE   // to combine tree partitions
};

typedef enum rreq_type RreqType;

// /**
// STRUCT      :: dest_info
// DESCRIPTION :: Structure storing Node address and sequence number
// **/
typedef struct dest_info
{
    unsigned int address;
    unsigned int seqNum;
} AddSeqInfo;

// /**
// STRUCT      :: str_maodv_neighbor
// DESCRIPTION :: Structure storing neighbor information
// **/
typedef struct str_maodv_neighbor
{
    NodeAddress destAddr;
    clocktype lastHeared;
    struct str_maodv_neighbor *next;
} MaodvNeighborNode;

// /**
// STRUCT      :: nei_table
// DESCRIPTION :: Structure storing neighbor information table
// **/
typedef struct nei_table
{
    MaodvNeighborNode *head;
    int size;
} MaodvNeighborTable;

// /**
// STRUCT      :: str_rreq_seen
// DESCRIPTION :: Structure storing rreq seen
// **/
typedef struct str_rreq_seen
{
    NodeAddress srcAddr;    // Source address of RREQ
    int floodingId;         // Broadcast address of RREQ
    NodeAddress leader;     // used for group hello
    struct str_rreq_seen *next;
} MaodvRreqSeenNode;

// /**
// STRUCT      :: MaodvRreqSeenTable
// DESCRIPTION :: Structure storing list for RST structure
// **/
typedef struct
{
    MaodvRreqSeenNode *front;
    MaodvRreqSeenNode *rear;
    int size;
} MaodvRreqSeenTable;

// /**
// STRUCT      :: str_fifo_buffer
// DESCRIPTION :: Structure to store packets temporarily until one route to the destination
// of the packet is found
// **/
typedef struct str_fifo_buffer
{
    NodeAddress destAddr;       // Destination address of the packet
    NodeAddress lastHopAddress; // The node from which it has got the packet
    clocktype timestamp;        // The time when the packet was inserted in
                                // the buffer
    NodeAddress previousHop;    // The last hop which sent the data
    Message *msg;               // The packet to be sent
    struct str_fifo_buffer *next;
} MaodvBufferNode;

// /**
// STRUCT      :: MaodvMessageBuffer
// DESCRIPTION :: Structure storing list for message buffer
// **/
typedef struct
{
    MaodvBufferNode *head;
    int size;                   // in Number of packets
    int numByte;
} MaodvMessageBuffer;

// /**
// STRUCT      :: MaodvRreqSentNode
// DESCRIPTION :: Structure to store information about messages for which RREQ has been sent
// These information are necessary until a route is found for the destination
// **/
typedef struct str_sent_node
{
    NodeAddress destAddr;  // Destination for which the RREQ has been sent
    int ttl;               // Last used TTL to find the route
    int times;             // Number of times RREQ has been sent
    RreqType type;
    clocktype timestamp;
    BOOL isReplied;
    struct str_sent_node *next;
} MaodvRreqSentNode;

// /**
// STRUCT      :: MaodvRreqSentTable
// DESCRIPTION :: Structure to store link list for Sent node entries
// **/
typedef struct
{
    MaodvRreqSentNode *head;
    int size;
} MaodvRreqSentTable;

// /**
// STRUCT      :: maodvMCE
// DESCRIPTION :: Structure to Message cache
// **/
typedef struct maodvMCE
{
    NodeAddress srcAddr;
    unsigned int pktId;
    unsigned char hash[16];
    BOOL sent;
    struct maodvMCE *next;
} MAODV_MC_Node;

// /**
// STRUCT      :: MAODV_MC
// DESCRIPTION :: Structure to Message cache
// **/
typedef struct
{
    MAODV_MC_Node *front;
    MAODV_MC_Node *rear;
    int size;
} MAODV_MC;

// /**
// ENUM      :: MaodvMemberType
// DESCRIPTION :: Member type
// **/
typedef enum
{
    LEADER,
    MEMBER,
    TREE
} MaodvMemberType;

// /**
// STRUCT      :: MAODV_MC
// DESCRIPTION :: Structure to  Membership table
// **/
typedef struct MaodvME
{
    NodeAddress mcastAddr;
    MaodvMemberType type;
    BOOL isUpdate;
    struct MaodvME *next;
} MAODV_M_Node;

// /**
// STRUCT      :: MAODV_Membership
// DESCRIPTION :: Structure to  Membership table
// **/
typedef struct
{
    MAODV_M_Node *head;
    int size;
} MAODV_Membership;

// /**
// ENUM      :: LinkDirection
// DESCRIPTION ::  Direction of the link relative to the location of the group
// leader
// **/
typedef enum
{
    UPSTREAM_DIRECTION = 0,               // next hop towards the group leader
    DOWNSTREAM_DIRECTION = 1              // next hop away from the group leader
} LinkDirection;

// /**
// STRUCT      :: MaodvNextHopNode
// DESCRIPTION :: Structure to store next hops list
// **/
typedef struct next_hops_str
{
    NodeAddress nextHopAddr;    // addresss of the next hop
    unsigned int interface;     // next hop interface field
    LinkDirection direction;    // Link direction
    BOOL isEnabled;             // Is the entry validated by MACT??
    clocktype lifetime;         // to maintain connectivity
    struct next_hops_str *next;
} MaodvNextHopNode;

// /**
// STRUCT      :: MaodvNextHops
// DESCRIPTION :: Structure to store next hops list
// **/
typedef struct next_hops_list
{
    MaodvNextHopNode *head;
    int size;
} MaodvNextHops;

// /**
// STRUCT      :: MaodvRouteEntry
// DESCRIPTION :: Structure to route entry
// **/
typedef struct maodv_route_table_row
{
    AddSeqInfo       destination; // Destination node and destination
                                  // sequence number
    unsigned int     interface;   // The interface through which the
                                  // packet should be forwarded to reach
                                  // the destination
    int              hopCount;    // Number of hops to traverse to reach
                                  // the destination
    int              lastHopCount;
    unsigned int     nextHop;     // The next hop address to which to
                                  // to forward the packet to reach the
                                  // destination

    clocktype        lifetime;    // The life time of the route
    clocktype        latency;     // Not used now, to be used to chose
                                  // among equal distant routes

    BOOL             activated;   // Whether the route is active
    struct maodv_route_table_row *next;
} MaodvRouteEntry;

// /**
// STRUCT      :: MaodvRoutingTable
// DESCRIPTION :: Structure to route table
// **/
typedef struct route_table_str
{
    MaodvRouteEntry *rtEntry;
    int   size;
} MaodvRoutingTable;

// /**
// STRUCT      :: MaodvMrouteEntry
// DESCRIPTION :: Structure to multicast route entry
// **/
typedef struct multi_route_table_str
{
    AddSeqInfo destination;   // Multicast Group IP Address and Seq Number
    NodeAddress mcastLeader;  // Multicast Group Leader IP Address
    MaodvNextHops nextHops;   // Next hops list
    int hopCount;             // Hop count to next multicast group member
    int hopCountLeader;       // Hop count to Multicast Group Leader
    clocktype lifetime;
    BOOL isValid;             // TRUE if it is a member of the tree
    BOOL isMember;            // TRUE if it is an active tree member
    struct multi_route_table_str *next;
} MaodvMrouteEntry;

// /**
// STRUCT      :: MaodvMroutingTable
// DESCRIPTION :: Structure to multicast route table
// **/
typedef struct mroute_table_str
{
    MaodvMrouteEntry *mrtEntry;
    int size;
} MaodvMroutingTable;

// /**
// STRUCT      :: NextHopInfo
// DESCRIPTION :: Maodv structure for passing next hop
// **/
typedef struct dest_nexthop
{
    NodeAddress mcastAddr;
    NodeAddress nextHopAddr;
} NextHopInfo;

// /**
// STRUCT      :: MaodvRequestEntry
// DESCRIPTION :: Multicast Group Leader Table or request table
// **/
typedef struct g_leader_table_str
{
    NodeAddress mcastAddr;           // Multicast address of the group
    NodeAddress leaderAddr;          // Multicast address of the group leader
    struct g_leader_table_str *next;
} MaodvRequestEntry;

// /**
// STRUCT      :: MaodvRequestTable
// DESCRIPTION :: Request table sturcture
// **/
typedef struct group_leader_table
{
    MaodvRequestEntry *requestEntry;
    int size;
} MaodvRequestTable;

typedef unsigned char MAODV_PacketType;

// /**
// CONSTANT    :: MAODV_RREQ:   1
// DESCRIPTION :: route request packet type
// **/
#define MAODV_RREQ     1

// /**
// CONSTANT    :: MAODV_RREP:   2
// DESCRIPTION :: route reply packet type
// **/
#define MAODV_RREP     2

// /**
// CONSTANT    :: MAODV_MGLEXT:   3
// DESCRIPTION :: Multicast group leader extension format
// **/
#define MAODV_MGLEXT   3

// /**
// CONSTANT    :: MAODV_MGREXT:   4
// DESCRIPTION :: Multicast group Rebuild extension format
// **/
#define MAODV_MGREXT   4

// /**
// CONSTANT    :: MAODV_MGIEXT:   5
// DESCRIPTION :: Multicast group information extension format
// **/
#define MAODV_MGIEXT   5

// /**
// CONSTANT    :: MAODV_MGIEXT:   6
// DESCRIPTION :: Multicast Activation (MACT) Message Format
// **/
#define MAODV_MACT     6

// /**
// CONSTANT    :: MAODV_MGIEXT:   7
// DESCRIPTION :: Group Hello (GRPH) Message Format
// **/
#define MAODV_GRPH     7

// /**
// STRUCT      :: MaodvRequestTable
// DESCRIPTION :: Maodv route request message format
// **/
typedef struct RreqPacket
{
    UInt32 RreqPkt;//type     : 8,  // 1 for request
                 //J        : 1,  // Join Flag
                 //R        : 1,  // Repair Flag
                 //G        : 1,  // Gratuitous rreq flag
                 //reserved : 13, // 0
                 //hopCount : 8;
    unsigned int  floodingId;
    AddSeqInfo    destination;   // destination address and sequence
    AddSeqInfo    source;        // source address and sequence
} RreqPacket;


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetType()
//
// PURPOSE:      Set the value of type for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetType(UInt32 *RreqPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *RreqPkt = *RreqPkt & maskInt(9, 32);

    //setting the value of type in RreqPkt
    *RreqPkt = *RreqPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetJ()
//
// PURPOSE:      Set the value of J for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               J, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetJ(UInt32 *RreqPkt, BOOL J)
{
    //masks J within boundry range
    J = J & maskInt(32, 32);

    //clears the ninth bit
    *RreqPkt = *RreqPkt & (~(maskInt(9, 9)));

    //setting the value of J in RreqPkt
    *RreqPkt = *RreqPkt | LshiftInt(J, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetR()
//
// PURPOSE:      Set the value of R for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetR(UInt32 *RreqPkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the tenth bit
    *RreqPkt = *RreqPkt & (~(maskInt(10, 10)));

    //setting the value of R in RreqPkt
    *RreqPkt = *RreqPkt | LshiftInt(R, 10);
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetG()
//
// PURPOSE:      Set the value of G for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               G, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetG(UInt32 *RreqPkt, BOOL G)
{
    //masks G within boundry range
    G = G & maskInt(32, 32);

    //clears the eleventh bit
    *RreqPkt = *RreqPkt & (~(maskInt(11, 11)));

    //setting the value of G in RreqPkt
    *RreqPkt = *RreqPkt | LshiftInt(G, 11);
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetResv()
//
// PURPOSE:      Set the value of reserved for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetResv(UInt32 *RreqPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(20, 32);

    //clears the 12-24 bit
    *RreqPkt = *RreqPkt & (~(maskInt(12, 24)));

    //setting the value of reserved in RreqPkt
    *RreqPkt = *RreqPkt | LshiftInt(reserved, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RreqPacketSetHopCount(UInt32 *RreqPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *RreqPkt = *RreqPkt & maskInt(1, 24);

    //setting the value of hopCount in RreqPkt
    *RreqPkt = *RreqPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetType()
//
// PURPOSE:      Returns the value of type for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RreqPacketGetType(UInt32 RreqPkt)
{
    UInt32 type = RreqPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetJ()
//
// PURPOSE:      Returns the value of J for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL RreqPacketGetJ(UInt32 RreqPkt)
{
    UInt32 J = RreqPkt;

    //clears all the bits except ninth
    J = J & maskInt(9, 9);

    //right shifts so that last bit represent J
    J = RshiftInt(J, 9);

    return (BOOL)J;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetR()
//
// PURPOSE:      Returns the value of R for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL RreqPacketGetR(UInt32 RreqPkt)
{
    UInt32 R = RreqPkt;

    //clears all the bits except tenth
    R = R & maskInt(10, 10);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 10);

    return R;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetG()
//
// PURPOSE:      Returns the value of G for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL RreqPacketGetG(UInt32 RreqPkt)
{
    UInt32 G = RreqPkt;

    //clears all the bits except eleventh
    G = G & maskInt(11, 11);

    //right shifts so that last bit represent G
    G = RshiftInt(G, 11);

    return (BOOL)G;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RreqPacketGetResv(UInt32 RreqPkt)
{
    UInt32 reserved = RreqPkt;

    //clears all the bits except 12-24
    reserved = reserved & maskInt(12, 24);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 24);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     RreqPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for RreqPacket
//
// ARGUMENTS:    RreqPkt, The variable containing the value of type,J,R,G,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RreqPacketGetHopCount(UInt32 RreqPkt)
{
    UInt32 hopCount = RreqPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: MglextPacket
// DESCRIPTION :: Maodv multicast group leader extension format
// this packet format is used when the RREQ
// is unicasted to the leader
// **/
typedef struct mgl_extension
{
    UInt32 MglextPkt;//type   : 8,   // 3 for mgl extension
               //J        : 1,   // Join Flag
               //R        : 1,   // Repair Flag
               //G        : 1,   // Gratuitous rreq flag
               //reserved : 13,  // 0
               //hopCount : 8;
    unsigned int  floodingId;
    AddSeqInfo    destination; // destination address and sequence
    AddSeqInfo    source;      // source address and sequence
    NodeAddress   leader;      // Multicast group leader IP address
    char          padding[2];
} MglextPacket;

//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetType()
//
// PURPOSE:      Set the value of type for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetType(UInt32 *MglextPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *MglextPkt = *MglextPkt & maskInt(9, 32);

    //setting the value of type in MglextPkt
    *MglextPkt = *MglextPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetJ()
//
// PURPOSE:      Set the value of J for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               J, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetJ(UInt32 *MglextPkt, BOOL J)
{
    //masks J within boundry range
    J = J & maskInt(32, 32);

    //clears the ninth bit
    *MglextPkt = *MglextPkt & (~(maskInt(9, 9)));

    //setting the value of J in MglextPkt
    *MglextPkt = *MglextPkt | LshiftInt(J, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetR()
//
// PURPOSE:      Set the value of R for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetR(UInt32 *MglextPkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the tenth bit
    *MglextPkt = *MglextPkt & (~(maskInt(10, 10)));

    //setting the value of R in MglextPkt
    *MglextPkt = *MglextPkt | LshiftInt(R, 10);
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetG()
//
// PURPOSE:      Set the value of G for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               G, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetG(UInt32 *MglextPkt, BOOL G)
{
    //masks G within boundry range
    G = G & maskInt(32, 32);

    //clears the eleventh bit
    *MglextPkt = *MglextPkt & (~(maskInt(11, 11)));

    //setting the value of G in MglextPkt
    *MglextPkt = *MglextPkt | LshiftInt(G, 11);
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetResv()
//
// PURPOSE:      Set the value of reserved for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetResv(UInt32 *MglextPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(20, 32);

    //clears the 12-24 bit
    *MglextPkt = *MglextPkt & (~(maskInt(12, 24)));

    //setting the value of reserved in MglextPkt
    *MglextPkt = *MglextPkt | LshiftInt(reserved, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MglextPacketSetHopCount(UInt32 *MglextPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *MglextPkt = *MglextPkt & maskInt(1, 24);

    //setting the value of hopCount in MglextPkt
    *MglextPkt = *MglextPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetType()
//
// PURPOSE:      Returns the value of type for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MglextPacketGetType(UInt32 MglextPkt)
{
    UInt32 type = MglextPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetJ()
//
// PURPOSE:      Returns the value of J for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MglextPacketGetJ(UInt32 MglextPkt)
{
    UInt32 J = MglextPkt;

    //clears all the bits except ninth
    J = J & maskInt(9, 9);

    //right shifts so that last bit represent J
    J = RshiftInt(J, 9);

    return (BOOL)J;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetR()
//
// PURPOSE:      Returns the value of R for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MglextPacketGetR(UInt32 MglextPkt)
{
    UInt32 R = MglextPkt;

    //clears all the bits except tenth
    R = R & maskInt(10, 10);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 10);

    return R;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetG()
//
// PURPOSE:      Returns the value of G for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MglextPacketGetG(UInt32 MglextPkt)
{
    UInt32 G = MglextPkt;

    //clears all the bits except eleventh
    G = G & maskInt(11, 11);

    //right shifts so that last bit represent G
    G = RshiftInt(G, 11);

    return (BOOL)G;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MglextPacketGetResv(UInt32 MglextPkt)
{
    UInt32 reserved = MglextPkt;

    //clears all the bits except 12-24
    reserved = reserved & maskInt(12, 24);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 24);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     MglextPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for MglextPacket
//
// ARGUMENTS:    MglextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MglextPacketGetHopCount(UInt32 MglextPkt)
{
    UInt32 hopCount = MglextPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: MgrextPacket
// DESCRIPTION :: Maodv multicast group Rebuild extension format
// used to request tree repair
// **/
typedef struct mgr_extension
{
    UInt32 MgrextPkt;//type   : 8,    // 4 for mgr extension
               //J        : 1,    // Join Flag
               //R        : 1,    // Repair Flag
               //G        : 1,    // Gratuitous rreq flag
               //reserved : 13,   // 0
               //hopCount : 8;
    unsigned int floodingId;
    AddSeqInfo   destination;   // destination address and sequence
    AddSeqInfo   source;        // source address and sequence
    NodeAddress  leader;        // Multicast group leader IP address
    int          groupHopCount; // Multicast group hop count
    char         padding[4];
} MgrextPacket;

//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetType()
//
// PURPOSE:      Set the value of type for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetType(UInt32 *MgrextPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *MgrextPkt = *MgrextPkt & maskInt(9, 32);

    //setting the value of type in MgrextPkt
    *MgrextPkt = *MgrextPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetJ()
//
// PURPOSE:      Set the value of J for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               J, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetJ(UInt32 *MgrextPkt, BOOL J)
{
    //masks J within boundry range
    J = J & maskInt(32, 32);

    //clears the ninth bit
    *MgrextPkt = *MgrextPkt & (~(maskInt(9, 9)));

    //setting the value of J in MgrextPkt
    *MgrextPkt = *MgrextPkt | LshiftInt(J, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetR()
//
// PURPOSE:      Set the value of R for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetR(UInt32 *MgrextPkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the tenth bit
    *MgrextPkt = *MgrextPkt & (~(maskInt(10, 10)));

    //setting the value of R in MgrextPkt
    *MgrextPkt = *MgrextPkt | LshiftInt(R, 10);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetG()
//
// PURPOSE:      Set the value of G for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               G, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetG(UInt32 *MgrextPkt, BOOL G)
{
    //masks G within boundry range
    G = G & maskInt(32, 32);

    //clears the eleventh bit
    *MgrextPkt = *MgrextPkt & (~(maskInt(11, 11)));

    //setting the value of G in MgrextPkt
    *MgrextPkt = *MgrextPkt | LshiftInt(G, 11);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetResv()
//
// PURPOSE:      Set the value of reserved for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetResv(UInt32 *MgrextPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(20, 32);

    //clears the 12-24 bit
    *MgrextPkt = *MgrextPkt & (~(maskInt(12, 24)));

    //setting the value of reserved in MgrextPkt
    *MgrextPkt = *MgrextPkt | LshiftInt(reserved, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgrextPacketSetHopCount(UInt32 *MgrextPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *MgrextPkt = *MgrextPkt & maskInt(1, 24);

    //setting the value of hopCount in MgrextPkt
    *MgrextPkt = *MgrextPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetType()
//
// PURPOSE:      Returns the value of type for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgrextPacketGetType(UInt32 MgrextPkt)
{
    UInt32 type = MgrextPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetJ()
//
// PURPOSE:      Returns the value of J for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MgrextPacketGetJ(UInt32 MgrextPkt)
{
    UInt32 J = MgrextPkt;

    //clears all the bits except ninth
    J = J & maskInt(9, 9);

    //right shifts so that last bit represent J
    J = RshiftInt(J, 9);

    return (BOOL)J;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetR()
//
// PURPOSE:      Returns the value of R for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MgrextPacketGetR(UInt32 MgrextPkt)
{
    UInt32 R = MgrextPkt;

    //clears all the bits except tenth
    R = R & maskInt(10, 10);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 10);

    return (BOOL)R;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetG()
//
// PURPOSE:      Returns the value of G for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MgrextPacketGetG(UInt32 MgrextPkt)
{
    UInt32 G = MgrextPkt;

    //clears all the bits except eleventh
    G = G & maskInt(11, 11);

    //right shifts so that last bit represent G
    G = RshiftInt(G, 11);

    return (BOOL)G;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgrextPacketGetResv(UInt32 MgrextPkt)
{
    UInt32 reserved = MgrextPkt;

    //clears all the bits except 12-24
    reserved = reserved & maskInt(12, 24);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 24);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgrextPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for MgrextPacket
//
// ARGUMENTS:    MgrextPkt, The variable containing the value of type,J,R,G,
//                          reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgrextPacketGetHopCount(UInt32 MgrextPkt)
{
    UInt32 hopCount = MgrextPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: MactType
// DESCRIPTION :: MACT type
// **//
typedef enum
{
    J,
    P,
    G,
    U,
    R
} MactType;

// /**
// STRUCT      :: RrepPacket
// DESCRIPTION :: Maodv route reply message format
// **//
typedef struct route_reply
{
    UInt32 Rreppkt;//type      :  8,// 2 for reply
                 //R         :  1,// Repair Flag
                 //reserved  : 10,
                 //prefixSz  :  5,// Prefix not used necessary for subnetting
                 //hopCount  :  8;
    AddSeqInfo   destination;    // Destination address and sequence
    unsigned int sourceAddr;     // Address of the source node which issued
                                 // the request
    unsigned int lifetime;       // Lifetime of the destination
} RrepPacket;

//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketSetType()
//
// PURPOSE:      Set the value of type for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RrepPacketSetType(UInt32 *Rreppkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *Rreppkt = *Rreppkt & maskInt(9, 32);

    //setting the value of type in Rreppkt
    *Rreppkt = *Rreppkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketSetR()
//
// PURPOSE:      Set the value of R for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RrepPacketSetR(UInt32 *Rreppkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the ninth bit
    *Rreppkt = *Rreppkt & (~(maskInt(9, 9)));

    //setting the value of J in Rreppkt
    *Rreppkt = *Rreppkt | LshiftInt(R, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketSetResv()
//
// PURPOSE:      Set the value of reserved for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RrepPacketSetResv(UInt32 *Rreppkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(23, 32);

    //clears the 10-19 bit
    *Rreppkt = *Rreppkt & (~(maskInt(10, 19)));

    //setting the value of reserved in Rreppkt
    *Rreppkt = *Rreppkt | LshiftInt(reserved, 19);
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketSetPrefixSz()
//
// PURPOSE:      Set the value of prefixSz for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//               prefixSz, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RrepPacketSetPrefixSz(UInt32 *Rreppkt, UInt32 prefixSz)
{
    //masks prefixSz within boundry range
    prefixSz = prefixSz & maskInt(28, 32);

    //clears the 20-24 bit
    *Rreppkt = *Rreppkt & (~(maskInt(20, 24)));

    //setting the value of prefixSz in Rreppkt
    *Rreppkt = *Rreppkt | LshiftInt(prefixSz, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void RrepPacketSetHopCount(UInt32 *Rreppkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *Rreppkt = *Rreppkt & maskInt(1, 24);

    //setting the value of hopCount in Rreppkt
    *Rreppkt = *Rreppkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketGetType()
//
// PURPOSE:      Returns the value of type for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RrepPacketGetType(UInt32 Rreppkt)
{
    UInt32 type = Rreppkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketGetR()
//
// PURPOSE:      Returns the value of R for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL RrepPacketGetR(UInt32 Rreppkt)
{
    UInt32 R = Rreppkt;

    //clears all the bits except ninth
    R = R & maskInt(9, 9);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 9);

    return R;
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RrepPacketGetResv(UInt32 Rreppkt)
{
    UInt32 reserved = Rreppkt;

    //clears all the bits except 10-19
    reserved = reserved & maskInt(10, 19);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 19);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketGetPrefixSz()
//
// PURPOSE:      Returns the value of prefixSz for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RrepPacketGetPrefixSz(UInt32 Rreppkt)
{
    UInt32 prefixSz = Rreppkt;

    //clears all the bits except 20-24
    prefixSz = prefixSz & maskInt(20, 24);

    //right shifts so that last bit represent prefixSz
    prefixSz = RshiftInt(prefixSz, 24);

    return prefixSz;
}


//---------------------------------------------------------------------------
// FUNCTION:     RrepPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for RrepPacket
//
// ARGUMENTS:    Rreppkt, The variable containing the value of type,R,
//                        reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 RrepPacketGetHopCount(UInt32 Rreppkt)
{
    UInt32 hopCount = Rreppkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// Extension TO RREP

// /**
// STRUCT      :: MgiextPacket
// DESCRIPTION :: Maodv multicast group Information extension format
// this packet is used when replying to the tree join/repair request
// **//
typedef struct mgi_extension
{
    UInt32 MgiextPkt;//type     : 8, // 5 for MGI extension
               //R          : 1,   // Repair Flag
               //reserved   : 10,
               //prefixSz   : 5,  // Prefix not used necessary for subnetting
               //hopCount   : 8;
    AddSeqInfo   destination;    // Destination address and sequence
    unsigned int sourceAddr;     // Address of the source node which issued
                                 // the request
    unsigned int lifetime;       // Lifetime of the destination
    int          groupHopCount;  // Multicast group hop count
    NodeAddress  leader;         // Multicast group leader IP address
    char         padding[2];
} MgiextPacket;

//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketSetType()
//
// PURPOSE:      Set the value of type for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgiextPacketSetType(UInt32 *MgiextPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *MgiextPkt = *MgiextPkt & maskInt(9, 32);

    //setting the value of type in MgiextPkt
    *MgiextPkt = *MgiextPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketSetR()
//
// PURPOSE:      Set the value of R for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgiextPacketSetR(UInt32 *MgiextPkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the ninth bit
    *MgiextPkt = *MgiextPkt & (~(maskInt(9, 9)));

    //setting the value of J in MgiextPkt
    *MgiextPkt = *MgiextPkt | LshiftInt(R, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketSetResv()
//
// PURPOSE:      Set the value of reserved for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void

//---------------------------------------------------------------------------
static void MgiextPacketSetResv(UInt32 *MgiextPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(23, 32);

    //clears the 10-19 bit
    *MgiextPkt = *MgiextPkt & (~(maskInt(10, 19)));

    //setting the value of reserved in MgiextPkt
    *MgiextPkt = *MgiextPkt | LshiftInt(reserved, 19);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketSetPrefixSz()
//
// PURPOSE:      Set the value of prefixSz for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//               prefixSz, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgiextPacketSetPrefixSz(UInt32 *MgiextPkt, UInt32 prefixSz)
{
    //masks prefixSz within boundry range
    prefixSz = prefixSz & maskInt(28, 32);

    //clears the 20-24 bit
    *MgiextPkt = *MgiextPkt & (~(maskInt(20, 24)));

    //setting the value of prefixSz in MgiextPkt
    *MgiextPkt = *MgiextPkt | LshiftInt(prefixSz, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MgiextPacketSetHopCount(UInt32 *MgiextPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *MgiextPkt = *MgiextPkt & maskInt(1, 24);

    //setting the value of hopCount in MgiextPkt
    *MgiextPkt = *MgiextPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketGetType()
//
// PURPOSE:      Returns the value of type for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgiextPacketGetType(UInt32 MgiextPkt)
{
    UInt32 type = MgiextPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketGetR()
//
// PURPOSE:      Returns the value of R for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MgiextPacketGetR(UInt32 MgiextPkt)
{
    UInt32 R = MgiextPkt;

    //clears all the bits except ninth
    R = R & maskInt(9, 9);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 9);

    return (BOOL)R;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgiextPacketGetResv(UInt32 MgiextPkt)
{
    UInt32 reserved = MgiextPkt;

    //clears all the bits except 10-19
    reserved = reserved & maskInt(10, 19);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 19);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketGetPrefixSz()
//
// PURPOSE:      Returns the value of prefixSz for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgiextPacketGetPrefixSz(UInt32 MgiextPkt)
{
    UInt32 prefixSz = MgiextPkt;

    //clears all the bits except 20-24
    prefixSz = prefixSz & maskInt(20, 24);

    //right shifts so that last bit represent prefixSz
    prefixSz = RshiftInt(prefixSz, 24);

    return prefixSz;
}


//---------------------------------------------------------------------------
// FUNCTION:     MgiextPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for MgiextPacket
//
// ARGUMENTS:    MgiextPkt, The variable containing the value of type,R,
//                          reserved,prefixSz and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MgiextPacketGetHopCount(UInt32 MgiextPkt)
{
    UInt32 hopCount = MgiextPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: MactPacket
// DESCRIPTION :: Multicst Activation (MACT) Message Format
// **//
typedef struct mact_str
{
    UInt32 MactPkt;//type   : 8,  // 6 for MACT message
               //J        : 1,  // Join flag
               //P        : 1,  // Prune Flag
               //G        : 1,  // Group leader flag
               //U        : 1,  // Update flag
               //R        : 1,  // Reboot Flag
               //reserved : 11, // sent as 0 and ignored on reception
               //hopCount : 8;
    NodeAddress  srcAddr;     // IP address of the sending node

    NodeAddress  leaderAddr;  // used when U flag is set
    AddSeqInfo   mcastDest;   // Address and seq number of the current route
} MactPacket;

//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetType()
//
// PURPOSE:      Set the value of type for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetType(UInt32 *MactPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *MactPkt = *MactPkt & maskInt(9, 32);

    //setting the value of type in MactPkt
    *MactPkt = *MactPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetJ()
//
// PURPOSE:      Set the value of J for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               J, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetJ(UInt32 *MactPkt, BOOL J)
{
    //masks J within boundry range
    J = J & maskInt(32, 32);

    //clears the ninth bit
    *MactPkt = *MactPkt & (~(maskInt(9, 9)));

    //setting the value of J in MactPkt
    *MactPkt = *MactPkt | LshiftInt(J, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetP()
//
// PURPOSE:      Set the value of P for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               P, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetP(UInt32 *MactPkt, BOOL P)
{
    //masks P within boundry range
    P = P & maskInt(32, 32);

    //clears the tenth bit
    *MactPkt = *MactPkt & (~(maskInt(10, 10)));

    //setting the value of P in MactPkt
    *MactPkt = *MactPkt | LshiftInt(P, 10);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetG()
//
// PURPOSE:      Set the value of G for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                        R,reserved and hopCount
//               G, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetG(UInt32 *MactPkt, BOOL G)
{
    //masks G within boundry range
    G = G & maskInt(32, 32);

    //clears the eleventh bit
    *MactPkt = *MactPkt & (~(maskInt(11, 11)));

    //setting the value of G in MactPkt
    *MactPkt = *MactPkt | LshiftInt(G, 11);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetU()
//
// PURPOSE:      Set the value of U for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               U, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetU(UInt32 *MactPkt, BOOL U)
{
    //masks R within boundry range
    U = U & maskInt(32, 32);

    //clears the twelfth bit
    *MactPkt = *MactPkt & (~(maskInt(12, 12)));

    //setting the value of U in MactPkt
    *MactPkt = *MactPkt | LshiftInt(U, 12);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetR()
//
// PURPOSE:      Set the value of R for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               R, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetR(UInt32 *MactPkt, BOOL R)
{
    //masks R within boundry range
    R = R & maskInt(32, 32);

    //clears the thirteenth bit
    *MactPkt = *MactPkt & (~(maskInt(13, 13)));

    //setting the value of R in MactPkt
    *MactPkt = *MactPkt | LshiftInt(R, 13);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetResv()
//
// PURPOSE:      Set the value of reserved for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetResv(UInt32 *MactPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(22, 32);

    //clears the 14-24 bit
    *MactPkt = *MactPkt & (~(maskInt(14, 24)));

    //setting the value of reserved in MactPkt
    *MactPkt = *MactPkt | LshiftInt(reserved, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void MactPacketSetHopCount(UInt32 *MactPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *MactPkt = *MactPkt & maskInt(1, 24);

    //setting the value of hopCount in MactPkt
    *MactPkt = *MactPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetType()
//
// PURPOSE:      Returns the value of type for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MactPacketGetType(UInt32 MactPkt)
{
    UInt32 type = MactPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetJ()
//
// PURPOSE:      Returns the value of J for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MactPacketGetJ(UInt32 MactPkt)
{
    UInt32 J = MactPkt;

    //clears all the bits except ninth
    J = J & maskInt(9, 9);

    //right shifts so that last bit represent J
    J = RshiftInt(J, 9);

    return (BOOL)J;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetP()
//
// PURPOSE:      Returns the value of P for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MactPacketGetP(UInt32 MactPkt)
{
    UInt32 P = MactPkt;

    //clears all the bits except tenth
    P = P & maskInt(10, 10);

    //right shifts so that last bit represent P
    P = RshiftInt(P, 10);

    return (BOOL)P;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetG()
//
// PURPOSE:      Returns the value of G for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MactPacketGetG(UInt32 MactPkt)
{
    UInt32 G = MactPkt;

    //clears all the bits except eleventh
    G = G & maskInt(11, 11);

    //right shifts so that last bit represent G
    G = RshiftInt(G, 11);

    return (BOOL)G;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetU()
//
// PURPOSE:      Returns the value of U for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MactPacketGetU(UInt32 MactPkt)
{
    UInt32 U = MactPkt;

    //clears all the bits except eleventh
    U = U & maskInt(12, 12);

    //right shifts so that last bit represent U
    U = RshiftInt(U, 12);

    return (BOOL)U;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetR()
//
// PURPOSE:      Returns the value of R for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL MactPacketGetR(UInt32 MactPkt)
{
    UInt32 R = MactPkt;

    //clears all the bits except thirteenth
    R = R & maskInt(13, 13);

    //right shifts so that last bit represent R
    R = RshiftInt(R, 13);

    return (BOOL)R;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MactPacketGetResv(UInt32 MactPkt)
{
    UInt32 reserved = MactPkt;

    //clears all the bits except 14-24
    reserved = reserved & maskInt(14, 24);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 24);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     MactPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for MactPacket
//
// ARGUMENTS:    MactPkt, The variable containing the value of type,J,P,G,U,
//                         R,reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 MactPacketGetHopCount(UInt32 MactPkt)
{
    UInt32 hopCount = MactPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: GrphPacket
// DESCRIPTION ::Group Hello (GRPH) Message Format
// **//
typedef struct grph_str
{
    UInt32 GrphPkt;//type     : 8,  // 7 for GRPH message
                 //U        : 1,  // Update flag
                 //M        : 1,  // Off_Mtree flag
                 //reserved : 14, // sent as 0 and ignored on reception
                 //hopCount : 8;
    NodeAddress  leaderAddr;    // IP address of the group leader
    AddSeqInfo   mcastGroup;    // Group address and sequence number of the
                                // current route
} GrphPacket;

//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketSetType()
//
// PURPOSE:      Set the value of type for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//               type, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void GrphPacketSetType(UInt32 *GrphPkt, UInt32 type)
{
    //masks type within boundry range
    type = type & maskInt(25, 32);

    //clears the first 8 bits
    *GrphPkt = *GrphPkt & maskInt(9, 32);

    //setting the value of type in GrphPkt
    *GrphPkt = *GrphPkt | LshiftInt(type, 8);
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketSetU()
//
// PURPOSE:      Set the value of U for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//               U, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void GrphPacketSetU(UInt32 *GrphPkt, BOOL U)
{
    //masks U within boundry range
    U = U & maskInt(32, 32);

    //clears the ninth bit
    *GrphPkt = *GrphPkt & (~(maskInt(9, 9)));

    //setting the value of U in GrphPkt
    *GrphPkt = *GrphPkt | LshiftInt(U, 9);
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketSetM()
//
// PURPOSE:      Set the value of M for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//               M, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void GrphPacketSetM(UInt32 *GrphPkt, BOOL M)
{
    //masks M within boundry range
    M = M & maskInt(32, 32);

    //clears the tenth bit
    *GrphPkt = *GrphPkt & (~(maskInt(10, 10)));

    //setting the value of M in GrphPkt
    *GrphPkt = *GrphPkt | LshiftInt(M, 10);
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketSetResv()
//
// PURPOSE:      Set the value of reserved for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//               reserved, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void GrphPacketSetResv(UInt32 *GrphPkt, UInt32 reserved)
{
    //masks reserved within boundry range
    reserved = reserved & maskInt(19, 32);

    //clears the 11-24 bit
    *GrphPkt = *GrphPkt & (~(maskInt(11, 24)));

    //setting the value of reserved in GrphPkt
    *GrphPkt = *GrphPkt | LshiftInt(reserved, 24);
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketSetHopCount()
//
// PURPOSE:      Set the value of hopCount for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//               hopCount, Input value for set operation
//
// RETURN:       void
//---------------------------------------------------------------------------
static void GrphPacketSetHopCount(UInt32 *GrphPkt, UInt32 hopCount)
{
    //masks hopCount within boundry range
    hopCount = hopCount & maskInt(25, 32);

    //clears the last 8 bits
    *GrphPkt = *GrphPkt & maskInt(1, 24);

    //setting the value of hopCount in GrphPkt
    *GrphPkt = *GrphPkt | hopCount;
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketGetType()
//
// PURPOSE:      Returns the value of type for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 GrphPacketGetType(UInt32 GrphPkt)
{
    UInt32 type = GrphPkt;

    //clears all the bits except first 8
    type = type & maskInt(1, 8);

    //right shifts so that last 8 bits represent type
    type = RshiftInt(type, 8);

    return type;
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketGetU()
//
// PURPOSE:      Returns the value of U for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL GrphPacketGetU(UInt32 GrphPkt)
{
    UInt32 U = GrphPkt;

    //clears all the bits except ninth
    U = U & maskInt(9, 9);

    //right shifts so that last bit represent U
    U = RshiftInt(U, 9);

    return (BOOL)U;
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketGetM()
//
// PURPOSE:      Returns the value of M for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//
// RETURN:       BOOL
//---------------------------------------------------------------------------
static BOOL GrphPacketGetM(UInt32 GrphPkt)
{
    UInt32 M = GrphPkt;

    //clears all the bits except tenth
    M = M & maskInt(10, 10);

    //right shifts so that last bit represent M
    M = RshiftInt(M, 10);

    return M;
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketGetResv()
//
// PURPOSE:      Returns the value of reserved for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 GrphPacketGetResv(UInt32 GrphPkt)
{
    UInt32 reserved = GrphPkt;

    //clears all the bits except 12-24
    reserved = reserved & maskInt(12, 24);

    //right shifts so that last bit represent reserved
    reserved = RshiftInt(reserved, 11);

    return reserved;
}


//---------------------------------------------------------------------------
// FUNCTION:     GrphPacketGetHopCount()
//
// PURPOSE:      Returns the value of hopCount for GrphPacket
//
// ARGUMENTS:    GrphPkt, The variable containing the value of type,U,M,
//                        reserved and hopCount
//
// RETURN:       UInt32
//---------------------------------------------------------------------------
static UInt32 GrphPacketGetHopCount(UInt32 GrphPkt)
{
    UInt32 hopCount = GrphPkt;

    //clears all the bits except 25,-32
    hopCount = hopCount & maskInt(25, 32);

    return hopCount;
}


// /**
// STRUCT      :: MaodvStats
// DESCRIPTION :: Structure to store the statistical informations of MAodv
// **//
typedef struct
{
    int numRequestInitiated;
    int numJoinRequestResent;
    int numNonJoinRequestResent;
    int numJoinRequestRelayed;
    int numNonJoinRequestRelayed;

    int numJoinReqInitiated;
    int numNonJoinReqInitiated;
    int numRepairReqInitiated;
    int numJoinReqRecd;
    int numNonJoinReqRecd;
    int numRepairReqRecd;

    int numMreplyForwarded;

    int numRequestRecved;
    int numRequestDuplicate;
    int numRequestTtlExpired;
    int numRequestRecvedAsDest;
    int numReplyInitiatedAsMember;
    int numReplyInitiatedAsNonMember;
    int numReplyForwarded;
    int numReplyInitiatedAsDest;
    int numReplyInitiatedAsIntermediate;
    int numReplyRecved;
    int numReplyRecvedAsSource;

    int numHelloSent;
    int numGroupHelloSent;
    int numGroupHelloRecved;
    int numGroupHelloRelayed;
    int numHelloRecved;

    int numMdataInitiated;
    int numMdataForwarded;
    int numMdataDropped;

    int numDupDataRecd;
    int numMdataDelivered;

    int numMactSent;

    int numDataRecved;
    int numDataDroppedForNoRoute;
    int numDataDroppedForOverlimit;
    int numMdataDroppedAsSource;
} MaodvStats;

// /**
// STRUCT      :: MaodvData
// DESCRIPTION :: maodv main structure to store all necessary informations
//                for MAodv
// **//
typedef struct struct_network_maodv_str
{
    // set of user configurable parameters
    int          netDiameter;
    clocktype    nodeTraversalTime;
    clocktype    myRouteTimeout;
    int          allowedHelloLoss;
    clocktype    activeRouteTimeout;
    int          rreqRetries;
    clocktype    helloInterval;
    int          rtDeletionConstant;
    unsigned int pktId;

    // Multicast Group hello interval
    clocktype         groupHelloInterval;
    MAODV_Membership  memberFlag;
    MAODV_MC          messageCache;

    // set of maodv protocol dependent parameters
    MaodvRoutingTable  routeTable;
    MaodvRreqSeenTable seenTable;

    MaodvNeighborTable neighborTable;
    MaodvMessageBuffer msgBuffer;
    int                bufferSizeInNumPacket;
    int                bufferSizeInByte;
    MaodvRreqSentTable sent;

    MaodvMroutingTable mrouteTable;
    MaodvRequestTable  requestTable;

    MaodvStats stats;
    BOOL       statsCollected;
    BOOL       statsPrinted;
    BOOL       processHello;
    int        seqNumber;
    int        floodingId;
    clocktype  lastBroadcastSent;

    RandomSeed broadcastJitterSeed;
    RandomSeed helloIntervalSeed;
    RandomSeed jitterSeed;
} MaodvData;

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------

void MaodvInit(
    Node *node,
    const NodeInput *nodeInput,
    int interfaceIndex);

//-----------------------------------------------------------------------------
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
//-----------------------------------------------------------------------------
void MaodvHandleProtocolPacket(
    Node *node,
    Message *msg,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    int ttl,
    int interfaceIndex);


//-----------------------------------------------------------------------------
// FUNCTION: MaodvHandleProtocolEvent
//
// PURPOSE: Handles all the protocol events
//
// ARGUMENTS: node, the node received the event
//            msg,  msg containing the event type
//
//-----------------------------------------------------------------------------

void
MaodvHandleProtocolEvent(Node *node,Message *msg);


//-----------------------------------------------------------------------------
// FUNCTION: MaodvFinalize
//
// PURPOSE:  Called at the end of the simulation to collect the results
//
// ARGUMENTS: node, The node for which the statistics are to be printed
//
// RETURN:    None
//-----------------------------------------------------------------------------

void MaodvFinalize(Node *node);

#endif /* _MAODV_H_ */

