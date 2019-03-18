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

#ifndef _MTS_H_
#define _MTS_H_

#include <iostream>
#include <vector>

using namespace std;

#include "external_socket.h"
#include "messageapi.h"
#include "socket-interface.h"

enum MTS_Mode
{
    MTS_RealTime,          // No time management
    MTS_TimeManaged,       // Time managed based on a multiple of real-time
    MTS_TimeManagedFast    // Time managed, executed as fast as possible
};

enum MTS_PauseType          //enums implicitly convert types to ints, explicitly stating for clarity
{
    MTS_PauseNA = 0,
    MTS_PauseReal = 1,
    MTS_PauseStop = 2,
    MTS_PauseReset = 3

};

struct MTS_PauseData
{
    MTS_PauseType pauseType;
    double pauseDuration;
};

struct MTS_MessageList
{
    double time;
    double interval;
    double duration;
    double endTime;
    MTS_PauseType pauseType;
    SocketInterface_Message *message;
    MTS_MessageList *next;
};

#define MAX_OUTGOING_MESSAGES 50000

struct MTS_State
{
    char address[MAX_STRING_LENGTH];
    std::string scenarioString;
    int port;
    EXTERNAL_Socket socket;
    BOOL waitingForIdle;
    MTS_Mode mode;
    SocketInterface_CoordinateSystemType coordinateSystem;
    SocketInterface_StateType socketState;
    double realTimeMultiple;
    UInt8 sourceResponseMulticast;

    // True if performing an SNMP walk
    BOOL isWalking;
    BOOL isExecute;
    std::list<MTS_PauseData> pauseData;

    // Advance Time Message
    SocketInterface_AdvanceTimeMessage* advance; 
    double advanceSendTime;
    BOOL cpuHog;
    BOOL timeStamp;
    BOOL gotFirstIdle;
    BOOL sendInitializeSimulation;
    BOOL timeStampSpecified;

    // Slow send will send packets to QualNet 1 byte at a time
    BOOL slowSend;

    // Test warmup will test the warmup phase
    BOOL testWarmup;

    // A hack to work force sending a warmup message
    // Only use when another software does not send a warmup message
    BOOL forceWarmup;

    double federationTime;
    double lookAheadTime;
    pthread_mutex_t timeMutex;
    pthread_cond_t waitingAdvance;

    BOOL printTimeAdvance;
    BOOL printSendMessage;

    std::vector<MTS_MessageList*> messageList;

    pthread_t receiverThread;
    pthread_mutex_t mutex;
    pthread_cond_t receiverNotFull;
    pthread_cond_t receiverNotEmpty;

    volatile MTS_MessageList* outgoingMessages;
    volatile MTS_MessageList* lastOutgoingMessage;
    volatile int numOutgoingMessages;

    pthread_t senderThread;
    pthread_mutex_t outgoingMutex;
    pthread_cond_t outgoingNotFull;
    pthread_cond_t outgoingNotEmpty;

    pthread_mutex_t timeAdvanceMutex;
    clocktype timePaused;
    clocktype pauseTimeStart;
};

void MTS_AddMessageToList(
    MTS_MessageList **list,
    double time,
    double interval,
    SocketInterface_Message *message);

// does not free message memory
void MTS_RemoveFirstNode(MTS_MessageList **list);

void MTS_PrintMessageList(char *prefix, MTS_MessageList *list);

void MTS_ReadConfigFile(
    MTS_State *state,
    char *fileName);

void MTS_HandleMessage(MTS_State *state, SocketInterface_Message *message);

void MTS_HandleSerializedData(MTS_State *state, EXTERNAL_VarArray *data);

void MTS_GrantTimeAdvance(MTS_State *state);

void MTS_ProcessMessages(MTS_State *state);

#endif /* _MTS_H_ */
