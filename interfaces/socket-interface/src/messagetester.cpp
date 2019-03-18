#include <iostream>

#include "api.h"
#include "messageapi.h"

using namespace::std;

void TestInitializeSimulation()
{
    SocketInterface_InitializeSimulationMessage* initialize;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create initialize simulation message
    initialize = new SocketInterface_InitializeSimulationMessage(SocketInterface_TimeManagementType_RealTime);
    initialize->SetCoordinateSystem(SocketInterface_CoordinateSystemType_LatLonAlt);
    initialize->SetScenario("Scenario script");

    // Serialize it
    data = initialize->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    initialize->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_InitializeSimulation strings do not match!\n");
        pass = FALSE;
    }

    if (initialize->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_InitializeSimulation sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_InitializeSimulation data size does not match!\n");
        pass = FALSE;
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_InitializeSimulation passes" << endl << endl;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_InitializeSimulation duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Size--;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_InitializeSimulation did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message type
        data->m_Size++;
        data->m_Data[0] = 99;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_InitializeSimulation did not get bad message exception!\n");
    }

    delete initialize;
    delete data;
}

void TestPauseSimulation()
{
    SocketInterface_PauseSimulationMessage* pause;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create pause simulation message
    pause = new SocketInterface_PauseSimulationMessage();
    pause->SetPauseTime(100.0);

    // Serialize it
    data = pause->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    pause->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_PauseSimulation strings do not match!\n");
        pass = FALSE;
    }

    if (pause->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_PauseSimulation sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_PauseSimulation data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_PauseSimulation duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong size of optional time parameter
        data->m_Data[7] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_PauseSimulation did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[7] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_PauseSimulation did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_PauseSimulation passes" << endl << endl;
    }

    delete pause;
    delete data;
}

void TestExecuteSimulation()
{
    SocketInterface_ExecuteSimulationMessage* execute;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create execute simulation message
    execute = new SocketInterface_ExecuteSimulationMessage();

    // Serialize it
    data = execute->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    execute->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_ExecuteSimulation strings do not match!\n");
        pass = FALSE;
    }

    if (execute->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_ExecuteSimulation sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_ExecuteSimulation data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_ExecuteSimulation duplicated string do not match!\n");
        pass = FALSE;
    }

    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Make message too large
        data->m_Data[3] = 5;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_ExecuteSimulation did not get size exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_ExecuteSimulation passes" << endl << endl;
    }

    delete execute;
    delete data;
}

void TestStopSimulation()
{
    SocketInterface_StopSimulationMessage* stop;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create stop simulation message
    stop = new SocketInterface_StopSimulationMessage();
    stop->SetStopTime(100.0);
    stop->SetWaitForResponses(1);

    // Serialize it
    data = stop->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    stop->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        cout << "SocketInterface_MessageType_StopSimulation strings do not match!" << endl;
        cout << str << " vs. " << str2 << endl;
        pass = FALSE;
    }

    if (stop->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_StopSimulation sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_StopSimulation data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        cout << "SocketInterface_MessageType_StopSimulation duplicated string do not match!" << endl;
        cout << str << " vs. " << str2 << endl;
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_StopSimulation did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_StopSimulation did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_StopSimulation passes" << endl << endl;
    }

    delete stop;
    delete data;
}

void TestResetSimulation()
{
    SocketInterface_ResetSimulationMessage* reset;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create reset simulation message
    reset = new SocketInterface_ResetSimulationMessage();
    reset->SetResetTime(100.0);

    // Serialize it
    data = reset->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    reset->Print(&str);
    message->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_ResetSimulation strings do not match!\n");
        pass = FALSE;
    }

    if (reset->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_ResetSimulation sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_ResetSimulation data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_ResetSimulation duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong size of optional time parameter
        data->m_Data[7] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_ResetSimulation did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set non-zero zero required field in optional header
        data->m_Data[7] = 0;
        data->m_Data[5] = 3;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_ResetSimulation did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_ResetSimulation passes" << endl << endl;
    }

    delete reset;
    delete data;
}

void TestTimeAdvance()
{
    SocketInterface_AdvanceTimeMessage* advance;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create advance time message
    advance = new SocketInterface_AdvanceTimeMessage(999.3);

    // Serialize it
    data = advance->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    advance->Print(&str);
    message->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_AdvanceTime strings do not match!\n");
        pass = FALSE;
    }

    if (advance->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_AdvanceTime sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_AdvanceTime data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_AdvanceTime duplicated string do not match!\n");
        pass = FALSE;
    }
    
    if (pass)
    {
        cout << "SocketInterface_MessageType_AdvanceTime passes" << endl << endl;
    }

    delete message;
    delete advance;
    delete data;
}

void TestSimulationIdle()
{
    SocketInterface_SimulationIdleMessage* idle;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;

    // Create simulation idle message
    idle = new SocketInterface_SimulationIdleMessage(1234.567);

    // Serialize it
    data = idle->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    idle->Print(&str);
    message->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_SimulationIdle strings do not match!\n");
        pass = FALSE;
    }

    if (idle->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_SimulationIdle sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_SimulationIdle data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_SimulationIdle duplicated string do not match!\n");
        pass = FALSE;
    }
    
    if (pass)
    {
        cout << "SocketInterface_MessageType_SimulationIdle passes" << endl << endl;
    }

    delete message;
    delete idle;
    delete data;
}

void TestCreatePlatform()
{
    SocketInterface_CreatePlatformMessage* create;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    SocketInterface_Coordinates pos;

    std::string id = "test";
    pos.SetXYZ(100.0, 200.0, 300.0);

    // Create stop simulation message
    create = new SocketInterface_CreatePlatformMessage(&id, &pos, SocketInterface_PlatformStateType_Undamaged);
    create->SetCreateTime(50.0);
    create->SetType(SocketInterface_PlatformType_Air);

    std::string group = "225.0.0.1";
    create->AddMulticastGroup(&group);
    group = "225.0.0.2";
    create->AddMulticastGroup(&group);

    // Serialize it
    data = create->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    create->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CreatePlatform strings do not match!\n");
        pass = FALSE;
    }

    if (create->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_CreatePlatform sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_CreatePlatform data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CreatePlatform duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CreatePlatform did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CreatePlatform did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_CreatePlatform passes" << endl << endl;
    }

    delete create;
    delete data;
}

void TestUpdatePlatform()
{
    SocketInterface_UpdatePlatformMessage* update;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    SocketInterface_Coordinates pos;

    std::string id = "EntityA300";
    pos.SetXYZ(100.0, 200.0, 300.0);

    // Create stop simulation message
    update = new SocketInterface_UpdatePlatformMessage(&id);
    update->SetUpdateTime(62.33);
    update->SetState(SocketInterface_PlatformStateType_Undamaged);
    update->SetSpeed(23.2);

    std::string group = "225.0.0.1";
    update->AddLeaveMulticastGroup(&group);
    group = "225.0.0.7";
    update->AddJoinMulticastGroup(&group);

    // Serialize it
    data = update->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    update->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_UpdatePlatform strings do not match!\n");
        pass = FALSE;
    }

    if (update->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_UpdatePlatform sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_UpdatePlatform data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_UpdatePlatform duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_UpdatePlatform did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_UpdatePlatform did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_UpdatePlatform passes" << endl << endl;
    }

    delete update;
    delete data;
}

void TestError()
{
    SocketInterface_ErrorMessage* error;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    SocketInterface_Coordinates pos;

    std::string errorString = "Test error string";

    // Create stop simulation message
    error = new SocketInterface_ErrorMessage(SocketInterface_ErrorType_InvalidMessage, &errorString);

    SocketInterface_PauseSimulationMessage* pause = new SocketInterface_PauseSimulationMessage();
    pause->SetPauseTime(399.9);
    error->SetMessage(pause);
    delete pause;

    // Serialize it
    data = error->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    error->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_Error strings do not match!\n");
        pass = FALSE;
    }

    if (error->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_Error sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_Error data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_Error duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_Error did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_Error did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_Error passes" << endl << endl;
    }

    delete error;
    delete data;
}


void TestCommEffectsRequest()
{
    SocketInterface_CommEffectsRequestMessage* request;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    //SocketInterface_Coordinates pos;

    UInt64 Id1 = 100000;
    UInt64 Id2 = 2000;
    std::string sender = "1navinCES";
    std::string receiver = "2jesseCES";
    SocketInterface_ProtocolType protocol = SocketInterface_ProtocolType_ProtocolUDP;
    UInt32 size = 12000;
    std::string description = "Navin is sending a request";

    // Create Request message
    request = new SocketInterface_CommEffectsRequestMessage(Id1, Id2, protocol, size, &sender, &receiver);
    request->SetDescription(description);
    // Set optional messages
    request->SetSendTime(500);
    request->SetFailureTimeout(90);
    request->SetIdleWhenResponseSent(0);
    request->SetPhyTransmissionType(0);

    // Serialize it
    data = request->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    request->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CommEffectsRequest strings do not match!\n");
        pass = FALSE;
    }

    if (request->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_CommEffectsRequest sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_CommEffectsRequest data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CommEffectsRequest duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CommEffectsRequest did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CommEffectsRequest did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_CommEffectsRequest passes" << endl << endl;
    }

    delete request;
    delete data;
}


void TestCommEffectsResponse()
{
    SocketInterface_CommEffectsResponseMessage* response;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    //SocketInterface_Coordinates pos;

    UInt64 Id1 = 1;
    UInt64 Id2 = 2;
    UInt8 status = 0;
    SocketInterface_TimeType receiveTime = 125.0;
    SocketInterface_TimeType latency = 12.0;

    std::string description = "Navin received a response";
    
    std::string sender = "1";
    std::string receiver = "2";
    // Create Request message
    response = new SocketInterface_CommEffectsResponseMessage(Id1,
        Id2,        
        &sender,
        &receiver,
        status,
        receiveTime,
        latency);
    response->SetDescription(description);
  

    // Serialize it
    data = response->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    response->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CommEffectsResponse strings do not match!\n");
        pass = FALSE;
    }

    if (response->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_CommEffectsResponse sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_CommEffectsResponses data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_CommEffectsResponse duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CommEffectsRequest did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_CommEffectsResponse did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_CommEffectsResponse passes" << endl << endl;
    }

    delete response;
    delete data;
}


void TestDynamicCommand()
{
    SocketInterface_DynamicCommandMessage* dynamic;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    //SocketInterface_Coordinates pos;

  
    UInt8 type = 1;
    std::string path = "CBR 1 2 100 500 1000";
    std::string arg = "INTERVAL 10";

    // Create Request message
    dynamic = new SocketInterface_DynamicCommandMessage(type, &path, &arg);
    
    // Serialize it
    data = dynamic->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    dynamic->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_DynamicCommand strings do not match!\n");
        pass = FALSE;
    }

    if (dynamic->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_DynamicCommand sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_DynamicCommand data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_DynamicCommand duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_DynamicCommand did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_DynamicCommand did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_DynamicCommand passes" << endl << endl;
    }

    delete dynamic;
    delete data;
}

void TestDynamicResponse()
{
    SocketInterface_DynamicResponseMessage* dynamic;
    SocketInterface_SerializedMessage* data;
    SocketInterface_Message* message;
    SocketInterface_Message* message2;
    std::string str;
    std::string str2;
    BOOL pass = true;
    BOOL gotException;
    //SocketInterface_Coordinates pos;

  
    UInt8 type = 1;
    std::string path = "CBR 1 2 100 500 1000";
    std::string arg = "INTERVAL 10";
    std::string output = "THE traffic was sent every 10 Seconds";
    // Create Request message
    dynamic = new SocketInterface_DynamicResponseMessage(type, &path, &arg, &output);
    
    // Serialize it
    data = dynamic->Serialize();

    // Deserialize it
    message = data->Deserialize();

    // Convert both messages to strings
    dynamic->Print(&str);
    message->Print(&str2);

    cout << str2 << endl;

    if (str != str2)
    {
        printf("SocketInterface_MessageType_DynamicResponse strings do not match!\n");
        pass = FALSE;
    }

    if (dynamic->GetSize() != message->GetSize())
    {
        printf("SocketInterface_MessageType_DynamicResponse sizes do not match!\n");
        pass = FALSE;
    }

    if (data->m_CurrentSize != message->GetSize())
    {
        printf("SocketInterface_MessageType_DynamicResponse data size does not match!\n");
        pass = FALSE;
    }

    message2 = message->Duplicate();
    str2.clear();
    message2->Print(&str2);

    if (str != str2)
    {
        printf("SocketInterface_MessageType_DynamicResponse duplicated string do not match!\n");
        pass = FALSE;
    }
    
    delete message;

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong message size
        data->m_Data[2] = 1;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_DynamicResponse did not get size exception!\n");
    }

    // Catch exception
    gotException = FALSE;
    try
    {
        // Set wrong optional parameter type
        data->m_Data[2] = 0;
        data->m_Data[SOCKET_INTERFACE_HEADER_SIZE] = SocketInterface_OptionType_JoinMulticastGroups;
        message = data->Deserialize();
    }
    catch (SocketInterface_Exception &e)
    {
        gotException = TRUE;
    }
    if (!gotException)
    {
        pass = FALSE;
        printf("SocketInterface_MessageType_DynamicResponse did not get bad message exception!\n");
    }

    if (pass)
    {
        cout << "SocketInterface_MessageType_DynamicResponse passes" << endl << endl;
    }

    delete dynamic;
    delete data;
}

int main()
{
    try
    {
        TestInitializeSimulation();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_InitializeSimulation got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestPauseSimulation();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_PauseSimulation got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }
    
    try
    {
        TestExecuteSimulation();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_ExecuteSimulation got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestStopSimulation();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_StopSimulation got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestResetSimulation();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_ResetSimulation got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestTimeAdvance();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_AdvanceTime got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestSimulationIdle();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_SimulationIdle got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestCreatePlatform();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_CreatePlatform got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestUpdatePlatform();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_UpdatePlatform got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestError();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_Error got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestCommEffectsRequest();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_CommEffectsRequest got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestCommEffectsResponse();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_CommEffectsResponse got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestDynamicCommand();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_DynamicCommand got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    try
    {
        TestDynamicResponse();
    }
    catch (SocketInterface_Exception &e)
    {
        cout << "SocketInterface_MessageType_DynamicResponse got unexpected error " << e.GetError() << ": " << e.GetErrorString() << endl;
    }

    return 0;

}
