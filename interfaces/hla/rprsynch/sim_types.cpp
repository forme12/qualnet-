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

#include <RTI.hh>
#include <iostream>
#include "Config.h"
#include "HLAbase.h"
#include "hla_types.h"
#include "rpr_types.h"
#include "sim_types.h"
#include <sstream>

namespace SNT_HLA
{

unsigned int Channel::nextID = 0;

Channel::Channel(std::string model, ForceIdentifierEnum8 force) :
        phyModelName(model),
        forceID(force)
{
    channelID = nextID++;
}

int Hierarchy::count = 0;
Hierarchy* Hierarchy::rootHierarchy = 0;
unsigned char Network::nextIPv4 = 0;

Network::Network(std::string mac, std::string proto, ForceIdentifierEnum8 force, Hierarchy* hierarchy) :
        hier(hierarchy),
        macProtocol(mac),
        forceID(force),
        protocol(proto),
        commonParameters(0)
{
    address = getNextAddress(protocol, hier->nodes.size());
}

void Network::addRouterModel(std::string routerModel)
{

    models.insert(routerModel);
//    models.insert
    
//    const Parameter &p3 = plist->getParameter("PHY-MODEL");
//    if( p3 != Parameter::unknownParameter )
//    {
//        channel = channels.getChannel(forceID, p3.value);
//    }
}

std::string Network::getGroup(std::string model)
{
    int numNodes = 0;
    std::stringstream group;
    std::set<Node*>::iterator nit = nodes.begin();
    while( nit != nodes.end() )
    {
        std::set<RadioTransmitter*>::iterator rit = (*nit)->radios.begin();
        while( rit != (*nit)->radios.end() )
        {
            ParameterMap* pm = Config::instance().getModelParameterList((*rit)->RadioSystemType);
            if( pm && pm->getParameter("ROUTER-MODEL").value == model )
            {
                if( numNodes == 0 )
                    group << "[";
                else
                    group << " ";
                group << (*nit)->NodeId;
                numNodes++;
                break;
            }
            rit++;
        }
        nit++;
    }
    if( numNodes > 0 )
        group << "]";

    return group.str();
}

std::string Network::getNextAddress(std::string protocol, size_t numberNodes)
{
    std::stringstream addr;
    //if( protocol == "IP" )
    {
        addr << "N8-";
        addr << (unsigned int) Config::instance().networkPrefix;
        addr << ".1.";
        addr << (unsigned int) nextIPv4++;
        addr << ".0";
        return addr.str();
    }
}
void Hierarchy::setName(const char* str)
{
    const char *p = str;
    while( *p )
    {
        char c = *p;
        if( c == ' ' )
            c = '_';
        name.push_back(c);
        p++;
    }
}
void Hierarchy::computeTerrainBounds()
{
    TerrainBounds bounds;
    bounds.computeTerrainBounds(&nodes);
    position.x = bounds.centerX;
    position.y = bounds.centerY;
    position.z = bounds.centerZ;
    if( bounds.eastLon > bounds.westLon )
        dimensions.x = bounds.eastLon - bounds.westLon;
    else
        dimensions.x = bounds.westLon - bounds.eastLon;
    dimensions.y = bounds.maxLat - bounds.minLat;
    dimensions.z = bounds.maxAlt - bounds.minAlt;
}

Network* NetworkSet::find(ForceIdentifierEnum8 forceID, const std::string macProtocol, const std::string networkProtocol, Hierarchy* hier )
{
    Network dummy;
    dummy.macProtocol = macProtocol;
    dummy.protocol = networkProtocol;
    dummy.forceID = forceID;
    dummy.hier = hier;
    iterator it = std::set<Network*, Network::less>::find(&dummy);
    if( it != end() )
        return *it;
    else
        return 0;
}

void NetworkSet::assignNodeToNetwork(Node* node,int interfaceNum, ParameterMap* pm)
{
    if( !pm )
        return;

    const Parameter &routerModelparm = pm->getParameter("ROUTER-MODEL");
    if( routerModelparm == Parameter::unknownParameter )
        return;
    const std::string& routerModel = routerModelparm.value;

    const Parameter &phyModelparm = pm->getParameter("PHY-MODEL");
    if( phyModelparm == Parameter::unknownParameter )
        return;
    const std::string& phyModel = phyModelparm.value;

    const Parameter &macProtocolparm = pm->getParameter("MAC-PROTOCOL");
    if( macProtocolparm == Parameter::unknownParameter )
        return;
    const std::string& macProtocol = macProtocolparm.value;

    const Parameter &networkProtocolparm = pm->getParameter("NETWORK-PROTOCOL");
    if( networkProtocolparm == Parameter::unknownParameter )
        return;
    const std::string& networkProtocol = networkProtocolparm.value;

    Network* net = 0;
    if( interfaceNum == 0 )
    {
        net = find(node->entity->ForceIdentifier, macProtocol, networkProtocol, node->hier);
        if( !net )
        {
            std::cout << "creating new subnet for MAC protocol " << macProtocol <<
                " NETWORK protocol " << networkProtocol <<
                " Force " << node->entity->ForceIdentifier <<
                " hierarchy " << node->hier->name << "(" << node->hier->id << ")" <<
                std::endl << std::flush;
            net = new Network(macProtocol, networkProtocol, node->entity->ForceIdentifier, node->hier);
            node->hier->addNetwork(net);
            net->channel = channels->getChannel(node->entity->ForceIdentifier, phyModel);
            insert(net);
        }
    }
    else
    {
        Hierarchy* hier = node->hier;
        while( !net && hier )
        {
            net = find(node->entity->ForceIdentifier, macProtocol, networkProtocol, hier);
            if( net && (net->nodes.find(node) != net->nodes.end()) )
                net = 0;
            hier = hier->parent;
        }

        if( !net )
        {
            int i;
            for( i=0; i<interfaceNum; i++ )
            {
                net = find(node->entity->ForceIdentifier, macProtocol, networkProtocol, getDummyHierarchy(i));
                if( !net )
                    break;
                else if( net->nodes.find(node) == net->nodes.end() )
                    break;
            }
            if( !net )
            {
                std::cout << "creating new subnet for MAC protocol " << macProtocol <<
                    " NETWORK protocol " << networkProtocol <<
                    " Force " << node->entity->ForceIdentifier <<
                    " hierarchy " << node->hier->name << "(" << node->hier->id << ")" <<
                    std::endl << std::flush;
                net = new Network(macProtocol, networkProtocol, node->entity->ForceIdentifier, getDummyHierarchy(i));
                net->channel = channels->getChannel(node->entity->ForceIdentifier, phyModel);
                insert(net);
            }
        }
        std::cout << "adding node " << node->nodeName << "(" << node->NodeId <<
            ") to subnet " << net->address << std::endl;
        net->addNode(node);
    }
    std::cout << "adding node " << node->nodeName << "(" << node->NodeId <<
        ") to subnet " << net->address << std::endl;
    net->addNode(node);
    net->addRouterModel(routerModel);
}

void NodeSet::extractNodes(FedAmb* fed)
{
    std::map<RTI::ObjectHandle, HLAObject*>::iterator it = fed->allObjects.begin();
    while( it != fed->allObjects.end() )
    {
        AggregateEntity* aggregate = dynamic_cast<AggregateEntity*>(it->second);
        PhysicalEntity* entity = dynamic_cast<PhysicalEntity*>(it->second);
        RadioTransmitter* radio = dynamic_cast<RadioTransmitter*>(it->second);
        if(aggregate)
            updateAggregate(aggregate);
        if( entity )
            updateEntity(entity);
        if( radio )
            updateRadio(radio);
        it++;
    }
    if( Config::instance().allEntitiesHaveRadios )
        createRadios();
    purgeIncompleteNodes();
    assignHierarchies();
    assignNodeIds();
    computeTerrainBounds();
    createNetworks();
}
std::set<std::string>& NodeSet::modelsUsed()
{
    iterator it = begin();
    while( it != end() )
    {
        std::set<RadioTransmitter*> radios = (*it)->radios;
        std::set<RadioTransmitter*>::iterator rit = radios.begin();
        while( rit != radios.end() )
        {
            RadioTypeStruct type = (*rit)->RadioSystemType;
            ParameterMap* pm = Config::instance().getModelParameterList((*rit)->RadioSystemType);
            if( pm )
            {
                const Parameter &p = pm->getParameter("ROUTER-MODEL");
                if( p != Parameter::unknownParameter )
                    models.insert(p.value);
            }
            rit++;
        }
        it++;
    }
    return models;
}
void NodeSet::updateAggregate(AggregateEntity* aggregate)
{
    if( !aggregate )
        return;
    aggregates.insert(aggregate);
}

void NodeSet::updateEntity(PhysicalEntity* entity)
{
    if( !entity )
        return;
    Node* newNode = new Node;
    newNode->id = entity->EntityIdentifier;
    newNode->entity = entity;
    const std::string &in = Config::instance().getIconForType(entity->EntityType);
    newNode->setIconName(Config::instance().getIconForType(entity->EntityType));
    iterator it = find(newNode);
    if( it == end() )
        insert(newNode);
    else
    {
        (*it)->entity = entity;
        delete newNode;
    }
}
void NodeSet::updateRadio(RadioTransmitter* radio)
{
    if( !radio )
        return;
    Node* newNode = new Node;
    newNode->id = radio->EntityIdentifier;
    newNode->radios.insert(radio);
    iterator it = find(newNode);
    if( it == end() )
    {
        insert(newNode);
    }
    else
    {
        delete newNode;
        (*it)->radios.insert(radio);
    }
}
Node* NodeSet::findByNodeID(int nodeId)
{
    iterator it = begin();
    while( it != end() )
    {
        if( (*it)->entity && (*it)->NodeId == nodeId )
            return *it;
        it++;
    }
    return 0;
}

Node* NodeSet::findByObjectName(std::string objName)
{
    iterator it = begin();
    while( it != end() )
    {
        if( (*it)->entity && (*it)->entity->myName == objName )
            return *it;
        it++;
    }
    return 0;
}

Network* NodeSet::getNetwork(Node* node, RadioTransmitter* radio)
{
    ParameterMap* pm = Config::instance().getModelParameterList(radio->RadioSystemType);
    if( !pm )
        pm = Config::instance().getModelParameterList(Config::instance().defaultRouterModel);
    if( !pm )
        return 0;
    ForceIdentifierEnum8 forceID = node->entity->ForceIdentifier;
    const std::string macProtocol = pm->getParameter("MAC-PROTOCOL").value;
    const std::string networkProtocol = pm->getParameter("NETWORK-PROTOCOL").value;
    Hierarchy* hier = node->hier;
    return networks.find(forceID, macProtocol, networkProtocol, hier);
}

Network* NodeSet::getNetwork(const std::string& addr)
{
    NetworkSet::iterator it = networks.begin();
    while( it != networks.end() )
    {
        if( (*it)->address == addr )
            return *it;
        it++;
    }
    return 0;
}
std::string NodeSet::getNetworkAddress(Node* node, RadioTransmitter* radio)
{
    Network* net = getNetwork(node, radio);
    if( net )
        return net->address;
    else
        return "";
}
bool NodeSet::usesSlots()
{
    std::set<std::string>::iterator it = modelsUsed().begin();
    while( it != modelsUsed().end() )
    {
        ParameterMap* parameters = Config::instance().getModelParameterList(*it);
        const Parameter& model = parameters->getParameter("MAC-PROTOCOL");
        if( model != Parameter::unknownParameter )
        {
            if( model.value == "MAC-LINK-16" )
                return true;
        }
        it++;
    }
    return false;
}

void NodeSet::createRadios()
{
    iterator it = begin();
    while( it != end() )
    {
        (*it)->radios.clear();
        RadioTransmitter* radio = Factory<RadioTransmitter>::instance().newObject(0, "");
        radio->EntityIdentifier = (*it)->id;
        radio->RadioIndex = 0;
        (*it)->radios.insert(radio);
        it++;
    }
}

void NodeSet::assignHierarchies()
{
    std::set<AggregateEntity*, AggregateEntity::less>::iterator it = aggregates.begin();
    while( it != aggregates.end() )
    {
        AggregateEntity* ae = *it;
        Hierarchy* hier = new Hierarchy();
        RTIObjectIdArrayStruct ids = (*it)->EntityIDs;
        hier->position.x = ae->WorldLocation.lon;
        hier->position.y = ae->WorldLocation.lat;
        hier->position.z = ae->WorldLocation.alt;
        hier->dimensions.x = ae->Dimensions.XAxisLength;
        hier->dimensions.y = ae->Dimensions.YAxisLength;
        hier->dimensions.z = ae->Dimensions.ZAxisLength;
        hier->setName((char *)ae->AggregateMarking.MarkingData);
        for( unsigned int i=0; i<ids.size(); i++ )
        {
            Node* node = findByObjectName(ids[i]);
            if(node)
            {
                node->entity->myName;
                node->hier = hier;
                hier->addNode(node);
            }
        }
        if( hier->nodes.size() )
            hierarchies[*it] = hier;
        else
            delete hier;
        it++;
    }
    it = aggregates.begin();
    while( it != aggregates.end() )
    {
        AggregateEntity2HierarchyMap::iterator ait = hierarchies.find(*it);
        if( ait == hierarchies.end() )
        {
            it++;
            continue;
        }
        Hierarchy* parent = ait->second;
        if( !parent )
        {
            it++;
            continue;
        }
        RTIObjectIdArrayStruct ids = (*it)->SubAggregateIDs;
        for( unsigned int i=0; i<ids.size(); i++ )
        {
            Hierarchy* hier = hierarchies.findByID(ids[i]);
            parent->children.insert(hier);
            hier->parent = parent;
        }
        it++;
    }
    Hierarchy* rootHierarchy = new Hierarchy(Hierarchy::Root);
    AggregateEntity2HierarchyMap::iterator hit = hierarchies.begin();
    while( hit != hierarchies.end() )
    {
        if( !hit->second->parent )
        {
            rootHierarchy->children.insert(hit->second);
            hit->second->parent = rootHierarchy;
        }
        hit++;
    }
    iterator nit = begin();
    while( nit != end() )
    {
        if( !(*nit)->hier )
        {
            rootHierarchy->addNode(*nit);
            (*nit)->hier = rootHierarchy;
        }
        nit++;
    }
}

void NodeSet::purgeIncompleteNodes()
{
    iterator it = begin();
    while( it != end() )
    {
        if( !(*it)->entity || !(*it)->radios.size() )
        {
            iterator deadManWalking = it;
            it++;
            erase(deadManWalking);
        }
        else if( (*it)->entity->Marking.MarkingData[0] == 0 )
        {
            iterator deadManWalking = it;
            it++;
            erase(deadManWalking);
        }
        else
        {
            it++;
        }
    }
}
void NodeSet::assignNodeIds()
{
    std::map<int, Node*> assignedNodes;
    iterator it = begin();
    while( it != end() )
    {
        if( (*it)->NodeId != -1 )
            assignedNodes[(*it)->NodeId] = *it;
        it++;
    }
    int Id = Config::instance().firstNodeId;
    it = begin();
    while( it != end() )
    {
        if( (*it)->NodeId == -1 )
        {
            while( assignedNodes.find(Id) != assignedNodes.end() )
                Id++;
            (*it)->NodeId = Id++;
            assignedNodes[(*it)->NodeId] = *it;
        }
        it++;
    }
}

void  NodeSet::computeTerrainBounds()
{
    TerrainBounds bounds;
    bounds.computeTerrainBounds(this);

    std::stringstream swCorner;
    swCorner.precision(9);
    swCorner << "( " << bounds.minLat << ", " << bounds.westLon << " )";
    std::stringstream neCorner;
    neCorner.precision(9);
    neCorner << "( " << bounds.maxLat << ", " << bounds.eastLon << " )";
    Config::instance().setParameter(Parameter("TERRAIN-SOUTH-WEST-CORNER", swCorner.str()));
    Config::instance().getParameter("TERRAIN-NORTH-EAST-CORNER");
    Config::instance().setParameter(Parameter("TERRAIN-NORTH-EAST-CORNER", neCorner.str()));
}

void NodeSet::createNetworks()
{
    iterator it = begin();
    while( it != end() )
    {
        Node* node = *it;
        std::set<RadioTransmitter*> radios = node->radios;
        std::set<RadioTransmitter*>::iterator rit = radios.begin();
        int interfaceNum = 0;
        while( rit != radios.end() )
        {
            RadioTypeStruct type = (*rit)->RadioSystemType;
            ParameterMap* pm = Config::instance().getModelParameterList((*rit)->RadioSystemType);
            networks.assignNodeToNetwork(node, interfaceNum, pm);

            interfaceNum++;
            rit++;
        }
        it++;
    }
}

};
