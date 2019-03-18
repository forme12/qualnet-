#include "api.h"
#include "partition.h"
#include "external_util.h"
#include "agi_interface.h"
#include "agi_interface_util.h"
#include <iostream>

//---------------------------------------------------------------------------
// External Interface API Functions
//---------------------------------------------------------------------------
void AgiInterfacePreBootstrap(
    int argc,
    char* argv [],
    SimulationProperties* simProps)
{
    int cntr = 0;
    while (cntr < argc)
    {
        if (strcmp(argv[cntr], "-agi_interface_max_stk_ver") == 0)
        {
            std::cout << "MaxVersion="<< MAXSTKVERSION << std::endl;
            exit(EXIT_SUCCESS);
        }
        cntr++;
    }
}

void AgiInterfaceCommandlineHandler(char* option)
{
    if (strcmp(option, "-agi_interface_max_stk_ver") == 0)
    {
        std::cout << "MaxVersion="<< MAXSTKVERSION << std::endl;
        exit(EXIT_SUCCESS);
    }
}

void AgiInterfaceInitialize(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput)
{
#if DEBUGBREAK == 1
    DebugBreak();
#endif

    BOOL retVal = FALSE;
    char stkVer[100];
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "AGI-STK-VERSION", &retVal, stkVer);

    CAgiInterfaceUtil::StkVersion ver = CAgiInterfaceUtil::eStk9;

    if ((retVal == TRUE) && (strcmp(stkVer, "10") == 0))
    {
        ver = CAgiInterfaceUtil::eStk10;
    }

    CAgiInterfaceUtil::GetInstance().Initialize(ver, retVal);
    iface->partition->isAgiInterfaceEnabled = TRUE;
}

void AgiInterfaceUpdatePosition(Node* node, Message* msg)
{
    clocktype now = getSimTime(node);

    CAgiInterfaceUtil::LLA lla;
    if (CAgiInterfaceUtil::GetInstance().ComputeNodePosition(node->nodeId, now, lla))
    {
        // Used to determine rate and direction of nodes
        Coordinates pos;
        pos.type = GEODETIC;
        pos.latlonalt.latitude = (CoordinateType)lla.latitude_degrees;
        pos.latlonalt.longitude = (CoordinateType)lla.longitude_degrees;
        pos.latlonalt.altitude = (CoordinateType)lla.altitude_meters;

        GUI_MoveNode(node->nodeId,
                     pos,
                     now);
    }

    MESSAGE_Send(node, msg, node->agiData.positionUpdateInterval);
}

void AgiInterfaceInitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput)
{
    BOOL retVal = FALSE;
    Node* nextNode = NULL;

    // Initialize each node.
    nextNode = iface->partition->firstNode;

    if ((nextNode != NULL) && (nextNode->guiOption == TRUE))
    {
        while (nextNode != NULL)
        {
            nextNode->agiData.positionUpdateInterval = 1000000000;

            IO_ReadTime(
                nextNode->nodeId,
                ANY_ADDRESS,
                nodeInput,
                "AGI-POSITION-UPDATE-INTERVAL",
                &retVal,
                &nextNode->agiData.positionUpdateInterval);

            Message* msg = MESSAGE_Alloc(nextNode,
                            EXTERNAL_LAYER,
                            0,
                            MSG_EXTERNAL_AgiUpdatePosition);

            MESSAGE_Send(nextNode,
                         msg,
                         nextNode->agiData.positionUpdateInterval);

            nextNode = nextNode->nextNodeData;
        }
    }
}
