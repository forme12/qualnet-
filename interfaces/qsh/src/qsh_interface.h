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

#ifndef _QSH_H_
#define _QSH_H_

#include "external.h"
#include "external_socket.h"
#include "dynamic.h"

struct QshData;

class QshListenerCallback : public D_ListenerCallback
{
    private:
        std::string m_Path;
        EXTERNAL_Interface* m_Iface;

    public:
        QshListenerCallback(const std::string& path, EXTERNAL_Interface* iface) : m_Path(path), m_Iface(iface)
        {
            // empty
        }

        virtual void operator () (const std::string& newValue)
        {
            std::string output;

            output = m_Path + ":\n" + newValue + "\n";
            //Typecasted forwardSize to int to support 64 bit compilation.
            EXTERNAL_ForwardData(
                m_Iface,
                NULL,
                (void*) output.c_str(),
                (output.size() + 1));
    }
};

class D_QshChangeModeCommand : public D_Command
{
    private:
        EXTERNAL_Interface* iface;

    public:
        D_QshChangeModeCommand(EXTERNAL_Interface* newIface)
        {
            iface = newIface;
        }

        virtual void ExecuteAsString(const std::string& in, std::string& out);
};

struct QshData
{
    QshData(EXTERNAL_Interface* newIface);

    std::string cwd;

    D_Clocktype horizon;
    D_String mode;
    D_QshChangeModeCommand changeMode;

    EXTERNAL_Socket listenSocket;
    EXTERNAL_Socket s;
};

// prototypes
void QshSendPrompt(EXTERNAL_Interface* iface);

void QshInitialize(
    EXTERNAL_Interface* iface,
    NodeInput* nodeInput);

void QshReceive(EXTERNAL_Interface* iface);

void QshForward(
    EXTERNAL_Interface* iface,
    Node* node,
    void* forwardData,
    int forwardSize);

void QshFinalize(EXTERNAL_Interface* iface);

#endif
