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

#ifndef _PHY_CONNECTIVITY_H_
#define _PHY_CONNECTIVITY_H_

#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <set>

#include "stats_global.h"

using namespace std;

#ifdef _WIN32
#include <hash_map>
using namespace stdext;
#else
#include <ext/hash_map>
using namespace __gnu_cxx;
#endif

#define PHY_CONN_DEFAULT_CONNECTIVITY_INTERVAL (600 * SECOND)

class PHY_CONN_NodePositionData
{
public:
    enum MsgType
    {
        LISTENABLE_CHANNEL_NODE_INFO,
        LISTENING_CHANNEL_NODE_INFO,
        LINK_NODE_INFO,
        SATCOM_NODE_INFO,
        ETHERNET_NODE_INFO, 
        RX_POWER_INFO,
        CONNECTIVITY,
        PATHLOSS,
    };

    BOOL phyConnEnabled;
    BOOL endSimulation;

    BOOL listenableChannelSent;
    BOOL linkInformationSent;
    BOOL subnetInformationSent;
    BOOL ethernetInformationSent;

    PHY_CONN_NodePositionData () {
        phyConnEnabled = FALSE;
        listenableChannelSent = FALSE;
        linkInformationSent = FALSE;
        subnetInformationSent = FALSE;
        ethernetInformationSent = FALSE;
        endSimulation = TRUE;
    }
    struct SatComNodeInfo
    {
        int nodeId;
        int interfaceIndex;        

        SatComNodeInfo() {};
        SatComNodeInfo(int n, int f): 
            nodeId(n), interfaceIndex(f){};

        const bool operator < (const SatComNodeInfo &info2) const
        {
            if(nodeId != info2.nodeId) return nodeId < info2.nodeId;

            return interfaceIndex < info2.interfaceIndex;
        };

    };

    struct ChannelNodeInfo
    {
        int nodeId;
        int phyIndex;        
        //
        PhyModel model;
        AntennaModelType type;
        float antennaHeight;
        double txPower_mW;
        double systemLossIndB;
        double gain_dBi; // Antenna gain, non-directional        
        //        
        ChannelNodeInfo(int n, int p, PhyModel m = PHY_NONE, AntennaModelType t = ANTENNA_OMNIDIRECTIONAL, float ah = 0.0, double tp = 0.0, double loss = 0.0, double gain = 0.0) : nodeId(n), phyIndex(p), type(t), model(m), antennaHeight(ah), txPower_mW(tp), systemLossIndB(loss), gain_dBi(gain) {} 

        const bool operator < (const ChannelNodeInfo &info2) const
        {
            if(nodeId != info2.nodeId) {
                return nodeId < info2.nodeId;
            }
            return phyIndex < info2.phyIndex;
            
        };
    };

    struct ListeningChannelNodeInfo
    {
        int nodeId;
        int phyIndex;
        
        ListeningChannelNodeInfo(int n, int p) :nodeId(n), phyIndex(p) {}         

        // Sorting is only done on nodeId, phyIndex since that is unique
        const bool operator < (const ListeningChannelNodeInfo &info2) const
        {
            if (nodeId < info2.nodeId) 
            {
                return true;
            }
            if (nodeId > info2.nodeId) 
            {
                return false;
            }
            if (phyIndex < info2.phyIndex)
            {
                return true;
            }
            
            return false;
        };

        const bool operator == (const ListeningChannelNodeInfo &info2) const
        {
            return nodeId == info2.nodeId && phyIndex == info2.phyIndex;
        };
    };
    
    // ListeningChannelNodeList implements a DATA_Summarizer which shares
    // channel listening information among all partitions.  They key is
    // the channel index.
    typedef set<ListeningChannelNodeInfo> ListeningSet;
    typedef set<ChannelNodeInfo> ListenableSet;

    template <typename NodeId, typename SetT >
    class ChannelNodeList : public DATA_Summarizer<NodeId, SetT >
    {
    public:

        virtual void AddDuplicateKey(
            SetT& summary, const SetT& newSummary)
        {
                                
            for (typename SetT::const_iterator it = newSummary.begin();
                it != newSummary.end();
                it++)
            {
                summary.insert(*it);
            }

        };

#ifdef USE_MPI
        virtual UInt32 SummarySize(const SetT* summary)
        {
            return sizeof(UInt32) + summary->size() * sizeof(typename SetT::key_type);//sizeof(SetInfo);
        };
        virtual void SerializeSummary(
            const SetT* summary,
            UInt8* out)
        {
            UInt32 size = summary->size();
            //SetInfo* info;
            typename SetT::key_type *info;

            // Copy list size
            memcpy(out, &size, sizeof(UInt32));

            // Copy set members
            info = (typename SetT::key_type*) (out + sizeof(UInt32));
            for (typename SetT::const_iterator it = summary->begin();
                it != summary->end();
                it++)
            {
                *info = *it;
                info++;
            }
        };
        virtual void DeserializeSummary(
            SetT* summary,
            const UInt8* in)
        {
            UInt32 size;
            typename SetT::key_type* info;

            // Copy list members
            memcpy(&size, in, sizeof(UInt32));

            // Copy set members
            summary->clear();
            info = (typename SetT::key_type*) (in + sizeof(UInt32));
            for (int i = 0; i < size; i++)
            {
                summary->insert(*info);
                info++;
            }
        };
#endif    
    
    };

    typedef ChannelNodeList<NodeId, ListeningSet>  ListeningChannelNodeList;
    typedef ChannelNodeList<NodeId, ListenableSet> ListenableChannelNodeList;

    ListeningChannelNodeList listeningChannelNodeList;
    ListenableChannelNodeList listenableChannelNodeList;

    void GetListeningInfo(            
            NodeId nodeId,
            int phyIndex,
            int channelIndex,            
            BOOL* found);
    void GetChannelNodeInfo(
            NodeId nodeId,
            int phyIndex,
            int channelIndex,
            ListenableSet::iterator* info,
            BOOL* found);

    // for satcom/switched ethernet information
    // < (subnetIndex, type), (nodeId, phyIndex)>
    typedef std::map<std::pair<int, int>, std::set<SatComNodeInfo> *> PER_PARTITION_SATCOM_NODELIST;
    PER_PARTITION_SATCOM_NODELIST PerPartitionSatcomNodeList;

    // for link information
    // <(senderId, interfaceIndex1), (receiverId, interfaceIndex2)>
    typedef std::map<std::pair<int, int>, std::pair<int, int> > PER_PARTITION_LINK_NODELIST;
    PER_PARTITION_LINK_NODELIST PerPartitionLinkNodeList;
    
    // for 802_3 information
    // <(senderId, interfaceIndex1), (receiverId, interfaceIndex2)>
    //typedef std::map<std::pair<int, int>, std::set<SatComNodeInfo> *> 
    //    PER_PARTITION_802_3_NODELIST;
    PER_PARTITION_SATCOM_NODELIST PerPartition802_3NodeList;

    // below is collected during system initialization
    // can not process dynamic position changes

    typedef DATA_Summarizer<NodeId, Coordinates> NodePositionDatabase;
    NodePositionDatabase positions;

    Coordinates ReturnNodePosition(NodeId nodeId);    
    
    enum InterfaceType{
        WIRELESS,
        SATCOM_GROUND,
        SATCOM_SATELLITE,
        WIRED_LINK,
        ETHERNET,
        SWITCHED_ETHERNET,
        NCW_GROUND,
        NCW_SATELLITE, 
    };

    struct AdjacencyNodeKey
    {
        int rcverId; // Receiver node id
        int channelIndex;
        int senderIndex; // Phy index of sender
        int rcverIndex; // Phy index of receiver

        bool operator < (const AdjacencyNodeKey &info2) const
        {
            if(rcverId != info2.rcverId) 
            {
                return rcverId < info2.rcverId;
            }
            if(channelIndex != info2.channelIndex)
            {
                return channelIndex < info2.channelIndex;
            }
            if(senderIndex != info2.senderIndex)
            {
                return senderIndex < info2.senderIndex;
            }
            return rcverIndex < info2.rcverIndex;
        
        };
    
        bool operator == (const AdjacencyNodeKey &info2) const
        {
            return rcverId == info2.rcverId 
                && channelIndex == info2.channelIndex 
                && senderIndex == info2.senderIndex 
                && rcverIndex == info2.rcverIndex;
        };

        AdjacencyNodeKey(int r, int c, int s, int r_i) :
            rcverId(r), channelIndex(c), 
                senderIndex(s), rcverIndex(r_i)
        { };
    };
    struct AdjacencyNodeValue
    {
        BOOL connected; // If both nodes are listening to same channel
        InterfaceType sType;
        InterfaceType rType;
        double pathloss_dB;
        double rxPower_mw;
        double rxPower_mwWorst;
        BOOL reachable;
        BOOL reachableWorst;
        MsgType syncType;
                
        AdjacencyNodeValue(
            InterfaceType st, InterfaceType rt, BOOL conn,
            double p): 
                sType(st),
                rType(rt), connected(conn), pathloss_dB(p)  {  
                    rxPower_mw = 0;
                    rxPower_mwWorst = 0;
                    reachable = FALSE;
                    reachableWorst = FALSE;
                    syncType = PATHLOSS;
                }

        AdjacencyNodeValue( 
            InterfaceType st, InterfaceType rt/*, BOOL conn,
            double mw, double mwWorst*/): 
                sType(st),
                rType(rt)/*, connected(conn), rxPower_mw(mw),
                rxPower_mwWorst(mwWorst)*/ {
                pathloss_dB = -1;
//                channelIndex = -1;
                connected = FALSE;
                rxPower_mw = -1;
                rxPower_mwWorst = -1;
                reachable = FALSE;
                reachableWorst = FALSE;
                syncType = RX_POWER_INFO;                
                }

    };
    
    //typedef std::set<AdjacencyNode> AdjacencyList;
    typedef std::map<AdjacencyNodeKey, AdjacencyNodeValue> AdjacencyList;
    typedef std::map<NodeId,  AdjacencyList> TxConnectivity;

    class ConnectivityMap : public DATA_Summarizer<NodeId, TxConnectivity>
    {    
    public:
        MsgType global_syncType;
        ConnectivityMap() {global_syncType = PATHLOSS;}
        virtual void AddDuplicateKey(
            TxConnectivity& summary,
            const TxConnectivity& newSummary);

#ifdef USE_MPI
        virtual UInt32 SummarySize(const TxConnectivity* summary);
        virtual void SerializeSummary(
            const TxConnectivity* summary,
            UInt8* out);
        virtual void DeserializeSummary(
            TxConnectivity* summary,
            const UInt8* in);
#endif
    };

    ConnectivityMap connectivity;
    

    void HandleChannelPhyConnInsertion(
        Node* node,
        PartitionData* partition);

#ifdef USE_MPI
    vector<Message *> size_msgList;
    vector<Message *> snd_sizeMsgList;
    vector<Message*> msgList;
    vector<Message*> snd_msgList;
#endif
};

typedef pair<PHY_CONN_NodePositionData::AdjacencyNodeKey, 
    PHY_CONN_NodePositionData::AdjacencyNodeValue> PairAdjacencyKeyValue;
void PHY_CONN_ObtainNodesOnOtherPartitionPositions(
        PartitionData * partitionData, 
        NodeInput* nodeInput);


void PHY_CONN_CollectConnectSample(PartitionData* partition);
void PHY_CONN_HandlePhyConnTableUpdate(Node *senderNode,
    Node *receiverNode, int channelIndex, int phyIndex);
clocktype PHY_CONN_GetConnectSampleInterval(Node *node,
    Message *msg);

void PHY_CONN_CollectChannelPositionInfo(PartitionData *partitionData);

void PHY_CONN_SendSatComNodeMsg(PartitionData *partitionData);

void PHY_CONN_Send802_3NodeMsg(PartitionData *partitionData);

void PHY_CONN_SendLinkNodeMsg(PartitionData *partitionData);


BOOL PHY_CONN_ReturnPhyConnectivity(PartitionData *partition,
        int txNodeId, int rxNodeId); 

BOOL PHY_CONN_ReturnPotPhyConnectivity(
    PartitionData *partition, int txNodeId, int rxNodeId);

BOOL PHY_CONN_ReturnPhyConnectivity(PartitionData *partition,
    int txNodeId, vector<pair<PHY_CONN_NodePositionData::AdjacencyNodeKey, 
    PHY_CONN_NodePositionData::AdjacencyNodeValue > > &); 

#endif
