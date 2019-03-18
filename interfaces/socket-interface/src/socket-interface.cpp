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

#include <iostream>
#include <vector>
#include <math.h>
#include <list>
#include <map>

using namespace std;


#ifndef _WIN32 /* unix/linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#ifdef sgi
#include <unistd.h>
#endif
#endif

#ifdef _WIN32 /* windows */
#include <winsock2.h>
#endif

#include "api.h"
#include "app_util.h"
#include "partition.h"
#include "WallClock.h"
#include "mac_csma.h"
#include "mac_tdma.h"
#include "antenna.h"
#include "socket-interface.h"
#include "messageapi.h"
#include "platform.h"
#include "interfaceutil.h"
#include "network_ip.h"
#ifdef JNE_LIB
#include "routing_ces_malsr.h"
#include "network_ces_subnet.h"
#include "configparser.h"
#endif
#include "entity.h"
#include "dynamic.h"


#if defined(_WIN32)
static clocktype llround(double x)
{
    double xx = floor(x + 0.5);
    
    clocktype xxx = (clocktype)xx;
    
    return xxx;
}
#endif /* _WIN32 */

// The gHaveSocketIface variable is used ONLY for propagation logging,
static bool gHaveSocketIface = false;

#define DEBUG_MESSAGE_SENT 0

// SocketInterface_Sockets constructor
SocketInterface_Sockets::SocketInterface_Sockets()
{
    numPorts = 0;
    numConnections = 0;

    pthread_mutex_init(&socketMutex, NULL);
    pthread_mutex_init(&receiverMutex, NULL);
    pthread_cond_init(&receiverNotFull, NULL);
    pthread_cond_init(&receiverNotEmpty, NULL);
    pthread_mutex_init(&senderMutex, NULL);
    pthread_cond_init(&senderNotFull, NULL);
    pthread_cond_init(&senderNotEmpty, NULL);
}

// Returns the simulation time from the MTS perspective
// Subtracts warmup time
// This function should be used any place when dealing with external time
static clocktype SocketInterface_Timestamp(EXTERNAL_Interface* iface)
{
    return EXTERNAL_QuerySimulationTime(iface) - EXTERNAL_QueryWarmupTime(iface);
}

// Returns the simulation time for a message from the MTS
// Add warmup time
// This function should be used any place when scheduling a messae
static clocktype SocketInterface_TimestampToQualNet(EXTERNAL_Interface* iface, SocketInterface_TimeType time)
{
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;

    if (data->simulationState == SocketInterface_StateType_Standby || data->simulationState == SocketInterface_StateType_Warmup)
    {
        return (clocktype) llround(time * SECOND);
    }
    else if (EXTERNAL_QueryWarmupTime(iface) == -1)
    {
        return (clocktype) llround(time * SECOND);
    }
    else
    {
        return (clocktype) llround(time * SECOND + EXTERNAL_QueryWarmupTime(iface));
    }
}

// Static functions
SocketInterface_RequestHash::SocketInterface_RequestHash()
{
    nextId = 1;
}

Int32 SocketInterface_RequestHash::HashRequest(SocketInterface_PacketData* packetData)
{
    Int32 requestId = nextId++;
    int index = requestId % SOCKET_INTERFACE_REQUEST_HASH_SIZE;

    ERROR_Assert(index >= 0 && index < SOCKET_INTERFACE_REQUEST_HASH_SIZE, "Invalid index");

    hash[index][requestId] = packetData;

    return requestId;
}

BOOL SocketInterface_RequestHash::CheckHash(Int32 id)
{
    int index = id % SOCKET_INTERFACE_REQUEST_HASH_SIZE;
    std::map<Int32, SocketInterface_PacketData*>::iterator it;

    ERROR_Assert(index >= 0 && index < SOCKET_INTERFACE_REQUEST_HASH_SIZE, "Invalid index");

    it = hash[index].find(id);

    if (it == hash[index].end())
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

void SocketInterface_RequestHash::RemoveHash(Int32 id)
{
    int index = id % SOCKET_INTERFACE_REQUEST_HASH_SIZE;
    std::map<Int32, SocketInterface_PacketData*>::iterator it;

    it = hash[index].find(id);

    if (it != hash[index].end())
    {
        // Remove it and return
        hash[index].erase(it);
    }
}

SocketInterface_PacketData* SocketInterface_RequestHash::PeekRequest(Int32 id)
{
    int index = id % SOCKET_INTERFACE_REQUEST_HASH_SIZE;
    std::map<Int32, SocketInterface_PacketData*>::iterator it;
    SocketInterface_PacketData* data;

    it = hash[index].find(id);

    if (it == hash[index].end())
    {
        return NULL;
    }
    else
    {
        // Return data without removing
        data = it->second;
        return data;
    }
}

static void StringSplit(
    std::string* str,
    char* delim,
    std::vector<std::string>* out)
{
    unsigned pos = 0;
    unsigned next;

    out->clear();

    // Get each token before up to the last space
    next = str->find(delim, pos);
    while (next != -1)
    {
        if (next != pos)
        {
            out->push_back(str->substr(pos, next - pos));
        }

        pos = next + 1;
        next = str->find(delim, pos);
    }

    // Get the token after the final space
    if (pos < str->size())
    {
        out->push_back(str->substr(pos));
    }
}

static void SocketInterface_ScheduleMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message,
    clocktype timestamp)
{
    Message* delayedMessage;
    clocktype delay;

    delayedMessage = MESSAGE_Alloc(
        iface->partition->firstNode,
        EXTERNAL_LAYER,
        EXTERNAL_SOCKET,
        MSG_EXTERNAL_SOCKET_INTERFACE_DelayedMessage);

    // Add the message to the info field
    MESSAGE_InfoAlloc(
        iface->partition->firstNode,
        delayedMessage,
        sizeof(SocketInterface_Message*));

    memcpy(MESSAGE_ReturnInfo(delayedMessage), &message, sizeof(SocketInterface_Message*));

    // Compute delay
    if (timestamp < EXTERNAL_QuerySimulationTime(iface))
    {
        delay = 0;
    }
    else
    {
        delay = timestamp - EXTERNAL_QuerySimulationTime(iface);
    }

    MESSAGE_Send(
        iface->partition->firstNode,
        delayedMessage,
        delay);
}

void SocketInterface_SendMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message,
    int socketId)
{
    SocketInterface_InterfaceData* data;
    SocketInterface_MessageList* node;

    data = (SocketInterface_InterfaceData*) iface->data;

    // Create linked list node
    node = new SocketInterface_MessageList;
    node->message = message->Duplicate();
    node->socketId = socketId;
    node->next = NULL;

    // Wait until the buffer is not full
    pthread_mutex_lock(&data->sockets.senderMutex);
    if (data->numOutgoingMessages >= SOCKET_INTERFACE_MAX_QUEUE_SIZE)
    {
        pthread_cond_wait(
            &data->sockets.senderNotFull,
            &data->sockets.senderMutex);
    }

    // Add to outgoing queue
    if (data->outgoingMessages == NULL)
    {
        data->outgoingMessages = node;
    }
    else
    {
        data->lastOutgoingMessage->next = node;
    }
    data->lastOutgoingMessage = node;
    data->numOutgoingMessages++;
    pthread_mutex_unlock(&data->sockets.senderMutex);

    // Signal that the sending thread is not empty
    pthread_cond_signal(&data->sockets.senderNotEmpty);
}

static void SocketInterface_HandleSimulationStateMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_StateType oldState,
    SocketInterface_StateType newState)
{
    SocketInterface_InterfaceData* data;
    data = (SocketInterface_InterfaceData*) iface->data;

    // Create a simulation state response message.
    SocketInterface_SimulationStateMessage *stateResponse;
    if (data->clientProtocolVersion >= 6)
    {
        stateResponse =
           new SocketInterface_SimulationStateMessage(newState,
                                          oldState,
                                          SocketInterface_Timestamp(iface) / (double)SECOND);
    }
    else
    {
        stateResponse = new SocketInterface_SimulationStateMessage(newState, oldState);
    }

    // Now to send the message to all the active connections.
    for (unsigned i = 0; i < data->sockets.connections.size(); i++)
    {
        if (data->sockets.activeConnections[i]
            && EXTERNAL_SocketValid(data->sockets.connections[i]))
        {
            // Found an active connection send the warning.
            SocketInterface_SendMessage(iface, stateResponse, i);

            data->responses++;
        }
    }

    // Print to the response log
    std::string str;
    stateResponse->Print(&str);
    str += "\n";

    SocketInterface_PrintLog(
        iface,
        data->responsesLogFile,
        str.c_str());

    delete stateResponse;
}

static void SocketInterface_ChangeSimulationState(
    EXTERNAL_Interface* iface,
    SocketInterface_StateType newState)
{
    SocketInterface_InterfaceData *data = (SocketInterface_InterfaceData*) iface->data;

    // First check for valid transition
    switch (newState)
    {
        case SocketInterface_StateType_Warmup:
            if (data->simulationState != SocketInterface_StateType_Standby)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidTransition,
                    "Warmup state may only be transitioned to from Standby");
            }
            break;

        case SocketInterface_StateType_Initialized:
            if (data->simulationState != SocketInterface_StateType_Standby
                && data->simulationState != SocketInterface_StateType_Warmup)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidTransition,
                    "Initialized state may only be transitioned to from Standby or Warmup");
            }

            // Update the bit for Initialization Phase
            data->inInitializePhase = TRUE;
            break;

        case SocketInterface_StateType_Paused:
            if (data->simulationState != SocketInterface_StateType_Initialized
                && data->simulationState != SocketInterface_StateType_Executing)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidTransition,
                    "Paused state may only be transitioned to from Initialized or Executing");
            }

            // If pausing from the executing state record the pause time
            if (data->simulationState == SocketInterface_StateType_Executing)
            {
                // We pause time only if the parameter 
                // SOCKET-INTERFACE-PAUSE-ADVANCE-SIMULATION-TIME is set to NO, 
                //else if do not pause time. We change the state either way.
                if (!data->pauseAdvanceSimTime)
                {
                    EXTERNAL_Pause(iface);
                }
                data->paused = TRUE;
            }
            break;

        case SocketInterface_StateType_Executing:
            if (data->simulationState != SocketInterface_StateType_Paused
                && data->simulationState != SocketInterface_StateType_Initialized)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidTransition,
                    "Executing state may only be transitioned to from Paused or Initialized");
            }

            if (data->paused)
            {
                // We unpause time only if the parameter 
                // SOCKET-INTERFACE-PAUSE-ADVANCE-SIMULATION-TIME is set to NO, 
                //else if do not pause time. We change the state either way.
                if (!data->pauseAdvanceSimTime)
                {
                    EXTERNAL_Resume(iface);
                }
                data->paused = FALSE;
            }
            else if (data->simulationState == SocketInterface_StateType_Paused)
            {
                SocketInterface_InitializeRealTime(iface);
            }
            else if (data->simulationState == SocketInterface_StateType_Initialized)
            {
                SocketInterface_InitializeRealTime(iface);
            }
            break;

        case SocketInterface_StateType_Stopping:
            break;

        case SocketInterface_StateType_Resetting:
            break;

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidTransition,
                "Unknown state");
            break;
    }

    // Save the old state for the response message
    SocketInterface_StateType oldState = data->simulationState;

    // Set new state
    data->simulationState = newState;

    // send the response for the change of state
    SocketInterface_HandleSimulationStateMessage(iface, oldState, newState);
}

static void SocketInterface_HandleAdvanceTime(
    EXTERNAL_Interface *iface,
    SocketInterface_AdvanceTimeMessage *advanceMessage)
{
    SocketInterface_InterfaceData *data;
    clocktype advanceDelay;
    int i;
    int numBoundaries;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Print warning if qualnet in real time mode.
    if (data->timeManagement == SocketInterface_TimeManagementType_RealTime)
    {
        printf ("Warning: QualNet in realTime mode\n");
        return;
    }

    if (!data->gotFirstAdvance)
    {
        data->gotFirstAdvance = TRUE;
        printf("SOCKET-INTERFACE received first time advance to %f.\n",
           advanceMessage->m_TimeAllowance);
    }

    data->timeAllowance = SocketInterface_TimestampToQualNet(iface, advanceMessage->m_TimeAllowance);

    // Since we just received a time advance we are no longer waiting
    // for a time advance (duh).  This value is set to TRUE when an
    // idle state is sent back to the MTS, and also at the start of
    // the simulation.  See SocketInterface_SimulationHorizon.
    data->waitingForTimeAdvance = FALSE;
    data->inIdleResponse = FALSE;

    // Make sure the time the idle message was sent is valid.  It will be
    // invalid for the first time advance we get.
    if (data->idleSentRealTime != 0)
    {
        advanceDelay = EXTERNAL_QueryRealTime(iface)
                       - data->idleSentRealTime;

        // If this is the first recorded boundary
        if (data->firstBoundary == -1)
        {
            data->boundaryTimes[0] = advanceDelay;
            data->firstBoundary = 0;
            data->lastBoundary = 0;
        }
        else
        {
            // If the boundary buffer is full
            numBoundaries = data->lastBoundary - data->firstBoundary + 1;
            if (numBoundaries == data->advanceBoundaries)
            {
                // Get rid of the earliest boundary, and move the rest down
                for (i = 0; i < numBoundaries - 1; i++)
                {
                    data->boundaryTimes[i] = data->boundaryTimes[i + 1];
                }

                // Replace the last boundary
                data->boundaryTimes[numBoundaries - 1] = advanceDelay;
                data->firstBoundary++;
                data->lastBoundary++;
            }
            else
            {
                // Add in the last boundary
                data->boundaryTimes[numBoundaries] = advanceDelay;
                data->lastBoundary++;
            }
        }
    }
}

static void SocketInterface_HandleReceiverMessage(
    EXTERNAL_Interface *iface,
    SocketInterface_Message *message,
    int socketId)
{
    SocketInterface_InterfaceData *data;
    SocketInterface_Message* messageToQueue = NULL;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    message->SetSocketId(socketId);
    messageToQueue = message;

    // If we need to queue a message for the main thread
    if (messageToQueue != NULL)
    {
        SocketInterface_MessageList* node;

        // Create linked list node
        node = new SocketInterface_MessageList;
        memset(node, 0, sizeof(SocketInterface_MessageList));
        node->message = messageToQueue;
        node->socketId = socketId;

        // Add to incoming message list
        pthread_mutex_lock(&data->sockets.receiverMutex);

        // If running in real time it is possible for us to get overloaded
        // with messages.  If we've filled up the queue then wait.
        if (data->timeManagement == SocketInterface_TimeManagementType_RealTime &&
            data->numIncomingMessages >= SOCKET_INTERFACE_MAX_QUEUE_SIZE)
        {
            pthread_cond_wait(
                &data->sockets.receiverNotFull,
                &data->sockets.receiverMutex);
        }
        if (data->incomingMessages == NULL)
        {
            data->incomingMessages = node;
        }
        else
        {
            data->lastIncomingMessage->next = node;
            data->lastIncomingMessage = node;
        }
        data->lastIncomingMessage = node;
        data->numIncomingMessages++;
        pthread_mutex_unlock(&data->sockets.receiverMutex);

        // Signal that the receiver is not empty
        pthread_cond_signal(&data->sockets.receiverNotEmpty);
    }
}

void SocketInterface_HandleDynamicCommandException(
    EXTERNAL_Interface* iface,
    D_Exception* exception,
    SocketInterface_Message* message,
    int socketId,
    SocketInterface_ErrorType errType)
{
    SocketInterface_InterfaceData* data;
    std::string str;
    std::string errorString;

    errorString = exception->GetError();
    SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(errType, &errorString);
    error->SetMessage(message);

    SocketInterface_SendMessage(iface, error, socketId);

    data = (SocketInterface_InterfaceData*) iface->data;
    error->Print(&str);
    str += "\n";
    SocketInterface_PrintLog(iface, data->errorsLogFile, str.c_str());

    delete error;
}

static void GetMulticastGroups(Node* node, std::string& out)
{
    NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
    ListItem *item = ip->multicastGroupList->first;
    NetworkIpMulticastGroupEntry *entry;
    char addr[MAX_STRING_LENGTH];
    BOOL first = TRUE;
    int size = 0;

    // Initially no output
    out = "";

    // Go through the group list
    while (item)
    {
        entry = (NetworkIpMulticastGroupEntry *) item->data;
        size++;
        // Group already exists, so incrememt member count.
        if (entry->memberCount > 0)
        {
            if (first)
            {
                // Initialize out with group address
                IO_ConvertIpAddressToString(entry->groupAddress, addr);
                out += std::string(addr);
                first = FALSE;
            }
            else
            {
                // Create string " xxx.xxx.xxx.xxx" and append to out
                IO_ConvertIpAddressToString(entry->groupAddress, addr);
                out += std::string(" ") + std::string(addr);
            }
        }

        item = item->next;
    }
}

void SocketInterface_HandleException(
    EXTERNAL_Interface* iface,
    SocketInterface_Exception* exception,
    SocketInterface_Message* message,
    int socketId)
{
    SocketInterface_InterfaceData* data;
    std::string str;
    std::string errorString;
    // NOTE: Might need extra function for sender thread

    SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(exception->GetError(), &exception->GetErrorString());
    error->SetMessage(message);

    SocketInterface_SendMessage(iface, error, socketId);

    data = (SocketInterface_InterfaceData*) iface->data;
    error->Print(&str);
    str += "\n";
    SocketInterface_PrintLog(iface, data->errorsLogFile, str.c_str());

    delete error;
}

void SocketInterface_HandleException(
    EXTERNAL_Interface* iface,
    SocketInterface_Exception* exception,
    SocketInterface_SerializedMessage* message,
    int socketId)
{
    SocketInterface_InterfaceData* data;
    std::string str;
    std::string errorString;
    // NOTE: Might need extra function for sender thread

    SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(exception->GetError(), &exception->GetErrorString());
    error->SetSerializedMessage(message);

    SocketInterface_SendMessage(iface, error, socketId);

    data = (SocketInterface_InterfaceData*) iface->data;
    error->Print(&str);
    str += "\n";
    SocketInterface_PrintLog(iface, data->errorsLogFile, str.c_str());

    delete error;
}

void* SocketInterface_ReceiverThread(void* voidIface)
{
    EXTERNAL_Interface* iface = (EXTERNAL_Interface*) voidIface;
    SocketInterface_InterfaceData* socketData = (SocketInterface_InterfaceData*) iface->data;
    SocketInterface_SerializedMessage data;
    int err;
    unsigned i;
    fd_set readSet;
    int max;
    timeval t;
    BOOL gotData;
    char str[MAX_STRING_LENGTH];

    // Continue receiving until end of program
    while (socketData->simulationState != SocketInterface_StateType_Shutdown)
    {
        // Create set of all input sockets
        FD_ZERO(&readSet);
        max = -1;
        for (i = 0; i < (unsigned)socketData->sockets.numPorts; i++)
        {
            FD_SET(
                (unsigned int) socketData->sockets.listeningSockets[i]->socketFd,
                &readSet);
            if (socketData->sockets.listeningSockets[i]->socketFd > max)
            {
                max = socketData->sockets.listeningSockets[i]->socketFd;
            }
        }

        // If CPU hog return from select immediately.  If not cpu hog then
        // wait for 1 milli second.
        if (socketData->cpuHog)
        {
            t.tv_sec = 0;
            t.tv_usec = 0;
        }
        else
        {
            t.tv_sec = 0;
            t.tv_usec = 1000;
        }

        // Select for readable sockets
        err = select(max + 1, &readSet, NULL, NULL, &t);
        if (err == -1)
        {
#ifndef _WIN32
            // In unix EINTR is interrupted system call.  This means that the
            // select call did not finish waiting which is not an error,
            // so set available to false and return no error.
            if (errno == EINTR)
            {
                continue;
            }

            perror("Error calling select");
            return NULL;
#endif
        }

        // Now see which sockets have new data
        for (i = 0; i < (unsigned)socketData->sockets.numPorts; i++)
        {
            //cout << "Socket fd is " << socketData->sockets.listeningSockets[i]->socketFd << endl;
            if (EXTERNAL_SocketValid(socketData->sockets.listeningSockets[i]) &&
                FD_ISSET((
                unsigned int) socketData->sockets.listeningSockets[i]->socketFd,
                &readSet))
            {
                CreateNewConnection(
                    iface,
                    &socketData->sockets,
                    i,
                    str);
                if (str[0] != 0)
                {
                    printf(str); fflush(stdout);
                }

                // Send state to new connection
                SocketInterface_SimulationStateMessage* state = new SocketInterface_SimulationStateMessage(
                    socketData->simulationState,
                    socketData->simulationState);
                SocketInterface_SendMessage(iface, state, socketData->sockets.connections.size() - 1);
                delete state;
            }
        }

        for (i = 0; i < socketData->sockets.connections.size(); i++)
        {
            bool avail = FALSE;

            if (socketData->sockets.activeConnections[i])
            {
                if (!EXTERNAL_SocketValid(socketData->sockets.connections[i]))
                {
                    HandleSocketError(
                        iface,
                        &socketData->sockets,
                        i,
                        EXTERNAL_SocketError,
                        str);
                }

                EXTERNAL_SocketDataAvailable(
                    socketData->sockets.connections[i],
                    &avail);

                if (avail)
                {
                    do
                    {
                        gotData = FALSE;
                        try
                        {
                            SocketInterface_ReceiveSerializedMessage(
                                socketData->sockets.connections[i],
                                &data);
                        }
                        catch (SocketInterface_Exception& e)
                        {
                            gotData = FALSE;
                            if (e.GetError() == SocketInterface_ErrorType_SocketError)
                            {
                                HandleSocketError(
                                    iface,
                                    &socketData->sockets,
                                    i,
                                    EXTERNAL_SocketError,
                                    str);
                            }
                            SocketInterface_HandleException(iface, &e, &data, i);
                        }

                        if (data.m_Size >= SOCKET_INTERFACE_HEADER_SIZE)
                        {
                            // Deserialize the message
                            gotData = TRUE;
                            SocketInterface_Message* message = NULL;
                            try
                            {
                                message = data.Deserialize();
                            }
                            catch (SocketInterface_Exception& e)
                            {
                                gotData = FALSE;
                                SocketInterface_HandleException(iface, &e, &data, i);
                            }

                            // Handle the message
                            if (gotData)
                            {
                                try
                                {
                                    SocketInterface_HandleReceiverMessage(iface, message, i);
                                }
                                catch (SocketInterface_Exception& e)
                                {
                                    gotData = FALSE;
                                    SocketInterface_HandleException(iface, &e, message, i);
                                    delete message;
                                }
                            }
                        }
                        else
                        {
                            gotData = FALSE;
                        }
                    } while (gotData);
                }
            }
        }
    }

    return NULL;
}

void* SocketInterface_SenderThread(void* voidIface)
{
    EXTERNAL_Interface* iface = (EXTERNAL_Interface*) voidIface;
    SocketInterface_InterfaceData* socketData = (SocketInterface_InterfaceData*) iface->data;
    //EXTERNAL_VarArray data;
    EXTERNAL_SocketErrorType err;
    SocketInterface_MessageList* node;
    int numOutgoing;
    char errString[MAX_STRING_LENGTH];
    char str[MAX_STRING_LENGTH];
    SocketInterface_SerializedMessage* serializedMessage;

    //EXTERNAL_VarArrayInit(&data);

    // Loop until end of program
    //while (socketData->simulationState != SocketInterface_StateType_Stopping && socketData->simulationState != SocketInterface_StateType_Resetting)
    while (socketData->simulationState != SocketInterface_StateType_Shutdown)
    {
        // Handle any messages
        pthread_mutex_lock(&socketData->sockets.senderMutex);
        if (socketData->outgoingMessages == NULL)
        {
            pthread_cond_wait(
                &socketData->sockets.senderNotEmpty,
                &socketData->sockets.senderMutex);
        }

        while (socketData->outgoingMessages == NULL && socketData->simulationState != SocketInterface_StateType_Shutdown)
        {
            pthread_cond_wait(
                &socketData->sockets.senderNotEmpty,
                &socketData->sockets.senderMutex);
        }

        if (socketData->simulationState == SocketInterface_StateType_Shutdown)
        {
            break;
        }

        // Remove from outgoing list
        node = (SocketInterface_MessageList*) socketData->outgoingMessages;
        assert(node != NULL);

        socketData->outgoingMessages = node->next;
        if (node->next == NULL)
        {
            socketData->lastOutgoingMessage = NULL;
        }
        socketData->numOutgoingMessages--;
        numOutgoing = socketData->numOutgoingMessages;
        pthread_mutex_unlock(&socketData->sockets.senderMutex);

        if (numOutgoing < SOCKET_INTERFACE_MAX_QUEUE_SIZE * 0.9)
        {
            pthread_cond_signal(&socketData->sockets.senderNotFull);
        }

        // If socket is valid
        if (EXTERNAL_SocketValid(socketData->sockets.connections[node->socketId]))
        {
            // Send the message
            // First Serialize the message
            if (DEBUG_MESSAGE_SENT)
            {
                std::string temp;
                node->message->Print(&temp);
                printf ("%s\n", temp.c_str());
            }
            serializedMessage = node->message->Serialize();
            if (serializedMessage->m_Data == NULL)
            {
                sprintf (errString, "Unknown message type\n");
                SocketInterface_PrintLog(iface, socketData->errorsLogFile, errString);
                delete node->message;
                delete node;
                return NULL;
            }

            err = EXTERNAL_SocketSend(
                socketData->sockets.connections[node->socketId],
                (char*) serializedMessage->m_Data,
                serializedMessage->m_Size);
            if (err != 0)
            {
                HandleSocketError(
                    iface,
                    &socketData->sockets,
                    node->socketId,
                    EXTERNAL_SocketError,
                    str);
            }
            delete serializedMessage;
        }

        // Now free memory
        delete node->message;
        delete node;
    }

    return NULL;
}

static void SocketInterface_SendCommEffectsResponseZeroDelay(
    EXTERNAL_Interface* iface,
    Node* node,
    NodeAddress receiverAddress,
    std::string receiver,
    SocketInterface_PacketData *packetData,
    int socketId)
{
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;

    if (receiverAddress == ANY_ADDRESS)
    {
        for (int i = 0; i < node->numberInterfaces; i++)
        {
            NodeAddress subnetAddr = 
                MAPPING_GetSubnetAddressForInterface(
                    node,
                    node->nodeId,
                    i);
            
            for (unsigned j = 0; j < data->subnetNodeMap[subnetAddr].size(); j++)
            {
                NodeId receiverId = data->subnetNodeMap[subnetAddr][j];

                // If it's not us and there is a platform mapped to this ID send a response
                if (receiverId != node->nodeId)
                {
                    EntityData* rec;

                    rec = EntityMappingLookup(&data->entityMapping, receiverId);

                    if (rec != NULL)
                    {
                        std::string entityId = rec->entityId;

                        SocketInterface_CommEffectsResponseMessage* response = new
                            SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                                            packetData->id2,
                                                            &packetData->originator,
                                                            &entityId,
                                                            SocketInterface_Status_Success,
                                                            (double) SocketInterface_Timestamp(iface) / SECOND,
                                                            0.0);
                        response->SetDescription(packetData->description);

                        if (EXTERNAL_SocketValid(data->sockets.connections[socketId]))
                        {
                            SocketInterface_SendMessage(
                                iface,
                                response,
                                packetData->socketId);

                            data->responses++;
                            data->successes++;
                        }
                        std::string str;
                        response->Print(&str);
                        str += "\n";

                         SocketInterface_PrintLog(
                            iface,
                            data->responsesLogFile,
                            str.c_str());
                        delete response;
                    }
                }
            }
        }
        // Get subnet address
    }
    else if (NetworkIpIsMulticastAddress(iface->partition->firstNode, receiverAddress))
    {
        // Loop over all platform mappings. 
        // If platform is a member of this multicast group then send a response
        EntityData* mapping; 
        BOOL isInGroup;
        std::string receiverId;

        // Loop over all platform mappings
        std::map<std::string, EntityData*>::iterator platformit; 
        for (platformit = data->entityMapping.entities.begin(); 
            platformit != data->entityMapping.entities.end(); 
            platformit++)
        {
            mapping = platformit->second; 
            isInGroup = FALSE;

            // Ignore if the mapping is us
            if (mapping->entityId == packetData->originator)
            {
                continue;
            }

            std::map<NodeId, Entity_NodeData>::iterator entityit; 
            for (entityit = mapping->nodes.begin(); 
                entityit != mapping->nodes.end(); 
                entityit++)
            {
                Entity_NodeData entity = entityit->second; 
            
                // Check if platform is in group
                std::vector<MulticastGroup>::iterator it;
                for (it = entity.multicastGroups.begin();
                    it != entity.multicastGroups.end();
                    it++)
                {
                    if (it->group == receiver)
                    {
                        receiverId = mapping->entityId;
                        isInGroup = TRUE;
                        break;
                    }
                }

                // If it's in the group then send response
                if (isInGroup)
                {
                    // Send warmup response
                    SocketInterface_CommEffectsResponseMessage* response = new
                        SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                                        packetData->id2,
                                                        &packetData->originator,
                                                        &receiverId,
                                                        SocketInterface_Status_Success,
                                                        (double) SocketInterface_Timestamp(iface) / SECOND,
                                                        0.0);
                    response->SetDescription(packetData->description);

                    // Source response, send response to source
                    if (data->sourceResponsibleMulticast)
                    {
                        if (EXTERNAL_SocketValid(data->sockets.connections[socketId]))
                        {
                            SocketInterface_SendMessage(
                                iface,
                                response,
                                packetData->socketId);

                            data->responses++;
                            data->successes++;
                        }
                    }
                    else
                    {
                        // Destination response, send response to all subscribers
                        for (unsigned j = 0; j < it->socketSubscribers.size(); j++)
                        {
                            if (EXTERNAL_SocketValid(data->sockets.connections[it->socketSubscribers[j]]))
                            {
                                SocketInterface_SendMessage(
                                    iface,
                                    response,
                                    it->socketSubscribers[j]);

                                data->responses++;
                                data->successes++;
                            }
                        }
                            
                    }
                    std::string str;
                    response->Print(&str);
                    str += "\n";

                    SocketInterface_PrintLog(
                        iface,
                        data->responsesLogFile,
                        str.c_str());

                   delete response;
                }
            }
        }
    }
    else
    {
        // Send warmup response
        SocketInterface_CommEffectsResponseMessage* response = new
            SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                            packetData->id2,
                                            &packetData->originator,
                                            &packetData->receiver,
                                            SocketInterface_Status_Success,
                                            (double) SocketInterface_Timestamp(iface) / SECOND,
                                            0.0);
        response->SetDescription(packetData->description);

        if (EXTERNAL_SocketValid(data->sockets.connections[socketId]))
        {
            SocketInterface_SendMessage(
                iface,
                response,
                packetData->socketId);

            data->responses++;
            data->successes++;
        }
        std::string str;
        response->Print(&str);
        str += "\n";

        SocketInterface_PrintLog(
            iface,
            data->responsesLogFile,
            str.c_str());
        delete response;
    }
}

static void SocketInterface_SendCommEffectsRequest(
    EXTERNAL_Interface* iface,
    Node* node,
    SocketInterface_CommEffectsRequestMessage *requestMessage,
    NodeAddress receiverAddress,
    int socketId,
    clocktype delay)
{ 
    SocketInterface_PacketData *packetData;
    SocketInterface_InterfaceData *data;
    NodeAddress receiverNodeId;
    char *messageData;
    char *tmpMessageData;
    int messageSize;
    int tmpPktSize;

    // The actual packet will contain the message hash id as well as the
    // failure timeout window.
    // TODO: remove the failure timeout window.  It doesn't make sense here.
    UInt32 actualPacketSize = sizeof(SocketInterface_PacketHeader);

    char err[MAX_STRING_LENGTH];
    char receiverString[MAX_STRING_LENGTH];

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Add space for network packet indicator
    if (data->useStatisticalModel) 
    {
        actualPacketSize += sizeof(BOOL);
    }

#ifdef DEBUG
    {
    char        senderBuf [MAX_STRING_LENGTH];
    char        receiverBuf [MAX_STRING_LENGTH];
    strcpy (senderBuf, requestMessage->m_SenderId.c_str ());
    strcpy (receiverBuf, requestMessage->m_ReceiverId.c_str ());
    printf("Message #%d request from %s to %s p = %d t = %f\n",
           (int)requestMessage->m_Id1,
           senderBuf,
           receiverBuf,
           requestMessage->m_Protocol,
           requestMessage->m_SendTime);
    }
#endif

    NodeAddress originatorAddress = MappingGetSingleNodeAddress(iface->partition,
                                        &data->entityMapping,
                                        requestMessage->m_SenderId);
    if (originatorAddress == 0)
    {
        sprintf(err,
            "SocketInterface_SendCommEffectsRequest Invalid Sender node (no node address): %s",
            requestMessage->m_SenderId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
    }

    // Allocate message data.  This is temporary and will be freed by the
    // call to EXTERNAL_CreateMessageAppLayer.  The message data will contain
    // a pointer to the SocketInterface_PacketData structure which has information about
    // the comm effects request message and a pointer to the
    // EXTERNAL_TreeNode structure that keeps track of all packets in the
    // simulation.  The memory referenced by messageData will be placed
    // inside a message, and then freed.
    if (requestMessage->m_Size < actualPacketSize)
    {
        messageSize = actualPacketSize;
    }
    else
    {
        messageSize = requestMessage->m_Size;
    }

    messageData = (char*) MEM_malloc(sizeof(SocketInterface_PacketHeader));
    memset(messageData, 0, sizeof(SocketInterface_PacketHeader));
    
    // Construct packet data
    packetData = new SocketInterface_PacketData;
    packetData->id1 = requestMessage->m_Id1;
    packetData->id2 = requestMessage->m_Id2;
    packetData->size = requestMessage->m_Size;
    packetData->waveform = requestMessage->m_Waveform;
    packetData->route = requestMessage->m_Route;
    packetData->waveformSpecified = requestMessage->m_WaveformSpecified;
    packetData->routeSpecified = requestMessage->m_RouteSpecified;
    packetData->protocol = requestMessage->m_Protocol;
    packetData->description = requestMessage->m_Description.c_str();
    packetData->socketId = socketId;
    packetData->multicast = receiverAddress == ANY_ADDRESS
            || NetworkIpIsMulticastAddress(iface->partition->firstNode, receiverAddress);

    // Initialize packetData's receivers effects hashmap
    if ((data->useStatisticalModel == TRUE) && (packetData->multicast)) 
    {
        packetData->effects = new SocketInterface_MulticastModeledReceiversCommEffects();
    }
    else 
    {
        packetData->effects = NULL;
    }

    // Determine the receiver for this request
    std::string receiver;
    receiverNodeId = MAPPING_GetNodeIdFromInterfaceAddress(
        node,
        receiverAddress);
    if (receiverNodeId == INVALID_MAPPING)
    {
        // If it's an invalid mapping check if it is a multicast address.
        if (packetData->multicast)
        {
            IO_ConvertIpAddressToString(
                receiverAddress,
                receiverString);
            receiver = (std::string)receiverString;
        }
        else
        {
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidReceiverId, "Invalid receiver address in SocketInterface_SendCommEffectsRequest");
        }
    }
    else
    {
        // Get the receiver entity id
        receiver = MappingGetEntityId(
            &data->entityMapping,
            receiverNodeId);

        if (strcmp(receiver.c_str(), "") == 0)
        {
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidReceiverId, "Receiver in SocketInterface_SendCommEffectsRequest is not a mapped entity");
        }
    }

    // Allocate receiver
    packetData->receiver = receiver;

    // Note, these times are going to be wrong if the
    // request came in asking for a delay.
    packetData->originator = requestMessage->m_SenderId;
    packetData->beginProcessingTime =
        EXTERNAL_QuerySimulationTime(iface);
    packetData->beginProcessingRealTime =
        EXTERNAL_QueryRealTime(iface);
    packetData->beginProcessingCPUTime =
        EXTERNAL_QueryCPUTime(iface);
    packetData->boundary = data->lastBoundary + 1;

    EXTERNAL_TreeNode* record;
    if (packetData->multicast)
    {
        record = EXTERNAL_TreeAllocateNode(&data->multicastTree);
    }
    else
    {
        record = EXTERNAL_TreeAllocateNode(&data->requestTree);
    }

    packetData->treeNode = record;
    record->data = (void*) packetData;
    // NOTE: record time set later depending on UDP/TCP

    // Hash the request
    packetData->hashId = data->requestHash.HashRequest(packetData);

    // Store the header inside the message
    SocketInterface_PacketHeader* header = (SocketInterface_PacketHeader*) messageData;
    header->hashId = packetData->hashId;
    header->failureTimeout = data->udpFailureTimeout;
    header->id1 = requestMessage->m_Id1;
    header->id2 = requestMessage->m_Id2;

    // Update tos if specified
    TosType tos = 0;
    if (requestMessage->m_PrecedenceSpecified || requestMessage->m_DscpSpecified
        || requestMessage->m_TosSpecified)
    {
        if (requestMessage->m_PrecedenceSpecified)
        {
            tos = requestMessage->m_Precedence << 5;
        }
        else if (requestMessage->m_DscpSpecified)
        {
            tos = requestMessage->m_Dscp << 2;
        }
        else if (requestMessage->m_TosSpecified)
        {
            tos = requestMessage->m_Tos;
        }
    }

    // Determine how to handle the request
    if (data->simulationState == SocketInterface_StateType_Standby)
    {
        // Ignore during pause and standby, invalid message.  Send
        // error message instead
        MEM_free(messageData);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSimulationState, "Cannot accept CommEffectsRequest during Standby state");
    }
    else if (data->simulationState == SocketInterface_StateType_Paused && data->pauseSendZeroDelay)
    {
        // Send an immediate comm response during warmup and
        // pause state if SOCKET-INTERFACE-PAUSE-REPLY-ZERO-DELAY is YES.  After sending the
        // response free the message data and exit this function, no further
        // processing is needed.
        SocketInterface_SendCommEffectsResponseZeroDelay(
            iface,
            node,
            receiverAddress,
            receiver,
            packetData,
            socketId);
        MEM_free(messageData);
        return;
    }
    else if (requestMessage->m_Protocol == SocketInterface_ProtocolType_ProtocolTCP)
    {
        SocketInterface_UnicastModelingParameters params;
        BOOL statResult;

        // Determine failure timeout depending on whether the message has a
        // timestamp
        if (requestMessage->m_SendTimeSpecified)
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                   + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                packetData->failureTimeout = (double) (requestMessage->m_FailureTimeout);
            }
            else
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                    + data->tcpFailureTimeout;
                packetData->failureTimeout = ((double) data->tcpFailureTimeout)/SECOND;
            }
        }
        else
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = getSimTime(node) + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                }
                packetData->failureTimeout = (double) (requestMessage->m_FailureTimeout);
            }
            else
            {
                record->time = getSimTime(node) + data->tcpFailureTimeout;
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + data->tcpFailureTimeout;
                }
                packetData->failureTimeout = ((double) data->tcpFailureTimeout)/SECOND;
            }
        }

        // TCP cannot be multicast
        if (packetData->multicast)
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidProtocol,
                "Multicast messages may not be sent using TCP");
        }
        
        // If running in distributed qualnet mode, send responses and update stats
        // based on statistical model. Also, update message accordingly
        if (data->useStatisticalModel) 
        {
            // Add the network packet indicator to the message
            BOOL isNetworkPacket = FALSE;
            memcpy(
                   messageData + sizeof(Int32) + sizeof(clocktype),
                   &isNetworkPacket,
                   sizeof(BOOL));
            
            params.protocol = SocketInterface_ProtocolType_ProtocolTCP;
            params.senderNodeAddress = originatorAddress;
            params.receiverNodeAddress = receiverAddress;
            params.sendTime = EXTERNAL_QuerySimulationTime(iface);
            statResult = SocketInterface_SendModeledResponses(
                             iface,
                             packetData,
                             (void *)&params);
            tmpMessageData = messageData;
            tmpPktSize = actualPacketSize;
            
            actualPacketSize += (sizeof(BOOL) + 
                                 sizeof(BOOL) + 
                                 sizeof(SocketInterface_UnicastModelingParameters));
            
            if (requestMessage->m_Size < actualPacketSize)
            {
                messageSize = actualPacketSize;
            }
            else
            {
                messageSize = requestMessage->m_Size;
            }
            
            messageData = (char *) MEM_malloc(messageSize);
            memset(messageData, 0, messageSize);
            memcpy(
                messageData,
                tmpMessageData,
                tmpPktSize);
            
            memcpy(
                messageData + tmpPktSize,
                &statResult,
                sizeof(BOOL));
            memcpy(
                messageData + tmpPktSize + sizeof(BOOL),
                &(packetData->multicast),
                sizeof(BOOL));
            memcpy(
                messageData + tmpPktSize + sizeof(BOOL) + sizeof(BOOL),
                &params,
                sizeof(SocketInterface_UnicastModelingParameters));
            
            MEM_free(tmpMessageData);
        }
        
        if (requestMessage->m_TtlSpecified)
        {
            // Send TCP message
            EXTERNAL_SendVirtualDataAppLayerTCP(
                iface,
                originatorAddress,
                receiverAddress,
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay,
                requestMessage->m_TTL);
        }
        else
        {
            // Send TCP message
            EXTERNAL_SendVirtualDataAppLayerTCP(
                iface,
                originatorAddress,
                receiverAddress,
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay);
        }
    }
    else if (requestMessage->m_Protocol == SocketInterface_ProtocolType_ProtocolUDP)
    {
        SocketInterface_UnicastModelingParameters uParams;
        SocketInterface_MulticastModelingParameters mParams;
        size_t paramSize;
        BOOL statResult;

        // Determine failure timeout depending on whether the message has a
        // timestamp
        if (requestMessage->m_SendTimeSpecified)
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                   + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                packetData->failureTimeout = (double) (requestMessage->m_FailureTimeout);
            }
            else
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                    + data->udpFailureTimeout;
                packetData->failureTimeout = ((double) data->udpFailureTimeout)/SECOND;
            }
        }
        else
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = getSimTime(node) + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                }
                packetData->failureTimeout = (double) (requestMessage->m_FailureTimeout);
            }
            else
            {
                record->time = getSimTime(node) + data->udpFailureTimeout;
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + data->udpFailureTimeout;
                }
                packetData->failureTimeout = ((double) data->udpFailureTimeout)/SECOND;
            }
        }

        // If running in distributed qualnet mode, send responses and update stats
        // based on statistical model. Also, update message accordingly
        if (data->useStatisticalModel) 
        {
            // Add the network packet indicator to the message
            BOOL isNetworkPacket = FALSE;
            memcpy(
                   messageData + sizeof(Int32) + sizeof(clocktype),
                   &isNetworkPacket,
                   sizeof(BOOL));
            
            if (packetData->multicast) 
            {
                mParams.protocol = SocketInterface_ProtocolType_ProtocolUDP;
                mParams.senderNodeAddress = originatorAddress;
                mParams.receiverNodeAddress = receiverAddress;
                mParams.sendTime = EXTERNAL_QuerySimulationTime(iface);
                statResult = SocketInterface_SendModeledResponses(
                                  iface,
                                  packetData,
                                  (void *)&mParams);
                paramSize = sizeof(SocketInterface_MulticastModelingParameters);
            }
            else 
            {
                uParams.protocol = SocketInterface_ProtocolType_ProtocolUDP;
                uParams.senderNodeAddress = originatorAddress;
                uParams.receiverNodeAddress = receiverAddress;
                uParams.sendTime = EXTERNAL_QuerySimulationTime(iface);
                statResult = SocketInterface_SendModeledResponses(
                                  iface,
                                  packetData,
                                  (void *)&uParams);
                paramSize = sizeof(SocketInterface_UnicastModelingParameters);
            }
            tmpMessageData = messageData;
            tmpPktSize = actualPacketSize;
            
            actualPacketSize += (sizeof(BOOL) + 
                                 sizeof(BOOL) + 
                                 paramSize);
            
            if (requestMessage->m_Size < actualPacketSize)
            {
                messageSize = actualPacketSize;
            }
            else
            {
                messageSize = requestMessage->m_Size;
            }
            
            messageData = (char *) MEM_malloc(messageSize);
            memset(messageData, 0, messageSize);
                   
            memcpy(
                messageData,
                tmpMessageData,
                tmpPktSize);            
            
            memcpy(
                messageData + tmpPktSize,
                &statResult,
                sizeof(BOOL));
            memcpy(
                messageData + tmpPktSize + sizeof(BOOL),
                &(packetData->multicast),
                sizeof(BOOL));
            if (packetData->multicast) 
            {
                memcpy(
                    messageData + tmpPktSize + sizeof(BOOL) + sizeof(BOOL),
                    &mParams,
                    paramSize);
            }
            else 
            {
                memcpy(
                    messageData + tmpPktSize + sizeof(BOOL) + sizeof(BOOL),
                    &uParams,
                    paramSize);                    
            }
            MEM_free(tmpMessageData);
        }

        if (requestMessage->m_TtlSpecified)
        {
            // Send UDP message
            EXTERNAL_SendVirtualDataAppLayerUDP(
                iface,
                originatorAddress,
                receiverAddress,
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay,
                APP_FORWARD,
                TRACE_FORWARD,
                tos,
                requestMessage->m_TTL);
        }
        else
        {
            // Send UDP message
            EXTERNAL_SendVirtualDataAppLayerUDP(
                iface,
                originatorAddress,
                receiverAddress,
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay,
                APP_FORWARD,
                TRACE_FORWARD,
                tos);
        }
    }
    else if (requestMessage->m_Protocol == SocketInterface_ProtocolType_ProtocolNetwork)
    {
        // Add the network packet indicator to the message
        if (data->useStatisticalModel) 
        {
            BOOL isNetworkPacket = TRUE;
            memcpy(
                   messageData + sizeof(Int32) + sizeof(clocktype),
                   &isNetworkPacket,
                   sizeof(BOOL));
        }

        if (requestMessage->m_TtlSpecified)
        {
            EXTERNAL_SendVirtualDataNetworkLayer(
                iface,
                originatorAddress,
                originatorAddress,
                receiverAddress,
                tos,
                IPPROTO_EXTERNAL,
                requestMessage->m_TTL, // ttl
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay);
        }
        else
        {
            EXTERNAL_SendVirtualDataNetworkLayer(
                iface,
                originatorAddress,
                originatorAddress,
                receiverAddress,
                tos,
                IPPROTO_EXTERNAL,
                IPDEFTTL, // default ttl
                messageData,
                sizeof(SocketInterface_PacketHeader),
                messageSize - sizeof(SocketInterface_PacketHeader),
                EXTERNAL_QuerySimulationTime(iface) + delay);
        }



        // Determine failure timeout depending on whether the message has a
        // timestamp
        if (requestMessage->m_SendTimeSpecified)
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                   + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
            }
            else
            {
                record->time = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                    + data->udpFailureTimeout;
            }
        }
        else
        {
            if (requestMessage->m_FailureTimeoutSpecified)
            {
                record->time = getSimTime(node) + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + (clocktype) (requestMessage->m_FailureTimeout * SECOND);
                }
            }
            else
            {
                record->time = getSimTime(node) + data->udpFailureTimeout;
                if (data->deterministicTiming)
                {
                    record->time = iface->horizon + data->udpFailureTimeout;
                }
            }
        }
    }
    else
    {
        MEM_free(messageData);

        sprintf(err, "Unknown protocol \"%d\"", requestMessage->m_Protocol);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, err);
    }

    // Insert the comm request into the request tree
    if (packetData->multicast)
    {
        EXTERNAL_TreeInsert(&data->multicastTree, record);
    }
    else
    {
        EXTERNAL_TreeInsert(&data->requestTree, record);
    }
    MEM_free(messageData);
}

static void SocketInterface_HandleStopSimulation(
    EXTERNAL_Interface* iface,
    SocketInterface_StopSimulationMessage* stopMessage)
{
    char errString[MAX_STRING_LENGTH];
    SocketInterface_InterfaceData* data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Check for timeStamp
    if (stopMessage->m_StopSpecified)
    {
        // timestamp is specified. Now to make sure that
        // the time is not past.
        if (SocketInterface_TimestampToQualNet(iface, stopMessage->m_StopTime) <
            EXTERNAL_QuerySimulationTime(iface))
        {
            if (data->timeManagement == SocketInterface_TimeManagementType_RealTime)
            {
                EXTERNAL_SetSimulationEndTime(
                    iface,
                    EXTERNAL_QuerySimulationTime(iface));
            }
            else
            {
                // In non-real-time this is an error -- we should never receive
                // a message in the past.
                sprintf(errString,
                    "Invalid Stop time: %fs",
                    stopMessage->m_StopTime);
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidStopTime, errString);
            }
        }
        else
        {
            // set the new maxSimClock;
            EXTERNAL_SetSimulationEndTime(
                iface,
                SocketInterface_TimestampToQualNet(iface, stopMessage->m_StopTime));
        }
    }
    else
    {
        // No timeStamp, that is, stop immediately.
        EXTERNAL_SetSimulationEndTime(
            iface,
            EXTERNAL_QuerySimulationTime(iface));
    }
}

static void SocketInterface_HandleResetSimulation(
    EXTERNAL_Interface* iface,
    SocketInterface_ResetSimulationMessage* reset)
{
    // Create the reponse message.
    SocketInterface_InterfaceData *data;
    char errString[MAX_STRING_LENGTH];

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Check for timeStamp
    if (reset->m_ResetSpecified)
    {
        // timestamp is specified. Now to make sure that
        // the time is not past.
        if (SocketInterface_TimestampToQualNet(iface, reset->m_ResetTime) <
            EXTERNAL_QuerySimulationTime(iface))
        {
            if (data->timeManagement == SocketInterface_TimeManagementType_RealTime)
            {
                EXTERNAL_SetSimulationEndTime(
                    iface,
                    SocketInterface_TimestampToQualNet(iface, reset->m_ResetTime));
            }
            else
            {
                // In non-real-time this is an error -- we should never receive
                // a message in the past.
                sprintf(
                    errString,
                    "Invalid Stop time: %fs",
                    reset->m_ResetTime);
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidStopTime, errString);
            }
        }
        else
        {
            // set the new maxSimClock;
            EXTERNAL_SetSimulationEndTime(
                iface,
                SocketInterface_TimestampToQualNet(iface, reset->m_ResetTime));
        }
    }
    else
    {
        // No timeStamp, that is, stop immediately.
        EXTERNAL_SetSimulationEndTime(
            iface,
            EXTERNAL_QuerySimulationTime(iface));
    }

    data->resetting = TRUE;
}

static void SocketInterface_HandleDynamicResponse(
    EXTERNAL_Interface* iface,
    SocketInterface_DynamicCommandMessage* dynamic,
    int socketId,
    std::string* result)
{
    // Create the reponse message.
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    SocketInterface_DynamicResponseMessage* response;
    response = new SocketInterface_DynamicResponseMessage(dynamic->m_Type,
                                              &dynamic->m_Path,
                                              &dynamic->m_Args,
                                              result);

    // Check for the socket data
    if (EXTERNAL_SocketValid(data->sockets.connections[socketId]))
    {
        SocketInterface_SendMessage(
            iface,
            response,
            socketId);

        data->responses++;
    }

    std::string str;
    response->Print(&str);
    str += "\n";

    SocketInterface_PrintLog(
        iface,
        data->responsesLogFile,
        str.c_str());

    // delete the message that was created.
    delete response;
}

// Robust Interface client uses Dynamic Commands to read and write parameters
// controlling the robust protocol.
// The localDynamicRead and localDynamicWrite handle these commands even if
// the scenario does not enable the dynamic hierarchy
static bool localDynamicRead(EXTERNAL_Interface* iface,
                        const std::string &path,
                        std::string &result)
{
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;
    std::stringstream rs;
    if (path == "/CESSocketInterface/ServerProtocolVersion")
    {
        rs << data->serverProtocolVersion;
        rs >> result;
        return true;
    }
    else if (path == "/CESSocketInterface/ClientProtocolVersion")
    {
        rs << data->clientProtocolVersion;
        rs >> result;
        return true;
    }
    else
        return false;
}

static bool localDynamicWrite(EXTERNAL_Interface* iface,
                        const std::string &path,
                        const std::string &args)
{
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;
    std::istringstream is(args);
    if (path == "/CESSocketInterface/ClientProtocolVersion")
    {
        int v;
        is >> v;
        data->clientProtocolVersion = v;
        return true;
    }
    else
        return false;
}

static void SocketInterface_HandleDynamicCommand(
    EXTERNAL_Interface* iface,
    SocketInterface_DynamicCommandMessage* dynamic,
    int socketId)
{
    char errString[MAX_STRING_LENGTH];
    std::string result;
    D_Hierarchy* h = &iface->partition->dynamicHierarchy;

    std::string encodedPath = "";
    try
    {
        encodedPath = h->EncodePath(dynamic->m_Path);
    }
    catch (D_Exception)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidDynamicCommand,
            "Invalid Path for the Dynamic Command");
    }
    // Check the type for the command
    switch(dynamic->m_Type)
    {
        case SocketInterface_OperationType_Read:
        {
            try
            {
                // Robust Interface
                if (!localDynamicRead(iface, encodedPath, result))
                {
                    // Use the encoded path
                    h->ReadAsString(
                        encodedPath.c_str(),
                        result);
                }
                // Need to send a response back to the socket
                SocketInterface_HandleDynamicResponse(
                    iface,
                    dynamic,
                    socketId,
                    &result);
            }
            catch (D_Exception& d)
            {
                // We have an exception.
                SocketInterface_HandleDynamicCommandException(iface,
                    &d,
                    (SocketInterface_Message*)dynamic,
                    socketId,
                    SocketInterface_ErrorType_InvalidDynamicCommand);
            }
            break;

        }

        case SocketInterface_OperationType_Write:
        {
            try
            {
                // Robust Interface
                if (!localDynamicWrite(iface, encodedPath, dynamic->m_Args))
                {
                    // Use the encoded path
                    h->WriteAsString(
                        encodedPath.c_str(),
                        dynamic->m_Args.c_str());
                }
            }
            catch (D_Exception& d)
            {
                SocketInterface_HandleDynamicCommandException(iface,
                    &d,
                    (SocketInterface_Message*)dynamic,
                    socketId,
                    SocketInterface_ErrorType_InvalidDynamicCommand);
            }
            break;
        }

        case SocketInterface_OperationType_Execute:
        {
            try
            {
                // Use the encoded path
                h->ExecuteAsString(
                    encodedPath.c_str(),
                    dynamic->m_Args.c_str(),
                    result);
                // Send a response for the command above
                SocketInterface_HandleDynamicResponse(
                    iface,
                    dynamic,
                    socketId,
                    &result);
            }
            catch (D_Exception& d)
            {
                SocketInterface_HandleDynamicCommandException(iface,
                    &d,
                    (SocketInterface_Message*)dynamic,
                    socketId,
                    SocketInterface_ErrorType_InvalidDynamicCommand);
            }
            break;
        }

        default:
             sprintf(
                errString,
                "Unknown operation type \"%d\"",
                dynamic->m_Type);
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidOperationType, errString);
            break;

    }
}

// /**
// FUNCTION   :: SocketInterface_CheckWaveformValidity
// PURPOSE    :: Checks if the waveform and route inputs 
//               are valid.
// PARAMETERS :: 
//             iface : Pointer to external interface.
//          waveform : waveform type
// waveformSpecified : Bool specifying whether waveform is present
//    routeSpecified : Bool specifying whether route is present
// RETURN     ::  Void : NULL.
// **/
void SocketInterface_CheckWaveformValidity(
    EXTERNAL_Interface* iface,
    SocketInterface_WaveformType waveform,
    BOOL waveformSpecified,
    BOOL routeSpecified)
{
    char errString[MAX_STRING_LENGTH];
    assert(iface != NULL);
    
    if (routeSpecified && !waveformSpecified)
    {
        sprintf(errString,
            "Invalid Message, Route cannot be secified without Waveform");
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, errString);
    }
    if (waveformSpecified)
    {
        if (waveform == SocketInterface_WaveformType_SRW && routeSpecified)
        {
            sprintf(errString,
                "Invalid Message, Route cannot be secified with SRW");
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, errString);
        }
        if (waveform == SocketInterface_WaveformType_WNW && !routeSpecified)
        {
            sprintf(errString,
                "Invalid Message, WNW waveform requires a route");
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, errString);
        }   
    }
}

static void SocketInterface_HandleCommEffectsRequest(
    EXTERNAL_Interface *iface,
    SocketInterface_CommEffectsRequestMessage *requestMessage,
    int socketId)
{
    char errString[MAX_STRING_LENGTH];
    Node *sendNode;
    NodeAddress originatorAddress;
    clocktype delay;
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    SocketInterface_CheckWaveformValidity(
        iface,
        requestMessage->m_Waveform,
        requestMessage->m_WaveformSpecified,
        requestMessage->m_RouteSpecified);

#ifdef DEBUG
    printf("request t = %f sim = %f\n",
           requestMessage->m_SendTime,
           (double) EXTERNAL_QuerySimulationTime(iface) / SECOND);
#endif

    // Determine sendNode, allowed to be node on remote partition    
    sendNode = MappingGetSingleNodePointer(
        iface->partition,
        &data->entityMapping,
        requestMessage->m_SenderId);
    if (sendNode == NULL)
    {
        sprintf(errString,
            "SocketInterface_HandleCommEffectsRequest Invalid Sender node: %s",
            requestMessage->m_SenderId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, errString);
    }
    else
    {
        // check if the sender is mapped
        std::string senderEntityId = "";
        senderEntityId = MappingGetEntityId(
            &data->entityMapping,
            (NodeAddress)sendNode->nodeId);

        if (strcmp(senderEntityId.c_str(), "") == 0)
        {
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, "Sender in SocketInterface_SendCommEffectsRequest is not a mapped entity");
        }
    }
    

    originatorAddress = MappingGetSingleNodeAddress(iface->partition,
                                        &data->entityMapping,
                                        requestMessage->m_SenderId);
    if (originatorAddress == 0)
    {
        sprintf(errString,
            "SocketInterface_HandleCommEffectsRequest Invalid Sender node (no node address): %s",
            requestMessage->m_SenderId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, errString);
    }    

     // Check if send time is specified
    if (requestMessage->m_SendTimeSpecified)
    {
        // Check if the request is occuring in the past (ie, its sendTime is
        // before the simulation time)
        if (SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime) <
            EXTERNAL_QuerySimulationTime(iface))
        {
            if (data->timeManagement == SocketInterface_TimeManagementType_RealTime)
            {
                // In real-time this is OK, just send the message as soon as
                // possible
                delay = 0;
                if (data->deterministicTiming)
                {
                    delay = (iface->horizon -
                        EXTERNAL_QuerySimulationTime (iface));
                }
            }
            else
            {
                // In non-real-time this is an error -- we should never receive
                // a message in the past.
                sprintf(errString,
                    "Invalid Send time: %fs",
                    requestMessage->m_SendTime);
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSendTime, errString);
            }
        }
        else
        {
            delay = SocketInterface_TimestampToQualNet(iface, requestMessage->m_SendTime)
                - EXTERNAL_QuerySimulationTime(iface);
        }
    }
    else
    {
        // We have no send time. Send immediately
        delay = 0;
        if (data->deterministicTiming)
        {
            delay = (iface->horizon - EXTERNAL_QuerySimulationTime(iface));
        }
    }

    // TODO JDL: I don't know why SocketInterface_HandleCommEffectsReceiver is a separate function.  It also has a weird name.
    // What do you think about combining the two functions?
    SocketInterface_HandleCommEffectsReceiver(
        iface,
        sendNode,
        requestMessage,
        socketId,
        delay);
}

static void HandleCartesianCoordinateValidity(
     EXTERNAL_Interface* iface,
     double coord1,
     double coord2,
     double coord3)
{
    assert(iface != NULL);
    char errString[MAX_STRING_LENGTH];
    struct cartesian_coordinates origin;
    struct cartesian_coordinates dimensions;

    origin = iface->partition->terrainData->getOrigin().cartesian;
    dimensions = iface->partition->terrainData->getDimensions().cartesian;

    if (coord1 < origin.x)
    {
        sprintf(
            errString,
            "X out of range: %f (minimum is %f)",
            coord1, origin.x);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord1 > dimensions.x)
    {
        sprintf(
            errString,
            "X out of range: %f (maximum is %f)",
            coord1, dimensions.x);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }

    if (coord2 < origin.y)
    {
        sprintf(
            errString,
            "Y out of range: %f (minimum is %f)",
            coord2, origin.y);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord2 > dimensions.y)
    {
        sprintf(
            errString,
            "Y out of range: %f (maximum is %f)",
            coord2, dimensions.y);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }

    if (coord3 < origin.z)
    {
        sprintf(
            errString,
            "Z out of range: %f (minimum is %f)",
            coord3, origin.z);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord3 > dimensions.z)
    {
        sprintf(
            errString,
            "Z out of range: %f (maximum is %f)",
            coord3, dimensions.z);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
}

static void HandleLatLonAltCoordinateValidity(
     EXTERNAL_Interface* iface,
     double coord1,
     double coord2,
     double coord3)
{
    assert(iface != NULL);
    char errString[MAX_STRING_LENGTH];
    struct common_coordinates origin;
    struct common_coordinates dimensions;

    origin = iface->partition->terrainData->getOrigin().common;
    dimensions = iface->partition->terrainData->getDimensions().common;

    if (coord1 < origin.c1)
    {
        sprintf(
            errString,
            "Latitude out of range: %f (minimum is %f)",
            coord1, origin.c1);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord1 > origin.c1 + dimensions.c1)
    {
        sprintf(
            errString,
            "Latitude out of range: %f (maximum is %f)",
            coord1, origin.c1 + dimensions.c1);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }

    if (coord2 < origin.c2)
    {
        sprintf(
            errString,
            "Longitude out of range: %f (minimum is %f)",
            coord2, origin.c2);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord2 > origin.c2 + dimensions.c2)
    {
        sprintf(
            errString,
            "Longitude out of range: %f (maximum is %f)",
            coord2, origin.c2 + dimensions.c2);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }

    if (coord3 < origin.c3)
    {
        sprintf(
            errString,
            "Altitude out of range: %f (minimum is %f)",
            coord3, origin.c3);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }
    else if (coord3 > origin.c3 + dimensions.c3)
    {
        sprintf(
            errString,
            "Altitude out of range: %f (maximum is %f)",
            coord3, origin.c3 + dimensions.c3);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidCoordinates, errString);
    }

}

void SocketInterface_HandleMulticastChange(
    EXTERNAL_Interface* iface,
    Node* node,
    clocktype eventTime,
    std::string entityId,
    int socketId,
    std::vector<std::string>* joinGroups,
    std::vector<std::string>* leaveGroups)
{
    SocketInterface_InterfaceData *data;
    unsigned i;
    std::string join;
    std::string leave;

    data = (SocketInterface_InterfaceData*) iface->data;

    // Add each multicast group in sequential mode.  This is handled in
    // SocketInterface_PARALLEL_HandleCreatePlatformNotice when running in parallel.
    // Note that MappingAddMulticastGroup
    // checks to make sure the platform is not already in the group.
    NodeAddress group;
    int hostBits;
    BOOL isNodeId;

    if (joinGroups != NULL)
    {
        for (i = 0; i < joinGroups->size(); i++)
        {
            IO_ParseNodeIdHostOrNetworkAddress(
                (*joinGroups)[i].c_str(),
                &group,
                &hostBits,
                &isNodeId);
            if (NetworkIpIsMulticastAddress(node, group))
            {
                // If it's a multicast address then join the group
                join += (*joinGroups)[i] + " ";
            }
            else if (group != ANY_ADDRESS)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidGroup,
                    "Invalid multicast group");
            }

            // Update the number of memebers in the group.
            if (data->listOfMulticastGroups.size() <= 0)
            {
                // No members in the list.
                data->listOfMulticastGroups.insert(make_pair((*joinGroups)[i], 1));
                //data->listOfMulticastGroups[(*joinGroups)[i]] = 1;
            }
            else
            {
                // the list is not empty.
                int tempCount;
                std::map<std::string, int>::iterator iter;
                iter = data->listOfMulticastGroups.find((*joinGroups)[i]);
                if (iter != data->listOfMulticastGroups.end())
                {
                    // the element exists in the list.
                    tempCount = data->listOfMulticastGroups[(*joinGroups)[i]];
                    tempCount++;
                    data->listOfMulticastGroups[(*joinGroups)[i]] = tempCount;
                }
                else
                {
                    // this element is not in the list. Add it
                    data->listOfMulticastGroups.insert(make_pair((*joinGroups)[i], 1));
                }
            }
            // At this point it is either multicast or broadcast.  Add it
            // to the entity mapping
            MappingAddMulticastGroup(
                &data->entityMapping,
                (std::string)entityId,
                &(*joinGroups)[i],
                socketId);
        }
    }

    if (leaveGroups != NULL)
    {
        // Leave each multicast group.  Note that Socket_MappingRemoveMulticastGroup
        // checks to make sure the platform is in the group.
        for (i = 0; i < leaveGroups->size(); i++)
        {
            IO_ParseNodeIdHostOrNetworkAddress(
                (*leaveGroups)[i].c_str(),
                &group,
                &hostBits,
                &isNodeId);
            if (NetworkIpIsMulticastAddress(node, group))
            {
                leave += (*leaveGroups)[i] + " ";
            }
            else
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidGroup,
                    "Invalid multicast group");
            }

            // update the member count.
            int tempCount;
            tempCount = data->listOfMulticastGroups[(*leaveGroups) [i]];
            if (tempCount <= 0)
            {
                data->listOfMulticastGroups[(*leaveGroups) [i]] = 0;
            }
            else
            {
                tempCount--;
                data->listOfMulticastGroups[(*leaveGroups) [i]] = tempCount;
            }

            MappingRemoveMulticastGroup(
                &data->entityMapping,
                (std::string)entityId,
                &(*leaveGroups)[i]);
        }
    }

    // Send message to node that will add it to multicast groups
    Message* multicastMsg;

    multicastMsg = MESSAGE_Alloc(
        iface->partition->firstNode,
        EXTERNAL_LAYER,
        EXTERNAL_SOCKET,
        MSG_EXTERNAL_SOCKET_INTERFACE_MulticastGroup);

    // Fill message info with join/leave groups as follows
    // 4 bytes: size of join string (N)
    // 4 bytes: size of leave string (M)
    // N + 1 bytes: join string + trailing 0
    // M + 1 bytes: leave string + trailing 0

    UInt32 n = 0;
    UInt32 m = 0;

    if (joinGroups != NULL)
    {
        n = join.size();
    }
    if (leaveGroups != NULL)
    {
        m = leave.size();
    }

    char* info = MESSAGE_InfoAlloc(
        iface->partition->firstNode,
        multicastMsg,
        sizeof(UInt32) * 2 + n + m + 2);

    memcpy(info, &n, sizeof(UInt32));
    info += sizeof(UInt32);
    memcpy(info, &m, sizeof(UInt32));
    info += sizeof(UInt32);
    if (n > 0)
    {
        memcpy(info, join.c_str(), n);
        info += n;
        *info++ = 0;
    }
    if (m > 0)
    {
        memcpy(info, leave.c_str(), m);
        info += m;
        *info++ = 0;
    }

    clocktype delay;
    if (getSimTime(node) > eventTime)
    {
        delay = 0;
    }
    else
    {
        delay = eventTime - getSimTime(node);
    }
    EXTERNAL_MESSAGE_SendAnyNode(
        iface,
        node,
        multicastMsg,
        delay,
        EXTERNAL_SCHEDULE_SAFE);
}

//#define TEST_EXTERNAL_REMOTESEND
#ifdef TEST_EXTERNAL_REMOTESEND
class SocketInterface_ExternalPingInfo
{
public:
    int msgNumber;
    int sourcePartition;
};
#define MSG_EXTERNAL_RemoteTest 40000
#endif

static void SocketInterface_HandleCreatePlatform(
    EXTERNAL_Interface *iface,
    SocketInterface_CreatePlatformMessage *message)
{
    SocketInterface_InterfaceData *data;
    EntityData* rec;  
    std::list<Entity_NodeData> entities;
    char errString[MAX_STRING_LENGTH];
    double c1;
    double c2;
    double c3;
    double v1 = 0;                  //velocity v1, v2, v3
    double v2 = 0;
    double v3 = 0;
    clocktype eventTime;

#ifdef TEST_EXTERNAL_REMOTESEND
    // bleair - simple unit test to send an external message to partition 1
    assert (iface->partition->isRunningInParallel());
    static int g_pingCount = 1000;

    // allocate a simple message
    Message * externalPingMessage;

    externalPingMessage = MESSAGE_Alloc(
        iface->partition->firstNode,
        EXTERNAL_LAYER,    // special layer
        EXTERNAL_SOCKET,         // EXTERNAL_SOCKET,           // protocol
        MSG_EXTERNAL_RemoteTest);

    // Create the info to communicate in our 'Ping" message.
    SocketInterface_ExternalPingInfo * pingInfo;
    pingInfo = (SocketInterface_ExternalPingInfo *)MESSAGE_InfoAlloc(
        iface->partition->firstNode, externalPingMessage,
        sizeof(SocketInterface_ExternalPingInfo));
    pingInfo->sourcePartition = iface->partition->partitionId;
    pingInfo->msgNumber = g_pingCount++;

    // Send it to our external interafce that is on partition 1
    EXTERNAL_MESSAGE_RemoteSend (iface, 1,
        externalPingMessage, 0, EXTERNAL_SCHEDULE_SAFE);
    //    externalPingMessage, 0, EXTERNAL_SCHEDULE_SAFE);
    printf ("DEBUG - at %lld sending a remote message %d\n",
    iface->partition->theCurrentTime,
    g_pingCount);
#endif

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    std::string entityId = message->m_EntityId; 

#ifdef DEBUG
    printf("SocketInterface_HandleCreatePlatform for entityId %s\n", 
        entityId.c_str()); 
    fflush(stdout); 
#endif

    if (strcmp(message->m_EntityId.c_str(), "") == 0)
    {
        // Invalid paramter, should have an entity id.
        // Throw an exception
        sprintf(
            errString,
            "Invalid Entity Id Parameter \"%s\"",
            entityId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
    }

    rec = EntityMappingLookup(&data->entityMapping, entityId); 

    if (data->entityMapping.dynamic)
    {
        // At his point we have a node, as well as a valid entity Id.
        // Now to check if the mapping for the entity exists.
        // If the mapping already exists, then we have a problem.
        // For the dynamic case, assume one-to-one mapping
        if (rec != NULL)
        {
            sprintf(
                errString,
                "Mapping for the EntityId already exists \"%s\"",
                entityId.c_str());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }

        // Add the new entity Id to QualNet.
        // Create a new mapping
        rec = DynamicMappingAdd( 
            &data->entityMapping,
            message->m_EntityId);

        if (rec == NULL)
        {
            sprintf(
                errString,
                "Mapping Failure, Free Nodes unavailable \"%s\"",
                entityId.c_str());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }
    }
    else // static case
    {
        // Record should already exist (inactive)
        if (rec == NULL)
        {
            // Mapping does not exists. We have an error.
            sprintf(
                errString,
                "Mapping does not exit for platform \"%s\"",
                message->m_EntityId.c_str());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }
    }        

    if (rec->isActive)
    {
        // Node is already created, duplicate creation error.
        sprintf(
            errString,
            "Duplicate node creation for platform \"%s\"",
            message->m_EntityId.c_str());
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
    }

    // Set the generic data for the entity. The data that is common
    // to the entity and not specific to the mapping modes.

    rec->isActive = TRUE;

    // Check for invalid damage state
    if (message->m_State > SocketInterface_PlatformStateType_Damaged)
    {
        sprintf(errString, "Invalid State \"%d\"", message->m_State);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidState, errString);
    }

    // Fill in the entity data information
    rec->damageState = message->m_State;
#ifdef DEBUG
    printf("(orig)damage state for entity %s create is [%d]\n", 
        rec->entityId.c_str(), message->m_State); 
    fflush(stdout); 
#endif

    // Set generic data.  This is only used for printing the graph log and
    // nowhere else.
    rec->commTypes = 0;

    // Check for the coordinate system. Convert the system
    // if necessary, also convert velocity if speficied.
    if (data->coordinateSystem == SocketInterface_CoordinateSystemType_GccCartesian)
    {
        ConvertGCCCartesianToGCC(
            message->m_Position.m_C1,
            message->m_Position.m_C2,
            message->m_Position.m_C3,
            &c1,
            &c2,
            &c3);

        if (message->m_VelocitySpecified)
        {
            ConvertGCCCartesianVelocityToGCC(
                message->m_Position.m_C1,
                message->m_Position.m_C2,
                message->m_Position.m_C2,
                message->m_Velocity.m_C1,
                message->m_Velocity.m_C2,
                message->m_Velocity.m_C3,
                &v1,
                &v2,
                &v3);
        }
    }
    else
    {
        c1 = message->m_Position.m_C1;
        c2 = message->m_Position.m_C2;
        c3 = message->m_Position.m_C3;

        if (message->m_VelocitySpecified)
        {
            v1 = message->m_Velocity.m_C1;
            v2 = message->m_Velocity.m_C2;
            v3 = message->m_Velocity.m_C3;
        }
    }

    // Check if the message contains valid coordinates. If the
    // coordinates are not in range throw exception. The function below
    // checks for coordinates.
    if (data->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
    {
        HandleLatLonAltCoordinateValidity(iface, c1, c2, c3);
    }
    else if (data->coordinateSystem == SocketInterface_CoordinateSystemType_Cartesian)
    {
        HandleCartesianCoordinateValidity(iface, c1, c2, c3);
    }

    // Check for and assign the platform type.
    // Note: this is repeated for each node in the for loop
    if (message->m_TypeSpecified)
    {
        // Check for the type, if type = SocketInterface_PlatformType_Ground then set
        // MOBILITY-GROUND-NODE to true, else if the type is
        // SocketInterface_PlatformType_Air then set MOBILITY-GROUND-NODE to false.
        if (message->m_Type == SocketInterface_PlatformType_Ground)
        {
            rec->type = SocketInterface_PlatformType_Ground;
        }
        else if (message->m_Type == SocketInterface_PlatformType_Air)
        {
            rec->type = SocketInterface_PlatformType_Air;
        }
        else
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Platform type not supported yet");
        }
    }
    else
    {
        // Default to ground
        rec->type = SocketInterface_PlatformType_Ground;
    }

    if (data->deterministicTiming)
    {
        // Parallel or not, this schedules the creation at the next
        // external-interface federation time. This adds delay, but
        // the delay will be consistent between par and seq. For example
        // a qualnet - interface step of 500 msecs will delay this create
        // to the next time advance.
        eventTime = EXTERNAL_QueryExternalTime(iface) + 1;
    }
    else
    {
        // Request the creation for right now. The ChangeNodePosition
        // event will be delayed by safeTime when running in parallel.
        eventTime = EXTERNAL_QuerySimulationTime(iface) + 1;
    }

    data->creates++; 

    // Now update all node-specific data
    std::map<NodeId, Entity_NodeData>::iterator entityit; 
    for (entityit = rec->nodes.begin(); entityit != rec->nodes.end(); entityit++)
    {
        Node* node = (entityit->second.node); 

        // Check for and assign the platform type.
        // Note: this is done for the entire platform above the for loop
        if (message->m_TypeSpecified)
        {
            // Check for the type, if type = SocketInterface_PlatformType_Ground then set
            // MOBILITY-GROUND-NODE to true, else if the type is
            // SocketInterface_PlatformType_Air then set MOBILITY-GROUND-NODE to false.
            if (message->m_Type == SocketInterface_PlatformType_Ground)
            {
                node->mobilityData->groundNode = TRUE;
            }
            else if (message->m_Type == SocketInterface_PlatformType_Air)
            {
                node->mobilityData->groundNode = FALSE;
            }
            else
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Platform type not supported yet");
            }
        }
        else
        {
            // Default to ground
            node->mobilityData->groundNode = TRUE;
        }

#ifdef PARALLEL //Parallel
        {
            // Now Notify any other partitions so that they may also update
            // their mapping table.
            // Allocate a message for partition communication to signal
            // all the other platforms that we are creating an QualNet platform.
            // Info of this message is a copy of the QualNet creation message
            // PLUS the nodeId that has just been assigned for this entity Id.

            Message* createNotifyMsg = MESSAGE_Alloc(
                iface->partition->firstNode,
                PARTITION_COMMUNICATION,    // special layer
                data->createPlatformCommunicator,           // protocol
                PARTITION_MSG_CREATE_PLATFORM_NOTICE);
            MESSAGE_InfoAlloc(
                iface->partition->firstNode,
                createNotifyMsg,
                sizeof(SocketInterface_CreatePlatformNotificationInfo));
            SocketInterface_CreatePlatformNotificationInfo* notificationMessage =
                (SocketInterface_CreatePlatformNotificationInfo*) MESSAGE_ReturnInfo(createNotifyMsg);
            notificationMessage->nodeId = node->nodeId;
            strcpy(notificationMessage->entityId, rec->entityId.c_str());
            notificationMessage->damageState = rec->damageState; //message->m_State;
            notificationMessage->typeSpecified = SocketInterface_PlatformType_Unspecified;
            if (message->m_TypeSpecified)
            {
                notificationMessage->typeSpecified = message->m_Type;
            }

            // This triggers the receiving partitions to receive callbacks to
            // SocketInterface_PARALLEL_HandleCreatePlatformNotice ()
            PARTITION_COMMUNICATION_SendToAllPartitions (iface->partition,
                createNotifyMsg, 0);
        }
#endif //Parallel        

        // Check for damage state condition.
        // If the state is damaged then do not activate the node.
        if (rec->damageState == SocketInterface_PlatformStateType_Undamaged)
        {
            EXTERNAL_ActivateNode(iface, node, EXTERNAL_SCHEDULE_SAFE);
        }

        // Add the system Group Stats.
        InitInterfaceSysStats(
            iface,
            node,
            rec,
            SocketInterface_TimestampToQualNet(iface, message->m_CreateTime));

        SocketInterface_HandleMulticastChange(
            iface,
            node,
            eventTime,
            message->m_EntityId.c_str(),
            message->GetSocketId(),
            &message->m_MulticastGroups,
            NULL);

        // Now check for mobility condition. If the MOBILITY
        // parameter is set in QualNet, then external interface has
        // problem. The MOBILITY parameter must be set to NONE,
        // for the node to be mobilized using the EXTERNAL INTERFACE.
        if (node->mobilityData->mobilityType != NO_MOBILITY)
        {
            sprintf(
                errString,
                "Mobility is not NONE in the QualNet config file for node %d",
                node->nodeId);
            throw SocketInterface_Exception(SocketInterface_ErrorType_ConfigurationError, errString);
        }

#ifdef DEBUG
        printf(
            "moving (creating) node %d to %f %f %f\n",
            node->nodeId,
            message->m_Position.m_C1,
            message->m_Position.m_C2,
            message->m_Position.m_C3);
#endif        

        // update the node position based on the CreatePlatform message
        // This will relocate the node regardless of what partition it is
        // on.  Update orientation and velocity if specified.
        if (message->m_VelocitySpecified == FALSE)
        {
            EXTERNAL_ChangeNodePositionAtSimTime(
                iface,
                node,
                c1,
                c2,
                c3,
                eventTime);
        }
        else
        { 
            EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime(
                    iface,
                    node,
                    eventTime,
                    c1,
                    c2,
                    c3,
                    0,                      //azimuth
                    0,                      //elevation
                    v1,
                    v2,
                    v3);
        }
    } //end for each node in the platform
}

static void SocketInterface_HandleUpdatePlatform(
    EXTERNAL_Interface *iface,
    SocketInterface_UpdatePlatformMessage *message)
{
    SocketInterface_InterfaceData *data;    
    Coordinates coords;
    Vector3 velocity;
    char s[MAX_STRING_LENGTH];
    char errString[MAX_STRING_LENGTH];
    clocktype eventTime;
    Node* node = NULL; 
    std::string entityId = message->m_EntityId; 
    bool isIpAddress = false; // in case entityId in message is actually IP Address

#ifdef DEBUG
    printf("SocketInterface_HandleUpdatePlatform for entityId %s\n", entityId.c_str()); 
#endif

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);        

    if (!IsEntityIdString(entityId))
    {
        // check if the entityId is an ip address
        NodeAddress nadd; //ip address
        BOOL isNodeId; 
        IO_ParseNodeIdOrHostAddress(entityId.c_str(), &nadd, &isNodeId); 
        if (!isNodeId && nadd != INVALID_MAPPING)
        {
            // lookup the nodeId that belongs to this Ip Address
            NodeId nodeId = MAPPING_GetNodeIdFromInterfaceAddress(
                iface->partition->firstNode, nadd); 
#ifdef DEBUG
            std::cout << "(SocketInterface_HandleUpdatePlatform) MAPPING_GetNodeIdFromInterfaceAddress returned " 
                << nadd << " for entityId " << entityId << std::endl;
#endif
            isIpAddress = true; 

            // lookup Node* from nodeId
            node = EntityNodeLookup(&data->entityMapping, nodeId);         
        } // else this is an entityId
    }

    data->updates++;

    if (data->deterministicTiming)
    {
        // Parallel or not, this schedules the update at the next
        // external-interface federation time. This adds delay, but
        // the delay will be consistent between par and seq. For example
        // a qualnet - interface step of 500 msecs will delay this update msg
        // until the next time advance.
        eventTime = iface->horizon + 1;
    }
    else
    {
        // Request the be scheduled update for right now. FYI,
        // The _ChangeNodePosition will delay by safeTime when running in parallel.
        eventTime = EXTERNAL_QuerySimulationTime(iface) + 1;
    }

    // Check if there is a later event time
    if (message->m_UpdateTimeSpecified)
    {
        clocktype messageTime = SocketInterface_TimestampToQualNet(
            iface,
            message->m_UpdateTime);
        if (messageTime > eventTime)
        {
            eventTime = messageTime;
        }
    } 

    if (isIpAddress)
    {
        if (node != NULL)
        {
            SocketInterface_HandleMulticastChange(
                iface,
                node,
                eventTime,
                entityId.c_str(),
                message->GetSocketId(),
                &message->m_JoinMulticastGroups,
                &message->m_LeaveMulticastGroups);
        }
    }
    else // not by ip address --> update everything
    {
        EntityData* mapping = EntityMappingLookup(&data->entityMapping, 
            (std::string)entityId); 
        if (mapping == NULL)
        {
            sprintf(
                errString,
                "Mapping Failure for entity \"%s\"\n",
                entityId.c_str());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, s);
        }              

        if (message->m_SpeedSpecified)
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Speed not supported yet");
        }                        

        std::map<NodeId, Entity_NodeData>::iterator entityit; 
        for (entityit = mapping->nodes.begin(); 
            entityit != mapping->nodes.end(); 
            entityit++)
        {

            node = (entityit->second.node); 

            if (node == NULL)
            {

                sprintf(s, "Invalid Node in mapping: %s", entityId.c_str());
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, s);
            }     
            // Update node in platform if state if specified
            if (message->m_StateSpecified)
            {
                // Check for invalid damage state
                if (message->m_State > SocketInterface_PlatformStateType_Damaged)
                {
                    sprintf(s, "Invalid State \"%d\"", message->m_State);
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidState, s);
                }

                if (mapping->damageState == SocketInterface_PlatformStateType_Undamaged
                    && message->m_State == SocketInterface_PlatformStateType_Damaged)
                {
                    EXTERNAL_DeactivateNode(iface, node, EXTERNAL_SCHEDULE_SAFE,
                        eventTime);
                }
                else if (mapping->damageState == SocketInterface_PlatformStateType_Damaged
                        && message->m_State == SocketInterface_PlatformStateType_Undamaged)
                {
                    EXTERNAL_ActivateNode(iface, node, EXTERNAL_SCHEDULE_SAFE,
                        eventTime);
                }
            }

            // Update position if specified in the message
            if (message->m_PositionSpecified)
            {
                // Check if Mobility is set to NONE
                if (node->mobilityData->mobilityType == NO_MOBILITY)
                {
                    // Convert coordinates if necessary
                    if (data->coordinateSystem == SocketInterface_CoordinateSystemType_GccCartesian)
                    {
                        ConvertGCCCartesianToGCC(
                            message->m_Position.m_C1,
                            message->m_Position.m_C2,
                            message->m_Position.m_C3,
                            &coords.common.c1,
                            &coords.common.c2,
                            &coords.common.c3);
                    }
                    else
                    {
                        coords.common.c1 = message->m_Position.m_C1;
                        coords.common.c2 = message->m_Position.m_C2;
                        coords.common.c3 = message->m_Position.m_C3;
                    }

#ifdef DEBUG
                    printf("%lld p%d is moving %d to %f %f %f at event time %d\n",
                        iface->partition->theCurrentTime,
                        iface->partition->partitionId,
                        node->nodeId,
                        coords.common.c1,
                        coords.common.c2,
                        coords.common.c3, 
                        eventTime);        
#endif
                    // Check for coordinate range validity
                    if (data->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
                    {
                        HandleLatLonAltCoordinateValidity(iface,
                            coords.common.c1,
                            coords.common.c2,
                            coords.common.c3);
                    }

                    // if velocity is specified, position update 
                    // will be handled in velocity update call
                    if (message->m_VelocitySpecified == FALSE)
                    {
                        // Update the node's position
                        EXTERNAL_ChangeNodePositionAtSimTime(
                            iface,
                            node,
                            coords.common.c1,
                            coords.common.c2,
                            coords.common.c3, eventTime);
                    }
                }
                else
                {
                    sprintf(
                        errString,
                        "QualNet mobility is not set to NONE in QualNet configuration file.");
                    throw SocketInterface_Exception(SocketInterface_ErrorType_ConfigurationError, errString);
                }
            }

            if (message->m_VelocitySpecified)
            {
                // Check if Mobility is set to NONE
                if (node->mobilityData->mobilityType == NO_MOBILITY)
                {
                    // Convert coordinates if necessary
                    if (data->coordinateSystem == SocketInterface_CoordinateSystemType_GccCartesian)
                    {
                        //if position is not specified, get current position
                        //coordinates for conversion
                        if (message->m_PositionSpecified == FALSE)
                        {
                            MOBILITY_ReturnCoordinates(node,&coords);
                        }

                        ConvertGCCCartesianVelocityToGCC(
                                coords.common.c1,
                                coords.common.c2,
                                coords.common.c3,
                                message->m_Velocity.m_C1,
                                message->m_Velocity.m_C2,
                                message->m_Velocity.m_C3,
                                (double*) &velocity.x,
                                (double*) &velocity.y,
                                (double*) &velocity.z);
                    }
                    else
                    {
                        velocity.x = (double) message->m_Velocity.m_C1;
                        velocity.y = (double) message->m_Velocity.m_C2;
                        velocity.z = (double) message->m_Velocity.m_C3;
                    }

#ifdef DEBUG
                    printf("%lld p%d is moving %d with velocity %fx %fy %fz\n",
                       iface->partition->theCurrentTime,
                       iface->partition->partitionId,
                       node->nodeId,
                       (double) velocity.x,
                       (double) velocity.y,
                       (double) velocity.z);
#endif 

                    if (message->m_PositionSpecified)
                    {
                        EXTERNAL_ChangeNodePositionOrientationAndVelocityAtTime(
                                iface,
                                node,
                                eventTime,
                                coords.common.c1,
                                coords.common.c2,
                                coords.common.c3,
                                0,                      //azimuth
                                0,                      //elevation
                                (double) velocity.x,
                                (double) velocity.y,
                                (double) velocity.z);
                    }
                    else
                    {
                        EXTERNAL_ChangeNodeVelocityAtTime(
                                iface,
                                node,
                                eventTime,
                                (double) velocity.x,
                                (double) velocity.y,
                                (double) velocity.z);
                        
                    }
                }
                else
                {
                    sprintf(
                            errString,
                            "QualNet mobility is not set to NONE in QualNet configuration file.");
                    throw SocketInterface_Exception(SocketInterface_ErrorType_ConfigurationError, errString);
                } 
            }

            SocketInterface_HandleMulticastChange(
                iface,
                node,
                eventTime,
                message->m_EntityId.c_str(),
                message->GetSocketId(),
                &message->m_JoinMulticastGroups,
                &message->m_LeaveMulticastGroups);
        } // end for each node in the entity platform    

        // Finally update the platform state if specified
        if (message->m_StateSpecified)
        {
            // Store state
            mapping->damageState = message->m_State;
        }
    }
}

static void SocketInterface_HandleRequestFailure(
    EXTERNAL_Interface *iface,
    EXTERNAL_TreeNode *record)
{
    SocketInterface_PacketData *packetData;
    SocketInterface_CommEffectsResponseMessage* response;
    SocketInterface_InterfaceData *data;
    SocketInterface_Status state;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    packetData = (SocketInterface_PacketData*) record->data;

    if (data->alwaysSuccess)
    {
        state = SocketInterface_Status_Success;
    }
    else
    {
        state = SocketInterface_Status_Failure;
    }

    // Send a failure Response
    SocketInterface_TimeType latency;
    std::string receiver;
    std::string originator;

    receiver = packetData->receiver;
    originator = packetData->originator;

    latency = ((double) EXTERNAL_QuerySimulationTime(iface) - packetData->beginProcessingTime) / SECOND;
    response = new SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                                   packetData->id2,
                                                   &originator,
                                                   &receiver,
                                                   state,
                                                   (double) SocketInterface_Timestamp(iface) / SECOND,
                                                   latency);
    response->SetDescription(packetData->description);

    if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
    {
        SocketInterface_SendMessage(iface, response, packetData->socketId);

        // Update statistics
        data->responses++;
        data->failures++;
    }

    std::string str;
    response->Print(&str);
    str += "\n";

    SocketInterface_PrintLog(
        iface,
        data->responsesLogFile,
        str.c_str());

    // Set data to NULL so we know this was freed
    record->data = NULL;

    // Remove from request hash
    SocketInterface_PacketData* packetData2;
    packetData2 = data->requestHash.PeekRequest(packetData->hashId);
    assert(packetData2 == packetData);
    data->requestHash.RemoveHash(packetData->hashId);

    // Free memory
    delete packetData;

    // delete the response message
    delete response;
}

// non-static functions

void SocketInterface_CreateLogFiles(EXTERNAL_Interface* iface, char* dir)
{
    SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;
    char fileName[MAX_STRING_LENGTH];

    // Create driver log file
    sprintf(fileName, "%s/driver.log", dir);
    data->driverLogFile = fopen(fileName, "w");
    ERROR_Assert(data->driverLogFile != NULL,
                 "Could not create Socket Interface driver log file");

    // Create responses log file
    sprintf(fileName, "%s/responses.log", dir);
    data->responsesLogFile = fopen(fileName, "w");
    ERROR_Assert(data->responsesLogFile != NULL,
                 "Could not create Socket Interface responses log file");

    // Print header for the responses file
    SocketInterface_PrintLog(
        iface,
        data->responsesLogFile,
        "Socket Interface Responses Logfile\n",
        FALSE);

    // Create stats log file
    sprintf(fileName, "%s/stats.log", dir);
    data->statsLogFile = fopen(fileName, "w");
    ERROR_Assert(data->statsLogFile != NULL,
                 "Could not create Socket Interface stats log file");

    // Print header for the stats file
    SocketInterface_PrintLog(iface, data->statsLogFile, "Socket Interface Stats Logfile\n", FALSE);

    // Create graph log file
    sprintf(fileName, "%s/graph.log", dir);
    data->graphLogFile = fopen(fileName, "w");
    ERROR_Assert(data->graphLogFile != NULL,
                 "Could not create QualNet graph log file");

    // Print header for the graph file
    SocketInterface_PrintLog(iface, data->graphLogFile, "Socket Interface Graph Log\n", FALSE);

    // Create errors log file
    sprintf(fileName, "%s/errors.log", dir);
    data->errorsLogFile = fopen(fileName, "w");
    ERROR_Assert(data->errorsLogFile != NULL,
                 "Could not create Socket Interface errors log file");

    // Print header for the graph file
    SocketInterface_PrintLog(
        iface,
        data->errorsLogFile,
        "Socket Interface Errors Logfile\n",
        FALSE);

    if (data->logPropagation)
    {
        // Create propagation log file
        sprintf(fileName, "%s/propagation.log", dir);
        data->propagationLogFile = fopen(fileName, "w");
        ERROR_Assert(data->propagationLogFile != NULL,
                 "Could not create Socket Interface propagation log file");

        // Print header for the graph file
        SocketInterface_PrintLog(
            iface,
            data->propagationLogFile,
            "Socket Interface Propagation Logfile\n",
            FALSE);
    }
}

void SocketInterface_PrintLog(EXTERNAL_Interface *iface, FILE *f, const char *s, BOOL printTimestamp)
{
    SocketInterface_InterfaceData *data;

    data = (SocketInterface_InterfaceData*) iface->data;

    // If file is NULL, logging is disabled
    if (f == NULL)
    {
        return;
    }

    if (!printTimestamp)
    {
        fprintf(
            f, "%s",
            s);
    }
    else if (data->logPrintRealTime)
    {
        fprintf(
            f, "[%10f %10f] %s",
            SocketInterface_Timestamp(iface) / 1000000000.0,
            iface->partition->wallClock->getTrueRealTimeAsDouble(),
            s);
    }
    else
    {
        fprintf(
            f, "[%10f] %s",
            SocketInterface_Timestamp(iface) / 1000000000.0,
            s);
    }

    if (data->automaticFlush)
    {
        fflush(f);
    }
}

struct NodeList
{
    NodeAddress nodeId;
    int subnetId;
    std::vector<NodeAddress> nodes;
    std::vector<NodeAddress> raps;
};

bool RegionComp(const NodeList& a, const NodeList& b)
{
    return a.subnetId < b.subnetId;
}

static void SocketInterface_HandlePrintGraphLog(
    EXTERNAL_Interface* iface,
    EntityMapping* entityMapping,
    FILE* f)
{    
    SocketInterface_InterfaceData* socketData;
    Node *node;
    Coordinates coords;
    char s[200*MAX_STRING_LENGTH];
    std::string multiAddrString = "";
    unsigned i;

    std::vector<NodeList> regionMembership;
    std::vector<NodeAddress> inOtherRegion;
    std::map<int, std::vector<NodeAddress> > gateways;

    std::vector<NodeList>::iterator it;
    std::vector<NodeList>::iterator it2;
    std::vector<NodeAddress>::iterator it3;

    assert(iface != NULL);
    socketData = (SocketInterface_InterfaceData*) iface->data;
    assert(socketData != NULL);

    // If file is NULL, logging is disabled
    if (f == NULL)
    {
        return;
    }



    sprintf(s, "=============================================================\n");
    SocketInterface_PrintLog(iface, f, s);

    EntityData* mapping; 
    for (node = iface->partition->firstNode;
         node != NULL;
         node = node->nextNodeData)
    {
        // Skip this node if it doesn't map to an entity, or if the entity
        // is not yet created
        mapping = EntityMappingLookup(
            entityMapping,
            node->nodeId);

        if (mapping == NULL)
        {
            continue;
        }
        if (!mapping->isActive)
        {
            continue; 
        }

        std::map<NodeId, Entity_NodeData>::iterator entityit; 
        for (entityit = mapping->nodes.begin(); entityit != mapping->nodes.end(); entityit++)
        {
            Node* entityNode = (entityit->second.node);      
            if (entityNode != node)
            {
                continue;
            }

             sprintf(
                 s,
                 "EntityID: %-15s    NodeID: %d\n",
                 mapping->entityId.c_str(),
                 entityNode->nodeId );
            SocketInterface_PrintLog(iface, f, s);

            MOBILITY_ReturnCoordinates(node, &coords);
            if (socketData->coordinateSystem == SocketInterface_CoordinateSystemType_Cartesian)
            {
                sprintf(
                    s,
                    "    Location: X = %f, Y = %f, Z = %f\n",
                    coords.common.c1,
                    coords.common.c2,
                    coords.common.c3);
            }
            else
            {
                sprintf(
                    s,
                    "    Location: LAT = %f, LON = %f, ALT = %f\n",
                    coords.common.c1,
                    coords.common.c2,
                    coords.common.c3);
            }
            SocketInterface_PrintLog(iface, f, s);

            sprintf(s, "    Type: ");
            SocketInterface_PrintLog(iface, f, s);
            if (mapping->type == SocketInterface_PlatformType_Ground)
            {
                fputs("Ground\n", f);
            }
            else if (mapping->type == SocketInterface_PlatformType_Air)
            {
                fputs("Air\n", f);
            }
            else
            {
                fputs("Unknown\n", f);
            }
        }

// Check that routing protocol is part of JNE
#ifdef JNE_LIB
        NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
        RoutingCesMalsrData* malsrData = ip->routingCesMalsrData;

        EntityData* rec; 

        BOOL routingProtocolUsed = TRUE;
        int nodeID;
        int subnetId;
        char addressString[MAX_STRING_LENGTH];
        Address address;
        
        NodeList* list;
        
        assert (ip != NULL);
        if (malsrData == NULL)
        {
            routingProtocolUsed = FALSE;
        }

        if (routingProtocolUsed)
        {
            // Now to Print the RAP
            for (int i = 0; i < node->numberInterfaces; i++)
            {
                if (malsrData->activeOnInterfaceList[i])
                {
                    if (NetworkCesRegionIsRap(node, i))
                    {
                        //Print the interface Address.
                        NetworkGetInterfaceInfo(node, i , &address);
                        IO_ConvertIpAddressToString(&address, addressString);
                        sprintf(s, "    Interface: %s ", addressString);
                        SocketInterface_PrintLog(iface, f, s);

                        //Print the subnet id
                        subnetId = NetworkCesSubnetGetId (node, i);
                        sprintf(s, "    Subnet Id: %d [RAP]", subnetId);
                        fputs(s, f);

                        sprintf(s, "    Members: ");
                        fputs (s, f);

                        RoutingCesMalsrInterface* malsr = &malsrData->iface[i];

                        // Create region list
                        list = new NodeList;
                        list->nodeId = node->nodeId;
                        list->subnetId = subnetId;
                        list->nodes.push_back(node->nodeId);
                        list->raps.push_back(node->nodeId);
                        vector<NodeAddress>::iterator iter;
#ifdef MEMBER_UPDATE
                        RoutingCesMalsrUpdateRegionMemberList(node, i);
#endif
                        if (malsr->regionMemberList)
                        {
                            if (malsr->numRegionMembers > 0)
                            {
                                iter = malsr->regionMemberList->begin();
                                while (iter != malsr->regionMemberList->end())
                                {
                                    nodeID = *iter;
                                    rec = EntityMappingLookup(entityMapping, nodeID);
                                    if (rec == NULL)
                                    {
                                        continue;
                                    }
                                    sprintf (s, "%s ", rec->entityId.c_str());
                                    fputs (s, f);
                                    list->nodes.push_back(nodeID);
                                    ++iter;
                                }
                            }
                        }

                        // Add region list
                        regionMembership.push_back(*list);
                        delete list;

                        fputs ("\n", f);
                    }
                    else
                    {
                        //Print the interface Address.
                        NetworkGetInterfaceInfo(node, i , &address);
                        IO_ConvertIpAddressToString(&address, addressString);
                        sprintf(s, "    Interface: %s ", addressString);
                        SocketInterface_PrintLog(iface, f, s);

                        //Print the subnet id
                        subnetId = NetworkCesSubnetGetId (node, i);
                        sprintf(s, "    Subnet Id: %d ", subnetId);
                        fputs(s, f);
                        fputs ("\n", f);
                    }
                }
            }
        }
#endif
        //fputs ("\n", data->graphLogFile);
        sprintf(s, "    Is Gateway: ");
        SocketInterface_PrintLog(iface, f, s);

        BOOL isGateway = FALSE;

#ifdef JNE_LIB
        if (routingProtocolUsed)
        {
            if (NetworkCesSubnetIsTrueGateway(node))
            {
                for (int i = 0; i < node->numberInterfaces; i++)
                {
                    subnetId = NetworkCesSubnetGetId (node, i);
                    if (!isGateway)
                    {
                        fputs(" Yes, subnets", f);
                        isGateway = TRUE;
                    }
                    sprintf(s, " %d", subnetId);
                    fputs (s, f);

                    gateways[subnetId].push_back(node->nodeId);
                }
            }
        }
#endif
        if (!isGateway)
        {
            fputs("No\n", f);
        }
        else
        {
            fputs("\n", f);
        }
        // Print multicast groups
        // TODO: this code has a buffer overflow
        strcpy(s, "    Multicast Groups: ");
        GetMulticastGroups(node, multiAddrString);
        strcat(s, multiAddrString.c_str());
        strcat(s, "\n");
        SocketInterface_PrintLog(iface, f, s);
    }

    if (regionMembership.size() > 0)
    {
        // Sort region
        for (it = regionMembership.begin(); it != regionMembership.end(); it++)
        {
            sort(it->nodes.begin(), it->nodes.end());
        }

        // Check for identical regions
        for (it = regionMembership.begin(); it != regionMembership.end(); it++)
        {
            for (it2 = it + 1; it2 != regionMembership.end(); it2++)
            {
                // Compare different regions that are the same size
                if (it->nodes.size() == it2->nodes.size())
                {
                    BOOL same = TRUE;

                    for (i = 0; i < it->nodes.size(); i++)
                    {
                        if (it->nodes[i] != it2->nodes[i])
                        {
                            same = FALSE;
                            break;
                        }
                    }

                    // If they are the same, get rid of other region
                    if (same)
                    {
                        inOtherRegion.push_back(it2->nodeId);
                        it->raps.push_back(it2->nodeId);
                    }
                }
            }
        }

        // Sort regions by subnet
        sort(regionMembership.begin(), regionMembership.end(), RegionComp);

//f = stdout;
        SocketInterface_PrintLog(iface, f, "Region Summary.  Platforms in parenthesis are RAPs.\n");
        int lastSubnet = -1;
        for (it = regionMembership.begin(); it != regionMembership.end(); it++)
        {
            NodeAddress regionId = it->nodeId;

            if (find(inOtherRegion.begin(), inOtherRegion.end(), regionId) != inOtherRegion.end())
            {
                continue;
            }

            if (it->subnetId != lastSubnet)
            {
                sprintf(s, "    Subnet %d", it->subnetId);
                SocketInterface_PrintLog(iface, f, s);
                lastSubnet = it->subnetId;

                if (gateways[it->subnetId].size() > 0)
                {
                    fprintf(f, ", gateways");

                    for (it3 = gateways[it->subnetId].begin(); it3 != gateways[it->subnetId].end(); it3++)
                    {
                        mapping = EntityMappingLookup(
                            entityMapping,
                            *it3);
                        if (mapping == NULL)
                        {
                            continue;
                        }
                        if (!mapping->isActive)
                        {
                            continue; 
                        }

                        fprintf(f, " %s", mapping->entityId.c_str());
                    }
                }
                fprintf(f, "\n");
            }

            // Print RAPs for this region.  The node list nodeId will always
            // be equal o the first member of the rap list
            mapping = EntityMappingLookup(
                entityMapping,
                it->nodeId);
            if (mapping == NULL)
            {
                continue;
            }
            if (!mapping->isActive)
            {
                continue; 
            }

            sprintf(s, "        (%s", mapping->entityId.c_str());
            SocketInterface_PrintLog(iface, f, s);

            // Print rest of RAPs for this region
            for (i = 1; i < it->raps.size(); i++)
            {
                mapping = EntityMappingLookup(
                    entityMapping,
                    it->raps[i]);
                if (mapping == NULL)
                {
                    continue;
                }   
                if (!mapping->isActive)
                {
                    continue; 
                }           
                fprintf(f, " %s", mapping->entityId.c_str());
            }
            fprintf(f, ")");

            for (i = 0; i < it->nodes.size(); i++)
            {
                NodeAddress nodeId = it->nodes[i];

                // Only print if it's not a rap for this region
                if (find(it->raps.begin(), it->raps.end(), nodeId) == it->raps.end())
                {
                    mapping = EntityMappingLookup(
                        entityMapping,
                        nodeId);

                    if (mapping == NULL)
                    {
                        continue;
                    }
                    if (!mapping->isActive)
                    {
                        continue; 
                    }
                    fprintf(f, " %s", mapping->entityId.c_str());
                }
            }
            fprintf(f, "\n");
        }
    }
}

void SocketInterface_PrintGraph(
    EXTERNAL_Interface *iface,
    EntityMapping* entityMapping,
    FILE *f)
{
    assert(iface != NULL);
    SocketInterface_InterfaceData* socketData;
    socketData = (SocketInterface_InterfaceData*) iface->data;

    assert(socketData != NULL);
    //Helper function for printing graph
    SocketInterface_HandlePrintGraphLog(iface,entityMapping, f);

#ifdef PARALLEL //Parallel
    {

        Message* graphPrintNotifyMsg = MESSAGE_Alloc (
                        iface->partition->firstNode,
                        PARTITION_COMMUNICATION,    // special layer
                        socketData->printGraphLogCommunicator,           // protocol
                        PARTITION_MSG_PRINT_GRAPHLOG_NOTICE);

        // This triggers the receiving partitions to receive callbacks to
        PARTITION_COMMUNICATION_SendToAllPartitions (iface->partition,
            graphPrintNotifyMsg, 0);
    }
#endif //Parallel
}

void SocketInterface_PrintStats(EXTERNAL_Interface *iface)
{
    SocketInterface_InterfaceData *data;
    int totalMessages;
    double realTimePassed;
    double lastRealTimePassed;
    char s[MAX_STRING_LENGTH];
    char s2[MAX_STRING_LENGTH];

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    sprintf(s,
        "Performance Stats for Time Interval: SimTime = %f\n",
        (double) SocketInterface_Timestamp(iface) / SECOND);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);

    realTimePassed = (double) (EXTERNAL_QueryRealTime(iface)
                               - data->firstStatsUpdateRealTime) / SECOND;
    lastRealTimePassed = (double) (EXTERNAL_QueryRealTime(iface)
                               - data->lastStatsUpdateRealTime) / SECOND;

    sprintf(
        s,
        "Current Sim Time[s] = %14.2f  Real Time[s] = %7.2f\n",
        (double) SocketInterface_Timestamp(iface) / SECOND,
        (double) realTimePassed);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);
    totalMessages = data->creates + data->updates + data->requests;
    sprintf(
        s,
        "    Creates = %d, Updates = %d, Requests = %d, Messages = %d\n",
        data->creates,
        data->updates,
        data->requests,
        totalMessages);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);

    sprintf(
        s,
        "    Responses = %d, Successes = %d, Failures = %d\n",
        data->successes + data->failures,
        data->successes,
        data->failures);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);

    sprintf(
        s,
        "    Avg  Mesg/Sec = %.2f, Succ = %.2f, Fail = %.2f (%.2f late)\n",
        totalMessages / realTimePassed,
        data->successes / realTimePassed,
        data->failures / realTimePassed,
        data->successfulFailures / realTimePassed);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);

    sprintf(
        s,
        "    Last Mesg/sec = %.2f, Succ = %.2f, Fail = %.2f (%.2f late)\n",
        (totalMessages - data->lastTotalMessages) / lastRealTimePassed,
        (data->successes - data->lastSuccesses) / lastRealTimePassed,
        (data->failures - data->lastFailures) / lastRealTimePassed,
        (data->successfulFailures - data->lastSuccessfulFailures) / lastRealTimePassed);
    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);

    if (data->totalPackets > 0)
    {
        sprintf(
            s,
            "    Avg  Real Time Speed = %.2f, ",
            1.0 / (data->totalRealTimeSpeed / data->totalPackets));
    }
    else
    {
        sprintf(
            s,
            "    Avg Real Time Speed = No Packets, ");
    }

    if (data->totalPackets - data->lastTotalPackets > 0)
    {
        sprintf(
            s2,
            "Last Real Time Speed = %.2f\n",
            1.0 / ((data->totalRealTimeSpeed - data->lastTotalRealTimeSpeed) / (data->totalPackets - data->lastTotalPackets)));
    }
    else
    {
        sprintf(
            s2,
            "Last Real Time Speed = No Packets\n");
    }
    strcat(s, s2);

    SocketInterface_PrintLog(iface, data->statsLogFile, s);
    fputs(s, stdout);

    data->lastTotalMessages = totalMessages;
    data->lastTotalPackets = data->totalPackets;
    data->lastTotalRealTimeSpeed = data->totalRealTimeSpeed;
    data->lastResponses = data->responses;
    data->lastSuccesses = data->successes;
    data->lastFailures = data->failures;
    data->lastSuccessfulFailures = data->successfulFailures;
}

void SocketInterface_ReceiveSerializedMessage(
    EXTERNAL_Socket* s,
    SocketInterface_SerializedMessage* message)
{
    SocketInterface_Header header;
    unsigned int length;
    unsigned int recvLength;
    UInt32 messageLength;
    EXTERNAL_SocketErrorType err;

    if (s->socketFd < 0)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_SocketError,
            "Invalid socket in SocketInterface_ReceiveSerializedMessage");
    }

    // Attempt to receive the message header.  If not then return with an
    // empty message size.
    message->m_Size = 0;
    length = SOCKET_INTERFACE_HEADER_SIZE;
    err = EXTERNAL_SocketRecv(
        s,
        (char*) &header,
        SOCKET_INTERFACE_HEADER_SIZE,
        &recvLength,
        FALSE);
    if (err == EXTERNAL_NoSocketError && recvLength == 0)
    {
        // No data, return
        message->m_Size = 0;
        return;
    }
    else if (err != EXTERNAL_NoSocketError || length != recvLength)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_SocketError,
            "Error receiving message header");
    }

    messageLength = header.messageSize;
    EXTERNAL_ntoh(&messageLength, sizeof(UInt32));

    // Make sure message can hold enough info and add header
    if (messageLength < sizeof(SocketInterface_Header))
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Received a QualNet header packet with too small of size.");
    }
    message->Resize(messageLength);
    message->AddData((UInt8*) &header, sizeof(SocketInterface_Header), FALSE);

    // Receive rest of message (if more than a header)
    if (message->m_Size > SOCKET_INTERFACE_HEADER_SIZE)
    {
        // Read the rest of the message into the array
        length = message->m_Size - SOCKET_INTERFACE_HEADER_SIZE;
        err = EXTERNAL_SocketRecv(
            s,
            (char*) &message->m_Data[SOCKET_INTERFACE_HEADER_SIZE],
            length,
            &recvLength);
        if (err != EXTERNAL_NoSocketError || length != recvLength)
        {
            message->m_Size = 0;
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_SocketError,
                "Error receiving message body");
        }
    }
}

void SocketInterface_HandlePropagationFailureSender(
    PartitionData *partition,
    NodeAddress destNodeId,
    Message *msg,
    double pathloss)
{
    SocketInterface_InterfaceData *data;
    Node *node;
    Node *destNode;
    Coordinates nodeCoords;
    Coordinates destNodeCoords;
    char str[MAX_STRING_LENGTH];
    CoordinateType d;

    // gHaveSocketIface will be NULL if this interface is not being used
    if (!gHaveSocketIface)
    {
        return;
    }

    EXTERNAL_Interface * socketIface;
    socketIface = partition->interfaceTable [EXTERNAL_SOCKET];
    // Extract the QualNet data from this interface
    data = (SocketInterface_InterfaceData*) socketIface->data;

    // Only log if parameter is set
    if (!data->logPropagation)
    {
        return;
    }

    // Get the pointer to the source/destination node
    node = MAPPING_GetNodePtrFromHash(
        partition->nodeIdHash,
        msg->originatingNodeId);
    destNode = MAPPING_GetNodePtrFromHash(
        partition->nodeIdHash,
        destNodeId);

    // Check the MAC header to make sure it is being sent to this node
    if (!IsMyMac(destNode, msg))
    {
        return;
    }

    // Get the node coordinates
    MOBILITY_ReturnCoordinates(node, &nodeCoords);
    MOBILITY_ReturnCoordinates(destNode, &destNodeCoords);

    // Calculate distance between 2 positions
    COORD_CalcDistance(
        partition->terrainData->getCoordinateSystem(),
        &nodeCoords,
        &destNodeCoords,
        &d);

    sprintf(
        str,
        "PS: N1: %s N2: %s D: %f TPL: %f\n",
        (MappingGetEntityId(&data->entityMapping, msg->originatingNodeId)).c_str(),
        (MappingGetEntityId(&data->entityMapping, destNodeId)).c_str(),
        d,
        pathloss);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    C1: (%f, %f %f) C2: (%f, %f, %f)\n",
        nodeCoords.common.c1,
        nodeCoords.common.c2,
        nodeCoords.common.c3,
        destNodeCoords.common.c1,
        destNodeCoords.common.c2,
        destNodeCoords.common.c3);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    S: N\n");
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);
}

static double CalculatePathloss(
    Node *node1,
    Node *node2,
    Message *msg)
{
    const int channelIndex = MESSAGE_GetInstanceId(msg);
    assert(channelIndex < node1->numberChannels);
    const int txNodeListId = node1->propData[channelIndex].nodeListId;
    PropPathProfile *profile =
        &(node1->propData[channelIndex].pathProfile[txNodeListId]);
    PropChannel *propChannel =
        &(node1->partitionData->propChannel[channelIndex]);
    double wavelength = propChannel->profile->wavelength;
    float txAntennaHeight;
    float rxAntennaHeight;
    double pathloss_dB;

    // should be time and phy dependent
    txAntennaHeight = ANTENNA_ReturnHeight(node1, 0);
    rxAntennaHeight = ANTENNA_ReturnHeight(node2, 0);

    // The urban prop models require the txAntennaGain and txPower

    double txPower_dBm;
    double txPower_mW;
    double txAntennaGain_dBi;

    PHY_GetTransmitPower(node2,
                         channelIndex,
                         &txPower_mW);

    txPower_dBm = 10.*log10(txPower_mW);

    txAntennaGain_dBi =
        ANTENNA_GainForThisDirectionWithPatternIndex(
            node2,
            channelIndex,
            ANTENNA_OMNIDIRECTIONAL_PATTERN,
            profile->txDOA);


    PROP_CalculatePathloss(
        node1,
        node1->nodeId,
        node2->nodeId,
        channelIndex,
        wavelength,
        txAntennaHeight,
        rxAntennaHeight,
        profile,
        &pathloss_dB);

    return pathloss_dB;
}

void SocketInterface_HandlePropagationFailureReceiver(
    Node *node,
    Message *msg,
    double sinr)
{
    SocketInterface_InterfaceData *data;
    Node *sourceNode;
    Coordinates nodeCoords;
    Coordinates sourceNodeCoords;
    char str[MAX_STRING_LENGTH];
    CoordinateType d;
    double pathloss;

    // gCesIface will be NULL if this interface is not being used
    if (!gHaveSocketIface)
    {
        return;
    }

    // Extract the QualNet data from this interface
    EXTERNAL_Interface * socketIface;
    socketIface = node->partitionData->interfaceTable [EXTERNAL_SOCKET];
    data = (SocketInterface_InterfaceData*) socketIface->data;

    // Only log if parameter is set
    if (!data->logPropagation)
    {
        return;
    }

    // Get the pointer to the source/destination node
    sourceNode = MAPPING_GetNodePtrFromHash(
        node->partitionData->nodeIdHash,
        msg->originatingNodeId);

    // Check the MAC header to make sure it is being sent to this node
    if (!IsMyMac(node, msg))
    {
        return;
    }

    // Get the node coordinates
    MOBILITY_ReturnCoordinates(node, &nodeCoords);
    MOBILITY_ReturnCoordinates(sourceNode, &sourceNodeCoords);

    // Calculate distance between 2 positions
    COORD_CalcDistance(
        node->partitionData->terrainData->getCoordinateSystem(),
        &nodeCoords,
        &sourceNodeCoords,
        &d);

    // Calculate pathloss
    pathloss = CalculatePathloss(
        sourceNode,
        node,
        msg);

    sprintf(
        str,
        "PR: N1: %s N2: %s D: %f TPL: %f\n",
        (MappingGetEntityId(&data->entityMapping, msg->originatingNodeId)).c_str(),
        (MappingGetEntityId(&data->entityMapping, node->nodeId)).c_str(),
        d,
        pathloss);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    C1: (%f, %f %f) C2: (%f, %f, %f)\n",
        sourceNodeCoords.common.c1,
        sourceNodeCoords.common.c2,
        sourceNodeCoords.common.c3,
        nodeCoords.common.c1,
        nodeCoords.common.c2,
        nodeCoords.common.c3);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    SINR: %f S: N\n", sinr);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);
}

void SocketInterface_HandlePropagationSuccessReceiver(
    Node *node,
    Message *msg,
    double sinr)
{
    SocketInterface_InterfaceData *data;
    Node *sourceNode;
    Coordinates nodeCoords;
    Coordinates sourceNodeCoords;
    char str[MAX_STRING_LENGTH];
    CoordinateType d;
    double pathloss;

    // gCesIface will be NULL if this interface is not being used
    if (!gHaveSocketIface)
    {
        return;
    }

    EXTERNAL_Interface * socketIface;
    socketIface = node->partitionData->interfaceTable [EXTERNAL_SOCKET];
    // Extract the QualNet data from this interface
    data = (SocketInterface_InterfaceData*) socketIface->data;

    // Only log if parameter is set
    if (!data->logPropagation)
    {
        return;
    }

    // Get the pointer to the source/destination node
    sourceNode = MAPPING_GetNodePtrFromHash(
        node->partitionData->nodeIdHash,
        msg->originatingNodeId);

    // Check the MAC header to make sure it is being sent to this node
    if (!IsMyMac(node, msg))
    {
        return;
    }

    // Get the node coordinates
    MOBILITY_ReturnCoordinates(node, &nodeCoords);
    MOBILITY_ReturnCoordinates(sourceNode, &sourceNodeCoords);

    // Calculate distance between 2 positions
    COORD_CalcDistance(
        node->partitionData->terrainData->getCoordinateSystem(),
        &nodeCoords,
        &sourceNodeCoords,
        &d);

    // Calculate pathloss
    pathloss = CalculatePathloss(
        sourceNode,
        node,
        msg);

    std::string originatorentityid = 
        MappingGetEntityId(&data->entityMapping, msg->originatingNodeId); 
    std::string nodeentityid = 
        MappingGetEntityId(&data->entityMapping, node->nodeId); 

    sprintf(
        str,
        "PR: N1: %s N2: %s D: %f TPL: %f\n",
        originatorentityid.c_str(),
        nodeentityid.c_str(),
        d,
        pathloss);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    C1: (%f, %f %f) C2: (%f, %f, %f)\n",
        sourceNodeCoords.common.c1,
        sourceNodeCoords.common.c2,
        sourceNodeCoords.common.c3,
        nodeCoords.common.c1,
        nodeCoords.common.c2,
        nodeCoords.common.c3);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);

    sprintf(str, "    SINR: %f S: Y\n", sinr);
    SocketInterface_PrintLog(socketIface, data->propagationLogFile, str);
}

// Partition 0 has sent us notification that a platform has been created.
// We will update our entity mapping data structure so that we know
// the associations between the node id and the entity id
static void SocketInterface_HandleCreatePlatformNotification(
    EXTERNAL_Interface *iface,
    SocketInterface_CreatePlatformNotificationInfo* notificationMessage)
{
    SocketInterface_InterfaceData *data;
    char errString[MAX_STRING_LENGTH];

    ERROR_Assert(iface != NULL, "Interface undefined!"); 
    data = (SocketInterface_InterfaceData*) iface->data;
    ERROR_Assert(data != NULL, "SocketInterface_InterfaceData undefined!"); 

    std::string entityId = notificationMessage->entityId; 
#ifdef DEBUG
    printf("[partition %d] SocketInterface_HandleCreatePlatformNotification for entityId %s & node %d\n", 
        iface->partition->partitionId, entityId.c_str(), notificationMessage->nodeId); 
    fflush(stdout); 
#endif

    if (strcmp( notificationMessage->entityId, "") == 0)
    {
        // Invalid paramter, should have an entity id.
        // Throw an exception
        sprintf(
            errString,
            "Invalid Entity Id Parameter \"%s\"",
            notificationMessage->entityId);
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
    }

    //EntityData* rec = data->entityMapping.entities[notificationMessage->entityId]; 
    EntityData* rec = EntityMappingLookup(&data->entityMapping, notificationMessage->entityId); 

    if (data->entityMapping.dynamic)
    {
        // For the dynamic case, assume one-to-one mapping
        // If the mapping already exists, then we have a problem
        if (rec != NULL)
        {
            sprintf(
                errString,
                "Mapping for the EntityId already exists \"%s\"",
                notificationMessage->entityId);
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }

        // Assign the entityId for the node ID that we've from the notificaiton.
        rec = EntityMappingAddToPlatform( 
            &data->entityMapping,
            (std::string)notificationMessage->entityId,
            notificationMessage->nodeId);            

        if (rec == NULL)
        {
            sprintf(errString,
                "Mapping Failure, Free Nodes unavailable \"%s\"",
                errString);
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }
    }
    else // static case
    {
        // Record should already exist
        if (rec == NULL)
        {
            // Mapping does not exists. We have an error.
            sprintf(errString,
                "Mapping does not exist for platform \"%s\"",
                notificationMessage->entityId);
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidEntityId, errString);
        }
    }    
#ifdef DEBUG
    printf("damage state for node %d is [%d]\n", 
        notificationMessage->nodeId, notificationMessage->damageState); 
    fflush(stdout); 
#endif

    // Check for invalid damage state
    if (notificationMessage->damageState > SocketInterface_PlatformStateType_Damaged)
    {
        sprintf(errString, "Invalid Damage State \"%d\" for node %d",
            notificationMessage->damageState, notificationMessage->nodeId);        
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidState, errString);
    }

    // Fill in the entity data information
    rec->damageState = notificationMessage->damageState;
    
    // Check for the entity Id vs the message Id
    if (rec->entityId == notificationMessage->entityId)
    {
        // Match found, set the node active.
        rec->isActive = TRUE;
    }
/* temp
    // Deal with node type if the create platform indicated a type.
    if (notificationMessage->typeSpecified != SocketInterface_PlatformType_Unspecified)
    {
        if (notificationMessage->typeSpecified == SocketInterface_PlatformType_Ground)
        {
            rec->type = SocketInterface_PlatformType_Ground;
        }
        else if (notificationMessage->typeSpecified == SocketInterface_PlatformType_Air)
        {
            rec->type = SocketInterface_PlatformType_Air;
        }
    }
    else
    {
        // Default to ground node
        rec->type = SocketInterface_PlatformType_Ground;
    }
*/
    std::map<NodeId, Entity_NodeData>::iterator entityit; 
    for (entityit = rec->nodes.begin(); entityit != rec->nodes.end(); entityit++)
    {
        Node* node = (entityit->second.node);        

        // Deal with node type if the create platform indicated a type.
        if (notificationMessage->typeSpecified != SocketInterface_PlatformType_Unspecified)
        {
            if (notificationMessage->typeSpecified == SocketInterface_PlatformType_Ground)
            {
                node->mobilityData->groundNode = TRUE;
                rec->type = SocketInterface_PlatformType_Ground;
            }
            else if (notificationMessage->typeSpecified == SocketInterface_PlatformType_Air)
            {
                node->mobilityData->groundNode = FALSE;
                rec->type = SocketInterface_PlatformType_Air;
            }
        }
        else
        {
            // Default to ground node
            node->mobilityData->groundNode = TRUE;
            rec->type = SocketInterface_PlatformType_Ground;
        }
    } // end for each node in the platform
}

void SocketInterface_HandleGraphLogNotification(
    EXTERNAL_Interface *iface)
{
    EntityMapping* entityMapping;
    FILE* f;

    SocketInterface_InterfaceData* socketData;

    std::vector<NodeList> regionMembership;
    std::vector<NodeAddress> inOtherRegion;
    std::map<int, std::vector<NodeAddress> > gateways;

    std::vector<NodeList>::iterator it;
    std::vector<NodeList>::iterator it2;
    std::vector<NodeAddress>::iterator it3;

    assert(iface != NULL);
    socketData = (SocketInterface_InterfaceData*) iface->data;
    assert(socketData != NULL);

    entityMapping = &socketData->entityMapping;
    f = socketData->graphLogFile;

    SocketInterface_HandlePrintGraphLog(iface, entityMapping, f);
}

/*
 * FUNCTION     UpdateMulticastModelSingleReceiver
 * PURPOSE      Handles updates to multicast comm.effects model for a single receiver,
 *              for a specific multicast packet's comm. effects  
 * Parameters:
 *  packetData:     Pointer to the SocketInterface_PacketData structure,
 *  effects:        Pointer to the multicast modeled effects structure for a single receiver.
 */
void SocketInterface_UpdateTempMulticastModelSingleReceiver(
    SocketInterface_PacketData *packetData,
    SocketInterface_MulticastModeledReceiverCommEffects *effects)
{
    SocketInterface_MulticastModeledReceiverCommEffects *tempEffects;
    SocketInterface_MulticastModeledReceiversCommEffects::iterator it;
    
    assert(packetData != NULL);
    assert(effects != NULL);
    assert(packetData->multicast == TRUE);
    assert(packetData->effects != NULL);
        
    // Insert if the effects hashmap is empty
    if (packetData->effects->empty()) 
    {
        tempEffects = (SocketInterface_MulticastModeledReceiverCommEffects*) 
            MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
        memcpy(tempEffects,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
        
        packetData->effects->insert(pair<const NodeAddress *, 
            SocketInterface_MulticastModeledReceiverCommEffects *>
            (&(tempEffects->receivingPlatformAddress), tempEffects));
    }
    else 
    {
        it = packetData->effects->find(&(effects->receivingPlatformAddress));
        
        // Insert if effects hashmap is non-empty but does not contain the receiver model
        if (it == packetData->effects->end()) 
        {
            tempEffects = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            memcpy(tempEffects,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
                
            packetData->effects->insert(pair<const NodeAddress *, 
                SocketInterface_MulticastModeledReceiverCommEffects *>
                (&(tempEffects->receivingPlatformAddress),  tempEffects));
        }
        
        // Update if effects hashmap is non-empty and does contain the receiver model
        else 
        {
            if (it->second == NULL) 
            {
                it->second = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                    MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            }
            memcpy(it->second,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
        }
    }                
}

/*
 *  This function is a callback that is invoked when , SocketInterface_HandleCreatePlatform (),
 *  (on partition 0) sends a partition communication message to notify
 *  all partitions about the creation of the platform.
 */
static void SocketInterface_PARALLEL_HandleCreatePlatformNotice (PartitionData *
    partitionData, Message * message)
{
    // We are being notified (by partition 0) that a platform has
    // been created - perform all the normal steps when creating a
    // platform, _except_ we are being told wihch nodeId has been
    // assigned for the entityId.
    EXTERNAL_Interface * iface = partitionData->interfaceTable [EXTERNAL_SOCKET];
    SocketInterface_CreatePlatformNotificationInfo * notificationMessage =
        (SocketInterface_CreatePlatformNotificationInfo *) MESSAGE_ReturnInfo (
        message);

    SocketInterface_HandleCreatePlatformNotification(iface, notificationMessage);

    MESSAGE_Free(partitionData->firstNode, message);
}

static void SocketInterface_PARALLEL_HandleGraphLogNotice (PartitionData *
    partitionData, Message * message)
{

    EXTERNAL_Interface * iface = partitionData->interfaceTable [EXTERNAL_SOCKET];

    SocketInterface_HandleGraphLogNotification(iface);

    MESSAGE_Free(partitionData->firstNode, message);
}

//---------------------------------------------------------------------------
// External Interface API Functions
//---------------------------------------------------------------------------

void SocketInterface_Initialize(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput)
{
    SocketInterface_InterfaceData *data;

    // Verify sizes
    assert(sizeof(SocketInterface_Header) == SOCKET_INTERFACE_HEADER_SIZE);
    assert(sizeof(SocketInterface_OptionHeader) == SOCKET_INTERFACE_OPTION_HEADER_SIZE);
    assert(sizeof(SocketInterface_Coordinates) == SOCKET_INTERFACE_COORDINATE_SIZE);

    gHaveSocketIface = true;

    // Create SocketInterface_InterfaceData for this iface
    data = new SocketInterface_InterfaceData;

#ifdef PARALLEL //Parallel
    data->createPlatformCommunicator
        = PARTITION_COMMUNICATION_RegisterCommunicator(iface->partition,
        "SOCKET-INTERFACe notice create platform",
        SocketInterface_PARALLEL_HandleCreatePlatformNotice);

    data->printGraphLogCommunicator
        = PARTITION_COMMUNICATION_RegisterCommunicator(iface->partition,
        "SOCKET-INTERFACE notice Socket_Print graph",
        SocketInterface_PARALLEL_HandleGraphLogNotice);
#endif //Parallel

    // Initial state is standby
    data->simulationState = SocketInterface_StateType_Standby;
 
    // Initial state is NoServer
    data->qualnetMode = SocketInterface_ModeType_NoBootStrap;

    // At the start of the simulation we should wait for a time advance
    // before we begin processing events
    data->waitingForTimeAdvance = TRUE; 

    // Set the default values.  Some of these will be over-ridden by the user
    // in the InitializeNodes function.
    data->timeManagement = SocketInterface_TimeManagementType_TimeManaged;
    data->paused = FALSE;
    data->resetting = FALSE;
    data->idleSentRealTime = 0;
    data->timeAllowance = 0;
    data->idleWhenResponseSent = FALSE;
    data->doWarmup = FALSE;
    data->inIdleResponse = FALSE;
    data->alwaysSuccess = FALSE;
    data->cpuHog = FALSE;
    data->firstBoundary = 0;
    data->lastBoundary = 0;
    data->totalPackets = 0;
    data->totalRealTimeSpeed = 0.0;
    data->firstStatsUpdateRealTime = 0;
    data->firstStatsUpdateSimTime = 0;
    data->nextStatsUpdate = 0;
    data->lastTotalMessages = 0;
    data->lastTotalRealTimeSpeed = 0.0;
    data->lastTotalPackets = 0;
    data->lastResponses = 0;
    data->lastSuccesses = 0;
    data->lastFailures = 0;
    data->lastSuccessfulFailures = 0;
    data->lastStatsUpdateRealTime = 0;
    data->lastStatsUpdateSimTime = 0;
    data->nextGraphUpdate = 0;
    data->printPerPacketStats = FALSE;
    data->gotFirstAdvance = FALSE;
    data->creates = 0;
    data->updates = 0;
    data->requests = 0;
    data->responses = 0;
    data->successes = 0;
    data->failures = 0;
    data->successfulFailures = 0;
    data->deterministicTiming = 0;
    //memset(&data->entityMapping, 0, sizeof(data->entityMapping));
    memset(&data->requestTree, 0, sizeof(data->requestTree));
    memset(&data->multicastTree, 0, sizeof(data->multicastTree));
    data->tcpFailureTimeout = SOCKET_INTERFACE_DEFAULT_TCP_FAILURE_TIMEOUT;
    data->udpFailureTimeout = SOCKET_INTERFACE_DEFAULT_UDP_FAILURE_TIMEOUT;
    data->advanceBoundaries = SOCKET_INTERFACE_DEFAULT_ADVANCE_BOUNDARIES;
    data->coordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;
    data->automaticFlush = TRUE;
    data->driverLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->responsesLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->statsLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->graphLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->errorsLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->propagationLogFile = SOCKET_INTERFACE_DEFAULT_LOG_FILE;
    data->logPrintRealTime = FALSE;
    data->statsPrintRealTime = SOCKET_INTERFACE_DEFAULT_STATS_PRINT_REAL_TIME;
    data->statsPrintInterval = SOCKET_INTERFACE_DEFAULT_STATS_PRINT_INTERVAL;
    data->firstStatsUpdate = TRUE;
    data->graphPrintRealTime = SOCKET_INTERFACE_DEFAULT_GRAPH_PRINT_REAL_TIME;
    data->graphPrintInterval = SOCKET_INTERFACE_DEFAULT_GRAPH_PRINT_INTERVAL;
    data->firstGraphUpdate = TRUE;
    data->logPropagation = FALSE;
    data->inInitializePhase = FALSE;
    // Initialize socket structure.  Will be completed in InitializeNodes.
    data->sockets.numPorts = 0;
    data->pauseSendZeroDelay = FALSE;
    data->pauseAdvanceSimTime = FALSE;

    // Tree must use it's memory pool (tree nodes are recycled) so that
    // data field of nodes can be read from post-free.
    EXTERNAL_TreeInitialize(&data->requestTree, TRUE, 0);
    EXTERNAL_TreeInitialize(&data->multicastTree, TRUE, 0);

    // Add this data to the iface
    iface->data = data;
    // Allocating memory for the nodes in QualNet.
    // printf ("The number of nodes is %i\n",node->partitionData->numNodes);
    // Look up the node

    // SocketInterface_BootStrap (int argc, char * argv [],
    // will have changed the simulation time, so the simulation ends only
    // when it receives the stopSimulation message.
  
    data->incomingMessages = NULL;
    data->lastIncomingMessage = NULL;
    data->numIncomingMessages = 0;

    data->outgoingMessages = NULL;
    data->lastOutgoingMessage = NULL;
    data->numOutgoingMessages = 0;

    // Robust Interface protocol parameters
    data->serverProtocolVersion = 6;
    data->clientProtocolVersion = 3;
    try
    {
        D_Hierarchy *h = &iface->partition->dynamicHierarchy;
        std::string path;
        if (h->CreateExternalInterfacePath("CESSocketInterface",
                                           "ServerProtocolVersion", path))
        {
            D_Object* obj = new D_Int32Obj(&data->serverProtocolVersion);
            h->AddObject(path, obj);
            h->SetWriteable(path, FALSE);
        }
        if (h->CreateExternalInterfacePath("CESSocketInterface",
                                           "ClientProtocolVersion", path))
        {
            D_Object* obj = new D_Int32Obj(&data->clientProtocolVersion);
            h->AddObject(path, obj);
            h->SetWriteable(path, TRUE);
        }
    }
    catch (D_Exception& d)
    {
        // Not an error. The scenario doesn't enable the dynamic hierarchy.
    }
    data->onlyDisableMappedNodes = false;
}

static void SocketInterface_AddEntityMapping(Node*& node, SocketInterface_InterfaceData*& data)
{ 
    ERROR_Assert(node, "invalid node"); 
    ERROR_Assert(data, "invalid interfacedata");        
    
#ifdef DEBUG
    sprintf("SocketInterface_AddEntityMapping for node %d\n"); 
#endif 

    Entity_NodeData entity;   
    entity.node = node;      

    data->entityMapping.unmappedNodes[entity.node->nodeId] = entity; 
}

static BOOL SocketInterface_IsDynamicCommandOnDifferentPartition(
    EXTERNAL_Interface* iface,
    std::string* path,
    int* partition,
    BOOL* isPlatformPath,
    std::string* nodePath)
{
#ifdef PARALLEL //TODO JESSE
    std::list<Node*> nodes; 
    Node* node;
    std::vector<std::string> tokens;
    char nodeIdString[MAX_STRING_LENGTH];

    // Same partition by default
    *partition = iface->partition->partitionId;
    *isPlatformPath = FALSE;

    // Check for /node/nodeId or /platform/entityId
    StringSplit(
        path,
        "/",
        &tokens);
    if (tokens.size() >= 2)
    {
        if (tokens[0] == "node")
        {
            // Get node id.  First check local partition for node, then
            // check remote partition.
            NodeAddress nodeId = atoi(tokens[1].c_str());
            node = MAPPING_GetNodePtrFromHash(
                iface->partition->nodeIdHash,
                nodeId);
            if (node == NULL)
            {
                // Try find node using the shadow node hash.
                node = MAPPING_GetNodePtrFromHash(
                    iface->partition->remoteNodeIdHash, nodeId);
                if (node == NULL)
                {
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidDynamicCommand, "invalid node");
                }
            }
        }
        else if (tokens[0] == "platform")
        {
            SocketInterface_InterfaceData* data;
            data = (SocketInterface_InterfaceData*) iface->data;

            node = MappingGetSingleNodePointer(
                iface->partition,
                &data->entityMapping,
                tokens[1]);
            if (node == NULL)
            {
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidDynamicCommand, "invalid platform");
            }

            *isPlatformPath = TRUE;
            tokens[0] = "/node";
            sprintf(nodeIdString, "%d", node->nodeId);
            tokens[1] = nodeIdString;
            *nodePath = tokens[0];
            for (unsigned i = 1; i < tokens.size(); i++)
            {
                *nodePath += "/" + tokens[i];
            }
        }
        else
        {
            // It is not a platform or node string.  Return false (it is
            // probably on the same partition)
            return FALSE;
        }

        // Check that we got a valid node pointer.  If not return that it is
        // on the same partition since we don't know.  This will probably
        // end up with an invalid path.  If it's on the same partition also
        // return false.  If it's a different partition set the partition and
        // return true.
        if (node == NULL)
        {
            return FALSE;
        }
        else if (node->partitionId == iface->partition->partitionId)
        {
            return FALSE;
        }
        else
        {
            *partition = node->partitionId;
            return TRUE;
        }
    }
    else
    {
        // There are less than 2 paths so it's not a /node/xxx or
        // a /platform/xxx path.  Return false (same partition).
        return FALSE;
    }
#else
    *partition = iface->partition->partitionId;
    return FALSE;
#endif /* PARTITION */
}

static void SocketInterface_SendMessageToOtherPartition(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message,
    BOOL isNodePath,
    int partition)
{
    // Allocate a message for the remote partition
    Message* msg;

    msg = MESSAGE_Alloc(
        iface->partition->firstNode,
        EXTERNAL_LAYER,
        EXTERNAL_SOCKET,
        MSG_EXTERNAL_RemoteMessage);

    // Serialize the message we are to send and it it to the remote message.
    // The first 4 bytes will be the socket id, the remainder bytes will be
    // the serialized message.
    SocketInterface_SerializedMessage* serialized = message->Serialize();

    UInt32 socketId = message->GetSocketId();
    MESSAGE_PacketAlloc(
        iface->partition->firstNode,
        msg,
        serialized->m_Size + sizeof(UInt32) + sizeof(BOOL),
        TRACE_SOCKET_EXTERNAL);
    memcpy(
        MESSAGE_ReturnPacket(msg),
        &socketId,
        sizeof(UInt32));
    memcpy(
        MESSAGE_ReturnPacket(msg) + sizeof(UInt32),
        &isNodePath,
        sizeof(BOOL));
    memcpy(
        MESSAGE_ReturnPacket(msg) + sizeof(UInt32) + sizeof(BOOL),
        serialized->m_Data,
        serialized->m_Size);

    // Send it to our external interafce that is on the other partition
    EXTERNAL_MESSAGE_RemoteSend(
        iface,
        partition,
        msg,
        0,
        EXTERNAL_SCHEDULE_SAFE);
}

void SocketInterface_InitializeRealTime(
    EXTERNAL_Interface* iface)
{
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    if (data->inInitializePhase)
    {
        // We are transitioning to Execution state, and we need to 
        // resume the real time clock and set the interface
        // initialization time to the current time.
        EXTERNAL_Resume(iface);
        iface->initializeTime = EXTERNAL_QueryRealTime(iface);
        data->inInitializePhase = FALSE;
    }
}

static void SocketInterface_HandleFirstStatsUpdate(
    EXTERNAL_Interface* iface)
{
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    if (data->simulationState == SocketInterface_StateType_Executing)
    {
        if (data->statsPrintRealTime)
        {
            if (data->firstStatsUpdate)
            {
                data->firstStatsUpdate = FALSE;
                data->firstStatsUpdateRealTime = EXTERNAL_QueryRealTime(iface);
                data->firstStatsUpdateSimTime = EXTERNAL_QuerySimulationTime(iface);
                data->lastStatsUpdateRealTime = data->firstStatsUpdateRealTime;
                data->lastStatsUpdateSimTime = data->firstStatsUpdateSimTime;
                data->nextStatsUpdate = data->lastStatsUpdateRealTime
                    + data->statsPrintInterval;
            }
        }
        else
        {
            if (data->firstStatsUpdate)
            {
                data->firstStatsUpdate = FALSE;
                data->firstStatsUpdateRealTime = EXTERNAL_QueryRealTime(iface);
                data->firstStatsUpdateSimTime = EXTERNAL_QuerySimulationTime(iface);
                data->lastStatsUpdateRealTime = data->firstStatsUpdateRealTime;
                data->lastStatsUpdateSimTime = data->firstStatsUpdateSimTime;
                data->nextStatsUpdate = EXTERNAL_QuerySimulationTime(iface)
                    + data->statsPrintInterval;
            }
        }

        // Handle graph log update
        if (data->graphPrintRealTime)
        {
            if (data->firstGraphUpdate)
            {
                data->firstGraphUpdate = FALSE;
                data->nextGraphUpdate = EXTERNAL_QueryRealTime(iface)
                                        + data->graphPrintInterval;
            }
        }
        else
        {
            if (data->firstGraphUpdate)
            {
                data->firstGraphUpdate = FALSE;
                data->nextGraphUpdate = EXTERNAL_QuerySimulationTime(iface)
                                        + data->graphPrintInterval;
            }
        }
    }
}

static void SocketInterface_LogMessage(EXTERNAL_Interface* iface, SocketInterface_Message* message)
{
    SocketInterface_InterfaceData *data = (SocketInterface_InterfaceData*) iface->data;

    std::string str;
    message->Print(&str);
    str += "\n";
    SocketInterface_PrintLog(
        iface,
        data->driverLogFile,
        str.c_str());
}

static BOOL SocketInterface_HandleMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message)
{
    SocketInterface_InterfaceData *data;
    char errString[MAX_STRING_LENGTH];
    BOOL scheduled = FALSE;
    clocktype t;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    switch (message->GetType())
    {
        case SocketInterface_MessageType_InitializeSimulation:
        {
            SocketInterface_InitializeSimulationMessage* initialize;

            initialize = (SocketInterface_InitializeSimulationMessage*) message;

            // Set coordinate system
            if (initialize->m_CoordinateSpecified)
            {
                data->coordinateSystem = initialize->m_CoordinateSystem;
            }

            // Get time management mode
            data->timeManagement = initialize->m_TimeManagementMode;
            if (data->timeManagement == SocketInterface_TimeManagementType_RealTime)
            {
                 // Set the time management to real-time with 0 lookahead
                EXTERNAL_SetTimeManagementRealTime(
                    iface,
                    0);
            }

            // Make sure config file matches coordinate system
            if (data->coordinateSystem == SocketInterface_CoordinateSystemType_GccCartesian &&
                iface->partition->terrainData->getCoordinateSystem() != LATLONALT)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_ConfigurationError,
                    "GCC Cartesian coordinates may only be converted to "
                    "lat/lon/alt.  Make sure the QualNet config file uses "
                    "the LATLONALT COORDINATE-SYSTEM.");
            }
            if (data->coordinateSystem == SocketInterface_CoordinateSystemType_Cartesian &&
                iface->partition->terrainData->getCoordinateSystem() != CARTESIAN)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_ConfigurationError,
                    "Interface is configured to use cartesian coordinates but "
                    "QualNet configuration is not cartesian.  Make sure "
                    "COORDINATE-SYSTEM is set to CARESIAN");
            }
            else if (data->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt &&
                iface->partition->terrainData->getCoordinateSystem() != LATLONALT)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_ConfigurationError,
                    "Interface is configured to use lat/lon/alt coordinates but"
                    " QualNet configuration is not lat/lon/alt.  Make sure "
                    "COORDINATE-SYSTEM is set to LATLONALT");
            }


            // Set Source Response Multicast
            if (initialize->m_SourceResponseMulticastEnabled)
            {
                data->sourceResponsibleMulticast = initialize->m_SourceResponseMulticastEnabled;
            }
            else
            {
                data->sourceResponsibleMulticast = 0;
            }

            // Should only be sent when not using Server mode.
            if (SocketInterface_GetModeType(iface) == SocketInterface_ModeType_NoBootStrap)
            {
                if (initialize->m_Scenario.size() > 0)
                {
                    throw SocketInterface_Exception(
                        SocketInterface_ErrorType_InvalidMessage,
                        "Initialization scenario not supported in this mode");
                }
            }

            SocketInterface_LogMessage(iface, message);

            // If we are doing a warmup then go to the warmup phase.  This
            // block of code will wait for a warmup message then move to the
            // warmup phase.  When the warmup phase is over QualNet will move to
            // the intialize phase.
            if (data->doWarmup)
            {
                SocketInterface_HandleWaitForWarmup(iface);
            }
            else
            {
                // Change state to initialized
                SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Initialized);
            }
            break;
        }

        case SocketInterface_MessageType_BeginWarmup:
        {
            SocketInterface_BeginWarmupMessage* beginWarmup;

            SocketInterface_LogMessage(iface, message);

            beginWarmup = (SocketInterface_BeginWarmupMessage*) message;

            // Change state to SocketInterface_StateType_Warmup.  This will cause QualNet to leave
            // the SocketInterface_HandleWaitForWarmup routine (which calls this
            // function)
            SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Warmup);

            EXTERNAL_BeginWarmup(iface);
            break;
        }

        case SocketInterface_MessageType_PauseSimulation:
        {
            SocketInterface_PauseSimulationMessage* pause;

            SocketInterface_LogMessage(iface, message);

            pause = (SocketInterface_PauseSimulationMessage*) message;

            if (pause->m_PauseTimeSpecified)
            {
                t = SocketInterface_TimestampToQualNet(iface, pause->m_PauseTime);
                if (t > EXTERNAL_QuerySimulationTime(iface))
                {
                    SocketInterface_ScheduleMessage(
                        iface,
                        message,
                        t);
                    scheduled = TRUE;
                }
            }
            
            if (!scheduled)
            {
                SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Paused);
            }
            break;
        }

        case SocketInterface_MessageType_ExecuteSimulation:
        {
            SocketInterface_ExecuteSimulationMessage* execute;

            SocketInterface_LogMessage(iface, message);

            execute = (SocketInterface_ExecuteSimulationMessage*) message;

            // Change state to executing
            SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Executing);

            // Handle the print stats.
            SocketInterface_HandleFirstStatsUpdate(iface);
            break;
        }

        case SocketInterface_MessageType_StopSimulation:
            SocketInterface_LogMessage(iface, message);
            SocketInterface_HandleStopSimulation(
                iface,
                (SocketInterface_StopSimulationMessage*) message);
            break;

        case SocketInterface_MessageType_ResetSimulation:
            SocketInterface_LogMessage(iface, message);
            SocketInterface_HandleResetSimulation(
                iface,
                (SocketInterface_ResetSimulationMessage*) message);
            break;

        case SocketInterface_MessageType_AdvanceTime:
            SocketInterface_LogMessage(iface, message);
            SocketInterface_HandleAdvanceTime(
                iface,
                (SocketInterface_AdvanceTimeMessage*) message);
            break;

        case SocketInterface_MessageType_CommEffectsRequest:
        {
            SocketInterface_CommEffectsRequestMessage* request =
                (SocketInterface_CommEffectsRequestMessage*) message;

            SocketInterface_LogMessage(iface, message);
            data->requests++;

            if (request->m_SendTimeSpecified)
            {
                t = SocketInterface_TimestampToQualNet(iface, request->m_SendTime);
                if (t > EXTERNAL_QuerySimulationTime(iface))
                {
                    SocketInterface_ScheduleMessage(
                        iface,
                        message,
                        t);
                    scheduled = TRUE;
                }
            }
            
            if (!scheduled)
            {
                SocketInterface_HandleCommEffectsRequest(
                    iface,
                    (SocketInterface_CommEffectsRequestMessage*) message,
                    message->GetSocketId());
            }
            break;
        }

        case SocketInterface_MessageType_CreatePlatform:
        {
            SocketInterface_CreatePlatformMessage* create =
                (SocketInterface_CreatePlatformMessage*) message;

            SocketInterface_LogMessage(iface, message);
            if (create->m_CreateTimeSpecified)
            {
                t = SocketInterface_TimestampToQualNet(iface, create->m_CreateTime);
                if (t > EXTERNAL_QuerySimulationTime(iface))
                {
                    SocketInterface_ScheduleMessage(
                        iface,
                        message,
                        t);
                    scheduled = TRUE;
                }
            }

            if (!scheduled)
            {
                SocketInterface_HandleCreatePlatform(
                    iface,
                    (SocketInterface_CreatePlatformMessage*) message);
            }
            break;
        }

        case SocketInterface_MessageType_UpdatePlatform:
        {
            SocketInterface_UpdatePlatformMessage* update =
                (SocketInterface_UpdatePlatformMessage*) message;

            SocketInterface_LogMessage(iface, message);

            // Schedule the update if it has a timestamp
            SocketInterface_HandleUpdatePlatform(
                iface,
                update);
            break;
        }

        case SocketInterface_MessageType_DynamicCommand:
        {
            SocketInterface_DynamicCommandMessage* dynamic =
                (SocketInterface_DynamicCommandMessage*) message;
            
            SocketInterface_LogMessage(iface, message);

            SocketInterface_HandleDynamicCommand(
                iface,
                dynamic,
                message->GetSocketId());

            /*int partition;
            BOOL isPlatformPath;
            std::string nodePath;
            BOOL otherPartition = SocketInterface_IsDynamicCommandOnDifferentPartition(
                iface,
                &dynamic->m_Path,
                &partition,
                &isPlatformPath,
                &nodePath);
            if (otherPartition)
            {
                // If it's a platform path send the node path to the other
                // partition.  This is a temp hack because the other partition
                // doesn't know about the entity link.
                if (isPlatformPath)
                {
                    SocketInterface_DynamicCommandMessage* nodeDynamic = new SocketInterface_DynamicCommandMessage(dynamic->m_Type, &nodePath, &dynamic->m_Args);
                    nodeDynamic->SetSocketId(dynamic->GetSocketId());

                    SocketInterface_SendMessageToOtherPartition(
                        iface,
                        nodeDynamic,
                        isPlatformPath,
                        partition);
                    delete nodeDynamic;
                }
                else
                {
                    SocketInterface_SendMessageToOtherPartition(
                        iface,
                        message,
                        isPlatformPath,
                        partition);
                }
            }
            else
            {
                SocketInterface_HandleDynamicCommand(
                    iface,
                    dynamic,
                    message->GetSocketId());
            }*/
            break;
        }

        case SocketInterface_MessageType_QuerySimulationState:
        {
            SocketInterface_LogMessage(iface, message);
            SocketInterface_HandleSimulationStateMessage(iface,
                                         data->simulationState,
                                         data->simulationState);
            break;
        }

        default:
            sprintf(
                errString,
                "Unknown message type \"%d\"",
                message->GetType());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, errString);
            break;
    }
    
    return scheduled;
}


void SocketInterface_InitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput)
{
    SocketInterface_InterfaceData *data;
    char errString[MAX_STRING_LENGTH];
    char buf[MAX_STRING_LENGTH];
    int intBuf;
    BOOL retVal;
    BOOL boolVal;
    int i;
    int numPorts = 1;
    Node *node;
    EXTERNAL_SocketErrorType err;
     
    std::map<std::string, NodeIdList> hostNodes; 

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Create EntintyMappingRecords for all nodes, this will
    // result in the createplatform selecting nodes whose node Ids
    // will match regarldess of being local or remote, so that events
    // are equal between par and seq.
    NodePointerCollectionIter nodeIter;
    nodeIter = iface->partition->allNodes->begin();
    node = *nodeIter;

    int index = 0;

    for (nodeIter = iface->partition->allNodes->begin ();
        nodeIter != iface->partition->allNodes->end ();
        nodeIter++)
    {
        node = *nodeIter;
        assert(node != NULL); 
        SocketInterface_AddEntityMapping(node, data);  
#ifdef DEBUG
        printf("NodeId %d has hostname %s\n", node->nodeId, node->hostname);
        fflush(stdout); 
#endif        
        std::string hostname; 
        if (node->hostname != NULL)
        {
            hostname = node->hostname; 
        }
        else // sometimes when running in parallel, node->hostname is null
        {
            BOOL wasFound = false;
            char name[MAX_STRING_LENGTH];
            IO_ReadString(node->nodeId, ANY_ADDRESS,
                nodeInput, "HOSTNAME", &wasFound, name);
            if (!wasFound)
            {
                sprintf(name, "host %d", node->nodeId); //default
            }

            node->hostname = name;
            hostname = node->hostname;
        }
        std::map<std::string, NodeIdList>::iterator hostmapIt;
        hostmapIt = hostNodes.find(hostname); 
        if (hostmapIt != hostNodes.end())
        {
            hostmapIt->second.push_back(node->nodeId);                
        }  
        else
        {      
            NodeIdList nlist; 
            nlist.push_back(node->nodeId); 
            hostNodes[node->hostname] = nlist; 
        }
        // Testing
        //cout << "NodeId is " << node->nodeId << " index is " << data->table->nodeMap[node->nodeId]<< endl;        
        index++;
    }

    // Check for sockets from bootstrap phase
    std::vector<int>* ports;
    std::vector<EXTERNAL_Socket*>* listeningSockets;
    std::vector<EXTERNAL_Socket*>* connections;
    std::vector<SocketInterface_Message*>* messages;

    listeningSockets =
        (std::vector<EXTERNAL_Socket*>*) PARTITION_ClientStateFind(
        iface->partition,
        "Socket_ListeningSockets");

    ports =
        (std::vector<int>*) PARTITION_ClientStateFind(
        iface->partition,
        "Socket_ListeningPorts");

    connections =
        (std::vector<EXTERNAL_Socket*>*) PARTITION_ClientStateFind(
        iface->partition,
        "Socket_Connections");

    messages =
        (std::vector<SocketInterface_Message*>*) PARTITION_ClientStateFind(
        iface->partition,
        "Socket_Messages");

    if (listeningSockets != NULL &&
        ports != NULL &&
        connections != NULL &&
        messages != NULL)
    {
        // QualNet is running in bootstrap mode.
        // Change the QualNet mode.
        data->qualnetMode = SocketInterface_ModeType_BootStrap;

        data->sockets.listeningSockets = *listeningSockets;
        data->sockets.listenPorts = *ports;
        data->sockets.numPorts = ports->size();

        // Add connections.  All are initially active.
        data->sockets.connections = *connections;
        data->sockets.numConnections = connections->size();
        for (i = 0; i < data->sockets.numConnections; i++)
        {
            data->sockets.activeConnections.push_back(TRUE);
        }
    }
    bool isLegacyCES = false;
    //first check if interface is to be used
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "SOCKET-INTERFACE",
        &retVal,
        buf);

    if (!retVal)
    {
        //Backwards compatibility check
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-SOCKET",
            &retVal,
            buf);

        if (!retVal || strcmp(buf, "YES") != 0)
        {
            return;
        }
        ERROR_ReportWarning("CES-SOCKET has been deprecated, \
                            please replace with SOCKET-INTERFACE");
        isLegacyCES = true;
    }
    if (retVal && strcmp(buf, "YES") != 0)
    {
        return;
    }

    // Read the number of listening sockets to create
    if (isLegacyCES)
    {
        IO_ReadInt(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-SOCKET-NUM-PORTS",
            &retVal,
            &intBuf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-SOCKET-NUM-PORTS has been deprecated, \
                please replace with SOCKET-INTERFACE-NUM-PORTS");
        }
    }
    else
    {
        IO_ReadInt(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-NUM-PORTS",
            &retVal,
            &intBuf);
    }
    if (retVal)
    {
        numPorts = intBuf;
    }

    for (i = 0; i < numPorts; i++)
    {
        // Read the port for each socket
        if (isLegacyCES)
        {
            IO_ReadIntInstance(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "CES-SOCKET-PORT",
                i,
                TRUE,
                &retVal,
                &intBuf);
            if (retVal)
            {
                ERROR_ReportWarning("CES-SOCKET-PORT has been deprecated, \
                    please replace with SOCKET-INTERFACE-PORT");
            }
        }
        else
        {
            IO_ReadIntInstance(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "SOCKET-INTERFACE-PORT",
                i,
                TRUE,
                &retVal,
                &intBuf);
        }
        if (retVal)
        {
            // Do nothing, port will be added below
        }
        else
        {
            // If port is not specified, it's an error if there are multiple
            // ports.  If there is one port use the default port.
            if (numPorts > 1)
            {
                ERROR_ReportError(
                    "Must specify a port for every QualNet listening socket");
            }
            else
            {
                intBuf = SOCKET_INTERFACE_DEFAULT_PORT;
            }
        }

        // Open listening ports if on partition 0, and this port wasn't
        // already opened in bootstrap phase
        std::vector<int>::iterator it;
        it = std::find(
            data->sockets.listenPorts.begin(),
            data->sockets.listenPorts.end(),
            intBuf);
        if (iface->partition->partitionId == 0
            && it == data->sockets.listenPorts.end())
        {
            EXTERNAL_Socket *tempSocket = (EXTERNAL_Socket*) MEM_malloc(sizeof(EXTERNAL_Socket));

            data->sockets.listeningSockets.push_back(tempSocket);
            data->sockets.listenPorts.push_back(intBuf);
            data->sockets.numPorts++;

            do
            {
                err = EXTERNAL_SocketInitListen(
                    tempSocket,
                    intBuf,
                    FALSE,
                    TRUE);
                if (err != EXTERNAL_NoSocketError)
                {
                    sprintf(
                        errString,
                        "Unable to open socket on port %d.  Trying again in 3 seconds.",
                        intBuf);
                    ERROR_ReportWarning(errString);
                    EXTERNAL_Sleep(3 * SECOND);
                }
            } while (err != EXTERNAL_NoSocketError);
        }
    }

    // Read TCP failure timeout if present 
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-TCP-FAILURE-TIMEOUT",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-TCP-FAILURE-TIMEOUT has been deprecated, \
                please replace with SOCKET-INTERFACE-TCP-FAILURE-TIMEOUT");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,  
            "SOCKET-INTERFACE-TCP-FAILURE-TIMEOUT",
            &retVal,
            buf);
    }
    if (retVal)
    {
        data->tcpFailureTimeout = TIME_ConvertToClock(buf);
    }

    // Read UDP failure timeout if presen        
    if (isLegacyCES)
    {        
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-UDP-FAILURE-TIMEOUT",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-UCP-FAILURE-TIMEOUT has been deprecated, \
                please replace with SOCKET-INTERFACE-UCP-FAILURE-TIMEOUT");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-UDP-FAILURE-TIMEOUT",
            &retVal,
            buf);
    }
    if (retVal)
    {
        data->udpFailureTimeout = TIME_ConvertToClock(buf);
    }

    // Read warm up time if present
    if (iface->partition->partitionId == 0)
    {
        if (EXTERNAL_WarmupTimeEnabled(iface))
        {
            data->doWarmup = TRUE;
        }
    }

    // Read SOCKET-INTERFACE-ALWAYS-SUCCESS if present
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-ALWAYS-SUCCESS",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-ALWAYS-SUCCESS has been deprecated, \
                please replace with SOCKET-INTERFACE-ALWAYS-SUCCESS");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-ALWAYS-SUCCESS",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->alwaysSuccess = boolVal;
    }

    // Read SOCKET-INTERFACE-CPU-HOG if present    
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-CPU-HOG",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-CPU-HOG has been deprecated, \
                please replace with SOCKET-INTERFACE-CPU-HOG");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-CPU-HOG",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->cpuHog = boolVal;
        if (data->cpuHog)
        {
            EXTERNAL_SetCpuHog(iface);
        }
    }

    // Read SOCKET-INTERFACE-PAUSE-REPLY-ZERO-DELAY if present       
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-PAUSE-REPLY-ZERO-DELAY",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-PAUSE-REPLY-ZERO-DELAY has been deprecated, \
                please replace with SOCKET-INTERFACE-PAUSE-REPLY-ZERO-DELAY");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-PAUSE-REPLY-ZERO-DELAY",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->pauseSendZeroDelay = boolVal;
    }
    
    // Read SOCKET-INTERFACE-PAUSE-ADVANCE-SIMULATION-TIME if present       
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-PAUSE-ADVANCE-SIMULATION-TIME",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-PAUSE-ADVANCE-SIMULATION-TIME has been deprecated, \
                please replace with SOCKET-INTERFACE-PAUSE-ADVANCE-SIMULATION-TIME");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-PAUSE-ADVANCE-SIMULATION-TIME",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->pauseAdvanceSimTime = boolVal;
    }

    // Read SOCKET-INTERFACE-IDLE-WHEN-RESPONSE-SENT if present
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-IDLE-WHEN-RESPONSE-SENT",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-IDLE-WHEN-RESPONSE-SENT has been deprecated, \
                please replace with SOCKET-INTERFACE-IDLE-WHEN-RESPONSE-SENT");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-IDLE-WHEN-RESPONSE-SENT",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->idleWhenResponseSent = boolVal;
    }
    
    // Read advance boundaries if present    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-ADVANCE-BOUNDARIES",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-ADVANCE-BOUNDARIES has been deprecated, \
                please replace with SOCKET-INTERFACE-ADVANCE-BOUNDARIES");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-ADVANCE-BOUNDARIES",
            &retVal,
            buf);
    }
    if (retVal)
    {
        data->advanceBoundaries = atoi(buf);
    }

    // Read print per packet stats if present
    if (isLegacyCES)
    {
        //Backwards compatibility check
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-PRINT-PER-PACKET-STATS",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-PRINT-PER-PACKET-STATS has been deprecated, \
                please replace with SOCKET-INTERFACE-PRINT-PER-PACKET-STATS");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-PRINT-PER-PACKET-STATS",
            &retVal,
            buf);
    }

    if (retVal)
    {
        data->printPerPacketStats = TRUE;

        // Create the output file
        if (strcmp(buf, "STDOUT") == 0)
        {
            data->statsFile = stdout;
        }
        else
        {
            data->statsFile = fopen(buf, "w");
            if (data->statsFile == NULL)
            {
                printf ("Can not create the per packet stats file\n");
                exit(0);
            }
            //assert(data->statsFile != NULL);
        }
    }

    // Read the log propagation option    
    if (isLegacyCES)
    {
        //Backwards compatibility check
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-LOG-PROPAGATION",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-LOG-PROPAGATION has been deprecated, \
                please replace with SOCKET-INTERFACE-LOG-PROPAGATION");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-LOG-PROPAGATION",
            &retVal,
            buf);
    }
    if (retVal)
    {
        if (strcmp(buf, "1") == 0)
        {
            data->logPropagation = TRUE;
        }
        if (strcmp(buf, "YES") == 0)
        {
            data->logPropagation = TRUE;
        }
    }

    // Read the log file option    
    if (isLegacyCES)
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-LOG-AUTOMATIC-FLUSH",
            &retVal,
            &boolVal);
        if (retVal)
        {
            ERROR_ReportWarning("CES-LOG-AUTOMATIC-FLUSH has been deprecated, \
                please replace with SOCKET-INTERFACE-LOG-AUTOMATIC-FLUSH");
        }
    }
    else
    {
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-LOG-AUTOMATIC-FLUSH",
            &retVal,
            &boolVal);
    }
    if (retVal)
    {
        data->automaticFlush = boolVal;
    }

    // Read the log file option    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-LOG",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-LOG has been deprecated, \
                please replace with SOCKET-INTERFACE-LOG");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-LOG",
            &retVal,
            buf);
    }
    if (!retVal || (strcmp(buf, "FILE") == 0))
    {
        if (isLegacyCES)
        {
            MakeDirectories("CES-SOCKET_", buf, iface->partition->partitionId);
        }
        else
        {
            MakeDirectories("SOCKET-INTERFACE_", buf, iface->partition->partitionId);
        }
        SocketInterface_CreateLogFiles(iface, buf);
    }
    else if (strcmp(buf, "STDOUT") == 0)
    {
        data->driverLogFile = stdout;
        data->responsesLogFile = stdout;
        data->statsLogFile = stdout;
        data->graphLogFile = stdout;
        data->errorsLogFile = stdout;
        data->propagationLogFile = stdout;
    }
    else if (strcmp(buf, "NONE") == 0)
    {
        data->driverLogFile = NULL;
        data->responsesLogFile = NULL;
        data->statsLogFile = NULL;
        data->graphLogFile = NULL;
        data->errorsLogFile = NULL;
        data->propagationLogFile = NULL;
    }
    else
    {
        if (isLegacyCES)
        {
            sprintf(errString, "Unknown CES-LOG option \"%s\"", buf);
        }
        else
        {
            sprintf(errString, "Unknown SOCKET-INTERFACE-LOG option \"%s\"", buf);
        }
        ERROR_ReportError(errString);
    }

    // Read whether stats should be printed in real time or sim time    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-STATS-PRINT-REAL-TIME",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-STATS-PRINT-REAL-TIME has been deprecated, \
                please replace with SOCKET-INTERFACE-STATS-PRINT-REAL-TIME");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-STATS-PRINT-REAL-TIME",
            &retVal,
            buf);
    }
    if (retVal)
    {
        if (strcmp(buf, "0") == 0)
        {
            data->statsPrintRealTime = FALSE;
        }
        if (strcmp(buf, "NO") == 0)
        {
            data->statsPrintRealTime = FALSE;
        }
    }

    // Read the stats print interval    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-PRINT-REAL-TIME",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-PRINT-REAL-TIME has been deprecated, \
                please replace with SOCKET-INTERFACE-PRINT-REAL-TIME");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-PRINT-REAL-TIME",
            &retVal,
            buf);
    }
    if (retVal)
    {
        if (strcmp(buf, "1") == 0)
        {
            data->logPrintRealTime = TRUE;
        }
        if (strcmp(buf, "YES") == 0)
        {
            data->logPrintRealTime = TRUE;
        }
    }
    
    // Read the stats print interval    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-STATS-PRINT-INTERVAL",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-STATS-PRINT-INTERVAL has been deprecated, \
                please replace with SOCKET-INTERFACE-STATS-PRINT-INTERVAL");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-STATS-PRINT-INTERVAL",
            &retVal,
            buf);
    }
    if (retVal)
    {
        data->statsPrintInterval = atoi(buf) * SECOND;
        // Create timer for the print interval if printing in sim time
        if (!data->statsPrintRealTime && iface->partition->partitionId == 0)
        {
            Message* timer;
            timer = MESSAGE_Alloc(
                iface->partition->firstNode,
                EXTERNAL_LAYER,
                EXTERNAL_SOCKET,
                MSG_EXTERNAL_SOCKET_INTERFACE_StatsLogTimer);
            MESSAGE_Send(
                iface->partition->firstNode,
                timer,
                data->statsPrintInterval);
        }
    }

    // Read whether graph should be printed in real time or sim tim   
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-GRAPH-PRINT-REAL-TIME",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-GRAPH-PRINT-REAL-TIME has been deprecated, \
                please replace with SOCKET-INTERFACE-GRAPH-PRINT-REAL-TIME");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-GRAPH-PRINT-REAL-TIME",
            &retVal,
            buf);
    }
    if (retVal)
    {
        if (strcmp(buf, "0") == 0)
        {
            data->graphPrintRealTime = FALSE;
        }
        if (strcmp(buf, "NO") == 0)
        {
            data->graphPrintRealTime = FALSE;
        }
    }

    // Read the stats print interval    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-GRAPH-PRINT-INTERVAL",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-GRAPH-PRINT-INTERVAL has been deprecated, \
                please replace with SOCKET-INTERFACE-GRAPH-PRINT-INTERVAL");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-GRAPH-PRINT-INTERVAL",
            &retVal,
            buf);
    }
    if (retVal)
    {
        data->graphPrintInterval = atoi(buf) * SECOND;
        if (!data->graphPrintRealTime && iface->partition->partitionId == 0)
        {
            Message* timer;
            timer = MESSAGE_Alloc(
                iface->partition->firstNode,
                EXTERNAL_LAYER,
                EXTERNAL_SOCKET,
                MSG_EXTERNAL_SOCKET_INTERFACE_GraphLogTimer);
            MESSAGE_Send(
                iface->partition->firstNode,
                timer,
                data->graphPrintInterval);
        }
    }

     // Read the entity mapping file if present    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-ENTITY-MAPPING-FILE",
            &retVal,
            buf);
        if (!retVal)
        {
            IO_ReadString(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "CES-ENTITY-MAPPING",
                &retVal,
                buf);
            if (retVal)
            {
                ERROR_ReportWarning("CES-ENTITY-MAPPING has been deprecated, \
                    please replace with SOCKET-INTERFACE-ENTITY-MAPPING");
            }
        }
        else
        {
            ERROR_ReportWarning("CES-ENTITY-MAPPING-FILE has been deprecated, \
                please replace with SOCKET-INTERFACE-ENTITY-MAPPING");
        }

    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-ENTITY-MAPPING",
            &retVal,
            buf);
    }
    if (retVal) // static --> read from file
    {
        std::string errorString;
        BOOL fail;

        EntityMappingInitialize(&data->entityMapping, 31, FALSE);
        fail = ReadEntityMappingFile(iface, hostNodes, buf, &errorString);
        if (fail)
        {
            SocketInterface_PrintLog(iface, data->errorsLogFile, errorString.c_str());
        }
    }
    else // dynamic --> mapping created by messages
    {
        EntityMappingInitialize(&data->entityMapping, 31, TRUE);
    }

    // Read determinstic timing behavior, if present
    data->deterministicTiming = TRUE;    
    if (isLegacyCES)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "CES-DETERMINISTIC-TIMING",
            &retVal,
            buf);
        if (retVal)
        {
            ERROR_ReportWarning("CES-DETERMINISTIC-TIMING has been deprecated, \
                please replace with SOCKET-INTERFACE-DETERMINISTIC-TIMING");
        }
    }
    else
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "SOCKET-INTERFACE-DETERMINISTIC-TIMING",
            &retVal,
            buf);
    }
    if (retVal)
    {
        if (strcmp(buf, "0") == 0)
        {
            data->deterministicTiming = FALSE;
        }
        if (strcmp(buf, "NO") == 0)
        {
            data->deterministicTiming = FALSE;
        }
    }
    if (data->deterministicTiming)
    {
        // Turn off lookahead.
#ifdef PARALLEL //Parallel
        PARALLEL_SetProtocolIsNotEOTCapable (iface->partition->firstNode);
#endif
    }

    // Initialize the boundaries
    data->boundaryTimes = (clocktype*)
                          MEM_malloc(sizeof(clocktype)
                          * data->advanceBoundaries);
    for (i = 0; i < data->advanceBoundaries; i++)
    {
        data->boundaryTimes[i] = -1;
    }
    data->firstBoundary = -1;
    data->lastBoundary = -1;

    //Check if the user wishes to deactivate only mapped nodes, ex: plan to use
    //DIS for unmapped nodes
    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "SOCKET-INTERFACE-ONLY-DEACTIVATE-MAPPED-NODES",
        &retVal,
        &data->onlyDisableMappedNodes);

    if (iface->partition->partitionId == 0)
    {
        // Deactivate all nodes on all partitions
        NodePointerCollectionIter nodeIter;
        for (nodeIter = iface->partition->allNodes->begin ();
            nodeIter != iface->partition->allNodes->end ();
            nodeIter++)
        {
            node = *nodeIter;
            
            //Deactive node if all are set to be diasabled
            //or if node is currently mapped
            if (!data->onlyDisableMappedNodes || 
                data->entityMapping.unmappedNodes.find(node->nodeId) ==
                    data->entityMapping.unmappedNodes.end())
            {
                EXTERNAL_DeactivateNode(iface, node, EXTERNAL_SCHEDULE_SAFE);
            }
        }
    }

    // Handle the initialize message received
    // during the preinitialization stage. It is
    // important to process that before the other
    // message.
    if (data->qualnetMode == SocketInterface_ModeType_BootStrap)
    {
        SocketInterface_Message* message;

        // Extract the message from the ClientState dictionary.
        message =  (SocketInterface_Message*) PARTITION_ClientStateFind(
            iface->partition,
            "SocketInterface_InitializeSimulationMessage");

        // Handle the initialize message
        SocketInterface_HandleReceiverMessage(iface, message, message->GetSocketId());
    }

    // Process any messages any messages received during bootstrap phase
    if (messages != NULL)
    {
        for (i = 0; i < (int)messages->size(); i++)
        {
            // Handle the message
            BOOL scheduledMessage = FALSE;
            try
            {
                scheduledMessage = SocketInterface_HandleMessage(iface, (*messages)[i]);
            }
            catch (SocketInterface_Exception& e)
            {
                // Handle the exception
                SocketInterface_HandleException(iface, &e, (*messages)[i], (*messages)[i]->GetSocketId());
            }

            // Free memory.  Only delete messages that were not scheduled (ie,
            // messages that were already processed)
            if (!scheduledMessage)
            {
                delete (*messages)[i];
                (*messages)[i] = NULL;
            }
        }

        // Free messages array
        delete messages;
    }

    // Set simulation to never end
    EXTERNAL_SetSimulationEndTime(iface, CLOCKTYPE_MAX);

    // Start sender/receiver threads if on partition zero
    if (iface->partition->partitionId == 0)
    {
        pthread_create(
            &data->sockets.receiverThread,
            NULL,
            SocketInterface_ReceiverThread,
            iface);

        pthread_create(
            &data->sockets.senderThread,
            NULL,
            SocketInterface_SenderThread,
            iface);
    }

    // Read in if distributed QualNet environment behavior is required, 
    // if present and this is partition 0
    data->useStatisticalModel = FALSE;
    data->unicastStatModel = NULL;
    data->multicastStatModel = NULL;
    if (iface->partition->partitionId == 0) 
    {        
        if (isLegacyCES)
        {
            IO_ReadBool(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "CES-DISTRIBUTED-ENVIRONMENT",
                &retVal,
                &boolVal);
            if (retVal)
            {
                ERROR_ReportWarning("CES-DISTRIBUTED-ENVIRONMENT has been deprecated, \
                    please replace with SOCKET-INTERFACE-DISTRIBUTED-ENVIRONMENT");
            }
        }
        else
        {
            IO_ReadBool(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "SOCKET-INTERFACE-DISTRIBUTED-ENVIRONMENT",
                &retVal,
                &boolVal);
        }
        if (retVal)
        {
            // If requested, set the config value and create the
            // model hashmaps
            if (boolVal)
            {
                data->useStatisticalModel = TRUE;
                SocketInterface_InitializeModels(data);
            }
        }
    }
}

clocktype SocketInterface_FederationTime(EXTERNAL_Interface *iface)
{
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // Return the current time
    return data->timeAllowance;
}

void SocketInterface_SimulationHorizon(EXTERNAL_Interface* iface)
{
    SocketInterface_InterfaceData *data;
    int i;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    // We should never get here if running in real time mode
    assert(data->timeManagement != SocketInterface_TimeManagementType_RealTime);

    // We should not move the horizon if we are not in the
    // execution mode. Though there is one exception to this.
    // If we are in pause state and SOCKET-INTERFACE-PAUSE-ADVANCE-SIM-TIME is YES,
    // then we move the simulation hosrizon
    if (!(data->simulationState == SocketInterface_StateType_Executing || 
        (data->pauseAdvanceSimTime && data->simulationState == SocketInterface_StateType_Paused)))
    {
        return;
    }

    // If we have been granted extra time that has not been included
    // into the simulation horizon, update the horizon and return.  Only
    // do this if we are not idling because of IdleWhenResponseSent.
    // If we are idling because of a response then we will only change
    // the horizon upon receiving a new AdvanceSimulationTime message.
    //
    // The time horizon is advanced only in the execute state
    if (!data->inIdleResponse)
    {
        if (data->timeAllowance > iface->horizon)
        {
            iface->horizon = data->timeAllowance;
            return;
        }
    }

    // This function is called when the current horizon has been
    // reached. This means we are idle, and Therefore we should send a
    // ReportSimulationIdleState to the MTS, but only once.
    if (!data->waitingForTimeAdvance)
    {
        // Send a ReportSimulationIdleState message
        SocketInterface_SimulationIdleMessage* idle;

        // Create message and send it to all connections
        idle = new SocketInterface_SimulationIdleMessage(
            (double) SocketInterface_Timestamp(iface) / SECOND);
        for (i = 0; i < (int)data->sockets.connections.size(); i++)
        {
            if (data->sockets.activeConnections[i])
            {
                SocketInterface_SendMessage(iface, idle, i);
            }
        }

        // Print to the log file
        std::string str;
        idle->Print(&str);
        str += "\n";

        SocketInterface_PrintLog(
            iface,
            data->responsesLogFile,
            str.c_str());

        delete idle;

        // Set the sent idle message flag.  This flag is set to FALSE
        // when a time advance is received.  See SocketInterface_HandleAdvanceTime.
        data->waitingForTimeAdvance = TRUE;
        data->idleSentRealTime = EXTERNAL_QueryRealTime(iface);
    }
}

static void SocketInterface_HandleDelayedMessage(
    EXTERNAL_Interface* iface,
    SocketInterface_Message* message)
{
    char errString[MAX_STRING_LENGTH];

    switch (message->GetType())
    {
        case SocketInterface_MessageType_CreatePlatform:
            SocketInterface_HandleCreatePlatform(
                iface,
                (SocketInterface_CreatePlatformMessage*) message);
            break;

        case SocketInterface_MessageType_UpdatePlatform:
            SocketInterface_HandleUpdatePlatform(
                iface,
                (SocketInterface_UpdatePlatformMessage*) message);
            break;

        case SocketInterface_MessageType_CommEffectsRequest:
        {
            // Check if the socket id exists.
            if (message->HasSocketId())
            {
                SocketInterface_HandleCommEffectsRequest(
                    iface,
                    (SocketInterface_CommEffectsRequestMessage*) message,
                    message->GetSocketId());
            }
            else
            {
                // We have a problem throw an exception.
                sprintf(
                    errString,
                    "Invalid Socket Id \"%d\"",
                    message->GetSocketId());
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSocketId, errString);
            }
            break;
        }

        case SocketInterface_MessageType_PauseSimulation:
            SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Paused);
            break;

        default:
            sprintf(
                errString,
                "Unknown message type \"%d\"",
                message->GetType());
            throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidMessage, errString);
            break;
    }
}

void SocketInterface_HandleWaitForWarmup(EXTERNAL_Interface *iface)
{
    SocketInterface_InterfaceData *data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

#ifdef  DEBUG
    printf ("partition %d: In SocketInterface_HandleWaitForWarmup\n", iface->partition->partitionId);
#endif

    // Keep loouping until we get a BeginWarmup message
    while (data->simulationState == SocketInterface_StateType_Standby)
    {
        // Attempt to receive data from the socket.  If successful then handle
        // that data.
        if (data->incomingMessages != NULL)
        {
            SocketInterface_MessageList* listNode;

            // Retrieve the message from the list
            pthread_mutex_lock(&data->sockets.receiverMutex);
            listNode = (SocketInterface_MessageList*) data->incomingMessages;
            data->incomingMessages = listNode->next;
            if (listNode->next == NULL)
            {
                data->lastIncomingMessage = NULL;
            }
            data->numIncomingMessages--;
            pthread_mutex_unlock(&data->sockets.receiverMutex);

            // If running in real time we don't want to get overloaded with
            // messages.  Let the receiver thread know it can continue
            // receiving if the input queue is empty enough.
            if (data->timeManagement == SocketInterface_TimeManagementType_RealTime &&
                data->numIncomingMessages < SOCKET_INTERFACE_MAX_QUEUE_SIZE * 0.1)
            {
                pthread_cond_signal(&data->sockets.receiverNotFull);
            }

            // Handle the message
            BOOL scheduledMessage = FALSE;
            try
            {
                scheduledMessage = SocketInterface_HandleMessage(iface, listNode->message);
            }
            catch (SocketInterface_Exception& e)
            {
                // Handle the exception
                SocketInterface_HandleException(
                    iface,
                    &e,
                    listNode->message,
                    listNode->message->GetSocketId());
            }

            // Free memory.  Only delete messages that were not scheduled (ie,
            // messages that were already processed)
            if (!scheduledMessage)
            {
                delete listNode->message;
            }
            delete listNode;
        }
    }

#ifdef  DEBUG
    printf ("partition %d: Leaving SocketInterface_HandleWaitForWarmup\n", iface->partition->partitionId);
#endif
}

void SocketInterface_Receive(EXTERNAL_Interface *iface)
{
    SocketInterface_InterfaceData *data;
    EXTERNAL_TreeNode *record;
    //std::list<NodeAddress> addresses; 
    char err[MAX_STRING_LENGTH];

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

#ifdef  DEBUG
//printf ("partition %d: START receive msg\n", iface->partition->partitionId);
#endif
    //printf("STATE %d sim time %f real time = %f horizon = %f\n", data->simulationState, EXTERNAL_QuerySimulationTime(iface) / 1000000000.0, EXTERNAL_QueryRealTime(iface) / 1000000000.0, iface->horizon / 1000000000.0);

    // If in warmup time and we reached the warmup time move to
    // initialized state and exit function.
    if (data->simulationState == SocketInterface_StateType_Warmup && !EXTERNAL_IsInWarmup(iface))
    {
        SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Initialized);
        return;
    }
    else if (EXTERNAL_IsInWarmup(iface) && !data->doWarmup)
    {
        ERROR_ReportError(
            "QualNet socket interface must be configured for warmup if any "
            "external interface uses warmup time.");
    }

    // Handle statistics
    // Print if we are in executing state or 
    // if we are in pause state and advancing sim time in pause state
    if ((data->simulationState == SocketInterface_StateType_Executing ||
         (data->simulationState == SocketInterface_StateType_Paused &&
         data->pauseAdvanceSimTime))
        && !EXTERNAL_IsInWarmup(iface))
    {
        if (data->statsPrintRealTime)
        {
            if (EXTERNAL_QueryRealTime(iface) > data->nextStatsUpdate)
            {
                SocketInterface_PrintStats(iface);
                data->lastStatsUpdateRealTime = EXTERNAL_QueryRealTime(iface);
                data->lastStatsUpdateSimTime = EXTERNAL_QuerySimulationTime(iface);
                while (EXTERNAL_QueryRealTime(iface) > data->nextStatsUpdate)
                {
                    data->nextStatsUpdate += data->statsPrintInterval;
                }
            }
        }
        // Handle graph
        if (data->graphPrintRealTime)
        {
            if (EXTERNAL_QueryRealTime(iface) > data->nextGraphUpdate)
            {
                data->nextGraphUpdate = EXTERNAL_QueryRealTime(iface)
                                        + data->graphPrintInterval;

                SocketInterface_PrintGraph(
                    iface,
                    &data->entityMapping,
                    data->graphLogFile);
            }
        }
    }

    // Attempt to receive data from the socket.  If successful then handle
    // that data.
    while (data->incomingMessages != NULL)
    {
        SocketInterface_MessageList* listNode;

        // Retrieve the message from the list
        pthread_mutex_lock(&data->sockets.receiverMutex);
        listNode = (SocketInterface_MessageList*) data->incomingMessages;
        data->incomingMessages = listNode->next;
        if (listNode->next == NULL)
        {
            data->lastIncomingMessage = NULL;
        }
        data->numIncomingMessages--;
        pthread_mutex_unlock(&data->sockets.receiverMutex);

        // If running in real time we don't want to get overloaded with
        // messages.  Let the receiver thread know it can continue
        // receiving if the input queue is empty enough.
        if (data->timeManagement == SocketInterface_TimeManagementType_RealTime &&
            data->numIncomingMessages < SOCKET_INTERFACE_MAX_QUEUE_SIZE * 0.1)
        {
            pthread_cond_signal(&data->sockets.receiverNotFull);
        }

        // Handle the message
        BOOL scheduledMessage = FALSE;
        try
        {
            scheduledMessage = SocketInterface_HandleMessage(iface, listNode->message);
        }
        catch (SocketInterface_Exception& e)
        {
            // Handle the exception
            SocketInterface_HandleException(
                iface,
                &e,
                listNode->message,
                listNode->message->GetSocketId());
        }

        // Free memory.  Only delete messages that were not scheduled (ie,
        // messages that were already processed)
        if (!scheduledMessage)
        {
            delete listNode->message;
        }
        delete listNode;
    }

    // Look for expired requests
    record = EXTERNAL_TreePeekMin(&data->requestTree);
    while (record != NULL
           && record->time < EXTERNAL_QuerySimulationTime(iface))
    {
        // If its time is expired, extract it
        EXTERNAL_TreeExtractMin(&data->requestTree);

        // If the data is NULL it means this was already processed as a
        // success.
        if (record->data != NULL)
        {
            SocketInterface_PacketData* packetData = (SocketInterface_PacketData*) record->data;
            
            // Update model if running in distributed qualnet mode
            // and this is a non-network protocol packet    
            if ((data->useStatisticalModel) && (packetData->protocol != SocketInterface_ProtocolType_ProtocolNetwork)) 
            {                
                SocketInterface_UnicastModelingParameters params;
                SocketInterface_UnicastModeledCommEffects effects;                

                params.senderNodeAddress = MappingGetSingleNodeAddress(iface->partition,
                                            &data->entityMapping,
                                            packetData->originator); 
                if (params.senderNodeAddress == 0)
                {
                    sprintf(err,
                        "SocketInterface_Receive Invalid Sender node (no node address): %s",
                        packetData->originator.c_str());
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
                }

                params.receiverNodeAddress = MappingGetSingleNodeAddress(iface->partition,
                                            &data->entityMapping,
                                            packetData->receiver); 
                if (params.receiverNodeAddress == 0)
                {
                    sprintf(err,
                        "SocketInterface_Receive Invalid Receiver node (no node address): %s",
                        packetData->receiver.c_str());
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
                }

                params.protocol = packetData->protocol;
                params.sendTime = packetData->beginProcessingTime;
                effects.latency = 0.0;
                effects.success = FALSE; 
                SocketInterface_UpdateUnicastModel(data,&params,&effects);

                // Only send failure response if it has not already been send for this packet
                if (packetData->sentStatisticalModel == FALSE) 
                {
                    SocketInterface_HandleRequestFailure(iface, record);
                }
                else 
                {
                    // Set data to NULL so we know this was freed
                    record->data = NULL;
                    
                    // Remove from request hash
                    SocketInterface_PacketData* packetData2;
                    packetData2 = data->requestHash.PeekRequest(packetData->hashId);
                    assert(packetData2 == packetData);
                    data->requestHash.RemoveHash(packetData->hashId);
                    
                    // Free memory
                    delete packetData;                    
                }

            }

            // Send failure response otherwise
            else 
            {
                SocketInterface_HandleRequestFailure(iface, record);
            }
        }

        EXTERNAL_TreeFreeNode(&data->requestTree, record);

        // Look at the next one
        record = EXTERNAL_TreePeekMin(&data->requestTree);
    }

    // Free expired multicast messages
    record = EXTERNAL_TreePeekMin(&data->multicastTree);
    while (record != NULL
           && record->time < EXTERNAL_QuerySimulationTime(iface))
    {
        // If its time is expired, extract it and look at the next one
        EXTERNAL_TreeExtractMin(&data->multicastTree);

        SocketInterface_PacketData* packetData = (SocketInterface_PacketData*) record->data;

//todo: get rid of this????
        // Update model if running in distributed qualnet mode
        // and this is a non-network protocol packet    
        if ((data->useStatisticalModel) && 
            (packetData != NULL) && 
            (packetData->protocol != SocketInterface_ProtocolType_ProtocolNetwork)) 
        {
            SocketInterface_MulticastModelingParameters params;

            params.senderNodeAddress = MappingGetSingleNodeAddress(iface->partition,
                                            &data->entityMapping,
                                            packetData->originator); 
            if (params.senderNodeAddress == 0)
            {
                sprintf(err,
                    "SocketInterface_Receive Invalid Sender node (no node address): %s",
                    packetData->originator.c_str());
                throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
            }

            params.receiverNodeAddress = MappingGetSingleNodeAddress(iface->partition,
                                        &data->entityMapping,
                                        packetData->receiver); 
            
            if (params.receiverNodeAddress == 0) 
            {
                int numHostBits;
                BOOL isNodeId;
                
                IO_ParseNodeIdHostOrNetworkAddress(
                    packetData->receiver.c_str(),
                    &(params.receiverNodeAddress),
                    &numHostBits,
                    &isNodeId);                
                
                assert(isNodeId == FALSE);
            }

            params.protocol = packetData->protocol;
            params.sendTime = packetData->beginProcessingTime;
            SocketInterface_UpdateMulticastModel(data,&params,packetData->effects);
            
            // Free allocated memory for multicast receivers effects hashmap
            packetData->effects->clear();
            delete packetData->effects;
        }

        // Remove from request hash
        SocketInterface_PacketData* packetData2;
        packetData2 = data->requestHash.PeekRequest(packetData->hashId);
        assert(packetData2 == packetData);
        data->requestHash.RemoveHash(packetData->hashId);

        // Free memory
        EXTERNAL_TreeFreeNode(&data->multicastTree, record);
        delete packetData;

        record = EXTERNAL_TreePeekMin(&data->multicastTree);
    }

#ifdef  DEBUG
    //printf ("partition %d: END receive msg\n", iface->partition->partitionId);
#endif
}

void SocketInterface_Forward(
    EXTERNAL_Interface *iface,
    Node* node,
    void *forwardData,
    int forwardSize)
{
    SocketInterface_InterfaceData *data;
    Int32 requestId;
    SocketInterface_PacketData *packetData;
    SocketInterface_PacketData *treeNodePacketData;
    SocketInterface_CommEffectsResponseMessage* response;
    clocktype totalBoundaryDelay;
    clocktype simTime;
    clocktype cpuTime;
    clocktype interval;
    clocktype realTime;
    int i;
    EXTERNAL_TreeNode* treeNode;
    BOOL sentStatisticalModel;
    BOOL networkPacket = FALSE;
    BOOL multicast = FALSE;
    SocketInterface_UnicastModelingParameters uParams;
    SocketInterface_MulticastModelingParameters mParams;
    SocketInterface_UnicastModeledCommEffects uEffects;
    SocketInterface_MulticastModeledReceiverCommEffects mEffects;

    char* messageData = (char*) forwardData;

    ERROR_Assert(forwardData != NULL, "Invalid data");

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    if (iface->partition->partitionId != 0)
    {
        // Re-forward this compleition to partition 0, no delay
        EXTERNAL_RemoteForwardData(iface, node, forwardData, forwardSize, 0, 0);
        return;
    }

    sentStatisticalModel = FALSE; 

    // If running in distributed qualnet mode, read in additional
    // relevant parameters from message structure
    if (data->useStatisticalModel) 
    {
        memcpy(
            &networkPacket,
            messageData+sizeof(Int32)+sizeof(clocktype),
            sizeof(BOOL));
        
        if (!networkPacket) {
            memcpy(
                &sentStatisticalModel,
                messageData+sizeof(Int32)+sizeof(clocktype) + sizeof(BOOL),
                sizeof(BOOL));
            memcpy(
                &multicast,
                messageData+sizeof(Int32)+sizeof(clocktype) + sizeof(BOOL) + sizeof(BOOL),
                sizeof(BOOL));
            if (multicast) 
            {
                memcpy(
                    &mParams,
                    messageData+sizeof(Int32)+sizeof(clocktype) + sizeof(BOOL) + sizeof(BOOL) + sizeof(BOOL),
                    sizeof(SocketInterface_MulticastModelingParameters));
                mEffects.receivingPlatformAddress = node->partitionData->EXTERNAL_lastIdToInvokeForward;   
                mEffects.sendTime = mParams.sendTime;
            }
            else 
            {
                memcpy(
                    &uParams,
                    messageData+sizeof(Int32)+sizeof(clocktype) + sizeof(BOOL) + sizeof(BOOL) + sizeof(BOOL),
                    sizeof(SocketInterface_UnicastModelingParameters));            
            }            
        }
    }

    // Extract message hash id
    SocketInterface_PacketHeader* header = (SocketInterface_PacketHeader*) messageData;
    requestId = header->hashId;

    // If the request id is not in the hash then this message was already
    // designated as a failure, return and do nothing.
    if (data->requestHash.CheckHash(requestId) == FALSE)
    {
        // Update successful failures statistic and approporiate statistical model
        // if running in distributed qualnet mode
        if (sentStatisticalModel == FALSE) 
        {
            data->successfulFailures++;
        }
        if ((data->useStatisticalModel) && (!networkPacket)) 
        {
            simTime = EXTERNAL_QuerySimulationTime(iface);
            if (multicast)
            {
                simTime -= mParams.sendTime;
                mEffects.effects.latency = ((double)simTime)/SECOND;
                mEffects.effects.success = FALSE;
                SocketInterface_UpdateMulticastModelSingleReceiver(
                     data,
                     &mParams,
                     &mEffects);
            }
            else
            {
                simTime -= uParams.sendTime;
                uEffects.latency = (double) simTime / SECOND;
                uEffects.success = FALSE;
                SocketInterface_UpdateUnicastModel(
                     data,
                     &uParams,
                     &uEffects);
            }
        }
        return;
    }
    else
    {
        // Extract the packet data based on the request id
        packetData = data->requestHash.PeekRequest(requestId);
        treeNode = packetData->treeNode;

        // If not multicast, remove request.  This will allow other
        // multicast messages to come in from other platforms.
        if (!packetData->multicast)
        {
            data->requestHash.RemoveHash(requestId);
        }

        // If the datas don't match then it means the node has been
        // a failure, but the tree node has been reused for a different
        // comm request
        treeNodePacketData = (SocketInterface_PacketData*) treeNode->data;
        assert(treeNode->data == packetData);
    }

    // Calculate statistics
    packetData->finishProcessingTime =
        EXTERNAL_QuerySimulationTime(iface);
    packetData->finishProcessingRealTime =
        EXTERNAL_QueryRealTime(iface);
    packetData->finishProcessingCPUTime =
        EXTERNAL_QueryCPUTime(iface);

    simTime = packetData->finishProcessingTime
              - packetData->beginProcessingTime;
    realTime = packetData->finishProcessingRealTime
               - packetData->beginProcessingRealTime;
    cpuTime = packetData->finishProcessingCPUTime
              - packetData->beginProcessingCPUTime;

    // Calculate real time, taking boundary packets into account
    if (packetData->beginProcessingTime >= data->timeAllowance)
    {
        /*printf("begin = %8.4f end =
             %8.4f rt = %8.4f cpu =
             %8.4f speed = %6.2f\n",
               (double) packetData->beginProcessingTime / SECOND,
               (double) packetData->finishProcessingTime / SECOND,
               (double) realTime / SECOND,
               (double) cpuTime / SECOND,
               (double) realTime / simTime);*/
    }
    else
    {
        /*printf("boundary packet begin = %6.2f end = %6.2f rt = %6.2f "
               "time = %6.2f\n",
               (double) packetData->beginProcessingTime / SECOND,
               (double) packetData->finishProcessingTime / SECOND,
               (double) realTime / SECOND,
               (double) data->federationTime / SECOND);*/
        if (packetData->boundary >= data->firstBoundary)
        {
            totalBoundaryDelay = 0;
            for (i = packetData->boundary + 1; i <= data->lastBoundary; i++)
            {
                totalBoundaryDelay +=
                    data->boundaryTimes[i - data->firstBoundary];
            }
            /*printf("boundary = %d, total delay = %f\n",
                packetData->boundary,
               (double) totalBoundaryDelay / SECOND);*/

            realTime -= totalBoundaryDelay;
        }
        else
        {
            /*printf("boundary = %d out of range = %d\n",
                 packetData->boundary,
                     data->firstBoundary);*/
        }
    }

    // Option A occurs if the process was swapped out, and the measured
    // cpu time is a reliable value
    if (cpuTime < realTime && cpuTime != 0)
    {
        if (data->printPerPacketStats)
        {
            fprintf(data->statsFile, "A\t");
        }

        interval = cpuTime;
    }
    // Option B occurs if the process was swapped out, and the measured
    // cpu time is an UNreliable value, but we know that the elapsed real
    // time was greater than the cpu timing interval.
    else if (cpuTime < realTime && realTime > iface->cpuTimingInterval)
    {
        if (data->printPerPacketStats)
        {
            fprintf(data->statsFile, "B\t");
        }

        interval = iface->cpuTimingInterval;
    }
    // Otherwise we just use real time
    else
    {
        if (data->printPerPacketStats)
        {
            fprintf(data->statsFile, "C\t");
        }

        interval = realTime;
    }

    // Use when simTime is greater than 1ms.  For other values it will
    // be too inaccurate.  This is only used for display purposes so it is
    // okay to do this.
    if (simTime > 1 * MILLI_SECOND)
    {
        data->totalPackets++;
        data->totalRealTimeSpeed += (double) interval / simTime;
    }

    if (data->printPerPacketStats)
    {
        fprintf(data->statsFile, "Packet\t%f\t%f\t%f\t%f\t%f\n",
                                 (double) cpuTime / SECOND,
                                 (double) iface->cpuTimingInterval / SECOND,
                                 (double) realTime / SECOND,
                                 (double) simTime / SECOND,
                                 (double) interval / simTime);
    }

#ifdef DEBUG
    printf("packet processed %f sim time %f real time\n",
           (packetData->finishProcessingTime
               - packetData->beginProcessingTime) / 1000000000.0,
           (packetData->finishProcessingRealTime
               - packetData->beginProcessingRealTime) / 1000000000.0);
#endif

    // Send response and update statistics if 
    // it hasn't already been sent based on the model
    if (sentStatisticalModel == FALSE) 
    {
        // Send a Successful Response
        std::string receiver;
        std::string originator;
        SocketInterface_TimeType latency;

        latency = (double) EXTERNAL_QuerySimulationTime(iface) - packetData->beginProcessingTime;
        latency = latency / SECOND;
        
        originator = packetData->originator;
        receiver = packetData->receiver;

        EntityData* mapping = NULL;         
        
        if (packetData->multicast) // --> print IP of receiving node
        {
            // If it is multicast, get the entityId and multicast owner for this
            // node id.  If no multicast owner then send response to packet
            // originator.            
            mapping = EntityMappingLookup(&data->entityMapping, 
                node->partitionData->EXTERNAL_lastIdToInvokeForward);
            //receiver = mapping->entityId; 
            
            //if (mapping->nodes.size() > 1)
            //{
                NodeAddress nadd = MAPPING_GetDefaultInterfaceAddressFromNodeId(
                    node, 
                    node->nodeId); 
                if (nadd != INVALID_MAPPING)
                {
                    char* buf = new char[MAX_STRING_LENGTH]; 
                    IO_ConvertIpAddressToString(nadd, buf);
                    receiver = buf;                                     
                } 
            //}
#ifdef DEBUG            
            std::cout << "SocketInterface_Forward packetData->receiver = "
                << packetData->receiver << std::endl; 
            std::cout << "SocketInterface_Forward node->nodeId = "
                << node->nodeId << std::endl;  
            std::cout << "SocketInterface_Forward mapping->entityId = "
                << mapping->entityId << std::endl; 
            std::cout << "SocketInterface_Forward receiver = "
                << receiver << std::endl; 
#endif
        }

        response = new
            SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                            packetData->id2,
                                            &originator,
                                            &receiver,
                                            SocketInterface_Status_Success,
                                            (double) SocketInterface_Timestamp(iface) / SECOND,
                                            latency);
        response->SetDescription(packetData->description);

        BOOL sent = FALSE;
        if (packetData->multicast)
        {
            if (data->sourceResponsibleMulticast)
            {
                if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
                {
                    SocketInterface_SendMessage(iface,
                                     response,
                                     packetData->socketId);
                    
                    data->responses++;
                    data->successes++;
                    sent = TRUE;
                }
            }
            else
            {
                std::map<NodeId, Entity_NodeData>::iterator entityit; 
                for (entityit = mapping->nodes.begin(); entityit != mapping->nodes.end(); entityit++)
                {
                    Entity_NodeData entity = entityit->second; 

                    std::vector<MulticastGroup>::iterator it;
                    
                    // If multicast find the multicast group.  If found, send a copy of
                    // the successfull comm repsonse to each socket connection that is
                    // subscribing to multicast messages.
                    for (it = entity.multicastGroups.begin();
                         it != entity.multicastGroups.end();
                         it++)
                    {
                        if (it->group == packetData->receiver)
                        {
                            for (i = 0; i < (int)it->socketSubscribers.size(); i++)
                            {
                                if (EXTERNAL_SocketValid(data->sockets.connections[it->socketSubscribers[i]]))
                                {
                                    SocketInterface_SendMessage(iface,
                                                     response,
                                                     it->socketSubscribers[i]);
                                    sent = TRUE;
                                    
                                    data->responses++;
                                    data->successes++;
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }

        // If the packet was not sent to a multicast owner then send it to the
        // request originator.  This will be the case with all unicast packets
        // and may occur with some broadcast/multicast messages.
        if (!sent)
        {
            if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
            {
                SocketInterface_SendMessage(iface,
                                 response,
                                 packetData->socketId);
                
                data->responses++;
                data->successes++;
            }
            
            if (!packetData->multicast)
            {
                // Don't free the tree node yet.  Allow it to be removed when it is the
                // earlist record.  But set its data to NULL so we know it was sent
                // already.
                treeNode->data = NULL;
                
                // Free memory.  Note that the forwardData does not need to be freed
                // since this is part of the message.  The packet will be removed from
                // the tree in SocketInterface_Receive.
                delete packetData;
            }
        }
        
        std::string str;
        response->Print(&str);
        str += "\n";

        SocketInterface_PrintLog(
            iface,
            data->responsesLogFile,
            str.c_str());
        
        // delete the message
        delete response;
    }
    // Otherwise just set tree node's data to null and free the packetData structure
    else 
    {
        if (!packetData->multicast)
        {
            treeNode->data = NULL;
            delete packetData;
        }   
    }

    // If running in distributed qualnet mode and if this is not a network level packet
    // update the appropriate model
    if ((data->useStatisticalModel) && (!networkPacket)) {
        
        if (multicast) 
        {
            mEffects.effects.latency = ((double)simTime)/SECOND;
            mEffects.effects.success = TRUE;
            // Update the packetData's temp hashmap with this value
            SocketInterface_UpdateTempMulticastModelSingleReceiver(
                packetData,
                &mEffects);
        }
        else 
        {
            uEffects.latency = ((double)simTime)/SECOND;
            uEffects.success = TRUE;
            SocketInterface_UpdateUnicastModel(
                 data,
                 &uParams,
                 &uEffects);
            
        }
    }

    // If idle-when-response is set and not running in real-time mode then
    // go into idle mode
    if (data->idleWhenResponseSent && data->timeManagement != SocketInterface_TimeManagementType_RealTime)
    {
        SocketInterface_SimulationIdleMessage* idle;

        // Create message and send it to all connections
        idle = new SocketInterface_SimulationIdleMessage(
            (double) SocketInterface_Timestamp(iface) / SECOND);

#ifdef DEBUG
        printf("Sent response, now going idle\n");
#endif

        iface->horizon = EXTERNAL_QuerySimulationTime(iface);
        ERROR_Assert(!data->waitingForTimeAdvance, "should not be idle");

        // Send a ReportSimulationIdleState message to every connected
        // client
        for (i = 0; i < (int)data->sockets.connections.size(); i++)
        {
            if (data->sockets.activeConnections[i])
            {
                SocketInterface_SendMessage(iface, idle, i);
            }
        }

        // Print to the log file
        std::string str;
        idle->Print(&str);
        str += "\n";

        SocketInterface_PrintLog(
            iface,
            data->responsesLogFile,
            str.c_str());

        // Set the sent idle message flag.  This flag is set to FALSE when a
        // time advance is received.
        data->waitingForTimeAdvance = TRUE;
        data->inIdleResponse = TRUE;
        data->idleSentRealTime = EXTERNAL_QueryRealTime(iface);

        // delete the idle message
        delete idle;
    }
}

void SocketInterface_HandleCommEffectsReceiver(EXTERNAL_Interface *iface,
    Node* sendNode,
    SocketInterface_CommEffectsRequestMessage *message,
    int socketId,
    clocktype delay)
{
    SocketInterface_InterfaceData *data;
    Node *node;
    NodeAddress nodeAddress;
    char receiverString[MAX_STRING_LENGTH];
    char s[MAX_STRING_LENGTH];
    char err[MAX_STRING_LENGTH];
    char errString[10*MAX_STRING_LENGTH];
    char *receiverCh;
    char *sCh;
    int numNodes = 0;

    data = (SocketInterface_InterfaceData*) iface->data;

    strcpy(receiverString, message->m_ReceiverId.c_str());
    receiverCh = receiverString;

    // This function parses the receiver field, which looks something like
    // "102|103|105|19".  It operates by copying characters one at a time
    // into the string s.  Once a | or NULL is hit it will process what is
    // currently in s, reset s, and continue parsing.

    sCh = s;
    while (1)
    {
        // Copy a number
        *sCh++ = *receiverCh++;

        // If the next character is a | or NULL then process s
        if (*receiverCh == '|' || *receiverCh == 0)
        {
            // Terminate s
            *sCh = 0;

            node = MappingGetSingleNodePointer(
                iface->partition,
                &data->entityMapping,
                (std::string)s);
            if (node == NULL)
            {
                // The string is not a valid entity.  Check if it is an IP
                // address of a node.
                BOOL badAddress = FALSE;

#ifdef _WIN32 // windows
                // Convert to string using inet_addr
                nodeAddress = inet_addr(s);
                if (nodeAddress != INADDR_NONE)
                {
                    EXTERNAL_ntoh(&nodeAddress, sizeof(NodeAddress));
                }
#else // unix/linux
                int err;
                // Convert to string using inet_aton
                err = inet_aton(s, (in_addr*) &nodeAddress);
                if (err == 0)
                {
                    badAddress = TRUE;
                }
                else
                {
                    nodeAddress = ntohl(nodeAddress);
                }
#endif

                // Check if the string is an invalid address.  If it is a
                // valid address and not a multicast address, make sure it
                // is a valid IP address of a QualNet node.  Multicast
                // addresses are valid destination addresses.
                if (badAddress ||
                    (nodeAddress == ANY_ADDRESS && 
                    message->m_ReceiverId.compare("255.255.255.255") != 0))
                {
                    sprintf(
                        errString,
                        "Unknown Receiver node %s",
                        message->m_ReceiverId.c_str());
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidReceiverId, errString);
                }
                else if (!(nodeAddress == ANY_ADDRESS
                    || NetworkIpIsMulticastAddress(iface->partition->firstNode, nodeAddress)))
                {
                    NodeAddress nodeId;
                    nodeId = MAPPING_GetNodeIdFromInterfaceAddress(
                        iface->partition->firstNode,
                        nodeAddress);

                    if (nodeId == INVALID_MAPPING)
                    {
                        // It's not an IP address.  Throw an exception.
                        sprintf(
                            errString,
                            "IP address %s does not map to a platform",
                            s);
                        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidReceiverId, errString);
                    }
                }
            }
            else
            {
                nodeAddress = MappingGetSingleNodeAddress(iface->partition,
                                            &data->entityMapping,
                                            (std::string)s); 
                if (nodeAddress == 0)
                {
                    sprintf(err,
                        "SocketInterface_HandleCommEffectsReceiver Invalid Sender node (no node address): %s",
                        s);
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidSenderId, err);
                }
            }

            numNodes++;

            // We have a receiver, now send the message
            SocketInterface_SendCommEffectsRequest(
                iface,
                sendNode,
                message,
                nodeAddress,
                socketId,
                delay);

            // If this is the end of the string then break
            if (*receiverCh == 0)
            {
                break;
            }

            // Otherwise continue parsing
            receiverCh++;
            sCh = s;
        }
    }

    if (numNodes == 0)
    {
        sprintf(errString,
            "ERER: O:%s C:%s",
            message->m_Id1,
            message->m_Id2);
        throw SocketInterface_Exception(SocketInterface_ErrorType_EmptyReceiverList, errString);
    }
}

void SocketInterface_Finalize(EXTERNAL_Interface *iface)
{
    SocketInterface_InterfaceData *data;
    unsigned i;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    if (data->totalPackets > 0)
    {
        printf("QualNet average transactional real-time (per packet) = %f\n",
            data->totalRealTimeSpeed / data->totalPackets);
    }
    else
    {
        printf("QualNet average transactional real-time (per packet) = n/a "
               "(no packets)\n");
    }

    // Send the change of Simulation state message to stopping or resetting
    if (data->resetting)
    {
        SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Resetting);
    }
    else
    {
        SocketInterface_ChangeSimulationState(iface, SocketInterface_StateType_Stopping);
    }

    // Wait on second for all buffers to send
    EXTERNAL_Sleep(1 * SECOND);

    // Empty the buffers
    for (i = 0; i < data->sockets.connections.size(); i++)
    {
        // check the socket is valid
        if (data->sockets.activeConnections[i])
        {
            // Send out all the messages in the outgoing list.
            pthread_mutex_lock(&data->sockets.senderMutex);
            while (data->outgoingMessages != NULL)
            {
                pthread_cond_wait(
                    &data->sockets.senderNotFull,
                    &data->sockets.senderMutex);
            }
            pthread_mutex_unlock(&data->sockets.senderMutex);
        }

    }

    // Change the state to shut down. At this point we have cleaned
    // up the sender thread buffer and the outgoing message list.
    data->simulationState = SocketInterface_StateType_Shutdown;

    // Close log files the data
    if (data->driverLogFile != NULL)
    {
        fclose(data->driverLogFile);
    }

    if (data->responsesLogFile != NULL)
    {
        fclose(data->responsesLogFile);
    }

    if (data->errorsLogFile != NULL)
    {
        fclose(data->errorsLogFile);
    }

    if (data->statsLogFile != NULL)
    {
        fclose(data->statsLogFile);
    }

    if (data->graphLogFile != NULL)
    {
        fclose(data->graphLogFile);
    }

    // Check which partition are you on
    if (iface->partition->partitionId == 0)
    {
        // Close all sockets
        for (i = 0; i < (unsigned)data->sockets.numPorts; i++)
        {
            if (EXTERNAL_SocketValid(data->sockets.listeningSockets[i]))
            {
                EXTERNAL_SocketClose(data->sockets.listeningSockets[i]);
            }
        }

        for (i = 0; i < data->sockets.connections.size(); i++)
        {
            if (EXTERNAL_SocketValid(data->sockets.connections[i]))
            {
                EXTERNAL_SocketClose(data->sockets.connections[i]);
            }
        }

        // Wait for the sender and receiver threads to finish.  Signal that
        // the sender has work to do so it can exit.  It is most likely
        // currently waiting on this signal.
        pthread_cond_signal(&data->sockets.senderNotEmpty);
        pthread_join(data->sockets.senderThread, NULL);
        pthread_join(data->sockets.receiverThread, NULL);

        // Create new QualNet process if resetting
        if (data->resetting)
        {
            // Get command line
            std::vector<std::string>* command = (std::vector<std::string>*)
                PARTITION_ClientStateFind(
                iface->partition,
                "CesCommandLine");

            if (command == NULL)
            {
                ERROR_ReportError("Invalid command line, cannot reset");
            }

            ExecNewProcess(*command);
        }
        // Finalize model structures
        SocketInterface_FinalizeModels(data);
    }
}

static void NodePathToPlatformPath(
    EXTERNAL_Interface* iface,
    std::string* nodePath,
    std::string* platformPath)
{
    SocketInterface_InterfaceData* data;
    std::vector<std::string> tokens;
    NodeAddress nodeId;
    std::string entity;

    data = (SocketInterface_InterfaceData*) iface->data;

    StringSplit(
        nodePath,
        "/",
        &tokens);
    if (tokens.size() < 3)
    {
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidDynamicCommand, "invalid platform path from remote partition");
    }

    nodeId = atoi(tokens[1].c_str());
    entity = MappingGetEntityId(
        &data->entityMapping,
        nodeId);
    if (entity == "")
    {
        throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidDynamicCommand, "invalid platform path entity from remote partition");
    }
    tokens[0] = "/platform";
    tokens[1] = entity;
    *platformPath = tokens[0];
    for (unsigned i = 1; i < tokens.size(); i++)
    {
        *platformPath += "/" + tokens[i];
    }
}

static void SocketInterface_ProcessExternalMessage(
    EXTERNAL_Interface* iface,
    Message* msg)
{
    SocketInterface_SerializedMessage serialized;
    SocketInterface_Message* message = NULL;
    SocketInterface_Message* response = NULL;
    UInt32 socketId;
    BOOL isPlatformPath;

    // Extract serialized message from packet.  The first 4 bytes of the
    // packet are the socketId.  The remainder are the message.
    memcpy(
        &socketId,
        MESSAGE_ReturnPacket(msg),
        sizeof(UInt32));
    serialized.Resize(MESSAGE_ReturnPacketSize(msg) - sizeof(UInt32) - sizeof(BOOL));
    memcpy(
        &isPlatformPath,
        MESSAGE_ReturnPacket(msg) + sizeof(UInt32),
        sizeof(BOOL));
    memcpy(
        serialized.m_Data,
        MESSAGE_ReturnPacket(msg) + sizeof(UInt32) + sizeof(BOOL),
        serialized.m_Size);
    message = serialized.Deserialize();
    message->SetSocketId(socketId);

    switch (message->GetType())
    {
        case SocketInterface_MessageType_DynamicCommand:
        {
            SocketInterface_DynamicCommandMessage* dynamic = (SocketInterface_DynamicCommandMessage*) message;

            std::string result;
            char errString[MAX_STRING_LENGTH];
            D_Hierarchy* h = &iface->partition->dynamicHierarchy;

            //h->Print();
            // Check the type for the command
            switch(dynamic->m_Type)
            {
                case SocketInterface_OperationType_Read:
                {
                    try
                    {
                        h->ReadAsString(
                            dynamic->m_Path.c_str(),
                            result);

                        // Need to send a response back to the socket
                        response = new SocketInterface_DynamicResponseMessage(
                            dynamic->m_Type,
                            &dynamic->m_Path,
                            &dynamic->m_Args,
                            &result);
                    }
                    catch (D_Exception& d)
                    {
                        // We have an exception.
                        std::string err = d.GetError();
                        SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(
                            SocketInterface_ErrorType_InvalidDynamicCommand,
                            &err);
                        error->SetMessage(message);
                        response = error;
                    }
                    break;

                }

                case SocketInterface_OperationType_Write:
                {
                    try
                    {
                        h->WriteAsString(
                            dynamic->m_Path.c_str(),
                            dynamic->m_Args.c_str());
                    }
                    catch (D_Exception& d)
                    {
                        // We have an exception.
                        std::string err = d.GetError();
                        SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(
                            SocketInterface_ErrorType_InvalidDynamicCommand,
                            &err);
                        error->SetMessage(message);
                        response = error;
                    }
                    break;
                }

                case SocketInterface_OperationType_Execute:
                {
                    try
                    {
                        h->ExecuteAsString(
                            dynamic->m_Path.c_str(),
                            dynamic->m_Args.c_str(),
                            result);

                        // Send a response for the command above
                        response = new SocketInterface_DynamicResponseMessage(
                            dynamic->m_Type,
                            &dynamic->m_Path,
                            &dynamic->m_Args,
                            &result);
                    }
                    catch (D_Exception& d)
                    {
                        // We have an exception.
                        std::string err = d.GetError();
                        SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(
                            SocketInterface_ErrorType_InvalidDynamicCommand,
                            &err);
                        error->SetMessage(message);
                        response = error;
                    }
                    break;
                }

                default:
                    sprintf(
                        errString,
                        "Unknown operation type \"%d\"",
                        dynamic->m_Type);
                    throw SocketInterface_Exception(SocketInterface_ErrorType_InvalidOperationType, errString);
                    break;

            }

            // If we have a response (either a DynamicResult or Error) send
            // it back to partition 0
            if (response != NULL)
            {
                response->SetSocketId(message->GetSocketId());
                SocketInterface_SendMessageToOtherPartition(
                    iface,
                    response,
                    isPlatformPath,
                    0);
                delete response;
            }
            break;
        }

        // Send responses and errors back through the socket
        case SocketInterface_MessageType_DynamicResponse:
            if (isPlatformPath)
            {
                SocketInterface_DynamicResponseMessage* response = (SocketInterface_DynamicResponseMessage*) message;

                std::string platformPath;
                NodePathToPlatformPath(iface, &response->m_Path, &platformPath);

                // Create new response message using the platform path
                SocketInterface_DynamicResponseMessage* newResponse;
                newResponse = new SocketInterface_DynamicResponseMessage(
                    response->m_Type,
                    &platformPath,
                    &response->m_Args,
                    &response->m_Output);
                newResponse->SetSocketId(response->GetSocketId());
                SocketInterface_SendMessage(
                    iface,
                    newResponse,
                    newResponse->GetSocketId());
                delete newResponse;
            }
            else
            {
                SocketInterface_SendMessage(
                    iface,
                    message,
                    message->GetSocketId());
            }
            break;

        // Send responses and errors back through the socket
        case SocketInterface_MessageType_Error:
            SocketInterface_SendMessage(
                iface,
                message,
                message->GetSocketId());
            break;

        default:
        {
            std::string s;
            message->Print(&s);
            printf("Unknown remote message %s\n", s.c_str());
            break;
        }
    }

    delete message;
}

void SocketInterface_ProcessEvent(
    Node* node,
    Message* message)
{
    SocketInterface_Message* delayedMessage;
    EXTERNAL_Interface* iface;
    SocketInterface_InterfaceData* data;
    char errString[MAX_STRING_LENGTH];

    iface = node->partitionData->interfaceTable[EXTERNAL_SOCKET];
    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;

    switch (message->eventType)
    {
        case MSG_EXTERNAL_SOCKET_INTERFACE_DelayedMessage:
            memcpy(&delayedMessage, MESSAGE_ReturnInfo(message), sizeof(SocketInterface_Message*));
            try
            {
                SocketInterface_HandleDelayedMessage(
                    iface,
                    delayedMessage);
            }
            catch (SocketInterface_Exception& e)
            {
                SocketInterface_HandleException(iface, &e, delayedMessage, delayedMessage->GetSocketId());
            }

            delete delayedMessage;
            break;

        case MSG_EXTERNAL_SOCKET_INTERFACE_MulticastGroup:
        {
            // Decode message info with join/leave groups as follows
            // 4 bytes: size of join string (N)
            // 4 bytes: size of leave string (M)
            // N + 1 bytes: join string + trailing 0
            // M + 1 bytes: leave string + trailing 0

            UInt32 n;
            UInt32 m;
            std::string join;
            std::string leave;
            std::vector<std::string> groups;
            NodeAddress group;
            BOOL isNodeId;
            int hostBits;
            unsigned i;
            char* info = MESSAGE_ReturnInfo(message);

            memcpy(&n, info, sizeof(UInt32));
            info += sizeof(UInt32);
            memcpy(&m, info, sizeof(UInt32));
            info += sizeof(UInt32);

            if (n > 0)
            {
                // Extract list of join groups from info message
                join = info;
                info += n + 1;

                StringSplit(&join, " ", &groups);

                // Join each group, if it's a valid multicast group
                for (i = 0; i < groups.size(); i++)
                {
                    IO_ParseNodeIdHostOrNetworkAddress(
                        groups[i].c_str(),
                        &group,
                        &hostBits,
                        &isNodeId);
                    // Address is valid multicast group, verified in
                    // SocketInterface_HandleMulticastChange

                    NetworkIpJoinMulticastGroup(node, group, 0);
                }
            }

            if (m > 0)
            {
                // Extract list of leave groups
                leave = info;
                info += m + 1;

                StringSplit(&leave, " ", &groups);

                // Leave each group, if it's a valid multicast group
                for (i = 0; i < groups.size(); i++)
                {
                    IO_ParseNodeIdHostOrNetworkAddress(
                        groups[i].c_str(),
                        &group,
                        &hostBits,
                        &isNodeId);
                    // Address is valid multicast group, verified in
                    // SocketInterface_HandleMulticastChange


                    NetworkIpLeaveMulticastGroup(node, group, 0);
                }
            }
            break;
        }

        case MSG_EXTERNAL_RemoteMessage:
        {
            SocketInterface_ProcessExternalMessage(iface, message);
            break;
        }

#ifdef TEST_EXTERNAL_REMOTESEND
        case MSG_EXTERNAL_RemoteTest:
        {
            SocketInterface_ExternalPingInfo * pingInfo;
            pingInfo = (SocketInterface_ExternalPingInfo *) MESSAGE_ReturnInfo (message);
            printf ("At %lld p%d we've received an QualNet remoteTest message from src %d num %d\n",
                node->partitionData->theCurrentTime,
                node->partitionData->partitionId,
                pingInfo->sourcePartition,
                pingInfo->msgNumber);
            break;
        }
#endif

        case MSG_EXTERNAL_SOCKET_INTERFACE_StatsLogTimer:
        {
            if (data->simulationState == SocketInterface_StateType_Executing ||
                (data->simulationState == SocketInterface_StateType_Paused &&
                data->pauseAdvanceSimTime))
            {
                SocketInterface_PrintStats(iface);
            }
            
            MESSAGE_Send(node, 
                MESSAGE_Duplicate(node, message), 
                data->statsPrintInterval);
            break;
        }

        case MSG_EXTERNAL_SOCKET_INTERFACE_GraphLogTimer:
        {
            if (data->simulationState == SocketInterface_StateType_Executing ||
                (data->simulationState == SocketInterface_StateType_Paused &&
                data->pauseAdvanceSimTime))
            {
                SocketInterface_PrintGraph(
                    iface,
                    &data->entityMapping,
                    data->graphLogFile);
            }
                    
            MESSAGE_Send(node, 
                MESSAGE_Duplicate(node, message), 
                data->graphPrintInterval);
            break;
        }

        default:
            sprintf(errString, "Unknown QualNet event type %d", message->eventType);
            ERROR_ReportWarning(errString);
            break;
    }

    MESSAGE_Free(node, message);
}

/*
 * FUNCTION     SocketInterface_HandlePreInitializeException
 * PURPOSE      Handles exception that occured during
 *              pre-initialization stage of QualNet.
 *
 * Parameters:
 *    exception: the error that happened.
 *    message:  serialized message that caused the exception.
 *    socket:   socket used by QualNet.
 */
static void SocketInterface_HandlePreInitializeException(
    SocketInterface_Exception* exception,
    SocketInterface_SerializedMessage* message,
    EXTERNAL_Socket* socket)
{
     std::string str;
     EXTERNAL_SocketErrorType err;
     SocketInterface_SerializedMessage* sendMessage;

    // Create an error message.
    SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(exception->GetError(), &exception->GetErrorString());
    error->SetSerializedMessage(message);

    // check if the socket is valid.
    if (EXTERNAL_SocketValid(socket))
    {
        // Serialize the message.
        // Send the data back to the external interface.
        sendMessage = error->Serialize();
        err = EXTERNAL_SocketSend(socket,
                                  (char*) sendMessage->m_Data,
                                  sendMessage->m_Size);

        delete sendMessage;

    }

    delete error;
}

/*
 * FUNCTION     SocketInterface_HandlePreInitializeException
 * PURPOSE      Handles exception that occured during
 *              pre-initialization stage of QualNet.
 *
 * Parameters:
 *    exception: the error that happened.
 *    message:   message that caused the exception.
 *    socket:   socket used by QualNet.
 */
static void SocketInterface_HandlePreInitializeException(
    SocketInterface_Exception* exception,
    SocketInterface_Message* message,
    EXTERNAL_Socket* socket)
{
     std::string str;
     EXTERNAL_SocketErrorType err;
     SocketInterface_SerializedMessage* sendMessage;

    // Create an error message.
    SocketInterface_ErrorMessage* error = new SocketInterface_ErrorMessage(exception->GetError(), &exception->GetErrorString());
    error->SetMessage(message);

    // check if the socket is valid.
    if (EXTERNAL_SocketValid(socket))
    {
        // Serialize the message.
        // Send the data back to the external interface.
        sendMessage = error->Serialize();
        err = EXTERNAL_SocketSend(socket,
                                  (char*) sendMessage->m_Data,
                                  sendMessage->m_Size);

        delete sendMessage;
    }


    delete error;
}

/*
 * FUNCTION     SocketInterface_HandlePreInitializeSimulationMessage
 * PURPOSE      Handles the initializeSimulation message
 *              received at preinitialization stage.
 *
 * Parameters:
 *  message:        serialized message that caused the exception.
 *  scenarioFile:   name of the configuration file.
 */
static void SocketInterface_HandlePreInitializeSimulationMessage(
    SocketInterface_Message* message,
    char* scenarioFile)
{
    char errString[MAX_STRING_LENGTH];

    // Get the scenario string
    SocketInterface_InitializeSimulationMessage* initialize;
    initialize = (SocketInterface_InitializeSimulationMessage*) message;

    if (strcmp(initialize->m_Scenario.c_str(), "") == 0)
    {
        // Scenario string is missing, Throw an exception.
        sprintf(errString, "Empty Scenario String for InitializeMessage");
        throw SocketInterface_Exception(SocketInterface_ErrorType_EmptyScenarioString, errString);
    }
    else
    {
        // We have a scenario for qualnet.
        // Copy the string into a file.
        // Extract the name of the file

        char* token = NULL;
        char iotoken[MAX_STRING_LENGTH];
        token = IO_GetDelimitedToken(iotoken, initialize->m_Scenario.c_str(), " \t\n", NULL);
        token = IO_GetDelimitedToken(iotoken, iotoken, "<>", NULL);

        // Now we have the file name.
        FILE* fd;
        fd = fopen(iotoken, "wt");
        strcpy(scenarioFile, iotoken);
        if (fd == NULL)
        {
            ERROR_ReportError("Error: Unable to open the configuration file!\n");
        }
        fseek(fd, 0, SEEK_SET);
        fputs(initialize->m_Scenario.c_str(), fd);
        fclose(fd);
    }
}

/*
 * FUNCTION     SocketInterface_PreInitializeWaitingData
 * PURPOSE      Blocks on the socket until data is
 *              available.
 *
 * Parameters:
 *  connectSocket:  Socket it blocks on.
 *  avail:          Boolean value.
 */
static void SocketInterface_PreInitializeWaitingData(
    EXTERNAL_Socket* connectSocket,
    bool* avail)
{
    // Check to see if data is available
    while (!*avail)
    {
        EXTERNAL_SocketDataAvailable(
            connectSocket,
            avail);
    }
}

/*
 * FUNCTION     SocketInterface_PreInitialize
 * PURPOSE      Handles the preinitialization process
 *              This function allocates memory for the
 *              socket, and then stores the data structures for
 *              further use in the future, and creates the scenario
 *              from the scenario string sent using the socket.
 *
 * Parameters:
 *  partition:      Pointer to the partition data structure.
 *  ports:          Ports of listening sockes, ports[i] is sockets[i]
 *  sockets:        Vector of listening sockets
 *  fileString:     Stores the scenario filename.
 */
void SocketInterface_PreInitialize(
    PartitionData* partition,
    std::vector<int>* ports,
    std::vector<EXTERNAL_Socket*>* sockets,
    char* fileString)
{
    char errString[MAX_STRING_LENGTH];
    int err = 0;
    BOOL gotScenario = FALSE;
    std::vector<EXTERNAL_Socket*>* connections;
    std::vector<SocketInterface_Message*>* messages;
    bool avail;
    unsigned i;
    SocketInterface_Message* message;
    fd_set readSet;
    int max;
    timeval t;
    EXTERNAL_Socket* connectSocket = NULL;
    EXTERNAL_SocketErrorType socketErr;
    SocketInterface_SerializedMessage data;
    //WSADATA gWsaData;

    connections = new std::vector<EXTERNAL_Socket*>;
    messages = new std::vector<SocketInterface_Message*>;

    printf ("Waiting for an InitializeSimulation message in bootstrap mode\n");

    // Save socket variables for later use
    PARTITION_ClientStateSet(partition,
        "Socket_ListeningSockets",
        (void*) sockets);
    PARTITION_ClientStateSet(partition,
        "Socket_ListeningPorts",
        (void*) ports);
    PARTITION_ClientStateSet(partition,
        "Socket_Connections",
        (void*) connections);
    PARTITION_ClientStateSet(partition,
        "Socket_Messages",
        (void*) messages);

    while (!gotScenario)
    {
        // Create set of all input sockets
        FD_ZERO(&readSet);
        max = -1;
        for (i = 0; i < sockets->size(); i++)
        {
            FD_SET(
                (unsigned int) (*sockets)[i]->socketFd,
                &readSet);
            if ((*sockets)[i]->socketFd > max)
            {
                max = (*sockets)[i]->socketFd;
            }
        }

        // Select for 1 ms
        t.tv_sec = 0;
        t.tv_usec = 1000;

        // Select for readable sockets
        err = select(max + 1, &readSet, NULL, NULL, &t);
        if (err == -1)
        {
#ifndef _WIN32
            // In unix EINTR is interrupted system call.  This means that the
            // select call did not finish waiting which is not an error,
            // so set available to false and return no error.
            if (errno == EINTR)
            {
                continue;
            }

            perror("Error calling select");
            continue;
#endif
        }

        // Now see which sockets have new connection
        for (i = 0; i < sockets->size(); i++)
        {
            //cout << "Socket fd is " << socketData->sockets.listeningSockets[i]->socketFd << endl;
            if (FD_ISSET((
                unsigned int) (*sockets)[i]->socketFd,
                &readSet))
            {
                // Now to accept a connection
                // Initialize the connection socket, make it blocking and threaded.
                connectSocket = new EXTERNAL_Socket;
                EXTERNAL_SocketInit(connectSocket, TRUE, TRUE);
                socketErr = EXTERNAL_SocketAccept((*sockets)[i], connectSocket);
                if (socketErr != EXTERNAL_NoSocketError)
                {
                    sprintf(
                        errString,
                        "Unable to accept socket on port %d",
                        (*ports)[i]);
                    ERROR_ReportError(errString);
                }

                connections->push_back(connectSocket);

                printf("Accepted a new connection in bootstrap mode\n");

                // Send standby message
                SocketInterface_SimulationStateMessage* standby =
                    new SocketInterface_SimulationStateMessage(SocketInterface_StateType_Standby, SocketInterface_StateType_Standby);
                SocketInterface_SerializedMessage* sendMessage = standby->Serialize();
                err = EXTERNAL_SocketSend(
                    connectSocket,
                    (char*) sendMessage->m_Data,
                    sendMessage->m_Size);
                delete sendMessage;
                delete standby;
            }
        }

        // Scan all connected sockets, retrieving data
        for (i = 0; i < connections->size(); i++)
        {
            EXTERNAL_SocketDataAvailable(
                (*connections)[i],
                &avail);
            if (avail)
            {
                // Try to receive data
                try
                {
                    SocketInterface_ReceiveSerializedMessage(
                        (*connections)[i],
                        &data);
                }
                catch (SocketInterface_Exception& e)
                {
                    // Handle the exception here.
                    SocketInterface_HandlePreInitializeException(&e, &data, (*connections)[i]);
                    continue;
                }

                // Continue trying to receive message later if the entire message
                // could not be read
                if (data.m_Size == 0)
                {
                    continue;
                }

                // Try deserializing
                try
                {
                    message = data.Deserialize();
                    message->SetSocketId(i);
                }
                catch (SocketInterface_Exception& e)
                {
                    SocketInterface_HandlePreInitializeException(&e, &data, connectSocket);
                    continue;
                }

                if (message->GetType() == SocketInterface_MessageType_InitializeSimulation)
                {
                    // Handle initialize simulation message -- what we are
                    // looking for.  Loop will exit after this message.
                    try
                    {
                        // Read/save scenario, save name in fileString
                        SocketInterface_HandlePreInitializeSimulationMessage(message, fileString);
                    }
                    catch (SocketInterface_Exception& e)
                    {
                        // Catch the exception here.
                        SocketInterface_HandlePreInitializeException(&e, message, connectSocket);
                        continue;
                    }

                    // Save the message for use after QualNet has initialized.
                    gotScenario = TRUE;

                    PARTITION_ClientStateSet(partition,
                        "SocketInterface_InitializeSimulationMessage",
                        (void*) message);
                }
                else
                {
                    // Save all non-initialization messages
                    messages->push_back(message);
                }
            }
        }
    } // end of while
}

void CreateSocketArray(
    char* arg,
    std::vector<EXTERNAL_Socket*>** sockets,
    std::vector<int>** ports)
{
    char iotoken[MAX_STRING_LENGTH];
    char errString[MAX_STRING_LENGTH];
    char* token;
    char* next;
    int port;
    EXTERNAL_Socket* s;
    EXTERNAL_SocketErrorType socketErr;

    *sockets = new std::vector<EXTERNAL_Socket*>;
    *ports = new std::vector<int>;

    // Loop overall tokens in form x,y,z
    token = IO_GetDelimitedToken(iotoken, arg, ",", &next);
    while (token != NULL)
    {
        // Create new socket and determine port
        s = new EXTERNAL_Socket;
        port = atoi(token);

        // Keep trying to open listening port until success
        do
        {
            socketErr = EXTERNAL_SocketInitListen(
                s,
                port,
                TRUE,
                TRUE);
            if (socketErr != EXTERNAL_NoSocketError)
            {
                sprintf(
                    errString,
                    "Unable to open socket on port %d.  Trying again in 3 seconds.",
                    port);
                ERROR_ReportWarning(errString);
                EXTERNAL_Sleep(3 * SECOND);
            }
        } while (socketErr != EXTERNAL_NoSocketError);

        // Success.  Store socket in vector and get next token
        (*sockets)->push_back(s);
        (*ports)->push_back(port);
        token = IO_GetDelimitedToken(iotoken, next, ",", &next);
    }
}

static void HandleParserConfiguration(
    int argc,
    char* argv [],
    SimulationProperties* simProps)
{
    if (simProps->noMiniParser)
    {
        return;
    }
#ifdef JNE_LIB
    // Now to run the configuration file thru parser. So
    // get the path for the default file
    char defCesPath[512];
    char cesConfigurationFile[512];
    const char* defCesTailPath = "/libraries/ces/data/ces.default.config";
    const char* defCesTailPathWin32 = "\\libraries\\ces\\data";
    const char* defCesFileName = "\\ces.default.config";
    const char* defSettingFileName = "\\ces.parser.rc";
    const char* parserDefSettingPath = "/libraries/ces/data/ces.parser.rc";

    char settingPath[512];

    // create the path for the miniparser setting file
    strcpy(settingPath, simProps->qualnetHomePath);

    // create the path for qualnet default config file.
    strcpy(defCesPath, simProps->qualnetHomePath);
    //strcpy (simProps->qualnetHomePath, "C:/navin");

    char* found;
    found = strchr(simProps->qualnetHomePath, '/');
    if (found == NULL)
    {
        // We have "\" in the home path.
        strcat(defCesPath, defCesTailPathWin32);
        strcat(defCesPath, defCesFileName);

        strcat(settingPath, defCesTailPathWin32);
        strcat(settingPath, defSettingFileName);
    }
    else
    {
        // We have "/" in the path
        strcat(defCesPath, defCesTailPath);
        strcat(settingPath, parserDefSettingPath);
    }
    cout << "CONFIGFILE_PATH=" << defCesPath << endl;

    // Now use the parser to get the configuration file.
    HandleConfigurationFile(simProps->configFileName,
                            defCesPath,
                            cesConfigurationFile,
                            "ces.parser.rc");
    strcpy(simProps->configFileName, cesConfigurationFile);
#endif
}

void SocketInterface_BootStrap(int argc, char * argv [],
    SimulationProperties * simProps,
    PartitionData * partitionData)
{
    std::vector<EXTERNAL_Socket*>* serverSockets = NULL;
    std::vector<int>* serverPorts = NULL;
    BOOL serverMode = FALSE;

    if (partitionData->partitionId != 0)
    {
        return;
    }

    int thisArg = 1;
    while (thisArg < argc)
    {
        if (!strcmp(argv[thisArg], "-socketports"))
        {
            if (argc < thisArg + 1)
            {
                ERROR_ReportError("Not enough arguments to -socketports.\n"
                    "Correct Usage:\t -socketports X[,Y,Z,...].\n");
            }
            else
            {
                serverMode = TRUE;

                CreateSocketArray(
                    argv[thisArg + 1],
                    &serverSockets,
                    &serverPorts);
            }
        }
        else if (!strcmp(argv[thisArg], "-nomini"))
        {
            simProps->noMiniParser = TRUE;
        }
        thisArg++;
    }

    // If we are running in server mode retrieve config file from socket
    if (serverMode)
    {
        char fileString[MAX_STRING_LENGTH];

        SocketInterface_PreInitialize(
            partitionData,
            serverPorts,
            serverSockets,
            fileString);

        strncpy(
            simProps->configFileName,
            fileString,
            MAX_STRING_LENGTH);
        simProps->configFileName[MAX_STRING_LENGTH - 1] = '\0';
    }

    HandleParserConfiguration(argc, argv, simProps);

    // Save command line.  This may be necessary if QualNet receives a restart
    // message.
    std::vector<std::string>* commandLine = new std::vector<std::string>;
    for (thisArg = 0; thisArg < argc; thisArg++)
    {
        commandLine->push_back(argv[thisArg]);
    }

    PARTITION_ClientStateSet(
        partitionData,
        "CesCommandLine",
        (void*) commandLine);
}

SocketInterface_ModeType SocketInterface_GetModeType(EXTERNAL_Interface* iface)
{
    SocketInterface_InterfaceData* data;

    assert(iface != NULL);
    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);

    return data->qualnetMode;
}

void D_Sleep::ReadAsString(std::string& out)
{
    if (m_IsSleeping)
    {
        out = "yes";
    }
    else
    {
        out = "no";
    }
}

void D_Sleep::ExecuteAsString(const std::string& in, std::string& out)
{
    ERROR_ReportWarning("UGS sleep dynamic commands are not supported");
    out = "";
}

void D_C2Node::ReadAsString(std::string& out)
{
    ERROR_ReportWarning("UGS C2Node dynamic commands are not supported");
    out = "";
}

void D_C2Node::ExecuteAsString(const std::string& in, std::string& out)
{
    ERROR_ReportWarning("UGS C2Node dynamic commands are not supported");
    out = "";
}

void D_MulticastGroups::ReadAsString(std::string& out)
{
    GetMulticastGroups(m_Node, out);
}

void SocketInterface_AddNodeToSubnet(Node* node, NodeId nodeId, NodeAddress subnetAddress)
{
     // Get external interface
    EXTERNAL_Interface* iface = node->partitionData->interfaceTable[EXTERNAL_SOCKET];
    if (iface != NULL)
    {
        SocketInterface_InterfaceData* data = (SocketInterface_InterfaceData*) iface->data;

        data->subnetNodeMap[subnetAddress].push_back(nodeId);
    }
}

/*
 * FUNCTION     SocketInterface_InitializeModels
 * PURPOSE      Handles initialization of comm. effects statistical models 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure.
 */
void SocketInterface_InitializeModels(SocketInterface_InterfaceData *data) 
{
    assert(data != NULL); 

    // Create new hashmap for unicast model
    data->unicastStatModel = new SocketInterface_UnicastStatModel();
    
    
    // Create new hashmap for multicast model
    data->multicastStatModel = new SocketInterface_MulticastStatModel();
}

/*
 * FUNCTION     SocketInterface_FinalizeModels
 * PURPOSE      Handles finalization of comm. effects statistical models 
 * Parameters:
 *  data:           Pointer to the SocketInterface_InterfaceData (mts interface) structure.
 */
void SocketInterface_FinalizeModels(SocketInterface_InterfaceData *data) 
{
    assert(data != NULL);
    if ((data->unicastStatModel != NULL) && (!data->unicastStatModel->empty())) 
    {
            
        // Free all SocketInterface_UnicastModel objects in the hashmap
        SocketInterface_UnicastStatModel::iterator it;
        for (it = data->unicastStatModel->begin();
             it != data->unicastStatModel->end();
             ++it)
        {
            MEM_free(it->second);
            
        }
        // Free the hashmap
        data->unicastStatModel->clear();
    }
        
    if ((data->multicastStatModel != NULL) && (!data->multicastStatModel->empty()))
    {
        // Free all SocketInterface_MulticastModel objects in the hashmap
        SocketInterface_MulticastStatModel::iterator it;
        for (it = data->multicastStatModel->begin();
             it != data->multicastStatModel->end();
             ++it)
        {
            // Free all SocketInterface_MulticastModeledReceiverCommEffects objects 
            // in the SocketInterface_MulticastModeledReceiversCommEffects hashmap
            if (!it->second->effects->empty()) 
            {
                SocketInterface_MulticastModeledReceiversCommEffects::iterator it2;
                for (it2 = it->second->effects->begin();
                     it2 != it->second->effects->end();
                     ++it2)
                {
                    MEM_free(it2->second);
                }
                // Free the hashmap
                it->second->effects->clear();
            }
            MEM_free(it->second);
        }
        // Free the hashmap
        data->multicastStatModel->clear();
    }
}

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
    SocketInterface_UnicastModeledCommEffects *effects)
{ 
    SocketInterface_UnicastModel* returnedModel = NULL;
    SocketInterface_UnicastModel *model;
    SocketInterface_UnicastStatModel::iterator it;
    BOOL found = FALSE;
    BOOL hangingKey = FALSE;

    
    assert(params != NULL);
    assert(effects != NULL);
    assert(data != NULL);
    assert(data->unicastStatModel != NULL); 
    if (!data->unicastStatModel->empty()) 
    {
        
        it = data->unicastStatModel->find(params);
        
        // If model for corresponding parameters exist
        // in hashmap, modify this model
        if (it != data->unicastStatModel->end()) 
        {
            
            returnedModel = (SocketInterface_UnicastModel *)it->second;
            if (returnedModel != NULL) 
            {
                if (params->sendTime >= returnedModel->params.sendTime) 
                {
                    found = TRUE;
                }
                // If model reflects newer information, ignore the update and return
                else 
                {
                    return;
                }
            }
            // Make a note if empty model exists for corresponding parameters 
            else 
            {
                hangingKey = TRUE;
            }
        }
    }
        
    // If the model is found, update it. Otherwise add it into the hashmap
    if (found == TRUE) 
    {
        returnedModel->params.sendTime = params->sendTime;
        memcpy(&(returnedModel->effects), effects, sizeof(SocketInterface_UnicastModeledCommEffects));
    }
    else 
    {
        model = (SocketInterface_UnicastModel*) 
            MEM_malloc(sizeof(SocketInterface_UnicastModel));
                    
        memcpy(&(model->params), params, sizeof(SocketInterface_UnicastModelingParameters));
        memcpy(&(model->effects), effects, sizeof(SocketInterface_UnicastModeledCommEffects));
            
        if (hangingKey == FALSE) 
        {
            SocketInterface_UnicastModelingParameters *p;
            
            p = &(model->params);
            (*data->unicastStatModel) [p] = model;
        }
        // In case of empty model, update the parameters and add the model
        else 
        {
            it->second = model;
        }
    }
}

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
    SocketInterface_UnicastModelingParameters *params)
{ 
    SocketInterface_UnicastModel *model;
    SocketInterface_UnicastModeledCommEffects *effects;
    SocketInterface_UnicastStatModel::iterator it;
    
    effects = NULL;


    // Search for model in hashmap, and if found return it
    assert(params != NULL);
    assert(data != NULL);
    assert(data->unicastStatModel != NULL);
    
    if (!data->unicastStatModel->empty()) 
    {
        it = data->unicastStatModel->find(params);
        if (it != data->unicastStatModel->end()) 
        {
            model = it->second;
            if (model != NULL) 
            {
                effects = &(model->effects);
            }
        }
    }
    return effects;
}


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
    SocketInterface_MulticastModeledReceiversCommEffects *effects)
{
    SocketInterface_MulticastModel* returnedModel = NULL;
    SocketInterface_MulticastModel *model;
    SocketInterface_MulticastModeledReceiversCommEffects::iterator it;
    SocketInterface_MulticastModeledReceiversCommEffects::iterator it2;
    SocketInterface_MulticastStatModel::iterator it3;
    
    BOOL found = FALSE;
    BOOL hangingKey = FALSE;
            
    assert(params != NULL);
    assert(effects != NULL);
    assert(data != NULL);
    assert(data->multicastStatModel != NULL);
    
    if (!data->multicastStatModel->empty()) 
    {
        it3 = data->multicastStatModel->find(params);
        // If model for corresponding parameters exist
        // in hashmap, modify this model
        if (it3 != data->multicastStatModel->end()) 
        {
            returnedModel = (SocketInterface_MulticastModel *)it3->second;
            if (returnedModel != NULL) 
            {
                if (params->sendTime >= returnedModel->params.sendTime) 
                {
                    found = TRUE;
                }
                // If model reflects newer information, ignore the update and return
                else 
                {
                    return;
                }
            }
            // Make a note if empty model exists for corresponding parameters 
            else 
            {
                hangingKey = TRUE;
            }
        }
    }
        
        
    // If the model is found, update it. Otherwise add it into the hashmap
    if (found == TRUE) 
    {
        returnedModel->params.sendTime = params->sendTime;
        
        // Create comm. effects hashmap if it doesnt already exist
        if (returnedModel->effects == NULL) 
        {
            returnedModel->effects = new SocketInterface_MulticastModeledReceiversCommEffects();
        }
    }
    else 
    {
        model = (SocketInterface_MulticastModel*) 
            MEM_malloc(sizeof(SocketInterface_MulticastModel));
        memcpy(&(model->params), params, sizeof(SocketInterface_MulticastModelingParameters));
        model->effects = new SocketInterface_MulticastModeledReceiversCommEffects();
            
        if (hangingKey == FALSE) 
        {
            data->multicastStatModel->insert(
                pair<const SocketInterface_MulticastModelingParameters *, 
                SocketInterface_MulticastModel *>(&(model->params),model));
        }
        // In case of empty model, update the parameters and add the model
        else 
        {
            it3->second = model;
        }
        returnedModel = model;
    }

    
    // Set modeled comm. effects to default values for all existing receivers
    if (!returnedModel->effects->empty()) 
    {
        for (it = returnedModel->effects->begin();
             it != returnedModel->effects->end();
             ++it)
        {
            if (it->second == NULL) 
            {
                it->second = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                    MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            }
            it->second->sendTime = params->sendTime;
            it->second->effects.latency = 0.0;
            it->second->effects.success = FALSE;
        }
    }
    
    if (!effects->empty()) 
    {
        // Traverse through receivers comm. effects hashmap and update/insert values
        for (it = effects->begin();
             it != effects->end();
             ++it)
        {
            if (it->second != NULL) 
            {
                if (returnedModel->effects->empty()) 
                {
                    returnedModel->effects->insert(
                        pair<const NodeAddress *, 
                        SocketInterface_MulticastModeledReceiverCommEffects *>
                        (it->first,it->second));
                }
                else 
                {
                    it2 = returnedModel->effects->find(it->first);
                    if (it2 == returnedModel->effects->end()) 
                    {
                        returnedModel->effects->insert(
                            pair<const NodeAddress *, 
                            SocketInterface_MulticastModeledReceiverCommEffects *>
                            (it->first,it->second));
                    }
                    else 
                    {
                        memcpy(it2->second, it->second, sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
                        MEM_free(it->second);
                        it->second = NULL;
                    }
                }
            }
        }
    }
}

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
    SocketInterface_MulticastModeledReceiverCommEffects *effects)
{
    SocketInterface_MulticastModel* returnedModel = NULL;
    SocketInterface_MulticastModeledReceiverCommEffects *tempEffects;
    SocketInterface_MulticastModeledReceiversCommEffects::iterator it;
    SocketInterface_MulticastStatModel::iterator it2;
    
    BOOL found = FALSE;
            
    assert(params != NULL);
    assert(effects != NULL);
    assert(data != NULL);
    assert(data->multicastStatModel != NULL);

    if (!data->multicastStatModel->empty()) 
    {
        it2 = data->multicastStatModel->find(params);
        
        
        // If model for corresponding parameters exist
        // in hashmap, modify this model
        if (it2 != data->multicastStatModel->end()) 
        {
            returnedModel = (SocketInterface_MulticastModel *)it2->second;
            if (returnedModel != NULL) 
            {
                if (params->sendTime == returnedModel->params.sendTime) 
                {
                    found = TRUE;
                }
            }
        }
    }
            
    // Modify model only if it already exists in the hashmap
    if (found == TRUE) 
    {
        if (returnedModel->effects == NULL) 
        {
            returnedModel->effects = new SocketInterface_MulticastModeledReceiversCommEffects();
        }

        // Insert if the effects hashmap is empty
        if (returnedModel->effects->empty()) 
        {
            tempEffects = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            memcpy(tempEffects,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            
            returnedModel->effects->insert(
                pair<const NodeAddress *, 
                SocketInterface_MulticastModeledReceiverCommEffects *>
                (&(tempEffects->receivingPlatformAddress), tempEffects));
        }
        else 
        {
            it = returnedModel->effects->find(&(effects->receivingPlatformAddress));
            
            // Insert if effects hashmap is non-empty but does not contain the receiver model
            if (it == returnedModel->effects->end()) 
            {
                tempEffects = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                    MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
                memcpy(tempEffects,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
                
                returnedModel->effects->insert(
                    pair<const NodeAddress *, 
                    SocketInterface_MulticastModeledReceiverCommEffects *>
                    (&(tempEffects->receivingPlatformAddress), tempEffects));
            }
            // Update if effects hashmap is non-empty and does contain the receiver model
            else 
            {
                if (it->second == NULL) 
                {
                    it->second = (SocketInterface_MulticastModeledReceiverCommEffects*) 
                        MEM_malloc(sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
                }
                memcpy(it->second,effects,sizeof(SocketInterface_MulticastModeledReceiverCommEffects));
            }
        }                
    }
}

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
    SocketInterface_MulticastModelingParameters *params)
{
    SocketInterface_MulticastModel *model;
    SocketInterface_MulticastModeledReceiversCommEffects *effects;
    SocketInterface_MulticastStatModel::iterator it;
    
    effects = NULL;
        
    // Search for model in hashmap, and if found return it
    assert(params != NULL);
    assert(data != NULL);
    assert(data->multicastStatModel != NULL);
    
    if (!data->multicastStatModel->empty()) 
    {
        it = data->multicastStatModel->find(params);
        if (it != data->multicastStatModel->end()) 
        {
            model = it->second;
            if (model != NULL) 
            {
                effects = model->effects;
            }
        }
    }

    return effects;
}

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
    void *params)
{
    SocketInterface_InterfaceData *data;    
    const SocketInterface_MulticastModeledReceiversCommEffects *multiEffects;
    const SocketInterface_UnicastModeledCommEffects *uniEffects;    
    const SocketInterface_MulticastModeledReceiverCommEffects *multiRecEffects;
    SocketInterface_MulticastModeledReceiversCommEffects::const_iterator it;
    SocketInterface_CommEffectsResponseMessage* response;
    BOOL retVal;
    SocketInterface_TimeType latency;
    SocketInterface_Status state;
    std::string receiver;
    std::string originator;
    BOOL packetReturned;
    
    retVal = FALSE;

    assert(iface != NULL);

    data = (SocketInterface_InterfaceData*) iface->data;
    assert(data != NULL);
    assert(packetData != NULL);
    assert(params != NULL);
    
    if (packetData->multicast) 
    {
        uniEffects = NULL;
        // Check if effects for parameters have been modeled
        multiEffects = SocketInterface_GetMulticastCommEffects(
                            data,
                            (SocketInterface_MulticastModelingParameters *)params);
        // Set packet's indicator variable based on existence of modeled comm. effects
        if (multiEffects == NULL) 
        {
            packetData->sentStatisticalModel = FALSE;
        }
        else 
        {
            packetData->sentStatisticalModel = TRUE;
            retVal = TRUE;

            originator = packetData->originator;
            
            // Iterate through multicast receiver models hashmap
            // and send responses for all successful receivers to all concerned
            // response recipients
            if (!multiEffects->empty()) 
            {
                for (it = multiEffects->begin();
                     it != multiEffects->end();
                     ++it)
                {
                    if (it->second != NULL) 
                    {
                        multiRecEffects = (const SocketInterface_MulticastModeledReceiverCommEffects *) it->second;
                        latency = multiRecEffects->effects.latency;
                        
                        // Packet receipt is successful only if the latency is less than the failure timeout
                        // Otherwise if the model indicates that the packet was eventually receiver
                        // it is deemed a successful failure
                        if (latency > 0.0) 
                        {
                            if (latency >= packetData->failureTimeout) 
                            {
                                data->successfulFailures++;
                            }
                            else 
                            {
                                EntityData* mapping; 
                                
                                // If it is multicast, get the entityId and multicast owner for this
                                // node id.  If no multicast owner then send response to packet
                                // originator.
                                mapping = EntityMappingLookup(
                                              &data->entityMapping, 
                                              multiRecEffects->receivingPlatformAddress);
                                assert(mapping != NULL);

                                receiver = mapping->entityId;                                                      
                                // Construct a response message based on values derived from
                                // packetdata and the model
                                response = new
                                    SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                                                    packetData->id2,
                                                                    &originator,
                                                                    &receiver,
                                                                    SocketInterface_Status_Success,
                                                                    (((double) ((SocketInterface_MulticastModelingParameters *)params)->sendTime) / SECOND + 
                                                                     (double) latency),
                                                                    latency);
                                response->SetDescription(packetData->description);
                                
                                // Send the response and update stats
                                BOOL sent = FALSE;
                                if (data->sourceResponsibleMulticast)
                                {
                                    if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
                                    {
                                        SocketInterface_SendMessage(
                                             iface,
                                             response,
                                             packetData->socketId);
                                                
                                        data->responses++;
                                        data->successes++;
                                        sent = TRUE;
                                    }
                                }
                                else
                                {
                                    std::map<NodeId, Entity_NodeData>::iterator entityit; 
                                    for (entityit = mapping->nodes.begin(); 
                                        entityit != mapping->nodes.end(); 
                                        entityit++)
                                    {
                                        Entity_NodeData entity = entityit->second; 

                                        std::vector<MulticastGroup>::iterator it2;
                                        
                                        // If multicast find the multicast group.  If found, send a copy of
                                        // the successfull comm repsonse to each socket connection that is
                                        // subscribing to multicast messages.
                                        for (it2 = entity.multicastGroups.begin();
                                             it2 != entity.multicastGroups.end();
                                             it2++)
                                        {
                                            if (it2->group == packetData->receiver)
                                            {
                                                for (unsigned i = 0; i < it2->socketSubscribers.size(); i++)
                                                {
                                                    if (EXTERNAL_SocketValid(data->sockets.connections[it2->socketSubscribers[i]]))
                                                    {
                                                        SocketInterface_SendMessage(
                                                             iface,
                                                             response,
                                                             it2->socketSubscribers[i]);
                                                        sent = TRUE;
                                                        
                                                        data->responses++;
                                                        data->successes++;
                                                    }
                                                }
                                                break;
                                            }
                                        }
                                    } //end for each entity
                                }
                                
                                // If the packet was not sent to a multicast owner then send it to the
                                // request originator.  This may occur with some broadcast/multicast messages.
                                if (!sent)
                                {
                                    if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
                                    {
                                        SocketInterface_SendMessage(
                                             iface,
                                             response,
                                             packetData->socketId);
                                        
                                        data->responses++;
                                        data->successes++;
                                    }
                                }
                                
                                std::string str;
                                response->Print(&str);
                                str += "\n";
                                
                                SocketInterface_PrintLog(
                                     iface,
                                     data->responsesLogFile,
                                     str.c_str());
                                
                                // delete the message
                                delete response;
                            }
                        }
                    }
                }
            }                    
        }
    }
    else 
    {
        multiEffects = NULL;
        // Check if effects for parameters have been modeled
        uniEffects = SocketInterface_GetUnicastCommEffects(
                          data,
                          (SocketInterface_UnicastModelingParameters *)params);
        // Set packet's indicator variable based on existence of modeled comm. effects
        if (uniEffects == NULL) 
        {
            packetData->sentStatisticalModel = FALSE;
        }
        else 
        {
            packetData->sentStatisticalModel = TRUE;
            retVal = TRUE;
            
            originator = packetData->originator;
            receiver = packetData->receiver;
            latency = uniEffects->latency;
            
            // If packet was not received or it was received after the timeout, 
            // it is deeped a failure, unless the QualNet is configured to always return successes
            if ((latency == 0.0) || (latency >= packetData->failureTimeout)) 
            { 
                packetReturned = FALSE;
                if (data->alwaysSuccess) 
                {
                    state = SocketInterface_Status_Success;
                }
                else 
                {
                    state = SocketInterface_Status_Failure;
                }
                latency = packetData->failureTimeout;
            }
            else 
            {
                state = SocketInterface_Status_Success;
                packetReturned = TRUE;
            }
            
            // Construct a response message based on values derived from
            // packetdata and the model
            response = new
                SocketInterface_CommEffectsResponseMessage(packetData->id1,
                                                packetData->id2,
                                                &originator,
                                                &receiver,
                                                state,
                                                (((double) ((SocketInterface_UnicastModelingParameters *)params)->sendTime) / SECOND + 
                                                 (double) latency),
                                                latency);
            response->SetDescription(packetData->description);
                                                        
            // Update the Mibs
            // Send the response and update stats
            if (EXTERNAL_SocketValid(data->sockets.connections[packetData->socketId]))
            {
                SocketInterface_SendMessage(
                                 iface,
                                 response,
                                 packetData->socketId);
                
                data->responses++;
                if (packetReturned) 
                {
                    data->successes++;
                }
                else 
                {
                    data->failures++;
                    if (latency >= packetData->failureTimeout) 
                    {
                        data->successfulFailures++;
                    }
                }
            }
            
            std::string str;
            response->Print(&str);
            str += "\n";
            
            SocketInterface_PrintLog(
                          iface,
                          data->responsesLogFile,
                          str.c_str());
            
            // delete the message
            delete response;
        }
    }

    return retVal;
}

/*
 * FUNCTION     SocketInterface_GetMessageIds
 * PURPOSE      Retrieve the two message IDs from the SocketInterface_PacketHeader
 *              header of message from qualnet socket
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
    UInt64 *msgId2)
{
    if (payload != NULL && size >= sizeof(SocketInterface_PacketHeader))
    {
        SocketInterface_PacketHeader *pktHdrPtr = (SocketInterface_PacketHeader *) payload;

        *msgId1 = pktHdrPtr->id1;
        *msgId2 = pktHdrPtr->id2;

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

// SocketInterface_GetEntityMappingStructure is used to get the proper mapping structure
// for the socket interface. 
EntityMapping* SocketInterface_GetEntityMappingStructure(EXTERNAL_Interface* iface)
{
    // Check if qualnet socket is running
    if (iface->partition->interfaceTable[EXTERNAL_SOCKET] != NULL)
    {
        SocketInterface_InterfaceData* data2;
        data2 = (SocketInterface_InterfaceData*) iface->partition->interfaceTable[EXTERNAL_SOCKET]->data;
        return &data2->entityMapping;
    }
    else
    {
        return NULL;
    }
}
