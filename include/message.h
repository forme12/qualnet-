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
// PACKAGE      :: MESSAGE
// DESCRIPTION  :: This file describes the message structure used to implement events
//                 and functions for message operations.
// **/


#ifndef MESSAGE_H
#define MESSAGE_H

#include <vector>
#include <string>

#include "main.h"
#include "clock.h"
#include "trace.h"
#include "qualnet_error.h"

class TimerManager;

// Size constants should be multiples of 8/sizeof(double).

// /**
// CONSTANT    :: MSG_MAX_HDR_SIZE             :   512
// DESCRIPTION :: Maximum Header Size
// **/
#define MSG_MAX_HDR_SIZE                           512

// /**
// CONSTANT    :: SMALL_INFO_SPACE_SIZE        :   112
// DESCRIPTION :: Size of small Info field.  Should be larger
//                than all commonly used info field data structures,
//                especially PropTxInfo and PropRxInfo.
// **/
#define SMALL_INFO_SPACE_SIZE                      112

// /**
// CONSTANT    :: MSG_PAYLOAD_LIST_MAX         :   1000
// DESCRIPTION :: Maximum message payload list
// **/
#define MSG_PAYLOAD_LIST_MAX                       1000

// /**
// CONSTANT    :: MAX_CACHED_PAYLOAD_SIZE      :   1024
// DESCRIPTION :: Maximum cached payload size
// **/
#define MAX_CACHED_PAYLOAD_SIZE                    1024

// /**
// CONSTANT    :: MSG_INFO_LIST_MAX            :   1000
// DESCRIPTION :: Maximum message info list
// **/
#define MSG_INFO_LIST_MAX                          1000

// /**
// CONSTANT    :: MAX_INFO_FIELDS              :   12
// DESCRIPTION :: Maximum number of info fields
// **/
#define MAX_INFO_FIELDS                            12

// Arbitrary constant to support packet trace facility

// /**
// CONSTANT    :: MAX_HEADERS                  :   10
// DESCRIPTION :: Maximum number of headers
// **/
#define MAX_HEADERS                                10


// /**
// STRUCT      :: MessageInfoHeader
// DESCRIPTION :: This is a structure which contains information
//                about a info field.
// **/
struct MessageInfoHeader
{
    unsigned short infoType; // type of the info field
    unsigned int infoSize; // size of buffer pointed to by "info" variable
    char* info;              // pointer to buffer for holding info
};

struct MessageInfoBookKeeping
{
    int msgSeqNum; // Sequence number of the message
    int fragSize;  // Fragment size.
    int infoLowerLimit; // starting index for the info field.
    int infoUpperLimit; // ending index for the info field + 1
};

// /**
// ENUM        :: MessageInfoType
// DESCRIPTION :: Type of information in the info field. One message can only
//                have up to one info field with a specific info type.
// **/
enum MessageInfoType
{
    INFO_TYPE_UNDEFINED = 0,  // an empty info field.
    INFO_TYPE_DEFAULT = 1,    // default info type used in situations where
                              // specific type is given to the info field.
    INFO_TYPE_AbstractCFPropagation, // type for abstract contention free
                                     // propagation info field.
    INFO_TYPE_AppName,      // Pass the App name down to IP layer
    INFO_TYPE_StatCategoryName,
    INFO_TYPE_DscpName,
    INFO_TYPE_SourceAddr,
    INFO_TYPE_SourcePort,
    INFO_TYPE_DestAddr,
    INFO_TYPE_DestPort,
    INFO_TYPE_DeliveredPacketTtlTotal, // Pass from IP to APP for session-based hop counts
    INFO_TYPE_IpTimeStamp,
    INFO_TYPE_DataSize,
    INFO_TYPE_AbstractPhy,

    // CES
    INFO_TYPE_SendTime,
    INFO_TYPE_SubnetId,
    INFO_TYPE_SpectrumInterfaceIndex,
    INFO_TYPE_SpectrumManagerInfo,
    INFO_TYPE_VoiceData,
    INFO_TYPE_QNHeaderInfo,
    INFO_TYPE_MacCesUsapCellHeaderInfo,
    INFO_TYPE_MacCesUsapReTxInfo,
    INFO_TYPE_MacCesUsapTruePacketInfo,
    INFO_TYPE_MacCesMonitorPacketInfo,
    INFO_TYPE_NetworkCesIncSincgarsPrecedence,
    INFO_TYPE_RoutingCesSdrDuplicate,
    INFO_TYPE_NetworkCesIncEplrsCTU,
    INFO_TYPE_NetworkCesIncEplrsTU,
    INFO_TYPE_MulticastCesSrwMospfGatewayAddress,
    INFO_TYPE_MacCesSrwPktAggregate,
    INFO_TYPE_MaByteStream,
    INFO_TYPE_LinkCesWnwAdaptation,
    INFO_TYPE_TimeSyncTimerInfo,
    INFO_TYPE_MacCesSrwSignalQuality,
    INFO_TYPE_TransportOverhead,
    INFO_TYPE_NetworkOverhead,
    INFO_TYPE_MacOverhead,
    INFO_TYPE_PhyOverhead,
    INFO_TYPE_ALE_ChannelIndex,
    INFO_TYPE_PhyIndex,
    INFO_TYPE_MdpInfoData,
    INFO_TYPE_MdpObjectInfo,
    INFO_TYPE_SuperAppUDPData,
    INFO_TYPE_SuperAppTCPData,
	INFO_TYPE_RTS_PREV,	  //YG
	INFO_TYPE_CTS_PREV,
	INFO_TYPE_APPDRIVE,
	INFO_TYPE_NODEPREV,


    // cellular
    INFO_TYPE_UmtsRlcPduListInfo,
    INFO_TYPE_UmtsRlcSduInfo,
    INFO_TYPE_UmtsRlcSduSegInfo,
    INFO_TYPE_AppPhoneCallData,
    INFO_TYPE_UmtsCellSwitch,

    INFO_TYPE_OriginalInsertTime,
    INFO_TYPE_ExternalData,
    INFO_TYPE_UdpFragData,
    INFO_TYPE_StatsTiming,
    INFO_TYPE_AppStatsDbContent,
    INFO_TYPE_StatsDbMapping,
    INFO_TYPE_StatsDbAppSessionId,
    INFO_TYPE_NetStatsDbContent,
    INFO_TYPE_MacSummaryStats,
    INFO_TYPE_MacConnStats,
    INFO_TYPE_MessageNextPrevHop,
    INFO_TYPE_MessageAddrInfo,
	INFO_TYPE_APPINFO,

    // EXATA
    INFO_TYPE_ForwardInfo,

    INFO_TYPE_PhyConnCrossPartition,
    INFO_TYPE_ForwardTcpHeader,

    // Network Security
    INFO_TYPE_PadLen,
    INFO_TYPE_IAHEP_RUTNG,
    INFO_TYPE_RPProcessed,
    INFO_TYPE_JAM,
    INFO_TYPE_IAHEP_NextHop,

    // CES
    INFO_TYPE_TransportToAppData,
    INFO_TYPE_NetworkCesIncSincgarsType,
    INFO_TYPE_QosReplyPacket,
    INFO_TYPE_MIQoSAgePacket,
    INFO_TYPE_HSLSRegionId,
    INFO_TYPE_HaipeNextHop,
    INFO_TYPE_MacCesSrwPort,
    INFO_TYPE_ROSPFRedirectMetadata,

    INFO_TYPE_IPPacketSentTime,
    INFO_TYPE_IpHeaderSize,
    INFO_TYPE_DelayInfo,
    INFO_TYPE_DidDropPacket,
    INFO_TYPE_SRWCNRVoice,
    INFO_TYPE_SRWConcatMsg,
    INFO_TYPE_SRW_MACInfo,
    INFO_TYPE_SRW_IntranetCNRApp,

    // JREAP
    INFO_TYPE_JNE_JREAP_AvatarInfo,

    INFO_TYPE_MODE5,

    INFO_TYPE_SNMPV3,

    INFO_TYPE_Dot16BurstInfo,    // represents 802.16 burst information type

    // LTE
    INFO_TYPE_LtePhyDlTtiInfo,
    INFO_TYPE_LtePhyTxInfo,
    INFO_TYPE_LtePhyCqiInfo,
    INFO_TYPE_LtePhyRiInfo,
    INFO_TYPE_LtePhySrsInfo,
    INFO_TYPE_LtePhyPss,
    INFO_TYPE_LtePhyRandamAccessGrant, // RA Grant
    INFO_TYPE_LtePhyRrcConnectionSetupComplete, // RRCConnectionSetupComplete
    INFO_TYPE_LtePhyRrcConnectionReconfComplete,
    INFO_TYPE_LtePhyRandamAccessTransmitPreamble,
    INFO_TYPE_LtePhyRandamAccessTransmitPreambleTimerDelay,
    INFO_TYPE_LtePhyRandamAccessTransmitPreambleInfo,
    INFO_TYPE_LtePhyLtePhyToMacInfo,
    INFO_TYPE_LteDciForUl,
    INFO_TYPE_LteDci0Info,
    INFO_TYPE_LteDci1Info,
    INFO_TYPE_LteDci2aInfo,
    INFO_TYPE_LteMacDestinationInfo,
    INFO_TYPE_LteMacNoTransportBlock, // If no TB in Message from MAC to
                                      // PHY, add this INFO
    INFO_TYPE_LteMacPeriodicBufferStatusReport, // Periodic BSR
    INFO_TYPE_LteMacRRELCIDFLWith7bitSubHeader,
    INFO_TYPE_LteMacRRELCIDFLWith15bitSubHeader,
    INFO_TYPE_LteMacRRELCIDSubHeader,
    INFO_TYPE_LteMacMultiplexingMsg,
    INFO_TYPE_LteMacNumMultiplexingMsg,
    INFO_TYPE_LteMacTxInfo,
    INFO_TYPE_LteRlcAmSduToPdu,
    INFO_TYPE_LteRlcAmPduFormatFixed,
    INFO_TYPE_LteRlcAmPduSegmentFormatFixed,
    INFO_TYPE_LteRlcAmPduFormatExtension,
    INFO_TYPE_LteRlcAmStatusPduPduFormatFixed,
    INFO_TYPE_LteRlcAmStatusPduPduFormatExtension,
    INFO_TYPE_LteRlcAmStatusPdu,
    INFO_TYPE_LteRlcAmResetData,
    INFO_TYPE_LteRlcAmPdcpPduInfo,
    INFO_TYPE_LtePdcpTxInfo,
    INFO_TYPE_LtePdcpSrcMsg,

    INFO_TYPE_LteEpcAppContainer,
    INFO_TYPE_LtePdcpDiscardTimerInfo,
    INFO_TYPE_LtePdcpBufferType,
    INFO_TYPE_LtePhyRrcMeasReport,      // measurement report list
    INFO_TYPE_LtePhyRrcConnReconf,

    INFO_TYPE_LteStatsDbSduPduInfo,

    INFO_TYPE_Dot11nTimerInfo,
    INFO_TYPE_Dot11nBER,

    // zigbee gts feature
    INFO_TYPE_Gts_Trigger_Precedence,
    INFO_TYPE_Gts_Slot_Start,
    INFO_TYPE_ZigbeeApp_Info,

	INFO_TYPE_ZyMacRRELCIDFLWith15bitSubHeader,
	INFO_TYPE_ZyMacMultiplexingMsg,
	INFO_TYPE_ZyMacNumMultiplexingMsg
};


// /**
// STRUCT      :: Message
// DESCRIPTION :: This is the main data strucure that represents a discrete
//                event in qualnet. This is used to represent timer as well
//                as to simulate actual sending of packets across the network.
// **/
class Message
{
private:
    static const UInt8 SENT = 0x01; // Message is being sent
    static const UInt8 FREED = 0x02; // MESSAGE_Free has been called
    static const UInt8 DELETED = 0x04; // Deleted using "delete"
    UInt8 m_flags;
public:
    // The default constructor should not be used unless under specific
    // circumstances.  The message is not initialized here.除非在特定情况下，否则不应使用默认构造函数。这里没有初始化消息。
    Message();

    Message(const Message& m);
    Message(PartitionData *partition,
            int  layerType,
            int  protocol,
            int  eventType,
            bool isMT = false);
    virtual ~Message();
    void operator = (const Message &p);

    // Initialize the message with default values
    void initialize(PartitionData* partition);

    Message*  next; // For kernel use only.
    PartitionData* m_partitionData; // For kernel use only.

    // The following fields are simulation related information.

    short layerType;    /// Layer which will receive the message
    short protocolType; /// Protocol which will receive the message in the layer.
    short instanceId;   /// Which instance to give message to (for multiple
                        /// copies of a protocol or application).
    short m_radioId;    /// which radio this belongs to (if any)
    short eventType;    /// Message's Event type.

    unsigned int naturalOrder;  /// used to maintain natural ordering
                                /// for events at same time & node

    char error;         /// Does the packet contain errors?

    bool    mtWasMT;            // Messages handed to the worker thread
                                // can't participate in the message recycling.
                                // As the partitionData->msgFreeList isn't
                                // locked.

    bool getSent() { return (m_flags & SENT) != 0; }
    void setSent(bool v);
    bool getFreed() { return (m_flags & FREED) != 0; }
    void setFreed(bool v);
    bool getDeleted() { return (m_flags & DELETED) != 0; }
    void setDeleted(bool v);

    bool      allowLoose;   // used only by the parallel code
    NodeId    nodeId;       // used only by the parallel code
    clocktype eventTime;    // used only by the parallel code
    clocktype eot;          // used only by the parallel code
    int sourcePartitionId;  // used only by the parallel code


    // An array of fields carries any information that needs to be
    // transported between layers.
    // It can be used for carrying data for messages which are not packets.
    // It can also be used to carry additional information for messages
    // which are packets.

    double smallInfoSpace[SMALL_INFO_SPACE_SIZE / sizeof(double)];


    // The following two fields are only used when the message is being
    // used to simulate an actual packt.

    // PacketSize field will indicate the simulated packet size. As a
    // packet moves up or down between the various layers, this field
    // will be updated to reflect the addition or deletion of the various
    // layer headers. For most purposes this does not have to be modified
    // by the users as it will be controlled through the following
    // functions: MESSAGE_AllocPacket, MESSAGE_AddHeader,
    // MESSAGE_RemoveHeader

    int packetSize;

    // The "packet" as seen by a particular layer for messages
    // which are used to simulate packets.

    char *packet;

    // This field is used for messages used to send packets. It is
    // used for internal error checking and should not be used by users.

    char *payload;

    // Size of the buffer pointed to by payload.
    // This field should never be changed by the user.

    int payloadSize;

    // Size of additional payload which should affect the
    // transmission delay of the packet, but need not be stored
    // in the actual char *payload

    int virtualPayloadSize;

    // If this is a packet, its the creation time.
    clocktype packetCreationTime;
    clocktype pktNetworkSendTime;

    bool cancelled;

    // Extra fields to support packet trace facility.
    // Will slow things down.
    NodeAddress originatingNodeId;
    int sequenceNumber;
    int originatingProtocol;
    int numberOfHeaders;
    int headerProtocols[MAX_HEADERS];
    int headerSizes[MAX_HEADERS];
	//double SINR;
    // Added field for SatCom parallel mode
    // holds the hw address of relay ground
    // node to prevent message repeat
    NodeAddress relayNodeAddr;

    std::vector<MessageInfoHeader> infoArray;
    std::vector<MessageInfoBookKeeping> infoBookKeeping;

// MILITARY_RADIOS_LIB
    int subChannelIndex;  // for multiple frequencies per interface
// MILITARY_RADIOS_LIB

    BOOL isPacked;
    int actualPktSize;

    bool isEmulationPacket;

	bool packetIsEmpty;
    TimerManager* timerManager;

    bool isScheduledOnMainHeap;
    clocktype timerExpiresAt;

    // This is needed for SRW Base Packet code. Not used in Qualnet code.
    int hdrLength;

    // Users should not modify anything above this line.
    // or below this one.
    // inline methods
    /// inserts virtual payload
    void addVirtualPayload(int size) { virtualPayloadSize += size; }

    /// removes virtual payload
    void removeVirtualPayload(int size)
    {
        virtualPayloadSize -= size;
        ERROR_Assert(virtualPayloadSize >= 0, "invalid virtual payload size");
    }

    /// returns a pointer to the beginning of the data packet.
    char* returnPacket() const { return packet; }

    /// returns the packet size, including virtual data.
    int returnPacketSize() const { return packetSize + virtualPayloadSize; }

    /// returns the size the packet is supposed to represent in cases where
    /// the implementation differs from the standard
    int returnActualSize() const { return (isPacked)? actualPktSize : packetSize;}

    /// returns the amount of virtual data in the packet
    int returnVirtualSize() const { return virtualPayloadSize; }

    /// sets the layer and protocol for mapping the appropriate event handler
    void setLayer(int layer, int protocol) {
        layerType = layer;
        protocolType = protocol;
    }

    /// Returns the layer associated with a message
    int getLayer() const { return layerType; }

    /// Returns the protocol associated with a message
    int getProtocol() const { return protocolType; }

    /// Set the event associated with a message
    void setEvent(int event) { eventType = event; }

    /// Returns the event type of a message
    int getEvent() const { return eventType; }

    /// Sets the instanceId of a message
    void setInstanceId(int instance) { instanceId = instance; }

    /// Returns the instanceId of a message
    int getInstanceId() const { return instanceId; }

    void radioId(int p_radioId) { m_radioId = (short)p_radioId; }
    int radioId() const { return (int)m_radioId; }
    bool hasRadioId() const { return m_radioId >= 0; }

    /// Returns the packet creation time
    clocktype getPacketCreationTime() { return packetCreationTime; }
};

// Macros for legacy support.
#define MESSAGE_AddVirtualPayload(node, msg, payloadSize) \
        (msg->addVirtualPayload(payloadSize))
#define MESSAGE_RemoveVirtualPayload(node, msg, payloadSize) \
        (msg->removeVirtualPayload(payloadSize))
#define MESSAGE_ReturnPacket(msg) (msg->returnPacket())
#define MESSAGE_ReturnPacketSize(msg) (msg->returnPacketSize())
#define MESSAGE_ReturnActualPacketSize(msg) (msg->returnActualSize())
#define MESSAGE_ReturnVirtualPacketSize(msg) (msg->returnVirtualSize())
#define MESSAGE_SetLayer(msg, layer, protocol) (msg->setLayer(layer, protocol))
#define MESSAGE_GetLayer(msg) (msg->getLayer())
#define MESSAGE_GetProtocol(msg) (msg->getProtocol())
#define MESSAGE_SetEvent(msg, event) (msg->setEvent(event))
#define MESSAGE_GetEvent(msg) (msg->getEvent())
#define MESSAGE_SetInstanceId(msg, instance) (msg->setInstanceId(instance))
#define MESSAGE_GetInstanceId(msg) (msg->getInstanceId())
#define MESSAGE_GetPacketCreationTime(msg) (msg->getPacketCreationTime())


// mtPendingSend count that prevents wrap around - will leak, but won't crash.
#define MESSAGE_MT_PENDING_SEND_INFINITE 255

// /**
// API       :: MESSAGE_PrintMessage
// LAYER     :: ANY LAYER
// PURPOSE      Print out the contents of the message for debugging purposes.
// PARAMETERS ::
// + node    :  Node*     : node which is sending message
// + msg     :  Message*  : message to be printed
// RETURN    :: void : NULL
// **/
void MESSAGE_PrintMessage(Message* msg);

// /**
// API       :: MESSAGE_Send
// LAYER     :: ANY LAYER
// PURPOSE   :: Function call used to send a message within QualNet. When
//              a message is sent using this mechanism, only the pointer
//              to the message is actually sent through the system. So the
//              user has to be careful not to do anything with the content
//              of the pointer once MESSAGE_Send has been called.
// PARAMETERS ::
// + node    :  Node*     : node which is sending message
// + msg     :  Message*  : message to be delivered
// + delay   :  clocktype : delay suffered by this message.
// + isMT    :  bool      : is the function being called from a thread?
// RETURN    :: void : NULL
// **/
void MESSAGE_Send(Node*     node,
                  Message*  msg,
                  clocktype delay,
                  bool      isMT = false);

// /**
// API       :: MESSAGE_SendMT
// LAYER     :: ANY LAYER
// PURPOSE   :: Function call used to send a message from independent
//              threads running within QualNet, for example those associated
//              with external interfaces.
// PARAMETERS ::
// + node    :  Node*     : node which is sending message
// + msg     :  Message*  : message to be delivered
// + delay   :  clocktype : delay suffered by this message.
// RETURN    :: void : NULL
// **/
static void MESSAGE_SendMT(Node *node, Message *msg, clocktype delay) {
    MESSAGE_Send(node, msg, delay, true);
}

// /**
// API       :: MESSAGE_RemoteSend
// LAYER     :: ANY_LAYER
// PURPOSE   :: Function used to send a message to a node that might be
//              on a remote partition.  The system will make a shallow copy
//              of the message, meaning it can't contain any pointers in
//              the info field or the packet itself.  This function is very
//              unsafe.  If you use it, your program will probably crash.
//              Only I can use it.
//
// PARAMETERS ::
// + node       :  Node*     : node which is sending message
// + destNodeId :  NodeId    : nodeId of receiving node
// + msg        :  Message*  : message to be delivered
// + delay      :  clocktype : delay suffered by this message.
// RETURN    :: void : NULL
// **/
void MESSAGE_RemoteSend(Node*     node,
                        NodeId    destNodeId,
                        Message*  msg,
                        clocktype delay);

// /**
// API       :: MESSAGE_RouteReceivedRemoteEvent
// LAYER     :: ANY_LAYER
// PURPOSE   :: Counterpart to MESSAGE_RemoteSend, this function allows
//              models that send remote messages to provide special handling
//              for them on the receiving partition.  This function is
//              called in real time as the messages are received, so must
//              be used carefully.
//
// PARAMETERS ::
// + node       :  Node*     : node which is sending message
// + msg        :  Message*  : message to be delivered
// RETURN    :: void : NULL
// **/
void MESSAGE_RouteReceivedRemoteEvent(Node*    node,
                                      Message* msg);

// /**
// API       :: MESSAGE_CancelSelfMsg
// LAYER     :: ANY LAYER
// PURPOSE   :: Function call used to cancel a event message in the
//              QualNet scheduler.  The Message must be a self message
//              (timer) .i.e. a message a node sent to itself.  The
//              msgToCancelPtr must a pointer to the original message
//              that needs to be canceled.
// PARAMETERS ::
// + node    :  Node*   : node which is sending message
// + msgToCancelPtr     :  Message* : message to be cancelled
// RETURN    :: void : NULL
// **/
static void MESSAGE_CancelSelfMsg(Node *node, Message *msgToCancelPtr) {
   msgToCancelPtr->cancelled = TRUE;
}

static void MESSAGE_SetLooseScheduling(Message *msg) {
    msg->allowLoose = true;
}

static bool MESSAGE_AllowLooseScheduling(Message *msg) {
    return (msg->allowLoose);
}

// /**
// API        :: MESSAGE_Alloc
// LAYER      :: ANY LAYER
// PURPOSE    :: Allocate a new Message structure. This is called when a
//               new message has to be sent through the system. The last
//               three parameters indicate the layerType, protocol and the
//               eventType that will be set for this message.
// PARAMETERS ::
// + node     :  Node* : node which is allocating message
// + layerType:  int : Layer type to be set for this message
// + protocol :  int : Protocol to be set for this message
// + eventType:  int : event type to be set for this message
// RETURN     :: Message* : Pointer to allocated message structure
// **/
Message* MESSAGE_Alloc(
    Node *node, int layerType, int protocol, int eventType, bool isMT = false);  //产生的包括节点，层类型，协议，项目类型

// /**
// API        :: MESSAGE_Alloc
// LAYER      :: ANY LAYER
// PURPOSE    :: Allocate a new Message structure. This is called when a
//               new message has to be sent through the system. The last
//               three parameters indicate the layerType, protocol and the
//               eventType that will be set for this message.
// PARAMETERS ::
// + partition:  PartitionData* : partition that is allocating message
// + layerType:  int : Layer type to be set for this message
// + protocol :  int : Protocol to be set for this message
// + eventType:  int : event type to be set for this message
// RETURN     :: Message* : Pointer to allocated message structure
// **/
Message* MESSAGE_Alloc(PartitionData *partition,
                       int layerType,
                       int protocol,
                       int eventType,
    bool isMT = false);

// /**
// API        :: MESSAGE_AllocMT
// LAYER      :: ANY LAYER
// PURPOSE    :: Mutli-thread safe version of MESSAGE_Alloc for use
//               by worker threads.
// PARAMETERS ::
// + partition:  PartitionData* : partition that is allocating message
// + layerType:  int : Layer type to be set for this message
// + protocol :  int : Protocol to be set for this message
// + eventType:  int : event type to be set for this message
// RETURN     :: Message* : Pointer to allocated message structure
// **/
static
Message* MESSAGE_AllocMT(PartitionData *partition,
                         int layerType,
                         int protocol,
                         int eventType)
{
    return MESSAGE_Alloc(partition, layerType, protocol, eventType, true);
}

// /**
// API       :: MESSAGE_InfoFieldAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate space for one "info" field
// PARAMETERS ::
// + node    :  Node* : node which is allocating the space.
// + infoSize:  int : size of the space to be allocated
// RETURN    :: char* : pointer to the allocated space.
// **/
char* MESSAGE_InfoFieldAlloc(Node *node, int infoSize, bool isMT = false);

// /**
// API       :: MESSAGE_InfoFieldAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate space for one "info" field
// PARAMETERS ::
// + partition: PartitionData* : partition which is allocating the space.
// + infoSize:  int : size of the space to be allocated
// RETURN    :: char* : pointer to the allocated space.
// **/
char* MESSAGE_InfoFieldAlloc(PartitionData *partition, int infoSize,
                             bool isMT = false);

// /**
// API       :: MESSAGE_InfoFieldAllocMT
// PURPOSE   :: Multi-thread safe version of MESSAGE_InfoFieldAlloc
// PARAMETERS ::
// + partition: PartitionData* : partition which is allocating the space.
// + infoSize:  int : size of the space to be allocated
// RETURN    :: char* : pointer to the allocated space.
// **/
static
char* MESSAGE_InfoFieldAllocMT(PartitionData *partition, int infoSize)
{
    return MESSAGE_InfoFieldAlloc(partition, infoSize, true);
}

// /**
// API       :: MESSAGE_InfoFieldFree
// LAYER     :: ANY LAYER
// PURPOSE   :: Free space for one "info" field
// PARAMETERS ::
// + node    :  Node* : node which is allocating the space.
// + hdrPtr  :  MessageInfoHeader* : pointer to the "info" field
// RETURN    :: void : NULL
// **/
void MESSAGE_InfoFieldFree(Node *node, MessageInfoHeader* hdrPtr,
                           bool isMT = false);

// API       :: MESSAGE_InfoFieldFree
// LAYER     :: ANY LAYER
// PURPOSE   :: Free space for one "info" field
// PARAMETERS ::
// + partition:  PartitionData* : partition which is allocating the space.
// + hdrPtr  :  MessageInfoHeader* : pointer to the "info" field
// RETURN    :: void : NULL
// **/
void MESSAGE_InfoFieldFree(PartitionData *partition,
                           MessageInfoHeader* hdrPtr, bool isMT);

// API       :: MESSAGE_InfoFieldFreeMT
// LAYER     :: ANY LAYER
// PURPOSE   :: Multithread safe version of MESSAGE_InfoFieldFree ()
// PARAMETERS ::
// + partition:  PartitionData* : partition which is allocating the space.
// + hdrPtr  :  MessageInfoHeader* : pointer to the "info" field
// RETURN    :: void : NULL
// **/
static
void MESSAGE_InfoFieldFreeMT(PartitionData *partition,
                             MessageInfoHeader* hdrPtr) {
    MESSAGE_InfoFieldFree(partition, hdrPtr, true);
}

// /**
// API       :: MESSAGE_AddInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate one "info" field with given info type for the
//              message. This function is used for the delivery of data
//              for messages which are NOT packets as well as the delivery
//              of extra information for messages which are packets. If a
//              "info" field with the same info type has previously been
//              allocated for the message, it will be replaced by a new
//              "info" field with the specified size. Once this function
//              has been called, MESSAGE_ReturnInfo function can be used
//              to get a pointer to the allocated space for the info field
//              in the message structure.
// PARAMETERS ::
// + node    :  Node* : node which is allocating the info field.
// + msg     :  Message* : message for which "info" field
//                         has to be allocated
// + infoSize:  int : size of the "info" field to be allocated
// + infoType:  unsigned short : type of the "info" field to be allocated.
// RETURN    :: char* : Pointer to the added info field
// **/
char* MESSAGE_AddInfo(Node *node,
                      Message *msg,
                      int infoSize,
                      unsigned short infoType = INFO_TYPE_DEFAULT);

// /**
// API       :: MESSAGE_AddInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate one "info" field with given info type for the
//              message. This function is used for the delivery of data
//              for messages which are NOT packets as well as the delivery
//              of extra information for messages which are packets. If a
//              "info" field with the same info type has previously been
//              allocated for the message, it will be replaced by a new
//              "info" field with the specified size. Once this function
//              has been called, MESSAGE_ReturnInfo function can be used
//              to get a pointer to the allocated space for the info field
//              in the message structure.
// PARAMETERS ::
// + partition:  PartitionData* : partition which is allocating the info field.
// + msg     :  Message* : message for which "info" field
//                         has to be allocated
// + infoSize:  int : size of the "info" field to be allocated
// + infoType:  unsigned short : type of the "info" field to be allocated.
// RETURN    :: char* : Pointer to the added info field
// **/
char* MESSAGE_AddInfo(PartitionData *partition,
                      Message *msg,
                      int infoSize,
                      unsigned short infoType = INFO_TYPE_DEFAULT);

// /**
// API       :: MESSAGE_RemoveInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Remove one "info" field with given info type from the
//              info array of the message.
// PARAMETERS ::
// + node    :  Node* : node which is removing info field.
// + msg     :  Message* : message for which "info" field
//                         has to be removed
// + infoType:  unsigned short : type of the "info" field to be removed.
// RETURN    :: void : NULL
// **/
void MESSAGE_RemoveInfo(Node *node, Message *msg, unsigned short infoType);

// /**
// API       :: MESSAGE_InfoAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate the default "info" field for the message. This
//              function is similar to MESSAGE_AddInfo. The difference
//              is that it assumes the type of the info field to be
//              allocated is INFO_TYPE_DEFAULT.
// PARAMETERS ::
// + node    :  Node* : node which is allocating the info field.
// + msg     :  Message* : message for which "info" field
//                         has to be allocated
// + infoSize:  int : size of the "info" field to be allocated
// RETURN    :: char * :
// **/
static char * MESSAGE_InfoAlloc(Node *node, Message *msg, int infoSize)
{
    return (MESSAGE_AddInfo(node,
                            msg,
                            infoSize));
}

// /**
// API       :: MESSAGE_InfoAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate the default "info" field for the message. This
//              function is similar to MESSAGE_AddInfo. The difference
//              is that it assumes the type of the info field to be
//              allocated is INFO_TYPE_DEFAULT.
// PARAMETERS ::
// + partition:  PartitionData* : partition which is allocating the info field.
// + msg     :  Message* : message for which "info" field
//                         has to be allocated
// + infoSize:  int : size of the "info" field to be allocated
// RETURN    :: char * :
// **/
static char * MESSAGE_InfoAlloc(PartitionData *partition, Message *msg, int infoSize)
{
    return (MESSAGE_AddInfo(partition,
                            msg,
                            infoSize));
}

// /**
// API       :: MESSAGE_ReturnInfoSize
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the size of a "info" field with given info type
//              in the info array of the message.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + infoType:  unsigned short : type of the "info" field.
// + fragmentNumber: int: Location of the fragment in the TCP packet
// RETURN    :: int : size of the info field.
// **/
static int MESSAGE_ReturnInfoSize(Message *msg,
                                  unsigned short infoType,
                                  int fragmentNumber)
{
    int i;
    if ((unsigned)fragmentNumber >= msg->infoBookKeeping.size())
    {
        return 0;
    }
    int infoLowerLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
    int infoUpperLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit;

    for (i = infoLowerLimit; i < infoUpperLimit; i ++)
    {
        if (msg->infoArray[i].infoType == infoType)
        {
            return msg->infoArray[i].infoSize;
        }
    }

    return 0;
}
// /**
// API       :: MESSAGE_ReturnInfoSize
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the size of a "info" field with given info type
//              in the info array of the message.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + infoType:  unsigned short : type of the "info" field.
// RETURN    :: int : size of the info field.
// **/
static int MESSAGE_ReturnInfoSize(const Message* msg,
                                  unsigned short infoType = INFO_TYPE_DEFAULT)
{
    unsigned int i;

    if (msg->infoArray.size() > 0)
    {
        /*if (infoType == INFO_TYPE_DEFAULT)
        {
            return msg->infoArray[0].infoSize;
        }*/


        for (i = 0; i < msg->infoArray.size(); i ++)
        {
            MessageInfoHeader* hdrPtr = (MessageInfoHeader*)&(msg->infoArray[i]);
            if (hdrPtr->infoType == infoType)
            {
                return hdrPtr->infoSize;
            }
        }
    }

    return 0;
}

// /**
// API       :: MESSAGE_ReturnInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns a pointer to the "info" field with given info type
//              in the info array of the message.返回一个指向“info”字段的指针，该字段在消息的info数组中具有给定的info类型。
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + infoType:  unsigned short : type of the "info" field to be returned.
// RETURN    :: char* : Pointer to the "info" field with given type.
//                      NULL if not found.
// **/
static char* MESSAGE_ReturnInfo(const Message *msg,
                                unsigned short infoType = INFO_TYPE_DEFAULT)
{
    unsigned int i;

    if (msg->infoArray.size() > 0)
    {
        /*if (infoType == INFO_TYPE_DEFAULT)
        {
            return msg->infoArray[0].info;
        }*/

        for (i = 0; i < msg->infoArray.size(); i ++)
        {
            MessageInfoHeader* hdrPtr = (MessageInfoHeader*)&(msg->infoArray[i]);
            if (hdrPtr->infoType == infoType)
            {
                return hdrPtr->info;
            }
        }
    }
    return NULL;
}

// /**
// API       :: MESSAGE_CopyInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Copy the "info" fields of the source message to
//              the destination message.
// PARAMETERS ::
// + node    :  Node*    : Node which is copying the info fields
// + dsgMsg  :  Message* : Destination message
// + srcMsg  :  Message* : Source message
// RETURN    :: void : NULL
// **/
void MESSAGE_CopyInfo(Node *node, Message *dstMsg, Message *srcMsg);

// /**
// API       :: MESSAGE_CopyInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Copy the "info" fields of the source info header to
//              the destination message.
// PARAMETERS ::
// + node    :  Node*    : Node which is copying the info fields
// + dsgMsg  :  Message* : Destination message
// + srcInfo  :  MessageInfoHeader* : Info Header structure
// RETURN    :: void : NULL
// **/
void MESSAGE_CopyInfo(Node *node, Message *dstMsg, std::vector<MessageInfoHeader*> srcInfo);

// /**
// /**
// API       :: MESSAGE_ReturnInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns a pointer to the "info" field with given info type
//              in the info array of the message.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + infoType:  unsigned short : type of the "info" field to be returned.
// + fragmentNumber: int: Location of the fragment in the TCP packet.
// RETURN    :: char* : Pointer to the "info" field with given type.
//                      NULL if not found.
// **/
static char* MESSAGE_ReturnInfo(const Message *msg,
                                unsigned short infoType,
                                int fragmentNumber)
{
    int i;
    if ((unsigned int)fragmentNumber >= msg->infoBookKeeping.size())
    {
        return NULL;
    }
    int infoLowerLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
    int infoUpperLimit =
        msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit;

    for (i = infoLowerLimit ; i < infoUpperLimit; i ++)
    {
        if (msg->infoArray[i].infoType == infoType)
        {
            return msg->infoArray[i].info;
        }
    }

    return NULL;
}


// /**
// API       :: MESSAGE_FragmentPacket
// LAYER     :: ANY LAYER
// PURPOSE   :: Fragment one packet into multiple fragments
//              Note: The original packet will be freed in this function.
//                    The array for storing pointers to fragments will be
//                    dynamically allocated. The caller of this function
//                    will need to free the memory.
// PARAMETERS ::
// + node    :  Node* : node which is fragmenting the packet
// + msg     :  Message* : The packet to be fragmented
// + fragUnit:  int : The unit size for fragmenting the packet
// + fragList:  Message*** : A list of fragments created.
// + numFrags:  int* : Number of fragments in the fragment list.
// + protocolType : TraceProtocolType : Protocol type for packet tracing.
// RETURN    :: void : NULL
// **/
void MESSAGE_FragmentPacket(
         Node* node,
         Message* msg,
         int fragUnit,
         Message*** fragList,
         int* numFrags,
         TraceProtocolType protocolType);

// /**
// API       :: MESSAGE_ReassemblePacket
// LAYER     :: ANY LAYER
// PURPOSE   :: Reassemble multiple fragments into one packet
//              Note: All the fragments will be freed in this function.
// PARAMETERS ::
// + node    :  Node* : node which is assembling the packet
// + fragList:  Message** : A list of fragments.
// + numFrags:  int : Number of fragments in the fragment list.
// + protocolType : TraceProtocolType : Protocol type for packet tracing.
// RETURN    :: Message* : The reassembled packet.
// **/
Message* MESSAGE_ReassemblePacket(
             Node* node,
             Message** fragList,
             int numFrags,
             TraceProtocolType protocolType);

// FUNCTION   :: MESSAGE_Serialize
// LAYER      :: ANY
// PURPOSE    :: Serialize a single message into a buffer so that the orignal
//               message can be recovered from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to a messages
// + buffer    : string   : The string buffer the message will be serialzed into (append to the end)
// RETURN     :: void     : NULL
// **/
void MESSAGE_Serialize(Node* node,
                       Message* msg,
                       std::string& buffer);

// FUNCTION   :: MESSAGE_Serialize
// LAYER      :: ANY
// PURPOSE    :: Used in MPI code to serialize a single message into a buffer
//               for transmission to another processor
// PARAMETERS ::
// + msg       : Message* : Pointer to a message
// + buffer    : string   : The buffer the message will be serialzed into
// RETURN     :: int     : size of message in buffer
// **/
int MESSAGE_Serialize(Message* msg,
              unsigned char buffer[]);

// FUNCTION   :: MESSAGE_Unserialize
// LAYER      :: MAC
// PURPOSE    :: recover the orignal message from the buffer
// PARAMETERS ::
// + partitionData : PartitionData* : Pointer to partition data`:w
// + buffer    : string   : The string buffer containing the message was serialzed into
// + bufIndex  : size_t&  : the start position in the buffer pointing to the message
//                          updated to the end of the message after the unserialization.
// RETURN     :: Message* : Message pointer to be recovered
// **/
Message* MESSAGE_Unserialize(PartitionData* partitionData,
                             const char* buffer,
                             int& bufIndex,
                             bool mt = false);

// FUNCTION   :: MESSAGE_SerializeMsgList
// LAYER      :: MAC
// PURPOSE    :: Store a list of messages into a buffer so that the orignal
//               messages can be recovered from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to a message list
// + buffer    : string&  : The string buffer the messages will be serialzed into (append to the end)
// RETURN     :: int      : number of messages in the list
// **/
int MESSAGE_SerializeMsgList(Node* node,
                             Message* msgList,
                             std::string& buffer);

// /**
// FUNCTION   :: MESSAGE_PackMessage
// LAYER      :: MAC
// PURPOSE    :: Pack a list of messages to be one message structure
//               Whole contents of the list messages will be put as
//               payload of the new message. So the packet size of
//               the new message cannot be directly used now.
//               The original lis of msgs will be freed.
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msgList   : Message* : Pointer to a list of messages
// + origProtocol: TraceProtocolType : Protocol allocating this packet
// + actualPktSize : int* : For return sum of packet size of msgs in list
// RETURN     :: Message* : The super msg contains a list of msgs as payload
// **/
Message* MESSAGE_PackMessage(Node* node,
                             Message* msgList,
                             TraceProtocolType origProtocol,
                             int* actualPktSize);

// /**
// FUNCTION   :: MESSAGE_UnpackMessage
// LAYER      :: MAC
// PURPOSE    :: Unpack a super message to the original list of messages
//               The list of messages were stored as payload of this super
//               message.
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to the supper msg contains list of msgs
// + copyInfo  : bool     : Whether copy info from old msg to first msg
// + freeOld   : bool     : Whether the original message should be freed
// RETURN     :: Message* : A list of messages unpacked from original msg
// **/
Message* MESSAGE_UnpackMessage(Node* node,
                               Message* msg,
                               bool copyInfo,
                               bool freeOld);

// FUNCTION   :: MESSAGE_UnserializeMsgList
// LAYER      :: MAC
// PURPOSE    :: recover the orignal message list from the buffer
// PARAMETERS ::
// + partitionData : PartitionData*    : Pointer to partition data.
// + buffer    : string   : The string buffer containing the message list serialzed into
// + bufIndex  : size_t&  : the start position in the buffer pointing to the message list
//                          updated to the end of the message list after the unserialization.
// + numMsgs   : size_t   : Number of messages in the list
// RETURN     :: Message* : Pointer to the message list to be recovered
// **/
Message* MESSAGE_UnserializeMsgList(PartitionData* partitionData,
                                    const char* buffer,
                                    int& bufIndex,
                                    unsigned int numMsgs);

// FUNCTION   :: MESSAGE_Serialize
// LAYER      :: ANY
// PURPOSE    :: Serialize a single message into a buffer so that the orignal
//               message can be recovered from the buffer
// PARAMETERS ::
// + node      : Node*    : Pointer to node.
// + msg       : Message* : Pointer to a messages
// + buffer    : string   : The string buffer the message will be serialzed into (append to the end)
// RETURN     :: void     : NULL
// **/
void MESSAGE_Serialize(Node* node,
                       Message* msg,
                       std::string& buffer);

// /**
// API       :: MESSAGE_PacketAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate the "payload" field for the packet to be delivered.
//              Add additional free space in front of the packet for
//              headers that might be added to the packet. This function
//              can be called from the application layer or anywhere else
//              (e.g TCP, IP) that a packet may originiate from. The
//              "packetSize" variable will be set to the "packetSize"
//              parameter specified in the function call. Once this function
//              has been called the "packet" variable in the message
//              structure can be used to access this space.
// PARAMETERS ::
// + node    :  Node* : node which is allocating the packet
// + msg     :  Message* : message for which packet has to be allocated
// + packetSize: int : size of the packet to be allocated
// + originalProtocol: TraceProtocolType : Protocol allocating this packet
// RETURN    :: void : NULL
// **/
void MESSAGE_PacketAlloc(Node *node,
                         Message *msg,
                         int packetSize,
                         TraceProtocolType originalProtocol);

// /**
// API       :: MESSAGE_PacketAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate the "payload" field for the packet to be delivered.
//              Add additional free space in front of the packet for
//              headers that might be added to the packet. This function
//              can be called from the application layer or anywhere else
//              (e.g TCP, IP) that a packet may originiate from. The
//              "packetSize" variable will be set to the "packetSize"
//              parameter specified in the function call. Once this function
//              has been called the "packet" variable in the message
//              structure can be used to access this space.
// PARAMETERS ::
// + partition:  PartitionData* : artition which is allocating the packet
// + msg     :  Message* : message for which packet has to be allocated
// + packetSize: int : size of the packet to be allocated
// + originalProtocol: TraceProtocolType : Protocol allocating this packet
// + isMT    : bool   : Is this packet being created from a worker thread
// RETURN    :: void : NULL
// **/
void MESSAGE_PacketAlloc(PartitionData *partition,
                         Message *msg,
                         int packetSize,
                         TraceProtocolType originalProtocol,
                         bool isMT = false);

// /**
// API       :: MESSAGE_AddHeader
// LAYER     :: ANY LAYER
// PURPOSE   :: This function is called to reserve additional space for a
//              header of size "hdrSize" for the packet enclosed in the
//              message. The "packetSize" variable in the message structure
//              will be increased by "hdrSize".
//              Since the header has to be prepended to the current packet,
//              after this function is called the "packet" variable in the
//              message structure will point the space occupied by this new
//              header.
// PARAMETERS ::
// + node    : Node*    : node which is adding header
// + msg     : Message* : message for which header has to be added
// + hdrSize : int      : size of the header to be added
// + traceProtocol: TraceProtocolType : protocol name, from trace.h
// RETURN    :: void : NULL
// **/
void MESSAGE_AddHeader(Node *node,
                       Message *msg,
                       int hdrSize,
                       TraceProtocolType traceProtocol);

// /**
// API       :: MESSAGE_RemoveHeader
// LAYER     :: ANY LAYER
// PURPOSE   :: This function is called to remove a header from the packet.
//              The "packetSize" variable in the message will be decreased
//              by "hdrSize".
// PARAMETERS ::
// + node    :  Node*       : node which is removing the packet header
// + msg     :  Message*    : message for which header is being removed
// + hdrSize :  int         : size of the header being removed
// + traceProtocol: TraceProtocolType : protocol removing this header.
// RETURN    :: void : NULL
// **/
void MESSAGE_RemoveHeader(Node *node,
                          Message *msg,
                          int hdrSize,
                          TraceProtocolType traceProtocol);

// /**
// API       :: MESSAGE_ReturnHeader
// LAYER     :: ANY LAYER
// PURPOSE   :: This is kind of a hack so that MAC protocols (dot11) that
//              need to peak at a packet that still has the PHY header can
//              return the contents after the first (N) headers without
//              first removing those headers.
// PARAMETERS ::
// + msg     :  Message* : message containing a packet with headers
// + header  :  int      : number of the header to return.
// RETURN    :: char* : the packet starting at the header'th header
// **/
char* MESSAGE_ReturnHeader(const Message* msg,
                           int            header);

// /**
// API       :: MESSAGE_ExpandPacket
// LAYER     :: ANY LAYER
// PURPOSE   :: Expand packet by a specified size
// PARAMETERS ::
// + node    :  Node* : node which is expanding the packet
// + msg     :  Message* : message which is to be expanded
// + size    :  int : size to expand
// RETURN    :: void : NULL
// **/
void MESSAGE_ExpandPacket(Node *node,
                          Message *msg,
                          int size);

// /**
// API       :: MESSAGE_ShrinkPacket
// LAYER     :: ANY LAYER
// PURPOSE   :: This function is called to shrink
//              packet by a specified size.
// PARAMETERS ::
// + node    :  Node* : node which is shrinking packet
// + msg     :  Message* : message whose packet is be shrinked
// + size    :  int : size to shrink
// RETURN    :: void : NULL
// **/
void MESSAGE_ShrinkPacket(Node *node,
                          Message *msg,
                          int size);

// /**
// API       :: MESSAGE_Free
// LAYER     :: ANY LAYER
// PURPOSE   :: When the message is no longer needed it
//              can be freed. Firstly the "payload" and "info" fields
//              of the message are freed. Then the message itself is freed.
//              It is important to remember to free the message. Otherwise
//              there will nasty memory leaks in the program.
// PARAMETERS ::
// + partition:  PartitionData*    : partition which is freeing the message
// + msg     :  Message* : message which has to be freed
// RETURN    :: void : NULL
// **/
void MESSAGE_Free(PartitionData *partition, Message *msg);

/*
 * FUNCTION     MESSAGE_FreeMT
 * PURPOSE      Multithread safe version of MESSAGE_Free
 *
 * Parameters:
 *    partition:  partition which is freeing the message
 *    msg:        message which has to be freed
 */
static
void MESSAGE_FreeMT(PartitionData *partition, Message *msg)
{
    msg->mtWasMT = true;
    MESSAGE_Free(partition, msg);
}

// /**
// API       :: MESSAGE_Free
// LAYER     :: ANY LAYER
// PURPOSE   :: When the message is no longer needed it
//              can be freed. Firstly the "payload" and "info" fields
//              of the message are freed. Then the message itself is freed.
//              It is important to remember to free the message. Otherwise
//              there will nasty memory leaks in the program.
// PARAMETERS ::
// + node    :  Node*    : node which is freeing the message
// + msg     :  Message* : message which has to be freed
// RETURN    :: void : NULL
// **/
void MESSAGE_Free (Node *node, Message *msg);


// /**
// API       :: MESSAGE_FreeList
// LAYER     :: ANY LAYER
// PURPOSE   :: Free a list of message until the next pointer of the
//              message is NULL.
// PARAMETERS ::
// + node    :  Node*    : node which is freeing the message
// + msg     :  Message* : message which has to be freed
// RETURN    :: void : NULL
// **/
void MESSAGE_FreeList(Node *node, Message *msg);

// /**
// API       :: MESSAGE_Duplicate
// LAYER     :: ANY LAYER
// PURPOSE   :: Create a new message which is an exact duplicate
//              of the message supplied as the parameter to the function and
//              return the new message.
// PARAMETERS ::
// + node    :  Node*    : node is calling message copy
// + msg     :  Message* : message for which duplicate has to be made
// RETURN    :: Message* : Pointer to the new message
// **/
Message* MESSAGE_Duplicate(Node *node, const Message *msg, bool isMT = false);

// /**
// API       :: MESSAGE_Duplicate
// LAYER     :: ANY LAYER
// PURPOSE   :: Create a new message which is an exact duplicate
//              of the message supplied as the parameter to the function and
//              return the new message.
// PARAMETERS ::
// + partition:  PartitionData*    : partition is calling message copy
// + msg     :  Message* : message for which duplicate has to be made
// + isMT   : bool : Is this function being called from the context
//                    of multiple threads
// RETURN    :: Message* : Pointer to the new message
// **/
Message* MESSAGE_Duplicate (PartitionData *partition, const Message *msg,
    bool isMT = false);

// /**
// API       :: MESSAGE_DuplicateMT
// LAYER     :: ANY LAYER
// PURPOSE   :: Create a new message which is an exact duplicate
//              of the message supplied as the parameter to the function and
//              return the new message.
// PARAMETERS ::
// + partition:  PartitionData*    : partition is calling message copy
// + msg     :  Message* : message for which duplicate has to be made
// RETURN    :: Message* : Pointer to the new message
// **/
static
Message* MESSAGE_DuplicateMT(PartitionData *partition, const Message *msg)
{
    return MESSAGE_Duplicate(partition, msg, true);
}

// /**
// API       :: MESSAGE_PayloadAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate a character payload out of the free list,
//              if possible otherwise via malloc.
// PARAMETERS ::
// + node    :  Node* : node which is allocating payload
// + payloadSize: int : size of the field to be allocated
// RETURN    :: char* : pointer to the allocated memory
// **/
char* MESSAGE_PayloadAlloc(Node *node, int payloadSize, bool isMT = false);

// /**
// API       :: MESSAGE_PayloadAlloc
// LAYER     :: ANY LAYER
// PURPOSE   :: Allocate a character payload out of the free list,
//              if possible otherwise via malloc.
// PARAMETERS ::
// + partition  :  PartitionData* : partition which is allocating payload
// + payloadSize: int : size of the field to be allocated
// + isMT    : bool   : Is this packet being created from a worker thread
// RETURN    :: char* : pointer to the allocated memory
// **/
char* MESSAGE_PayloadAlloc(PartitionData *partition, int payloadSize, bool isMT = false);

// /**
// API       :: MESSAGE_PayloadFree
// LAYER     :: ANY LAYER
// PURPOSE   :: Return a character payload to the free list,
//              if possible otherwise free it.
// PARAMETERS ::
// + partition:  PartitionData* : partition which is freeing payload
// + payload :  Char* : Pointer to the payload field
// + payloadSize: int : size of the payload field
// RETURN    :: void : NULL
// **/
void MESSAGE_PayloadFree(PartitionData *partition, char *payload, int payloadSize,
    bool wasMT);

// /**
// API       :: MESSAGE_PayloadFree
// LAYER     :: ANY LAYER
// PURPOSE   :: Return a character payload to the free list,
//              if possible otherwise free it.
// PARAMETERS ::
// + node    :  Node* : node which is freeing payload
// + payload :  Char* : Pointer to the payload field
// + payloadSize: int : size of the payload field
// RETURN    :: void : NULL
// **/
void MESSAGE_PayloadFree(Node *node, char *payload, int payloadSize,
                         bool wasMT = false);

// /*
// * FUNCTION     MESSAGE_PayloadFreeMT
// * PURPOSE      Multithread safe version of MESSAGE_PayloadFree ()
// *
// * Parameters:
// *    partition:    partition which is allocating payload
// *    payloadSize:  size of the "info" field to be allocated
// */
static
void MESSAGE_PayloadFreeMT(PartitionData* partition,
                           char* payload,
                           int payloadSize)
{
    MESSAGE_PayloadFree(partition, payload, payloadSize, true);
}

// /**
// API       :: MESSAGE_FreeList
// LAYER     :: ANY LAYER
// PURPOSE   :: Free a list of messages until the next pointer of the
//              message is NULL.
// PARAMETERS ::
// + node    :  Node*     : node which is freeing the message
// + msg     :  Message*  : message which has to be freed
// RETURN    :: void : NULL
// **/
void MESSAGE_FreeList(Node *node, Message *msg);

// /**
// API       :: MESSAGE_ReturnNumFrags
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the number of fragments used to create a TCP packet.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// RETURN    :: int : Number of Fragments.
//                      0 if none.
// **/
static int MESSAGE_ReturnNumFrags(const Message* msg)
{
    return (int)msg->infoBookKeeping.size();
}

// /**
// API       :: MESSAGE_ReturnFragSeqNum
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the sequence number of a particular fragments
//              in the TCP packet.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + fragmentNumber: int : fragment location in the TCP message.
// RETURN    :: int : Sequence number of the fragment.
//                      -1 if none.
// **/
static int MESSAGE_ReturnFragSeqNum (const Message* msg,
                                     unsigned int fragmentNumber)
{
    if (fragmentNumber >= 0 &&
        fragmentNumber < msg->infoBookKeeping.size())
    {
        return msg->infoBookKeeping.at(fragmentNumber).msgSeqNum;
    }
    return -1;
}

// /**
// API       :: MESSAGE_ReturnFragSize
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the size of a particular fragment
//              in the TCP packet.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + fragmentNumber: int : fragment location in the TCP message.
// RETURN    :: int : Sequence number of the fragment.
//                      0 if none.
// **/
static int MESSAGE_ReturnFragSize (const Message* msg,
                                   unsigned int fragmentNumber)
{
    if (fragmentNumber >= 0 &&
        fragmentNumber < msg->infoBookKeeping.size())
    {
        return msg->infoBookKeeping.at(fragmentNumber).fragSize;
    }
    return 0;
}

// /**
// API       :: MESSAGE_ReturnFragNumInfos
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the number of info fields associated with
//              a particular fragment in the TCP packet.
// PARAMETERS ::
// + msg     :  Message* : message for which "info" field
//                         has to be returned
// + fragmentNumber: int : fragment location in the TCP message.
// RETURN    :: int : Sequence number of the fragment.
//                      0 if none.
// **/
static int MESSAGE_ReturnFragNumInfos (const Message* msg,
                                       unsigned int fragmentNumber)
{
    if (fragmentNumber >= 0 &&
        fragmentNumber < msg->infoBookKeeping.size())
    {
        int numInfos = 0;
        numInfos = msg->infoBookKeeping.at(fragmentNumber).infoUpperLimit -
                   msg->infoBookKeeping.at(fragmentNumber).infoLowerLimit;
        return numInfos;
    }
    return 0;
}

// /**
// API       :: MESSAGE_AppendInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Appends the "info" fields of the source message to
//              the destination message.
// PARAMETERS::
// + partitionData : PartitionData* : Partition which is copying the info fields
// + msg  :  Message* : Destination message
// + infosize: int : size of the info field
// + infoType: short : type of info field.
// RETURN    :: void : NULL
// **/
char* MESSAGE_AppendInfo(PartitionData* partitionData,
                        Message *msg,
                        int infoSize,
                        unsigned short infoType);

// /**
// API       :: MESSAGE_AppendInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Appends the "info" fields of the source message to
//              the destination message.
// PARAMETERS::
// + node    :  Node*    : Node which is copying the info fields
// + msg  :  Message* : Destination message
// + infosize: int : size of the info field
// + infoType: short : type of info field.
// RETURN    :: void : NULL
// **/
char* MESSAGE_AppendInfo(Node* node,
                        Message *msg,
                        int infoSize,
                        unsigned short infoType);
// /**
// API       :: MESSAGE_AppendInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Appends the "info" fields of the source message to
//              the destination message.
// PARAMETERS::
// + node    :  Node*    : Node which is copying the info fields
// + dsgMsg  :  Message* : Destination message
// + srcInfo  :  MessageInfoHeader* : Source message info vector
// RETURN    :: void : NULL
// **/
void MESSAGE_AppendInfo(Node *node, Message *dstMsg, std::vector<MessageInfoHeader> srcInfo);

// /**
// API       :: MESSAGE_AppendInfo
// LAYER     :: ANY LAYER
// PURPOSE   :: Appends the "info" fields of the source message to
//              the destination message.
// PARAMETERS::
// + node    :  Node*    : Node which is copying the info fields
// + dsgMsg  :  Message* : Destination message
// + srcMsg  :  Message* : Source message
// RETURN    :: void : NULL
// **/
void MESSAGE_AppendInfo(Node *node, Message *dstMsg, Message* srcMsg);

// /**
// API       :: MESSAGE_SizeOf
// LAYER     :: ANY LAYER
// PURPOSE   :: Returns the size of a message.  Used in place of sizeof() in
//              the kernel code to allow for users to add more fields to
//              the message.
// PARAMETERS::
// RETURN    :: size_t : sizeof(msg)
// **/
size_t MESSAGE_SizeOf();

// /**
// API       :: MESSAGE_FragmentPacket
// LAYER     :: ANY LAYER
// PURPOSE   :: Fragment one packet into TWO fragments
//              Note:(i) This API treats the original packet as raw packet
//                       and does not take account of fragmentation related
//                       information like fragment id. The caller of this API
//                       will have to itself put in logic for distinguishing
//                       the fragmented packets
//                  (ii) Overloaded MESSAGE_FragmentPacket
// PARAMETERS::
// + node       :  Node* : node which is fragmenting the packet
// + msg        :  Message*& : The packet to be fragmented
// + fragmentedMsg:Message*& : First fragment
// + remainingMsg :Message*& : Remaining packet
// + fragUnit   :  int : The unit size for fragmenting the packet
// + protocolType : TraceProtocolType : Protocol type for packet tracing.
// + freeOriginalMsg: bool : If TRUE, then original msg is set to NULL
// RETURN    :: BOOL : TRUE if any fragment is created, FALSE otherwise
// **/

BOOL
MESSAGE_FragmentPacket(
    Node* node,
    Message*& msg,
    Message*& fragmentedMsg,
    Message*& remainingMsg,
    int fragUnit,
    TraceProtocolType protocolType,
    bool freeOriginalMsg);


// /**
// API       :: MESSAGE_ReassemblePacket
// LAYER     :: ANY LAYER
// PURPOSE   :: Reassemble TWO fragments into one packet
//              Note: (i) None of the fragments will be freed in this API.
//                        The caller of this API will itself have to free
//                        the fragments
//                   (ii) Overloaded MESSAGE_ReassemblePacket
// PARAMETERS::
// + node    : Node* : node which is assembling the packet
// + fragMsg1: Message*  : First fragment
// + fragMsg2: Message*  : Second fragment
// + protocolType : TraceProtocolType : Protocol type for packet tracing.
// RETURN    :: Message* : The reassembled packet.
// **/
Message*
MESSAGE_ReassemblePacket(
             Node* node,
             Message* fragMsg1,
             Message* fragMsg2,
             TraceProtocolType protocolType);

// /**
// API       :: MESSAGE_SendAsEarlyAsPossible
// LAYER     :: ANY LAYER
// PURPOSE   :: This function is used primarily by external interfaces to
//              inject events into the Simulator as soon as possible without
//              causing problems for parallel execution.
// PARAMETERS ::
// + node    :  Node*     : node which is sending message
// + msg     :  Message*  : message to be delivered
// RETURN    :: void : NULL
// **/
void MESSAGE_SendAsEarlyAsPossible(Node *node, Message *msg);

void MESSAGE_RemoteSendSafely(
    Node* node,
    NodeId destNodeId,
    Message*  msg,
    clocktype delay);

/*
 * FUNCTION     MESSAGE_FreeContents
 * PURPOSE      Free memory for the message contents: Info fields, payload,
 *              and so on.
 *
 * Parameters:
 *    partition:  partition which is freeing the message
 *    msg:        message which has to be freed
 */
void MESSAGE_FreeContents(PartitionData *partition, Message *msg);

/*
 * FUNCTION     MESSAGE_DebugSend
 * PURPOSE      Perform debugging and tracing activities for this message
 *              as it is sent
 *
 * Parameters:
 *    partition:  partition which is debugging
 *    node:       node which is debugging.  May be null.
 *    msg:        message to debug and trace
 */
void MESSAGE_DebugSend(PartitionData *partition, Node* node, Message* msg);

/*
 * FUNCTION     MESSAGE_DebugProcess
 * PURPOSE      Perform debugging and tracing activities for this message
 *              as it is processed
 *
 * Parameters:
 *    partition:  partition which is debugging
 *    node:       node which is debugging.  May be null.
 *    msg:        message to debug and trace
 */
void MESSAGE_DebugProcess(PartitionData *partition, Node* node, Message* msg);

#endif /* _MESSAGE_H_ */

