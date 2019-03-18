#ifndef _AGI_INTERFACE_H_
#define _AGI_INTERFACE_H_

#include "clock.h"
#include "external.h"
#include "coordinates.h"

#define MAXSTKVERSION 10

struct AgiData
{
    // What interval to use to update a node's position in STK.
    clocktype positionUpdateInterval;
};

// forward declaration
struct EXTERNAL_Interface;
struct SimulationProperties;

void AgiInterfacePreBootstrap(
    int argc,
    char* argv [],
    SimulationProperties* simProps);

void AgiInterfaceCommandlineHandler(char* option);

void AgiInterfaceInitialize(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);

void AgiInterfaceInitializeNodes(
    EXTERNAL_Interface *iface,
    NodeInput *nodeInput);

void AgiInterfaceUpdatePosition(
    Node* node,
    Message* msg);

#endif /* _AGI_INTERFACE_H_ */
