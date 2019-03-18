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

#ifndef HLA_H
#define HLA_H

const unsigned g_hlaUserInputLineBufSize = 1024;

void
ProcessCommandLineArguments(unsigned argc, char* argv[]);

void
ShowUsage();

void
EnterCommandLoop();

void
ShowCommands();

void
RegisterObjects();

void
MoveEntity(char* next);

bool
ParseMoveEntityCommand(
    char* next,
    unsigned& nodeId,
    double& lat,
    double& lon,
    double& alt);

void
ChangeEntityOrientation(char* next);

bool
ParseChangeEntityOrientationCommand(
    char* next,
    unsigned& nodeId,
    double& orientationPsiDegrees,
    double& orientationThetaDegrees,
    double& orientationPhiDegrees);

void
ChangeEntityVelocity(char* next);

bool
ParseChangeEntityVelocityCommand(
    char* next,
    unsigned& nodeId,
    float& xVelocity,
    float& yVelocity,
    float& zVelocity);

void
SendCommEffectsRequest(char* next);

bool
ParseSendCommEffectsRequestCommand(
    char*       next,
    unsigned&   srcNodeId,
    bool&       isData,
    unsigned&   msgSize,
    double&     timeoutDelay,
    bool&       dstNodeIdPresent,
    unsigned&   dstNodeId);

void
ChangeDamageState(char* next);

bool
ParseChangeDamageStateCommand(
    char* next,
    unsigned& nodeId,
    unsigned& damageState);

void
ChangeTxOperationalStatus(char* next);

bool
ParseChangeTxOperationalStatusCommand(
    char* next,
    unsigned& nodeId,
    unsigned char& txOperationalStatus);

#endif /* HLA_H */
