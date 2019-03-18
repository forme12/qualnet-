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
// PACKAGE :: NODE
// DESCRIPTION ::
//      This file defines the Node data structure and some generic
//      operations on nodes.
// **/

#ifndef NODE_H
#define NODE_H

#include "clock.h"
#include "main.h"

#ifdef WIRELESS_LIB
#include "battery_model.h" 
#endif //WIRELESS_LIB


#include "user.h"
#include "application.h"
#include "coordinates.h"
#include "gui.h"
#include "mac.h"
#include "message.h"
#include "terrain.h"
#include "mobility.h"
#include "network.h"
#include "phy.h"
#include "propagation.h"
#include "splaytree.h"
#include "trace.h"
#include "transport.h"
#include "external.h"
#ifdef JNE_BLACKSIDE_INTEROP_INTERFACE
#include "external_socket.h"
#endif
#include "scheduler_types.h"
#include "atm_layer2.h"
#include "adaptation.h"

#ifdef CELLULAR_LIB
#include "cellular_gsm.h"
#endif // CELLULAR_LIB

#if defined(SATELLITE_LIB)
#include <string>
#include <map>
#endif // SATELLITE_LIB

#ifdef CYBER_LIB
#include "os_resource_manager.h"
#include "firewall_model.h"
#endif // CYBER_LIB

#ifdef EXATA
#include "socketlayer.h"
#endif

#ifdef AGI_INTERFACE
#include "agi_interface.h"
#endif
#ifdef NETSNMP_INTERFACE
class NetSnmpAgent;
#endif

#ifdef LTE_LIB
typedef struct struct_epc_data EpcData;
#endif // LTE_LIB


// /**
// ENUM :: NodeGlobalIndex
// DESCRIPTION ::
//  This enumeration contains indexes into the nodeGlobal array
//  used for module data. For example, nodeGlobal[NodeGlobal_JNE]
//  points to a JneData structure that stores data related to
//  JNE interfaces and applications.
// **/
enum GlobalDataIndex
{
    GlobalData_JNE = 0,
    GlobalData_Count = 4 // leave some room for additional data entries
};

// /**
// STRUCT :: Node
// DESCRIPTION ::
//  This struct includes all the information for a particular node.
//  State information for each layer can be accessed from this structure.
// **/

struct Node {
    // Information about other nodes in the same partition.
    Node      *prevNodeData;
    Node      *nextNodeData;

    //! nodeIndex will store a value from 0 to (the number of nodes - 1);
    //! each node has a unique nodeIndex (even across multiple partitions).
    //! A node keeps the same nodeIndex even if it becomes handled by
    //! another partition.  nodeIndex should not be used by the protocol
    //! code at any layer.
    unsigned    nodeIndex;
	

    NodeAddress nodeId;    //!< the user-specified node identifier
    char*       hostname;  //!< hostname (Default: "hostN" where N is nodeId).

	

    Int32       globalSeed;
    Int32       numNodes;  //!< number of nodes in the simulation

    SplayTree splayTree;
    clocktype timeValue;

    clocktype* currentTime;
    clocktype* startTime;

    BOOL          packetTrace;
    unsigned      packetTraceSeqno;

    BOOL          guiOption;

    PartitionData* partitionData;
    int            partitionId;

    int*          lookaheadCalculatorIndices;

    int           numberChannels;
    int           numberPhys;
    int           numberInterfaces;

    MobilityData* mobilityData;

    //
    // End QualNet kernel context
    //

    // Users should not modify anything above this line.

    // Layer-specific information for the  node.
    PropChannel*    propChannel;
    PropData*       propData;
    PhyData**       phyData;             // phy layer
    MacData**       macData;             // MAC layer
    MacSwitch*      switchData;          // MAC switch
	
	bool isSourceNode;
	int framenum;
	bool change;

    NetworkData     networkData;         // network layer
    TransportData   transportData;       // transport layer
    AppData         appData;             // application layer
    TraceData*      traceData;           // tracing
    SchedulerInfo*  schedulerInfo;       // Pointer to the info struct for the
                                         // scheduler to be used with this node
    UserData*       userData;            // User Data
    void* globalData[GlobalData_Count];  // Global Data

    int             numAtmInterfaces;    // Number of atm interfaces
    AtmLayer2Data** atmLayer2Data;       // ATM LAYER2
    AdaptationData  adaptationData;      // ADAPTATION Layer

#ifdef AGI_INTERFACE
    // AGI STK interface
    AgiData         agiData;
#endif

    int currentInterface;
    static const int InterfaceNone = -1;

    void enterInterface(int intf);

    void exitInterface();

    int ifidx();

#ifdef WIRELESS_LIB
    Battery*        battery;
    float*          hwTable;
#endif //WIRELESS_LIB

#ifdef ADDON_BOEINGFCS
    // Spectrum Manager Code
    BOOL isEdgeNode;
    BOOL isSpectrumManager;
    BOOL usapManager;

    // MA interface Code
    BOOL mAEnabled;
#endif /* ADDON_BOEINGFCS */

#ifdef EXATA
    SLData*     slData;

#ifdef IPNE_INTERFACE
    BOOL isIpneNode;                        // is IPNE node? 
#endif 

#ifdef GATEWAY_INTERFACE
    NodeAddress internetGateway;            // which node is internet gateway
#endif

#endif  // EXATA

#ifdef HITL_INTERFACE
    NodeAddress hitlGateway;
    BOOL isHitlNode;                        // is HITL node? 
#endif //HITL_INTERFACE

#ifdef JNE_BLACKSIDE_INTEROP_INTERFACE
    EXTERNAL_Socket jsrSocket;
    BOOL    isJsrNode;
    unsigned short  jsrSeq;
#endif

#ifdef CELLULAR_LIB
    GSMNodeParameters *gsmNodeParameters;
#endif // CELLULAR_LIB

#if defined(SATELLITE_LIB)
    std::map<std::string,void*> localMap;
#endif // SATELLITE_LIB

    clocktype* lastGridPositionUpdate;
    int* currentGridBoxNumber;

    // STATS DB CODE
#ifdef ADDON_DB
    MetaDataStruct* meta_data;
#endif

#ifdef CYBER_LIB
    BOOL eavesdrop;
    OSResourceManager* resourceManager;
    FirewallModel* firewallModel;
#endif

#ifdef ADDON_ABSTRACT
    google::dense_hash_map<NodeAddress, clocktype>* oneHopNeighbors;
    clocktype lastNeighborUpdate;
    clocktype neighborUpdateInterval;    
    int neighborNumInterfaces;
#endif

#ifdef INTERFACE_JNE_VMF
    unsigned int urn;
    unsigned short sa_srcport;
    unsigned short sa_dstport;
    unsigned int c2urn;

    void* hitl_socket;
#endif /* INTERFACE_JNE_VMF */

#ifdef NETSNMP_INTERFACE
    BOOL isSnmpEnabled;
    char *snmpdConfigFilePath;
    BOOL generateTrap;
    BOOL generateInform;
    int notification_para;
    NodeAddress managerAddress;
    int snmpVersion;
    int SNMP_TRAP_LINKDOWN_counter;
    NetSnmpAgent *netSnmpAgent;
#endif
    
#ifdef LTE_LIB
    EpcData* epcData;
#endif // LTE_LIB
};


// /**
// STRUCT :: NodePositions
// DESCRIPTION ::
//  Contains information about the initial positions of nodes.
// **/
struct NodePositions {
    NodeAddress       nodeId;
    int               partitionId;
    NodePlacementType nodePlacementType;
    MobilityData*     mobilityData;
};


// /**
// FUNCTION   :: NODE_CreateNode
// PURPOSE    :: Function used to allocate and initialize a node.
// PARAMETERS ::
// + partitionData : PartitionData* : the partition that owns the node
// + nodeId        : NodeId         : the node's ID
// + index         : int            : the node's index within the partition
//                                    since nodeID is non-contiguous
// RETURN :: void :
// **/

Node* NODE_CreateNode(PartitionData* partitionData,
                      NodeId         nodeId,
                      int            partitionId,
                      int            index);

// /**
// FUNCTION   :: NODE_ProcessEvent
// PURPOSE    :: Function used to call the appropriate layer to execute
//               instructions for the message
// PARAMETERS ::
// + node : Node*    : node for which message is to be delivered
// + msg  : Message* : message for which instructions are to be executed
// RETURN :: void :
// **/
void NODE_ProcessEvent(Node* node, Message* msg);

// /**
// API        :: NODE_PrintLocation
// PURPOSE    :: Prints the node's three dimensional coordinates.
// PARAMETERS ::
// + node                 : Node* : the node
// + coordinateSystemType : int   : Cartesian or LatLonAlt
// RETURN :: void :
// **/
void NODE_PrintLocation(Node* node,
                        int   coordinateSystemType);

// /**
// API        :: NODE_GetTerrainPtr
// PURPOSE    :: Get terrainData pointer.
// PARAMETERS ::
// + node                 : Node* : the node
// RETURN :: TerrainData* : TerrainData pointer
// **/
TerrainData* NODE_GetTerrainPtr(Node* node);



#endif // NODE_H
