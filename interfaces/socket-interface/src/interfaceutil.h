#ifndef _INTERFACE_UTIL_H_
#define _INTERFACE_UTIL_H_

#include "external_socket.h"
#include "entity.h"

struct SocketInterface_Sockets
{
    // The number of listening ports as specified by the config file,
    // plus any ports used in bootstrap mode
    int numPorts;

    // Array of listening ports (size numPorts)
    std::vector<int> listenPorts;

    // The listening sockets (size numPorts)
    std::vector<EXTERNAL_Socket*> listeningSockets;

    // Number of connections with MTS used (in connections array)
    int numConnections;

    // Array pointing to which connections are active
    std::vector<BOOL> activeConnections;

    // The socket connections with MTS, etc
    std::vector<EXTERNAL_Socket*> connections;

    // Mutex used for maintaining socket connections
    pthread_mutex_t socketMutex;

    pthread_t receiverThread;
    pthread_mutex_t receiverMutex;
    pthread_cond_t receiverNotFull;
    pthread_cond_t receiverNotEmpty;

    pthread_t senderThread;
    pthread_mutex_t senderMutex;
    pthread_cond_t senderNotFull;
    pthread_cond_t senderNotEmpty;

    SocketInterface_Sockets();
};

BOOL IsMyMac(
    Node *node,
    Message *msg);

void ConvertGCCCartesianToGCC(
    double x,
    double y,
    double z,
    double *lat,
    double *lon,
    double *alt);

void ConvertGCCCartesianVelocityToGCC(
    double xpos,
    double ypos,
    double zpos,
    double xvel,
    double yvel,
    double zvel,
    double *latvel,
    double *lonvel,
    double *altvel);

void CreateNewConnection(
    EXTERNAL_Interface* iface,
    SocketInterface_Sockets* sockets,
    int portIndex,
    char* outputString);

void HandleSocketError(
    EXTERNAL_Interface *iface,
    SocketInterface_Sockets *sockets,
    int socketId,
    EXTERNAL_SocketErrorType err,
    char* outputString);

#endif /* _INTERFACE_UTIL_H_ */
