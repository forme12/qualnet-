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

#ifndef _Socket_H_
#define _Socket_H_

// Assumptions:
//
// - QualNet will not send a ReportSimulationIdleState message once it has
//   started.  It will instead wait for an AdvanceSimulationTime message
//   from the MTS.
// - Each platform must be activated by a CreatePlatform message.  That is,
//   the platforms are de-activated at the start of the QualNet simulation.
//   This is not the case with other nodes that are not part of the platforms.
// - The XYZ in the platform messages refer to our local coordinate system

#include "external.h"
#include "external_util.h"
#include "external_socket.h"
#include "messageapi.h"
#include "entity.h"
#include "interfaceutil.h"
#include "partition.h"
#include "dynamic.h"
#include <map>

#define SOCKET_INTERFACE_DEFAULT_UDP_FAILURE_TIMEOUT 15 * SECOND
#define SOCKET_INTERFACE_DEFAULT_TCP_FAILURE_TIMEOUT 90 * SECOND
#define SOCKET_INTERFACE_DEFAULT_ADVANCE_BOUNDARIES 4
#define SOCKET_INTERFACE_DEFAULT_CONNECTION_ATTEMPTS 20
#define SOCKET_INTERFACE_DEFAULT_PORT 5033
#define SOCKET_INTERFACE_DEFAULT_LOG_FILE stdout
#define SOCKET_INTERFACE_DEFAULT_STATS_PRINT_REAL_TIME 1
#define SOCKET_INTERFACE_DEFAULT_STATS_PRINT_INTERVAL 10 * SECOND
#define SOCKET_INTERFACE_DEFAULT_GRAPH_PRINT_REAL_TIME 1
#define SOCKET_INTERFACE_DEFAULT_GRAPH_PRINT_INTERVAL 60 * SECOND
#define SOCKET_INTERFACE_CONNECT_SLEEP_TIME 3
#define SOCKET_INTERFACE_LOG_TIMESTAMP 1
#define SOCKET_INTERFACE_DEFAULT_MAX_CONNECTIONS 10
#define SOCKET_INTERFACE_MAX_RECEIVER_NODES 32
#define SOCKET_INTERFACE_MAX_QUEUE_SIZE 100000


// An array containing node pointers for each CommEffectsRequest recipient.
struct SocketInterface_ReceiverNodes
{
    int num;
    Node* nodePointers[SOCKET_INTERFACE_MAX_RECEIVER_NODES];
    NodeAddress nodeAddresses[SOCKET_INTERFACE_MAX_RECEIVER_NODES];
};

struct SocketInterface_MessageList
{
    int socketId;
    SocketInterface_Message* message;
    SocketInterface_MessageList* next;
};

// This structure contains all the parameters used
// to model unicast comm. effects for distributed QualNet
struct SocketInterface_UnicastModelingParameters
{
    NodeAddress senderNodeAddress;
    NodeAddress receiverNodeAddress;
    SocketInterface_ProtocolType protocol;
    clocktype sendTime;
};

// This structure contains all the modeled comm. effects
// in unicast communication for distributed QualNet
struct SocketInterface_UnicastModeledCommEffects
{
    SocketInterface_TimeType latency;
    BOOL success;
};

// This structure contains the statistical model
// for unicast comm. for a pair of nodes used in distributed QualNet
struct SocketInterface_UnicastModel
{
    SocketInterface_UnicastModelingParameters params;
    SocketInterface_UnicastModeledCommEffects effects;
};

// The comparator object used by the unicast model map
// to compare keys
struct SocketInterface_CompareUniicastModels
{
    bool operator() (
        const SocketInterface_UnicastModelingParameters* mp1, 
        const SocketInterface_UnicastModelingParameters* mp2) const
    {
        if (mp1->protocol != mp2->protocol) 
        {
            return (mp1->protocol < mp2->protocol);
        }
        else if (mp1->senderNodeAddress != mp2->senderNodeAddress)
        {
            return (mp1->senderNodeAddress < mp2->senderNodeAddress);
        }
        else if (mp1->receiverNodeAddress != mp2->receiverNodeAddress)
        {
            return (mp1->receiverNodeAddress < mp2->receiverNodeAddress);
        }
        else
        {
            return FALSE;
        }
    }
};

// The unicast model map
typedef map <const SocketInterface_UnicastModelingParameters *, 
             SocketInterface_UnicastModel *, 
             SocketInterface_CompareUniicastModels> SocketInterface_UnicastStatModel;


// This structure contains all the parameters used
// to model multicast comm. effects for distributed QualNet
struct SocketInterface_MulticastModelingParameters
{
    NodeAddress senderNodeAddress;
    NodeAddress receiverNodeAddress;
    SocketInterface_ProtocolType protocol;
    clocktype sendTime;
};

// This structure contains all the modeled comm. effects
// in multicast communication for a single receiver in distributed QualNet
struct SocketInterface_MulticastModeledCommEffects
{
    SocketInterface_TimeType latency;
    BOOL success;
};

// This structure contains all the data for a single receiver pertaining
// to multicast communication in distributed QualNet
struct SocketInterface_MulticastModeledReceiverCommEffects
{
    NodeAddress receivingPlatformAddress;
    clocktype sendTime;
    SocketInterface_MulticastModeledCommEffects effects;
};

// The comparator object used by the multicast receiver models map
// to compare keys
struct SocketInterface_CompareMulticastReceivers
{
    bool operator() (const NodeAddress* r1, const NodeAddress* r2) const
    {
        return (*r1 < *r2);
    }
};

// The multicast receiver models map
typedef map<const NodeAddress *, 
            SocketInterface_MulticastModeledReceiverCommEffects *, 
            SocketInterface_CompareMulticastReceivers> SocketInterface_MulticastModeledReceiversCommEffects;

// This structure contains the statistical model
// for multicast comm. for a group of receivers used in distributed QualNet
struct SocketInterface_MulticastModel
{
    SocketInterface_MulticastModelingParameters params;
    SocketInterface_MulticastModeledReceiversCommEffects *effects;
};

// The comparator object used by the multicast models map
// to compare keys
struct SocketInterface_CompareMulticastModels
{
    bool operator() (
        const SocketInterface_MulticastModelingParameters* mp1, 
        const SocketInterface_MulticastModelingParameters* mp2) const
    {
        if (mp1->protocol != mp2->protocol) 
        {
            return (mp1->protocol < mp2->protocol);
        }
        else if (mp1->senderNodeAddress != mp2->senderNodeAddress)
        {
            return (mp1->senderNodeAddress < mp2->senderNodeAddress);
        }
        else if (mp1->receiverNodeAddress != mp2->receiverNodeAddress)
        {
            return (mp1->receiverNodeAddress < mp2->receiverNodeAddress);
        }
        else
        {
            return FALSE;
        }
    }
};
    
// The multicast models map
typedef map<const SocketInterface_MulticastModelingParameters *, 
            SocketInterface_MulticastModel *, 
            SocketInterface_CompareMulticastModels> SocketInterface_MulticastStatModel;

struct SocketInterface_PacketHeader
{
    Int32 hashId;
    clocktype failureTimeout;
    UInt64 id1;
    UInt64 id2;
};

class SocketInterface_PacketData
{
public:
    // MTS data
    UInt64 id1;
    UInt64 id2;
    UInt32 size;
    std::string description;
    std::string receiver;
    std::string originator;
    SocketInterface_WaveformType waveform;
    SocketInterface_RouteType route;
    BOOL waveformSpecified;
    BOOL routeSpecified;
    SocketInterface_ProtocolType protocol;

    // Times representing when qualnet began processing/finished processing
    // the packet in simulation time and real time.
    clocktype beginProcessingTime;
    clocktype finishProcessingTime;
    clocktype beginProcessingRealTime;
    clocktype finishProcessingRealTime;
    clocktype beginProcessingCPUTime;
    clocktype finishProcessingCPUTime;

    // Whether the packet is multicast
    BOOL multicast;

    // The boundary that this packet falls in
    int boundary;

    // The socket this was received on
    int socketId;

    // The id in the request hash
    Int32 hashId;

    // Which node the packet is on in the tree
    EXTERNAL_TreeNode* treeNode;

    // Whether reponse for packet was sent based on statistical model
    BOOL sentStatisticalModel;

    // comm effects data form for multicast packet
    SocketInterface_MulticastModeledReceiversCommEffects *effects;

    // Failure Timeout to be used as latency for failed packets by 
    // Distributed QualNet statistical model
    SocketInterface_TimeType failureTimeout;
};

struct SocketInterface_CreatePlatformNotificationInfo
{
    NodeAddress     nodeId;
    char            entityId[MAX_STRING_LENGTH];
    UInt8           damageState;
    // platform type
    SocketInterface_PlatformType    typeSpecified;
};

#define SOCKET_INTERFACE_REQUEST_HASH_SIZE 127

class SocketInterface_RequestHash
{
private:
    Int32 nextId;
    std::map<Int32, SocketInterface_PacketData*> hash[SOCKET_INTERFACE_REQUEST_HASH_SIZE];

public:
    SocketInterface_RequestHash();

    Int32 HashRequest(SocketInterface_PacketData* packetData);

    BOOL CheckHash(Int32 id);
    void RemoveHash(Int32 id);

    SocketInterface_PacketData* PeekRequest(Int32 id);
};

struct SocketInterface_MulticastGroupList
{
    std::string groupAddr;
    int memberCount;
};

struct SocketInterface_InterfaceData
{
    SocketInterface_Sockets sockets;
    
    // The current simulation state
    SocketInterface_StateType simulationState;

    // The mode in which qualnet in running
    SocketInterface_ModeType qualnetMode;

    // The type of time management we are using
    SocketInterface_TimeManagementType timeManagement;

    // True if we are paused (paused from executing)
    BOOL paused;

    // True if we received a reset command
    BOOL resetting;

    // A flag indacting TRUE if we are waiting for an AdvanceSimulationTime
    // message
    int waitingForTimeAdvance;
    clocktype idleSentRealTime;

    // The maximum allowable sim time
    clocktype timeAllowance;

    // Whether to go into idle mode when a comm response is sent
    BOOL idleWhenResponseSent;

    // Whether the socket interface is configured for warmup
    BOOL doWarmup;

    // If QualNet is idle because of a response
    BOOL inIdleResponse;

    // The amount of time to wait before flagging a CommEffectsRequest
    // message as FAILURE
    clocktype tcpFailureTimeout;
    clocktype udpFailureTimeout;

    // What coordinate system the interface is using
    SocketInterface_CoordinateSystemType coordinateSystem;

    //Do we want source responsible multicast?
    UInt8 sourceResponsibleMulticast;

    // Whether to always generate comm effects request response successes
    BOOL alwaysSuccess;

    // Whether to be a CPU hog
    BOOL cpuHog;

    // The number of boundaries to keep track of for boundary statistics
    int advanceBoundaries;
    int firstBoundary;
    int lastBoundary;
    clocktype *boundaryTimes;

    // Per packet statistics (transactional real time)
    int totalPackets;
    int lastTotalPackets;
    double totalRealTimeSpeed;
    double lastTotalRealTimeSpeed;
    BOOL printPerPacketStats;
    FILE *statsFile;

    // The log files.  Either files created by QualNet, or stdout.
    BOOL automaticFlush;
    FILE *driverLogFile;
    FILE *responsesLogFile;
    FILE *statsLogFile;
    FILE *graphLogFile;
    FILE *errorsLogFile;
    FILE *propagationLogFile;

    // Stats log file maintenance variables
    bool logPrintRealTime; // Whether to also print real time for log files
    bool statsPrintRealTime;
    clocktype statsPrintInterval;
    bool firstStatsUpdate;
    clocktype firstStatsUpdateRealTime;
    clocktype firstStatsUpdateSimTime;
    clocktype nextStatsUpdate;

    bool inInitializePhase;
    int lastTotalMessages;
    int lastResponses;
    int lastSuccesses;
    int lastFailures;
    int lastSuccessfulFailures;
    clocktype lastStatsUpdateRealTime;
    clocktype lastStatsUpdateSimTime;

    // Graph log file maintenance variables
    bool graphPrintRealTime;
    clocktype graphPrintInterval;
    bool firstGraphUpdate;
    clocktype nextGraphUpdate;

    // TRUE if propagation should be logged
    BOOL logPropagation;

    BOOL gotFirstAdvance;

    // Pause Simulation Updates.
    BOOL pauseSendZeroDelay;
    BOOL pauseAdvanceSimTime;

    // Variables for ALCES like real time stats
    int creates;
    int updates;
    int requests;
    int responses;
    int successes;
    int failures;
    int successfulFailures;

    // A mapping translating entity IDs to QualNet nodeIds.  The mappings
    // are read from a file.
    EntityMapping entityMapping;

    // A hash table of all requests
    SocketInterface_RequestHash requestHash;

    // Tree used for unicast messages
    EXTERNAL_Tree requestTree;

    // Tree used for multicast and broadcast messages
    EXTERNAL_Tree multicastTree;

    volatile SocketInterface_MessageList* incomingMessages;
    volatile SocketInterface_MessageList* lastIncomingMessage;
    volatile int numIncomingMessages;

    volatile SocketInterface_MessageList* outgoingMessages;
    volatile SocketInterface_MessageList* lastOutgoingMessage;
    volatile int numOutgoingMessages;

#ifdef PARALLEL //Parallel
    CommunicatorId  createPlatformCommunicator;
    CommunicatorId  printGraphLogCommunicator;
#endif

    BOOL        deterministicTiming;
    std::map<std::string, int> listOfMulticastGroups;

    // Map of which nodes belong to which subnet
    std::map<NodeAddress, std::vector<NodeId> > subnetNodeMap;

    // Whether to use statistical model for Distributed QualNet
    BOOL useStatisticalModel;

    // Statistical models
    SocketInterface_UnicastStatModel *unicastStatModel;
    SocketInterface_MulticastStatModel *multicastStatModel;    

    D_Int32 serverProtocolVersion;
    D_Int32 clientProtocolVersion;

    BOOL onlyDisableMappedNodes;
};

// Dynamic variables

class D_Sleep : public D_Object
{
    private:
        Node* m_Node;
        BOOL m_IsSleeping;

    public:
        D_Sleep(Node* node) : D_Object(D_VARIABLE)
        {
            executable = TRUE;
            readable = TRUE;
            m_Node = node;
            m_IsSleeping = FALSE;
        }

        void ReadAsString(std::string& out);
        void ExecuteAsString(const std::string& in, std::string& out);
};

class D_C2Node : public D_Object
{
    private:
        Node* m_Node;
        BOOL m_IsC2Node;

    public:
        D_C2Node(Node* node) : D_Object(D_VARIABLE)
        {
            executable = TRUE;
            readable = TRUE;
            m_Node = node;
            m_IsC2Node = FALSE;
        }

        void ReadAsString(std::string& out);
        void ExecuteAsString(const std::string& in, std::string& out);
};

class D_MulticastGroups : public D_Variable
{
    private:
        Node* m_Node;

    public:
        D_MulticastGroups(Node* node)
        {
            writeable = FALSE;
            readable = TRUE;
            m_Node = node;
        }

        void ReadAsString(std::string& out);
};

typedef enum {
    PARTITION_MSG_CREATE_PLATFORM_NOTICE,
    PARTITION_MSG_PRINT_GRAPHLOG_NOTICE,
    // PARTITION_MSG_DELETE_PLATFORM_NOTICE,
} SocketInterface_PartitionCommuncationType;

void SocketInterface_HandleException(
    EXTERNAL_Interface* iface,
    SocketInterface_Exception* exception,
    SocketInterface_Message* message,
    int socketId);

void SocketInterface_HandleException(
    EXTERNAL_Interface* iface,
    SocketInterface_Exception* exception,
    SocketInterface_SerializedMessage* message,
    int socketId);

// forward declaration
class SocketInterface_CommEffectsRequestMessage;

void SocketInterface_PrintLog(EXTERNAL_Interface *iface, FILE *f, const char *s, BOOL printTimestamp = TRUE);

void SocketInterface_HandleCommEffectsReceiver(
    EXTERNAL_Interface *iface,
    Node* sendNode,
    SocketInterface_CommEffectsRequestMessage *message,
    int socketId,
    clocktype delay);

void SocketInterface_ReceiveSerializedMessage(
    EXTERNAL_Socket* s,
    SocketInterface_SerializedMessage* message);

void SocketInterface_HandlePropagationFailureSender(
    PartitionData *partition,
    NodeAddress destNodeId,
    Message *msg,
    double pathloss);

void SocketInterface_HandlePropagationFailureReceiver(
    Node *node,
    Message *msg,
    double sinr);

void SocketInterface_HandlePropagationSuccessReceiver(
    Node *node,
    Message *msg,
    double sinr);

void SocketInterface_Initialize(EXTERNAL_Interface *iface, NodeInput *nodeInput);

void SocketInterface_InitializeNodes(EXTERNAL_Interface *iface, NodeInput *nodeInput);

clocktype SocketInterface_FederationTime(EXTERNAL_Interface *iface);

void SocketInterface_SimulationHorizon(EXTERNAL_Interface* iface);

void SocketInterface_Receive(EXTERNAL_Interface *iface);

void SocketInterface_Forward(
    EXTERNAL_Interface* iface,
    Node* node,
    void* forwardData,
    int forwardSize);

void SocketInterface_Finalize(EXTERNAL_Interface *iface);

void SocketInterface_HandleWaitForWarmup(EXTERNAL_Interface *iface);

void SocketInterface_ProcessEvent(
    Node* node,
    Message* message);

void SocketInterface_PreInitialize(
    PartitionData* partition,
    int portNumber,
    EXTERNAL_Socket* s,
    char* fileString);

void SocketInterface_HandleQualnetWarning(
   char* warningString);

void SocketInterface_HandleQualnetError(
   char* errString);

void SocketInterface_BootStrap(
    int argc,
    char* argv [], 
    SimulationProperties * simProps,
    PartitionData * partitionData);

void SocketInterface_SendMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message,
    int socketId);

void SocketInterface_HandleDynamicCommandException(
    EXTERNAL_Interface* iface,
    D_Exception* exception,
    SocketInterface_Message* message,
    int socketId,
    SocketInterface_ErrorType errType);

SocketInterface_ModeType SocketInterface_GetModeType(EXTERNAL_Interface* iface);

void SocketInterface_AddNodeToSubnet(Node* node, NodeId nodeId, NodeAddress subnetAddress);

void SocketInterface_InitializeRealTime(EXTERNAL_Interface* iface);

/*
 * FUNCTION     SocketInterface_InitializeModels
 * PURPOSE      Handles initialization of comm. effects statistical models 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure.
 */
void SocketInterface_InitializeModels(SocketInterface_InterfaceData *data);

/*
 * FUNCTION     SocketInterface_FinalizeModels
 * PURPOSE      Handles finalization of comm. effects statistical models 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure.
 */
void SocketInterface_FinalizeModels(SocketInterface_InterfaceData *data);

/*
 * FUNCTION     SocketInterface_UpdateUnicastModel
 * PURPOSE      Handles updates to unicast comm.effects model
 *              for a specific set of unicast model parameters 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure,
 *  params:         Pointer to the unicast model parameters structure,
 *  effects:        Pointer to the unicast modeled effects structure.
 */
void SocketInterface_UpdateUnicastModel(
    SocketInterface_InterfaceData *data,
    SocketInterface_UnicastModelingParameters *params,
    SocketInterface_UnicastModeledCommEffects *effects);

/*
 * FUNCTION     SocketInterface_GetUnicastCommEffects
 * PURPOSE      Returns modeled unicast comm.effects model
 *              for a specified set of unicast model parameters
 * Return value:    Pointer to a constant unicast modeled effects structure.
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure,
 *  params:         Pointer to the unicast model parameters structure.
 */
const SocketInterface_UnicastModeledCommEffects* SocketInterface_GetUnicastCommEffects(
    SocketInterface_InterfaceData *data,
    SocketInterface_UnicastModelingParameters *params);

/*
 * FUNCTION     SocketInterface_UpdateMulticastModel
 * PURPOSE      Handles updates to multicast comm.effects model for multiple receivers,
 *              for a specific set of multicast model parameters 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure,
 *  params:         Pointer to the multicast model parameters structure,
 *  effects:        Pointer to the multicast modeled effects hashmap for multiple receivers.
 */
void SocketInterface_UpdateMulticastModel(
    SocketInterface_InterfaceData *data,
    SocketInterface_MulticastModelingParameters *params,
    SocketInterface_MulticastModeledReceiversCommEffects *effects);

/*
 * FUNCTION     SocketInterface_UpdateMulticastModelSingleReceiver
 * PURPOSE      Handles updates to multicast comm.effects model for a single receiver,
 *              for a specific set of multicast model parameters 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure,
 *  params:         Pointer to the multicast model parameters structure,
 *  effects:        Pointer to the multicast modeled effects structure for a single receiver.
 */
void SocketInterface_UpdateMulticastModelSingleReceiver(
    SocketInterface_InterfaceData *data,
    SocketInterface_MulticastModelingParameters *params,
    SocketInterface_MulticastModeledReceiverCommEffects *effects);

/*
 * FUNCTION     SocketInterface_GetMulticastCommEffects
 * PURPOSE      Returns modeled multicast comm.effects model (multiple receivers)
 *              for a specified set of multicast model parameters
 * Return value:    Pointer to a constant multicast modeled effects hashmap.
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure,
 *  params:         Pointer to the multicast model parameters structure.
 */
const SocketInterface_MulticastModeledReceiversCommEffects* SocketInterface_GetMulticastCommEffects(
    SocketInterface_InterfaceData *data,
    SocketInterface_MulticastModelingParameters *params);

/*
 * FUNCTION     SocketInterface_SendModeledResponses
 * PURPOSE      Checks and sends comm. effects responses to designated receivers
 *              based on the availability, in the model, of comm. effects for 
 *              a specified set of unicast/multicast model parameters
 * Return value:    Boolean value indicating whether model for corresponding
 *                  parameters were found
 * Parameters:
 *  iface:          Pointer to the EXTERNAL_Interface (mts interface) structure,
 *  packetData:     Pointer to the SocketInterface_PacketData structure corresponding to the packet request.
 *  params:         Pointer to the unicast/multicast model parameters structure.
 */
BOOL SocketInterface_SendModeledResponses(
    EXTERNAL_Interface *iface,
    SocketInterface_PacketData *packetData,
    void *params);

/*
 * FUNCTION     SocketInterface_GetMessageIds
 * PURPOSE      Retrieve the two message IDs from the SocketInterface_PacketHeader
 *              header of message from socket socket
 * Return value:    BOOL indicate whether successfully or not
 * Parameters:
 *  payload:        Pointer to payload of message from socket interface
 *                  which contain header
 *  size:           Size of the payload.
 *  msgId1:         For returing ID1
 *  msgId2:         For returing ID2
 */
BOOL SocketInterface_GetMessageIds(
    char *payload,
    int size,
    UInt64 *msgId1,
    UInt64 *msgId2);

// SocketInterface_GetEntityMappingStructure is used to get the proper mapping structure
// for the socket interface.
EntityMapping* SocketInterface_GetEntityMappingStructure(EXTERNAL_Interface* iface);

void SocketInterface_CheckWaveformValidity(
    EXTERNAL_Interface* iface,
    SocketInterface_WaveformType waveform,
    BOOL waveformSpecified,
    BOOL routeSpecified);

#endif /* _Socket_H_ */
