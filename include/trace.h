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
// PACKAGE     :: TRACE
// DESCRIPTION :: This file describes data structures and functions used for packet tracing.
// **/

#ifndef TRACE_H
#define TRACE_H

#include <stdio.h>


// /**
//  CONSTANT    :  MAX_TRACE_LENGTH  : (4090)
//  DESCRIPTION :: Buffer for an XML trace record.
// **/
//TBD Attempt to reduce this or eliminate it altogether
#define MAX_TRACE_LENGTH 4090


// /**
//  CONSTANT    :  TRACE_STRING_LENGTH  : 400
//  DESCRIPTION :: Generic maximum length of a string. The maximum
//                 length of any line in the input file is 3x this value.
// **/
#define TRACE_STRING_LENGTH 400


// /**
// ENUM        :: TraceDirectionType
// DESCRIPTION :: Different direction of packet tracing
// **/
enum TraceDirectionType
{
    TRACE_DIRECTION_INPUT,
    TRACE_DIRECTION_OUTPUT,
    TRACE_DIRECTION_BOTH
};


// /**
// ENUM        :: PacketActionType
// DESCRIPTION :: Different types of action on packet
// **/
enum PacketActionType
{
    SEND = 1,
    RECV,
    DROP,
    ENQUEUE,
    DEQUEUE
};


// /**
// ENUM        :: PacketDirection
// DESCRIPTION :: Direction of packet with respect to the node
// **/
enum PacketDirection
{
    PACKET_IN ,
    PACKET_OUT
};


// /**
// ENUM        :: TraceLayerType
// DESCRIPTION :: Keeps track of which layer is being traced.
// **/
enum TraceLayerType
{
    TRACE_APPLICATION_LAYER,
    TRACE_TRANSPORT_LAYER,
    TRACE_NETWORK_LAYER,
    TRACE_MAC_LAYER,
    TRACE_ALL_LAYERS
};


// /**
// ENUM        :: TraceIncludedHeadersType
// DESCRIPTION :: Specifies if included headers are output.
// **/
enum TraceIncludedHeadersType
{
    TRACE_INCLUDED_NONE,
    TRACE_INCLUDED_ALL,
    TRACE_INCLUDED_SELECTED
};


// /**
// ENUM        :: PacketActionCommentType
// DESCRIPTION :: Gives specific comments on the packet action here
//                packet drop.
// **/
enum PacketActionCommentType
{
   NO_COMMENT,
   DROP_QUEUE_OVERFLOW = 1,
   DROP_NO_ROUTE,
   DROP_LIFETIME_EXPIRY,
   DROP_TTL_ZERO,
   DROP_NO_CONNECTION,
   DROP_EXCEED_RETRANSMIT_COUNT,
   DROP_INTERFACE_DOWN,
   DROP_INVALID_STATE,
   DROP_DUPLICATE_PACKET,
   DROP_SELF_PACKET,
   DROP_OUT_OF_ORDER,
   DROP_EXCEEDS_METERING_UNIT,
   DROP_RANDOM_SIMULATION_DROP,
   DROP_BAD_PACKET,
   DROP_BUFFER_SIZE_EXCEED,
   DROP_NO_BUFFER_SIZE_SPECIFIED,
   DROP_LOCAL_REPAIR_FAILED,
   DROP_RREQ_FLOODED_RETRIES_TIMES,
   DROP_RETRANSMIT_TIMEOUT,
   DROP_OPTION_FIELD_MISMATCH,
   DROP_PROTOCOL_UNAVAILABLE_AT_INTERFACE,
   DROP_PACKET_AT_WRONG_INTERFACE,
   DROP_WRONG_SEQUENCE_NUMBER,
   DROP_LINK_BANDWIDTH_ZERO,
   DROP_ACCESS_LIST,
   DROP_START_FRAGMENT_OFFSET_NOT_ZERO,
   DROP_ALLFRAGMENT_NOT_COLLECTED,
   DROP_ICMP_NOT_ENABLE,
   DROP_IPFORWARD_NOT_ENABLE,
   DROP_MULTICAST_ADDR_SELF_PACKET,
   DROP_MAODV_OPTION_FIELD,
   DROP_START_FRAG_LENG_LESSTHAN_IPHEDEARTYPE,
   DROP_MORE_FRAG_IPLENG_LESSEQ_IPHEDEARTYPE,
   DROP_HOP_LIMIT_ZERO,
   DROP_INVALID_LINK_ADDRESS,
   DROP_DESTINATION_MISMATCH,
   DROP_UNREACHABLE_DEST,
   DROP_UNIDENTIFIED_SOURCE,
   DROP_UNKNOWN_MSG_TYPE,
   DROP_BAD_ICMP_LENGTH,
   DROP_BAD_ROUTER_SOLICITATION,
   DROP_BAD_NEIGHBOR_SOLICITATION,
   DROP_BAD_NEIGHBOR_ADVERTISEMENT,
   DROP_ICMP6_PACKET_TOO_BIG,
   DROP_TARGET_ADDR_MULTICAST,
   DROP_ICMP6_ERROR_OR_REDIRECT_MESSAGE,
   DROP_INVALID_NETWORK_PROTOCOL,
   DROP_EXCEED_NET_DIAMETER,
};


// /**
// ENUM        :: TraceProtocolType
// DESCRIPTION :: Enlisting all the possible traces
// **/
enum TraceProtocolType
{
    TRACE_UNDEFINED = 0,
    TRACE_TCP,                // 1
    TRACE_UDP,                // 2
    TRACE_IP,                 // 3
    TRACE_CBR,                // 4
    TRACE_FTP,                // 5
    TRACE_GEN_FTP,            // 6
    TRACE_BELLMANFORD,        // 7
    TRACE_BGP,                // 8
    TRACE_FISHEYE,            // 9
    TRACE_HTTP,               // 10
    TRACE_L16_CBR,            // 11
    TRACE_MCBR,               // 12
    TRACE_MPLS_LDP,           // 13
    TRACE_MPLS_SHIM,          // 14
    TRACE_TELNET,             // 15
    TRACE_MGEN,               // 16
    TRACE_NEIGHBOR_PROT,      // 17
    TRACE_OSPFv2,             // 18
    TRACE_LINK,               // 19
    TRACE_802_11,             // 20
    TRACE_CSMA,               // 21
    TRACE_DAWN,               // 22
    TRACE_FCSC_CSMA,          // 23
    TRACE_SPAWAR_LINK16,      // 24
    TRACE_MACA,               // 25
    TRACE_SATCOM,             // 26
    TRACE_SWITCHED_ETHERNET,  // 27
    TRACE_TDMA,               // 28
    TRACE_802_3,              // 29
    TRACE_ICMP,               // 30
    TRACE_GSM,                // 31
    TRACE_MOBILE_IP,          // 32
    TRACE_RSVP,               // 33
    TRACE_AODV,               // 34
    TRACE_DSR,                // 35
    TRACE_FSRL,               // 36
    TRACE_DVMRP,
    TRACE_IGMP,
    TRACE_LAR1,
    TRACE_MOSPF,
    TRACE_ODMRP,
    TRACE_PIM,
    TRACE_STAR,
    TRACE_IGRP,
    TRACE_EIGRP,
    TRACE_LOOKUP,
    TRACE_OLSR,
    TRACE_MY_APP,
    TRACE_MYROUTE,
    TRACE_SEAMLSS,
    TRACE_STP,
    TRACE_VLAN,
    TRACE_GVRP,
    TRACE_HSRP,
    TRACE_802_11a,
    TRACE_802_11b,
	TRACE_BAC_RTS,	  //YG
	TRACE_BAC_CTS,
	TRACE_Req_Unicast,
	TRACE_Req_Multicast,
	TRACE_Req_Broadcast,
	TRACE_BAC_toCluHead,
	TRACE_BAC_toCluMem,
	TRACE_BAC_reqCluHead,

    //GL
	TRACE_FAC_RTS,	 
	TRACE_FAC_CTS,
	TRACE_FAC,
	
	
    TRACE_TRAFFIC_GEN,
    TRACE_TRAFFIC_TRACE,
    TRACE_RTP,
    TRACE_H323,
    TRACE_SIP,
    TRACE_NDP,
    TRACE_APP_MGEN,
    TRACE_AAL5,
    TRACE_SAAL,
    TRACE_ATM_LAYER,
    TRACE_UTIL_EXTERNAL,
    TRACE_UTIL_ABSTRACT_EVENT,
    // ADDON_HELLO
    TRACE_STOCHASTIC,

    TRACE_MESSENGER,
    TRACE_IPV6,
    TRACE_ESP,
    TRACE_AH,

    TRACE_VBR,
    TRACE_ALOHA,
    TRACE_GENERICMAC,
    TRACE_BRP,
    TRACE_SUPERAPPLICATION,
    TRACE_RIP,
    TRACE_RIPNG,
    TRACE_IARP,
    TRACE_ZRP,
    TRACE_IERP,

    //InsertPatch TRACE_VAR
    TRACE_CELLULAR,
    TRACE_SATTSM,
    TRACE_SATTSM_SHIM,
    TRACE_HELLO,
    TRACE_ARP,
    TRACE_HLA,
    // TUTORIAL_INTERFACE
    TRACE_INTERFACETUTORIAL,

    // ALE_ASAPS_LIB
    TRACE_ALE,

    TRACE_RTCP,

    // ADDON_NETWARS
    TRACE_NETWARS,

    // CES
    TRACE_BOEINGFCS_PHY,
    TRACE_MAC_CES_USAP,
    TRACE_ROUTING_CES_MALSR,
    TRACE_ROUTING_CES_SRW,
    TRACE_ROUTING_CES_ROSPF,
    TRACE_NETWORK_CES_REGION,
    TRACE_NETWORK_CES_CLUSTER,
    TRACE_ROUTING_CES_MPR,
    TRACE_HSLS,
    TRACE_SOCKET_EXTERNAL,
    TRACE_MAC_CES_WINT_NCW,
    TRACE_MAC_CES_WINTHNW,
    TRACE_MAC_CES_WINT_DAMA,
    TRACE_MAC_CES_WINTGBS,
    TRACE_RPIM,
    TRACE_NETWORK_CES_INC_EPLRS,
    TRACE_NETWORK_CES_INC_SINCGARS,
    TRACE_ROUTING_CES_SDR,
    TRACE_BOEING_ODR,
    TRACE_BOEING_GENERICMAC,
    TRACE_MAC_CES_SINCGARS,
    TRACE_MAC_CES_SRW,
    TRACE_ROUTING_CES_MALSR_ALSU,
    TRACE_MULTICAST_CES_SRW_MOSPF,
    TRACE_MAC_CES_EPLRS,
    TRACE_CES_ISAKMP,
    TRACE_CES_SRW_PORT,
    TRACE_USAP,
    TRACE_USAP_SLOT,
    TRACE_USAP_CELL,

    TRACE_FORWARD,

    // SATELLITE_LIB
    TRACE_SATELLITE_RSV,
    TRACE_SATELLITE_BENTPIPE,

    TRACE_DOT11,

    // ADVANCED_WIRELESS_LIB
    TRACE_DOT16,
    TRACE_MAC_DOT16,
    TRACE_PHY_DOT16,

    // MILITARY_RADIOS_LIB
    TRACE_TADIL_LINK11,
    TRACE_TADIL_LINK16,
    TRACE_TADIL_TRANSPORT,
    TRACE_THREADEDAPP,

    // ADDON_MAODV
    TRACE_MAODV,

    TRACE_OSPFv3,

    TRACE_OLSRv2_NIIGATA,

    TRACE_NETWORK_CES_QOS,

    TRACE_DYMO,
    TRACE_ANE,
    TRACE_ICMPV6,
    TRACE_LLC,

    // Network Security
    TRACE_WORMHOLE,
    TRACE_ANODR,
    TRACE_SECURENEIGHBOR,
    TRACE_SECURECOMMUNITY,
    TRACE_MACDOT11_WEP,
    TRACE_MACDOT11_CCMP,
    TRACE_ISAKMP,
    TRACE_MDP,
    TRACE_MAC_802_15_4,
    TRACE_JAMMER,
    TRACE_DOS,

    // Military Radios
    TRACE_EPLRS,
    TRACE_ODR,
    TRACE_SDR,

    TRACE_NETWORK_NGC_HAIPE,

    // start cellular
    TRACE_CELLULAR_PHONE,
    TRACE_UMTS_LAYER3,
    TRACE_UMTS_LAYER2,
    TRACE_UMTS_PHY,

    TRACE_SOCKETLAYER,

    /* EXata Related -- START */
    // SNMP 
    TRACE_SNMP,
    TRACE_EFTP,
    TRACE_EHTTP,
    TRACE_ETELNET,
    TRACE_NETCONF_AGENT,
    /* EXata Related -- END */

    TRACE_MLS_IAHEP,

    // CES
    TRACE_MI_CES_FORWARDING,
    TRACE_NETWORK_CES_INC_EPLRS_MPR,
    TRACE_MI_CES_MULTICAST_MESH,
    TRACE_SNDCF,
    TRACE_VMF,
    TRACE_SRW_CNR_VOICE,

    // JNE
    TRACE_JNE_JWNM,
    TRACE_JNE_CONFIGURATION_AGENT,
    TRACE_JNE_MONITORING_AGENT, 
    TRACE_JNE_AUDIT_APPLICATION,
    TRACE_JNE_POSITION_REPORTING_AGENT,

    // JREAP
    TRACE_JREAP,
    TRACE_MODE5,

    // LTE
    TRACE_RRC_LTE,
    TRACE_PDCP_LTE,
    TRACE_RLC_LTE,
    TRACE_MAC_LTE,
    TRACE_PHY_LTE,
    TRACE_EPC_LTE,

    TRACE_AMSDU_SUB_HDR,
    TRACE_ZIGBEEAPP,
	 TRACE_HAC,               // 28
	 TRACE_HAC_RTS,
	 TRACE_HAC_CTS,
    // Must be last one!!!
    TRACE_ANY_PROTOCOL
};



// FUNCTION POINTER :: TracePrintXMLFn
// DESCRIPTION :: Protocol callback funtion to print trace.

typedef void (*TracePrintXMLFn)(Node* node, Message* message);


// FUNCTION POINTER :: TracePrintXMLFn
// DESCRIPTION :: Protocol callback funtion to print trace.

typedef void (*TracePrintXMLFun)(Node* node, Message* message ,NetworkType netType);




// /**
// STRUCT      :: TraceData
// DESCRIPTION :: Keeps track of which protocol is being traced.
// **/
struct TraceData
{
    BOOL traceList[TRACE_ANY_PROTOCOL];
    BOOL traceAll;
    TraceDirectionType direction;

    BOOL layer[TRACE_ALL_LAYERS];
    TraceIncludedHeadersType traceIncludedHeaders;

    char xmlBuf[MAX_TRACE_LENGTH];
    TracePrintXMLFn xmlPrintFn[TRACE_ANY_PROTOCOL];
    TracePrintXMLFun xmlPrintFun[TRACE_ANY_PROTOCOL];
};


// /**
// STRUCT      :: PktQueue
// DESCRIPTION :: Gives details of the packet queue
// **/
struct PktQueue
{
   unsigned short interfaceID;
   unsigned char  queuePriority;
};


// /**
// STRUCT      :: ActionData
// DESCRIPTION :: Keeps track of protocol action
// **/
struct ActionData
{
    PacketActionType   actionType;
    PacketActionCommentType actionComment;
    PktQueue pktQueue;
};


// /**
// FUNCTION   :: TRACE_Initialize
// PURPOSE    :: Initialize necessary trace information before
//               simulation starts.
// PARAMETERS ::
// + node      : Node*           : this node
// + nodeInput : const NodeInput*: access to configuration file
// RETURN     :: void            : NULL
// **/
void TRACE_Initialize(Node* node, const NodeInput* nodeInput);


// /**
// API        :: TRACE_IsTraceAll
// PURPOSE    :: Determine if TRACE-ALL is enabled from
//               configuration file.
// PARAMETERS ::
// + node      : Node* : this node
// RETURN     :: BOOL  : TRUE if TRACE-ALL is enabled, FALSE otherwise.
// **/
BOOL TRACE_IsTraceAll(Node* node);


// /**
// API        :: TRACE_PrintTrace
// PURPOSE    :: Print trace information to file.  To be used with Tracer.
// PARAMETERS ::
// + node      : Node*          : this node
// + message   : Message*       : Packet to print trace info from.
// + layerType : TraceLayerType : Layer that is calling this function.
// + pktDirection : PacketDirection : If the packet is coming out of
//                                    arriving  at a node.
// + actionData : ActionData* : more details about the packet action
// RETURN     :: void         : NULL
// **/
void TRACE_PrintTrace(Node* node,
                      Message* message,
                      TraceLayerType layerType,
                      PacketDirection pktDirection,
                      ActionData* actionData);

// /**
// API        :: TRACE_PrintTrace
// PURPOSE    :: Print trace information to file.  To be used with Tracer.
// PARAMETERS ::
// + node      : Node*          : this node
// + message   : Message*       : Packet to print trace info from.
// + layerType : TraceLayerType : Layer that is calling this function.
// + pktDirection : PacketDirection : If the packet is coming out of
//                                 arriving  at a node
// + actionData : ActionData* : more details about the packet action
// + netType : NetworkType : The network type.
// RETURN     :: void         : NULL
// **/
void TRACE_PrintTrace(Node* node,
                      Message* message,
                      TraceLayerType layerType,
                      PacketDirection pktDirection,
                      ActionData* actionData,
                      NetworkType netType);

// /**
// API        :: TRACE_EnableTraceXML
// PURPOSE    :: Enable XML trace for a particular protocol.
// PARAMETERS ::
// + node      : Node*             : this node
// + protocol  : TraceProtocolType : protocol to enable trace for
// + protocolName : char*          : name of protocol
// + xmlPrintFn : TracePrintXMLFn  : callback function
// + writeMap  : BOOL              : flag to print protocol ID map
// RETURN     :: void              : NULL
// **/
void TRACE_EnableTraceXMLFun(Node* node,
                          TraceProtocolType protocol,
                          const char* protocolName,
                          TracePrintXMLFun xmlPrintFun,
                          BOOL writeMap
                          );


// /**
// API        :: TRACE_EnableTraceXML
// PURPOSE    :: Enable XML trace for a particular protocol.
// PARAMETERS ::
// + node      : Node*             : this node
// + protocol  : TraceProtocolType : protocol to enable trace for
// + protocolName : char*          : name of protocol
// + xmlPrintFn : TracePrintXMLFn  : callback function
// + writeMap  : BOOL              : flag to print protocol ID map
// RETURN     :: void              : NULL
// **/
void TRACE_EnableTraceXML(Node* node,
                          TraceProtocolType protocol,
                          const char* protocolName,
                          TracePrintXMLFn xmlPrintFn,
                          BOOL writeMap);


// /**
// API        :: TRACE_DisableTraceXML
// PURPOSE    :: Disable XML trace for a particular protocol.
// PARAMETERS ::
// + node      : Node*             : this node
// + protocol  : TraceProtocolType : protocol to enable trace for
// + protocolName : char*          : name of protocol
// + writeMap  : BOOL              : flag to print protocol ID map
// RETURN     :: void              : NULL
// **/
void TRACE_DisableTraceXML(Node* node,
                           TraceProtocolType protocol,
                           const char* protocolName,
                           BOOL writeMap);


// /**
// API          :: TRACE_WriteToBufferXML
// PURPOSE      :: Write trace information to a buffer, which will then
//                 be printed to a file.
// PARAMETERS ::
// + node      : Node* : This node.
// + buf       : char* : Content to print to trace file.
// RETURN     :: void  : NULL
// **/
void TRACE_WriteToBufferXML(Node* node, char* buf);


// /**
// API          :: TRACE_WriteTraceHeader
// PURPOSE      :: Write trace header information to the partition's
//                 trace file
// PARAMETERS   ::
// + fp          : FILE* : pointer to the trace file.
// RETURN       :: void  : NULL
// **/
void TRACE_WriteXMLTraceHeader(NodeInput* nodeInput, FILE* fp);


// /**
// API          :: TRACE_WriteXMLTraceTail
// PURPOSE      :: Write trace tail information to the partition's
//                 trace file
// PARAMETERS   ::
// + fp          : FILE* : pointer to the trace file.
// RETURN       :: void  : NULL
// **/
void TRACE_WriteXMLTraceTail(FILE* fp);


#endif //TRACE_H
