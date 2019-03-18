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

// /**
// PACKAGE     :: APP_UTIL
// DESCRIPTION :: This file describes Application Layer utility functions.
// **/

#ifndef _APP_UTIL_H_
#define _APP_UTIL_H_


#include <set>

#include "types.h"
#include "app_superapplication.h"
#include "api.h"
#include "app_mdp.h"

#ifdef WIN32
#ifdef ADDON_DB
// Causes compile errors on windows
// Can be removed when tcp/udp send functions are cleaned up
//#include "dbapi.h"
#endif
#endif

#ifdef ADDON_DB
class StatsDBAppEventParam;
//#include "dbapi.h"
#endif

enum PortType {
    START_NONE   = 0,
    START_EVEN   = 1,
    START_ODD    = 2
};

// /**
// Define      :: IPDEFTTL
// DESCRIPTION :: TTL default value.
//                Used in TCP/UDP app layer TTL
// **/
#define IPDEFTTL 64

enum AppMsgStatus {
    APP_MSG_NEW, // valid and new
    APP_MSG_OLD, // duplicate or out of order
    APP_MSG_UNKNOWN, // I don't know. Use seq cache to detect
};

// /**
// CLASS       :: SequenceNumber
// DESCRIPTION :: Implementation of sequence number cache. It maintains a
//                list of sequence numbers in order to detect whether a
//                newly arrived message is duplicated or out-of-order.
// **/
class SequenceNumber
{
    private:
        // private initialization function primarily for supporting multiple
        // constructors. We cannot call another constructor from one
        // constructor.
        void InitializeValues(int sizeLimit, Int64 lowestSeqNotRcvd);

    protected:
        std::set<Int64> m_SeqCache; // To store sequence numbers
        int m_SizeLimit;   // how many elements can be stored in the cache,
                           // 0 means unlimit. Must be 0 or larger than 1
        Int64 m_LowestSeqNotRcvd; // The lowest sequence number have not
                                  // received so far
        Int64 m_HighestSeqRcvd;   // Highest sequence number received so far
                                  // if cache is not empty. When cache is
                                  // empty, it is same as m_LowestSeqNotRcvd.

    public:
        // Status of a sequence number
        enum Status
        {
            SEQ_NEW,          // A new sequence number not seen before
            SEQ_DUPLICATE,    // This sequence number has been seen before
            SEQ_OUT_OF_ORDER  // This sequence number not seen before, but
                              // larger ones have been received. Thus this is
                              // an out of order arrival
        };

        // constructors
        SequenceNumber(); // A constructor without parameter. m_SizeLimit
                          // will be 0 (unlimit). Initial m_LowestSeqNotRcvd
                          // will be 0.

        // A constructor with size limit specified. Initial value of
        // m_LowestSeqNotRcvd will be 0
        SequenceNumber(int sizeLimit);

        // A constructor with size limit and lowest sequence number passed in
        SequenceNumber(int sizeLimit, Int64 lowestSeqNotRcvd);


        // The destructor. Does nothing.
        virtual ~SequenceNumber() {};

        // major interface function. For the specified seqNumber, return
        // whether it is new or duplicate or out-of-order. In addition, it
        // will be inserted into the cache.
        Status Insert(Int64 seqNumber);

        Status Insert(Int64 seqNumber, AppMsgStatus msgStatus);

        // For debug purpose, print out elements in the cache as well as
        // values of member variables.
        void Print(FILE* out);
};

// /**
// API        :: APP_RegisterNewApp
// PURPOSE    :: Insert a new application into the list of apps
//               on this node.
// PARAMETERS ::
// + node      : Node*    : node that is registering the application.
// + appType   : AppType  : application type
// + dataPtr   : void *   : pointer to the data space for this app
// RETURN     :: AppInfo* : pointer to the new AppInfo data structure
//                          for this app
// **/
AppInfo *
APP_RegisterNewApp(Node *node, AppType appType, void *dataPtr);

// /**
// API        :: APP_SetTimer
// PURPOSE    :: Set a new App Layer Timer and send to self after delay.
// PARAMETERS ::
// + node      : Node*     : node that is issuing the Timer.
// + appType   : AppType   : application type
// + connId    : int       : if applicable, the TCP connectionId for this timer
// + sourcePort: short     : the source port of the application setting
//                           this timer
// + timerType : int       : an integer value that can be used to
//                           distinguish between types of timers
// + delay     : clocktype : send the timer to self after this delay.
// RETURN     :: void :
// **/
void
APP_SetTimer(Node *node, AppType appType, int connId,
             unsigned short sourcePort, int timerType, clocktype delay);

// /**
// MACRO       :: APP_GetTimerType(x)
// DESCRIPTION :: Get the timerType for a received App Layer Timer.
// **/
#define APP_GetTimerType(x)   (((AppTimer *) (MESSAGE_ReturnInfo(x)))->type)


// /**
// API            :: APP_UdpSendNewData
// PURPOSE        :: Allocate data and send to UDP.
// PARAMETERS     ::
// + node          : Node *             : node that is sending the data.
// + appType       : AppType            : application type, to be used as destination port.
// + sourceAddr    : NodeAddress        : the source sending the data.
// + sourcePort    : short              : the application source port.
// + destAddr      : NodeAddress        : the destination node Id data is sent to.
// + payload       : char *             : pointer to the data.
// + payloadSize   : int                : size of the data in bytes.
// + delay         : clocktype          : send the data after this delay.
// + traceProtocol : TraceProtocolType  : specify the type of application used for
//                                        packet tracing.
// RETURN         :: Message * :
// **/
Message *
APP_UdpSendNewData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol);

#ifdef EXATA
// /**
// // API            :: APP_UdpSendNewData
// // PURPOSE        :: Allocate data and send to UDP.
// // PARAMETERS     ::
// // + node          : Node *             : node that is sending the data.
// // + appType       : AppType            : application type, to be used as destination port.
// // + sourceAddr    : NodeAddress        : the source sending the data.
// // + sourcePort    : short              : the application source port.
// // + destAddr      : NodeAddress        : the destination node Id data is sent to.
// // + payload       : char *             : pointer to the data.
// // + payloadSize   : int                : size of the data in bytes.
// // + delay         : clocktype          : send the data after this delay.
// // + traceProtocol : TraceProtocolType  : specify the type of application used for
// //                                        packet tracing.
// // + isEmulationPacket : BOOL           : if it's emulation packet
// // RETURN         :: void :
// // **/
//
void
APP_UdpSendNewData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isEmulationPacket);
#endif


// /**
// API            :: APP_UdpSendNewDataWithPriority
// PURPOSE        :: Allocate data with specified priority and send to UDP.
// PARAMETERS     ::
// + node          : Node *            : node that is sending the data.
// + appType       : AppType           : application type, to be used as
//                                       destination port.
// + sourceAddr    : NodeAddress       : the source sending the data.
// + sourcePort    : short             : the application source port.
// + destAddr      : NodeAddress       : the destination node Id data
//                                       is sent to.
// + outgoingInterface  : int          : interface used to send data.
// + payload       : char*             : pointer to the data.
// + payloadSize   : int               : size of the data in bytes.
// + priority      : TosType           : priority of data.
// + delay         : clocktype         : send the data after this delay.
// + traceProtocol : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + isMdpEnabled  : BOOL              : specify whether MDP is enabled.
// + uniqueId      : Int32             : specify uniqueId related to MDP.
// + mdpDataObjectInfo  : char*        : specify the mdp data object info if any.
// + mdpDataInfosize : UInt16          : specify the mdp data info size.
// RETURN         :: Message * :
// **/
Message *
APP_UdpSendNewDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    int outgoingInterface,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 uniqueId = -1,
    int regionId = -2,
    const char* mdpDataObjectInfo = NULL,
    unsigned short mdpDataInfosize = 0
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API            :: APP_UdpSendNewDataWithPriority
// PURPOSE        :: Allocate data with specified priority and send to UDP
//                   (For IPv6).
// PARAMETERS     ::
// + node          : Node *            : node that is sending the data.
// + appType       : AppType           : application type, to be used as
//                                       destination port.
// + sourceAddr    : NodeAddress       : the source sending the data.
// + sourcePort    : short             : the application source port.
// + destAddr      : NodeAddress       : the destination node Id data
//                                       is sent to.
// + outgoingInterface  : int          : interface used to send data.
// + payload       : char*             : pointer to the data.
// + payloadSize   : int               : size of the data in bytes.
// + priority      : TosType           : priority of data.
// + delay         : clocktype         : send the data after this delay.
// + traceProtocol : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + isMdpEnabled  : BOOL              : specify whether MDP is enabled.
// + uniqueId      : Int32             : specify uniqueId related to MDP.
// RETURN          :: Message*         : The sent message
// **/
Message*
APP_UdpSendNewDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    short sourcePort,
    Address destAddr,
    int outgoingInterface,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 uniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );


// /**
// API            :: APP_UdpSendNewHeaderData
// PURPOSE        :: Allocate header and data and send to UDP..
// PARAMETERS     ::
// + node          : Node *            : node that is sending the data.
// + appType       : AppType           : application type, to be used as
//                                       destination port.
// + sourceAddr    : NodeAddress       : the source sending the data.
// + sourcePort    : short             : the application source port.
// + destAddr      : NodeAddress       : the destination node Id data
//                                       is sent to.
// + header        : char*             : header of the payload.
// + headerSize    : int               : size of the header.
// + payload       : char*             : pointer to the data.
// + payloadSize   : int               : size of the data in bytes.
// + delay         : clocktype         : send the data after this delay.
// + traceProtocol : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: void :
// **/
void
APP_UdpSendNewHeaderData(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    char *payload,
    int payloadSize,
    clocktype delay,
    TraceProtocolType traceProtocol);

// /**
// API            :: APP_UdpSendNewHeaderDataWithPriority
// PURPOSE        :: Allocate header and data with specified priority
//                   and send to UDP
// PARAMETERS     ::
// + node          : Node *            : node that is sending the data.
// + appType       : AppType           : application type, to be used as
//                                       destination port.
// + sourceAddr    : NodeAddress       : the source sending the data.
// + sourcePort    : short             : the application source port.
// + destAddr      : NodeAddress       : the destination node Id data
//                                       is sent to.
// + outgoingInterface  : int          : interface used to send data.
// + header        : char*             : header of the payload.
// + headerSize    : int               : size of the header.
// + payload       : char*             : pointer to the data.
// + payloadSize   : int               : size of the data in bytes.
// + priority      : TosType           : priority of data.
// + delay         : clocktype         : send the data after this delay.
// + traceProtocol : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: void :
// **/
void
APP_UdpSendNewHeaderDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    int outgoingInterface,
    char *header,
    int headerSize,
    char *payload,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API            :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE        :: Allocate header + virtual data with specified priority
//                   and send to UDP
// PARAMETERS     ::
// + node          : Node *            : node that is sending the data.
// + appType       : AppType           : application type, to be used as
//                                       destination port.
// + sourceAddr    : NodeAddress       : the source sending the data.
// + sourcePort    : short             : the application source port.
// + destAddr      : NodeAddress       : the destination node Id data
//                                       is sent to.
// + header        : char*             : header of the payload.
// + headerSize    : int               : size of the header.
// + payloadSize   : int               : size of the data in bytes.
// + priority      : TosType           : priority of data.
// + delay         : clocktype         : send the data after this delay.
// + traceProtocol : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + isMdpEnabled   : BOOL              :status of MDP layer.
// + mdpUniqueId    : Int32             :unique id for MPD session.
// RETURN         :: Message * :
// **/
Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32  mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );


//added by wsy YG
Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    NodeAddress destId,
	int appCBRtype,
	int appSequence,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32  mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );



// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: Allocate header + virtual data with specified priority
//                    and send to UDP. Data is sent to a non-default destination
//                    port (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : NodeAddress       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : NodeAddress       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + infoData       : char*             : UDP header to be added in info.
// + infoSize       : int               : size of the UDP header.
// + infoType       : int               : info type of the UDP header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + isMdpEnabled   : BOOL              :status of MDP layer.
// + mdpUniqueId    : Int32             :unique id for MPD session.
// RETURN         :: void :
// **/
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + infoData       : char*             : UDP header to be added.
// + infoSize       : int               : size of the UDP header.
// + infoType       : int               : info type of the UDP header
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + appname        : char*             :Application name.
// + isMdpEnabled   : BOOL              :status of MDP layer.
// + mdpUniqueId    : Int32             :unique id for MPD session.
// RETURN         :: Message* : The message that was sent
// **/

void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + infoData       : char*             : UDP header to be added.
// + infoSize       : int               : size of the UDP header.
// + infoType       : int               : info type of the UDP header
// + isMdpEnabled   : BOOL              :status of MDP layer.
// + mdpUniqueId    : Int32             :unique id for MPD session.
// RETURN         :: void :
// **/

void
APP_UdpSendNewHeaderVirtualDataWithPriority(
                        Node *node,
                        NodeAddress sourceAddr,
                        unsigned short sourcePort,
                        NodeAddress destAddr,
                        unsigned short destinationPort,
                        char *header,
                        int headerSize,
                        int payloadSize,
                        TosType priority,
                        clocktype delay,
                        TraceProtocolType traceProtocol,
                        char *appName,
                        char *infoData,
                        int infoSize,
                        unsigned short infoType,
                        BOOL isMdpEnabled = FALSE,
                        Int32 mdpUniqueId = -1
#ifdef ADDON_DB
                        ,
                        StatsDBAppEventParam* appParam = NULL
#endif
                        );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: Allocate header + virtual data with specified priority
//                    and send to UDP. Data is sent to a non-default destination
//                    port (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : NodeAddress       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : NodeAddress       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: void :
// **/
void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );


void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName
#ifdef ADDON_DB
    , StatsDBAppEventParam* appParam
#endif
    );


// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for MDP) Allocate actual + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + appType        : AppType           : specify the application type.
// + sourceAddr     : Address           : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address           : the destination node on which
//                                        data is sent to.
//.+ destPort       : short             : the destination port.
// + header         : char*             : pointer to the payload.
// + headerSize     : int               : size of the payload.
// + payloadSize    : int               : size of the virtual data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + mdpInfo        : char*             : persistent info for Mdp.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                        used for packet tracing.
// + theMsgPtr      : Message*          : pointer the original message to copy
//                                        the other persistent info.
//                                        Default value is NULL.
// RETURN         :: void :
// **/

void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    unsigned short destPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    char* mdpInfo,
    TraceProtocolType traceProtocol,
    Message* theMsgPtr = NULL
#ifdef ADDON_DB
    ,
    BOOL fragIdSpecified = FALSE,
    Int32 fragId = -1
#endif
    );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: void :
// **/

void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: Message * :
// **/


Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
                        Node *node,
                        NodeAddress sourceAddr,
                        unsigned short sourcePort,
                        NodeAddress destAddr,
                        unsigned short destinationPort,
                        char *header,
                        int headerSize,
                        int payloadSize,
                        TosType priority,
                        clocktype delay,
                        TraceProtocolType traceProtocol,
                        char *appName,
                        SuperApplicationUDPDataPacket *data,
                        BOOL isMdpEnabled = FALSE,
                        Int32 mdpUniqueId = -1
#ifdef ADDON_DB
                        ,
                        StatsDBAppEventParam* appParam = NULL
#endif
                        );

#ifdef ADDON_BOEINGFCS
Message*
//APP_UdpSendNewHeaderVirtualDataWithPriorityBoeingFcs(
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName,
    SuperApplicationUDPDataPacket *data,
    int sessionId,
    int requiredTput,
    clocktype requiredDelay,
    float dataRate,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

struct NetworkCesQosReplyPacket;
Message *
//APP_UdpSendNewHeaderVirtualDataWithPriorityBoeingFcs(
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    unsigned short sourcePort,
    NodeAddress destAddr,
    unsigned short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    char *appName,
    SuperApplicationUDPDataPacket *data,
    NetworkCesQosReplyPacket* replyPkt,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );
#endif

// /**
// API         :: APP_UdpCreateNewHeaderVirtualDataWithPriority.
// PURPOSE     :: Create the message.
// PARAMETERS  ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: Message * :message created in the function
Message * APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    short destinationPort,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);




// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// + isMdpEnabled   : BOOL              :status of MDP layer.
// + mdpUniqueId    : Int32             :unique id for MPD session.
// RETURN         :: Message * :
// **/

Message *
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype delay,
    TraceProtocolType traceProtocol,
    BOOL isMdpEnabled = FALSE,
    Int32 mdpUniqueId = -1
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL
#endif
    );

// /**
// API             :: APP_UdpSendNewHeaderVirtualDataWithPriority
// PURPOSE         :: (Overloaded for IPv6) Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Data is sent to a non-default destination port
//                    (port number may not have same value as the AppType).
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + sourceAddr     : Address       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address       : the destination node Id data
//                                       is sent to.
// + destinationPort: short             : the destination port
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + endTime        : clocktype         : zigbeeApp end time.
// + itemSize       : UInt32            : zigbeeApp item size.
// + interval       : D_Clocktype       : zigbeeApp interval
// + delay          : clocktype         : send the data after this delay.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN         :: void :
// **/

void
APP_UdpSendNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    Address sourceAddr,
    unsigned short sourcePort,
    Address destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    clocktype endTime,
    UInt32 itemSize,
    D_Clocktype interval,
    clocktype delay,
    TraceProtocolType traceProtocol);

// /**
// API         :: APP_TcpServerListen.
// PURPOSE     :: Listen on a server port.
// PARAMETERS  ::
// + node       : Node *      : Node pointer that the protocol is
//                              being instantiated in
// + appType    : AppType     : which application initiates this request
// + serverAddr : NodeAddress : server address
// + serverPort : short       : server port number
// RETURN      :: void :
// **/
void
APP_TcpServerListen(
    Node *node,
    AppType appType,
    NodeAddress serverAddr,
    short serverPort);

// /**
// API         :: APP_TcpServerListen.
// PURPOSE     :: (Overloaded for IPv6) Listen on a server port.
// PARAMETERS  ::
// + node       : Node *      : Node pointer that the protocol is
//                              being instantiated in
// + appType    : AppType     : which application initiates this request
// + serverAddr : Address : server address
// + serverPort : short       : server port number
// RETURN      :: void :
// **/
void
APP_TcpServerListen(
    Node *node,
    AppType appType,
    Address serverAddr,
    short serverPort);

// /**
// API         :: APP_TcpServerListenWithPriority.
// PURPOSE     :: Listen on a server port with specified priority.
// PARAMETERS  ::
// + node       : Node *      : Node pointer that the protocol is
//                              being instantiated in
// + appType    : AppType     : which application initiates this request
// + serverAddr : NodeAddress : server address
// + serverPort : short       : server port number
// + priority   : TosType     : priority of this data for
//                                              this session.
// RETURN      :: void :
// **/
void
APP_TcpServerListenWithPriority(
    Node *node,
    AppType appType,
    NodeAddress serverAddr,
    short serverPort,
    TosType priority);

// /**
// API         :: APP_TcpServerListenWithPriority.
// PURPOSE     :: Listen on a server port with specified priority.
//                (Overloaded for IPv6)
// PARAMETERS  ::
// + node       : Node *      : Node pointer that the protocol is
//                              being instantiated in
// + appType    : AppType     : which application initiates this request
// + serverAddr : Address : server address
// + serverPort : short       : server port number
// + priority   : TosType     : priority of this data for
//                                              this session.
// RETURN      :: void :
// **/
void
APP_TcpServerListenWithPriority(
    Node *node,
    AppType appType,
    Address serverAddr,
    short serverPort,
    TosType priority);

// /**
// API        :: APP_TcpOpenConnection.
// PURPOSE    :: Open a connection.
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : NodeAddress : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: NodeAddress : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// +  waitTime : clocktype   : time until the session starts.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime);

// /**
// API        :: APP_TcpOpenConnection.
// PURPOSE    :: (Overloaded for IPv6) Open a connection.
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : Address : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: Address : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// +  waitTime : clocktype   : time until the session starts.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    Address localAddr,
    short localPort,
    Address remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime);

// /**
// API        :: APP_TcpOpenConnection.
// PURPOSE    :: Open a connection.
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : NodeAddress : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: NodeAddress : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// + waitTime : clocktype   : time until the session starts.
// + outgoingInterface : int : User specific outgoing Interface.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    int outgoingInterface);

// /**
// API        :: APP_TcpOpenConnection.
// PURPOSE    :: (Overloaded for IPv6) Open a connection.
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : Address : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: Address : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// + waitTime : clocktype   : time until the session starts.
// + outgoingInterface : int : User specific outgoing Interface.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnection(
    Node *node,
    AppType appType,
    Address localAddr,
    short localPort,
    Address remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    int outgoingInterface);

// /**
// API        :: APP_TcpOpenConnectionWithPriority.
// PURPOSE    :: Open a connection with specified priority.
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : NodeAddress : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: NodeAddress : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// + waitTime  : clocktype   : time until the session starts.
// + priority  : TosType     : priority of the data.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnectionWithPriority(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority);

// /**
// API        :: APP_TcpOpenConnectionWithPriority..
// PURPOSE    :: Open a connection with specified priority.
//               (Overloaded for IPv6)
// PARAMETERS ::
// + node      : Node *      : Node pointer that the protocol is
//                             being instantiated in
// + appType   : appType     : which application initiates this request
// + localAddr : Address : address of the source node.
// + localPort : short       : port number on the source node.
// + remoteAddr: Address : address of the remote node.
// + remotePort: short       : port number on the remote node (server port).
// + uniqueId  : int         : used to determine which client is requesting
//                             connection.
// + waitTime  : clocktype   : time until the session starts.
// + priority  : TosType     : priority of the data.
// RETURN     :: void :
// **/
void
APP_TcpOpenConnectionWithPriority(
    Node *node,
    AppType appType,
    Address localAddr,
    short localPort,
    Address remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority);


// API        :: APP_TcpOpenConnectionWithPriority..
// PURPOSE    :: Open a connection with specified priority.
// PARAMETERS ::
// + outgoingInterface : int : User specific outgoing Interface.
// RETURN     :: void :
void
APP_TcpOpenConnectionWithPriority(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority,
    int outgoingInterface);

// API        :: APP_TcpOpenConnectionWithPriority..
// PURPOSE    :: Open a connection with specified priority.
//               (Overloaded for IPv6)
// PARAMETERS ::
// + outgoingInterface : int : User specific outgoing Interface.
// RETURN     :: void :
void
APP_TcpOpenConnectionWithPriority(
    Node *node,
    AppType appType,
    Address localAddr,
    short localPort,
    Address remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority,
    int outgoingInterface);

// API        :: APP_TcpOpenConnectionWithPriority..
// PURPOSE    :: Open a connection with specified priority.
// PARAMETERS ::
// + outgoingInterface : int : User specific outgoing Interface.
// RETURN     :: void :
void
APP_TcpOpenConnectionWithPriority(
    Node *node,
    AppType appType,
    NodeAddress localAddr,
    short localPort,
    NodeAddress remoteAddr,
    short remotePort,
    int uniqueId,
    clocktype waitTime,
    TosType priority,
    int outgoingInterface);


// /**
// API         :: App_TcpCreateMessage.
// PURPOSE     :: Create the message.
// PARAMETERS  ::
// + node          : Node *           : Node pointer that the protocol is
//                                      being instantiated in
// + connId        : int              : connection id.
// + payload       : char *           : data to send.
// + length        : int              : length of the data to send.
// + traceProtocol : traceProtocolType : specify the type of application
//                                      used for packet tracing.
// RETURN         :: Message * :message created in the function
Message * App_TcpCreateMessage(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    UInt8 ttl);

// /**
// API            :: APP_TcpSendData.
// PURPOSE        :: send an application data unit.
// PARAMETERS     ::
// + node          : Node *           : Node pointer that the protocol is
//                                      being instantiated in
// + connId        : int              : connection id.
// + payload       : char *           : data to send.
// + length        : int              : length of the data to send.
// + traceProtocol : raceProtocolType : specify the type of application
//                                      used for packet tracing.
// RETURN         :: Message * :
// **/
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol
#ifdef ADDON_DB
    ,
    StatsDBAppEventParam* appParam = NULL,
    UInt8 ttl = IPDEFTTL);
#else
    ,UInt8 ttl = IPDEFTTL);
#endif

// /**
// // API            :: APP_TcpSendData.
// // PURPOSE        :: send an application data unit.
// // PARAMETERS     ::
// // + node          : Node *           : Node pointer that the protocol is
// //                                      being instantiated in
// // + connId        : int              : connection id.
// // + payload       : char *           : data to send.
// // + length        : int              : length of the data to send.
// // + traceProtocol : raceProtocolType : specify the type of application
// // + isEmulationPacket : BOOL         : Emulation packet?
// //                                      used for packet tracing.
// // RETURN         :: void :
// // **/
//
void
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    BOOL isEmulationPacket,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);


void
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    TraceProtocolType traceProtocol,
    char *appName,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

#ifdef ADDON_BOEINGFCS
Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *InfoData,
    int infoSize,
    unsigned short infoType,
    char *qosData,
    int qosSize,
    unsigned short qosType,
    TraceProtocolType traceProtocol,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);
#endif

Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *InfoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
    unsigned short tos,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam,
#endif
    UInt8 ttl = IPDEFTTL);


Message *
APP_TcpSendData(
    Node *node,
    int connId,
    char *payload,
    int length,
    char *infoData,
    int infoSize,
    unsigned short infoType,
    TraceProtocolType traceProtocol,
    unsigned short tos,
    NodeAddress clientAddr,
    int sourcePort,
    NodeAddress serverAddr,
    int destinationPort,
    int sessionId,
    int reqSize,
    int requiredTput,
    clocktype requiredDelay,
    float dataRate,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam,
#endif
    UInt8 ttl);


// /**
// API            :: APP_TcpSendNewHeaderVirtualData.
// PURPOSE        :: Send header and virtual data using TCP.
// PARAMETERS     ::
// + node          : Node *           : Node pointer that the protocol is
//                                      being instantiated in
// + connId        : int              : connection id.
// + header        : char *           : header to send.
// + headerLength  : int              : length of the header to send.
// + payloadSize   : int              : size of data to send along with header.
// + traceProtocol : raceProtocolType : specify the type of application
//                                      used for packet tracing.
// RETURN         :: Message* : Sent message
// **/
Message*
APP_TcpSendNewHeaderVirtualData(
    Node *node,
    int connId,
    char *header,
    int headerLength,
    int payloadSize,
    TraceProtocolType traceProtocol,
#ifdef ADDON_DB
    StatsDBAppEventParam* appParam = NULL,
#endif
    UInt8 ttl = IPDEFTTL);

// /**
// API        :: APP_TcpCloseConnection
// PURPOSE    :: Close the connection.
// PARAMETERS ::
// + node      : Node *  : Node pointer that the protocol is
//                         being instantiated in
// + connId    : int     : connection id.
// RETURN     :: void :
// **/
void
APP_TcpCloseConnection(
    Node *node,
    int connId);

// /**
// API        :: APP_InitMulticastGroupMembershipIfAny
// PURPOSE    :: Start process of joining multicast group if need to do so.
// PARAMETERS ::
// + node      : Node *          : node - node that is joining a group.
// + nodeInput : const NodeInput : used to access configuration file.
// RETURN     :: void :
// **/
void APP_InitMulticastGroupMembershipIfAny(
    Node *node,
    const NodeInput *nodeInput);

// /**
// API        :: APP_CheckMulticastByParsingSourceAndDestString
// PURPOSE    :: Application input parsing API. Parses the source and
//               destination strings.At the same time validates those
//               strings for multicast address.
// PARAMETERS ::
// + node            : Node *        : A pointer to Node.
// + inputString     : const char *  : The input string.
// + sourceString    : const char *  : The source string.
// + sourceNodeId    : NodeAddress * : A pointer to NodeAddress.
// + sourceAddr      : NodeAddress * : A pointer to NodeAddress.
// + destString      : const char *  : The destination string.
// + destNodeId      : NodeAddress * : A pointer to NodeAddress.
// + destAddr        : NodeAddress * : A pointer to NodeAddress.
// + isDestMulticast : BOOL *        : Pointer to multicast checking flag.
// RETURN :: void :
// **/
void APP_CheckMulticastByParsingSourceAndDestString(
    Node *node,
    const char *inputString,
    const char *sourceString,
    NodeAddress *sourceNodeId,
    NodeAddress *sourceAddr,
    const char *destString,
    NodeAddress *destNodeId,
    NodeAddress *destAddr,
    BOOL *isDestMulticast);

// /**
// API        :: APP_ParsingSourceAndDestString
// PURPOSE    :: API to parse the input source and destination strings read
//               from the *.app file.At the same time checks and fills the
//               destination type parameter.
// PARAMETERS ::
// + node            : Node *           : A pointer to Node.
// + inputString     : const char *     : The input string.
// + sourceString    : const char *     : The source string.
// + sourceNodeId    : NodeAddress *    : A pointer to NodeAddress.
// + sourceAddr      : NodeAddress *    : A pointer to NodeAddress.
// + destString      : const char *     : The destination string.
// + destNodeId      : NodeAddress *    : A pointer to NodeAddress.
// + destAddr        : NodeAddress *    : A pointer to NodeAddress.
// + destType        : DestinationType *: A pointer to Destinationtype.
// RETURN     :: void :
// **/
void APP_ParsingSourceAndDestString(
    Node* node,
    const char*  inputString,
    const char*  sourceString,
    NodeAddress* sourceNodeId,
    NodeAddress* sourceAddr,
    const char*  destString,
    NodeAddress* destNodeId,
    NodeAddress* destAddr,
    DestinationType* destType);

// /**
// API        :: APP_ParsingSourceAndDestString
// PURPOSE    :: API to parse the input source and destination strings read
//               from the *.app file. At the same time checks and fills the
//               destination type parameter.
// PARAMETERS ::
// + node            : Node *           : A pointer to Node.
// + inputString     : const char *     : The input string.
// + sourceString    : const char *     : The source string.
// + sourceNodeId    : NodeId *         : A pointer to NodeAddress.
// + sourceAddr      : Address *        : A pointer to NodeAddress.
// + destString      : const char *     : The destination string.
// + destNodeId      : NodeId *         : A pointer to NodeAddress.
// + destAddr        : Address *        : A pointer to NodeAddress.
// + destType        : DestinationType *: A pointer to DestinationType.
// RETURN     :: void :
// **/
void APP_ParsingSourceAndDestString(
    Node* node,
    const char* inputString,
    const char* sourceString,
    NodeId* sourceNodeId,
    Address* sourceAddr,
    const char* destString,
    NodeId* destNodeId,
    Address* destAddr,
    DestinationType* destType);


// /**
// API        :: APP_RegisterNewApp
// PURPOSE:   :: Insert a new application into the list of apps on this node.
//               Also inserts the port number being used for this app in the
//               port table.
// PARAMETERS ::
// + node      : Node*    : node that is registering the application.
// + appType   : AppType  : application type
// + dataPtr   : void *   : pointer to the data space for this app
// + myPort    : short    : port number to be inserted in the port table
// RETURN     :: AppInfo* : pointer to the new AppInfo data structure
// **/
AppInfo *
APP_RegisterNewApp(
    Node *node,
    AppType appType,
    void *dataPtr,
    unsigned short myPort);

// /**
// API        :: APP_IsFreePort
// PURPOSE:   :: Check if the port number is free or in use. Also check if
//               there is an application running at the node that uses
//               an AppType that has been assigned the same value as this
//               port number. This is done since applications such as CBR
//               use the value of AppType as destination port.
// PARAMETERS ::
// + node      : Node*    : node that is checking it's port table
// + portNumber: short    : port number to check
// RETURN     :: BOOL     : indicates if the port is free
// **/
BOOL APP_IsFreePort(
            Node* node,
            unsigned short portNumber);

// /**
// API        :: APP_GetFreePort
// PURPOSE:   :: Returns a free port
// PARAMETERS ::
// + node      : Node*    : node that is requesting a free port
// RETURN     :: short    : returns a free port
// **/
unsigned short APP_GetFreePort(Node *node);


short APP_GetFreePortForEachServer(Node *node, NodeAddress serverAddr);

// /**
// API        :: APP_InserInPortTable
// PURPOSE:   :: Insert an entry in the port table
// PARAMETERS ::
// + node     : Node*    : node that needs to be insert in port table
// + appType  : AppType  : application running at the port
// + myPort   : short    : port number to check
// RETURN     :: void :
// **/
void APP_InserInPortTable(
    Node* node,
    AppType appType,
    unsigned short myPort);

// /**
// API        :: APP_GetProtocolType
// PURPOSE:   :: Returns the protocol for which the message is destined
// PARAMETERS ::
// + node     : Node*    : node that received the message
// + msg      : Message* : pointer to the message received
// RETURN     : unsigned short : protocol which will receive the message
// **/
unsigned short APP_GetProtocolType(
    Node* node,
    Message* msg);

// /**
// API        :: APP_AssignTos
// PURPOSE    :: Application input parsing API. Parses the tos string and
//               tos value strings.At the same time validates those
//               strings for proper ranges.
// PARAMETERS ::
// + tosString       : char array  : The tos string.
// + tosValString    : char array  : The tos value string.
// + tosVal          : unsigned *  : A pointer to equivalent 8-bit TOS value.
// RETURN :: BOOL :
// **/
BOOL APP_AssignTos(
    char tosString[],
    char tosValString[],
    unsigned *tosVal);

// /**
// API             :: APP_UdpCreateNewHeaderVirtualDataWithPriority
// PURPOSE         :: Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Generally used with messenger app.
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + appType        : AppType           : type of application data
// + sourceAddr     : NodeAddress       : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : NodeAddress       : the destination node Id data
//                                        is sent to.
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN :: Message* : Pointer to allocated message structure
// **/
Message* APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    NodeAddress sourceAddr,
    short sourcePort,
    NodeAddress destAddr,
    char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);

// /**
// API             :: APP_UdpCreateNewHeaderVirtualDataWithPriority
// PURPOSE         :: Allocate header + virtual
//                    data with specified priority and send to UDP.
//                    Generally used with messenger app.
// PARAMETERS      ::
// + node           : Node *            : node that is sending the data.
// + appType        : AppType           : type of application data
// + sourceAddr     : Address           : the source sending the data.
// + sourcePort     : short             : the application source port.
// + destAddr       : Address           : the destination node Id data
//                                        is sent to.
// + header         : char*             : header of the payload.
// + headerSize     : int               : size of the header.
// + payloadSize    : int               : size of the data in bytes.
// + priority       : TosType           : priority of data.
// + traceProtocol  : TraceProtocolType : specify the type of application
//                                       used for packet tracing.
// RETURN :: Message* : Pointer to allocated message structure
// **/
Message* APP_UdpCreateNewHeaderVirtualDataWithPriority(
    Node *node,
    AppType appType,
    const Address& sourceAddr,
    short sourcePort,
    const Address& destAddr,
    const char *header,
    int headerSize,
    int payloadSize,
    TosType priority,
    TraceProtocolType traceProtocol);

// /**
// API        :: APP_UnregisterApp
// PURPOSE    :: Remove an application from list of apps
//               on this node.
// PARAMETERS ::
// + node      : Node*    : node that is unregistering the application.
// + dataPtr   : void *   : pointer to the data space for this app.
// + freeData   : bool   : if true, free (via MEM_free) the dataPtr
// RETURN     :: void : None
// **/
void
APP_UnregisterApp(
    Node *node,
    void *dataPtr,
    bool freeData = true);

// /**
// API        :: APP_UnregisterApp
// PURPOSE:   :: Remove an application from the list of apps on this node.
//               Also Remove the port number being used for this app in the
//               port table.
// PARAMETERS ::
// + node      : Node*    : node that is registering the application.
// + appType   : AppType  : application type
// + dataPtr   : void *   : pointer to the data space for this app
// + myPort    : short    : port number to be inserted in the port table
// RETURN     :: void : None
// **/
void
APP_UnregisterApp(
    Node *node,
    void *dataPtr,
    unsigned short myPort);

// /**
// API        :: APP_IsFreePort
// PURPOSE:   :: Check if the port number is free or in use. Also check if
//               there is an application running at the node that uses
//               an AppType that has been assigned the same value as this
//               port number. This is done since applications such as CBR
//               use the value of AppType as destination port.
// PARAMETERS ::
// + node      : Node*    : node that is checking it's port table.
// + portNumber: short    : port number to check.
// RETURN     :: BOOL     : indicates if the port is free.
// **/
BOOL APP_IsFreePort(
    Node* node,
    unsigned short portNumber,
    AppType appType);

// /**
// API        :: APP_RemoveFromPortTable
// PURPOSE:   :: Delete an entry from the port table
// PARAMETERS ::
// + node     : Node*    : node that needs to be remove from port table
// + myPort   : short    : port number to check
// RETURN     :: void :
// **/
void APP_RemoveFromPortTable(
    Node* node,
    unsigned short myPort);

short GetFreePort(
    Node *node,
    PortType first_port,
    int num_of_nodes);

BOOL IsNextPortFree(Node *node,
                    unsigned short sourcePort,
                    int num_of_nodes);

short GetFreePort(Node *node,PortType first_port,int num_of_nodes);
BOOL IsNextPortFree(Node *node,unsigned short sourcePort,int num_of_nodes);



#ifdef ADDON_DB
// /**
// FUNCTION     :: APP_ReportStatsDbReceiveEvent
// LAYER        :: Application
// PURPOSE      :: Report receive event to StatsDB app event table
//                 This function will check duplicate and out of order msgs
// PARAMETERS   ::
// + node        : Node*    : Pointer to a node who recieves the msg
// + msg         : Message* : The received message or fragment
// + seqCache    : SequenceNumber** : Pointer to the sequence number cache
//                                    which is used to detect duplicate
// + seqNo       : Int64    : Sequence number of the message or fragment
// + delay       : clocktype: Delay of the message/fragment
// + jitter      : clocktype: Smoothed jitter of the received message
// + size        : int      : Size of msg/fragment to be report to db
// + numRcvd     : int      : # of msgs/frags received so far
// + msgStatus   : AppMsgStatus : This is for performance optimization. If
//                            the app already know this is a new msg, it is
//                            helpful when dup record and out of order
//                            record are both disabled.
// Note: The last four parameters are required by the stats DB API
// RETURN       :: SequenceNumber::Status : Indicate whether the msg is dup
//                                          or out of order or new
// **/
SequenceNumber::Status APP_ReportStatsDbReceiveEvent(
                           Node* node,
                           Message* msg,
                           SequenceNumber **seqCache,
                           Int64 seqNo,
                           clocktype delay,
                           clocktype jitter,
                           int size,
                           int numRcvd,
                           AppMsgStatus msgStatus = APP_MSG_UNKNOWN);
#endif // ADDON_DB

/*
 * FUNCTION:   App_HandleIcmpMessage
 * PURPOSE:    Used to handle ICMP message received from transport layer
 * PARAMETERS: node - node that received the message.
 *             sourcePort - Source port of original message which generated
 *                          ICMP Error Message
 *             destinationPort - Destination port of original message which
 *                               generated ICMP Error Message
 *             icmpType - ICMP Meassage Type
 *             icmpCode - ICMP Message Code
 * RETURN:     void.
 */
void App_HandleIcmpMessage(
    Node *node,
    unsigned short sourcePort,
    unsigned short destinationPort,
    unsigned short icmpType,
    unsigned short icmpCode);

#endif /* _APP_UTIL_H_ */
