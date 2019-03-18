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

#ifndef _MESSAGEAPI_H_
#define _MESSAGEAPI_H_

#include <string>
#include <vector>
#include <iostream>

#define SOCKET_INTERFACE_HEADER_SIZE 8
#define SOCKET_INTERFACE_OPTION_HEADER_SIZE 8
#define SOCKET_INTERFACE_STRING_LENGTH_SIZE 2
#define SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE 4
#define SOCKET_INTERFACE_COORDINATE_SIZE 24

typedef UInt8 SocketInterface_MessageType;

#define SocketInterface_MessageType_SimulationState 0
#define SocketInterface_MessageType_InitializeSimulation 1
#define SocketInterface_MessageType_PauseSimulation 2
#define SocketInterface_MessageType_ExecuteSimulation 3
#define SocketInterface_MessageType_StopSimulation 4
#define SocketInterface_MessageType_ResetSimulation 5
#define SocketInterface_MessageType_AdvanceTime 6
#define SocketInterface_MessageType_SimulationIdle 7
#define SocketInterface_MessageType_DynamicCommand 8
#define SocketInterface_MessageType_DynamicResponse 9
#define SocketInterface_MessageType_CreatePlatform 10
#define SocketInterface_MessageType_UpdatePlatform 11
#define SocketInterface_MessageType_CommEffectsRequest 12
#define SocketInterface_MessageType_CommEffectsResponse 13
#define SocketInterface_MessageType_Error 14
#define SocketInterface_MessageType_QuerySimulationState 20
#define SocketInterface_MessageType_BeginWarmup 21

typedef UInt8 SocketInterface_OptionType;

#define SocketInterface_OptionType_CoordinateSystem 0
#define SocketInterface_OptionType_Scenario 1
#define SocketInterface_OptionType_TimeStamp 2
#define SocketInterface_OptionType_WaitForResponses 3
#define SocketInterface_OptionType_Type 4
#define SocketInterface_OptionType_MulticastGroups 5
#define SocketInterface_OptionType_Position 6
#define SocketInterface_OptionType_State 7
#define SocketInterface_OptionType_Speed 8
#define SocketInterface_OptionType_JoinMulticastGroups 9
#define SocketInterface_OptionType_LeaveMulticastGroups 10
#define SocketInterface_OptionType_PrecedenceOption 11
#define SocketInterface_OptionType_Description 12
#define SocketInterface_OptionType_FailureTimeout 13
#define SocketInterface_OptionType_IdleWhenResponseSent 14
#define SocketInterface_OptionType_PhyTransmissionType 15
#define SocketInterface_OptionType_OriginatingMessage 16
#define SocketInterface_OptionType_ReceiverIpAddresses 17
#define SocketInterface_OptionType_Dscp 18
#define SocketInterface_OptionType_Tos 19
#define SocketInterface_OptionType_Waveform 20
#define SocketInterface_OptionType_Route 21
#define SocketInterface_OptionType_Velocity 22
#define SocketInterface_OptionType_SourceResponseMulticast 23
#define SocketInterface_OptionType_TTL 24

typedef UInt8 SocketInterface_StateType;

#define SocketInterface_StateType_Standby 1
#define SocketInterface_StateType_Initialized 2
#define SocketInterface_StateType_Paused 3
#define SocketInterface_StateType_Executing 4
#define SocketInterface_StateType_Resetting 6
#define SocketInterface_StateType_Stopping 8
#define SocketInterface_StateType_Shutdown 9
#define SocketInterface_StateType_Warmup 10

typedef UInt8 SocketInterface_ModeType;

#define SocketInterface_ModeType_BootStrap 0
#define SocketInterface_ModeType_NoBootStrap 1

typedef UInt8 SocketInterface_TimeManagementType;

#define SocketInterface_TimeManagementType_TimeManaged 0
#define SocketInterface_TimeManagementType_RealTime 1

typedef UInt8 SocketInterface_PlatformStateType;

#define SocketInterface_PlatformStateType_Undamaged 0
#define SocketInterface_PlatformStateType_Damaged 1

typedef Int8 SocketInterface_PlatformType;

#define SocketInterface_PlatformType_Unspecified -1
#define SocketInterface_PlatformType_Ground 0
#define SocketInterface_PlatformType_Air 1

typedef UInt8 SocketInterface_CoordinateSystemType;

#define SocketInterface_CoordinateSystemType_Cartesian 0
#define SocketInterface_CoordinateSystemType_LatLonAlt 1
#define SocketInterface_CoordinateSystemType_GccCartesian 2

typedef UInt8 SocketInterface_Status;

#define SocketInterface_Status_Success 0
#define SocketInterface_Status_Failure 1

typedef UInt8 SocketInterface_ProtocolType;

#define SocketInterface_ProtocolType_ProtocolTCP 0
#define SocketInterface_ProtocolType_ProtocolUDP 1
#define SocketInterface_ProtocolType_ProtocolNetwork 2

typedef UInt8 SocketInterface_WaveformType;

#define SocketInterface_WaveformType_WNW 0
#define SocketInterface_WaveformType_SRW 1

typedef UInt8 SocketInterface_RouteType;

#define SocketInterface_RouteType_USI 0
#define SocketInterface_RouteType_Secret 1

typedef UInt8 SocketInterface_ErrorType;

#define SocketInterface_ErrorType_NoError 0
#define SocketInterface_ErrorType_ConfigurationError 1
#define SocketInterface_ErrorType_MessageCreationError 2
#define SocketInterface_ErrorType_InvalidMessage 3
#define SocketInterface_ErrorType_InvalidTransition 4
#define SocketInterface_ErrorType_InvalidEntityId 5
#define SocketInterface_ErrorType_InvalidState 6
#define SocketInterface_ErrorType_InvalidCoordinates 7
#define SocketInterface_ErrorType_InvalidReceiverId 8
#define SocketInterface_ErrorType_EmptyReceiverList 9
#define SocketInterface_ErrorType_InvalidSenderId  10
#define SocketInterface_ErrorType_InvalidSendTime 11
#define SocketInterface_ErrorType_InvalidStopTime 12
#define SocketInterface_ErrorType_InvalidSocketId 13
#define SocketInterface_ErrorType_QualnetWarning 14
#define SocketInterface_ErrorType_QualnetError 15
#define SocketInterface_ErrorType_SocketError 16
#define SocketInterface_ErrorType_InvalidOperationType 17
#define SocketInterface_ErrorType_EmptyScenarioString 18
#define SocketInterface_ErrorType_InvalidDynamicCommand 19
#define SocketInterface_ErrorType_InvalidProtocol 20
#define SocketInterface_ErrorType_InvalidGroup 21
#define SocketInterface_ErrorType_InvalidSimulationState 22
#define SocketInterface_ErrorType_InvalidNodeId 23

typedef UInt8 SocketInterface_OperationType;

#define SocketInterface_OperationType_Read 0
#define SocketInterface_OperationType_Write 1
#define SocketInterface_OperationType_Execute 2

typedef UInt8 SocketInterface_PrecedenceType;

#define SocketInterface_PrecedenceType_Routine 0
#define SocketInterface_PrecedenceType_Priority 1
#define SocketInterface_PrecedenceType_Immediate 2
#define SocketInterface_PrecedenceType_Flash 3
#define SocketInterface_PrecedenceType_FlashOverride 4
#define SocketInterface_PrecedenceType_Critical 5
#define SocketInterface_PrecedenceType_InternetControl 6
#define SocketInterface_PrecedenceType_NetControl 7

typedef Float64 SocketInterface_TimeType;


class SocketInterface_Coordinates
{
public:
    Float64 m_C1;
    Float64 m_C2;
    Float64 m_C3;

    void SetXYZ(Float64 x, Float64 y, Float64 z);
    void SetLatLonAlt(Float64 lat, Float64 lon, Float64 alt);
};

class SocketInterface_Message;

class SocketInterface_Exception
{
protected:
    SocketInterface_ErrorType m_Error;
    std::string m_ErrorString;

public:
    SocketInterface_Exception()
    {
        m_Error = SocketInterface_ErrorType_NoError;
    }
    SocketInterface_Exception(SocketInterface_ErrorType err, const char* errString)
    {
        m_Error = err;
        m_ErrorString = errString;
    }

    SocketInterface_ErrorType GetError()
    {
        return m_Error;
    }

    std::string& GetErrorString()
    {
        return m_ErrorString;
    }
};

struct SocketInterface_Header
{
    UInt8 messageType;
    UInt8 numOptionFields;
    UInt16 reserved;
    UInt32 messageSize;
};

struct SocketInterface_OptionHeader
{
    UInt8 optionType;
    UInt8 reserved;
    UInt16 reserved2;
    UInt32 optionSize;
};

class SocketInterface_Message;

class SocketInterface_SerializedMessage
{
public:
    UInt8* m_Data;

    // Used for serializing/deserializing message
    UInt32 m_CurrentSize;

    // Amount of data stored
    UInt32 m_Size;

    // Amount of data allocated
    UInt32 m_MaxSize;

    SocketInterface_SerializedMessage();
    ~SocketInterface_SerializedMessage();

    void Resize(UInt32 maxSize);
    void AddHeader(SocketInterface_MessageType type, UInt32 messageSize);
    void AddOptionHeader(SocketInterface_OptionType type, UInt32 optionSize);
    void AddData(UInt8* data, UInt32 size, BOOL swapBytes = TRUE);
    void AddString(std::string* string);
    void AddLongString(std::string* string);

    void ReadData(UInt8* data, UInt32 size, BOOL swapBytes = TRUE);
    void ReadString(std::string* string);
    void ReadLongString(std::string* string);
    BOOL NextOptionHeader(SocketInterface_OptionType* type, UInt32* size);

    void Print(std::string* str);

    SocketInterface_Message* Deserialize();
    SocketInterface_SerializedMessage* Duplicate();
};

class SocketInterface_Message
{
private:
    BOOL m_HasSocketId;
    int m_SocketId;

public:
    SocketInterface_Message();

    // Virtual destructor needed for proper memory freeing of derived
    // classes
    virtual ~SocketInterface_Message() {}

    // Return the type of message
    virtual SocketInterface_MessageType GetType() = 0;

    // Return the size of the message once serialized
    virtual UInt32 GetSize() = 0;

    // Serialize the message into a stream of bytes
    virtual SocketInterface_SerializedMessage* Serialize() = 0;

    // Print the message to a string
    virtual void Print(std::string* str) = 0;

    // Return a duplicate of this message
    SocketInterface_Message* Duplicate();

    // Set the Socket ID for this message
    void SetSocketId(int socketId);

    // Check if this message has a socket ID
    BOOL HasSocketId();

    // Returns the socket id for this message.  Returns -1
    // if there is no socket id.
    int GetSocketId();
};

#define SOCKET_INTERFACE_MIN_SIMULATION_SIZE 10

class SocketInterface_SimulationStateMessage  : public SocketInterface_Message
{
public:
    SocketInterface_StateType m_State;
    SocketInterface_StateType m_OldState;
    BOOL m_SimTimeSpecified;
    SocketInterface_TimeType m_SimTime;


    SocketInterface_SimulationStateMessage(SocketInterface_StateType state,
        SocketInterface_StateType oldState);

    SocketInterface_SimulationStateMessage(SocketInterface_StateType state,
        SocketInterface_StateType oldState,
        SocketInterface_TimeType simTime);

    SocketInterface_SimulationStateMessage(SocketInterface_SerializedMessage* serialize);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_SimulationState; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_INITIALIZE_SIZE 9

class SocketInterface_InitializeSimulationMessage : public SocketInterface_Message
{
public:
    SocketInterface_TimeManagementType m_TimeManagementMode;

    BOOL m_CoordinateSpecified;
    SocketInterface_CoordinateSystemType m_CoordinateSystem;

    UInt8 m_SourceResponseMulticastEnabled;

    std::string m_Scenario;

    SocketInterface_InitializeSimulationMessage(SocketInterface_TimeManagementType mode);
    SocketInterface_InitializeSimulationMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_InitializeSimulation; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetCoordinateSystem(SocketInterface_CoordinateSystemType coordinateSystem);
    void SetScenario(std::string scenario);
    void SetSourceResponseMulticast(UInt8 srm);
};

#define SOCKET_INTERFACE_MIN_PAUSE_SIZE 8

class SocketInterface_PauseSimulationMessage : public SocketInterface_Message
{
public:
    BOOL m_PauseTimeSpecified;
    SocketInterface_TimeType m_PauseTime;

    SocketInterface_PauseSimulationMessage();
    SocketInterface_PauseSimulationMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_PauseSimulation; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetPauseTime(SocketInterface_TimeType time);
};

#define SOCKET_INTERFACE_MIN_BEGIN_WARM_UP_SIZE 8

class SocketInterface_BeginWarmupMessage : public SocketInterface_Message
{
public:
    SocketInterface_BeginWarmupMessage();
    SocketInterface_BeginWarmupMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_BeginWarmup; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_EXECUTE_SIZE 8

class SocketInterface_ExecuteSimulationMessage : public SocketInterface_Message
{
public:
    SocketInterface_ExecuteSimulationMessage();
    SocketInterface_ExecuteSimulationMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_ExecuteSimulation; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_STOP_SIZE 8

class SocketInterface_StopSimulationMessage : public SocketInterface_Message
{
public:
    BOOL m_StopSpecified;
    SocketInterface_TimeType m_StopTime;

    BOOL m_WaitSpecified;
    UInt8 m_WaitForResponses;

    SocketInterface_StopSimulationMessage();
    SocketInterface_StopSimulationMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_StopSimulation; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetStopTime(SocketInterface_TimeType time);
    void SetWaitForResponses(UInt8 waitForResponses);
};

#define SOCKET_INTERFACE_MIN_RESET_SIZE 8

class SocketInterface_ResetSimulationMessage : public SocketInterface_Message
{
public:
    BOOL m_ResetSpecified;
    SocketInterface_TimeType m_ResetTime;

    SocketInterface_ResetSimulationMessage();
    SocketInterface_ResetSimulationMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_ResetSimulation; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetResetTime(SocketInterface_TimeType time);
};

#define SOCKET_INTERFACE_MIN_ADVANCE_SIZE 16

class SocketInterface_AdvanceTimeMessage : public SocketInterface_Message
{
public:
    SocketInterface_TimeType m_TimeAllowance;

    SocketInterface_AdvanceTimeMessage(SocketInterface_TimeType timeAllowance);
    SocketInterface_AdvanceTimeMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_AdvanceTime; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_IDLE_SIZE 16

class SocketInterface_SimulationIdleMessage : public SocketInterface_Message
{
public:
    SocketInterface_TimeType m_CurrentTime;

    SocketInterface_SimulationIdleMessage(SocketInterface_TimeType currentTime);
    SocketInterface_SimulationIdleMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_SimulationIdle; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_CREATE_SIZE 34

class SocketInterface_CreatePlatformMessage : public SocketInterface_Message
{
public:
    std::string m_EntityId;
    SocketInterface_Coordinates m_Position;
    SocketInterface_PlatformStateType m_State;

    BOOL m_CreateTimeSpecified;
    SocketInterface_TimeType m_CreateTime;

    BOOL m_TypeSpecified;
    SocketInterface_PlatformType m_Type;

    BOOL m_VelocitySpecified;
    SocketInterface_Coordinates m_Velocity;

    std::vector<std::string> m_MulticastGroups;

    SocketInterface_CreatePlatformMessage(
        std::string* entityId,
        SocketInterface_Coordinates* position,
        SocketInterface_PlatformStateType state);
    SocketInterface_CreatePlatformMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_CreatePlatform; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetCreateTime(SocketInterface_TimeType time);
    void SetType(SocketInterface_PlatformType type);
    void SetVelocity(SocketInterface_Coordinates* velocity);    
    void AddMulticastGroup(std::string* group);
};

#define SOCKET_INTERFACE_MIN_UPDATE_SIZE 10

class SocketInterface_UpdatePlatformMessage : public SocketInterface_Message
{
public:
    std::string m_EntityId;

    BOOL m_UpdateTimeSpecified;
    SocketInterface_TimeType m_UpdateTime;

    BOOL m_PositionSpecified;
    SocketInterface_Coordinates m_Position;

    BOOL m_StateSpecified;
    SocketInterface_PlatformStateType m_State;

    BOOL m_SpeedSpecified;
    Float64 m_Speed;
    
    BOOL m_VelocitySpecified;
    SocketInterface_Coordinates m_Velocity;

    std::vector<std::string> m_JoinMulticastGroups;

    std::vector<std::string> m_LeaveMulticastGroups;

    SocketInterface_UpdatePlatformMessage(
        std::string* entityId);
    SocketInterface_UpdatePlatformMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_UpdatePlatform; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetUpdateTime(SocketInterface_TimeType time);
    void SetPosition(SocketInterface_Coordinates* position);
    void SetState(SocketInterface_PlatformStateType state);
    void SetSpeed(Float64 speed);
    void SetVelocity(SocketInterface_Coordinates* velocity);
    void AddJoinMulticastGroup(std::string* group);
    void AddLeaveMulticastGroup(std::string* group);
};

#define SOCKET_INTERFACE_MIN_ERROR_SIZE 11

class SocketInterface_ErrorMessage : public SocketInterface_Message
{
public:
    UInt8 m_Code;
    std::string m_Error;

    BOOL m_MessageSpecified;
    SocketInterface_SerializedMessage* m_OriginatingMessage;

    SocketInterface_ErrorMessage(UInt8 code, std::string* error);
    SocketInterface_ErrorMessage(SocketInterface_SerializedMessage* serialized);
    SocketInterface_ErrorMessage(const SocketInterface_ErrorMessage& errorMessage);
    ~SocketInterface_ErrorMessage();

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_Error; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetMessage(SocketInterface_Message* message);
    void SetSerializedMessage(SocketInterface_SerializedMessage* message);
};

#define SOCKET_INTERFACE_MIN_COMM_REQUEST_SIZE 32
#define MTSDEFTTL 64

class SocketInterface_CommEffectsRequestMessage : public SocketInterface_Message
{
public:
    UInt64 m_Id1;
    UInt64 m_Id2;
    SocketInterface_ProtocolType m_Protocol;
    UInt32 m_Size;
    std::string m_SenderId;
    std::string m_ReceiverId;

    BOOL m_SendTimeSpecified;
    SocketInterface_TimeType m_SendTime;

    BOOL m_PrecedenceSpecified;
    SocketInterface_PrecedenceType m_Precedence;

    BOOL m_DscpSpecified;
    UInt8 m_Dscp;

    BOOL m_TosSpecified;
    UInt8 m_Tos;

    BOOL m_TtlSpecified;
    UInt8 m_TTL;

    std::string m_Description;

    BOOL m_FailureTimeoutSpecified;
    SocketInterface_TimeType m_FailureTimeout;

    BOOL m_IdleWhenResponseSentSpecified;
    UInt8 m_IdleWhenResponseSent;

    BOOL m_PhyTransmissionTypeSpecified;
    UInt8 m_PhyTransmissionType;

    BOOL m_WaveformSpecified;
    SocketInterface_WaveformType m_Waveform;

    BOOL m_RouteSpecified;
    SocketInterface_RouteType m_Route;

    // Constructors
    SocketInterface_CommEffectsRequestMessage(
        UInt64 Id1,
        UInt64 Id2,
        SocketInterface_ProtocolType protocol,
        UInt32 size,
        std::string* sender,
        std::string* receiver);
    SocketInterface_CommEffectsRequestMessage(SocketInterface_SerializedMessage* serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_CommEffectsRequest; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);

    void SetPrecedence(SocketInterface_PrecedenceType precedence);
    void SetDscp(UInt8 dscp);
    void SetTos(UInt8 tos);
    void SetDescription(std::string description);
    void SetSendTime(SocketInterface_TimeType time);
    void SetFailureTimeout(SocketInterface_TimeType failureTime);
    void SetIdleWhenResponseSent(UInt8 idleWhenResponseSent);
    void SetPhyTransmissionType(UInt8 transmiisionType);
    void SetWaveform(SocketInterface_WaveformType waveform);
    void SetRoute(SocketInterface_RouteType route);
    void SetTTL(UInt8 ttl);
};

#define SOCKET_INTERFACE_MIN_COMM_RESPONSE_SIZE 44

class SocketInterface_CommEffectsResponseMessage : public SocketInterface_Message
{
public:
    UInt64 m_Id1;
    UInt64 m_Id2;
    std::string m_SenderId;
    std::string m_ReceiverId;

    SocketInterface_Status m_Status;

    SocketInterface_TimeType m_ReceiveTime;
    SocketInterface_TimeType m_Latency;

    std::string m_Description;

    SocketInterface_CommEffectsResponseMessage(UInt64 id1,
        UInt64 id2,
        std::string* sender,
        std::string* receiver,
        SocketInterface_Status status,
        SocketInterface_TimeType receiveTime,
        SocketInterface_TimeType latency);
    SocketInterface_CommEffectsResponseMessage(SocketInterface_SerializedMessage *serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_CommEffectsResponse; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
    void SetDescription(std::string description);
};

#define SOCKET_INTERFACE_MIN_DYNAMIC_COMMAND_SIZE 12

class SocketInterface_DynamicCommandMessage : public SocketInterface_Message
{
public:
    SocketInterface_OperationType m_Type;
    std::string m_Path;
    std::string m_Args;

    SocketInterface_DynamicCommandMessage(SocketInterface_OperationType type,
        std::string* path,
        std::string* args);
    SocketInterface_DynamicCommandMessage(SocketInterface_SerializedMessage *serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_DynamicCommand; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};


#define SOCKET_INTERFACE_MIN_DYNAMIC_RESPONSE_SIZE 14

class SocketInterface_DynamicResponseMessage : public SocketInterface_Message
{
public:
    SocketInterface_OperationType m_Type;
    std::string m_Path;
    std::string m_Args;
    std::string m_Output;

    SocketInterface_DynamicResponseMessage(SocketInterface_OperationType type,
        std::string* path,
        std::string* args,
        std::string* output);
    SocketInterface_DynamicResponseMessage(SocketInterface_SerializedMessage *serialized);

    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_DynamicResponse; }

    UInt32 GetSize();

    SocketInterface_SerializedMessage* Serialize();

    void Print(std::string* str);
};

#define SOCKET_INTERFACE_MIN_QUERY_SIMULATION_STATE_SIZE 8

class SocketInterface_QuerySimulationStateMessage: public SocketInterface_Message
{
public:

    SocketInterface_QuerySimulationStateMessage();
    SocketInterface_QuerySimulationStateMessage(SocketInterface_SerializedMessage* message);
    SocketInterface_MessageType GetType() { return SocketInterface_MessageType_QuerySimulationState; }

    UInt32 GetSize();
    SocketInterface_SerializedMessage* Serialize();
    void Print(std::string* str);
};

void AddScenario (std::string* scenarioString,
                  std::string* configFile);


#endif /* _MESSAGE_API_H_ */
