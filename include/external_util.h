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
// PACKAGE     :: EXTERNAL_UTILITIES
// DESCRIPTION :: This file describes utilities for external interfaces.
// **/


#ifndef _EXTERNAL_UTIL_H_
#define _EXTERNAL_UTIL_H_

#include "external.h"
#include "network_ip.h"

#define EXTERNAL_MAX_TREE_STORE 1000

//---------------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------------

// /**
// STRUCT       :: EXTERNAL_TreeNode
// DESCRIPTION  :: Structure of each node of a Splaytree
// **/
struct EXTERNAL_TreeNode
{
    clocktype time;
    void* data;

    EXTERNAL_TreeNode* leftPtr;
    EXTERNAL_TreeNode* rightPtr;
    EXTERNAL_TreeNode* parentPtr;
};

// /**
// STRUCT       :: EXTERNAL_Tree
// DESCRIPTION  :: Structure of a Splaytree
// **/
struct EXTERNAL_Tree
{
    EXTERNAL_TreeNode* rootPtr;
    EXTERNAL_TreeNode* leastPtr;
    Int32 heapPos;

    BOOL useStore;
    int maxStore;

    EXTERNAL_TreeNode* store;
    int storeSize;
};

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

// /**
// API       :: EXTERNAL_TreeInitialize
// PURPOSE   :: To initialize the splaytree
// PARAMETERS ::
// + tree     : EXTERNAL_Tree*     : Pointer to the splaytree
// + useStore : BOOL :Use Store
// + maxStore : int : Max Store
// RETURN    :: void :
// **/
void EXTERNAL_TreeInitialize(
    EXTERNAL_Tree* tree,
    BOOL useStore = TRUE,
    int maxStore = EXTERNAL_MAX_TREE_STORE);

// /**
// API       :: SCHED_SplayTreeInsert
// PURPOSE   :: To insert a node into the Splaytree
// PARAMETERS ::
// + tree     : EXTERNAL_Tree*     : Pointer to the splaytree
// + treeNode : EXTERNAL_TreeNode* : Pointer to the splayNode
//                                    to be inserted
// RETURN    :: void :
// **/
void EXTERNAL_TreeInsert(EXTERNAL_Tree* tree, EXTERNAL_TreeNode* treeNode);

// /**
// API       :: EXTERNAL_TreePeekMin
// PURPOSE   :: To look up a node in the Splaytree
// PARAMETERS ::
// + tree     : EXTERNAL_Tree*     : Pointer to the splaytree
// RETURN    :: void :
// **/
EXTERNAL_TreeNode* EXTERNAL_TreePeekMin(EXTERNAL_Tree* tree);

// API       :: SCHED_SplayTreeExtractMin
// PURPOSE   :: To extract a a node from the tree
// PARAMETERS ::
// + node     : EXTERNAL_Tree*     : Pointer to the tree
// RETURN    :: EXTERNAL_TreeNode* : Pointer to extracted node
// **/
EXTERNAL_TreeNode* EXTERNAL_TreeExtractMin(EXTERNAL_Tree* tree);


// API       :: EXTERNAL_TreeAllocateNode
// PURPOSE   :: To allocate a node for the tree
// PARAMETERS ::
// + node     : EXTERNAL_Tree*     : Pointer to the tree
// RETURN    :: EXTERNAL_TreeNode* : Pointer to allocated node
// **/
EXTERNAL_TreeNode* EXTERNAL_TreeAllocateNode(EXTERNAL_Tree* tree);

// API       :: EXTERNAL_TreeFreeNode
// PURPOSE   :: To free a node from the tree
// PARAMETERS ::
// + tree     : EXTERNAL_Tree*     : Pointer to the tree
// + node     : EXTERNAL_TreeNode* : Pointer to allocated node
// RETURN    :: void :
// **/
void EXTERNAL_TreeFreeNode(
    EXTERNAL_Tree* tree,
    EXTERNAL_TreeNode* node);

//---------------------------------------------------------------------------
// Data Structures
//---------------------------------------------------------------------------


// /**
// STRUCT      :: EXTERNAL_ForwardInstantiate
// DESCRIPTION :: Info field used for instantiating a forward app
// **/
struct EXTERNAL_ForwardInstantiate
{
    NodeAddress localAddress;
    NodeAddress remoteAddress;
    char    interfaceName[MAX_STRING_LENGTH];
    int     interfaceId;
    BOOL    isServer;
};

// /**
// STRUCT      :: EXTERNAL_ForwardSendUdpData
// DESCRIPTION :: Info field used for sending a UDP forward app
// **/
struct EXTERNAL_ForwardSendUdpData
{
    NodeAddress localAddress;
    NodeAddress remoteAddress;
    char    interfaceName[MAX_STRING_LENGTH];
    int     interfaceId;
    TosType priority;
    UInt8 messageSize;
    BOOL isVirtual;
    UInt8 ttl;
};

// /**
// STRUCT      :: EXTERNAL_ForwardSendTcpData
// DESCRIPTION :: Info field used for sending a UDP forward app
// **/
struct EXTERNAL_ForwardSendTcpData
{
    NodeAddress fromAddress;
    NodeAddress toAddress;
    int         interfaceId;
    char        interfaceName [MAX_STRING_LENGTH];
    UInt8       ttl;
};

// /**
// STRUCT      :: EXTERNAL_TableRecord
// DESCRIPTION :: A record in the table.  Contains a pointer value and a
//                timestamp, as well as information for maintaining a linked
//                list.
// **/
typedef struct struct_external_table_record_str
{
    char *data;
    clocktype time;

    struct struct_external_table_record_str *next;
    struct struct_external_table_record_str *prev;
} EXTERNAL_TableRecord;

// /**
// STRUCT      :: EXTERNAL_SimulationDurationInfo
// DESCRIPTION :: A duration of simulation time
// **/
typedef struct struct_external_simulation_duration_info_str
{
    clocktype       maxSimClock;
} EXTERNAL_SimulationDurationInfo;


// /**
// STRUCT      :: EXTERNAL_TableOverflow
// DESCRIPTION :: A overflow record.
// **/
struct EXTERNAL_TableOverflow
{
    int size;
    EXTERNAL_TableRecord* records;

    EXTERNAL_TableOverflow* next;
};

// /**
// STRUCT      :: EXTERNAL_Table
// DESCRIPTION :: A table.  Generally used for storing external packet data,
//                but can be used for anything.
// **/
typedef struct struct_external_packet_table_str
{
    int size;
    EXTERNAL_TableRecord* table;

    EXTERNAL_TableOverflow* overflow;

    EXTERNAL_TableRecord* usedRecordList;
    EXTERNAL_TableRecord* emptyRecordList;
} EXTERNAL_Table;

// /**
// STRUCT      :: EXTERNAL_NetworkLayerPacket
// DESCRIPTION :: A packet that will be sent at the network layer.  Created
//                by EXTERNAL_SendDataNetworkLayer, sent by
//                EXTERNAL_SendNetworkLayerPacket
// **/
struct EXTERNAL_NetworkLayerPacket
{
    NodeAddress srcAddr;
    int externalId;
};

//---------------------------------------------------------------------------
// Functions
//---------------------------------------------------------------------------

// /**
// API       :: EXTERNAL_InitializeTable
// PURPOSE   :: This function will initialize the table.  The size parameter
//              represents the number of records that will be allocated in
//              one block.
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// + size : int : The size of the table
// RETURN    :: void :
// **/
void EXTERNAL_InitializeTable(EXTERNAL_Table *table, int size);

// /**
// API       :: EXTERNAL_FinalizeTable
// PURPOSE   :: This function will finalize the table
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// RETURN    :: void :
// **/
void EXTERNAL_FinalizeTable(EXTERNAL_Table *table);

// /**
// API       :: EXTERNAL_GetUnusedRecord
// PURPOSE   :: This function will retrieve an unused record from the table.
//              If the packet table is full it will allocate a new block of
//              records.  The user may fill in the record's contents.  It
//              will never return NULL.
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// RETURN    :: EXTERNAL_TableRecord* : The retrieved record
// **/
EXTERNAL_TableRecord* EXTERNAL_GetUnusedRecord(EXTERNAL_Table *table);

// /**
// API       :: EXTERNAL_GetEarliestRecord
// PURPOSE   :: This function will retrieve the earliest record in the table
//              or NULL if the table is empty.
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// RETURN    :: EXTERNAL_TableRecord* : The retrieved record
// **/
EXTERNAL_TableRecord* EXTERNAL_GetEarliestRecord(EXTERNAL_Table *table);

// /**
// API       :: EXTERNAL_GetEarliestRecord
// PURPOSE   :: This function will check if a data pointer is still in the
//              table.
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// + data : char* : The data to check for
// RETURN    :: BOOL : TRUE if it is in the table, FALSE if not
// **/
BOOL EXTERNAL_IsDataInTable(EXTERNAL_Table *table, char *data);

// /**
// API       :: EXTERNAL_FreeRecord
// PURPOSE   :: This function frees a record previously returned from
//              EXTERNAL_GetUnusedRecord().  The memory contained in the
//              data portion of the record is the user's responsiblity to
//              free.
// PARAMETERS ::
// + table : EXTERNAL_Table* : The table
// RETURN    :: EXTERNAL_TableRecord* : The retrieved record
// **/
void EXTERNAL_FreeRecord(EXTERNAL_Table *table, EXTERNAL_TableRecord *rec);

// /**
// API       :: EXTERNAL_SendDataAppLayerUDP
// PURPOSE   :: Sends data originating from the app layer using UDP.  When
//              the packet reaches its destination it will call the forward
//              function of the external interface, if it exists.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the sending node
// + to : NodeAddress : The address of the receiving node
// + data : char* : The data that is to be sent.  This may be NULL if there
//                  is no specific data, in which case the dataSize
//                  parameter is used but no actual data is sent.
// + dataSize : int : The size of the data
// + timestamp : clocktype : The time to send this message.  Pass 0 to send
//                           at the interface's current time according to
//                           its time function.  If no time function, sends
//                           immediately.
// + app : AppType : The application to send to, defaults to APP_FORWARD
// + trace : TraceProtocolType : The trace protocol, defaults to TRACE_FORWARD
// + priority : TosType : The priority to send this message at
// RETURN    :: void : None
// **/
void EXTERNAL_SendDataAppLayerUDP(
    EXTERNAL_Interface *iface,
    NodeAddress from,
    NodeAddress to,
    char *data,
    int dataSize,
    clocktype timestamp = 0,
    AppType app = APP_FORWARD,
    TraceProtocolType trace = TRACE_FORWARD,
    TosType priority = IPTOS_PREC_ROUTINE,
    UInt8 ttl = TTL_NOT_SET);

void EXTERNAL_SendDataAppLayerUDP(
    EXTERNAL_Interface *iface,
    Node* srcNode,
    NodeAddress srcAddress,
    Node* destNode,
    NodeAddress destAddress,
    char *data,
    int dataSize,
    clocktype timestamp,
    AppType app,
    TraceProtocolType trace,
    TosType priority,
    UInt8 ttl = TTL_NOT_SET);

// /**
// API       :: EXTERNAL_SendDataAppLayerUDP
// PURPOSE   :: Sends virtual data originating from the app layer using UDP.
//              When the packet reaches its destination it will call the forward
//              forward function of the external interface, if it exists.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the sending node
// + to : NodeAddress : The address of the receiving node
// + header : char* : The header that is to be sent. 
//                    No actual data is sent.
// + headerSize : int : The size of the header
// + virtualDataSize : int : The size of the virtual data
// + timestamp : clocktype : The time to send this message.  Pass 0 to send
//                           at the interface's current time according to
//                           its time function.  If no time function, sends
//                           immediately.
// + app : AppType : The application to send to, defaults to APP_FORWARD
// + trace : TraceProtocolType : The trace protocol, defaults to TRACE_FORWARD
// + priority : TosType : The priority to send this message at. defaults to IPTOS_PREC_ROUTINE
// RETURN    :: void : None
// **/
void EXTERNAL_SendVirtualDataAppLayerUDP(
    EXTERNAL_Interface *iface,
    NodeAddress from,
    NodeAddress to,
    char *header,
    int headerSize,
    int virtualDataSize,
    clocktype timestamp = 0,
    AppType app = APP_FORWARD,
    TraceProtocolType trace = TRACE_FORWARD,
    TosType priority = IPTOS_PREC_ROUTINE,
    UInt8 ttl = TTL_NOT_SET);

void EXTERNAL_SendVirtualDataAppLayerUDP(
    EXTERNAL_Interface *iface,
    Node* srcNode,
    NodeAddress srcAddress,
    NodeAddress destAddress,
    char *header,
    int headerSize,
    int virtualDataSize,
    clocktype timestamp,
    AppType app,
    TraceProtocolType trace,
    TosType priority,
    UInt8 ttl = TTL_NOT_SET);


// /**
// API       :: EXTERNAL_SendDataAppLayerTCP
// PURPOSE   :: Sends data originating from the app layer using TCP.  When
//              the last byte of data reaches its destination it will call
//              the forward function of the external interface, if it
//              exists.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the sending node
// + to : NodeAddress : The address of the receiving node
// + data : char* : The data that is to be sent.  This may be NULL if there
//                  is no specific data, in which case the dataSize
//                  parameter is used but no actual data is sent.
// + dataSize : int : The size of the data
// + timestamp : clocktype : The time to send this message.  Pass 0 to send
//                           immediately.  Defaults to currentSimTime + 1.
// RETURN    :: void :
// **/
void EXTERNAL_SendDataAppLayerTCP(
    EXTERNAL_Interface *iface,
    NodeAddress from,
    NodeAddress to,
    char *data,
    int dataSize,
    clocktype timestamp = 0,
    UInt8 ttl = TTL_NOT_SET);

void EXTERNAL_SendVirtualDataAppLayerTCP(
    EXTERNAL_Interface *iface,
    NodeAddress from,
    NodeAddress to,
    char *header,
    int dataSize,
    int virtualDataSize,
    clocktype timestamp = 0,
    UInt8 ttl = TTL_NOT_SET);

// /**
// API       :: EXTERNAL_SendDataNetworkLayer
// PURPOSE   :: Sends data originating from network layer.  No provisions
//              are made for handling this data once it enters the QualNet
//              network.  This is the responsibility of the external
//              interface or protocols the data is sent to.
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the node that will send the
//                       message.  Where the packet is sent from within the
//                       QualNet simulation.
// + srcAddr : NodeAddress : The IP address of the node originally
//                           creating the packet.  May be different than
//                           the from address.
// + destAddr : NodeAddress : The address of the receiving node
// + tos : TosType : The Type of Service field in the IP header
// + protocol : unsigned char : The protocol field in the IP header
// + ttl : unsigned int : The Time to Live field in the IP header
// + payload : char* : The data that is to be sent.  This should include
//                     appropriate transport headers.  If NULL the payload
//                     will consist of 0s.
// + payloadSize : int : The size of the data
// + timestamp : clocktype : The time to send this packet.  Pass 0 to send
//                           at the interface's current time according to
//                           the interface's time function (If the interface
//                           doesn't have a time function, the packet is
//                           sent immediately.
// RETURN    :: void : None
// **/
void EXTERNAL_SendDataNetworkLayer(
    EXTERNAL_Interface* iface,
    NodeAddress from,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    TosType tos,
    unsigned char protocol,
    unsigned int ttl,
    char* payload,
    int payloadSize,
    clocktype timestamp = 0);

#ifdef IPNE_INTERFACE
// /**
// API       :: EXTERNAL_SendDataNetworkLayerOnInterface
// PURPOSE   :: Sends data originating from network layer on a specific 
//              interface of the node. No provisions
//              are made for handling this data once it enters the QualNet
//              network.  This is the responsibility of the external
//              interface or protocols the data is sent to.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the node that will send the
//                       message.  Where the packet is sent from within the
//                       QualNet simulation.
// + srcAddr : NodeAddress : The IP address of the node originally
//                           creating the packet.  May be different than
//                           the from address.
// + destAddr : NodeAddress : The address of the receiving node
// + identification : unsigned short : The identification field in the IP
//                                     header
// + dontFragment : BOOL : Whether to set the dont fragment bit in the IP
//                         header
// + moreFragments : BOOL : Whether to set the more fragments bit in the IP
//                          header
// + fragmentOffset : unsigned short : The fragment offset field in the IP
//                                     header
// + tos : TosType : The Type of Service field in the IP header
// + protocol : unsigned char : The protocol field in the IP header
// + ttl : unsigned int : The Time to Live field in the IP header
// + payload : char* : The data that is to be sent.  This should include
//                     appropriate transport headers.  If NULL the payload
//                     will consist of 0s.
// + payloadSize : int : The size of the data
// + interfaceIndex : int : The interface index
// + timestamp : clocktype : The time to send this packet.  Pass 0 to send
//                           at the interface's current time according to
//                           the interface's time function (If the interface
//                           doesn't have a time function, the packet is
//                           sent immediately.
// RETURN    :: void
// **/
void EXTERNAL_SendDataNetworkLayerOnInterface(
    EXTERNAL_Interface* iface,
    NodeAddress from,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    unsigned short identification,
    BOOL dontFragment,
    BOOL moreFragments,
    unsigned short fragmentOffset,
    TosType tos,
    unsigned char protocol,
    unsigned int ttl,
    char* payload,
    int payloadSize,
    int interfaceIndex,
    clocktype timestamp = 0);

#endif

// /**
// API       :: EXTERNAL_SendVirtualDataNetworkLayer
// PURPOSE   :: Sends data originating from network layer.  No provisions
//              are made for handling this data once it enters the QualNet
//              network.  This is the responsibility of the external
//              interface or protocols the data is sent to.
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the node that will send the
//                       message.  Where the packet is sent from within the
//                       QualNet simulation.
// + srcAddr : NodeAddress : The IP address of the node originally
//                           creating the packet.  May be different than
//                           the from address.
// + destAddr : NodeAddress : The address of the receiving node
// + tos : TosType : The Type of Service field in the IP header
// + protocol : unsigned char : The protocol field in the IP header
// + ttl : unsigned int : The Time to Live field in the IP header
// + payload : char* : The data that is to be sent.  This should include
//                     appropriate transport headers.  If NULL the payload
//                     will consist of 0s.
// + dataSize : int : The size of the data
// + virtualSize : int : The size of the virtual data
// + timestamp : clocktype : The time to send this packet.  Pass 0 to send
//                           at the interface's current time according to
//                           the interface's time function (If the interface
//                           doesn't have a time function, the packet is
//                           sent immediately.
// RETURN    :: void : None
// **/
void EXTERNAL_SendVirtualDataNetworkLayer(
    EXTERNAL_Interface* iface,
    NodeAddress from,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    TosType tos,
    unsigned char protocol,
    unsigned int ttl,
    char* payload,
    int dataSize,
    int virtualSize,
    clocktype timestamp = 0);

// /**
// API       :: EXTERNAL_SendDataNetworkLayer
// PURPOSE   :: Sends data originating from network layer.  No provisions
//              are made for handling this data once it enters the QualNet
//              network.  This is the responsibility of the external
//              interface or protocols the data is sent to.
// PARAMETERS::
// + iface : EXTERNAL_Interface* : The external interface
// + from : NodeAddress : The address of the node that will send the
//                       message.  Where the packet is sent from within the
//                       QualNet simulation.
// + srcAddr : NodeAddress : The IP address of the node originally
//                           creating the packet.  May be different than
//                           the from address.
// + destAddr : NodeAddress : The address of the receiving node
// + identification : unsigned short : The identification field in the IP
//                                     header
// + dontFragment : BOOL : Whether to set the dont fragment bit in the IP
//                         header
// + moreFragments : BOOL : Whether to set the more fragments bit in the IP
//                          header
// + fragmentOffset : unsigned short : The fragment offset field in the IP
//                                     header
// + tos : TosType : The Type of Service field in the IP header
// + protocol : unsigned char : The protocol field in the IP header
// + ttl : unsigned int : The Time to Live field in the IP header
// + payload : char* : The data that is to be sent.  This should include
//                     appropriate transport headers.  If NULL the payload
//                     will consist of 0s.
// + payloadSize : int : The size of the data
// + ipHeaderLength : int : length of the IP Header including options if any
// + ipOptions : char* : pointer to the IP Option.
// + timestamp : clocktype : The time to send this packet.  Pass 0 to send
//                           at the interface's current time according to
//                           the interface's time function (If the interface
//                           doesn't have a time function, the packet is
//                           sent immediately.
// RETURN    :: void : None
// **/
void EXTERNAL_SendDataNetworkLayer(
    EXTERNAL_Interface* iface,
    NodeAddress from,
    NodeAddress srcAddr,
    NodeAddress destAddr,
    unsigned short identification,
    BOOL dontFragment,
    BOOL moreFragments,
    unsigned short fragmentOffset,
    TosType tos,
    unsigned char protocol,
    unsigned int ttl,
    char* payload,
    int payloadSize,
    int ipHeaderLength,
    char *ipOptions,
    clocktype timestamp = 0,
    NodeAddress nextHopAddr = INVALID_ADDRESS);

void EXTERNAL_SendDataDuringWarmup(EXTERNAL_Interface* iface, Node* node,
                                   Message* msg, BOOL *pktDrop);
// /**
// API       :: EXTERNAL_SendNetworkLayerPacket
// PURPOSE   :: Sends the packet from EXTERNAL_SendDataNetworkLayer after
//              some delay.  This function should never be called directly.
// PARAMETERS ::
// + node : Node* : The node sending the packet
// + msg : Message* : The message
// RETURN    :: void : None
// **/
void EXTERNAL_SendNetworkLayerPacket(Node* node, Message* msg);

// /**
// API       :: EXTERNAL_CreateMapping
// PURPOSE   :: Creates a mapping between a key and a value.
//              The key may be any value and any length, such as an IP
//              address, a MAC address, or a generic string.  The value may
//              be anything and is the responsibility of the user.  Memory
//              will be allocated for the key and the value.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + key : char* : The address of the key
// + keySize : int : The size of the key in bytes
// + value : char* : The address of what the value maps to
// + valueSize : int : The size of the value in bytes
// RETURN    :: void : None
// **/
void EXTERNAL_CreateMapping(
    EXTERNAL_Interface *iface,
    char *key,
    int keySize,
    char *value,
    int valueSize);

// /**
// API       :: EXTERNAL_ResolveMapping
// PURPOSE   :: Resolves a mapping created by EXTERNAL_CreateMapping.  If it
//              exists it is placed in the value and valueSize parameters
//              and returns 0.  The returned value will point to the memory
//              block allocated by EXTERNAL_CreateMapping.  If it does not
//              exist it returns non-zero and the value and valueSize
//              parameters are invalid.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + key : char* : Pointer to the key
// + keySize : int : The size of the key in bytes
// + value : char** : Pointer to the value (output)
// + valueSize : int* : The size of the key in bytes (output)
// RETURN    :: int : 0 if the mapping resolved, non-zero if it did not
// **/
int EXTERNAL_ResolveMapping(
    EXTERNAL_Interface *iface,
    char *key,
    int keySize,
    char **value,
    int *valueSize);

// /**
// API       :: EXTERNAL_DeleteMapping
// PURPOSE   :: Deletes a mapping created by EXTERNAL_CreateMapping.  
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + key : char* : Pointer to the key
// + keySize : int : The size of the key in bytes
// RETURN    :: int : 0 if the mapping resolved, non-zero if it did not
// **/
void EXTERNAL_DeleteMapping(
    EXTERNAL_Interface *iface,
    char *key,
    int keySize);

/*
EXTERNAL_Measurements
*/

// /**
// ENUM        :: ExternalScheduleType
// DESCRIPTION :: Enumeration of allowed scheduling operations - e.g.
//                EXTERNAL_ActivateNode
// **/
typedef enum {
    EXTERNAL_SCHEDULE_STRICT,
    EXTERNAL_SCHEDULE_SAFE,     // Best effort, but adjust to safeTime if needed.
    EXTERNAL_SCHEDULE_LOOSELY
} ExternalScheduleType;

// /**
// API       :: EXTERNAL_ActivateNode
// PURPOSE   :: Activate a node so that it can begin processing events.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// RETURN    :: void : None
// **/
void EXTERNAL_ActivateNode(EXTERNAL_Interface *iface, Node *node,
    ExternalScheduleType scheduling = EXTERNAL_SCHEDULE_STRICT,
    clocktype activationEventTime = -1);

void EXTERNAL_ActivateNodeInterface(EXTERNAL_Interface *iface, Node *node,
    ExternalScheduleType scheduling = EXTERNAL_SCHEDULE_STRICT,
    clocktype activationEventTime = -1, int interfaceIndex = 0);

// /**
// API       :: EXTERNAL_DectivateNode
// PURPOSE   :: Dectivate a node so that it stops processing events.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// RETURN    :: void : None
// **/
void EXTERNAL_DeactivateNode(EXTERNAL_Interface *iface, Node *node,
    ExternalScheduleType scheduling = EXTERNAL_SCHEDULE_STRICT,
    clocktype deactivationEventTime = -1);

void EXTERNAL_DeactivateNodeInterface(EXTERNAL_Interface *iface, Node *node,
    ExternalScheduleType scheduling = EXTERNAL_SCHEDULE_STRICT,
    clocktype deactivationEventTime = -1, int interfaceNum = 0);

struct EXTERNAL_SetPhyTxPowerInfo
{
    double          txPower;
    int             phyIndex;
    NodeAddress     nodeId;
};

// /**
// API       :: EXTERNAL_PHY_SetTxPower
// PURPOSE   :: Just like PHY_SetTxPower (), but able to handle setting
//              transmission power when node is owned by a remote partition.
//              Change to TxPower will be scheduled as "best-effort" for
//              remote nodes.
//              The range of coordinate values depends on the terrain data.
// PARAMETERS ::
// + node : Node* : The node (can be either a local node or remote)
// + phyIndex : int : The physical index
// + newTxPower : double : The new transmission power.
// RETURN    :: void : None
// **/
void EXTERNAL_PHY_SetTxPower (Node * node, int phyIndex, double newTxPower);

// /**
// API       :: EXTERNAL_PHY_GetTxPower
// PURPOSE   :: Just like PHY_GetTxPower (), but able to handle getting
//              transmission power when node is owned by a remote partition.
// PARAMETERS ::
// + node : Node* : The node (can be either a local node or remote)
// + phyIndex : int : The physical index
// + txPowerPtr : double * : (OUT) value of transmission power will be
//                           stored here.
// RETURN    :: void : None
// **/
void EXTERNAL_PHY_GetTxPower (Node * node, int phyIndex, double * txPowerPtr);

// /**
// API       :: EXTERNAL_ChangeNodePosition
// PURPOSE   :: Change the position of a node.  This function will work
//              using both coordinate systems.  Orientation is not changed.
//              Coordinate values are checked to be in the proper range, and
//              are converted if they are not.  The range of coordinate
//              values depends on the terrain data.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + c1 : double : The first coordinate:  latitude  or x
// + c2 : double : The second coordinate: longitude or y
// + c3 : double : The third coordinate:  altitude  or z
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodePosition(
    EXTERNAL_Interface *iface,
    Node *node,
    double c1,
    double c2,
    double c3,
    clocktype delay);

void EXTERNAL_ChangeNodePositionAtSimTime(
    EXTERNAL_Interface *iface,
    Node *node,
    double c1,
    double c2,
    double c3,
    clocktype mobilityEventTime);

// /**
// API       :: EXTERNAL_ChangeNodeOrientation
// PURPOSE   :: Change the orientation of a node.  Position is not changed.
//              Azimuth/elevation are checked to be in the proper range, and
//              are converted if they are not.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + azimuth : short : The azimuth, 0 <= azimuth <= 359
// + elevation : short : The elevation, -180 <= elevation <= 180
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodeOrientation(
    EXTERNAL_Interface *iface,
    Node *node,
    short azimuth,
    short elevation);

// /**
// API       :: EXTERNAL_ChangeNodePositionAndOrientation
// PURPOSE   :: Change both the position and orientation of a node.  This
//              function will work using both coordinate systems.
//              Coordinate values and Azimuth/elevation values are checked
//              to be in the proper range, and are converted if they are not.
//              The range of coordinate values depends on the terrain data.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + c1 : double : The first coordinate:  latitude  or x
// + c2 : double : The second coordinate: longitude or y
// + c3 : double : The third coordinate:  altitude  or z
// + azimuth : short : The azimuth, 0 <= azimuth <= 359
// + elevation : short : The elevation, -180 <= elevation <= 180
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodePositionAndOrientation(
    EXTERNAL_Interface *iface,
    Node *node,
    double c1,
    double c2,
    double c3,
    short azimuth,
    short elevation);

// /**
// API       :: EXTERNAL_ChangeNodePositionOrientationAndSpeedAtTime
// PURPOSE   :: Change the position, orientation, and speed of a node at a
//              user-specified time.  This function will work using both
//              coordinate systems.
//              Coordinate values, azimuth/elevation, and speed values are
//              checked to be in the proper range, and are converted if they
//              are not.
//              The range of coordinate values depends on the terrain data.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + mobilityEventTime : clocktype : The absolute simulation time (not delay)
// at which the mobility event should execute
// + c1 : double : The first coordinate:  latitude  or x
// + c2 : double : The second coordinate: longitude or y
// + c3 : double : The third coordinate:  altitude  or z
// + azimuth : short : The azimuth, 0 <= azimuth <= 359
// + elevation : short : The elevation, -180 <= elevation <= 180
// + speed : double : The speed in m/s
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodePositionOrientationAndSpeedAtTime(
    EXTERNAL_Interface *iface,
    Node *node,
    clocktype mobilityEventTime,
    double c1,
    double c2,
    double c3,
    short azimuth,
    short elevation,
    double speed);

// /**
// API       :: EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime
// PURPOSE   :: Update the position, orientation, and velocity vector of a
//              node at a user-specified time.  The velocity vector is
//              expected to be in the same distance units used for the
//              the position, per one second.  The speed parameter must also
//              be provided, accurate for the provided velocity vector, and
//              always in meters per second.
//              Coordinate values, azimuth/elevation, and speed values are
//              checked to be in the proper range, and are converted if they
//              are not.
//              The range of coordinate values depends on the terrain data.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + mobilityEventTime : clocktype : The absolute simulation time (not delay)
// at which the mobility event should execute
// + c1 : double : The first coordinate:  latitude  or x
// + c2 : double : The second coordinate: longitude or y
// + c3 : double : The third coordinate:  altitude  or z
// + azimuth : short : The azimuth, 0 <= azimuth <= 359
// + elevation : short : The elevation, -180 <= elevation <= 180
// + speed : double : The speed in m/s
// + c1Speed : double : The rate of change of the first coordinate in the
//                      units of the position, per second
// + c2Speed : double : The rate of change of the second coordinate in the
//                      units of the position, per second
// + c3Speed : double : The rate of change of the third coordinate in the
//                      units of the position, per second
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime(
    EXTERNAL_Interface *iface,
    Node *node,
    clocktype mobilityEventTime,
    double c1,
    double c2,
    double c3,
    short azimuth,
    short elevation,
    double speed,
    double c1Speed,
    double c2Speed,
    double c3Speed);

// /**
// API       :: EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime
// PURPOSE   :: Update the position, orientation, and velocity vector of a
//              node at a user-specified time.  The velocity vector is
//              expected to be in the same distance units used for the
//              the position, per one second.
//              Coordinate values, azimuth/elevation, and speed values are
//              checked to be in the proper range, and are converted if they
//              are not.
//              The range of coordinate values depends on the terrain data.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + mobilityEventTime : clocktype : The absolute simulation time (not delay)
// at which the mobility event should execute
// + c1 : double : The first coordinate:  latitude  or x
// + c2 : double : The second coordinate: longitude or y
// + c3 : double : The third coordinate:  altitude  or z
// + azimuth : short : The azimuth, 0 <= azimuth <= 359
// + elevation : short : The elevation, -180 <= elevation <= 180
// + c1Speed : double : The rate of change of the first coordinate in the
//                      units of the position, per second
// + c2Speed : double : The rate of change of the second coordinate in the
//                      units of the position, per second
// + c3Speed : double : The rate of change of the third coordinate in the
//                      units of the position, per second
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime(
    EXTERNAL_Interface *iface,
    Node *node,
    clocktype mobilityEventTime,
    double c1,
    double c2,
    double c3,
    short azimuth,
    short elevation,
    double c1Speed,
    double c2Speed,
    double c3Speed);

// /**
// API       :: EXTERNAL_ChangeNodeVelocityAtTime
// PURPOSE   :: Update the velocity vector of a node at a user-specified time.
//              The velocity vector is expected to be in the same distance
//              units used for the terrain, per one second.  The speed
//              parameter must also be provided, accurate for the provided
//              velocity vector, and always in meters per second.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + mobilityEventTime : clocktype : The absolute simulation time (not delay)
//                                  at which the mobility event should execute
// + speed : double : The speed in m/s
// + c1Speed : double : The rate of change of the first coordinate in the
//                      units of the terrain coordinate system, per second
// + c2Speed : double : The rate of change of the second coordinate in the
//                      units of the terrain coordinate system, per second
// + c3Speed : double : The rate of change of the third coordinate in the
//                      units of the terrain coordinate system, per second
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodeVelocityAtTime(
    EXTERNAL_Interface *iface,
    Node *node,
    clocktype mobilityEventTime,
    double speed,
    double c1Speed,
    double c2Speed,
    double c3Speed);

// /**
// API       :: EXTERNAL_ChangeNodeVelocityAtTime
// PURPOSE   :: Update the velocity vector of a node at a user-specified time.
//              The velocity vector is expected to be in the same distance
//              units used for the terrain, per one second.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// + node : Node* : The node
// + mobilityEventTime : clocktype : The absolute simulation time (not delay)
//                                  at which the mobility event should execute
// + c1Speed : double : The rate of change of the first coordinate in the
//                      units of the terrain coordinate system, per second
// + c2Speed : double : The rate of change of the second coordinate in the
//                      units of the terrain coordinate system, per second
// + c3Speed : double : The rate of change of the third coordinate in the
//                      units of the terrain coordinate system, per second
// RETURN    :: void : None
// **/
void EXTERNAL_ChangeNodeVelocityAtTime(
    EXTERNAL_Interface *iface,
    Node *node,
    clocktype mobilityEventTime,
    double c1Speed,
    double c2Speed,
    double c3Speed);

// /**
// API       :: EXTERNAL_ConfigStringPresent
// PURPOSE   :: This function will check the config file for a string.
//              Typically this is used during interface registration to see
//              if the interface is turned on in the config file.
// PARAMETERS ::
// + nodeInput : NodeInput* : The configuration file
// + string : char* : The string to check for
// RETURN    :: BOOL : TRUE if the string is present, FALSE otherwise
// **/
BOOL EXTERNAL_ConfigStringPresent(
    NodeInput *nodeInput,
    const char *string);

// /**
// API       :: EXTERNAL_ConfigStringIsYes
// PURPOSE   :: This function will check the config file for a string.
//              Typically this is used during interface registration to see
//              if the interface is turned on in the config file.  Checks
//              that the string is YES.
// PARAMETERS ::
// + nodeInput : NodeInput* : The configuration file
// + string : char* : The string to check for
// RETURN    :: BOOL : TRUE if the string is YES, FALSE otherwise
// **/
BOOL EXTERNAL_ConfigStringIsYes(
    NodeInput *nodeInput,
    char *string);

void EXTERNAL_MESSAGE_SendAnyNode (PartitionData * partitionData, Node * node,
    Message * msg, clocktype delay, ExternalScheduleType scheduling);

void EXTERNAL_MESSAGE_SendAnyNode(
    EXTERNAL_Interface* iface,
    Node* node,
    Message* msg,
    clocktype delay,
    ExternalScheduleType scheduling);
// /**
// API       :: EXTERNAL_MESSAGE_RemoteSend
// PURPOSE   :: Send a message to the external interface on a different partition.
//              This function makes it possible for your external interface to
//              send a message to your external interface that is on
//              on a different/remote partition.
//              You will then need to add your message handler into the function
//              EXTERNAL_ProcessEvent ().
//              Lastly, you can request a best-effort delivery of your message
//              to the remote external interface by passing in a delay value of 0
//              and a scheduling type of EXTERNAL_SCHEDULE_LOOSELY. Be aware that
//              best-effort messages may be scheduled at slightly different
//              simulation times each time your run your simulation.
//              Further notes about scheduling. If your external event won't
//              result in additional qualnet events, except those that will
//              be scheduled after safe time, then you can use LOOSELY. If,
//              your event is going to schedule additional qualnet event though,
//              then you must use EXTERNAL_SCHEDULE_SAFE (so that the event
//              is delayed to the next safe time). If you violate safe time
//              you will get assertion failures for safe time of signal receive
//              time.
//
// PARAMETERS::
// + iface : EXTERNAL_Interface* : Your external interface
// + destinationPartitionId : int : The partitionId that you want to send to
// + msg : Message * : The external message to send
// + delay : clocktype : When the message should be scheduled on the remote partion.
//                       Make certain that if you don't use Loose scheduling that
//                       you don't violate safe time.
// + scheduling : ExternalScheduleType : Whether this event can be executed lossely
// RETURN    :: void : None
// **/
void EXTERNAL_MESSAGE_RemoteSend (
    EXTERNAL_Interface* iface,
    int destinationPartitionId,
    Message* msg,
    clocktype delay,
    ExternalScheduleType scheduling);

void EXTERNAL_MESSAGE_RemoteSend (
    PartitionData* partition,
    int destinationPartitionId,
    Message* msg,
    clocktype delay,
    ExternalScheduleType scheduling);

// /**
// API       :: EXTERNAL_SetSimulationEndTime
// PURPOSE   :: This function is a means to programatically set the
//              end of the simulation.  The endTime argument can be omitted,
//              in which case the endTime is the current simulation time.
//              If the requested time has already passed, the simulation will
//              end as soon as possible.
// PARAMETERS ::
// + partitionData : partitionData* : pointer to data for this partition
// + endTime : clocktype : The simulation time to end at.
// RETURN    :: void : None
// **/
void EXTERNAL_SetSimulationEndTime(
    EXTERNAL_Interface* iface,
    clocktype endTime = 0);

// /**
// API       :: EXTERNAL_QueryRealTime
// PURPOSE   :: This function will return the wall clock time in the qualnet
//              time format.  NOTE: Interfaces that are running in real-time
//              should not use this function to check the simulation time.
//              The simulation time will not be the same as real time if the
//              simulation was paused.  Use the interface's time function
//              instead.
// PARAMETERS ::
// RETURN    :: clocktype : The real time, not adjusted for simulation pauses.
// **/
clocktype EXTERNAL_QueryRealTime();

// /**
// API       :: EXTERNAL_QueryRealTime
// PURPOSE   :: This function will return the wall clock time in the qualnet
//              time format.  NOTE: Interfaces that are running in real-time
//              should not use this function to check the simulation time.
//              The simulation time will not be the same as real time if the
//              simulation was paused.  Use the interface's time function
//              instead.
// PARAMETERS ::
// RETURN    :: clocktype : The real time, adjusted for simulation pauses.
// **/
clocktype EXTERNAL_QueryRealTime(EXTERNAL_Interface* iface);

// /**
// API       :: EXTERNAL_QueryCPUTime
// PURPOSE   :: This function will return the amount of Cpu time used by
//              QualNet.  The first call to this function will by an
//              interface will return 0, and timing will begin from that
//              point.
// PARAMETERS ::
// + iface : EXTERNAL_Interface* : The external interface
// RETURN    :: clocktype : The CPU time
// **/
clocktype EXTERNAL_QueryCPUTime(EXTERNAL_Interface *iface);

// /**
// API       :: EXTERNAL_Sleep
// PURPOSE   :: This function will sleep for a minimum amount of time as
//              indicated by the amount parameter.  Depending on which
//              platform it is called on the amount of time spent sleeping
//              could be greater.
// PARAMETERS ::
// + amount : clocktype : The amount of time to sleep
// RETURN    :: void :
// **/
void EXTERNAL_Sleep(clocktype amount);

#endif /* _EXTERNAL_UTIL_H_ */
