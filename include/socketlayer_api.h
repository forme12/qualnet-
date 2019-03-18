// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.
#ifndef SOCKET_LAYER_API_H
#define SOCKET_LAYER_API_H

#include <queue>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <poll.h>
#endif

#define SL_START_PORT_NUM 7000
#define SL_MAX_ACCEPT_BACKLOG 5
#define SL_IF_NAME_SIZE 16

typedef enum {
    SL_TIMER_ONCE,
    SL_TIMER_REPEAT
} SLTimerType;

typedef enum {
    SL_SOCKET_UNCONNECTED,  // =0
    SL_SOCKET_LISTENING,    // =1
    SL_SOCKET_CONNECTED,    // =2
    SL_SOCKET_CLOSING,      // =3
    SL_SOCKET_CLOSED        // =4
} SLSocketConnectionState;

typedef struct {
    struct sockaddr remoteAddr;
    int connectionId;
    int uniqueId;
    Node *node;
    int remoteNodeFd;
} SLSocketAcceptBacklog;

typedef struct {
    char ifname[16]; // TODO
    struct sockaddr_in ifaddr;
    struct sockaddr_in netmask;
    struct sockaddr_in bcastaddr;
    struct sockaddr phyaddr;
    short flags;
    int ifindex;
    int metric;
    int mtu;
}SLInterfaceInfo;

typedef struct sl_socket_data_str {
    int family;
    int protocol;
    int type;

    int refCount;

    int connectionId;
    int uniqueId;
    SLSocketConnectionState connectionState;

    struct sockaddr_in localAddress;
    struct sockaddr_in remoteAddress;

    BOOL isNonBlocking;

    TosType tos;

    SLSocketAcceptBacklog acceptBacklog[SL_MAX_ACCEPT_BACKLOG];
    int acceptBacklogIndex;

    struct sockaddr upaPhysicalAddr;
    BOOL isUpaSocket;
    std::queue<Message *> *upaRecvBuffer;
    BOOL lastBufferRead;

    void (*recvfromFunction)(Node *, int, char *, int, struct sockaddr *);
    void (*recvFunction)(Node *, int, char *, int);
    void (*listenFunction)(Node *, int);
    void (*acceptFunction)(Node *, int);
    void (*connectFunction)(Node *, int, sockaddr *);
    void (*sendFunction)(Node *, int, int);

    void *localData;
} SLSocketData;


typedef struct {
    void *data;
    void (*timerFunction)(Node *, void *);
    SLTimerType timerType;
    clocktype interval;
} SLTimerData;


/*******************************************
 * Socket related API
 ******************************************/
int SL_socket(
    Node *node, 
    int family, 
    int protocol, 
    int type);

int SL_ioctl(
    Node *node,
    int fd, 
    unsigned long cmd,
    unsigned long arg);

int SL_sendto(
    Node *node,
    int fd,
    void *data,
    int dataSize,
    struct sockaddr *addr,
    int virtualLength = 0);// the size of virtual data

int SL_send(
    Node *node,
    int fd, 
    void *data, 
    int dataSize,
    int virtualLength = 0);

int SL_read(
    Node *node,
    int fd, 
    void *buf, 
    int n);

int SL_recvfrom(
    Node *node,
    int fd, 
    void *buf, 
    int n,
    int flags, 
    struct sockaddr *addr,
    int *addr_len);

int SL_recv(
    Node *node,
    int fd, 
    void **buf, 
    int n,
    int flags);

int SL_write(
    Node *node,
    int fd, 
    void *buf, 
    int n,
    int virtualLength = 0);


int SL_bind(
    Node *node,
    int fd,
    struct sockaddr *addr,
    int addr_len);


int SL_connect(
    Node *node,
    int fd,
    struct sockaddr *addr,
    int addr_len);


int SL_accept(
    Node *node,
    int fd,
    struct sockaddr *addr,
    int *addr_len);

int SL_close(
    Node *node,
    int fd);

#ifndef _WIN32
int SL_poll(
    Node *node,
    struct pollfd *fds,
    nfds_t nfds,
    int timeout);
#endif


int SL_getsockname(
    Node *node,
    int fd,
    struct sockaddr *addr,
    int *addr_len);


int SL_getpeername(
    Node *node,
    int fd,
    struct sockaddr *addr,
    int *addr_len);


int SL_listen(
    Node *node,
    int fd,
    int backlog);


int SL_getsockopt(
    Node *node,
    int fd,
    int level,
    int optname,
    void *optval,
    int *optlen);


int SL_setsockopt(
    Node *node,
    int fd,
    int level,
    int optname,
    void *optval,
    int optlen);


int SL_fcntl(
    Node *node,
    int fd, 
    int cmd,
    int arg);


int SL_recvmsg(
    Node *node,
    int fd,
    struct msghdr *msg,
    int flags);


int SL_select(
    Node *node,
    int nfds,
    fd_set *readfds,
    fd_set *writefds,
    fd_set *exceptfds,
    struct timeval *timeout);

void SL_fork(
    Node *node,
    int fd);

void SocketLayerSetTimer(
    Node *node,
    SLTimerType timerType,
    struct timeval *interval,
    void (*timerFunction)(Node *, void *),
    void *data);

void SocketLayerRegisterCallbacks(
    Node *node,
    int fd,
    void (*recvfromFunction)(Node *, int, char *, int, struct sockaddr *),
    void (*recvFunction)(Node *, int, char *, int),
    void (*listenFunction)(Node *, int),
    void (*acceptFunction)(Node *, int),
    void (*connectFunction)(Node *, int, struct sockaddr *),
    void (*sendFunction)(Node *, int, int));

void SocketLayerStoreLocalData(
    Node *node,
    int fd,
    void *data);

void *SocketLayerGetLocalData(
    Node *node,
    int fd);

#endif
