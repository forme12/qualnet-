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

#ifndef _SIM_TYPES_H_
#define _SIM_TYPES_H_

#include "HLAbase.h"
#include "hla_types.h"
#include "rpr_types.h"
#include "FedAmb.h"
#include "Config.h"
#include <set>
#include <math.h>
#include "RPR_FOM.h"

namespace SNT_HLA
{


class Channel
{
    public:
        class less
        {
            public:
                bool operator()(const Channel* c1, const Channel* c2) const
                {
                    if( c1->forceID < c2->forceID )
                        return true;
                    else if( c1->forceID > c2->forceID )
                        return false;
                    else
                        return c1->phyModelName < c2->phyModelName;
                }
        };
        Channel() {}
        Channel(std::string model, ForceIdentifierEnum8 force);
        std::string phyModelName;
        ForceIdentifierEnum8 forceID;
        unsigned int channelID;
        static unsigned int nextID;
};

class ChannelSet : public std::set<Channel*, Channel::less>
{
    public:
        Channel* getChannel(ForceIdentifierEnum8 forceID, const std::string phyModelName)
        {
            Channel dummy;
            dummy.forceID = forceID;
            dummy.phyModelName = phyModelName;
            iterator it = find(&dummy);
            if( it != end() )
            {
                return *it;
            }
            else
            {
                Channel* channel = new Channel(phyModelName, forceID);
                insert(channel);
                return channel;
            }
        }
};
class Node;
class Network;
class Hierarchy;

typedef struct Vector3D
{
    double x;
    double y;
    double z;
} Vector3D;


class AggregateEntity2HierarchyMap : public std::map<AggregateEntity*, Hierarchy*>
{
    public:
        Hierarchy* findByID(std::string id)
        {
            iterator it = begin();
            while( it != end() )
            {
                AggregateEntity* ent = it->first;
                if( ent->myName == id )
                    return it->second;
                it++;
            }
            return 0;
        }
};

class Node
{
    public:
        class less
        {
            public:
                bool operator()(const Node *n1, const Node *n2) const
                {
                    return n1->id < n2->id;
                }
        };
        int NodeId;
        std::string nodeName;
        std::string iconName;
        Hierarchy* hier;
        EntityIdentifierStruct id;
        PhysicalEntity* entity;
        std::set<RadioTransmitter*> radios;
        Node() : NodeId(-1), hier(0), entity(0) {}
        const char* getNodeName()
        {
            if( !nodeName.empty() )
                return nodeName.c_str();
            else if( entity )
                return (const char*) entity->Marking.MarkingData;
            else
                return "";
        }
        void setNodeName(const char* name)
        {
            if( name )
                nodeName = name;
            else
                nodeName = "";
        }
        void setIconName(const std::string& name) {
            iconName = name;
        }
        const std::string& getIconName() { return iconName; }

};
class Network
{
    public:
        class less
        {
            public:
                bool operator()(const Network* n1, const Network* n2) const
                {
                    if( n1->forceID < n2->forceID )
                        return true;
                    else if( n1->forceID > n2->forceID )
                        return false;
                    else if( n1->macProtocol < n2->macProtocol )
                        return true;
                    else if( n1->macProtocol > n2->macProtocol )
                        return false;
                    else if( n1->protocol < n2->protocol )
                        return true;
                    else if( n1->protocol > n2->protocol )
                        return false;
                    else
                        return n1->hier < n2->hier;
                }
        };
        Hierarchy* hier;
        ForceIdentifierEnum8 forceID;
        std::string macProtocol;
        std::set<std::string> models;
        std::string address;
        ParameterMap* commonParameters;
        Channel* channel;
        std::string protocol;
        std::set<Node*> nodes;
        static unsigned char nextIPv4;

        Network() {}
        Network(std::string mac, std::string proto, ForceIdentifierEnum8 force, Hierarchy* hierarchy);

        void addNode(Node* node)
        {
            nodes.insert(node);
        }
        void removeNode(Node* node)
        {
            nodes.erase(node);
        }
        void addRouterModel(std::string routerModel);
        ParameterMap* getCommonParameters()
        {
            if( commonParameters )
                delete commonParameters;
            std::set<std::string>::iterator it = models.begin();
            if( models.size() == 0 )
                return 0;
            else if( models.size() == 1 )
                return Config::instance().getModelParameterList(*it);

            ParameterMap* p1 = Config::instance().getModelParameterList(*it);
            it++;
            ParameterMap* p2 = Config::instance().getModelParameterList(*it);
            it++;
            commonParameters = p1->Intersection(p2);
            while( it != models.end() )
            {
                p1 = Config::instance().getModelParameterList(*it);
                p2 = commonParameters->Intersection(p1);
                delete commonParameters;
                commonParameters = p2;
                it++;
            }
            return commonParameters;
        }
        std::string getGroup(std::string model);
        static std::string getNextAddress(std::string protocol, size_t numberNodes);
};

class Hierarchy
{
    public:
        typedef enum {Root} RootHierarchyFlag;
        static int count;
        static Hierarchy* rootHierarchy;
        unsigned int id;
        std::string name;
        Vector3D position;
        Vector3D dimensions;
        std::set<Node*, Node::less> nodes;
        std::set<Network*> networks;
        Hierarchy *parent;
        std::set<Hierarchy*> children;
        Hierarchy() : id(count+1), parent(0) { count++; }
        Hierarchy(RootHierarchyFlag f) : id(0), parent(0) { rootHierarchy = this; }
        void setName(const char* str);
        void addNode(Node* node) { nodes.insert(node); }
        void addNetwork(Network* net) { networks.insert(net); }
        void computeTerrainBounds();
};

class NetworkSet : public std::set<Network*, Network::less>
{
    std::map<int, Hierarchy*> dummyHierarchies;
    ChannelSet* channels;
    public:
        NetworkSet(ChannelSet* chans) : channels(chans) {}
        Network* find(ForceIdentifierEnum8 forceID, const std::string macProtocol, const std::string networkProtocol, Hierarchy* hier );
        void assignNodeToNetwork(Node* node, int interfaceNum, ParameterMap* pm);
        Hierarchy* getDummyHierarchy( int i )
        {
            if( i == 0 )
                return Hierarchy::rootHierarchy;
            std::map<int, Hierarchy*>::iterator it = dummyHierarchies.find(i);
            if( it != dummyHierarchies.end() )
                return it->second;
            else
                return dummyHierarchies[i] = new Hierarchy;
        }
};

class NodeSet : public std::set<Node*, Node::less>
{
    private:
        std::set<AggregateEntity*, AggregateEntity::less> aggregates;
        AggregateEntity2HierarchyMap hierarchies;
        std::set<std::string>models;
        ChannelSet channels;
        NetworkSet networks;
    public:
        NodeSet() : networks(&channels) {}
        void extractNodes(FedAmb* fed);
        std::set<std::string>& modelsUsed();
        ChannelSet& channelsUsed() { return channels; }
        NetworkSet& networksUsed() { return networks; }
        AggregateEntity2HierarchyMap& hierarchiesUsed() { return hierarchies; }
        void updateAggregate(AggregateEntity* aggregate);
        void updateEntity(PhysicalEntity* entity);
        void updateRadio(RadioTransmitter* radio);
        Node* findByNodeID(int NodeId);
        Node* findByObjectName(std::string objName);
        Network* getNetwork(Node* node, RadioTransmitter* radio);
        Network* getNetwork(const std::string& addr);
        std::string getNetworkAddress(Node* node, RadioTransmitter* radio);
        bool usesSlots();
        void createRadios();
        void assignHierarchies();
        void purgeIncompleteNodes();
        void assignNodeIds();
        void computeTerrainBounds();
        void createNetworks();
};

class TerrainBounds
{
    public:
    double minLat;
    double maxLat;
    double eastLon;
    double westLon;
    double minAlt;
    double maxAlt;
    double centerX;
    double centerY;
    double centerZ;
    bool lonCrosses180;

    TerrainBounds() :
            minLat(1.0e15),
            maxLat(-1.0e15),
            eastLon(1.0e15),
            westLon(-1.0e15),
            minAlt(1.0e15),
            maxAlt(-1.0e15),
            centerX(0.0),
            centerY(0.0),
            centerZ(0.0),
            lonCrosses180(false) {}

        void updateMinMax(double x, double& min, double&max)
        {
            if( x < min ) min = x;
            if( x > max )max = x;
        }
        double fixNegativeZero(double x)
        {
            if( x <= 0.0 && x > -1.0e-15 )
                x = 0.0;
            return x;
        }
        void  computeTerrainBounds(const std::set<Node*, Node::less>* nodes)
        {
            std::set<Node*, Node::less>::const_iterator it = nodes->begin();
            if( it != nodes->end() )
            {
                Node* node = *it;
                PhysicalEntity* ent = node->entity;
                if( ent )
                {
                    minLat = ent->WorldLocation.lat - 0.04;
                    maxLat = ent->WorldLocation.lat + 0.04;
                    if( ent->WorldLocation.lon - 0.04 < -180.0 )
                    {
                        lonCrosses180 = true;
                        eastLon = ent->WorldLocation.lon + 0.04;
                        westLon = ent->WorldLocation.lon - 0.04 + 360.0;
                    }
                    else if( ent->WorldLocation.lon + 0.04 >  180.0 )
                    {
                        lonCrosses180 = true;
                        eastLon = ent->WorldLocation.lon + 0.04 - 360.0;
                        westLon = ent->WorldLocation.lon - 0.04;
                    }
                    else
                    {
                        eastLon = ent->WorldLocation.lon + 0.04;
                        westLon = ent->WorldLocation.lon - 0.04;
                    }
                    minAlt = ent->WorldLocation.alt - 5.0;
                    maxAlt = ent->WorldLocation.alt + 5.0;
                    centerX = ent->WorldLocation.lat;
                    centerY = ent->WorldLocation.lon;
                    centerZ = ent->WorldLocation.alt;
                }
                it++;
            }
            while( it != nodes->end() )
            {
                Node* node = *it;
                PhysicalEntity* ent = node->entity;
                if( !ent )
                {
                    it++;
                    continue;
                }
                updateMinMax(ent->WorldLocation.lat, minLat, maxLat);
                std::cout.precision(10);
                std::cout << "east " << eastLon << " west " << westLon << " ent " << ent->WorldLocation.lon << std::endl << std::flush;
                if( lonCrosses180 )
                {
                    if( ent->WorldLocation.lon > eastLon && ent->WorldLocation.lon < westLon )
                    {
                        if( ent->WorldLocation.lon - eastLon <= westLon - ent->WorldLocation.lon )
                        {
                            eastLon = ent->WorldLocation.lon;
                        }
                        else
                        {
                            westLon = ent->WorldLocation.lon;
                        }
                    }
                }
                else
                {
                    if( ent->WorldLocation.lon > eastLon )
                    {
                        if( ent->WorldLocation.lon - eastLon <= westLon + 360.0 - ent->WorldLocation.lon )
                        {
                            eastLon = ent->WorldLocation.lon;
                        }
                        else
                        {
                            lonCrosses180 = true;
                            westLon = ent->WorldLocation.lon;
                        }
                    }
                    else if( ent->WorldLocation.lon < westLon )
                    {
                        if( westLon - ent->WorldLocation.lon <= ent->WorldLocation.lon + 360.0 - eastLon )
                        {
                            westLon = ent->WorldLocation.lon;
                        }
                        else
                        {
                            lonCrosses180 = true;
                            eastLon = ent->WorldLocation.lon;
                        }
                    }
                }
                std::cout << "east " << eastLon << " west " << westLon << std::endl << std::flush;

                updateMinMax(ent->WorldLocation.alt, minAlt, maxAlt);
                centerX += ent->WorldLocation.lon;
                centerY += ent->WorldLocation.lat;
                centerZ += ent->WorldLocation.alt;
                it++;
            }

            double dl;
            if( lonCrosses180 )
            {
                dl = eastLon + 360.0 - westLon;
            }
            else
            {
                dl = eastLon - westLon;
            }
            if( dl > 270.0 )
            {
                eastLon = 180.0;
                westLon = -180.0;
            }
            else
            {
                eastLon += dl / 8.0;
                westLon -= dl / 8.0;
                if( eastLon > 180.0 )
                    eastLon = 180.0;
                if( westLon < -180.0 )
                    westLon = -180.0;
            }
            dl = maxLat - minLat;
            minLat = minLat - dl;
            maxLat = maxLat + dl;
            if( minLat < -90.0 )
                minLat = -90.0;
            if( maxLat > 90.0 )
                maxLat = 90.0;
            minAlt = floor(minAlt);
            maxAlt = ceil(maxAlt);

            minLat = fixNegativeZero(minLat);
            maxLat = fixNegativeZero(maxLat);
            eastLon = fixNegativeZero(eastLon);
            westLon = fixNegativeZero(westLon);
            minAlt = fixNegativeZero(minAlt);
            maxAlt = fixNegativeZero(maxAlt);

            if( nodes->size() > 0 )
            {
                centerX = centerX/(1.0*nodes->size());
                centerY = centerY/(1.0*nodes->size());
                centerZ = centerZ/(1.0*nodes->size());
            }

        }
};

};

#endif
