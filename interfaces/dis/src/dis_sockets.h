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

#ifndef DIS_SOCKETS_H
#define DIS_SOCKETS_H

#ifdef _WIN64
#define SOCKET Int64  //To support SOCKET type UINT_PTR in 64 bit WINDOWS
#else
#ifdef _WIN32
#define SOCKET unsigned
#else /* _WIN32 */
#define SOCKET int
#endif /* _WIN32 */
#endif /* _WIN64 */

#ifdef _WIN32
void
DisInitWin32Sockets();

void
DisCleanupWin32Sockets();
#endif /* _WIN32 */

void
DisCreateUdpSocket(SOCKET& sd);

bool
DisSetSO_REUSEADDR(SOCKET sd);

bool
DisSetSO_BROADCAST(SOCKET sd);

void
DisBindSocket(SOCKET sd, unsigned ipAddress, unsigned short port);

void
DisJoinMulticastGroup(
    SOCKET sd, unsigned multicastIpAddress, unsigned ifIpAddress);

void
DisSetSocketToNonBlocking(SOCKET sd);

bool
DisAttemptRecv(
    SOCKET sd,
    char* buf,
    unsigned bufSize,
    unsigned& payloadSize);

bool
DisSendDatagram(
    SOCKET sd,
    unsigned dstIpAddress,
    unsigned short dstPort,
    char* buf,
    unsigned bufSize);

void
DisCloseUdpSocket(SOCKET sd);

#endif /* DIS_SOCKETS_H */
