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

#ifndef _ENTITY_H_
#define _ENTITY_H_

//#define PLATFORM_MAPPING

#include <vector>

typedef std::list<NodeId> NodeIdList; 

class D_SysUpTime : public D_Object
{
    private:
        EXTERNAL_Interface* m_Iface;
        clocktype m_Time;
        
    public:
        //D_SysUpTime()
        D_SysUpTime(EXTERNAL_Interface* iface, clocktype time) : D_Object (D_VARIABLE)
        {
            executable = FALSE;
            readable = TRUE;
            m_Iface = iface;
            m_Time = time;
            writeable = FALSE;
        }

        void ReadAsString(std::string& out);
};

class D_MiC2Node : public D_Object
{
    private:
        EXTERNAL_Interface* m_Iface;
        NodeAddress m_NodeId;
               
    public:
        //D_SysUpTime()
        D_MiC2Node(EXTERNAL_Interface* iface, NodeAddress nodeId) : D_Object (D_VARIABLE)
        {
            executable = FALSE;
            readable = TRUE;
            m_Iface = iface;
            m_NodeId = nodeId;
            writeable = FALSE;
        }

        void ReadAsString(std::string& out);
};

struct MulticastGroup
{
    // The name of the multicast group a platform belongs to
    std::string group;

    // Index into InterfaceData.sockets.connections (array socket.h).  Specifies which socket
    // connections will receive responses for this multicast group.
    std::vector<int> socketSubscribers;
};

struct Entity_NodeData // was EntityData
{    
    Node* node;    

    // Mutlicast group memberships for this platform.  Contents of this
    // array are filled in respose to CreatePlatform/UpdatePlatform
    std::vector<MulticastGroup> multicastGroups;  
};

struct EntityData 
{
    std::string entityId; 
    std::map<NodeId, Entity_NodeData> nodes; 

    short type;
    unsigned char commTypes;
    UInt8 damageState;

    // FALSE if the mapping of a entity exists but the node for the entity is 
    // inactive, TRUE otherwise.
    BOOL isActive;

    // Sysytem Group stats.
    D_String sysName;
};

struct EntityMapping
{
    // FALSE if the entity mapping file should be used.  If TRUE then
    // mappings will be created dynamically.
    BOOL dynamic;

    //int nextNodeId;

    // Maps the Nodes initialized by MTS but not yet mapped to an entityId
    std::map<NodeId, Entity_NodeData> unmappedNodes;
    // Maps each Node ID to an Entity (Platform)
    std::map<NodeId, EntityData*> nodeEntities; // was nodePlatforms
    // Collection of Entities (Platforms) using EntityID as the key
    std::map<std::string, EntityData*> entities; // was platforms
};

void EntityMappingInitialize(
    EntityMapping* mapping,
    int size,
    BOOL dynamic);

void EntityMappingDelete( 
    EntityMapping *mapping,
    std::string entityId); 

EntityData* DynamicMappingAdd( 
    EntityMapping* mapping, 
    std::string entityId);

EntityData* EntityMappingAddToPlatform( 
    EntityMapping* mapping, 
    std::string entityId,
    NodeAddress nodeId); 

EntityData* EntityMappingLookup(
    EntityMapping* mapping,
    std::string entityId); 

EntityData* EntityMappingLookup(
    EntityMapping* mapping,
    NodeAddress nodeId);

Node* EntityNodeLookup(
    EntityMapping *mapping, 
    NodeId nodeId); 

void MappingGetNodeIdList( 
    EntityMapping *mapping,
    std::string entityId, 
    NodeIdList&); 

void MappingGetNodeAddressList(
    PartitionData* partition,
    EntityMapping *mapping,
    std::string entityId, 
    std::list<NodeAddress>& nodeAddresses); 

void MappingGetNodePointerList(  
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId, 
    std::list<Node*>& nodes);

Node* MappingGetSingleNodePointer(
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId); 

NodeAddress MappingGetSingleNodeAddress(
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId);

std::string MappingGetEntityId(
    EntityMapping *mapping,
    NodeAddress);

void MappingAddMulticastGroup(
    EntityMapping* mapping,
    std::string entityId,
    std::string* group,
    int socket);

void MappingRemoveMulticastGroup(
    EntityMapping* mapping,
    std::string entityId, 
    std::string* group);

// Returns FALSE if no error, otherwise returns error TRUE.
BOOL ReadEntityMappingFile(
    EXTERNAL_Interface*& iface,
    std::map<std::string, NodeIdList>& hostNodes, 
    const char *fileName,
    std::string* errorString);

void InitInterfaceSysStats(
    EXTERNAL_Interface* iface,
    Node* node,
    EntityData* rec,
    clocktype time);

bool IsEntityIdString(std::string entityId); 

#endif /* _ENTITY_H_ */
