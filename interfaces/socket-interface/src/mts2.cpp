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

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <winsock2.h>
#else /* unix/linux */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

using namespace::std;

#include "api.h"
#include "messageapi.h"
#include "mts2.h"
#include "external.h"
#include "external_socket.h"
#include "interfaceutil.h"
#include "WallClock.h"

static clocktype init = 0;

// /**
// FUNCTION   :: strupcase
// PURPOSE    :: convert string to uppercase.
// PARAMETERS ::
// + str : string to be converted to upper case.
// RETURN     ::  uppercase string.
// **/
static char* strupcase(char* str)
{
    char *ch;

    for (ch = str; *ch != 0; ch++)
    {
        *ch = (char)toupper(*ch);
    }

    return str;
}

// /**
// FUNCTION   :: CurrentTime
// PURPOSE    :: Time function for the MTS. It returns the current
//               simulation time based on the timing mode MTS is
//               running.
// PARAMETERS ::
// + state : Pointer to the MTS Interface data structure
// RETURN     ::  uppercase string.
// **/
static double CurrentTime(MTS_State *state)
{
    switch (state->mode)
    {
        case MTS_RealTime:
            if (init == 0)
            {
                init = WallClock::getTrueRealTime();
            }

            if (state->socketState == SocketInterface_StateType_Paused)
            {
                return (double) (state->pauseTimeStart - init - state->timePaused) / SECOND;
            }
            return (double) (WallClock::getTrueRealTime() - init - state->timePaused) / SECOND;
            break;

        case MTS_TimeManaged:
        case MTS_TimeManagedFast:
            return state->federationTime;
            break;

        default:
            // Should not get here
            assert(0);
            return 0.0; // to remove warning
            break;
    }
}

// /**
// FUNCTION   :: MTS_SendPacket
// PURPOSE    :: The function is used to send packet through
//               socket to qualnet.
// PARAMETERS ::
// + state : Pointer to the MTS Interface data structure.
// + message : Mesasge to be sent.
// RETURN     ::  Void : NULL.
// **/
void MTS_SendPacket(MTS_State* state, SocketInterface_Message* message)
{
    MTS_MessageList* node;

    // Create linked list node
    node = (MTS_MessageList*) MEM_malloc(sizeof(MTS_MessageList));
    node->message = message->Duplicate();
    node->next = NULL;

    // Wait until the buffer is not full
    pthread_mutex_lock(&state->outgoingMutex);
    if (state->numOutgoingMessages == MAX_OUTGOING_MESSAGES)
    {
        pthread_cond_wait(&state->outgoingNotFull, &state->outgoingMutex);
    }

    // Add to outgoing queue
    if (state->outgoingMessages == NULL)
    {
        state->outgoingMessages = node;
    }
    else
    {
        state->lastOutgoingMessage->next = node;
    }
    state->lastOutgoingMessage = node;
    state->numOutgoingMessages++;
    pthread_mutex_unlock(&state->outgoingMutex);

    // Signal that the outgoing buffer is not empty
    pthread_cond_signal(&state->outgoingNotEmpty);
}

// /**
// FUNCTION   :: Comp
// PURPOSE    :: Comparison function for message list heap creation
// PARAMETERS ::
// + a : Pointer to the MTS message list data structure.
// + b : Pointer to the MTS message list data structure.
// RETURN     ::  Bool
// **/
bool Comp(MTS_MessageList*& a, MTS_MessageList*& b)
{
    return a->time > b->time;
}

// /**
// FUNCTION   :: MTS_AddMessageToList
// PURPOSE    :: The function is used to add a message to the MTS
//               message list data structure. The messages are inserted
//               based on the time they have to be sent. The list is a heap.
// PARAMETERS ::
// + state : Pointer to the MTS interface data structure.
// + time : Time to send the message.
// + interval : Message repetetion interval time.
// + duration : Waiting time before a messge is sent.
// + message : The message that is added to the message list.
// RETURN     ::  Void : NULL
// **/
void MTS_AddMessageToList(
    MTS_State* state,
    double time,
    double interval,
    double duration,
    double endTime,
    MTS_PauseType pauseType,
    SocketInterface_Message *message)
{
    MTS_MessageList *newNode;

    // Create the new node
    newNode = (MTS_MessageList*) MEM_malloc(sizeof(MTS_MessageList));
    newNode->time = time;
    newNode->interval = interval;
    newNode->duration = duration;
    newNode->endTime = endTime;
    newNode->message = message;
    newNode->pauseType = pauseType;

    pthread_mutex_lock(&state->mutex);
    state->messageList.push_back(newNode);
    push_heap(state->messageList.begin(), state->messageList.end(), Comp);
    pthread_mutex_unlock(&state->mutex);
}


// /**
// FUNCTION   :: MTS_PrintMessageList
// PURPOSE    :: The function is used to print the messages in
//               the message list
// PARAMETERS ::
// + prefix : string to be printed before the messages.
// + list : The message list.
// RETURN     ::  Void : NULL
// **/
void MTS_PrintMessageList(char *prefix, std::vector<MTS_MessageList*>& list)
{
    std::vector<MTS_MessageList*>::iterator it;

    for (it = list.begin(); it != list.end(); it++)
    {
        printf("%s%f ", prefix, (*it)->time);

        switch ((*it)->message->GetType())
        {
            case SocketInterface_MessageType_InitializeSimulation:
                printf("InitializeSimulation");
                break;

            case SocketInterface_MessageType_CommEffectsRequest:
                printf("CommEffectsRequest");
                break;

            case SocketInterface_MessageType_CreatePlatform:
                printf("CreatePlatform");
                break;

            case SocketInterface_MessageType_UpdatePlatform:
                printf("UpdatePlatform");
                break;

            case SocketInterface_MessageType_PauseSimulation:
                printf("PauseSimulation");
                break;

            case SocketInterface_MessageType_ExecuteSimulation:
                printf("ExecuteSimulation");
                break;

            case SocketInterface_MessageType_StopSimulation:
                printf("StopSimulation");
                break;

            case SocketInterface_MessageType_DynamicCommand:
                printf("DynamicCommand");
                break;

            case SocketInterface_MessageType_AdvanceTime:
                printf("Advance");
                break;

            case SocketInterface_MessageType_ResetSimulation:
                printf("ResetSimulation");
                break;

            case SocketInterface_MessageType_QuerySimulationState:
                printf("QuerySimulationState");
                break;
        }

        if ((*it)->interval > 0.0)
        {
            printf(" %f", (*it)->interval);
        }
        if ((*it)->endTime > 0.0)
        {
            printf(" %f", (*it)->endTime);
        }

        printf("\n");
    }
}

// /**
// FUNCTION   :: MTS_RemoveFirstNode
// PURPOSE    :: The function is used to remove the top
//               or the first message from the message list.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
MTS_MessageList* MTS_RemoveFirstNode(MTS_State* state)
{
    MTS_MessageList *temp;

    pthread_mutex_lock(&state->mutex);

    pop_heap(state->messageList.begin(), state->messageList.end(), Comp);
    temp = state->messageList.back();
    state->messageList.pop_back();

    pthread_mutex_unlock(&state->mutex);

    return temp;
}

// /**
// FUNCTION   :: MTS_PeekFirstNode
// PURPOSE    :: The function is used to look at the the top
//               or the first message from the message list.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
MTS_MessageList* MTS_PeekFirstNode(MTS_State* state)
{
    MTS_MessageList *temp;

    pthread_mutex_lock(&state->mutex);

    temp = *state->messageList.begin();

    pthread_mutex_unlock(&state->mutex);

    return temp;
}

// /**
// FUNCTION   :: MTS_WarmupWaitForInitialize
// PURPOSE    :: This function waits for a qualnet state transition to the
//               Initialized state.  Used during warmup time.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
static void MTS_WarmupWaitForInitialize(MTS_State* state)
{
    SocketInterface_SerializedMessage data;

    // Continue looping while qualnet is in warmup (not initialized)
    // The receiver thread will update the state
    while (state->socketState != SocketInterface_StateType_Initialized)
    {
        EXTERNAL_Sleep(100 * MILLI_SECOND);
    }
}

static void MTS_SendCommRequest(
    MTS_State* state)
{
    static UInt64 id1 = 1;
    std::string p1 = "1";
    std::string p2 = "2";

    SocketInterface_CommEffectsRequestMessage* comm = new SocketInterface_CommEffectsRequestMessage(
        id1,
        0,
        SocketInterface_ProtocolType_ProtocolUDP,
        383,
        &p1,
        &p2);
    MTS_SendPacket(state, comm);
    delete comm;

    id1++;
}

// /**
// FUNCTION   :: MTS_SendInitializeSimulation
// PURPOSE    :: The function is used to send the
//               initializeSimulation message to qualnet. Along
//               with the initializeSimulation the function also
//               sends PauseSimulation and the ExecuteSimulation
//               message.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
static void MTS_SendInitializeSimulation(
    MTS_State* state)
{
    SocketInterface_TimeManagementType timeManagementType;
    switch (state->mode)
    {
        case MTS_RealTime:
            timeManagementType = SocketInterface_TimeManagementType_RealTime;
            break;

        case MTS_TimeManaged:
        case MTS_TimeManagedFast:
            timeManagementType = SocketInterface_TimeManagementType_TimeManaged;
            break;

        default:
            // Should not get here, if it does use
            // default value.
            timeManagementType = SocketInterface_TimeManagementType_TimeManaged;
            break;
    }

    // Now we have all the necessary parameters for creating the
    // initializeSimulation message. Create the message.
    SocketInterface_InitializeSimulationMessage* initializeMessage =
        new SocketInterface_InitializeSimulationMessage(timeManagementType);

    // set the coordinate system specified.
    initializeMessage->SetCoordinateSystem(state->coordinateSystem);

    // Now to read the configuration file and create a string out of it.
    // Use the Message API for this task.
    std::string configFile = "";
    if (state->scenarioString.size() > 0)
    {
        AddScenario(&state->scenarioString, &configFile);
    }

    // Set the Scenario parameter for the message
    initializeMessage->SetScenario(configFile);

    // Set the Source Response Multicast parameter for the message
    initializeMessage->SetSourceResponseMulticast(state->sourceResponseMulticast);

    // Send the message
    MTS_SendPacket(state, (SocketInterface_Message*) initializeMessage);
    delete initializeMessage;

    if (state->testWarmup)
    {
        SocketInterface_Message *message;
        MTS_MessageList *el;

        // Send all messages at t0
        if (state->messageList.size() > 0)
        {
            BOOL loop;

            // Extract the message from the list
            el = MTS_PeekFirstNode(state);
            message = el->message;
            loop = el->time == 0;

            while (loop)
            {
                // Now remove it and send the message
                el = MTS_RemoveFirstNode(state);
                message = el->message;
                MTS_SendPacket(state, message);

                // Check if there is another at time 0 to send
                if (state->messageList.size() > 0)
                {
                    el = MTS_PeekFirstNode(state);
                    message = el->message;
                    loop = el->time == 0;
                }
                else
                {
                    loop = FALSE;
                }
            }
        }

        // Send a few CommEffectsRequets during standby
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);

        // Send begin warm-up message
        SocketInterface_BeginWarmupMessage* warmup = new SocketInterface_BeginWarmupMessage();
        MTS_SendPacket(state, warmup);
        delete warmup;
        state->socketState = SocketInterface_StateType_Warmup;

        // Send a few more CommEffectsRequests during warmup
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);

        // Wait for InitializedState
        MTS_WarmupWaitForInitialize(state);

        // Send a few more CommEffectsRequests during initialize
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
    }

    // We need to also send Pause simulation message.
    SocketInterface_PauseSimulationMessage *pauseSimulation = new SocketInterface_PauseSimulationMessage();

    MTS_SendPacket(state, (SocketInterface_Message*) pauseSimulation);
    delete pauseSimulation;

    if (state->testWarmup)
    {
        // Send a few more CommEffectsRequests during pause
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
        MTS_SendCommRequest(state);
    }

    // Send the execute simulation message.
    SocketInterface_ExecuteSimulationMessage *executeSimulation = new SocketInterface_ExecuteSimulationMessage();
    MTS_SendPacket(state, (SocketInterface_Message*) executeSimulation);
    delete executeSimulation;
}

// /**
// FUNCTION   :: ParseQuotes
// PURPOSE    :: The function is used to Parse quotes in the
//               MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + id : String that saves without the quotes.
// RETURN     ::  Void : NULL
// **/
static void ParseQuotes(
    char* iotoken,
    char** next,
    std::string* id)
{
    char* token;
    char* quote;

    if (*next == NULL)
    {
        *id = "";
        return;
    }

    // Get the next token
    token = IO_GetDelimitedToken(iotoken, *next, " \n", next);
    if (token == NULL)
    {
        *id == "";
        return;
    }
    if (token[0] == '"')
    {
        // If the token has a second " in it, extract up to the second
        // quote.  If not, read the rest of the ID up to the next "
        quote = strchr(&token[1], '"');
        if (quote != NULL)
        {
            *quote = 0;
            *id = &token[1];
        }
        else
        {
            *id = &token[1];

            // Get rest of ID up to the "
            token = IO_GetDelimitedToken(iotoken, *next, "\"", next);
            *id += token;

            // The next character should be a ".  If so, skip it.
            if (**next == '"')
            {
                (*next)++;
            }
        }
    }
    else
    {
        *id = token;
    }
}

// /**
// FUNCTION   :: ParseEntityId
// PURPOSE    :: The function is used to Parse entityId in
//               the MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + id : String that saves without the quotes.
// RETURN     ::  Void : NULL
// **/
static void ParseEntityId(
    char* iotoken,
    char** next,
    std::string* id)
{
    char* token;
    char* quote;

    // Get the next token
    token = IO_GetDelimitedToken(iotoken, *next, " \n", next);
    ERROR_Assert(token != NULL, "Missing entityId for CreatePlatform");

    // If the token begins with a ", extract everything up to the next "
    if (token[0] == '"')
    {
        // If the token has a second " in it, extract up to the second
        // quote.  If not, read the rest of the ID up to the next "
        quote = strchr(&token[1], '"');
        if (quote != NULL)
        {
            *quote = 0;
            *id = &token[1];
        }
        else
        {
            *id = &token[1];

            // Get rest of ID up to the "
            token = IO_GetDelimitedToken(iotoken, *next, "\"", next);
            *id += token;

            // The next character should be a ".  If so, skip it.
            if (**next == '"')
            {
                (*next)++;
            }
        }
    }
    else
    {
        *id = token;
    }
}

// /**
// FUNCTION   :: ParseSendExecute
// PURPOSE    :: The function is used to Parse the
//               SendExecute command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseSendExecute(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token != NULL)
    {
        if (strcmp(strupcase(token), "YES") == 0)
        {
            state->isExecute = TRUE;
        }
        else if (strcmp(strupcase(token), "NO") == 0)
        {
            state->isExecute = FALSE;
        }
        else
        {
            printf("WARNING: Unknown value for TimeStamp, IGNORED\n");
        }
    }
}

/*This function would change and  would not
 * be required anymore. We are changing the
 * parameter from Mode to InitializeSimulation.
 */
static void ParseTimeManagementMode(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // The TimeManagement mode is missing.
        // Use the default mode, that is
        // TimeManagedFast mode with lookahead 2.0.
        state->mode = MTS_TimeManagedFast;
        state->lookAheadTime = 2.0;
    }
    else
    {
        if (strcmp(strupcase(token), "REALTIME") == 0)
        {
            state->mode = MTS_RealTime;
            printf("Mode is real-time\n");

            // Check for Timestamp setting. If the user
            // has not set the timestamp, then set no
            // timestamp.
            if (!state->timeStampSpecified)
            {
                state->timeStamp = FALSE;
            }
        }
        else if (strcmp(strupcase(token), "TIMEMANAGED") == 0)
        {
            state->mode = MTS_TimeManaged;

            // Now read real-time multiple
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            if (token == NULL)
            {
                state->realTimeMultiple = 2.0;
            }
            else
            {
                state->realTimeMultiple = atof(token);
            }

            printf(
                "Mode is time managed running %fx faster than real-time\n",
                state->realTimeMultiple);
        }
        else if (strcmp(strupcase(token), "TIMEMANAGEDFAST") == 0)
        {
            state->mode = MTS_TimeManagedFast;

            // Now read advance interval, if present
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            if (token != NULL)
            {
                state->lookAheadTime = atof(token);
            }

            printf("Mode is time managed running as fast as possible\n");
        }
        else
        {
            printf("WARNING: Unknown TimeManagement Mode, Using Default \"%s\"\n", token);
            state->mode = MTS_TimeManagedFast;
            state->lookAheadTime = 2.0;
        }
    }
}

// /**
// FUNCTION   :: ParseCoordinateSystem
// PURPOSE    :: The function is used to Parse the
//               CoordinateSystem command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseCoordinateSystem(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        printf ("WARNING: CoordinateSystem Missing, Using default System!\n");
        state->coordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;
    }
    else
    {
        if (strcmp(strupcase(token), "CARTESIAN") == 0)
        {
            state->coordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;
            printf("Coordinate System is Cartesian\n");
        }
        else if (strcmp(strupcase(token), "LATLONALT") == 0)
        {
            state->coordinateSystem = SocketInterface_CoordinateSystemType_LatLonAlt;
            printf("Coordinate System is LatLonAlt\n");
        }
        else if (strcmp(strupcase(token), "GCCCARTESIAN") == 0)
        {
            state->coordinateSystem = SocketInterface_CoordinateSystemType_GccCartesian;
            printf("Coordinate System is GccCartesian\n");
        }
        else
        {
            printf("WARNING: Unknown Coordinate System,"
                   "Set to default\"%s\"\n", token);
            state->coordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;
        }
    }
}

// /**
// FUNCTION   :: ParseScenario
// PURPOSE    :: The function is used to Parse the
//               Scenario command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseScenario(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // We have no Scenario string specified. Use default value.
        // No Scenario.
        state->scenarioString = "";
    }
    else
    {
       state->scenarioString = token;
    }
}

// /**
// FUNCTION   :: ParseInitializeSimulation
// PURPOSE    :: The function is used to Parse the
//               InitializeSimualtion command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseInitializeSimulation(
    MTS_State *state,
    char* iotoken,
    char* next,
    char* delims)
{
    char* token;

    // Read the different parameters.
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    while (token != NULL)
    {
        if (strcmp(strupcase(token), "COORDINATESYSTEM") == 0)
        {
            // Parse the coordinate system.
            ParseCoordinateSystem(state,
                                  iotoken,
                                  next,
                                  delims);
        }
        else if (strcmp(strupcase(token), "SCENARIO") == 0)
        {
            // Parse the scenario
            ParseScenario(state,
                          iotoken,
                          next,
                          delims);
        }
        else if (strcmp(strupcase(token), "SOURCERESPONSEMULTICAST") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            if (token == NULL)
            {
                printf ("WARNING: Source Response Multicast value missing!\n");
            }
            else if (strcmp(strupcase(token), "YES") == 0)
            {
                // Source Response Multicast is on
                state->sourceResponseMulticast = 1;
            }
            else
            {
               // Source Response Multicast is off
               state->sourceResponseMulticast = 0;
            }
        }
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    }

    state->sendInitializeSimulation = TRUE;
}

// /**
// FUNCTION   :: ParsePauseSimulation
// PURPOSE    :: The function is used to Parse the
//               PauseSimulation command in MTS configuration file.
// PARAMETERS ::

// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParsePauseSimulation(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    double *duration,
    MTS_PauseType *PauseType,
    SocketInterface_PauseSimulationMessage* pauseSimulation)
{
    char *token;
    SocketInterface_TimeType pauseTime = 0.0;
    *PauseType = MTS_PauseReal;      //pause message should default to standard pause

    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // We have only PauseSimulation.
        // No time specified. Send immediately, and duration is 0.0;
        *time = 0.0;
        *duration = 0.0;
        return;
    }
    else if (strcmp(strupcase(token), "DURATION") == 0)
    {
        // In this case the user entered:
        // PauseSimulation Duration 2. So if send pause at
        // time = 0.0, and pause for 2 second duration.
        *time = 0.0;
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        if (token != NULL)
        {
            // We have the Duration time.
            *duration = atof(token);
        }
        else
        {
            // We do not have duration time. So use 0.0 Seconds
            *duration = 0.0;
        }
    }
    else
    {
        pauseTime = atof (token);
        *time = pauseTime;
    }

    if (state->timeStamp)
    {
        pauseSimulation->SetPauseTime(*time);
    }
    
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    while (token != NULL)
    {
        if (strcmp(strupcase(token), "DURATION") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Pause Duration value");
            *duration = atof(token);
        }
        else if (strcmp(strupcase(token), "STOP") == 0)
        {
            *PauseType = MTS_PauseStop;
        }
        else if (strcmp(strupcase(token), "RESET") == 0)
        {

            *PauseType = MTS_PauseReset;
        }
        else
        {
            printf ("Warning: Unknown parameter for PauseSimulation Message, IGNORED!\n");
        }
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    }
}

// /**
// FUNCTION   :: ParseStopSimulation
// PURPOSE    :: The function is used to Parse the
//               StopSimulation command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + stopSimulation: message after parsed fromt the file.
// RETURN     ::  Void : NULL
// **/
static void ParseStopSimulation(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    SocketInterface_StopSimulationMessage* stopSimulation)
{
    char *token;
    SocketInterface_TimeType stopTime = 0.0;
    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No TimeStamp, send immediately
        ERROR_ReportWarning ("WARNING: StopTime Missing in StopSimulation Message!\n");
        *time = 0.0;
    }
    else
    {
        stopTime = atof (token);
        *time = stopTime;
        if (state->timeStamp)
        {
            stopSimulation->SetStopTime(stopTime);
        }
    }
}

// /**
// FUNCTION   :: ParseResetSimulation
// PURPOSE    :: The function is used to Parse the
//               ResetSimulation command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + resetSimulation: message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseResetSimulation(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    SocketInterface_ResetSimulationMessage* resetSimulation)
{
    char *token;
    SocketInterface_TimeType resetTime = 0.0;
    // Read mode
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No timestamp reset immediately.
        *time = 0.0;
        ERROR_ReportWarning ("WARNING: ResetTime Missing in PauseSimulation Message!\n");
    }
    else
    {
        resetTime = atof (token);
        *time = resetTime;
        if (state->timeStamp)
        {
            resetSimulation->SetResetTime (resetTime);
        }
    }
}

// /**
// FUNCTION   :: ParsePort
// PURPOSE    :: The function is used to Parse the
//               Port command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParsePort(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read port number
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        printf ("WARNING: Port missing, Use default!\n");
    }
    else
    {
        state->port = atoi(token);
    }
}

// /**
// FUNCTION   :: ParseAddress
// PURPOSE    :: The function is used to Parse the
//               Address command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseAddress(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read address
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        printf ("WARNING: Address missing, Use default!\n");
    }
    else
    {
        strcpy(state->address, token);
    }
}

// /**
// FUNCTION   :: ParseTimeStamp
// PURPOSE    :: The function is used to Parse the
//               TimeStamps command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// RETURN     ::  Void : NULL
// **/
static void ParseTimeStamp(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims)
{
    char *token;

    // Read address
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        printf ("WARNING: Timestamp value missing!\n");
    }
    else
    {
        if (strcmp(strupcase(token), "YES") == 0)
        {
            state->timeStamp = TRUE;
            state->timeStampSpecified = TRUE;
        }
        else if (strcmp(strupcase(token), "NO") == 0)
        {
            state->timeStamp = FALSE;
            state->timeStampSpecified = TRUE;
        }
        else
        {
            printf("WARNING: Unknown value for TimeStamp, IGNORED\n");
        }
    }
}

// /**
// FUNCTION   :: ParseCommEffectsRequest
// PURPOSE    :: The function is used to Parse the
//               CommEffectsRequest command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + interval : time interval for the message.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseCommEffectsRequest(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    double *interval,
    double *endTime,
    SocketInterface_CommEffectsRequestMessage **message)
{

    char err[MAX_STRING_LENGTH];
    UInt64 id1 = 0;
    UInt64 id2 = 0;
    SocketInterface_ProtocolType protocol = 0;
    UInt32 size = 128;
    std::string senderId = "";
    std::string receiverId = "";
    SocketInterface_TimeType failureTimeout = 0.0;
    Int8 idleWhenResponseSent = 0;
    Int8 phyTransmissionType = 0;
    SocketInterface_WaveformType waveform = 0;
    SocketInterface_RouteType route = 0;
    SocketInterface_PrecedenceType precedence = 0;
    UInt8 dscp = 0;
    UInt8 tos = 0;
    UInt8 ttl = 0;
    std::string description = "";

    BOOL isPrecedence = FALSE;
    BOOL isDscp = FALSE;
    BOOL isTos = FALSE;
    BOOL isTtl = FALSE;
    BOOL isDescription = FALSE;
    BOOL isFailureTimeOut = FALSE;
    BOOL isIdleWhenResponseSent = FALSE;
    BOOL isPhyTransmissionType = FALSE;
    BOOL isWaveform = FALSE;
    BOOL isRoute = FALSE;

    double sendTime = 0.0;
    char *token;

    // Read originator
    ParseEntityId(iotoken, &next, &senderId);

    // Read receiver
    ParseEntityId(iotoken, &next, &receiverId);

    // Read time
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        printf ("WARNING: SendTime Missing, Use default!\n");
        *time = 0.0;
    }
    else
    {
        sendTime = atof(token);
        *time = sendTime;
    }

    // Read optional fields
    *interval = 0;
    *endTime = 0;
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    while (token != NULL)
    {
        if (strcmp(strupcase(token), "INTERVAL") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing INTERVAL value");
            *interval = atof(token);
        }
        else if (strcmp(strupcase(token), "END-TIME") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing INTERVAL value");
            *endTime = atof(token);
        }
        else if (strcmp(strupcase(token), "ID1") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing ID1 value");
            id1 = atoi(token);
        }
        else if (strcmp(strupcase(token), "ID2") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing ID2 value");
            id2 = atoi(token);
        }
        else if (strcmp(strupcase(token), "TCP") == 0)
        {
            protocol = SocketInterface_ProtocolType_ProtocolTCP;
        }
        else if (strcmp(strupcase(token), "UDP") == 0)
        {
            protocol = SocketInterface_ProtocolType_ProtocolUDP;
        }
        else if (strcmp(strupcase(token), "NETWORK") == 0)
        {
            protocol = SocketInterface_ProtocolType_ProtocolNetwork;
        }
        else if (strcmp(strupcase(token), "SIZE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing SIZE value");
            size = atoi(token);
        }
        else if (strcmp(strupcase(token), "PRECEDENCE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing PRECEDENCE value");

            if (strcmp(strupcase(token), "ROUTINE") == 0 ||
                strcmp(strupcase(token), "0") == 0)
            {
                precedence = SocketInterface_PrecedenceType_Routine;
            }
            else if (strcmp(strupcase(token), "PRIORITY") == 0 ||
                     strcmp(strupcase(token), "1") == 0)
            {
                precedence = SocketInterface_PrecedenceType_Priority;
            }
            else if (strcmp(strupcase(token), "IMMEDIATE") == 0 ||
                     strcmp(strupcase(token), "2") == 0)
            {
                precedence = SocketInterface_PrecedenceType_Immediate;
            }
            else if (strcmp(strupcase(token), "FLASH") == 0 ||
                     strcmp(strupcase(token), "3") == 0)
            {
                precedence = SocketInterface_PrecedenceType_Flash;
            }
            else if (strcmp(strupcase(token), "FLASHOVERRIDE") == 0 ||
                     strcmp(strupcase(token), "4") == 0)
            {
                precedence = SocketInterface_PrecedenceType_FlashOverride;
            }
            else if (strcmp(strupcase(token), "CRITICAL") == 0 ||
                     strcmp(strupcase(token), "5") == 0)
            {
                precedence = SocketInterface_PrecedenceType_Critical;
            }
            else if (strcmp(strupcase(token), "INTERNETCONTROL") == 0 ||
                     strcmp(strupcase(token), "6") == 0)
            {
                precedence = SocketInterface_PrecedenceType_InternetControl;
            }
            else if (strcmp(strupcase(token), "NETCONTROL") == 0 ||
                     strcmp(strupcase(token), "7") == 0)
            {
                precedence = SocketInterface_PrecedenceType_NetControl;
            }
            else
            {
                sprintf(err, "Unknown precedence type %s", token);
                ERROR_ReportError(err);
            }

            isPrecedence = TRUE;
        }
        else if (strcmp(strupcase(token), "DSCP") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing DSCP value");
            dscp = (UInt8)atoi(token);
            isDscp = TRUE;
        }
        else if (strcmp(strupcase(token), "TOS") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing TOS value");
            tos = (UInt8)atoi(token);
            isTos = TRUE;
        }
        else if (strcmp(strupcase(token), "TTL") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing TTL value");
            ttl = (UInt8)atoi(token);
            isTtl = TRUE;
        }
        else if (strcmp(strupcase(token), "DESCRIPTION") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing DESCRIPTION value");
            description = token;
            isDescription = TRUE;
        }
        else if (strcmp(strupcase(token), "FAILURETIMEOUT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing FAILURETIMEOUT value");
            failureTimeout = atof(token);
            isFailureTimeOut = TRUE;
        }
        else if (strcmp(strupcase(token), "IDLEWHENRESPONSESENT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing IDLEWHENRESPONSESENT value");
            idleWhenResponseSent = (Int8)atoi(token);
            isIdleWhenResponseSent = TRUE;
        }
        else if (strcmp(strupcase(token), "PHYTRANSMISSIONTYPE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing PHYTRANSMISSIONTYPE value");
            phyTransmissionType = (Int8)atoi(token);
            isPhyTransmissionType = TRUE;
        }
        else if (strcmp(strupcase(token), "WAVEFORM") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing WAVEFORM value");

            if (strcmp(strupcase(token), "WNW") == 0)
            {
                waveform = SocketInterface_WaveformType_WNW;
            }
            else if (strcmp(strupcase(token), "SRW") == 0)
            {
                waveform = SocketInterface_WaveformType_SRW;
            }
            else
            {
                sprintf(err, "Unknown waveform \"%s\"", token);
                ERROR_ReportError(err);
            }

            isWaveform = TRUE;
        }
        else if (strcmp(strupcase(token), "ROUTE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing ROUTE value");

            if (strcmp(strupcase(token), "USI") == 0)
            {
                route = SocketInterface_RouteType_USI;
            }
            else if (strcmp(strupcase(token), "SECRET") == 0)
            {
                route = SocketInterface_RouteType_Secret;
            }
            else
            {
                sprintf(err, "Unknown route \"%s\"", token);
                ERROR_ReportError(err);
            }

            isRoute = TRUE;
        }
        else
        {
            printf("Warning: Unknown CommEffectsRequest Parameter, IGNORED!\n");
        }

        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    }


    // Create CommEffectsRequest message.  Memory is allocated.
    SocketInterface_CommEffectsRequestMessage *requestMessage = new
        SocketInterface_CommEffectsRequestMessage(id1,
                                       id2,
                                       protocol,
                                       size,
                                       &senderId,
                                       &receiverId);
    if (state->timeStamp)
    {
        requestMessage->SetSendTime(sendTime);
    }
    if (isPrecedence)
    {
        requestMessage->SetPrecedence(precedence);
    }
    if (isDscp)
    {
        requestMessage->SetDscp(dscp);
    }
    if (isTos)
    {
        requestMessage->SetTos(tos);
    }
    if (isTtl)
    {
        requestMessage->SetTTL(ttl);
    }
    if (isDescription)
    {
        requestMessage->SetDescription(description);
    }
    if (isFailureTimeOut)
    {
        requestMessage->SetFailureTimeout(failureTimeout);
    }
    if (isIdleWhenResponseSent)
    {
        requestMessage->SetIdleWhenResponseSent(idleWhenResponseSent);
    }
    if (isPhyTransmissionType)
    {
        requestMessage->SetPhyTransmissionType(phyTransmissionType);
    }
    if (isRoute)
    {
        requestMessage->SetRoute(route);
    }
    if (isWaveform)
    {
        requestMessage->SetWaveform(waveform);
    }
    if (isWaveform)
    {
        requestMessage->SetWaveform(waveform);
    }
    if (isRoute)
    {
        requestMessage->SetRoute(route);
    }
    *message = requestMessage;
}

// /**
// FUNCTION   :: ParseSimulationState
// PURPOSE    :: The function is used to Parse the
//               SocketInterface_SimulationStateMessage command in MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseQuerySimulationState(
    char* iotoken,
    char* next,
    char* delims,
    double* time,
    SocketInterface_QuerySimulationStateMessage** message)
{
    SocketInterface_TimeType requestTime = 0.0;
    char* token;

    SocketInterface_QuerySimulationStateMessage* simulationState;
    simulationState = new SocketInterface_QuerySimulationStateMessage();

    // get the time
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No timestamp provided.
        *time = 0.0;
    }
    else
    {
        // we Have timeStamps
        requestTime = atof(token);
        *time = requestTime;
    }

    *message = simulationState;
}

static void ParsePath(
        char** next,
        std::string* id)
{
    std::string str = *next;
    int next1;
    std::string delims = "/";
    int pos = 0;
    std::string temp = "";
    *id = "/";

    next1 = str.find(delims, pos);
    while (next1 != -1)
    {
        if (next1 != pos)
        {
            temp = str.substr(pos, next1 - pos);
            *id += str.substr(pos, next1 - pos);
            *id += delims;
            // add the end delimiter
            if ((pos + next1) < (int)str.size())
            {
                if (str.at(pos + next1) == '\"')
                {
                    delims = "\"";
                    *id += delims;
                }
                else if (delims.compare("\"") != 0)
                {
                    delims = "/";
                }
            }
            else if (delims.compare("\"") != 0)
            {
                delims = "/";
            }
        }

        pos = next1 + 1;
        next1 = str.find(delims, pos);
    }
    if (next1 == -1)
    {
        next1 = str.find(" ", pos);
        if (next1 == -1)
        {
            // we do not have a space means that this is probably the end
            // of the line.
            next1 = str.find("\n", pos);
            if (next1 == -1)
            {
                // We have a problem
                exit(0);
            }
        }
        *id += str.substr(pos, next1 - pos);
    }

    *next = *next + next1;
}
// /**
// FUNCTION   :: ParseRead
// PURPOSE    :: The function is used to Parse the
//               Read(dynamic command) command in MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseRead(
    char* iotoken,
    char* next,
    char* delims,
    double* time,
    double* interval,
    SocketInterface_DynamicCommandMessage** message)
{
    std::string path = "";
    std::string args = "";

    char* token;

    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No timestamp provided. Send immediately.
        *time = 0.0;
    }
    else
    {
        *time = atof(token);

        // Read the Path and Args
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        while (token != NULL)
        {
            if (strcmp(strupcase(token), "PATH") == 0)
            {
                next++;
                ParsePath(&next, &path);
            }
            else if (strcmp(strupcase(token), "INTERVAL") == 0)
            {
                token = IO_GetDelimitedToken(iotoken, next, delims, &next);
                ERROR_Assert(token != NULL, "Missing INTERVAL value");
                *interval = atof(token);
            }
            else
            {
                printf("Warning: Unknown parameter for Read, IGNORED!\n");
            }

            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        }
    }

    // Now to create the dynamic command message
    SocketInterface_DynamicCommandMessage* command;
    command = new SocketInterface_DynamicCommandMessage(SocketInterface_OperationType_Read,
                                            &path,
                                            &args);
    *message = command;
}

// /**
// FUNCTION   :: ParseWrite
// PURPOSE    :: The function is used to Parse the
//               Write (dynamic command) command in MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseWrite(
    char* iotoken,
    char* next,
    char* delims,
    double* time,
    SocketInterface_DynamicCommandMessage** message)
{
    std::string path = "";
    std::string args = "";
    char* token;

    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No timestamp provided. Send immediately.
        *time = 0.0;
    }
    else
    {
        *time = atof(token);

        // Read the Path and Args
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        while (token != NULL)
        {
            if (strcmp(strupcase(token), "PATH") == 0)
            {
                next++;
                ParsePath(&next, &path);
            }
            else if (strcmp(strupcase(token), "ARGS") == 0)
            {
                // We have args for dynamic command
                token = IO_GetDelimitedToken(iotoken, next, delims, &next);
                ERROR_Assert(token != NULL, "Missing Args for Read");
                args = token;
            }
            else
            {
                printf("Warning: Unknown parameter for Write, IGNORED!\n");
            }

            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        }
    }

    // Now to create the dynamic command message
    SocketInterface_DynamicCommandMessage* command;
    command = new SocketInterface_DynamicCommandMessage(SocketInterface_OperationType_Write,
                                            &path,
                                            &args);
    *message = command;
}

// /**
// FUNCTION   :: ParseExecute
// PURPOSE    :: The function is used to Parse the
//               Execute (dynamic command) command in MTS configuration file.
// PARAMETERS ::
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseExecute(
    char* iotoken,
    char* next,
    char* delims,
    double* time,
    SocketInterface_DynamicCommandMessage** message)
{
    std::string path = "";
    std::string args = "";
    char* token;

    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    if (token == NULL)
    {
        // No timestamp provided. Send immediately.
        *time = 0.0;
    }
    else
    {
        *time = atof(token);

        // Read the Path and Args
        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        while (token != NULL)
        {
            if (strcmp(strupcase(token), "PATH") == 0)
            {
                next++;
                ParsePath(&next, &path);
            }
            else if (strcmp(strupcase(token), "ARGS") == 0)
            {
                // We have args for dynamic command
                token = IO_GetDelimitedToken(iotoken, next, delims, &next);
                ERROR_Assert(token != NULL, "Missing Args for Read");
                args = token;
            }
            else
            {
                printf("Warning: Unknown parameter for Execute, IGNORED!\n");
            }

            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
        }
    }

    // Now to create the dynamic command message
    SocketInterface_DynamicCommandMessage* command;
    command = new SocketInterface_DynamicCommandMessage(SocketInterface_OperationType_Execute,
                                            &path,
                                            &args);
    *message = command;
}

// /**
// FUNCTION   :: ParseCreatePlatform
// PURPOSE    :: The function is used to Parse the
//               CreatePlatform command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseCreatePlatform(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    SocketInterface_CreatePlatformMessage **message)
{
    // New API
    std::string entityID = "";
    Float64 c1 = 0.0;
    Float64 c2 = 0.0;
    Float64 c3 = 0.0;
    UInt8 pState = 0;
    Float64 createTime = 0.0;
    UInt8 type = 0;
    Float64 v1 = 0.0;
    Float64 v2 = 0.0;
    Float64 v3 = 0.0;
    SocketInterface_PlatformType platformType = SocketInterface_PlatformType_Ground;
    BOOL isMulticastGroups = FALSE;
    BOOL isPlatformType = FALSE;
    BOOL isVelocity = FALSE;
    std::vector <std::string> multicastGroups;
    SocketInterface_Coordinates position;
    SocketInterface_Coordinates velocity;
    SocketInterface_PlatformStateType platformState = SocketInterface_PlatformStateType_Undamaged;

    char *token;

    // read entityID
    ParseEntityId(iotoken, &next, &entityID);

    // Get the Create Time
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    ERROR_Assert(token != NULL, "Missing Timestamp for CreatePlatform");
    createTime = atof(token);
    *time = createTime;

    // Read optional fields
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    while (token != NULL)
    {
        if (strcmp(strupcase(token), "X") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing X value");
            c1 = atof(token);
        }
        else if (strcmp(strupcase(token), "Y") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Y value");
            c2 = atof(token);
        }
        else if (strcmp(strupcase(token), "Z") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Z value");
            c3 = atof(token);
        }
        else if (strcmp(strupcase(token), "LAT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing LATITUDE value");
            c1 = atof(token);
        }
        else if (strcmp(strupcase(token), "LON") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing LONGITUDE value");
            c2 = atof(token);
        }
        else if (strcmp(strupcase(token), "ALT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing ALTITUDE value");
            c3 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELX") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELX value");
            isVelocity = TRUE;
            v1 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELY") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELY value");
            isVelocity = TRUE;
            v2 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELZ") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELZ value");
            isVelocity = TRUE;
            v3 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELLAT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELLAT value");
            isVelocity = TRUE;
            v1 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELLON") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELLON value");
            isVelocity = TRUE;
            v2 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELALT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELALT value");
            isVelocity = TRUE;
            v3 = atof(token);
        }
        else if (strcmp(strupcase(token), "DAMAGESTATE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing DAMAGESTATE value");
            pState = (UInt8)atoi(token);
            if (pState == 0)
            {
                platformState = SocketInterface_PlatformStateType_Undamaged;
            }
            else if (pState == 1)
            {
                platformState = SocketInterface_PlatformStateType_Damaged;
            }
        }
        else if (strcmp(strupcase(token), "TYPE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing TYPE value");
            type = (UInt8)atoi(token);
            isPlatformType = TRUE;
            if (type == 0)
            {
                platformType = SocketInterface_PlatformType_Ground;
            }
            else if (type == 1)
            {
                platformType = SocketInterface_PlatformType_Air;
            }
            else
            {
                printf ("WARNING: Unknown Platform Type \n");
            }
        }
        else if (strcmp(strupcase(token), "MULTICASTGROUPS") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing MULTICASTGROUPS value");
            multicastGroups.push_back(token);
            isMulticastGroups = TRUE;
        }
        else
        {
            printf("Warning: Unknown CreatePlatform parameter, IGNORED\n");
        }

        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    }

    // set the coordinates and velocity for the platform
    if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
    {
        position.SetLatLonAlt(c1, c2, c3);
        if (isVelocity)
        {
            velocity.SetLatLonAlt(v1,v2,v3);
        }
    }
    else
    {
        position.SetXYZ(c1, c2, c3);
        if (isVelocity)
        {
            velocity.SetXYZ(v1,v2,v3);
        }
    }

     // Create CreatePlatform message.  Memory is allocated.
    SocketInterface_CreatePlatformMessage *createPlatformMessage =
        new SocketInterface_CreatePlatformMessage(&entityID,
                                       &position,
                                       platformState);

    if (state->timeStamp)
    {
        createPlatformMessage->SetCreateTime(createTime);
    }
    if (isPlatformType)
    {
        createPlatformMessage->SetType(platformType);
    }
    if (isVelocity)
    {
        createPlatformMessage->SetVelocity(&velocity);
    }
    if (isMulticastGroups)
    {
        for (unsigned i = 0; i < multicastGroups.size(); i++)
        {
            createPlatformMessage->AddMulticastGroup(&multicastGroups.at(i));
        }
    }

    *message = createPlatformMessage;
}

// /**
// FUNCTION   :: ParseUpdatePlatform
// PURPOSE    :: The function is used to Parse the
//               UpdatePlatform command in MTS configuration file.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + iotoken : Buffer to copy to
// + next : source string
// + delims : delimiter string.
// + time : Time at which the message should be sent.
// + interval : Message interval time.
// + isValidMessage : message is valid or not.
// + message : message after being parsed from the file
// RETURN     ::  Void : NULL
// **/
static void ParseUpdatePlatform(
    MTS_State *state,
    char *iotoken,
    char *next,
    char *delims,
    double *time,
    double *interval,
    BOOL *isValidMessage,
    SocketInterface_UpdatePlatformMessage **message)
{
    std::string entityID = "";
    Float64 c1 = 0.0;
    Float64 c2 = 0.0;
    Float64 c3 = 0.0;
    Float64 updateTime = 0.0;
    Float64 v1 = 0.0;
    Float64 v2 = 0.0;
    Float64 v3 = 0.0;
    SocketInterface_PlatformStateType platformState = SocketInterface_PlatformStateType_Undamaged;
    UInt8 entityState = 0;
    Float64 speed  = 0;
    char *token;
    std::string joinMulticastGroups = "";
    std::string leaveMulticastGroups = "";
    SocketInterface_Coordinates position;
    SocketInterface_Coordinates velocity;

    BOOL isC1 = FALSE;
    BOOL isC2 = FALSE;
    BOOL isC3 = FALSE;
    BOOL isV1 = FALSE;
    BOOL isV2 = FALSE;
    BOOL isV3 = FALSE;
    BOOL isPosition = FALSE;
    BOOL isSpeed = FALSE;
    BOOL isState = FALSE;
    BOOL isVelocity = FALSE;
    //BOOL isJoinMulticastGroup = FALSE;
    //BOOL isLeaveMulticastGroup = FALSE;

    // read entityID
    ParseEntityId(iotoken, &next, &entityID);

    // read time
    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    ERROR_Assert(token != NULL, "Missing timestamp for UpdatePlatform");
    updateTime = atof(token);
    *time = updateTime;

    // Read optional fields
    *interval = 0.0;

    // Create the message.
    SocketInterface_UpdatePlatformMessage *updatePlatformMessage =
            new SocketInterface_UpdatePlatformMessage(&entityID);

    token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    while (token != NULL)
    {
        if (strcmp(strupcase(token), "INTERVAL") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing interval value");
            *interval = atof(token);
        }
        else if (strcmp(strupcase(token), "X") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing X value");
            c1 = atof(token);
            isC1 = TRUE;
        }
        else if (strcmp(strupcase(token), "Y") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Y value");
            c2 = atof(token);
            isC2 = TRUE;
        }
        else if (strcmp(strupcase(token), "Z") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Z value");
            c3 = atof(token);
            isC3 = TRUE;
        }
        else if (strcmp(strupcase(token), "LAT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing LATITUDE value");
            c1 = atof(token);
            isC1 = TRUE;
        }
        else if (strcmp(strupcase(token), "LON") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing LONGITUDE value");
            c2 = atof(token);
            isC2 = TRUE;
        }
        else if (strcmp(strupcase(token), "ALT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing ALTITUDE value");
            c3 = atof(token);
            isC3 = TRUE;
        }
        else if (strcmp(strupcase(token), "VELX") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELX value");
            isV1 = TRUE;
            v1 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELY") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELY value");
            isV2 = TRUE;
            v2 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELZ") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELZ value");
            isV3 = TRUE;
            v3 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELLAT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELLAT value");
            isV1 = TRUE;
            v1 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELLON") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELLON value");
            isV2 = TRUE;
            v2 = atof(token);
        }
        else if (strcmp(strupcase(token), "VELALT") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing VELALT value");
            isV3 = TRUE;
            v3 = atof(token);
        }
        else if (strcmp(strupcase(token), "DAMAGESTATE") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing Damage State value");
            entityState = (UInt8)atoi(token);
            isState = TRUE;
            switch (entityState)
            {
            case 0:
                platformState = SocketInterface_PlatformStateType_Undamaged;
                break;
            case 1:
                platformState = SocketInterface_PlatformStateType_Damaged;
                break;
            default:
                printf ("Unknown DamageState, No damage is reported\n");
                break;
            }

        }
        else if (strcmp(strupcase(token), "SPEED") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing SPEED value");
            speed = atof(token);
            isSpeed = TRUE;
        }
        else if (strcmp(strupcase(token), "JOINMULTICASTGROUPS") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing JOINMULTICASTGROUPS value");
            joinMulticastGroups = token;
            updatePlatformMessage->AddJoinMulticastGroup(&joinMulticastGroups);
            //isJoinMulticastGroup = TRUE;
        }
        else if (strcmp(strupcase(token), "LEAVEMULTICASTGROUPS") == 0)
        {
            token = IO_GetDelimitedToken(iotoken, next, delims, &next);
            ERROR_Assert(token != NULL, "Missing LEAVEMULTICASTGROUPS value");
            leaveMulticastGroups = token;
            updatePlatformMessage->AddLeaveMulticastGroup(&leaveMulticastGroups);
            //isLeaveMulticastGroup = TRUE;
        }
        else
        {
            printf("Warning: Unknown UpdatePlatform parameter, IGNORED\n");
        }

        token = IO_GetDelimitedToken(iotoken, next, delims, &next);
    }

    if (isC1 && isC2 && isC3)
    {
        isPosition = TRUE;
    }
    else if (isC1 && isC2 && state->coordinateSystem != SocketInterface_CoordinateSystemType_GccCartesian)
    {
        isPosition = TRUE;
        c3 = 0.0;
    }
    else if (!isC1 && !isC2 && !isC3)
    {
        // We might have other updates.
        *isValidMessage = TRUE;
    }
    else if (!isC1)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Latitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing X coordinate, Position update will not be reported\n");
        }
    }
    else if (!isC2)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Longitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing Y coordinate, Position update will not be reported\n");
        }
    }
    else if (!isC3)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Altitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing Z coordinate, Position update will not be reported\n");
        }
    }

    if (isPosition)
    {
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            position.SetLatLonAlt(c1,c2,c3);
        }
        else
        {
            position.SetXYZ(c1,c2,c3);
        }
    }

    if (isV1 && isV2 && isV3)
    {
        isVelocity = TRUE;
    }
    else if (isV1 && isV2 && state->coordinateSystem != SocketInterface_CoordinateSystemType_GccCartesian)
    {
        isVelocity = TRUE;
        c3 = 0.0;
    }
    else if (!isV1 && !isV2 && !isV3)
    {
        // We might have other updates.
        *isValidMessage = TRUE;
    }
    else if (!isV1)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Velocity Latitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing Velocity X coordinate, Position update will not be reported\n");
        }
    }
    else if (!isV2)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Velocity Longitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing Velocity Y coordinate, Position update will not be reported\n");
        }
    }
    else if (!isV3)
    {
        *isValidMessage = FALSE;
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            printf ("WARINING: Missing Velocity Altitude, Position update will not be reported\n");
        }
        else
        {
            printf ("WARINING: Missing Velocity Z coordinate, Position update will not be reported\n");
        }
    }
    if (isVelocity)
    {
        if (state->coordinateSystem == SocketInterface_CoordinateSystemType_LatLonAlt)
        {
            velocity.SetLatLonAlt(v1,v2,v3);
        }
        else
        {
            velocity.SetXYZ(v1,v2,v3);
        }
    }

    // Create UpdatePlatform message only if the message is valid.
    if (*isValidMessage)
    {
        if (state->timeStamp)
        {
            updatePlatformMessage->SetUpdateTime(updateTime);
        }
        if (isPosition)
        {
            updatePlatformMessage->SetPosition(&position);
        }
        if (isState)
        {
            updatePlatformMessage->SetState(platformState);
        }
        if (isVelocity)
        {
            updatePlatformMessage->SetVelocity(&velocity);
        }
        if (isSpeed)
        {
            updatePlatformMessage->SetSpeed(speed);
        }

        *message = updatePlatformMessage;
    }
    else
    {
        // We do not need the message
        delete updatePlatformMessage;
    }
}

// /**
// FUNCTION   :: MTS_ReadConfigFile
// PURPOSE    :: The function is used to read the MTS
//               configuration file. This function parses the file,
//               and adds the messages to the message list.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + fileName : Name of the file to parse.
// RETURN     ::  Void : NULL
// **/
void MTS_ReadConfigFile(
    MTS_State *state,
    char *fileName)
{
    char iotoken[MAX_STRING_LENGTH];
    char line[MAX_STRING_LENGTH];
    char *delims = " \n";
    char* next;
    char *token;
    char err[MAX_STRING_LENGTH];
    FILE *f;
    double time;
    double interval;
    double endTime;
    double duration;
    BOOL isValidMessage = TRUE;
    MTS_PauseType PauseType;

    SocketInterface_CommEffectsRequestMessage *requestMessage;
    SocketInterface_CreatePlatformMessage *createMessage;
    SocketInterface_UpdatePlatformMessage *updateMessage;
    //SocketInterface_InitializeSimulationMessage* initializeMessage;
    SocketInterface_DynamicCommandMessage* dynamicWriteCommand;
    SocketInterface_DynamicCommandMessage* dynamicExecuteCommand;
    SocketInterface_DynamicCommandMessage* dynamicReadCommand;
    SocketInterface_QuerySimulationStateMessage* simulationState;

    f = fopen(fileName, "r");
    if (f == NULL)
    {
        sprintf(err, "MTS-SOCKET could not open config file \"%s\"", fileName);
        ERROR_ReportError(err);
    }

    // Continue reading lines until the end of the file
    fgets(line, MAX_STRING_LENGTH, f);
    while (!feof(f))
    {
        interval = 0.0;
        endTime = 0.0;
        duration = 0.0;

        // Read first token
        token = IO_GetDelimitedToken(iotoken, line, delims, &next);
        if (token == NULL || token[0] == '#')
        {
            // Do nothing if empty line or comment
        }
        else if (strcmp(strupcase(token), "SENDEXECUTE") == 0)
        {
            ParseSendExecute(state,
                      iotoken,
                      next,
                      delims);
        }
        else if (strcmp(strupcase(token), "TIMEMANAGEMENTMODE") == 0)
        {
            ParseTimeManagementMode(state,
                      iotoken,
                      next,
                      delims);
        }
        else if (strcmp(strupcase(token), "QUERYSIMULATIONSTATE") == 0)
        {
            ParseQuerySimulationState(
                iotoken,
                next,
                delims,
                &time,
                &simulationState);

            // Add to the message list
            interval = 0.0;
            duration = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) simulationState);
        }        
        else if (strcmp(strupcase(token), "READ") == 0)
        {
            ParseRead(iotoken,
                      next,
                      delims,
                      &time,
                      &interval,
                      &dynamicReadCommand);

            // Add to the message list
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) dynamicReadCommand);
        }
        else if (strcmp(strupcase(token), "WRITE") == 0)
        {
            ParseWrite(iotoken,
                      next,
                      delims,
                      &time,
                      &dynamicWriteCommand);

            // Add to the message list
            interval = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) dynamicWriteCommand);
        }
        else if (strcmp(strupcase(token), "EXECUTE") == 0)
        {
            ParseExecute(iotoken,
                      next,
                      delims,
                      &time,
                      &dynamicExecuteCommand);

            // Add to the message list
            interval = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) dynamicExecuteCommand);
        }
        else if (strcmp(strupcase(token), "INITIALIZESIMULATION") == 0)
        {
            ParseInitializeSimulation(state,
                                      iotoken,
                                      next,
                                      delims);

        }
        else if (strcmp(strupcase(token), "PAUSESIMULATION") == 0)
        {
            SocketInterface_PauseSimulationMessage *pauseSimulationMessage = new SocketInterface_PauseSimulationMessage();
            ParsePauseSimulation(state,
                                 iotoken,
                                 next,
                                 delims,
                                 &time,
                                 &duration,
                                 &PauseType,
                                 pauseSimulationMessage);

            // Add to the message List.
            interval = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 PauseType,
                                 (SocketInterface_Message*) pauseSimulationMessage);
        }
        /*else if (strcmp(strupcase(token), "EXECUTESIMULATION") == 0)
        {
            SocketInterface_ExecuteSimulationMessage *executeSimulation = new SocketInterface_ExecuteSimulationMessage();
            ParseExecuteSimulation(state,
                                   iotoken,
                                   next,
                                   delims,
                                   &time,
                                   executeSimulation);

            // Add to the message list
            interval = 0.0;
            duration = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 (SocketInterface_Message*) executeSimulation);
        }*/
        else if (strcmp(strupcase(token), "STOPSIMULATION") == 0)
        {
            SocketInterface_StopSimulationMessage *stopSimulationMessage = new SocketInterface_StopSimulationMessage();
            ParseStopSimulation(state,
                                iotoken,
                                next,
                                delims,
                                &time,
                                stopSimulationMessage);

            // Add to the message list
            interval = 0.0;
            duration = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) stopSimulationMessage);
        }
        else if (strcmp(strupcase(token), "RESETSIMULATION") == 0)
        {
            SocketInterface_ResetSimulationMessage *resetSimulationMessage = new SocketInterface_ResetSimulationMessage();
            ParseResetSimulation(state,
                                 iotoken,
                                 next,
                                 delims,
                                 &time,
                                 resetSimulationMessage);
            interval = 0.0;
            duration = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) resetSimulationMessage);
        }
        else if (strcmp(strupcase(token), "TIMESTAMPS") == 0)
        {
            ParseTimeStamp(state,
                           iotoken,
                           next,
                           delims);
        }
        else if (strcmp(strupcase(token), "PORT") == 0)
        {
            ParsePort(
                state,
                iotoken,
                next,
                delims);
        }
        else if (strcmp(strupcase(token), "PRINTTIMEADVANCE") == 0)
        {
            state->printTimeAdvance = TRUE;
        }
        else if (strcmp(strupcase(token), "PRINTSENDMESSAGE") == 0)
        {
            state->printSendMessage = TRUE;
        }
        else if (strcmp(strupcase(token), "CPUHOG") == 0)
        {
            state->cpuHog = TRUE;
        }
        else if (strcmp(strupcase(token), "SLOWSEND") == 0)
        {
            state->slowSend = TRUE;
        }
        else if (strcmp(strupcase(token), "TESTWARMUP") == 0)
        {
            state->testWarmup = TRUE;
        }
        else if (strcmp(strupcase(token), "FORCEWARMUP") == 0)
        {
            state->forceWarmup = TRUE;
        }
        else if (strcmp(strupcase(token), "ADDRESS") == 0)
        {
            ParseAddress(
                state,
                iotoken,
                next,
                delims);
        }
        else if (strcmp(strupcase(token), "COMMEFFECTSREQUEST") == 0)
        {
            ParseCommEffectsRequest(state,
                                    iotoken,
                                    next,
                                    delims,
                                    &time,
                                    &interval,
                                    &endTime,
                                    &requestMessage);

            // Add the created message to the list.  The allocated memory
            // will be freed once the message is sent to qualnet.
            duration = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) requestMessage);
        }
        else if (strcmp(strupcase(token), "CREATEPLATFORM") == 0)
        {

            ParseCreatePlatform(state,
                                iotoken,
                                next,
                                delims,
                                &time,
                                &createMessage);

            // Add the created message to the list.  The allocated memory
            // will be freed once the message is sent to qualnet.  The time
            // this message will occur is increased by the lookAheadTime.
            // This is done because there is no timestamp on the
            // CreatePlatform message which means the platform would be
            // created early otherwise.
            interval = 0.0;
            duration = 0.0;
            endTime = 0.0;
            MTS_AddMessageToList(state,
                                 time,
                                 interval,
                                 duration,
                                 endTime,
                                 MTS_PauseNA,                                 //not a pause message
                                 (SocketInterface_Message*) createMessage);
        }
        else if (strcmp(strupcase(token), "UPDATEPLATFORM") == 0)
        {
            ParseUpdatePlatform(state,
                                iotoken,
                                next,
                                delims,
                                &time,
                                &interval,
                                &isValidMessage,
                                &updateMessage);

            // Add the created message to the list.  The allocated memory
            // will be freed once the message is sent to qualnet. Check if
            // the message is valid before adding.
            duration = 0.0;
            endTime = 0.0;
            if (isValidMessage)
            {
                MTS_AddMessageToList(state,
                    time,
                    interval,
                    duration,
                    endTime,
                    MTS_PauseNA,                                 //not a pause message
                    (SocketInterface_Message*) updateMessage);
            }
        }
        else
        {
            sprintf(err, "Unknown message type \"%s\"", token);
            ERROR_ReportError(err);
        }

        fgets(line, MAX_STRING_LENGTH, f);
    }

    fclose(f);
}

// /**
// FUNCTION   :: MTS_HandleMessage
// PURPOSE    :: The function handles the messages received
//               from qualnet. The messages include CommEffectsResponse,
//               error message, DynamicResponse, SimulationIdle, and Simulation state messages.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// + message : message received from qualnet.
// RETURN     ::  Void : NULL
// **/
void MTS_HandleMessage(MTS_State *state, SocketInterface_Message *message)
{
    switch (message->GetType())
    {
        case SocketInterface_MessageType_CommEffectsResponse:
        {
           SocketInterface_CommEffectsResponseMessage* response;
           response = (SocketInterface_CommEffectsResponseMessage*) message;
           //std::string responseString;
           //response->Print(&responseString);
           //cout << responseString << endl;
           // Printing the output to the MTS2 window.
           // MSVC 6 is unable to compile this legal C++ --  error C2593: 'operator <<' is ambiguous
           cout << "Message " << response->m_Id1 << "." << response->m_Id2;
           cout << " Entity " << response->m_SenderId << " To " << response->m_ReceiverId;
           cout << " ReceiveTime " << response->m_ReceiveTime;
           if (response->m_Status == SocketInterface_Status_Failure)
           {
               cout << " FAILED, ";
           }
           else
           {
               cout << " SUCCESS, ";
           }
           cout << "Latency = " << response->m_Latency;
           if (strcmp(response->m_Description.c_str(), "") != 0)
           {
               cout << " Description " << response->m_Description;
           }
           cout << endl;
           break;
        }
        case SocketInterface_MessageType_Error:
        {
            SocketInterface_ErrorMessage *errorMessage;
            errorMessage = (SocketInterface_ErrorMessage*) message;

            std::string errorString;
            errorMessage->Print(&errorString);
            cout << errorString << endl;
            break;
        }

        case SocketInterface_MessageType_SimulationIdle:
        {
            SocketInterface_SimulationIdleMessage *simulationIdleMessage;
            simulationIdleMessage = (SocketInterface_SimulationIdleMessage*) message;
            // Do not grant a time advance if we are in real-time mode.  An
            // idle message may be sent if another MTS is connected to qualnet
            // while operating in a time managed mode.
            if (state->mode != MTS_RealTime)
            {
                pthread_mutex_lock(&state->timeAdvanceMutex);

                MTS_GrantTimeAdvance(state);

                pthread_mutex_unlock(&state->timeAdvanceMutex);
            }

            if (!state->gotFirstIdle)
            {
                state->gotFirstIdle = TRUE;
                printf("MTS-SOCKET received first idle message.\n");
            }
            break;
        }

        case SocketInterface_MessageType_DynamicResponse:
        {
            SocketInterface_DynamicResponseMessage* dynamicResponse;
            dynamicResponse = (SocketInterface_DynamicResponseMessage*) message;

            std::string dynamicResponseString;
            dynamicResponse->Print(&dynamicResponseString);
            cout << dynamicResponseString << endl;
            break;
        }
        case SocketInterface_MessageType_SimulationState:
        {
            SocketInterface_SimulationStateMessage* stateResponse;
            stateResponse = (SocketInterface_SimulationStateMessage*) message;

            std::string stateString;
            stateResponse->Print(&stateString);
            cout << stateString << endl;

            // Change the state for qualnet. We need to know which state
            // qualnet is running.
            state->socketState = stateResponse->m_State;

            // Handle Pause
            if (state->socketState == SocketInterface_StateType_Paused)
            {
                if (state->pauseData.size() > 0)
                {
                    MTS_PauseData pauseData = state->pauseData.front();
                    if (pauseData.pauseType != MTS_PauseNA)
                    {
                        state->pauseTimeStart = WallClock::getTrueRealTime();
                        EXTERNAL_Sleep((clocktype) (pauseData.pauseDuration * SECOND));
                        state->timePaused += (WallClock::getTrueRealTime() - state->pauseTimeStart);

                        if (pauseData.pauseType == MTS_PauseStop)
                        {
                            SocketInterface_StopSimulationMessage *stopSimulation = new SocketInterface_StopSimulationMessage();
                            MTS_SendPacket(state, (SocketInterface_Message*) stopSimulation);
                            delete stopSimulation;
                        }
                        else if (pauseData.pauseType == MTS_PauseReset)
                        {
                            SocketInterface_ResetSimulationMessage *resetSimulation = new SocketInterface_ResetSimulationMessage();
                            MTS_SendPacket(state, (SocketInterface_Message*) resetSimulation);
                            delete resetSimulation;
                        }
                        else if (pauseData.pauseType == MTS_PauseReal)
                        {
                            SocketInterface_ExecuteSimulationMessage *executeSimulation = new SocketInterface_ExecuteSimulationMessage();
                            MTS_SendPacket(state, (SocketInterface_Message*) executeSimulation);
                            delete executeSimulation;
                        }
                        else
                        {
                            //we should never have a pause message that is not a pause message
                            printf("Warning: Unknown Pause message, IGNORED!\n");
                        }
                    }
                    state->pauseData.pop_front();
                }
            }

            break;
        }

        default:
            printf("Warning: Unknown Response message, IGNORED!\n");
            break;
    }
}

// /**
// FUNCTION   :: MTS_GrantTimeAdvance
// PURPOSE    :: The function is used when MTS and qualnet is
//               running in time managed mode. This function is used to
//               grant time advances to qualnet. This function only sets
//               the advance value.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
void MTS_GrantTimeAdvance(MTS_State *state)
{
    if (state->printTimeAdvance)
    {
        printf(
            "time advanced to %f + %f\n",
            state->federationTime,
            state->lookAheadTime);
    }

    // Send the advance message
    state->advance = new SocketInterface_AdvanceTimeMessage
        (state->federationTime + state->lookAheadTime);

    state->advanceSendTime = state->federationTime +
                             state->lookAheadTime;

    // Advance time
    if (state->mode == MTS_TimeManaged)
    {
        if (init == 0)
        {
            init = WallClock::getTrueRealTime();
        }

        // x times faster than real-time
        pthread_mutex_lock(&state->timeMutex);
        state->federationTime = (double) state->realTimeMultiple
            * (WallClock::getTrueRealTime() - init - state->timePaused) / SECOND;

        state->waitingForIdle = FALSE;
        pthread_cond_signal(&state->waitingAdvance);
        pthread_mutex_unlock(&state->timeMutex);
    }
    else if (state->mode == MTS_TimeManagedFast)
    {
        pthread_mutex_lock(&state->timeMutex);
        state->federationTime += state->lookAheadTime;
        state->waitingForIdle = FALSE;
        pthread_cond_signal(&state->waitingAdvance);
        pthread_mutex_unlock(&state->timeMutex);
    }
    else
    {
        // Should not get here
        assert(0);
    }
}

// /**
// FUNCTION   :: MTS_ProcessMessages
// PURPOSE    :: The function is the main process event handler.
//               It processes all the messages in the message list.
//               It sends messsages in the message list and then
//               appropriately advances the time.
// PARAMETERS ::
// + state : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
void MTS_ProcessMessages(MTS_State *state)
{
    SocketInterface_Message *message;
    MTS_MessageList *el;
    SocketInterface_SerializedMessage data;

    // Keep running while the socket is valid and there are messages to send
    while (EXTERNAL_SocketValid(&state->socket))
    {
        // Keep looping until a message can't be sent due to time
        // constraints
        while (state->messageList.size() > 0)
        {
            // If the MTS is not running in time-managed fast mode, make sure
            // that message is not send ahead of real-time
            if (state->mode == MTS_RealTime)
            {
                if (state->messageList[0]->time >
                    CurrentTime(state) + state->lookAheadTime)
                {
                    break;
                }
            }
            else
            {
                // If we have not received an idle message (qualnet is still
                // processing messages we have sent)
                if (state->waitingForIdle == TRUE)
                {
                    // Don't send any non-CommEffectsRequest messages
                    if (state->messageList[0]->message->GetType () !=
                        SocketInterface_MessageType_CommEffectsRequest)
                    {
                        break;
                    }

                    // Send a CommEffectsRequest message one lookAheadTime
                    // into the future.  This will keep qualnet queue filled
                    // with messages, and won't require the MTS to send a
                    // huge group of CommEffectsRequest messages at the same
                    // time as the AdvanceSimulationTime message.
                    pthread_mutex_lock(&state->timeMutex);
                    if (state->federationTime + state->lookAheadTime <
                        state->messageList[0]->time
                        && state->waitingForIdle)
                    {
                        // Wait for time advance
                        pthread_cond_wait(
                            &state->waitingAdvance,
                            &state->timeMutex);
                        pthread_mutex_unlock(&state->timeMutex);
                        break;
                    }
                    pthread_mutex_unlock(&state->timeMutex);
                }
            }

            if (state->messageList[0]->time > state->advanceSendTime &&
                !state->waitingForIdle &&
                state->mode != MTS_RealTime)
            {
                pthread_mutex_lock(&state->timeAdvanceMutex);

                MTS_SendPacket(state, (SocketInterface_Message*) state->advance);
                state->waitingForIdle = TRUE;
                delete state->advance;

                pthread_mutex_unlock(&state->timeAdvanceMutex);
                break;
            }

            // Extract the message from the list
            el = MTS_RemoveFirstNode(state);
            message = el->message;

            // If this is a time advance then set waitingForIdle to be TRUE.
            // We do this because we are not allowed to send any messages
            // until qualnet reports an idle state.  waitingForIdle
            // will be set to FALSE when an idle message is received.  See
            // MTS_HandleSerializedData.
            if (message->GetType() == SocketInterface_MessageType_AdvanceTime)
            {
                state->waitingForIdle = TRUE;
            }

            // Print the message if MTS is configured to do so
            if (state->printSendMessage)
            {
                //printf("Sending message %d (", message->GetType());
                switch (message->GetType())
                {
                    case SocketInterface_MessageType_CommEffectsRequest:
                    {
                        SocketInterface_CommEffectsRequestMessage *request =
                            (SocketInterface_CommEffectsRequestMessage*) message;
                        std::string requestString;
                        request->Print(&requestString);
                        cout << requestString << endl;
                        break;
                    }
                    case SocketInterface_MessageType_AdvanceTime:
                    {
                        SocketInterface_AdvanceTimeMessage *advance =
                            (SocketInterface_AdvanceTimeMessage*) message;
                        std::string advanceString;
                        advance->Print(&advanceString);
                        cout << advanceString << endl;
                        break;
                    }
                }
            }

            // Send the message to qualnet
            MTS_SendPacket(state, message);
            // Check what type of message. If message is PauseSimulation
            // then record the pause type and the duration of pause.
            // Once we receive the state transition message from qualnet, we
            // will pause for the duration and then send the apropriate message.
            if (message->GetType() == SocketInterface_MessageType_PauseSimulation)
            {
                // Record the time.
                MTS_PauseData pauseData;
                pauseData.pauseType = el->pauseType;
                pauseData.pauseDuration = el->duration;
                state->pauseData.push_back(pauseData);
            }

            if (el->interval > 0)
            {
                if (message->GetType() == SocketInterface_MessageType_CommEffectsRequest)
                {
                    SocketInterface_CommEffectsRequestMessage *request =
                        (SocketInterface_CommEffectsRequestMessage*) message;
                    request->m_SendTime = el->time + el->interval;
                    if (request->m_SendTime <  el->endTime ||
                        el->endTime == 0)
                    {
                        request->m_Id2++;
                        // Add to the messge list.
                        MTS_AddMessageToList(
                            state,
                            el->time + el->interval,
                            el->interval,
                            0.0,
                            el->endTime,
                            MTS_PauseNA,                                 //not a pause message
                            message);
                    }
                }
                else
                {
                    // Add the other messages except CommEffectsRequest.
                    MTS_AddMessageToList(
                        state,
                        el->time + el->interval,
                        el->interval,
                        0.0,
                        el->endTime,
                        MTS_PauseNA,                                 //not a pause message
                        message);
                }
            }
            else
            {
                // If it is not re-occurring then free it
                delete message;
            }

            // Free the message list node
            delete el;
        } // end of while message list not empty

        // there is a possibility that the messagelist is empty, but the simulation
        // still has to go on. So keep sending the time advance message.
        if (state->messageList.empty() &&
            state->mode != MTS_RealTime)
        {
            if (!state->waitingForIdle)
            {
                pthread_mutex_lock(&state->timeAdvanceMutex);

                // Send a time Advance message.
                MTS_SendPacket(state, (SocketInterface_Message*) state->advance);
                state->waitingForIdle = TRUE;
                delete state->advance;

                pthread_mutex_unlock(&state->timeAdvanceMutex);
            }
        }

        if (!state->cpuHog)
        {
            EXTERNAL_Sleep(1 * MILLI_SECOND);
        }
    }
}

// /**
// FUNCTION   :: MTS_ReceiverThread
// PURPOSE    :: The function is the receiver thread handler. This
//               function receives the message from the socket, and
//               deserializes the messages.
// PARAMETERS ::
// + voidState : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
void* MTS_ReceiverThread(void *voidState)
{
    MTS_State *state = (MTS_State*) voidState;
    SocketInterface_SerializedMessage data;

    // Continue receiving while the socket is open
    while (state->socket.isOpen && !state->socket.error)
    {
        // Handle any messages
        data.m_Size = 0;
        try
        {
            SocketInterface_ReceiveSerializedMessage(&state->socket, &data);
        }
        catch (SocketInterface_Exception& e)
        {
            // Got an exception from Serialize
            // Check the type and handle the error.
            if (e.GetError() == SocketInterface_ErrorType_SocketError)
            {
                // There was some socket error.
                state->socket.error = TRUE;
            }
            else
            {
                // We have an invalid message.
                printf("Warning: Unknown response message, IGNORED!\n");
                continue;
            }
        }
        if (data.m_Size > 0)
        {
            SocketInterface_Message *message;
            try
            {
                message = data.Deserialize();
            }
            catch (SocketInterface_Exception& e)
            {
                printf("Exception %s\n", e.GetErrorString().c_str());
                continue;
            }
            MTS_HandleMessage(state, message);
            delete message;
        }

        // Check if the state is SocketInterface_StateType_Stopping or SocketInterface_StateType_Resetting.
        // If we have the above two states, then stop MTS.
        if (state->socketState == SocketInterface_StateType_Stopping ||
            state->socketState == SocketInterface_StateType_Resetting)
        {
            // close the socket.
            printf ("Closing connection, received STOP\n");
            // Wait for a second for the buffers to empty.
            EXTERNAL_Sleep(1 * SECOND);
            EXTERNAL_SocketClose(&state->socket);
        }
    }

    // Signal a waitingAdvance if in time managed mode.  This will allow
    // the ProcessMessage function to finish if it is waiting.
    pthread_mutex_lock(&state->timeMutex);
    pthread_cond_signal(&state->waitingAdvance);
    pthread_mutex_unlock(&state->timeMutex);

    return NULL;
}

// /**
// FUNCTION   :: MTS_SenderThread
// PURPOSE    :: The function is the sender thread handler. This
//               function sends the message from MTS to qualnet using
//               the TCP sockets. The functions serializes the messages also.
// PARAMETERS ::
// + voidState : Pointer to MTS interface data structure.
// RETURN     ::  Void : NULL
// **/
void* MTS_SenderThread(void *voidState)
{
    MTS_State *state = (MTS_State*) voidState;
    SocketInterface_SerializedMessage socketData;
    EXTERNAL_SocketErrorType err;
    MTS_MessageList* node;
    int numOutgoing;

    // Continue receiving while the socket is open
    while (state->socket.isOpen && !state->socket.error)
    {
        // Handle any messages
        pthread_mutex_lock(&state->outgoingMutex);
        if (state->outgoingMessages == NULL)
        {
            pthread_cond_wait(
                &state->outgoingNotEmpty,
                &state->outgoingMutex);
        }

        while (state->outgoingMessages == NULL)
        {
            pthread_cond_wait(
                &state->outgoingNotEmpty,
                &state->outgoingMutex);
        }

        // Remove from outgoing list
        node = (MTS_MessageList*) state->outgoingMessages;
        assert(node != NULL);
        state->outgoingMessages = node->next;
        if (node->next == NULL)
        {
            state->lastOutgoingMessage = NULL;
        }
        state->numOutgoingMessages--;
        numOutgoing = state->numOutgoingMessages;
        pthread_mutex_unlock(&state->outgoingMutex);

        if (numOutgoing < MAX_OUTGOING_MESSAGES * 0.5)
        {
            pthread_cond_signal(&state->outgoingNotFull);
        }

        // Now send the message
        SocketInterface_SerializedMessage* message = node->message->Serialize();
        if (state->slowSend)
        {
            // Slow send sends the packet 1 byte at a time
            for (UInt32 i = 0; i < message->m_Size; i++)
            {
                printf("send 1 byte\n");
                err = EXTERNAL_SocketSend(&state->socket, (char*) message->m_Data + i, 1);
                if (err != 0)
                {
                    ERROR_ReportWarning("Socket error -- exiting");
                    state->socket.error = TRUE;
                }
                EXTERNAL_Sleep(1 * MILLI_SECOND);
            }
        }
        else
        {
            err = EXTERNAL_SocketSend(&state->socket, (char*) message->m_Data, message->m_Size);
            if (err != 0)
            {
                ERROR_ReportWarning("Socket error -- exiting");
                state->socket.error = TRUE;
            }
        }
        socketData.m_Size = 0;

        // Now free memory
        delete message;
        delete node->message;
        delete node;
    }

    return NULL;
}

// /**
// FUNCTION   :: MTS_SenderThread
// PURPOSE    :: The main function for MTS Interface
// **/
int main(int argc, char **argv)
{
    EXTERNAL_SocketErrorType err;
    MTS_State *state;

    if (argc < 2)
    {
        ERROR_ReportWarning("Missing configuration file");
        exit(EXIT_FAILURE);
    }

    state = new MTS_State();

    // Initialize the state
    strcpy(state->address, "127.0.0.1");
    state->port = 5032;
    EXTERNAL_SocketInit(&state->socket, TRUE, FALSE);
    state->waitingForIdle = FALSE;
    state->mode = MTS_TimeManagedFast;
    state->coordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;
    state->socketState = SocketInterface_StateType_Standby;
    state->realTimeMultiple = 2.0;
    state->isWalking = FALSE;
    state->isExecute = TRUE;
    state->advance = NULL;
    state->advanceSendTime = 0.0;
    state->cpuHog = FALSE;
    state->timeStamp = TRUE;
    state->gotFirstIdle = FALSE;
    state->sendInitializeSimulation = FALSE;
    state->timeStampSpecified = FALSE;
    state->slowSend = FALSE;
    state->testWarmup = FALSE;
    state->forceWarmup = FALSE;
    state->federationTime = 0.0;
    state->lookAheadTime = 1.0;
    pthread_mutex_init(&state->timeMutex, NULL);
    pthread_cond_init(&state->waitingAdvance, NULL);
    state->printTimeAdvance = FALSE;
    state->printSendMessage = FALSE;
    pthread_mutex_init(&state->mutex, NULL);
    pthread_cond_init(&state->receiverNotFull, NULL);
    pthread_cond_init(&state->receiverNotEmpty, NULL);
    state->outgoingMessages = NULL;
    state->lastOutgoingMessage = NULL;
    state->numOutgoingMessages = 0;
    pthread_mutex_init(&state->outgoingMutex, NULL);
    pthread_cond_init(&state->outgoingNotFull, NULL);
    pthread_cond_init(&state->outgoingNotEmpty, NULL);
    pthread_mutex_init(&state->timeAdvanceMutex, NULL);
    state->timePaused = 0;
    state->pauseTimeStart = 0;
    // Read the config file
    MTS_ReadConfigFile(state, argv[1]);

    printf("Message list:\n");
    MTS_PrintMessageList("    ", state->messageList);

    // Open socket connection with qualnet
    printf(
        "MTS-SOCKET attempting to connect to qualnet at %s port %d...\n",
        state->address,
        state->port);
    err = EXTERNAL_SocketConnect(
        &state->socket,
        state->address,
        state->port,
        20);
    if (err != EXTERNAL_NoSocketError)
    {
        ERROR_ReportWarning("Socket error -- exiting");
        exit(EXIT_FAILURE);
    }

    if (state->slowSend)
    {
        int sockErr;
        int on = 1;
        sockErr = setsockopt(
            state->socket.socketFd,
            IPPROTO_TCP,
            TCP_NODELAY,
            (char*) &on,
            sizeof(on));
        if (err == -1)
        {
            ERROR_ReportError("Could not create socket with TCP_NODELAY");
        }
    }

    printf("... MTS connected to qualnet\n");

    pthread_create(
        &state->receiverThread,
        NULL,
        MTS_ReceiverThread,
        state);

    pthread_create(
        &state->senderThread,
        NULL,
        MTS_SenderThread,
        state);

    if (state->forceWarmup)
    {
        // Send begin warmup
        SocketInterface_BeginWarmupMessage *beginWarmup = new SocketInterface_BeginWarmupMessage();
        MTS_SendPacket(state, (SocketInterface_Message*) beginWarmup);
        delete beginWarmup;

        // Wait for InitializedState
        printf("Sent warmup message, waiting for warmup to finish\n");
        fflush(stdout);
        MTS_WarmupWaitForInitialize(state);

        // Send pause
        SocketInterface_PauseSimulationMessage *pauseSimulation =
            new SocketInterface_PauseSimulationMessage();
        MTS_SendPacket(state, (SocketInterface_Message*) pauseSimulation);
        delete pauseSimulation;

        // Send the execute simulation message.
        SocketInterface_ExecuteSimulationMessage *executeSimulation =
            new SocketInterface_ExecuteSimulationMessage();
        MTS_SendPacket(state, (SocketInterface_Message*) executeSimulation);
        delete executeSimulation;

        // Wait for them to be sent
        while (state->outgoingMessages != NULL)
        {
            EXTERNAL_Sleep(100 * MILLI_SECOND);
        }

        printf("Warmup finished, exiting.\n");
        exit(0);
    }

    // Now check to see if the MTS has to send a Initialize Simulation
    // message. If yes then send the messsage.
    MTS_SendInitializeSimulation(state);

    // Grant initial time advance if not running in real-time.  The
    // lookAheadTime defaults to 1.0 or is set in the config file.
    if (state->mode != MTS_RealTime)
    {
       MTS_GrantTimeAdvance(state);
    }

    // Process messages until finished
    MTS_ProcessMessages(state);

    printf("Exiting...\n");

    if (EXTERNAL_SocketValid(&state->socket))
    {
        EXTERNAL_SocketClose(&state->socket);
    }

    return 0;
}

