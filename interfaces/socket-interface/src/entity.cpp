#include "api.h"
#include "partition.h"
#include "entity.h"
#include "messageapi.h"
#include "socket-interface.h"

static int EntityMappingHash(const char *entityId, int hashSize)
{
    int hash = 0;

    // This is a poor hash function

    while (*entityId != 0)
    {
        hash += *entityId;
        entityId++;
    }

    return hash % hashSize;
}

void EntityMappingInitialize(
    EntityMapping *mapping,
    int size,
    BOOL dynamic)
{
    // Create and zero new table    
    mapping->dynamic = dynamic;
}

void EntityMappingDelete(
    EntityMapping *mapping,
    std::string entityId)
{
    ERROR_Assert(mapping != NULL, "EntityMapping undefined!");

    std::map<std::string, EntityData*>::iterator platformit; 
    platformit = mapping->entities.find(entityId); 
    if (platformit != mapping->entities.end())
    {
        mapping->entities.erase(platformit); 
    }
}

// Creates a new dynamic mapping for the given entityId
// and returns a pointer to the new platform mapping
EntityData* DynamicMappingAdd(
    EntityMapping* mapping, 
    std::string entityId)
{
    ERROR_Assert(mapping != NULL, "EntityMapping undefined!"); 
#ifdef DEBUG
    printf("DynamicMappingAdd for entityId %d\n", entityId); 
#endif
    //if (mapping->unmappedNodes.begin() != mapping->unmappedNodes.end())
    if (!mapping->unmappedNodes.empty())
    {
        return EntityMappingAddToPlatform(mapping, 
            entityId, 
            mapping->unmappedNodes.begin()->first); 
            //mapping->unmappedNodes.front().node->nodeId); 
    }

    return NULL; 
}

// Creates a new mapping for the given entityId & nodeId
// and returns a pointer to the platform mapping
EntityData* EntityMappingAddToPlatform(
    EntityMapping* mapping, 
    std::string entityId, 
    NodeAddress nodeId)
{
    ERROR_Assert(mapping != NULL, "EntityMapping undefined!");
#ifdef DEBUG
    printf("EntityMappingAddToPlatform for entity %s and node %d\n", entityId.c_str(), nodeId); 
    fflush(stdout); 
#endif 

    EntityData* platform = NULL; 
    Entity_NodeData entity; 

    // find Entity_NodeData with Node pointer already populated
    // if it exists, otherwise create a new one (empty Node)
    std::map<NodeId, Entity_NodeData>::iterator unmappedit; 
    unmappedit = mapping->unmappedNodes.find(nodeId); 
    if (unmappedit != mapping->unmappedNodes.end())
    {
        entity = unmappedit->second; 

        // This node is now in the mapping so remove from "unused" nodes
        mapping->unmappedNodes.erase(unmappedit); 
    } 
    else
    {        
        // This is an error
        char err[MAX_STRING_LENGTH];
        sprintf(err, "ERROR: node %d not found in unmappedNodes!\n", nodeId);
        ERROR_ReportError(err); 
        return NULL; 
    }

    // For one-to-many mapping, this platform could already exist
    // we could be adding a new Entity_NodeData to the collection    
    std::map<std::string, EntityData*>::iterator platformit; 
    platformit = mapping->entities.find(entityId); 
    if (platformit != mapping->entities.end())
    {
        platform = platformit->second; 
        platform->nodes[nodeId] = entity;         
    }
    else // new platform
    {
        std::map<NodeId, EntityData*>::iterator nodeEntityIt; 
        nodeEntityIt = mapping->nodeEntities.find(nodeId); 
        if (nodeEntityIt != mapping->nodeEntities.end())
        {
            // This means that the nodeId exists in a platform already
            // Even though the entityId provided is a new platform
            // This is an error! Nodes can only belong to one platform
            char err[MAX_STRING_LENGTH];
            sprintf(err, "NodeId %d already maps to an existing Entity %s", 
                nodeId, 
                nodeEntityIt->second->entityId.c_str()); 
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidEntityId,
                err);
        }
        platform = new EntityData; 
        platform->entityId = entityId; 
        platform->isActive = false; 
    }

    platform->nodes[nodeId] = entity; 
    
    mapping->entities[entityId] = platform; 
    mapping->nodeEntities[nodeId] = platform;  

    return platform; 
}

EntityData* EntityMappingLookup(
    EntityMapping *mapping,
    std::string entityId)
{
    EntityData *rec = NULL; 

    std::map<std::string, EntityData*>::iterator platformit; 
    platformit = mapping->entities.find(entityId); 
    if (platformit != mapping->entities.end())
    {
        rec = platformit->second; 
    }

    return rec;
}

EntityData* EntityMappingLookup(
    EntityMapping* mapping,
    NodeId nodeId)
{
    EntityData* platform = NULL; 

    std::map<NodeId, EntityData*>::iterator npIt; 
    npIt = mapping->nodeEntities.find(nodeId); 
    if (npIt != mapping->nodeEntities.end())
    {
        platform = npIt->second;
    }

    return platform; 
}

Node* EntityNodeLookup(
    EntityMapping *mapping, 
    NodeId nodeId)
{
    Node* node = NULL; 

    EntityData* platform = EntityMappingLookup(mapping, nodeId); 
    if (platform)
    {
        std::map<NodeId, Entity_NodeData>::iterator entityIt; 
        entityIt = platform->nodes.find(nodeId); 
        if (entityIt != platform->nodes.end())
        {
            node = (entityIt->second.node); 
        }
    }
    return node; 
}

static BOOL IsValidNumQuotesInMapping(char* token)
{
    std::string buf = token;
    int numQuote = 0;
    int pos = 0;
    int nextPos = 0;
    nextPos = buf.find("\"", pos);
    while (nextPos != -1)
    {
        numQuote++;
        pos = ++nextPos;
        nextPos = buf.find("\"", pos);
    }
    if (numQuote > 4 || numQuote % 2 != 0)
    {
        return FALSE;
    }
    return TRUE;
}

static BOOL ParseEntityMapping(
    char* strLine,
    char* token,
    char* result,
    char** next)
{
    char* tok;
    char* temp;
    char iotoken[10*MAX_STRING_LENGTH];
    char err[MAX_STRING_LENGTH];

    //char str[MAX_STRING_LENGTH];
    //strcpy(token, str);

    if (token[0] == '\"')
    {
        // First character is a quote, then extract till the end of 
        // the next quote.
        temp = strchr(&token[1], '"');
        if (temp == NULL)
        {
            sprintf (err, 
                "Entity Mapping: Missing Quotes: %s\n", 
                strLine);
            ERROR_ReportWarning(err);
            return FALSE;
            //fgets(str, MAX_STRING_LENGTH, f);
            //continue;
        }
        tok = IO_GetDelimitedToken(iotoken, token, "\"", next);

        // Successfully extracted the entity Id.
        strcpy(result, tok);

        // As we have found the 2 quotes, increment the pointer.
        (*next)++;
    }
    else
    {
        // No quotes being used so get everything till space.
        tok = IO_GetDelimitedToken(iotoken, token, " ", next);

        // Successfully extracted the entity Id.
        if (tok != NULL && result != NULL)
        {
            strcpy(result, tok);
        }
        else
        {
            return false; 
        }
    }

    // Check if the entity has any delimiter
    temp = strchr(&result[0], '"');
    if (temp != NULL)
    {
        sprintf (err, 
            "Entity Mapping: Invalid entry: %s\n", 
            strLine);
        ERROR_ReportWarning(err);
        return FALSE;
    }
    return TRUE;
}

BOOL ReadEntityMappingFile(
    EXTERNAL_Interface*& iface,
    std::map<std::string, NodeIdList>& hostNodes, 
    const char *fileName,
    std::string* errorString)
{     
    FILE *f;
    char err[MAX_STRING_LENGTH];
    char str[MAX_STRING_LENGTH];
    char entityId[MAX_STRING_LENGTH];
    char nodeId[MAX_STRING_LENGTH];
    memset(nodeId, '\0', MAX_STRING_LENGTH);
    NodeIdList nodes;
    char iotoken[10*MAX_STRING_LENGTH];
    char *entityDelims = "\"\n";
    char* next;
    char *token;
    *errorString = "";
    BOOL quotesUsed = FALSE;

    ERROR_Assert(iface != NULL, "Interface undefined!"); 
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*)iface->data;
    ERROR_Assert(data != NULL, "SocketInterface_InterfaceData undefined!"); 
    EntityMapping *mapping = &data->entityMapping; 

    std::string quote = "";
    // Open the file
    f = fopen(fileName, "r");
    if (f == NULL)
    {
        sprintf(err, "Unable to open entity mapping file \"%s\"", fileName);
        ERROR_ReportError(err);
    }

    // Read all lines
    fgets(str, MAX_STRING_LENGTH, f);
    std::string id;
    char* tok;
    char* temp;
    while (!feof(f))
    {
        //get rid of the end of line characters.
        entityDelims = "\n";
        token = IO_GetDelimitedToken(iotoken, str, entityDelims, &next);
        if (token == NULL)
        {
            fgets(str, MAX_STRING_LENGTH, f);
            continue;
        }
        else
        {
            // check if we how many quotes. Valid number is 0, 2, and 4.
            if (!IsValidNumQuotesInMapping(token))
            {
                // we have an error. 
                sprintf (err, 
                    "Entity Mapping: Missing Quotes: %s\n", 
                    str);
                ERROR_ReportWarning(err);
                fgets(str, MAX_STRING_LENGTH, f);
                continue;
            }
        }
        id = token;
        if (token[0] == '#') // comment
        {
            fgets(str, MAX_STRING_LENGTH, f);
            continue;
        }
    
        if (!ParseEntityMapping(str, token, entityId, &next))
        {
            fgets(str, MAX_STRING_LENGTH, f);
            continue;
        }
        // Now we should have a space as the next charater. If the character
        // is not space then we have a problem
        if (next[0] != ' ')
        {
            // We have a problem.
            sprintf (err, 
                "Entity Mapping: Invalid entry: %s (no nodes in the entity)\n", 
                str);
            ERROR_ReportError(err);
            fgets(str, MAX_STRING_LENGTH, f);
            continue;
        }
        
        bool parsed = false; 
        bool nextline = false; 
        do {
            next++;
            // This is Node-ID, Host-IP, or JTRS-ID
            if (!ParseEntityMapping(str, next, nodeId, &next))
            {
                // This checks for the case of nodeIds followed by spaces
                // As long as there is at least one nodeId, 
                // this entry is valid
                if (nodes.size() > 0) 
                {
                    parsed = true; 
                }
                else // no nodeIds --> skip this line
                {
                    sprintf (err, 
                        "ReadEntityMappingFile Error: No nodes in the entity %s\n", 
                        entityId);
                    ERROR_ReportError(err);
                    nextline = true; 
                }
            }
            else // nodeId parsed fine
            {
                if (nodeId[0] == '#') // comment
                { 
                    parsed = true; 
                }
                else
                {
                    // Check for hostname in collection 
                    //printf("searching for %s in hostNodes, size %d", nodeId, hostNodes.size());
                    //fflush(stdout);                    
                    std::map<std::string, NodeIdList>::iterator hostnodeIt; 
                    hostnodeIt = hostNodes.find(nodeId); // if nodeId == a hostname
                        
                    if (hostnodeIt != hostNodes.end())
                    {        
#ifdef DEBUG
                        printf("host %s has %d nodeIds: ", hostnodeIt->first.c_str(), hostnodeIt->second.size());  
#endif

                        NodeIdList::iterator nodelistIt; 
                        for (nodelistIt = hostnodeIt->second.begin(); 
                            nodelistIt != hostnodeIt->second.end(); 
                            nodelistIt++)
                        {
#ifdef DEBUG
                            printf("%d ", *nodelistIt); 
#endif
                            nodes.push_back(*nodelistIt); 
                        }
#ifdef DEBUG
                        printf("\n");                       
#endif
                    }
                    else // not hostname --> host-ip or node-id
                    {
#ifdef DEBUG
                        printf("nodeId %s not found in hostNodes\n", nodeId); 
                        fflush(stdout); 
#endif

                        // Check for node associated with Host-IP
                        if (!IsEntityIdString(std::string(nodeId))) //only if a number, not a string
                        {
                            NodeAddress nadd; 
                            BOOL isNodeId; 
                            IO_ParseNodeIdOrHostAddress(nodeId, &nadd, &isNodeId); 
                            if (!isNodeId && nadd != INVALID_MAPPING)
                            {                                
                                nadd = MAPPING_GetNodeIdFromInterfaceAddress(
                                    iface->partition->firstNode, nadd); 
#ifdef DEBUG
                                printf("nodeId %s is an ip address with nodeId %d\n", nodeId, nadd); 
                                fflush(stdout); 
#endif 
                                nodes.push_back(nadd); 
                            } 
                            else // it's just a nodeId
                            {
#ifdef DEBUG
                                printf("nodeId %s being added to nodes\n", nodeId); 
                                fflush(stdout); 
#endif
                                nodes.push_back((NodeId)atoi(nodeId)); 
                            }
                        }
                        else //either a string nodeid, or a hostname that wasn't found
                        {
                            char err[MAX_STRING_LENGTH];
                            sprintf(err,
                                "Hostname %s not found/Not a valid NodeId",
                                nodeId);
                            ERROR_ReportError(err); 
                        }
                    }
                }
                fflush(stdout); 

                // If there are more nodes (one-to-many mapping) associated with this entityId, 
                // We should have a space as the next character
                // (if no space, no problem --> we are done with this entity)
                if (next[0] != ' ' && !parsed && !nextline)
                {
                    parsed = true; //done parsing this entity
                }
            }
        } while (!parsed && !nextline); 
        
        if (nextline)
        {
            fgets(str, MAX_STRING_LENGTH, f);
            continue; 
        }

        EntityData* rec;        
        NodeIdList::iterator nodeIt; 
        for (nodeIt = nodes.begin(); nodeIt != nodes.end(); nodeIt++)
        {
            NodeId nid = *nodeIt;

#ifdef DEBUG
            printf("adding node [%d] to entity [%s]\n", nid, entityId); 
            fflush(stdout); 
#endif

            rec = EntityMappingAddToPlatform(
                mapping,
                entityId,
                nid);

            if (rec == NULL)
            {                
                std::stringstream sstr; 
                sstr.str(""); 
                if (*errorString == "")
                {
                    sstr << "ECMF: E: Mapping failure, Following node(s) do not exist: "
                        << nid; 
                    //*errorString = "ECMF: E: Mapping failure, Following node(s) do not exist: " + nid;
                    *errorString = sstr.str(); 
                }
                else
                {
                    sstr << *errorString << ", " << nid; 
                    //*errorString += ", " + nid;
                    *errorString = sstr.str(); 
                }
            } 
            else
            {
#ifdef DEBUG
                printf("successfully added %d to platform %s (size: %d)\n", 
                    nid, entityId, rec->nodes.size()); 
                fflush(stdout); 
#endif
            }
#ifdef DEBUG
            printf(
                "Read entityId \"%s\" nodeId \"%d\"\n",
                entityId,
                nid);
#endif
        }
        nodes.clear(); 
        fgets(str, MAX_STRING_LENGTH, f);
    }

    fclose(f);

    if (*errorString == "")
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

std::string MappingGetEntityId(
    EntityMapping *mapping,
    NodeAddress nodeId)
{
    std::string entityId = ""; 
    std::map<NodeId, EntityData*>::iterator nodeit; 
    nodeit = mapping->nodeEntities.find((NodeId)nodeId); 
    if (nodeit != mapping->nodeEntities.end())  
    {
        EntityData* platform = nodeit->second; 
        if (platform)
        {
            entityId = platform->entityId; 
        }
    }

    return entityId; 
}

void MappingGetNodeIdList(
    EntityMapping *mapping,
    std::string entityId, 
    std::list<NodeId>& nodeIds)
{
    EntityData* rec; 
    rec = EntityMappingLookup(mapping, entityId);

    if (rec != NULL)
    {
        std::map<NodeId, Entity_NodeData>::iterator it; 
        for (it = rec->nodes.begin(); it != rec->nodes.end(); it++)
        {
            nodeIds.push_back(it->second.node->nodeId); 
        }
    }
}

void MappingGetNodeAddressList(
    PartitionData* partition,
    EntityMapping *mapping,
    std::string entityId, 
    std::list<NodeAddress>& nodeAddresses)
{
    NodeIdList nodeIds;
    MappingGetNodeIdList(mapping, entityId, nodeIds);

    NodeIdList::iterator it; 
    for (it = nodeIds.begin(); it != nodeIds.end(); it++)
    {
        NodeAddress n = MAPPING_GetDefaultInterfaceAddressFromNodeId(
            partition->firstNode, *it); 
        if (n != INVALID_MAPPING)
        {
            nodeAddresses.push_back(n);
        }        
        else
        {
            char err[MAX_STRING_LENGTH];
            sprintf(err,
                "NodeId %d does not map to an interface address",
                *it);
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidNodeId,
                err);
        }
    }
}

NodeAddress MappingGetSingleNodeAddress(
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId)
{
    // first check if it's an ip address
    if (!IsEntityIdString(entityId)) //only if a number, not a string
    {
        NodeAddress nadd, temp; 
        BOOL isNodeId; 
        IO_ParseNodeIdOrHostAddress(entityId.c_str(), &nadd, &isNodeId); 
        if (!isNodeId && nadd != INVALID_MAPPING) //ip address
        {
            char* buf = new char[MAX_STRING_LENGTH]; 
            IO_ConvertIpAddressToString(nadd, buf);
#ifdef DEBUG
            std::cout << "(MappingGetSingleNodeAddress) returning address " 
                << buf << " for entityId " << entityId << std::endl; 
#endif
            return nadd; 
        }                   
    }

    std::list<NodeAddress> addresses;
    MappingGetNodeAddressList(partition, mapping, entityId, addresses);

    if (addresses.size() > 1)
    {
        char err[MAX_STRING_LENGTH];
        sprintf(err,
            "Invalid EntityId (multiple mappings): %s",
            entityId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
    }
    else if (addresses.size() == 1)
    {
        return *(addresses.begin()); 
    }
    else
    {
        //std::cout << "MappingGetSingleNodeAddress for entity " << entityId 
        //    << " found no node addresses!\n"; 
        return 0; // Invalid NodeAddress
    }
}


Node* MappingGetSingleNodePointer(
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId)
{
    Node* node = NULL; 

    std::list<Node*> nodes; 
    MappingGetNodePointerList(
        partition,
        mapping,
        entityId,
        nodes);
    if (nodes.size() > 1)
    {
        char err[MAX_STRING_LENGTH];
        sprintf(err,
            "Invalid EntityId (multiple mappings): %s",
            entityId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
    }
    else if (nodes.size() == 1)
    {
        node = *nodes.begin(); 
    }
    //else node = null

    return node; 
}

void MappingGetNodePointerList(  
    PartitionData* partition,
    EntityMapping* mapping,
    std::string entityId, 
    std::list<Node*>& nodes)
{
    std::list<NodeAddress> nodeIds;
    
    // first check if it's an ip address
    if (!IsEntityIdString(entityId)) //only if a number, not a string
    {
        NodeAddress nadd, temp; 
        BOOL isNodeId; 
        IO_ParseNodeIdOrHostAddress(entityId.c_str(), &nadd, &isNodeId); 
        if (!isNodeId && nadd != INVALID_MAPPING) //ip address
        {                                
            nadd = MAPPING_GetNodeIdFromInterfaceAddress(
                partition->firstNode, nadd); 
            if (nadd != INVALID_MAPPING)
            {
#ifdef DEBUG
                std::cout << "(MappingGetNodePointerList) adding node " 
                    << nadd << " for entityId " << entityId << std::endl;
#endif
                nodeIds.push_back(nadd); 
            }
        }
    }

    MappingGetNodeIdList(mapping, entityId, nodeIds);

    std::list<NodeAddress>::iterator it; 
    for (it = nodeIds.begin(); it != nodeIds.end(); it++)
    {
        NodeAddress nodeId = *it; 
        Node* node = NULL; 
        PARTITION_ReturnNodePointer(partition, &node, nodeId, true); 
        nodes.push_back(node); 
    }
}


void MappingAddMulticastGroup(
    EntityMapping* mapping,
    std::string entityId,
    std::string* group,
    int socket)
{    

    BOOL present = FALSE;    

    // Get the mapping.  If no mapping return.
    EntityData* rec; 
    rec = EntityMappingLookup(mapping, entityId);
    if (rec == NULL)
    {
        return;
    }

    std::map<NodeId, Entity_NodeData>::iterator nodeit; 
    for (nodeit = rec->nodes.begin(); nodeit != rec->nodes.end(); nodeit++)
    {
        Entity_NodeData entity = nodeit->second; 

        // Check if the group is present.  If it is present, update the
        // owner.
        std::vector<MulticastGroup>::iterator it;
        for (it = entity.multicastGroups.begin(); it != entity.multicastGroups.end(); it++)
        {
            if (it->group == *group)
            {
                present = TRUE;

                // Check if this socket connection is not already in the list
                // of subscribers.  If not, add it.
                if (find(it->socketSubscribers.begin(),
                         it->socketSubscribers.end(),
                         socket) == it->socketSubscribers.end())
                {
                    it->socketSubscribers.push_back(socket);
                }
                break;
            }
        }

        // If it's not present then add it.
        if (!present)
        {
            MulticastGroup newGroup;

            newGroup.group = *group;
            newGroup.socketSubscribers.push_back(socket);

            entity.multicastGroups.push_back(newGroup);
        }

        rec->nodes[nodeit->first] = entity; // save it back
    } // end for each entity
}

void MappingRemoveMulticastGroup(
    EntityMapping* mapping,
    std::string entityId,
    std::string* group)
{
    EntityData* rec; 

    // Get the mapping.  If no mapping return.
    rec = EntityMappingLookup(mapping, entityId);
    if (rec == NULL)
    {
        return;
    }

    std::map<NodeId, Entity_NodeData>::iterator nodeit; 
    for (nodeit = rec->nodes.begin(); nodeit != rec->nodes.end(); nodeit++)
    {
        Entity_NodeData entity = nodeit->second; 

        // Check if the group is present.  If it is present, remove it.
        std::vector<MulticastGroup>::iterator it;
        for (it = entity.multicastGroups.begin(); it != entity.multicastGroups.end(); it++)
        {
            if (it->group == *group)
            {
                entity.multicastGroups.erase(it);
                break;
            }
        }

        rec->nodes[nodeit->first] = entity; // save it back
    } // end for each entity
}

void D_SysUpTime::ReadAsString(std::string& out)
{
    char t[MAX_STRING_LENGTH];
    clocktype temp;
    temp = EXTERNAL_QuerySimulationTime(m_Iface) - m_Time;
    TIME_PrintClockInSecond(temp, t); 
    out = t;
}

void D_MiC2Node::ReadAsString(std::string& out)
{
    SocketInterface_InterfaceData* data;
    EntityMapping* mapping;
    data = (SocketInterface_InterfaceData*) m_Iface->data;
    assert(data != NULL);
    mapping = &data->entityMapping;
    // Get the entityId from the node id.
    if (m_NodeId == 0)
    {
        out = "";
        return;
    }
    out = MappingGetEntityId(mapping, m_NodeId);

}

void InitInterfaceSysStats(
    EXTERNAL_Interface* iface,
    Node* node,
    EntityData* rec, 
    clocktype time)
{
    char nodeStr[MAX_STRING_LENGTH];
    char platformStr[MAX_STRING_LENGTH];
    D_Level* platformLevel;
    D_Level* nodeLevel;

    // Make the stats dynamic
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;
    std::string path;
    char result[MAX_STRING_LENGTH];
    char mibsIdPath[MAX_STRING_LENGTH];
    BOOL createPath = FALSE;
    NodeAddress nodeId;
    std::string newPath = "";    

    // Create a link in dynamic hierarchy from platform to node if
    // hiearchy is enabled
   if (h->IsEnabled())
   {
        // check if platform maps to only 1 node
        if (rec->nodes.size() == 1)
        {
            try 
            {
                sprintf(nodeStr, "/node/%d", node->nodeId);
                newPath += h->EncodeString(rec->entityId);
                sprintf(platformStr, "/platform/%s", newPath.c_str());
                iface->partition->dynamicHierarchy.AddLink(platformStr, nodeStr);
            }
            catch (D_Exception)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidDynamicCommand,
                    "Unable to set dynamic link from platform to node");
            }
        }

        // handle the system group stats.
        rec->sysName.Set(rec->entityId);
               
        // create path for sysUpTime
        createPath = 
            h->CreateNodePath(
            node,
            "SysUpTime",
            path);
        if (createPath)
        {
            h->AddObject(
                path,
                new D_SysUpTime(iface, time));
        
            h->SetWriteable(path, FALSE);
            h->SetExecutable(path, FALSE);
            sprintf(mibsIdPath,
                "/node/%d/snmp/1.3.6.1.2.1.1.3.0",
                node->nodeId);
            h->AddLink(mibsIdPath, path);
            sprintf(mibsIdPath,
                "/node/%d/snmp/WnwSecret/1.3.6.1.2.1.1.3.0",
                node->nodeId);
            h->AddLink(mibsIdPath, path);
            sprintf(mibsIdPath,
                "/node/%d/snmp/WnwUsi/1.3.6.1.2.1.1.3.0",
                node->nodeId);
            h->AddLink(mibsIdPath, path);
            sprintf(mibsIdPath,
                "/node/%d/snmp/Srw/1.3.6.1.2.1.1.3.0",
                node->nodeId);
            h->AddLink(mibsIdPath, path);
        }
        // create path for sysName
        createPath = 
            h->CreateNodePath(
            node,
            "SysName",
            path);
        if (createPath)
        {
            h->AddObject(
                path,
                new D_StringObj(&rec->sysName));
        
            h->SetReadable(path, TRUE);
            h->SetWriteable(path, FALSE);
            h->SetExecutable(path, FALSE);
            sprintf(mibsIdPath, 
                "/node/%d/snmp/1.3.6.1.2.1.1.5.0",
                node->nodeId);
            h->AddLink(mibsIdPath, path);
        }
        // Check if node has a miC2Node.  If it does it was created by
        // generic_ls.  Add link to node's miC2Node to create snmp
        // MIB object.
        char c2NodePath[MAX_STRING_LENGTH];
        sprintf(c2NodePath, "/node/%d/miC2Node", node->nodeId);
        if (h->IsValidPath(c2NodePath))
        {
            sprintf(mibsIdPath,
                "/node/%d/snmp/1.3.6.1.4.1.3333.3.1",
                node->nodeId);
            h->AddLink(mibsIdPath, c2NodePath);
        }
   }
}

// Checks if the string is a valid "number" 
// (hex, ip address, mac address)
// or string and returns true for string, 
// returns false for number
bool IsEntityIdString(std::string entityId)
{
    bool isString = true; 
    if (entityId.length() > 1) 
    {
        if (entityId[0] == '0' && entityId[1] == 'x')
        {
            isString = false; 
        }
    }
    if (isString)
    {
        isString = false; 
        int i = 0; 
        while (!isString && i < entityId.length())
        {
            //std::cout << nodeId[i] << std::endl; 
            if (entityId[i] == '.' || 
                entityId[i] == ':' ||
                isdigit(entityId[i]))
            {                                    
                isString = false; 
                i++; 
            }
            else
            {
                //std::cout << "char " << entityId[i] << " makes isString\n"; 
                isString = true; 
                break; 
            }
        }
    }
    return isString; 
}

