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

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define socklen_t      int

// The macro below prevents a re-definition error of in6_addr_struct in
// include/main.h.  ws2tcpip.h (necessary for multicast) for MSVC2005 Express
// Edition has a conflict.

#define _NETINET_IN_H

#else /* _WIN32 */
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#define BSD_COMP
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#define ioctlsocket    ioctl

#ifndef INADDR_NONE
#define INADDR_NONE    -1
#endif /* INADDR_NONE */

#define INVALID_SOCKET -1
#define SOCKET_ERROR   -1
#define SD_SEND        SHUT_WR
#endif /* _WIN32 */

#include "dis_shared.h"

#ifdef _WIN32
void
DisInitWin32Sockets()
{
    WSADATA wsaData;

    // Indicate Windows Sockets 1.1 is the highest supported version.

    DisVerify(
        WSAStartup(MAKEWORD(1, 1), &wsaData) == 0,
        "WSAStartup() returned error code");
}

void
DisCleanupWin32Sockets()
{
    // Clean up Windows Sockets (ignore any error code).

    WSACleanup();
}
#endif /* _WIN32 */

void
DisCreateUdpSocket(SOCKET& sd)
{
    // Create UDP socket.

    sd = socket(AF_INET, SOCK_DGRAM, 0);
}

bool
DisSetSO_REUSEADDR(SOCKET sd)
{
    const int opt = 1;

    if (setsockopt(
            sd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (char*) &opt,
            sizeof(opt)) == 0)
    { return true; }
    else
    { return false; }
}

bool
DisSetSO_BROADCAST(SOCKET sd)
{
    const int opt = 1;

    if (setsockopt(
            sd,
            SOL_SOCKET,
            SO_BROADCAST,
            (char*) &opt,
            sizeof(opt)) == 0)
    { return true; }
    else
    { return false; }
}

void
DisBindSocket(SOCKET sd, unsigned ipAddress, unsigned short port)
{
    // Issue bind() on socket for IPv4 address and port.

    sockaddr_in sinSrc;

    sinSrc.sin_family      = AF_INET;
    sinSrc.sin_addr.s_addr = htonl(ipAddress);
    sinSrc.sin_port        = htons(port);

    DisVerify(
        bind(sd, (sockaddr*) &sinSrc, sizeof(sinSrc)) != INVALID_SOCKET,
        "Can't bind() socket");
}

void
DisJoinMulticastGroup(
    SOCKET sd, unsigned multicastIpAddress, unsigned ifIpAddress)
{
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = htonl(multicastIpAddress);
    mreq.imr_interface.s_addr = htonl(ifIpAddress);
    DisVerify(
        setsockopt(
            sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*) &mreq, sizeof(mreq))
          == 0,
        "Can't join multicast group");
}

void
DisSetSocketToNonBlocking(SOCKET sd)
{
    // Set socket to non-blocking mode.

    unsigned long argp = 1;

    DisVerify(
        ioctlsocket(sd, FIONBIO, &argp) != SOCKET_ERROR,
        "ioctl() or ioctlsocket() error");
}

bool
DisAttemptRecv(
    SOCKET sd,
    char* buf,
    unsigned bufSize,
    unsigned& payloadSize)
{
    // Call recv() to obtain a packet from the socket, filling buf with
    // the payload, and return true.
    // If recv() indicates there is no packet currently available, return
    // false.
    // If bufSize is too small, return false.

    const int flags = 0;
    int recvVal = recv(sd, buf, bufSize, flags);

    if (recvVal == SOCKET_ERROR)
    {
#ifdef _WIN32
        if (WSAGetLastError() == WSAEWOULDBLOCK)
#else /* _WIN32 */
        if (errno == EAGAIN)
#endif /* _WIN32 */
        {
            // There are no pending UDP datagrams.

            return false;
        }

        // Another kind of error.

        DisReportWarning("Unknown error while calling recv() on socket");
        return false;
    }

    payloadSize = recvVal;

    return true;
}

bool
DisSendDatagram(
    SOCKET sd,
    unsigned dstIpAddress,
    unsigned short dstPort,
    char* buf,
    unsigned bufSize)
{
    int sendtoVal;

    struct sockaddr_in sinDst;

    sinDst.sin_family      = AF_INET;
    sinDst.sin_addr.s_addr = htonl(dstIpAddress);
    sinDst.sin_port        = htons(dstPort);

    sendtoVal
        = sendto(sd,
                 buf,
                 bufSize,
                 0,
                 (struct sockaddr*) &sinDst,
                 sizeof(sinDst));

    if (sendtoVal < 0) { return false; }
    else { return true; }
}

void
DisCloseUdpSocket(SOCKET sd)
{
#ifdef _WIN32
    // Close socket with shutdown(), ignoring any further received data.

    DisVerify(
        shutdown(sd, SD_SEND) != SOCKET_ERROR,
        "Can't shutdown()");

    DisVerify(
        closesocket(sd) != SOCKET_ERROR,
        "Can't closesocket()");
#endif /* _WIN32 */
}
