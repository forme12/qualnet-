#include <iostream>

#include "api.h"
#include "messageapi.h"
#include "external_socket.h"

void SocketInterface_Coordinates::SetXYZ(Float64 x, Float64 y, Float64 z)
{
    m_C1 = x;
    m_C2 = y;
    m_C3 = z;
}

void SocketInterface_Coordinates::SetLatLonAlt(Float64 lat, Float64 lon, Float64 alt)
{
    m_C1 = lat;
    m_C2 = lon;
    m_C3 = alt;
}

SocketInterface_Message::SocketInterface_Message()
{
    m_SocketId = -1;
    m_HasSocketId = FALSE;
}

SocketInterface_Message* SocketInterface_Message::Duplicate()
{
    switch (GetType())
    {
    case SocketInterface_MessageType_SimulationState:
        return new SocketInterface_SimulationStateMessage(*((SocketInterface_SimulationStateMessage*) this));
        break;

    case SocketInterface_MessageType_InitializeSimulation:
        return new SocketInterface_InitializeSimulationMessage(*((SocketInterface_InitializeSimulationMessage*) this));
        break;

    case SocketInterface_MessageType_BeginWarmup:
        return new SocketInterface_BeginWarmupMessage(*((SocketInterface_BeginWarmupMessage*) this));
        break;

    case SocketInterface_MessageType_PauseSimulation:
        return new SocketInterface_PauseSimulationMessage(*((SocketInterface_PauseSimulationMessage*) this));
        break;

    case SocketInterface_MessageType_ExecuteSimulation:
        return new SocketInterface_ExecuteSimulationMessage(*((SocketInterface_ExecuteSimulationMessage*) this));
        break;

    case SocketInterface_MessageType_StopSimulation:
        return new SocketInterface_StopSimulationMessage(*((SocketInterface_StopSimulationMessage*) this));
        break;

    case SocketInterface_MessageType_ResetSimulation:
        return new SocketInterface_ResetSimulationMessage(*((SocketInterface_ResetSimulationMessage*) this));
        break;

    case SocketInterface_MessageType_AdvanceTime:
        return new SocketInterface_AdvanceTimeMessage(*((SocketInterface_AdvanceTimeMessage*) this));
        break;

    case SocketInterface_MessageType_SimulationIdle:
        return new SocketInterface_SimulationIdleMessage(*((SocketInterface_SimulationIdleMessage*) this));
        break;

    case SocketInterface_MessageType_CreatePlatform:
        return new SocketInterface_CreatePlatformMessage(*((SocketInterface_CreatePlatformMessage*) this));
        break;

    case SocketInterface_MessageType_UpdatePlatform:
        return new SocketInterface_UpdatePlatformMessage(*((SocketInterface_UpdatePlatformMessage*) this));
        break;

    case SocketInterface_MessageType_Error:
        return new SocketInterface_ErrorMessage(*((SocketInterface_ErrorMessage*) this));
        break;

    case SocketInterface_MessageType_CommEffectsRequest:
        return new SocketInterface_CommEffectsRequestMessage(*((SocketInterface_CommEffectsRequestMessage*) this));
        break;

    case SocketInterface_MessageType_CommEffectsResponse:
        return new SocketInterface_CommEffectsResponseMessage(*((SocketInterface_CommEffectsResponseMessage*) this));
        break;

    case SocketInterface_MessageType_DynamicCommand:
        return new SocketInterface_DynamicCommandMessage (*((SocketInterface_DynamicCommandMessage*) this));
        break;

    case SocketInterface_MessageType_DynamicResponse:
        return new SocketInterface_DynamicResponseMessage (*((SocketInterface_DynamicResponseMessage*) this));
        break;

    case SocketInterface_MessageType_QuerySimulationState:
        return new SocketInterface_QuerySimulationStateMessage(*((SocketInterface_QuerySimulationStateMessage*) this));
        break;

    default:
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Invalid message type");
    }
}

void SocketInterface_Message::SetSocketId(int socketId)
{
    m_SocketId = socketId;
    m_HasSocketId = TRUE;
}

BOOL SocketInterface_Message::HasSocketId()
{
    return m_HasSocketId;
}

int SocketInterface_Message::GetSocketId()
{
    if (m_HasSocketId)
    {
        return m_SocketId;
    }
    else
    {
        return -1;
    }
}

SocketInterface_SerializedMessage::SocketInterface_SerializedMessage()
{
    m_Data = NULL;
    m_CurrentSize = 0;
    m_Size = 0;
    m_MaxSize = 0;
}

SocketInterface_SerializedMessage::~SocketInterface_SerializedMessage()
{
    if (m_MaxSize > 0)
    {
        assert(m_Data != NULL);
        delete [] m_Data;
    }
}

void SocketInterface_SerializedMessage::Resize(UInt32 maxSize)
{
    // Update size
    m_CurrentSize = 0;
    m_Size = maxSize;

    // Allocate data or increase max size if necessary
    if (m_MaxSize == 0)
    {
        m_Data = new UInt8[maxSize];
        m_MaxSize = maxSize;
    }
    else if (m_MaxSize < maxSize)
    {
        delete [] m_Data;
        m_Data = new UInt8[maxSize];
        m_MaxSize = maxSize;
    }
}

void SocketInterface_SerializedMessage::AddHeader(
                                      SocketInterface_MessageType type,
                                      UInt32 messageSize)
{
    if (messageSize < SOCKET_INTERFACE_HEADER_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_MessageCreationError,
            "Message too small to add header");
    }

    Resize(messageSize);

    m_Data[0] = type;

    // Set NumOptionalFields to 0.  This will be increased each time an
    // optional header is added.
    m_Data[1] = 0;

    // Reserved
    m_Data[2] = 0;
    m_Data[3] = 0;
    m_CurrentSize = 4;
    AddData((UInt8*) &messageSize, sizeof(UInt32));
}

void SocketInterface_SerializedMessage::AddOptionHeader(
    SocketInterface_OptionType type,
    UInt32 optionSize)
{
    if (m_CurrentSize + SOCKET_INTERFACE_OPTION_HEADER_SIZE > m_Size)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_MessageCreationError,
            "Message too small to add option header");
    }

    // Increase NumOptionalFields
    m_Data[1]++;

    m_Data[m_CurrentSize] = type;

    // Reserved
    m_Data[m_CurrentSize + 1] = 0;
    m_Data[m_CurrentSize + 2] = 0;
    m_Data[m_CurrentSize + 2] = 0;
    m_CurrentSize += 4;

    // Include header size in option size
    optionSize += SOCKET_INTERFACE_OPTION_HEADER_SIZE;
    AddData((UInt8*) &optionSize, sizeof(UInt32));
}

void SocketInterface_SerializedMessage::AddData(
                                    UInt8* data,
                                    UInt32 size,
                                    BOOL swapBytes)
{
    if (size + m_CurrentSize > m_Size)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_MessageCreationError,
            "Message too small to add data");
    }

    memcpy(&m_Data[m_CurrentSize], data, size);
    if (swapBytes)
    {
        EXTERNAL_hton(&m_Data[m_CurrentSize], size);
    }
    m_CurrentSize += size;
}

void SocketInterface_SerializedMessage::AddString(std::string* string)
{
    UInt16 size;

    size = string->size();

    AddData((UInt8*) &size, sizeof(UInt16));

    // Add all of string, do not swap bit order
    AddData((UInt8*) string->c_str(), size, FALSE);
}

void SocketInterface_SerializedMessage::AddLongString(std::string* string)
{
    UInt32 size;

    size = string->size();

    AddData((UInt8*) &size, sizeof(UInt32));

    // Add all of string, do not swap bit order
    AddData((UInt8*) string->c_str(), size, FALSE);
}

void SocketInterface_SerializedMessage::Print(std::string* str)
{
    char hexStr[3];
    UInt32 i;

    for (i = 0; i < m_Size; i++)
    {
        sprintf(hexStr, "%02x", m_Data[i]);
        *str += hexStr;

        if (i % 4 == 3)
        {
            *str += ":";
        }
    }
}

void SocketInterface_SerializedMessage::ReadData(UInt8* data, UInt32 size, BOOL swapBytes)
{
    if (m_CurrentSize + size > m_Size)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Data size is larger than packet");
    }

    // Set scenario string, taking account for string size
    memcpy(data, &m_Data[m_CurrentSize], size);
    if (swapBytes)
    {
        EXTERNAL_hton(data, size);
    }


    m_CurrentSize += size;
}

void SocketInterface_SerializedMessage::ReadString(std::string* string)
{
    UInt16 stringSize;

    memcpy(&stringSize, &m_Data[m_CurrentSize], SOCKET_INTERFACE_STRING_LENGTH_SIZE);
    EXTERNAL_ntoh(&stringSize, sizeof(UInt16));
    if (m_CurrentSize + stringSize + SOCKET_INTERFACE_STRING_LENGTH_SIZE > m_Size)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "String size is larger than packet");
    }

    // Set scenario string, taking account for string size
    string->insert(
        0,
        (char*) &m_Data[m_CurrentSize + SOCKET_INTERFACE_STRING_LENGTH_SIZE],
        stringSize);

    m_CurrentSize += stringSize + SOCKET_INTERFACE_STRING_LENGTH_SIZE;
}

void SocketInterface_SerializedMessage::ReadLongString(std::string* string)
{
    UInt32 stringSize;

    memcpy(&stringSize, &m_Data[m_CurrentSize], SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE);
    EXTERNAL_ntoh(&stringSize, sizeof(UInt32));
    if (m_CurrentSize + stringSize + SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE > m_Size)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "String size is larger than packet");
    }

    // Set scenario string, taking account for string size
    string->insert(
        0,
        (char*) &m_Data[m_CurrentSize + SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE],
        stringSize);

    m_CurrentSize += stringSize + SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE;
}

BOOL SocketInterface_SerializedMessage::NextOptionHeader(
    SocketInterface_OptionType* type,
    UInt32* size)
{
    if (m_CurrentSize >= m_Size)
    {
        return FALSE;
    }
    else
    {
        if (m_CurrentSize + SOCKET_INTERFACE_OPTION_HEADER_SIZE > m_Size)
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Message too small for option header");
        }

        *type = (SocketInterface_OptionType) m_Data[m_CurrentSize];
        memcpy(size, &m_Data[m_CurrentSize + 4], sizeof(UInt32));
        EXTERNAL_ntoh(size, sizeof(UInt32));

        if (m_CurrentSize + *size > m_Size)
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Message too small for option data");
        }

        // Make sure required zero field is actually zero
        if (m_Data[m_CurrentSize + 1] != 0)
        {
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Second byte of optional header must be zero");
        }

        // Increase size to the option data
        m_CurrentSize += SOCKET_INTERFACE_OPTION_HEADER_SIZE;

        // Remove header size from option size
        *size -= SOCKET_INTERFACE_OPTION_HEADER_SIZE;

        return TRUE;
    }
}

SocketInterface_Message* SocketInterface_SerializedMessage::Deserialize()
{
    if (m_Size < SOCKET_INTERFACE_HEADER_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too small for header");
    }

    UInt32 length;
    memcpy(&length, &m_Data[4], sizeof(UInt32));
    EXTERNAL_ntoh(&length, sizeof(UInt32));
    if (m_Size != length)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message is incorrect size");
    }

    switch (m_Data[0])
    {
    case SocketInterface_MessageType_SimulationState:
        return new SocketInterface_SimulationStateMessage(this);
        break;

    case SocketInterface_MessageType_InitializeSimulation:
        return new SocketInterface_InitializeSimulationMessage(this);
        break;

    case SocketInterface_MessageType_BeginWarmup:
        return new SocketInterface_BeginWarmupMessage(this);
        break;

    case SocketInterface_MessageType_PauseSimulation:
        return new SocketInterface_PauseSimulationMessage(this);
        break;

    case SocketInterface_MessageType_ExecuteSimulation:
        return new SocketInterface_ExecuteSimulationMessage(this);
        break;

    case SocketInterface_MessageType_StopSimulation:
        return new SocketInterface_StopSimulationMessage(this);
        break;

    case SocketInterface_MessageType_ResetSimulation:
        return new SocketInterface_ResetSimulationMessage(this);
        break;

    case SocketInterface_MessageType_AdvanceTime:
        return new SocketInterface_AdvanceTimeMessage(this);
        break;

    case SocketInterface_MessageType_SimulationIdle:
        return new SocketInterface_SimulationIdleMessage(this);
        break;

    case SocketInterface_MessageType_CreatePlatform:
        return new SocketInterface_CreatePlatformMessage(this);
        break;

    case SocketInterface_MessageType_UpdatePlatform:
        return new SocketInterface_UpdatePlatformMessage(this);
        break;

    case SocketInterface_MessageType_Error:
        return new SocketInterface_ErrorMessage(this);
        break;

    case SocketInterface_MessageType_CommEffectsRequest:
        return new SocketInterface_CommEffectsRequestMessage(this);
        break;

    case SocketInterface_MessageType_CommEffectsResponse:
        return new SocketInterface_CommEffectsResponseMessage(this);
        break;

    case SocketInterface_MessageType_DynamicCommand:
        return new SocketInterface_DynamicCommandMessage(this);
        break;

    case SocketInterface_MessageType_DynamicResponse:
        return new SocketInterface_DynamicResponseMessage(this);
        break;

    case SocketInterface_MessageType_QuerySimulationState:
        return new SocketInterface_QuerySimulationStateMessage(this);
        break;

    default:
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Unknown message type");
        break;
    }

    return NULL;
}

SocketInterface_SerializedMessage* SocketInterface_SerializedMessage::Duplicate()
{
    SocketInterface_SerializedMessage* n;

    // Create new serialized message
    n = new SocketInterface_SerializedMessage;
    n->Resize(m_Size);

    // Copy the data, do not convert bit order
    n->AddData(m_Data, m_Size, FALSE);

    return n;
}

SocketInterface_SimulationStateMessage::SocketInterface_SimulationStateMessage(
    SocketInterface_StateType state,
    SocketInterface_StateType oldState)
{
    m_State = state;
    m_OldState = oldState;

    m_SimTimeSpecified = FALSE;
}

SocketInterface_SimulationStateMessage::SocketInterface_SimulationStateMessage(
    SocketInterface_StateType state,
    SocketInterface_StateType oldState,
    SocketInterface_TimeType simTime)
{
    m_State = state;
    m_OldState = oldState;
    m_SimTimeSpecified = TRUE;
    m_SimTime = simTime;
}

SocketInterface_SimulationStateMessage::SocketInterface_SimulationStateMessage(SocketInterface_SerializedMessage* serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt32 size;

    m_SimTimeSpecified = FALSE;

    if (serialized->m_Size < SOCKET_INTERFACE_MIN_SIMULATION_SIZE)

    {

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small");

    }

    m_State = (SocketInterface_StateType) data[serialized->m_CurrentSize];

    serialized->m_CurrentSize += sizeof(UInt8);

    m_OldState = (SocketInterface_StateType) data[serialized->m_CurrentSize];

    serialized->m_CurrentSize += sizeof(UInt8);

    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Simulation time option not 8 bytes");
            }

            m_SimTimeSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_SimTime,
                sizeof(SocketInterface_TimeType));
            break;

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}


UInt32 SocketInterface_SimulationStateMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_MIN_SIMULATION_SIZE;

    if (m_SimTimeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    return size;
}


SocketInterface_SerializedMessage* SocketInterface_SimulationStateMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_State, sizeof(UInt8));

    data->AddData((UInt8*) &m_OldState, sizeof(UInt8));
    if (m_SimTimeSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_SimTime, sizeof(SocketInterface_TimeType));
    }

    return data;
}



void SocketInterface_SimulationStateMessage::Print(std::string* str)

{

    char str2[MAX_STRING_LENGTH];

    sprintf (str2, "SimulationState: State = ");

    *str += str2;

    switch (m_State)

    {
        case SocketInterface_StateType_Standby:

            *str += "Standby";

            break;

        case SocketInterface_StateType_Warmup:

            *str += "Warmup";

            break;

        case SocketInterface_StateType_Initialized:

            *str += "Initialized";

            break;

        case SocketInterface_StateType_Paused:

            *str += "Paused";

            break;

        case SocketInterface_StateType_Executing:

            *str += "Executing";

            break;

        case SocketInterface_StateType_Stopping:

            *str += "Stopping";

            break;

        case SocketInterface_StateType_Resetting:

            *str += "Resetting";

            break;

        default:

            *str += "Unknown";

            break;

    }

    *str += ", OldState = ";

    switch (m_OldState)

    {

        case SocketInterface_StateType_Standby:

            *str += "Standby";

            break;


        case SocketInterface_StateType_Warmup:

            *str += "Warmup";

            break;

        case SocketInterface_StateType_Initialized:

            *str += "Initialized";

            break;

        case SocketInterface_StateType_Paused:

            *str += "Paused";

            break;

        case SocketInterface_StateType_Executing:

            *str += "Executing";

            break;

        case SocketInterface_StateType_Stopping:

            *str += "Stopping";

            break;

        case SocketInterface_StateType_Resetting:

            *str += "Resetting";

            break;

        default:

            *str += "Unknown";

            break;
    }
    if (m_SimTimeSpecified)
    {
        sprintf(str2, ": Simulation Time = %f", m_SimTime);
        *str += str2;
    }

}



SocketInterface_InitializeSimulationMessage::SocketInterface_InitializeSimulationMessage(

    SocketInterface_TimeManagementType mode)

{

    m_TimeManagementMode = mode;

    m_CoordinateSpecified = FALSE;

    m_CoordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;

    m_SourceResponseMulticastEnabled = FALSE; 

}



SocketInterface_InitializeSimulationMessage::SocketInterface_InitializeSimulationMessage(SocketInterface_SerializedMessage* serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt32 size;



    m_CoordinateSpecified = FALSE;

    m_CoordinateSystem = SocketInterface_CoordinateSystemType_Cartesian;

    m_SourceResponseMulticastEnabled = FALSE; 



    if (serialized->m_Size < SOCKET_INTERFACE_MIN_INITIALIZE_SIZE)

    {

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small for time management mode");

    }



    m_TimeManagementMode = (SocketInterface_TimeManagementType) data[SOCKET_INTERFACE_HEADER_SIZE];

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE + sizeof(Int8);



    if (m_TimeManagementMode > SocketInterface_TimeManagementType_RealTime)

    {

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Invalid time management mode");

    }



    while (serialized->NextOptionHeader(&type, &size))

    {

        switch (type)

        {

        case SocketInterface_OptionType_CoordinateSystem:

            if (size != sizeof(UInt8))

            {

                throw SocketInterface_Exception(

                    SocketInterface_ErrorType_InvalidMessage,

                    "Coordinate option not 1 byte");

            }



            m_CoordinateSpecified = TRUE;

            serialized->ReadData(

                (UInt8*) &m_CoordinateSystem,

                sizeof(UInt8));



            if (m_CoordinateSystem > 2)

            {

                throw SocketInterface_Exception(

                    SocketInterface_ErrorType_InvalidMessage,

                    "Invalid coordinate system");

            }

            break;



        case SocketInterface_OptionType_Scenario:

            serialized->ReadLongString(&m_Scenario);

            break;



        case SocketInterface_OptionType_SourceResponseMulticast:

            serialized->ReadData((UInt8*) &m_SourceResponseMulticastEnabled,

                sizeof(UInt8));

            break;



        default:

            throw SocketInterface_Exception(

                SocketInterface_ErrorType_InvalidMessage,

                "Invalid option type");

            break;

        }

    }

}



UInt32 SocketInterface_InitializeSimulationMessage::GetSize()

{

    UInt32 size = SOCKET_INTERFACE_HEADER_SIZE + sizeof(UInt8);



    if (m_CoordinateSpecified)

    {

        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);

    }



    if (m_Scenario.size() > 0)

    {

        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE

            + m_Scenario.size();

    }



    if (m_SourceResponseMulticastEnabled)

    {

        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);

    }

    

    return size;

}



SocketInterface_SerializedMessage* SocketInterface_InitializeSimulationMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;



    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_TimeManagementMode, sizeof(UInt8));



    if (m_CoordinateSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_CoordinateSystem, sizeof(UInt8));

        data->AddData((UInt8*) &m_CoordinateSystem, sizeof(UInt8));

    }



    if (m_Scenario.size() > 0)

    {

        data->AddOptionHeader(

            SocketInterface_OptionType_Scenario,

            SOCKET_INTERFACE_LONG_STRING_LENGTH_SIZE + m_Scenario.size());

        data->AddLongString(&m_Scenario);

    }



    if (m_SourceResponseMulticastEnabled)

    {

        data->AddOptionHeader(SocketInterface_OptionType_SourceResponseMulticast, sizeof(UInt8));

        data->AddData((UInt8*) &m_SourceResponseMulticastEnabled, sizeof(UInt8));

    }



    return data;

}



void SocketInterface_InitializeSimulationMessage::Print(std::string* str)

{

    char intStr[MAX_STRING_LENGTH];



    *str += "InitializeSimulation: TimeManagementMode = ";



    //sprintf(intStr, "%d", m_TimeManagementMode);

    switch (m_TimeManagementMode)

    {

    case SocketInterface_TimeManagementType_TimeManaged:

        *str += "TimeManaged";

        break;



    case SocketInterface_TimeManagementType_RealTime:

        *str += "RealTime";

        break;

    }



    if (m_CoordinateSpecified)

    {

        *str += ", CoordinateSystem = ";



        //sprintf(intStr, "%d", m_CoordinateSystem);

        switch (m_CoordinateSystem)

        {

        case SocketInterface_CoordinateSystemType_Cartesian:

            *str += "Cartesian";

            break;



        case SocketInterface_CoordinateSystemType_LatLonAlt:

            *str += "LatLonAlt";

            break;



        case SocketInterface_CoordinateSystemType_GccCartesian:

            *str += "GccCartesian";

            break;

        }

    }



    if (m_SourceResponseMulticastEnabled)

    {

        *str += ", Source Response Multicast enabled";

    }



    if (m_Scenario.size() > 0)

    {

        *str += ", scenario script = \"" + m_Scenario + "\"";

    }

}



void SocketInterface_InitializeSimulationMessage::SetCoordinateSystem(

    SocketInterface_CoordinateSystemType coordinateSystem)

{

    m_CoordinateSpecified = TRUE;

    m_CoordinateSystem = coordinateSystem;

}



void SocketInterface_InitializeSimulationMessage::SetScenario(std::string scenario)

{

    m_Scenario = scenario;

}



void SocketInterface_InitializeSimulationMessage::SetSourceResponseMulticast(UInt8 srm)

{

    m_SourceResponseMulticastEnabled = srm;

}



SocketInterface_PauseSimulationMessage::SocketInterface_PauseSimulationMessage()

{

    m_PauseTimeSpecified = FALSE;

    m_PauseTime = 0;

}



SocketInterface_PauseSimulationMessage::SocketInterface_PauseSimulationMessage(SocketInterface_SerializedMessage* serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt32 size;



    m_PauseTimeSpecified = FALSE;

    m_PauseTime = 0;



    if (serialized->m_Size < SOCKET_INTERFACE_MIN_PAUSE_SIZE)

    {

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small");

    }



    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;

    while (serialized->NextOptionHeader(&type, &size))

    {

        switch (type)

        {

        case SocketInterface_OptionType_TimeStamp:

            if (size != sizeof(SocketInterface_TimeType))

            {

                throw SocketInterface_Exception(

                    SocketInterface_ErrorType_InvalidMessage,

                    "Stop time option not 8 bytes");

            }



            m_PauseTimeSpecified = TRUE;

            serialized->ReadData(

                (UInt8*) &m_PauseTime,

                sizeof(SocketInterface_TimeType));

            break;



        default:

            throw SocketInterface_Exception(

                SocketInterface_ErrorType_InvalidMessage,

                "Invalid option type");

            break;

        }

    }

}



UInt32 SocketInterface_PauseSimulationMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_HEADER_SIZE;

    if (m_PauseTimeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_PauseSimulationMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    if (m_PauseTimeSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_PauseTime, sizeof(SocketInterface_TimeType));
    }

    return data;
}

void SocketInterface_PauseSimulationMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];
    *str += "PauseSimulation";

    if (m_PauseTimeSpecified)
    {
        sprintf(str2, ": PauseTime = %f", m_PauseTime);
        *str += str2;
    }
}

void SocketInterface_PauseSimulationMessage::SetPauseTime(SocketInterface_TimeType time)
{
    m_PauseTimeSpecified = TRUE;
    m_PauseTime = time;
}

SocketInterface_BeginWarmupMessage::SocketInterface_BeginWarmupMessage()
{
}

SocketInterface_BeginWarmupMessage::SocketInterface_BeginWarmupMessage(SocketInterface_SerializedMessage* serialized)
{
    SocketInterface_OptionType type;
    UInt32 size;

    if (serialized->m_Size < SOCKET_INTERFACE_MIN_PAUSE_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too small");
    }

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

UInt32 SocketInterface_BeginWarmupMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_HEADER_SIZE;

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_BeginWarmupMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    return data;
}

void SocketInterface_BeginWarmupMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];
    *str += "BeginWarmup";
}

SocketInterface_ExecuteSimulationMessage::SocketInterface_ExecuteSimulationMessage()
{
}

SocketInterface_ExecuteSimulationMessage::SocketInterface_ExecuteSimulationMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    if (serialized->m_Size > SOCKET_INTERFACE_HEADER_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too large");
    }

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    if (serialized->NextOptionHeader(&type, &size))
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Invalid option type");
    }
}

UInt32 SocketInterface_ExecuteSimulationMessage::GetSize()
{
    return SOCKET_INTERFACE_HEADER_SIZE;
}

SocketInterface_SerializedMessage* SocketInterface_ExecuteSimulationMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());
    return data;
}

void SocketInterface_ExecuteSimulationMessage::Print(std::string* str)
{
    *str += "ExecuteSimulation";
}

SocketInterface_StopSimulationMessage::SocketInterface_StopSimulationMessage()
{
    m_StopSpecified = FALSE;
    m_StopTime = 0;
    m_WaitSpecified = FALSE;
    m_WaitForResponses = 0;
}

SocketInterface_StopSimulationMessage::SocketInterface_StopSimulationMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_StopSpecified = FALSE;
    m_StopTime = 0;
    m_WaitSpecified = FALSE;
    m_WaitForResponses = 0;

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Stop time option not 8 bytes");
            }

            m_StopSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_StopTime,
                sizeof(SocketInterface_TimeType));
            break;

        case SocketInterface_OptionType_WaitForResponses:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Stop time option not 8 bytes");
            }

            m_WaitSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_WaitForResponses,
                sizeof(UInt8));
            break;

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

UInt32 SocketInterface_StopSimulationMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_HEADER_SIZE;

    if (m_StopSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    if (m_WaitSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_StopSimulationMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    if (m_StopSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_StopTime, sizeof(SocketInterface_TimeType));
    }

    if (m_WaitSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_WaitForResponses, sizeof(UInt8));
        data->AddData(&m_WaitForResponses, sizeof(UInt8));
    }

    return data;
}

void SocketInterface_StopSimulationMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];
    BOOL first = true;

    *str += "StopSimulation";

    if (m_StopSpecified)
    {
        if (first)
        {
            *str += ": ";
            first = FALSE;
        }

        sprintf(str2, "StopTime = %f", m_StopTime);
        *str += str2;
    }

    if (m_WaitSpecified)
    {
        if (first)
        {
            *str += ": ";
            first = FALSE;
        }
        else
        {
            *str += ", ";
        }

        sprintf(str2, "WaitForResponses = %d", m_WaitForResponses);
        *str += str2;
    }
}

void SocketInterface_StopSimulationMessage::SetStopTime(SocketInterface_TimeType stopTime)
{
    m_StopSpecified = TRUE;
    m_StopTime = stopTime;
}

void SocketInterface_StopSimulationMessage::SetWaitForResponses(UInt8 waitForResponses)
{
    m_WaitSpecified = TRUE;
    m_WaitForResponses = waitForResponses;
}

SocketInterface_ResetSimulationMessage::SocketInterface_ResetSimulationMessage()
{
    m_ResetSpecified = FALSE;
    m_ResetTime = 0;
}

SocketInterface_ResetSimulationMessage::SocketInterface_ResetSimulationMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_ResetSpecified = FALSE;
    m_ResetTime = 0;

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Reset time option not 8 bytes");
            }

            m_ResetSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_ResetTime,
                sizeof(SocketInterface_TimeType));
            break;

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

UInt32 SocketInterface_ResetSimulationMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_HEADER_SIZE;

    if (m_ResetSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_ResetSimulationMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    if (m_ResetSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_ResetTime, sizeof(SocketInterface_TimeType));
    }

    return data;
}

void SocketInterface_ResetSimulationMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];
    BOOL first = true;

    *str += "ResetSimulation";

    if (m_ResetSpecified)
    {
        sprintf(str2, ": ResetTime = %f", m_ResetTime);
        *str += str2;
    }
}

void SocketInterface_ResetSimulationMessage::SetResetTime(SocketInterface_TimeType resetTime)
{
    m_ResetSpecified = TRUE;
    m_ResetTime = resetTime;
}

SocketInterface_AdvanceTimeMessage::SocketInterface_AdvanceTimeMessage(SocketInterface_TimeType timeAllowance)
{
    m_TimeAllowance = timeAllowance;
}

SocketInterface_AdvanceTimeMessage::SocketInterface_AdvanceTimeMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    if (serialized->m_Size > SOCKET_INTERFACE_MIN_ADVANCE_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too large");
    }

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    serialized->ReadData((UInt8*) &m_TimeAllowance, sizeof(SocketInterface_TimeType));

    if (serialized->NextOptionHeader(&type, &size))
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Invalid option type");
    }
}

UInt32 SocketInterface_AdvanceTimeMessage::GetSize()
{
    return SOCKET_INTERFACE_MIN_ADVANCE_SIZE;
}

SocketInterface_SerializedMessage* SocketInterface_AdvanceTimeMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_TimeAllowance, sizeof(SocketInterface_TimeType));

    return data;
}

void SocketInterface_AdvanceTimeMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];

    sprintf(str2, "AdvanceTime: TimeAllowance = %f", m_TimeAllowance);
    *str += str2;
}

SocketInterface_SimulationIdleMessage::SocketInterface_SimulationIdleMessage(SocketInterface_TimeType currentTime)
{
    m_CurrentTime = currentTime;
}

SocketInterface_SimulationIdleMessage::SocketInterface_SimulationIdleMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    if (serialized->m_Size > SOCKET_INTERFACE_MIN_IDLE_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too large");
    }

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    serialized->ReadData((UInt8*) &m_CurrentTime, sizeof(SocketInterface_TimeType));

    if (serialized->NextOptionHeader(&type, &size))
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Invalid option type");
    }
}

UInt32 SocketInterface_SimulationIdleMessage::GetSize()
{
    return SOCKET_INTERFACE_MIN_IDLE_SIZE;
}

SocketInterface_SerializedMessage* SocketInterface_SimulationIdleMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_CurrentTime, sizeof(SocketInterface_TimeType));

    return data;
}

void SocketInterface_SimulationIdleMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];

    sprintf(str2, "SimulationIdle: CurrentTime = %f", m_CurrentTime);
    *str += str2;
}

SocketInterface_CreatePlatformMessage::SocketInterface_CreatePlatformMessage(
    std::string* entityId,
    SocketInterface_Coordinates* position,
    SocketInterface_PlatformStateType state)
{
    m_EntityId = *entityId;
    m_Position = *position;
    m_State = state;

    m_CreateTimeSpecified = FALSE;
    m_CreateTime = 0.0;

    m_TypeSpecified = FALSE;
    m_Type = SocketInterface_PlatformType_Ground;
    
    m_VelocitySpecified = FALSE;
    m_Velocity.m_C1 = 0.0;
    m_Velocity.m_C2 = 0.0;
    m_Velocity.m_C3 = 0.0;
}

SocketInterface_CreatePlatformMessage::SocketInterface_CreatePlatformMessage(
    SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_CreateTimeSpecified = FALSE;
    m_CreateTime = 0.0;

    m_TypeSpecified = FALSE;
    m_Type = SocketInterface_PlatformType_Ground;

    m_VelocitySpecified = FALSE;
    m_Velocity.m_C1 = 0.0;
    m_Velocity.m_C2 = 0.0;
    m_Velocity.m_C3 = 0.0;

    if (serialized->m_Size < SOCKET_INTERFACE_MIN_CREATE_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too small");
    }

    // Read default parameters
    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    serialized->ReadString(&m_EntityId);
    serialized->ReadData((UInt8*) &m_Position.m_C1, sizeof(Float64));
    serialized->ReadData((UInt8*) &m_Position.m_C2, sizeof(Float64));
    serialized->ReadData((UInt8*) &m_Position.m_C3, sizeof(Float64));


    m_State = (SocketInterface_PlatformStateType) data[serialized->m_CurrentSize];
    serialized->m_CurrentSize += sizeof(UInt8);

    if (m_State > SocketInterface_PlatformStateType_Damaged)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Invalid platform state");
    }

    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Create time option not 8 bytes");
            }

            m_CreateTimeSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_CreateTime,
                sizeof(SocketInterface_TimeType));
            break;

        case SocketInterface_OptionType_Type:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Platform type option not 1 byte");
            }

            m_TypeSpecified = TRUE;
            serialized->ReadData((UInt8*) &m_Type, sizeof(UInt8));

            if (m_Type > SocketInterface_PlatformType_Air)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Invalid platform type");
            }
            break;

        case SocketInterface_OptionType_Velocity:
            if (size != sizeof(SocketInterface_Coordinates))
            {
                throw SocketInterface_Exception(
                                    SocketInterface_ErrorType_InvalidMessage,
                                    "Coordinates not 24 bytes");
            }
            m_VelocitySpecified = TRUE;
            serialized->ReadData((UInt8*) &m_Velocity.m_C1, sizeof(Float64));
            serialized->ReadData((UInt8*) &m_Velocity.m_C2, sizeof(Float64));
            serialized->ReadData((UInt8*) &m_Velocity.m_C3, sizeof(Float64));

            break;

        case SocketInterface_OptionType_MulticastGroups:
            {
                UInt8 count = 0;
                std::string group;

                serialized->ReadData(&count, sizeof(UInt8));

                while (count > 0)
                {
                    group.erase();
                    serialized->ReadString(&group);
                    AddMulticastGroup(&group);
                    count--;
                }
                break;
            }

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

UInt32 SocketInterface_CreatePlatformMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_MIN_CREATE_SIZE + sizeof(SocketInterface_PlatformStateType);

    size += m_EntityId.size();

    if (m_CreateTimeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    if (m_TypeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_VelocitySpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_Coordinates);
    }

    if (m_MulticastGroups.size() > 0)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);

        unsigned int i;
        for (i = 0; i < m_MulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_MulticastGroups[i].size();
        }
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_CreatePlatformMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());
    data->AddString(&m_EntityId);
    //data->AddData((UInt8*) &m_Position, sizeof(SocketInterface_Coordinates));
    data->AddData((UInt8*) &m_Position.m_C1, sizeof(Float64));
    data->AddData((UInt8*) &m_Position.m_C2, sizeof(Float64));
    data->AddData((UInt8*) &m_Position.m_C3, sizeof(Float64));
    data->AddData((UInt8*) &m_State, sizeof(SocketInterface_PlatformStateType));

    if (m_CreateTimeSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_CreateTime, sizeof(SocketInterface_TimeType));
    }

    if (m_TypeSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_Type, sizeof(UInt8));
        data->AddData((UInt8*) &m_Type, sizeof(UInt8));
    }

    if (m_VelocitySpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_Velocity, sizeof(SocketInterface_Coordinates));
        data->AddData((UInt8*) &m_Velocity.m_C1, sizeof(Float64));
        data->AddData((UInt8*) &m_Velocity.m_C2, sizeof(Float64));
        data->AddData((UInt8*) &m_Velocity.m_C3, sizeof(Float64));
    }

    if (m_MulticastGroups.size() > 0)
    {
        UInt8 count;
        UInt16 size = 0;

        unsigned int i;
        for (i = 0; i < m_MulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_MulticastGroups[i].size();
        }

        data->AddOptionHeader(SocketInterface_OptionType_MulticastGroups, size + sizeof(UInt8));

        count = m_MulticastGroups.size();
        data->AddData(&count, sizeof(UInt8));

        for (i = 0; i < m_MulticastGroups.size(); i++)
        {
            data->AddString(&m_MulticastGroups[i]);
        }
    }

    return data;
}

void SocketInterface_CreatePlatformMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];

    *str += "CreatePlatform: EntityId = " + m_EntityId;

    sprintf(
        str2,
        ", Position = (%f, %f, %f)",
        m_Position.m_C1,
        m_Position.m_C2,
        m_Position.m_C3);
    *str += str2;

    *str += ", State = ";
    switch (m_State)
    {
    case SocketInterface_PlatformStateType_Undamaged:
        *str += "Undamaged";
        break;

    case SocketInterface_PlatformStateType_Damaged:
        *str += "Damaged";
        break;
    }

    if (m_CreateTimeSpecified)
    {
        sprintf(str2, ", CreateTime = %f", m_CreateTime);
        *str += str2;
    }

    if (m_TypeSpecified)
    {
        sprintf(str2, ", Type = %d", m_Type);
        *str += str2;
    }

    if (m_VelocitySpecified)
    {
        sprintf(
                str2,
                ", Velocity = (%f, %f, %f)",
                m_Velocity.m_C1,
                m_Velocity.m_C2,
                m_Velocity.m_C3);
        *str += str2;
    }

    if (m_MulticastGroups.size() > 0)
    {
        *str += ", MulticastGroups = ";

        unsigned int i;
        for (i = 0; i < m_MulticastGroups.size(); i++)
        {
            if (i > 0)
            {
                *str += " ";
            }

            *str += m_MulticastGroups[i];
        }
    }
}

void SocketInterface_CreatePlatformMessage::SetCreateTime(SocketInterface_TimeType time)
{
    m_CreateTimeSpecified = TRUE;
    m_CreateTime = time;
}

void SocketInterface_CreatePlatformMessage::SetType(SocketInterface_PlatformType type)
{
    m_TypeSpecified = TRUE;
    m_Type = type;
}

void SocketInterface_CreatePlatformMessage::AddMulticastGroup(std::string* group)
{
    m_MulticastGroups.push_back(*group);
}

void SocketInterface_CreatePlatformMessage::SetVelocity(SocketInterface_Coordinates* velocity)
{
    m_VelocitySpecified = TRUE;
    m_Velocity = *velocity;
}

SocketInterface_UpdatePlatformMessage::SocketInterface_UpdatePlatformMessage(
    std::string* entityId)
{
    m_EntityId = *entityId;

    m_UpdateTimeSpecified = FALSE;
    m_UpdateTime = 0.0;

    m_PositionSpecified = FALSE;
    m_Position.m_C1 = 0.0;
    m_Position.m_C2 = 0.0;
    m_Position.m_C3 = 0.0;

    m_StateSpecified = FALSE;
    m_State = SocketInterface_PlatformStateType_Undamaged;

    m_SpeedSpecified = FALSE;
    m_Speed = 0.0;
    
    m_VelocitySpecified = FALSE;
    m_Velocity.m_C1 = 0.0;
    m_Velocity.m_C2 = 0.0;
    m_Velocity.m_C3 = 0.0;
}

SocketInterface_UpdatePlatformMessage::SocketInterface_UpdatePlatformMessage(
    SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_UpdateTimeSpecified = FALSE;
    m_UpdateTime = 0.0;

    m_PositionSpecified = FALSE;
    m_Position.m_C1 = 0.0;
    m_Position.m_C2 = 0.0;
    m_Position.m_C3 = 0.0;

    m_StateSpecified = FALSE;
    m_State = SocketInterface_PlatformStateType_Undamaged;

    m_SpeedSpecified = FALSE;
    m_Speed = 0.0;

    m_VelocitySpecified = FALSE;
    m_Velocity.m_C1 = 0.0;
    m_Velocity.m_C2 = 0.0;
    m_Velocity.m_C3 = 0.0;

    if (serialized->m_Size < SOCKET_INTERFACE_MIN_UPDATE_SIZE)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too small");
    }

    // Read default parameters
    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    serialized->ReadString(&m_EntityId);

    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Update time option not 8 bytes");
            }

            m_UpdateTimeSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_UpdateTime,
                sizeof(SocketInterface_TimeType));
            break;

        case SocketInterface_OptionType_Position:
            if (size != sizeof(SocketInterface_Coordinates))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Coordinates not 24 bytes");
            }

            m_PositionSpecified = TRUE;
            serialized->ReadData((UInt8*) &m_Position.m_C1, sizeof(Float64));
            serialized->ReadData((UInt8*) &m_Position.m_C2, sizeof(Float64));
            serialized->ReadData((UInt8*) &m_Position.m_C3, sizeof(Float64));

            break;

        case SocketInterface_OptionType_State:
            if (size != sizeof(SocketInterface_PlatformStateType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "State option not 1 byte");
            }

            m_StateSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_State,
                sizeof(SocketInterface_PlatformStateType));
            break;

        case SocketInterface_OptionType_Speed:
            if (size != sizeof(Float64))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Speed option not 8 bytes");
            }

            m_SpeedSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Speed,
                sizeof(Float64));
            break;

            case SocketInterface_OptionType_Velocity:
                if (size != sizeof(SocketInterface_Coordinates))
                {
                    throw SocketInterface_Exception(
                                        SocketInterface_ErrorType_InvalidMessage,
                                        "Coordinates not 24 bytes");
                }

                m_VelocitySpecified = TRUE;
                serialized->ReadData((UInt8*) &m_Velocity.m_C1, sizeof(Float64));
                serialized->ReadData((UInt8*) &m_Velocity.m_C2, sizeof(Float64));
                serialized->ReadData((UInt8*) &m_Velocity.m_C3, sizeof(Float64));

                break;

        case SocketInterface_OptionType_JoinMulticastGroups:
            {
                UInt8 count;
                std::string group;

                serialized->ReadData(&count, sizeof(UInt8));

                while (count > 0)
                {
                    group.erase();
                    serialized->ReadString(&group);
                    AddJoinMulticastGroup(&group);
                    count--;
                }
                break;
            }

        case SocketInterface_OptionType_LeaveMulticastGroups:
            {
                UInt8 count;
                std::string group;

                serialized->ReadData(&count, sizeof(UInt8));

                while (count > 0)
                {
                    group.erase();
                    serialized->ReadString(&group);
                    AddLeaveMulticastGroup(&group);
                    count--;
                }
                break;
            }

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

UInt32 SocketInterface_UpdatePlatformMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_MIN_UPDATE_SIZE;

    size += m_EntityId.size();

    if (m_UpdateTimeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    if (m_PositionSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_Coordinates);
    }

    if (m_StateSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_PlatformStateType);
    }

    if (m_SpeedSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(Float64);
    }

    if (m_VelocitySpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_Coordinates);
    }
    if (m_JoinMulticastGroups.size() > 0)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);

        unsigned int i;
        for (i = 0; i < m_JoinMulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_JoinMulticastGroups[i].size();
        }
    }

    if (m_LeaveMulticastGroups.size() > 0)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);

        unsigned int i;
        for (i = 0; i < m_LeaveMulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_LeaveMulticastGroups[i].size();
        }
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_UpdatePlatformMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());
    data->AddString(&m_EntityId);

    if (m_UpdateTimeSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));
        data->AddData((UInt8*) &m_UpdateTime, sizeof(SocketInterface_TimeType));
    }

    if (m_PositionSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_Position, sizeof(SocketInterface_Coordinates));
        //data->AddData((UInt8*) &m_Position, sizeof(SocketInterface_Coordinates));
        data->AddData((UInt8*) &m_Position.m_C1, sizeof(Float64));
        data->AddData((UInt8*) &m_Position.m_C2, sizeof(Float64));
        data->AddData((UInt8*) &m_Position.m_C3, sizeof(Float64));

    }

    if (m_StateSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_State, sizeof(SocketInterface_PlatformStateType));
        data->AddData((UInt8*) &m_State, sizeof(SocketInterface_PlatformStateType));
    }

    if (m_SpeedSpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_Speed, sizeof(Float64));
        data->AddData((UInt8*) &m_Speed, sizeof(Float64));
    }

    if (m_VelocitySpecified)
    {
        data->AddOptionHeader(SocketInterface_OptionType_Velocity, sizeof(SocketInterface_Coordinates));
        data->AddData((UInt8*) &m_Velocity.m_C1, sizeof(Float64));
        data->AddData((UInt8*) &m_Velocity.m_C2, sizeof(Float64));
        data->AddData((UInt8*) &m_Velocity.m_C3, sizeof(Float64));
    }

    if (m_JoinMulticastGroups.size() > 0)
    {
        UInt16 size = 0;

        unsigned int i;
        for (i = 0; i < m_JoinMulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_JoinMulticastGroups[i].size();
        }

        data->AddOptionHeader(SocketInterface_OptionType_JoinMulticastGroups, size + sizeof(UInt8));

        UInt8 count = m_JoinMulticastGroups.size();
        data->AddData(&count, sizeof(UInt8));

        for (i = 0; i < m_JoinMulticastGroups.size(); i++)
        {
            data->AddString(&m_JoinMulticastGroups[i]);
        }
    }

    if (m_LeaveMulticastGroups.size() > 0)
    {
        UInt16 size = 0;

        unsigned int i;
        for (i = 0; i < m_LeaveMulticastGroups.size(); i++)
        {
            size += SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_LeaveMulticastGroups[i].size();
        }

        data->AddOptionHeader(SocketInterface_OptionType_LeaveMulticastGroups, size + sizeof(UInt8));

        UInt8 count = m_LeaveMulticastGroups.size();
        data->AddData(&count, sizeof(UInt8));

        for (i = 0; i < m_LeaveMulticastGroups.size(); i++)
        {
            data->AddString(&m_LeaveMulticastGroups[i]);
        }
    }

    return data;
}

void SocketInterface_UpdatePlatformMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];

    *str += "UpdatePlatform: EntityId = " + m_EntityId;

    if (m_UpdateTimeSpecified)
    {
        sprintf(str2, ", UpdateTime = %f", m_UpdateTime);
        *str += str2;
    }

    if (m_PositionSpecified)
    {
        sprintf(
            str2,
            ", Position = (%f, %f, %f)",
            m_Position.m_C1,
            m_Position.m_C2,
            m_Position.m_C3);
        *str += str2;
    }

    if (m_StateSpecified)
    {
        *str += ", State = ";
        switch (m_State)
        {
        case SocketInterface_PlatformStateType_Undamaged:
            *str += "Undamaged";
            break;

        case SocketInterface_PlatformStateType_Damaged:
            *str += "Damaged";
            break;
        }

    }

    if (m_SpeedSpecified)
    {
        sprintf(str2, ", Speed = %f", m_Speed);
        *str += str2;
    }

    if (m_VelocitySpecified)
    {
        sprintf(
                str2,
                ", Velocity = (%f, %f, %f)",
                m_Velocity.m_C1,
                m_Velocity.m_C2,
                m_Velocity.m_C3);
        *str += str2;
    }
    
    if (m_JoinMulticastGroups.size() > 0)
    {
        *str += ", JoinMulticastGroups = ";

        unsigned int i;
        for (i = 0; i < m_JoinMulticastGroups.size(); i++)
        {
            if (i > 0)
            {
                *str += " ";
            }

            *str += m_JoinMulticastGroups[i];
        }
    }

    if (m_LeaveMulticastGroups.size() > 0)
    {
        *str += ", LeaveMulticastGroups = ";

        unsigned int i;
        for (i = 0; i < m_LeaveMulticastGroups.size(); i++)
        {
            if (i > 0)
            {
                *str += " ";
            }

            *str += m_LeaveMulticastGroups[i];
        }
    }
}

void SocketInterface_UpdatePlatformMessage::SetUpdateTime(SocketInterface_TimeType time)
{
    m_UpdateTimeSpecified = TRUE;
    m_UpdateTime = time;
}

void SocketInterface_UpdatePlatformMessage::SetPosition(SocketInterface_Coordinates* position)
{
    m_PositionSpecified = TRUE;
    m_Position = *position;
}

void SocketInterface_UpdatePlatformMessage::SetState(SocketInterface_PlatformStateType state)
{
    m_StateSpecified = TRUE;
    m_State = state;
}

void SocketInterface_UpdatePlatformMessage::SetSpeed(Float64 speed)
{
    m_SpeedSpecified = TRUE;
    m_Speed = speed;
}

void SocketInterface_UpdatePlatformMessage::SetVelocity(SocketInterface_Coordinates* velocity)
{
    m_VelocitySpecified = TRUE;
    m_Velocity = *velocity;
}

void SocketInterface_UpdatePlatformMessage::AddJoinMulticastGroup(std::string* group)
{
    m_JoinMulticastGroups.push_back(*group);
}

void SocketInterface_UpdatePlatformMessage::AddLeaveMulticastGroup(std::string* group)
{
    m_LeaveMulticastGroups.push_back(*group);
}



SocketInterface_ErrorMessage::SocketInterface_ErrorMessage(UInt8 code, std::string* error)
{
    m_Code = code;
    m_Error = *error;

    m_MessageSpecified = FALSE;
    m_OriginatingMessage = NULL;
}

SocketInterface_ErrorMessage::SocketInterface_ErrorMessage(SocketInterface_SerializedMessage* serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_MessageSpecified = FALSE;
    m_OriginatingMessage = NULL;

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;

    serialized->ReadData(&m_Code, sizeof(UInt8));
    serialized->ReadString(&m_Error);

    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_OriginatingMessage:
            {
                if (size < SOCKET_INTERFACE_HEADER_SIZE)
                {
                    throw SocketInterface_Exception(
                        SocketInterface_ErrorType_InvalidMessage,
                        "Message must be at least 4 bytes");
                }

                m_MessageSpecified = TRUE;

                // Extract serialized message from option, do not swap
                // bytes
                m_OriginatingMessage = new SocketInterface_SerializedMessage;
                m_OriginatingMessage->Resize(size);
                serialized->ReadData(
                    m_OriginatingMessage->m_Data,
                    size,
                    FALSE);
                break;
            }

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid option type");
            break;
        }
    }
}

SocketInterface_ErrorMessage::SocketInterface_ErrorMessage(const SocketInterface_ErrorMessage& error)
{
    // Use duplicate on the inner message.
    *this = error;
    if (error.m_MessageSpecified && error.m_OriginatingMessage !=  NULL)
    {
        SetSerializedMessage(error.m_OriginatingMessage);    
    }
}

SocketInterface_ErrorMessage::~SocketInterface_ErrorMessage()
{
    if (m_MessageSpecified)
    {
        assert(m_OriginatingMessage != NULL);
        delete m_OriginatingMessage;
    }
}

UInt32 SocketInterface_ErrorMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_MIN_ERROR_SIZE;

    size += m_Error.size();

    // If the error includes a message, include the size of that message
    // in its serialized form
    if (m_MessageSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + m_OriginatingMessage->m_Size;
    }

    return size;
}

SocketInterface_SerializedMessage* SocketInterface_ErrorMessage::Serialize()
{
    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());
    data->AddData(&m_Code, sizeof(UInt8));
    data->AddString(&m_Error);

    if (m_MessageSpecified)
    {
        // Add the serialized message, do not swap bytes
        data->AddOptionHeader(SocketInterface_OptionType_OriginatingMessage, m_OriginatingMessage->m_Size);
        data->AddData(m_OriginatingMessage->m_Data, m_OriginatingMessage->m_Size, FALSE);
    }

    return data;
}

void SocketInterface_ErrorMessage::Print(std::string* str)
{
    char str2[MAX_STRING_LENGTH];

    *str += "SocketInterface_ErrorMessage: ";

    switch(m_Code)
    {
    case SocketInterface_ErrorType_NoError:
        *str += "NoError";
        break;

    case SocketInterface_ErrorType_ConfigurationError:
        *str += "ConfigurationError";
        break;

    case SocketInterface_ErrorType_MessageCreationError:
        *str += "MessageCreationError";
        break;

    case SocketInterface_ErrorType_InvalidMessage:
        *str += "InvalidMessage";
        break;

    case SocketInterface_ErrorType_InvalidTransition:
        *str += "InvalidTransition";
        break;

    case SocketInterface_ErrorType_InvalidEntityId:
        *str += "InvalidEntityId";
        break;

    case SocketInterface_ErrorType_InvalidState:
        *str += "InvalidState";
        break;

    case SocketInterface_ErrorType_InvalidCoordinates:
        *str += "InvalidCoordinates";
        break;

    case SocketInterface_ErrorType_InvalidReceiverId:
        *str += "InvalidReceiverId";
        break;

    case SocketInterface_ErrorType_EmptyReceiverList:
        *str += "EmptyReceiverList";
        break;

    case SocketInterface_ErrorType_InvalidSenderId:
        *str += "InvalidSenderId";
        break;

    case SocketInterface_ErrorType_InvalidSendTime:
        *str += "InvalidSendTime";
        break;

    case SocketInterface_ErrorType_InvalidStopTime:
        *str += "InvalidStopTime";
        break;

    case SocketInterface_ErrorType_InvalidSocketId:
        *str += "InvalidSocketId";
        break;

    case SocketInterface_ErrorType_QualnetWarning:
        *str += "QualnetWarning";
        break;

    case SocketInterface_ErrorType_QualnetError:
        *str += "QualnetError";
        break;

    case SocketInterface_ErrorType_SocketError:
        *str += "SocketError";
        break;

    case SocketInterface_ErrorType_InvalidOperationType:
        *str += "InvalidOperationType";
        break;

    case SocketInterface_ErrorType_EmptyScenarioString:
        *str += "EmptyScenarioString";
        break;

    case SocketInterface_ErrorType_InvalidDynamicCommand:
        *str += "InvalidDynamicCommand";
        break;

    case SocketInterface_ErrorType_InvalidProtocol:
        *str += "InvalidProtocol";
        break;

    case SocketInterface_ErrorType_InvalidGroup:
        *str += "InvalidGroup";
        break;

    case SocketInterface_ErrorType_InvalidSimulationState:
        *str += "InvalidSimulationState";
        break;

    default:
        *str += "UnknownError";
        break;
    }

    *str += ", Error = " + m_Error;

    if (m_MessageSpecified)
    {
        std::string str3;

        // Attempt to deserialize the message.  If it can't be deserialized
        // then print it as binary data.
        SocketInterface_Message* message;
        try
        {
            message = m_OriginatingMessage->Deserialize();
            message->Print(&str3);
            delete message;
        }
        catch (SocketInterface_Exception& e)
        {
            m_OriginatingMessage->Print(&str3);
        }

        *str += ", OriginatingMessage = " + str3;
    }
}

void SocketInterface_ErrorMessage::SetMessage(SocketInterface_Message* message)
{
    m_MessageSpecified = TRUE;
    m_OriginatingMessage = message->Serialize();
}

void SocketInterface_ErrorMessage::SetSerializedMessage(SocketInterface_SerializedMessage* message)
{
    m_MessageSpecified = TRUE;
    m_OriginatingMessage = message->Duplicate();
}

SocketInterface_CommEffectsRequestMessage::SocketInterface_CommEffectsRequestMessage(UInt64 Id1, 
                                                               UInt64 Id2,
                                                               SocketInterface_ProtocolType protocol, 
                                                               UInt32 size,
                                                               std::string *sender, 
                                                               std::string *receiver)
{
    m_Id1 = Id1;
    m_Id2 = Id2;
    m_Protocol = protocol;
    m_Size = size;
    m_SenderId = *sender;
    m_ReceiverId = *receiver;

    m_SendTimeSpecified = FALSE;
    m_SendTime = 0.0;

    m_PrecedenceSpecified = FALSE;
    m_Precedence = SocketInterface_PrecedenceType_Routine;

    m_DscpSpecified = FALSE;
    m_Dscp = 0;

    m_TosSpecified = FALSE;
    m_Tos = 0;

    m_TtlSpecified = FALSE;
    m_TTL = MTSDEFTTL;

    m_Description = "";

    m_FailureTimeoutSpecified = FALSE;
    m_FailureTimeout = 0.0;

    m_IdleWhenResponseSentSpecified = FALSE;
    m_IdleWhenResponseSent = 0;

    m_PhyTransmissionTypeSpecified = FALSE;
    m_PhyTransmissionType = 0;

    m_WaveformSpecified = FALSE;
    m_Waveform = 0;

    m_RouteSpecified = FALSE;
    m_Route = 0;
}

SocketInterface_CommEffectsRequestMessage::SocketInterface_CommEffectsRequestMessage(SocketInterface_SerializedMessage *serialized)
{
    UInt8* data = serialized->m_Data;
    SocketInterface_OptionType type;
    UInt32 size;

    m_SendTimeSpecified = FALSE;
    m_SendTime = 0.0;

    m_PrecedenceSpecified = FALSE;
    m_Precedence = SocketInterface_PrecedenceType_Routine;

    m_DscpSpecified = FALSE;
    m_Dscp = 0;

    m_TosSpecified = FALSE;
    m_Tos = 0;

    m_TtlSpecified = FALSE;
    m_TTL = MTSDEFTTL;

    m_Description = "";

    m_FailureTimeoutSpecified = FALSE;
    m_FailureTimeout = 0.0;

    m_IdleWhenResponseSentSpecified = FALSE;
    m_IdleWhenResponseSent = 0;

    m_PhyTransmissionTypeSpecified = FALSE;
    m_PhyTransmissionType = 0;

    m_WaveformSpecified = FALSE;
    m_Waveform = 0;

    m_RouteSpecified = FALSE;
    m_Route = 0;

    if (serialized->m_Size < SOCKET_INTERFACE_MIN_COMM_REQUEST_SIZE)
    {
        // EXCEPTION, message too small
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Message too small");
    }

    // Read default parameters
    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;
    serialized->ReadData((UInt8*) &m_Id1, sizeof (UInt64));
    serialized->ReadData((UInt8*) &m_Id2, sizeof (UInt64));
    serialized->ReadData((UInt8*) &m_Protocol, sizeof(SocketInterface_ProtocolType));
    serialized->ReadData((UInt8*) &m_Size, sizeof (UInt32));

    serialized->ReadString(&m_SenderId);
    serialized->ReadString(&m_ReceiverId);

    // Check OPTION parameters
    while (serialized->NextOptionHeader(&type, &size))
    {
        switch (type)
        {
        case SocketInterface_OptionType_TimeStamp:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Request time option not 8 bytes");
            }
            m_SendTimeSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_SendTime,
                sizeof(SocketInterface_TimeType));
            break;

        case SocketInterface_OptionType_PrecedenceOption:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Precedence option not 1 byte");
            }
            m_PrecedenceSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Precedence,
                sizeof(UInt8));

            if (m_Precedence > SocketInterface_PrecedenceType_NetControl)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Invalid precedence value");
            }
            break;

        case SocketInterface_OptionType_Dscp:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "DSCP option not 1 byte");
            }
            m_DscpSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Dscp,
                sizeof(UInt8));

            if (m_Dscp > 63)
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Invalid DSCP value");
            }
            break;

        case SocketInterface_OptionType_Tos:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "TOS option not 1 byte");
            }
            m_TosSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Tos,
                sizeof(UInt8));
            break;

        case SocketInterface_OptionType_TTL:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "TTL option not 1 byte");
            }
            m_TtlSpecified = TRUE;
            serialized->ReadData((UInt8*) &m_TTL,
                                   sizeof(UInt8));
            break;

        case SocketInterface_OptionType_Description:
            serialized->ReadString(&m_Description);
            break;

        case SocketInterface_OptionType_FailureTimeout:
            if (size != sizeof(SocketInterface_TimeType))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Failure timeout option not 8 bytes");
            }
            m_FailureTimeoutSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_FailureTimeout,
                sizeof(SocketInterface_TimeType));
            break;

        case SocketInterface_OptionType_IdleWhenResponseSent:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "IdleWhenResponseSent option not 1 byte");
            }
            m_IdleWhenResponseSentSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_IdleWhenResponseSent,
                sizeof(UInt8));
            break;

        case SocketInterface_OptionType_PhyTransmissionType:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Phy transmission type option not 1 byte");
            }
            m_PhyTransmissionTypeSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_PhyTransmissionType,
                sizeof(UInt8));
            break;

        case SocketInterface_OptionType_Waveform:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Waveform option not 1 byte");
            }
            m_WaveformSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Waveform,
                sizeof(UInt8));
            break;

        case SocketInterface_OptionType_Route:
            if (size != sizeof(UInt8))
            {
                throw SocketInterface_Exception(
                    SocketInterface_ErrorType_InvalidMessage,
                    "Route option not 1 byte");
            }
            m_RouteSpecified = TRUE;
            serialized->ReadData(
                (UInt8*) &m_Route,
                sizeof(UInt8));
            break;

        default:
            throw SocketInterface_Exception(
                SocketInterface_ErrorType_InvalidMessage,
                "Invalid Option type");
            break;
        }
    }

    // Verify dscp and precedence are both not specified
    if (m_DscpSpecified && m_PrecedenceSpecified
        || m_DscpSpecified && m_TosSpecified
        || m_TosSpecified && m_PrecedenceSpecified)
    {
        throw SocketInterface_Exception(
            SocketInterface_ErrorType_InvalidMessage,
            "Only one of Precedence, DSCP and TOS may be specified in the same message");
    }
}

UInt32 SocketInterface_CommEffectsRequestMessage::GetSize()
{
    UInt32 size = SOCKET_INTERFACE_MIN_COMM_REQUEST_SIZE + sizeof(SocketInterface_ProtocolType);

    size += m_SenderId.size();
    size += m_ReceiverId.size();

    if (m_SendTimeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    if (m_PrecedenceSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_DscpSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_TosSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_TtlSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_Description.size() > 0)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + 
            SOCKET_INTERFACE_STRING_LENGTH_SIZE +
            m_Description.size();
    }

    if (m_FailureTimeoutSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(SocketInterface_TimeType);
    }

    if (m_IdleWhenResponseSentSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_PhyTransmissionTypeSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_WaveformSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    if (m_RouteSpecified)
    {
        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE + sizeof(UInt8);
    }

    return size;
}

void SocketInterface_CommEffectsRequestMessage::Print(std::string *str)
{
    char str2[MAX_STRING_LENGTH];

    sprintf (str2, "CommEffectsRequest: Id1 = %d", m_Id1);
    *str += str2;

    sprintf (str2, ", Id2 = %d", m_Id2);
    *str += str2;

    switch (m_Protocol)
    {
    case SocketInterface_ProtocolType_ProtocolTCP:
        sprintf (str2, ", Protocol = TCP");
        *str += str2;
        break;

    case SocketInterface_ProtocolType_ProtocolUDP:
        sprintf (str2, ", Protocol = UDP");
        *str += str2;
        break;

    case SocketInterface_ProtocolType_ProtocolNetwork:
        sprintf (str2, ", Protocol = Network");
        *str += str2;
        break;
    }

    sprintf(str2, ", Size = %d", m_Size);
    *str += str2;

    *str += ", Sender = " + m_SenderId;
    *str += ", Receiver = " + m_ReceiverId;

    if (m_SendTimeSpecified)
    {
        sprintf(str2, ", RequestTime = %f", m_SendTime);
        *str += str2;
    }

    if (m_PrecedenceSpecified)
    {
        *str += ", Priority = ";
        switch (m_Precedence)
        {
        case SocketInterface_PrecedenceType_Routine:
            *str += "Routine";
            break;

        case SocketInterface_PrecedenceType_Priority:
            *str += "Priority";
            break;

        case SocketInterface_PrecedenceType_Immediate:
            *str += "Immediate";
            break;

        case SocketInterface_PrecedenceType_Flash:
            *str += "Flash";
            break;

        case SocketInterface_PrecedenceType_FlashOverride:
            *str += "FlashOverride";
            break;

        case SocketInterface_PrecedenceType_Critical:
            *str += "Critical";
            break;

        case SocketInterface_PrecedenceType_InternetControl:
            *str += "InternetControl";
            break;

        case SocketInterface_PrecedenceType_NetControl:
            *str += "NetControl";
            break;

        default:
            *str += "???Unknown???";
            break;
        }
    }

    if (m_DscpSpecified)

    {

        sprintf(str2, ", Dscp = %d", m_Dscp);

        *str += str2;

    }



    if (m_TosSpecified)

    {

        sprintf(str2, ", Tos = %d", m_Tos);

        *str += str2;

    }



    if (m_TtlSpecified)

    {

        sprintf(str2, ", TTL = %d", m_TTL);

        *str += str2;

    }



    if (m_Description.size() > 0)

    {

        *str += ", Description = " + m_Description;

    }



    if (m_FailureTimeoutSpecified)

    {

        sprintf(str2, ", FailureTimeout = %f", m_FailureTimeout);

        *str += str2;

    }



    if (m_IdleWhenResponseSentSpecified)

    {

        sprintf(str2, ", IdleWhenResponseSent = %d", m_IdleWhenResponseSent);

        *str += str2;

    }



    if (m_PhyTransmissionTypeSpecified)

    {

        sprintf(str2, ", PhyTransmissionType = %d", m_PhyTransmissionType);

        *str += str2;

    }



    if (m_WaveformSpecified)

    {

        *str += ", Waveform = ";

        switch (m_Waveform)

        {

        case SocketInterface_WaveformType_WNW:

            *str += "WNW";

            break;



        case SocketInterface_WaveformType_SRW:

            *str += "SRW";

            break;



        default:

            *str += "???Unknown???";

            break;

        }

    }



    if (m_RouteSpecified)

    {

        *str += ", Route = ";

        switch (m_Route)

        {

        case SocketInterface_RouteType_USI:

            *str += "USI";

            break;



        case SocketInterface_RouteType_Secret:

            *str += "Secret";

            break;



        default:

            *str += "???Unknown???";

            break;

        }

    }

}



void SocketInterface_CommEffectsRequestMessage::SetPrecedence(SocketInterface_PrecedenceType precedence)

{

    m_PrecedenceSpecified = TRUE;

    m_Precedence = precedence;

}



void SocketInterface_CommEffectsRequestMessage::SetDscp(UInt8 dscp)

{

    m_DscpSpecified = TRUE;

    m_Dscp = dscp;

}



void SocketInterface_CommEffectsRequestMessage::SetTos(UInt8 tos)

{

    m_TosSpecified = TRUE;

    m_Tos = tos;

}



void SocketInterface_CommEffectsRequestMessage::SetTTL(UInt8 ttl)

{

    m_TtlSpecified = TRUE;

    m_TTL = ttl;

}



void SocketInterface_CommEffectsRequestMessage::SetDescription(std::string description)

{

    m_Description = description;

}



void SocketInterface_CommEffectsRequestMessage::SetSendTime(SocketInterface_TimeType time)

{

    m_SendTimeSpecified = TRUE;

    m_SendTime = time;

}



void SocketInterface_CommEffectsRequestMessage::SetFailureTimeout(SocketInterface_TimeType failureTime)

{

    m_FailureTimeoutSpecified = TRUE;

    m_FailureTimeout = failureTime;

}



void SocketInterface_CommEffectsRequestMessage::SetIdleWhenResponseSent(UInt8 idleWhenResponseSent)

{

    m_IdleWhenResponseSentSpecified = TRUE;

    m_IdleWhenResponseSent = idleWhenResponseSent;

}



void SocketInterface_CommEffectsRequestMessage::SetPhyTransmissionType(UInt8 transmissionType)

{

    m_PhyTransmissionTypeSpecified = TRUE;

    m_PhyTransmissionType = transmissionType;

}



void SocketInterface_CommEffectsRequestMessage::SetWaveform(SocketInterface_WaveformType waveform)

{

    m_WaveformSpecified = TRUE;

    m_Waveform = waveform;

}



void SocketInterface_CommEffectsRequestMessage::SetRoute(SocketInterface_RouteType route)

{

    m_RouteSpecified = TRUE;

    m_Route = route;

}



SocketInterface_SerializedMessage* SocketInterface_CommEffectsRequestMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;



    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_Id1, sizeof(UInt64));

    data->AddData((UInt8*) &m_Id2, sizeof(UInt64));

    data->AddData((UInt8*) &m_Protocol, sizeof(SocketInterface_ProtocolType));

    data->AddData((UInt8*) &m_Size, sizeof(UInt32));

    data->AddString(&m_SenderId);

    data->AddString(&m_ReceiverId);



    //take care Option part

    if (m_SendTimeSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_TimeStamp, sizeof(SocketInterface_TimeType));

        data->AddData((UInt8*) &m_SendTime, sizeof(SocketInterface_TimeType));

    }



    if (m_PrecedenceSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_PrecedenceOption, sizeof(UInt8));

        data->AddData((UInt8*) &m_Precedence, sizeof(UInt8));

    }



    if (m_DscpSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_Dscp, sizeof(UInt8));

        data->AddData((UInt8*) &m_Dscp, sizeof(UInt8));

    }



    if (m_TosSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_Tos, sizeof(UInt8));

        data->AddData((UInt8*) &m_Tos, sizeof(UInt8));

    }



    if (m_TtlSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_TTL, sizeof(UInt8));

        data->AddData((UInt8*) &m_TTL, sizeof(UInt8));

    }



    if (m_Description.size() > 0)

    {

        data->AddOptionHeader(

            SocketInterface_OptionType_Description,

            SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_Description.size());

        data->AddString(&m_Description);

    }



    if (m_FailureTimeoutSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_FailureTimeout, sizeof(SocketInterface_TimeType));

        data->AddData((UInt8*) &m_FailureTimeout, sizeof(SocketInterface_TimeType));

    }



    if (m_IdleWhenResponseSentSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_IdleWhenResponseSent, sizeof(UInt8));

        data->AddData((UInt8*) &m_IdleWhenResponseSent, sizeof(UInt8));

    }



    if (m_PhyTransmissionTypeSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_PhyTransmissionType, sizeof(UInt8));

        data->AddData((UInt8*) &m_PhyTransmissionType, sizeof(UInt8));

    }



    if (m_WaveformSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_Waveform, sizeof(UInt8));

        data->AddData((UInt8*) &m_Waveform, sizeof(UInt8));

    }



    if (m_RouteSpecified)

    {

        data->AddOptionHeader(SocketInterface_OptionType_Route, sizeof(UInt8));

        data->AddData((UInt8*) &m_Route, sizeof(UInt8));

    }



    return data;

}



SocketInterface_CommEffectsResponseMessage::SocketInterface_CommEffectsResponseMessage(UInt64 id1, 

                                                                 UInt64 id2,

                                                                 std::string* sender,

                                                                 std::string* receiver,

                                                                 SocketInterface_Status status,

                                                                 SocketInterface_TimeType receiveTime,

                                                                 SocketInterface_TimeType latency)

{

    m_Id1 = id1;

    m_Id2 = id2;

    m_SenderId = *sender;

    m_ReceiverId = *receiver;

    m_Status = status;

    m_ReceiveTime = receiveTime;

    m_Latency = latency;



    m_Description = "";

}



SocketInterface_CommEffectsResponseMessage::SocketInterface_CommEffectsResponseMessage(SocketInterface_SerializedMessage* serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt32 size;



    if (serialized->m_Size < SOCKET_INTERFACE_MIN_COMM_RESPONSE_SIZE)

    {

        //EXCEPTION, message too small

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small");

    }

    //Read default parameters

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;

    serialized->ReadData((UInt8*) &m_Id1, sizeof(UInt64));

    serialized->ReadData((UInt8*) &m_Id2, sizeof(UInt64));



    serialized->ReadString(&m_SenderId);

    serialized->ReadString(&m_ReceiverId);



    m_Status = (SocketInterface_Status) data[serialized->m_CurrentSize];

    serialized->m_CurrentSize += sizeof(SocketInterface_Status);



    serialized->ReadData((UInt8*) &m_ReceiveTime, sizeof(SocketInterface_TimeType));

    serialized->ReadData((UInt8*) &m_Latency, sizeof(SocketInterface_TimeType));



    while (serialized->NextOptionHeader(&type, &size))

    {

        switch (type)

        {

        case SocketInterface_OptionType_Description:

            serialized->ReadString(&m_Description);

            break;



        default:

            throw SocketInterface_Exception(

                SocketInterface_ErrorType_InvalidMessage,

                "Invalid Option type");

            break;

        }

    }

}





UInt32 SocketInterface_CommEffectsResponseMessage::GetSize()

{

    UInt32 size = SOCKET_INTERFACE_MIN_COMM_RESPONSE_SIZE + sizeof(SocketInterface_Status);



    size += m_SenderId.size();

    size += m_ReceiverId.size();



    if (m_Description.size() > 0)

    {

        size += SOCKET_INTERFACE_OPTION_HEADER_SIZE +

            SOCKET_INTERFACE_STRING_LENGTH_SIZE +

            m_Description.size();

    }

    return size;

}



void SocketInterface_CommEffectsResponseMessage::SetDescription(std::string description)

{

    m_Description = description;

}



void SocketInterface_CommEffectsResponseMessage::Print(std::string *str)

{

    char str2[MAX_STRING_LENGTH];



    sprintf (str2, "CommEffectsResponse: Id1 = %d", m_Id1);

    *str += str2;



    sprintf (str2, ", Id2 = %d", m_Id2);

    *str += str2;



    *str += ", Sender = " + m_SenderId;

    *str += ", Receiver = " + m_ReceiverId;



    switch(m_Status)

    {

    case SocketInterface_Status_Success:

        *str += ", Status = Success";

        break;



    case SocketInterface_Status_Failure:

        *str += ", Status = Failure";

        break;

    }



    sprintf(str2, ", ReceiveTime = %f", m_ReceiveTime);

    *str += str2;



    sprintf(str2, ", Latency = %f", m_Latency);

    *str += str2;



    if (m_Description.size() > 0)

    {

        *str += ", Description = " + m_Description;

    }

}



SocketInterface_SerializedMessage* SocketInterface_CommEffectsResponseMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;



    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_Id1, sizeof(UInt64));

    data->AddData((UInt8*) &m_Id2, sizeof(UInt64));

    data->AddString(&m_SenderId);

    data->AddString(&m_ReceiverId);

    data->AddData((UInt8*) &m_Status, sizeof(SocketInterface_Status));

    data->AddData((UInt8*) &m_ReceiveTime, sizeof(SocketInterface_TimeType));

    data->AddData((UInt8*) &m_Latency, sizeof(SocketInterface_TimeType));



    //Take care of Optional data

    if (m_Description.size() > 0)

    {

        data->AddOptionHeader(

            SocketInterface_OptionType_Description,

            SOCKET_INTERFACE_STRING_LENGTH_SIZE + m_Description.size());

        data->AddString(&m_Description);

    }



    return data;

}



SocketInterface_DynamicCommandMessage::SocketInterface_DynamicCommandMessage(SocketInterface_OperationType type,

                                                     std::string *path, 

                                                     std::string *args)

{

    m_Type = type;

    m_Path = *path;

    m_Args = *args;

}



SocketInterface_DynamicCommandMessage::SocketInterface_DynamicCommandMessage(SocketInterface_SerializedMessage *serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt16 size;



    if (serialized->m_Size < SOCKET_INTERFACE_MIN_DYNAMIC_COMMAND_SIZE)

    {

        //EXCEPTION, message too small

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small");

    }

    //Read default parameters

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;

    m_Type = (SocketInterface_OperationType) data[serialized->m_CurrentSize ];

    serialized->m_CurrentSize += sizeof(SocketInterface_OperationType);

    serialized->ReadString(&m_Path);

    serialized->ReadString(&m_Args);

}



UInt32 SocketInterface_DynamicCommandMessage::GetSize()

{

    UInt32 size = SOCKET_INTERFACE_MIN_DYNAMIC_COMMAND_SIZE + sizeof(SocketInterface_OperationType);



    size += m_Path.size();

    size += m_Args.size();



    return size;

}



SocketInterface_SerializedMessage* SocketInterface_DynamicCommandMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;



    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_Type, sizeof(SocketInterface_OperationType));

    data->AddString(&m_Path);

    data->AddString(&m_Args);



    return data;

}



void SocketInterface_DynamicCommandMessage::Print(std::string *str)

{

    char str2[MAX_STRING_LENGTH];

    sprintf (str2, "DynamicCommand: Type = ");

    *str += str2;

    switch (m_Type)

    {

    case SocketInterface_OperationType_Read:

        *str += "Read";

        break;



    case SocketInterface_OperationType_Write:

        *str += "Write";

        break;



    case SocketInterface_OperationType_Execute:

        *str += "Execute";

        break;

    }



    *str += ", Path = " + m_Path;

    if (m_Args.size() > 0)

    {

        *str += ", Args = " + m_Args;

    }

}



SocketInterface_DynamicResponseMessage::SocketInterface_DynamicResponseMessage(SocketInterface_OperationType type, 

                                                       std::string *path, 

                                                       std::string *args, 

                                                       std::string *output)

{

    m_Type = type;

    m_Path = *path;

    m_Args = *args;

    m_Output = *output;

}



SocketInterface_DynamicResponseMessage::SocketInterface_DynamicResponseMessage(SocketInterface_SerializedMessage *serialized)

{

    UInt8* data = serialized->m_Data;

    SocketInterface_OptionType type;

    UInt16 size;



    if (serialized->m_Size < SOCKET_INTERFACE_MIN_DYNAMIC_RESPONSE_SIZE)

    {

        //EXCEPTION, message too small

        throw SocketInterface_Exception(

            SocketInterface_ErrorType_InvalidMessage,

            "Message too small");

    }

    //Read default parameters

    serialized->m_CurrentSize = SOCKET_INTERFACE_HEADER_SIZE;

    m_Type = (SocketInterface_OperationType) data[serialized->m_CurrentSize ];

    serialized->m_CurrentSize += sizeof(SocketInterface_OperationType);

    serialized->ReadString(&m_Path);

    serialized->ReadString(&m_Args);

    serialized->ReadString(&m_Output);

}



UInt32 SocketInterface_DynamicResponseMessage::GetSize()

{

    UInt32 size = SOCKET_INTERFACE_MIN_DYNAMIC_RESPONSE_SIZE + sizeof(SocketInterface_OperationType);



    size += m_Path.size();

    size += m_Args.size();

    size += m_Output.size();



    return size;

}



SocketInterface_SerializedMessage* SocketInterface_DynamicResponseMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;



    data->AddHeader(GetType(), GetSize());

    data->AddData((UInt8*) &m_Type, sizeof(SocketInterface_OperationType));

    data->AddString(&m_Path);

    data->AddString(&m_Args);

    data->AddString(&m_Output);



    return data;

}



void SocketInterface_DynamicResponseMessage::Print(std::string *str)

{

    char str2[MAX_STRING_LENGTH];

    sprintf (str2, "DynamicResponse: Type = ");

    *str += str2;

    switch (m_Type)

    {

    case SocketInterface_OperationType_Read:

        *str += "Read";

        break;



    case SocketInterface_OperationType_Write:

        *str += "Write";

        break;



    case SocketInterface_OperationType_Execute:

        *str += "Execute";

        break;

    }



    *str += ", Path = " + m_Path;

    *str += ", Args = " + m_Args;

    *str += ", Output = " + m_Output;

}


SocketInterface_QuerySimulationStateMessage::SocketInterface_QuerySimulationStateMessage()

{

}



SocketInterface_QuerySimulationStateMessage::SocketInterface_QuerySimulationStateMessage(SocketInterface_SerializedMessage* serialized)

{

}



UInt32 SocketInterface_QuerySimulationStateMessage::GetSize()

{

    UInt32 size = SOCKET_INTERFACE_MIN_QUERY_SIMULATION_STATE_SIZE;

    return size;

}



SocketInterface_SerializedMessage* SocketInterface_QuerySimulationStateMessage::Serialize()

{

    SocketInterface_SerializedMessage* data = new SocketInterface_SerializedMessage;

    data->AddHeader(GetType(), GetSize());

    return data;

}



void SocketInterface_QuerySimulationStateMessage::Print(std::string* str)

{

    *str += "QuerySimulationState";

}



void AddScenario(std::string* fileName, std::string* scenarioFile)

{

    FILE *fd;

    *scenarioFile = "";

    char buf[MAX_STRING_LENGTH];



    // Check if the fileName exists.

    if (strcmp(fileName->c_str(), "") == 0)

    {

        // No Scenario entered by the user.

        printf ("Warning: No Scenario provided\n");

        return;

    }

    fd = fopen(fileName->c_str(), "r");



    if (fd == NULL)

    {

        sprintf(buf, "Can't open input scenario file '%s'", fileName);

        ERROR_ReportError(buf);

    }



    char readStr[MAX_INPUT_FILE_LINE_LENGTH];

    char *string;



    // We start at the beginning of the file.

    fseek(fd, 0, SEEK_SET);



    // Now go to each line and parse it...

    while (fgets(readStr, MAX_INPUT_FILE_LINE_LENGTH, fd) != NULL)

    {

        unsigned readStrLen;    // line length, minus comments



        readStrLen = strlen(readStr);

        if (readStrLen >= (MAX_INPUT_FILE_LINE_LENGTH - 1))

        {

            sprintf(buf, "Input line is too long", readStr);

            ERROR_ReportError(buf);         

        }

        *scenarioFile += readStr;

    }

    fclose(fd);

}
