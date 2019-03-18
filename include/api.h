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
// PACKAGE     :: API
// DESCRIPTION :: This file enumerates the basic message/events exchanged
// during the simulation process and the various layer functions (initialize, finalize, and event handling
// functions) and other miscellaneous routines and data structure definitions.
// **/

#ifndef API_H
#define API_H

#include "clock.h"
#include "qualnet_error.h"
#include "main.h"
#include "random.h"

#include "fileio.h"
#include "coordinates.h"
#include "splaytree.h"
#include "mapping.h"

#include "propagation.h"
#include "phy.h"
#include "network.h"
#include "mac.h"
#include "transport.h"
#include "application.h"
#include "user.h"

#include "message.h"

#include "gui.h"

#include "node.h"
#include "terrain.h"
#include "mobility.h"
#include "trace.h"

// /**
// ENUM        :: MESSAGE/EVENT
// DESCRIPTION :: Event/message types exchanged in the simulation
// **/
enum
{
    /* Special message types used for internal design. */
    MSG_SPECIAL_Timer                          = 0,

    /* Message Types for Environmental Effects/Weather */
    MSG_WEATHER_MobilityTimerExpired           = 50,

    /* Message Types for Channel layer */
    MSG_PROP_SignalArrival                     = 100,
    MSG_PROP_SignalEnd                         = 101,
    MSG_PROP_SignalReleased                    = 102,
    // Phy Connectivity Database
    MSG_PROP_SAMPLE_PATHLOSS                   = 103,
    MSG_PROP_SAMPLE_CONNECT                    = 104,

    // Stats Manager
    MSG_PROP_StatsManager_PhyCoverage          = 105,

    // end PHY Connectivity Database
    /* Message Types for Phy layer */
    MSG_PHY_TransmissionEnd                    = 200,
    MSG_PHY_CollisionWindowEnd,

    /* Message Types for MAC layer */
    MSG_MAC_UMTS_LAYER2_HandoffPacket          = 287,
    MSG_MAC_DOT11_Scan_Start_Timer             = 288,
    MSG_MAC_DOT11_Reassociation_Start_Timer    = 289,
    MSG_MAC_DOT11_Authentication_Start_Timer   = 290,
    MSG_MAC_DOT11_Beacon_Wait_Timer            = 291,
    MSG_MAC_DOT11_Probe_Delay_Timer            = 292,
    MSG_MAC_DOT11_Active_Scan_Long_Timer       = 293,
    MSG_MAC_DOT11_Active_Scan_Short_Timer      = 294,
    MSG_MAC_DOT11_Station_Inactive_Timer       = 295,
    MSG_MAC_DOT11_Management_Authentication    = 296,
    MSG_MAC_DOT11_Management_Association       = 297,
    MSG_MAC_DOT11_Management_Probe             = 298,
    MSG_MAC_DOT11_Management_Reassociation     = 299,
    MSG_MAC_FromPhy                            = 300,
    MSG_MAC_FromNetwork                        = 301,
    MSG_MAC_ReportChannelStatus                = 302,
    MSG_MAC_TransmissionFinished               = 303,
    MSG_MAC_TimerExpired                       = 304,
	MSG_CTS_TR											 =305,
    MSG_MAC_LinkToLink                         = 306,
    MSG_MAC_FrameStartOrEnd                    = 307,
    MSG_MAC_FromChannelForPacket               = 308,
    MSG_MAC_StartTransmission                  = 309,
    MSG_MAC_JamSequence                        = 310,
    MSG_MAC_StartFault                         = 311,
    MSG_MAC_EndFault                           = 312,
    MSG_MAC_FullDupToFullDup                   = 313,

    MSG_MAC_StartBGTraffic                     = 314,
    MSG_MAC_EndBGTraffic                       = 315,

    MSG_MAC_GroundToSatellite                  = 316,
    MSG_MAC_SatelliteToGround                  = 317,
    MSG_MAC_SwitchTick                         = 318,
    MSG_MAC_FromBackplane                      = 319,
    MSG_MAC_GarpTimerExpired                   = 320,

    MSG_MAC_802_11_Beacon                      = 321,
    MSG_MAC_802_11_CfpEnd                      = 322,

    MSG_MAC_SatTsm                             = 323,
    MSG_MAC_SatTsmShim                         = 324,
	MSG_MAC_Assignedslotinitial                            =325,		//YG
	MSG_MAC_Assignedslotagain									 =326,
	MSG_APP_INFORM_CHANNEL_REQUEST												   =327,	 //YG
	MSG_clear_convec                                              =328,		   //YG

    // Message Types of DOT11 MAC
    MSG_MAC_DOT11_Enable_Management_Timer      = 329,
    MSG_MAC_DOT11_Beacon                       = 330,
    MSG_MAC_DOT11_CfpBeacon                    = 331,
    MSG_MAC_DOT11_CfpEnd                       = 332,
    MSG_MAC_DOT11_Management                   = 333,

    // Power-Save-Mode-Updates
    MSG_BATTERY_TimerExpired                   = 334,
    MSG_MAC_DOT11_ATIMWindowTimerExpired       = 335,
    MSG_MAC_DOT11_ATIMPeriodExpired            = 336,
    MSG_MAC_DOT11_PSStartListenTxChannel       = 337,

    // TADIL events
    MSG_MAC_TADIL_FromAppSend                  = 338,
    MSG_MAC_TADIL_Timer                        = 339,
    MSG_APP_FromMac                            = 340,

    // ALE events
    MSG_MAC_AleChannelScan                     = 341,
    MSG_MAC_AleChannelCheck                    = 342,
    MSG_MAC_AleStartCall                       = 343,
    MSG_MAC_AleEndCall                         = 344,
    MSG_MAC_AleCallSetupTimer                  = 345,
    MSG_MAC_AleSlotTimer                       = 346,
    MSG_MAC_AleClearCall                       = 347,

    /* Message Types for Network layer */
    MSG_NETWORK_BrpDeleteQueryEntry            = 350,

     /* Message types for routing - Hsls */
    MSG_APP_CES_HSLS_Hello                     = 360,
    MSG_APP_CES_HSLS_T_P                       = 361,
    MSG_APP_CES_HSLS_T_E                       = 362,
    MSG_APP_CES_HSLS_CheckNeighbor             = 363,
    MSG_APP_MA_INTERNAL_ROUTING                = 364,

    MSG_NETWORK_FromApp                        = 400,
    MSG_NETWORK_FromMac                        = 401,
    MSG_NETWORK_FromTransportOrRoutingProtocol = 402,
    MSG_NETWORK_DelayedSendToMac               = 403,
    MSG_NETWORK_RTBroadcastAlarm               = 404,
    MSG_NETWORK_CheckTimeoutAlarm              = 405,
    MSG_NETWORK_TriggerUpdateAlarm             = 406,
    MSG_NETWORK_InitiateSend                   = 407,
    MSG_NETWORK_FlushTables                    = 408,
    MSG_NETWORK_CheckAcked                     = 409,
    MSG_NETWORK_CheckReplied                   = 410,
    MSG_NETWORK_DeleteRoute                    = 411,
    MSG_NETWORK_BlacklistTimeout               = 412,
    MSG_NETWORK_SendHello                      = 413,
    MSG_NETWORK_PacketTimeout                  = 414,
    MSG_NETWORK_RexmtTimeout                   = 415,
    MSG_NETWORK_PrintRoutingTable              = 416,
    MSG_NETWORK_Ip_Fragment                    = 417,

    // Military Radios
    MSG_EPLRS_DelayedSendToMac                 = 418,

    MSG_NETWORK_JoinGroup                      = 420,
    MSG_NETWORK_LeaveGroup                     = 421,
    MSG_NETWORK_SendData                       = 422,
    MSG_NETWORK_SendRequest                    = 423,
    MSG_NETWORK_SendReply                      = 424,
    MSG_NETWORK_CheckFg                        = 425,

    MSG_NETWORK_Retx                           = 430,

    MSG_NETWORK_PacketDropped                  = 440,
    MSG_NETWORK_CheckRouteTimeout              = 441,
    MSG_NETWORK_CheckNeighborTimeout           = 442,
    MSG_NETWORK_CheckCommunityTimeout          = 443,

    /* Messages Types special for IP */
    MSG_NETWORK_BuffTimerExpired               = 450,
    MSG_NETWORK_Backplane                      = 451,
    MSG_NETWORK_EmptyBroadcastMapping          = 452,

    /* Message Types for IPSec */
    MSG_NETWORK_IPsec                          = 455,

    /* Message Types for IAHEP */
    MSG_NETWORK_IAHEP                          = 458,

    MSG_NETWORK_IgmpData                       = 460,
    MSG_NETWORK_IgmpQueryTimer                 = 461,
    MSG_NETWORK_IgmpOtherQuerierPresentTimer   = 462,
    MSG_NETWORK_IgmpGroupReplyTimer            = 463,
    MSG_NETWORK_IgmpGroupMembershipTimer       = 464,
    MSG_NETWORK_IgmpJoinGroupTimer             = 465,
    MSG_NETWORK_IgmpLeaveGroupTimer            = 466,
    MSG_NETWORK_IgmpLastMemberTimer            = 467,

    /* Messages Types special for ICMP */
    MSG_NETWORK_IcmpEcho                       = 470,
    MSG_NETWORK_IcmpTimeStamp                  = 471,
    MSG_NETWORK_IcmpData                       = 472,
    MSG_NETWORK_IcmpRouterSolicitation         = 473,
    MSG_NETWORK_IcmpRouterAdvertisement        = 474,
    MSG_NETWORK_IcmpValidationTimer            = 475,
    MSG_NETWORK_IcmpRedirect                   = 476,
    MSG_NETWORK_IcmpRedirectRetryTimer         = 477,

    MSG_NETWORK_AccessList                     = 480,

    /* LANMAR routing timers */
    MSG_FsrlScheduleSPF                        = 489,
    MSG_FsrlNeighborTimeout                    = 490,
    MSG_FsrlIntraUpdate                        = 491,
    MSG_FsrlLMUpdate                           = 492,

    MSG_NETWORK_RegistrationRequest            = 493,
    MSG_NETWORK_RegistrationReply              = 494,
    MSG_NETWORK_MobileIpData                   = 495,
    MSG_NETWORK_RetransmissionRequired         = 496,
    MSG_NETWORK_AgentAdvertisementRefreshTimer = 497,
    MSG_NETWORK_VisitorListRefreshTimer        = 498,
    MSG_NETWORK_BindingListRefreshTimer        = 499,

    /* Message Types for Transport layer */
    MSG_TRANSPORT_FromNetwork                  = 500,
    MSG_TRANSPORT_FromAppListen                = 501,
    MSG_TRANSPORT_FromAppOpen                  = 502,
    MSG_TRANSPORT_FromAppSend                  = 503,
    MSG_TRANSPORT_FromAppClose                 = 504,
    MSG_TRANSPORT_TCP_TIMER_FAST               = 505,
    MSG_TRANSPORT_TCP_TIMER_SLOW               = 506,
    MSG_TRANSPORT_Tcp_CheckTcpOutputTimer      = 507,

    /* Message Types for Setting Timers */
    MSG_TRANSPORT_TCP_TIMER                    = 510,

    /* Messages Types for Transport layer with NS TCP */
    MSG_TCP_SetupConnection                    = 520,

    /* Messages Types for RSVP & RSVP/TE */
    MSG_TRANSPORT_RSVP_InitApp                 = 540,
    MSG_TRANSPORT_RSVP_PathRefresh             = 541,
    MSG_TRANSPORT_RSVP_HelloExtension          = 542,
    MSG_TRANSPORT_RSVP_InitiateExplicitRoute   = 543,
    MSG_TRANSPORT_RSVP_ResvRefresh             = 544,

    /* Message Types for Application layer */
    MSG_APP_FromTransListenResult              = 600,
    MSG_APP_FromTransOpenResult                = 601,
    MSG_APP_FromTransDataSent                  = 602,
    MSG_APP_FromTransDataReceived              = 603,
    MSG_APP_FromTransCloseResult               = 604,
    MSG_APP_TimerExpired                       = 605,
    MSG_APP_SessionStatus                      = 606,
    /* Messages Types for Application layer from UDP */
    MSG_APP_FromTransport                      = 610,

    /* Messages Types for Application layer from NS TCP */
    MSG_APP_NextPkt                            = 620,
    MSG_APP_SetupConnection                    = 621,

    /* Messages Type for Application layer directly from IP */
    MSG_APP_FromNetwork                        = 630,

    /* MGEN specific event */
    MSG_APP_DrecEvent                          = 645,


    /* Message Types for MPLS LDP */
    MSG_APP_MplsLdpKeepAliveTimerExpired       = 650,
    MSG_APP_MplsLdpSendKeepAliveDelayExpired   = 651,
    MSG_APP_MplsLdpSendHelloMsgTimerExpired    = 652,
    MSG_APP_MplsLdpLabelRequestTimerExpired    = 653,
    MSG_APP_MplsLdpFaultMessageDOWN            = 654,
    MSG_APP_MplsLdpFaultMessageUP              = 655,

    /* Message Types for Promiscuous Routing Algorithms */
    MSG_ROUTE_FromTransport                    = 700,
    MSG_ROUTE_FromNetwork                      = 701,

    /* Messages for FCS QoS */
    MSG_APP_UpdateTable                        = 702,
    MSG_APP_UpdateIngress                      = 703,
    MSG_APP_GotIngressPacket                   = 704,
    MSG_APP_RemoveFlow                         = 705,
    MSG_APP_RemarkIngress                      = 706,
    MSG_APP_NewSamplePeriod                    = 707,

    /* Message Types for Routing - BGP */
    MSG_APP_BGP_KeepAliveTimer                 = 710,
    MSG_APP_BGP_HoldTimer                      = 711,
    MSG_APP_BGP_ConnectRetryTimer              = 712,
    MSG_APP_BGP_StartTimer                     = 713,
    MSG_APP_BGP_RouteAdvertisementTimer        = 714,
    MSG_APP_BGP_OriginationTimer               = 715,

    /* Message Types for Routing - OSPF , Q-OSPF*/
    MSG_ROUTING_OspfScheduleHello              = 720,
    MSG_ROUTING_OspfIncrementLSAge             = 721,
    MSG_ROUTING_OspfScheduleLSDB               = 722,
    MSG_ROUTING_OspfPacket                     = 723,
    MSG_ROUTING_OspfRxmtTimer                  = 724,
    MSG_ROUTING_OspfInactivityTimer            = 725,
    MSG_ROUTING_QospfSetNewConnection          = 726,
    MSG_ROUTING_QospfScheduleLSDB              = 727,
    MSG_ROUTING_QospfInterfaceStatusMonitor    = 728,

    MSG_ROUTING_OspfWaitTimer                  = 729,
    MSG_ROUTING_OspfFloodTimer                 = 730,
    MSG_ROUTING_OspfDelayedAckTimer            = 731,
    MSG_ROUTING_OspfScheduleSPF                = 732,
    MSG_ROUTING_OspfNeighborEvent              = 733,
    MSG_ROUTING_OspfInterfaceEvent             = 734,
    MSG_ROUTING_OspfSchedASExternal            = 735,
    MSG_ROUTING_OspfMaxAgeRemovalTimer         = 736,

    /* Messages types for Multicast Routing - DVMRP */
    MSG_ROUTING_DvmrpScheduleProbe             = 740,
    MSG_ROUTING_DvmrpPeriodUpdateAlarm         = 741,
    MSG_ROUTING_DvmrpTriggeredUpdateAlarm      = 742,
    MSG_ROUTING_DvmrpNeighborTimeoutAlarm      = 743,
    MSG_ROUTING_DvmrpRouteTimeoutAlarm         = 744,
    MSG_ROUTING_DvmrpGarbageCollectionAlarm    = 745,
    MSG_ROUTING_DvmrpPruneTimeoutAlarm         = 746,
    MSG_ROUTING_DvmrpGraftRtmxtTimeOut         = 747,
    MSG_ROUTING_DvmrpPacket                    = 748,

    /* Message types for routing protocol IGRP     */
    MSG_ROUTING_IgrpBroadcastTimerExpired      = 750,
    MSG_ROUTING_IgrpPeriodicTimerExpired       = 751,
    MSG_ROUTING_IgrpHoldTimerExpired           = 752,
    MSG_ROUTING_IgrpTriggeredUpdateAlarm       = 753,

    /* Message types for routing protocol EIGRP    */
    MSG_ROUTING_EigrpHelloTimerExpired         = 760,
    MSG_ROUTING_EigrpHoldTimerExpired          = 761,
    MSG_ROUTING_EigrpRiseUpdateAlarm           = 762,
    MSG_ROUTING_EigrpRiseQueryAlarm            = 763,
    MSG_ROUTING_EigrpStuckInActiveRouteTimerExpired = 764,

    /* Message types for Bellmanford. */
    MSG_APP_PeriodicUpdateAlarm                = 770,
    MSG_APP_CheckRouteTimeoutAlarm             = 771,
    MSG_APP_TriggeredUpdateAlarm               = 772,


    /* Used for debugging purposes. */
    MSG_APP_PrintRoutingTable                  = 775,

    /* Message types for routing - Fisheye */
    MSG_APP_FisheyeNeighborTimeout             = 780,
    MSG_APP_FisheyeIntraUpdate                 = 781,
    MSG_APP_FisheyeInterUpdate                 = 782,

    /* Messages for HSRP */
    MSG_APP_HelloTimer                         = 785,
    MSG_APP_ActiveTimer                        = 786,
    MSG_APP_StandbyTimer                       = 787,

    /* Message types for OLSR */
    MSG_APP_OlsrPeriodicHello                  = 790,
    MSG_APP_OlsrPeriodicTc                     = 791,
    MSG_APP_OlsrNeighHoldTimer                 = 792,
    MSG_APP_OlsrTopologyHoldTimer              = 793,
    MSG_APP_OlsrDuplicateHoldTimer             = 794,
    MSG_APP_OlsrPeriodicMid                    = 795,
    MSG_APP_OlsrPeriodicHna                    = 796,
    MSG_APP_OlsrMidHoldTimer                   = 797,
    MSG_APP_OlsrHnaHoldTimer                   = 798,

    /* Message Types for Application Layer CBR */
    MSG_APP_CBR_NEXT_PKT                       = 800,

    /* Message Types for Netwars*/
    MSG_APP_NW_SELF_INTERRUPT                  = 815,
    MSG_APP_NW_IER                             = 816,

    // Message types for H.323
    MSG_APP_H323_Connect_Timer                 = 820,
    MSG_APP_H323_Call_Timeout                  = 821,

    // Message types for H.225Ras
    MSG_APP_H225_RAS_GRQ_PeriodicRefreshTimer  = 830,
    MSG_APP_H225_RAS_GatekeeperRequestTimer    = 831,
    MSG_APP_H225_RAS_RegistrationRequestTimer  = 832,
    MSG_APP_H225_SETUP_DELAY_Timer             = 833,

    // Message types for SIP
    MSG_APP_SipConnectionDelay                 = 840,
    MSG_APP_SipCallTimeOut                     = 841,

    //Message Type RTP Jitter Buffer
    MSG_APP_JitterNominalTimer                 = 850,
    MSG_APP_TalkspurtTimer                     = 851,

    MSG_APP_RTP                                = 855,
    MSG_RTP_TerminateSession                   = 856,
    MSG_RTP_InitiateNewSession                 = 857,
    MSG_RTP_SetJitterBuffFirstPacketAsTrue     = 858,

    /* Message types for Messenger App */
    MSG_APP_SendRequest                        = 860,
    MSG_APP_SendResult                         = 861,
    MSG_APP_SendNextPacket                     = 862,
    MSG_APP_ChannelIsIdle                      = 863,
    MSG_APP_ChannelIsBusy                      = 864,
    MSG_APP_ChannelInBackoff                   = 865,
	//MSG_APP_RequestChannel                      =866,  //YG
    MSG_APP_RequestChannel_Unicast                      =866,		 //yg
    MSG_APP_RequestChannel_Multicast                      =867,
    MSG_APP_RequestChannel_Broadcast                      =868,

    /* Message types for Multicast Routing - PIM-DM */
    MSG_ROUTING_PimScheduleHello                 = 870,
    MSG_ROUTING_PimDmNeighborTimeOut             = 871,
    MSG_ROUTING_PimDmPruneTimeoutAlarm           = 872,
    MSG_ROUTING_PimDmAssertTimeOut               = 873,
    MSG_ROUTING_PimDmDataTimeOut                 = 874,
    MSG_ROUTING_PimDmGraftRtmxtTimeOut           = 875,
    MSG_ROUTING_PimDmJoinTimeOut                 = 876,
    MSG_ROUTING_PimDmScheduleJoin                = 877,
    MSG_ROUTING_PimPacket                        = 878,

    /* Message types for PIM-SM. */
    MSG_ROUTING_PimSmScheduleHello             = 880,
    MSG_ROUTING_PimSmScheduleCandidateRP       = 881,
    MSG_ROUTING_PimSmCandidateRPTimeOut        = 882,
    MSG_ROUTING_PimSmBootstrapTimeOut          = 883,
    MSG_ROUTING_PimSmRegisterStopTimeOut       = 884,
    MSG_ROUTING_PimSmExpiryTimerTimeout        = 885,
    MSG_ROUTING_PimSmPrunePendingTimerTimeout  = 886,
    MSG_ROUTING_PimSmJoinTimerTimeout          = 887,
    MSG_ROUTING_PimSmOverrideTimerTimeout      = 888,
    MSG_ROUTING_PimSmAssertTimerTimeout        = 889,
    MSG_ROUTING_PimSmKeepAliveTimeOut          = 890,
    MSG_ROUTING_PimSmNeighborTimeOut           = 891,
    MSG_ROUTING_PimScheduleTriggeredHello      = 892,

    MSG_ROUTING_PimSmRouterGrpToRPTimeout      = 893,
    MSG_ROUTING_PimSmBSRGrpToRPTimeout         = 894,
    MSG_NETWORK_RedistributeData               = 895,

    /* Message Types for GSM */
    MSG_MAC_GsmSlotTimer                              = 900,
    MSG_MAC_GsmCellSelectionTimer                     = 901,
    MSG_MAC_GsmCellReSelectionTimer                   = 902,
    MSG_MAC_GsmRacchTimer                             = 903,
    MSG_MAC_GsmIdleSlotStartTimer                     = 904,
    MSG_MAC_GsmIdleSlotEndTimer                       = 905,
    MSG_MAC_GsmHandoverTimer                          = 906,
    MSG_MAC_GsmTimingAdvanceDelayTimer                = 907,
    MSG_NETWORK_GsmCallStartTimer                     = 910,
    MSG_NETWORK_GsmCallEndTimer                       = 911,
    MSG_NETWORK_GsmSendTrafficTimer                   = 912,
    MSG_NETWORK_GsmHandoverTimer                      = 913,

    // Call Control Timers
    MSG_NETWORK_GsmAlertingTimer_T301                 = 920,
    MSG_NETWORK_GsmCallPresentTimer_T303              = 921,
    MSG_NETWORK_GsmDisconnectTimer_T305               = 922,
    MSG_NETWORK_GsmReleaseTimer_T308                  = 923,
    MSG_NETWORK_GsmCallProceedingTimer_T310           = 924,
    MSG_NETWORK_GsmConnectTimer_T313                  = 925,
    MSG_NETWORK_GsmModifyTimer_T323                   = 926,
    MSG_NETWORK_GsmCmServiceAcceptTimer_UDT1          = 927,
    // Radio Resource Management Timers
    MSG_NETWORK_GsmImmediateAssignmentTimer_T3101     = 930,
    MSG_NETWORK_GsmHandoverTimer_T3103                = 931,
    MSG_NETWORK_GsmMsChannelReleaseTimer_T3110        = 932,
    MSG_NETWORK_GsmBsChannelReleaseTimer_T3111        = 933,
    MSG_NETWORK_GsmPagingTimer_T3113                  = 934,
    MSG_NETWORK_GsmChannelRequestTimer                = 935,

    // Mobility Management Timers
    MSG_NETWORK_GsmLocationUpdateRequestTimer_T3210   = 941,
    MSG_NETWORK_GsmLocationUpdateFailureTimer_T3211   = 942,
    MSG_NETWORK_GsmPeriodicLocationUpdateTimer_T3212  = 943,
    MSG_NETWORK_GsmCmServiceRequestTimer_T3230        = 944,
    MSG_NETWORK_GsmTimer_T3240                        = 945,
    MSG_NETWORK_GsmTimer_T3213                        = 946,
    MSG_NETWORK_GsmTimer_BSSACCH                      = 947,

    // Message type for IPv6 (reserved from 950 to 999)
    MSG_NETWORK_Icmp6_NeighSolicit            = 950,
    MSG_NETWORK_Icmp6_NeighAdv                = 951,
    MSG_NETWORK_Icmp6_RouterSolicit           = 952,
    MSG_NETWORK_Icmp6_RouterAdv               = 953,
    MSG_NETWORK_Icmp6_Redirect                = 954,
    MSG_NETWORK_Icmp6_Error                   = 955,
    // IPV6 EVENT TYPE
    MSG_NETWORK_Ipv6_InitEvent                = 956,
    MSG_NETWORK_Ipv6_Rdvt                     = 957,
    MSG_NETWORK_Ipv6_Ndadv6                   = 958,
    MSG_NETWORK_Ipv6_Ndp_Process              = 959,
    MSG_NETWORK_Ipv6_RetryNeighSolicit        = 960,
    MSG_NETWORK_Ipv6_Fragment                 = 961,
    MSG_NETWORK_Ipv6_RSol                     = 962,
    MSG_NETWORK_Ipv6_NdAdvt                   = 963,

    MSG_NETWORK_Ipv6_Update_Address           = 999,

    MSG_ATM_ADAPTATION_SaalEndTimer           = 1100,
    MSG_NETWORK_FromAdaptation                = 1101,

    MSG_MAC_DOT11s_HwmpActiveRouteTimeout      = 1110,
    MSG_MAC_DOT11s_HwmpRreqReplyTimeout        = 1111,
    MSG_MAC_DOT11s_HwmpSeenTableTimeout        = 1112,
    MSG_MAC_DOT11s_HwmpDeleteRouteTimeout      = 1113,
    MSG_MAC_DOT11s_HwmpBlacklistTimeout        = 1114,

    MSG_MAC_DOT11s_HwmpTbrEventTimeout         = 1120,
    MSG_MAC_DOT11s_HwmpTbrRannTimeout          = 1121,
    MSG_MAC_DOT11s_HwmpTbrMaintenanceTimeout   = 1122,
    MSG_MAC_DOT11s_HmwpTbrRrepTimeout          = 1123,

    MSG_MAC_DOT11s_AssocRetryTimeout           = 1130,
    MSG_MAC_DOT11s_AssocOpenTimeout            = 1131,
    MSG_MAC_DOT11s_AssocCancelTimeout          = 1132,
    MSG_MAC_DOT11s_LinkSetupTimer              = 1133,
    MSG_MAC_DOT11s_LinkStateTimer              = 1134,
    MSG_MAC_DOT11s_PathSelectionTimer          = 1135,
    MSG_MAC_DOT11s_InitCompleteTimeout         = 1136,
    MSG_MAC_DOT11s_MaintenanceTimer            = 1137,
    MSG_MAC_DOT11s_PannTimer                   = 1138,
    MSG_MAC_DOT11s_PannPropagationTimeout      = 1139,
    MSG_MAC_DOT11s_LinkStateFrameTimeout       = 1140,
    MSG_MAC_DOT11s_QueueAgingTimer             = 1141,

    // NDP
    NETWORK_NdpPeriodicHelloTimerExpired       = 1150,
    NETWORK_NdpHoldTimerExpired                = 1151,

    // ARP
    MSG_NETWORK_ArpTickTimer                   = 1160,
    MSG_NETWORK_ARPRetryTimer                  = 1161,

    // ALOHA
    MSG_MAC_CheckTransmit                      = 1170,

    // Generic  MAC
    MAC_GenTimerExpired                        = 1180,
    MAC_GenExpBackoff                          = 1181,

    // SuperApplication
    MSG_APP_TALK_TIME_OUT                      = 1190,

    // RIP
    MSG_APP_RIP_RegularUpdateAlarm             = 1200,
    MSG_APP_RIP_TriggeredUpdateAlarm           = 1201,
    MSG_APP_RIP_RouteTimerAlarm                = 1202,

    // RIPng
    MSG_APP_RIPNG_RegularUpdateAlarm           = 1210,
    MSG_APP_RIPNG_RouteTimerAlarm              = 1211,
    MSG_APP_RIPNG_TriggeredUpdateAlarm         = 1212,

    // IARP
    ROUTING_IarpBroadcastTimeExpired           = 1220,
    ROUTING_IarpRefraceTimeExpired             = 1221,

    // IERP
    ROUTING_IerpRouteRequestTimeExpired        = 1230,
    ROUTING_IerpFlushTimeOutRoutes             = 1231,

    /* Message Types for OPNET support */
    MSG_OPNET_SelfTimer                        = 1700,

    MSG_EXTERNAL_HLA_HierarchyMobility         = 1800,
    MSG_EXTERNAL_HLA_ChangeMaxTxPower          = 1801,
    MSG_EXTERNAL_HLA_AckTimeout                = 1802,
    MSG_EXTERNAL_HLA_CheckMetricUpdate         = 1803,
//HlaLink11Begin
    MSG_EXTERNAL_HLA_SendRtss                  = 1804,
//HlaLink11End
    MSG_EXTERNAL_HLA_StartMessengerForwarded   = 1805,
    MSG_EXTERNAL_HLA_SendRtssForwarded         = 1806,
    MSG_EXTERNAL_HLA_CompletedMessengerForwarded = 1807,

    MSG_EXTERNAL_DIS_HierarchyMobility         = 1900,
    MSG_EXTERNAL_DIS_ChangeMaxTxPower          = 1901,

    /* FCS data structure */
    MSG_BoeingFcsSetSlaveSlot,
    MSG_BoeingFcsDnsUpdate,
    MSG_BoeingFcsDnsFromHostUpdate,
    MSG_BoeingFcsHostRoutingUpdate,
    MSG_BoeingFcsHelloBroadcastTimeout,
    MSG_RoutingCesMalsrLsuBroadcastTimeout,
    MSG_RoutingCesSrwBroadcastTimeout,
    MSG_RoutingCesRospfLsuBroadcastTimeout,
    MSG_NetworkCesClusterHelloBroadcastTimeout,
    MSG_NetworkCesRegionRapHelloBroadcastTimeout,
    MSG_NetworkCesRegionRapPeriodicKeepaliveTimer,
    MSG_NetworkCesRegionTopologyActTimer,
    MSG_NetworkCesRegionUnblockDock,
    MSG_NetworkCesRegionRapReqBandwidthTimeout,
    MSG_NetworkCesRegionMalsrNbrReport,
    MSG_NetworkCesRegionRapInitRespTimer,
    MSG_NetworkCesRegionRapConfReqTimer,
    MSG_NetworkCesRegionRapConfRespTimer,
    // time synchronization timer event
    MSG_NETWORK_MDL_TimeSync_TimerExpire,
//MM-ends
    //MM-starts:neighbor monitor timer msgs
    MSG_NETWORK_CheckExpiredNbrs,
    MSG_NETWORK_MDL_NeighborReport,
//MM-ends
    MSG_EXTERNAL_SOCKET_INTERFACE_DelayedMessage,
    MSG_EXTERNAL_SOCKET_INTERFACE_MulticastGroup,
    MSG_RoutingCesMalsrSimulatedHelloTimeout,
    MSG_RoutingCesSrwToWnwIntfTimeout,
    MSG_RoutingCesSrwInitializeToWnwIntfTimeout,
    MSG_RoutingCesSrwUgsSleepTimeout,
    MSG_RoutingCesSrwUgsHibernateTimeout,
    MSG_RoutingCesSrwUgsC2WakeUpTimeout,
    MSG_RoutingCesSrwScheduleMospf,
    MSG_RoutingCesRospfDRElectionTimer,
    MSG_RoutingCesRospfDRResignationTimer,
    MSG_RoutingCesRospfAdjRecalcTimer,
    MSG_RoutingCesRospfDbdgRetransmitTimer,
    MSG_RoutingCesRospfDbdgFreeTimer,

    MSG_RoutingCesRospfRedHelloTimer,
    MSG_RoutingCesRospfRedHello,
    MSG_RoutingCesRospfRedAddP2PLink,
    MSG_RoutingCesRospfRedForceDR,
    MSG_RoutingCesRospfRedForceDRResignation,

    MSG_RoutingCesSdrLsuBroadcastTimeout,
    MSG_NetworkCesSincgarsNADTimerExpired,
    MSG_NetworkCesSincgarsNacTpTimerExpired,
    MSG_NetworkCesSincgarsNacType3AckTxTimer,
    MSG_NetworkCesSincgarsNacDapNadNewRoundTimer,
    MSG_NetworkCesSincgarsVoiceTimerExpired,
    MSG_NetworkCesSincgarsReXmitTimerExpired,
    MSG_NetworkCesSincgarsType3ReXmitTimerExpired,
    MSG_NetworkCesSincgarsNeighborAgentTimerExpired,
    MSG_NETWORK_CES_SINCGARS_FMU,
    MSG_NetworkCesSincgarsTimeSendSAUpFrame,
    MSG_NetworkCesSincgarsTimeSendSADownFrame,
    MSG_NetworkCesSincgarsXNPTimerExpired,

    MSG_NETWORK_CES_INC_EPLRS_DelayedSendToMac,
    MSG_NETWORK_CES_INC_EPLRS_BufferTimerExpired,
    MSG_NETWORK_CES_INC_EPLRS_TimerExpired,
    MSG_NETWORK_CES_INC_EPLRS_GlobalTimerExpired,
    MSG_NETWORK_CES_INC_EPLRS_CRM_delay,
    MSG_NETWORK_CES_INC_EPLRS_CircuitExpirationTimer,
    MSG_NETWORK_CES_INC_EPLRS_CircuitIdleTimer,
    MSG_NETWORK_CES_INC_EPLRS_CircuitTuWaitTimer,

    MSG_BoeingGenericMacSlotTimerExpired,
    MSG_MacCesSincgarsSlotTimerExpired,

    /* Message types for Multicast Routing - RPIM */
    MSG_ROUTING_RPimScheduleHello,
    MSG_ROUTING_RPimNeighborTimeOut,
    MSG_ROUTING_RPimPruneTimeoutAlarm,
    MSG_ROUTING_RPimAssertTimeOut,
    MSG_ROUTING_RPimDataTimeOut,
    MSG_ROUTING_RPimGraftRtmxtTimeOut,
    MSG_ROUTING_RPimJoinTimeOut,
    MSG_ROUTING_RPimScheduleJoin,
    MSG_ROUTING_RPimPacket,
    MSG_EXTERNAL_RemoteMessage, // A message sent from the remote partition

    MSG_MAC_ABSTRACT_TimerExpired,

    MSG_MAC_CES_SRW,
    MSG_MAC_CES_SRW_CmmaResponseTimer,
    MSG_MAC_CES_SRW_SlotTimerExpired,
    MSG_MAC_CES_SRW_CircuitDataFlowStopTimer,
    MSG_MAC_CES_SRW_ProcessPendingListTimer,
    MSG_MAC_CES_WINT_DAMA_EndOfRSA,
    MSG_MAC_CES_WINT_DAMA_DSAComputation,
    MSG_MAC_CES_WINT_DAMA_EndOfDSA,
    MSG_MAC_CES_USAP_MonitorPacket,
    MSG_MULTICAST_CES_FlushCache,
    MSG_MULTICAST_CES_RescheduleJoinOrLeave,
	MSG_RTS_TR,   //YG
	MSG_RTS_START,
	MSG_CACHE_TR,

    MSG_NETWORK_CES_INC_EPLRS_CES_FlushCache,

    MSG_MULTICAST_CES_SRW_MOSPF_ScheduleGroupMembershipLSA,

    MSG_EXTERNAL_SOCKET_INTERFACE_StatsLogTimer,
    MSG_EXTERNAL_SOCKET_INTERFACE_GraphLogTimer,
    MSG_MacCesEplrsSlotTimerExpired,

    // SRW_PORT
    MSG_SRW_CORBA_INTRANET_FROM_SNDCF_Bypass,
    MSG_SRW_CORBA_INTRANET_FROM_SNDCF_Multiplex,
    MSG_SRW_CORBA_INTRANET_FROM_SRD_Bypass,
    MSG_SRW_CORBA_INTRANET_FROM_SRD_Multiplex,
    MSG_SRW_CORBA_LL_FROM_INTRANET,
    MSG_SRW_CORBA_INTRANET_FROM_LL,
    MSG_SRW_CORBA_SNDCF_BYPASS,
    MSG_SRW_CORBA_SNDCF_DEMULTIPLEX,
    MSG_SRW_CORBA_Intranet,
    MSG_SRW_IntranetEventTimer,
    MSG_SRW_IntranetOSTimer,
    MSG_SRW_LLQueueTimer,
    MSG_SRW_CORBA_SRD_BYPASS,
    MSG_SRW_CORBA_SRD_DEMULTIPLEX,
    MSG_SRW_CORBA_SRD_FROM_SNDCF,
    MSG_SRW_CORBA_SNDCF_FROM_SRD,
    MSG_SRW_IGMP_QUERY_TO_IP,
    MSG_SRW_IGMP_HOST_GROUP_TIME_OUT,
    MSG_SRW_IGMP_REPORT_TO_ANET,
    MSG_SRW_IGMP_SELF_QUERY_TO_IP,
    MSG_SRW_IGMP_BROADCAST_TO_ANET,

    MSG_SRW_TEST_SNDCF_SEND_TO_SNDCF_BYPASS,
    MSG_SRW_TEST_SNDCF_SEND_TO_SNDCF_DEMULTIPLEX,

    MSG_SRW_SRD_TIMER,
    // ND/ATP timers
    MSG_SRW_ND_TimerExpired,
    MSG_SRW_MAC_SlotTimer,
    MSG_SRW_LLQueueEventTimer,

    // MA interface
    MSG_ROUTING_OspfStartTimeMA,

    MSG_MAC_CES_WINT_DAMA_SlotRequest,
    MSG_MAC_CES_WINT_DAMA_SlotReply,
    MSG_MAC_WINT_SatelliteToGround,

    MSG_PARALLEL_PROP_DELAY_Exchange,
    MSG_PARALLEL_PROP_InfoField,

    // SINCGARS events
    MSG_SdrLsuBroadcastTimeout,

    // NMS
    MSG_EXTERNAL_HistStatUpdate,
    MSG_NETWORK_NGC_HAIPE_SenderProcessingDelay,
    MSG_NETWORK_NGC_HAIPE_ReceiverProcessingDelay,

    MSG_EXTERNAL_SendPacket,
    MSG_EXTERNAL_ForwardInstantiate,
    MSG_EXTERNAL_ForwardTcpListen,
    MSG_EXTERNAL_ForwardTcpConnect,
    MSG_EXTERNAL_ForwardSendUdpData,
    MSG_EXTERNAL_ForwardBeginExternalTCPData,
    MSG_EXTERNAL_ForwardSendTcpData,
    MSG_EXTERNAL_Heartbeat,
    MSG_EXTERNAL_PhySetTxPower,

    MSG_EXTERNAL_DelayFunc,
    MSG_EXTERNAL_DelayFuncTrueEmul,
    MSG_EXTERNAL_DelayFuncTrueEmulPacket,
    MSG_NETWORK_DelayFunc,
    MSG_EXTERNAL_RecordStats,

    // Realtime Indicator
    MSG_EXTERNAL_RealtimeIndicator,

    // Messages used by the dynamic API
    MSG_DYNAMIC_Command,
    MSG_DYNAMIC_CommandOob,
    MSG_DYNAMIC_Response,
    MSG_DYNAMIC_ResponseOob,

    // Messages used by DXML interface
    MSG_EXTERNAL_DxmlCommand,

    // MAODV events
    MSG_ROUTING_MaodvFlushMessageCache,
    MSG_ROUTING_MaodvTreeUtilizationTimer,
    MSG_ROUTING_MaodvCheckMroute,
    MSG_ROUTING_MaodvCheckNextHopTimeout,
    MSG_ROUTING_MaodvRetransmitTimer,
    MSG_ROUTING_MaodvPruneTimeout,
    MSG_ROUTING_MaodvDeleteMroute,
    MSG_ROUTING_MaodvSendGroupHello,

    MSG_EXTERNAL_Mobility,

//startCellular
    //application
    MSG_APP_CELLULAR_FromNetworkCallAccepted,
    MSG_APP_CELLULAR_FromNetworkCallRejected,
    MSG_APP_CELLULAR_FromNetworkCallArrive,
    MSG_APP_CELLULAR_FromNetworkCallEndByRemote,
    MSG_APP_CELLULAR_FromNetworkCallDropped,
    MSG_APP_CELLULAR_FromNetworkPktArrive,

    //layer3
    MSG_NETWORK_CELLULAR_FromAppStartCall,
    MSG_NETWORK_CELLULAR_FromAppEndCall,
    MSG_NETWORK_CELLULAR_FromAppCallAnswered,
    MSG_NETWORK_CELLULAR_FromAppCallHandup,
    MSG_NETWORK_CELLULAR_FromAppPktArrival,
    MSG_NETWORK_CELLULAR_FromMacNetworkNotFound,
    MSG_NETWORK_CELLULAR_FromMacMeasurementReport,
    MSG_NETWORK_CELLULAR_TimerExpired,
    MSG_NETWORK_CELLULAR_PollHandoverForCallManagement,

    //Mac layer
    MSG_MAC_CELLULAR_FromNetworkScanSignalPerformMeasurement,
    MSG_MAC_CELLULAR_FromNetwork,
    MSG_MAC_CELLULAR_FromTch,
    MSG_MAC_CELLULAR_ScanSignalTimer,
    MSG_MAC_CELLULAR_FromNetworkCellSelected,
    MSG_MAC_CELLULAR_FromNetworkHandoverStart,
    MSG_MAC_CELLULAR_FromNetworkHandoverEnd,
    MSG_MAC_CELLULAR_FromNetworkTransactionActive,
    MSG_MAC_CELLULAR_FromNetworkNoTransaction,
    MSG_MAC_CELLULAR_ProcessTchMessages,

    //MISC
    MSG_CELLULAR_PowerOn,
    MSG_CELLULAR_PowerOff,

//endCellular

    //User Layer
    MSG_USER_StatusChange,
    MSG_USER_ApplicationArrival,
    MSG_USER_PhoneStartup,
    MSG_USER_TimerExpired,

    /* Message Types for Pedestrian Mobility */
    MSG_MOBILITY_PedestrianDynamicDataUpdate,
    MSG_MOBILITY_PedestrianPartitionDynamicDataUpdate,

    MSG_CONFIG_ChangeValueTimer,
    MSG_CONFIG_DisableNode,
    MSG_CONFIG_EnableNode,
    MSG_CONFIG_ContinueEnable,

    MSG_GenericMacSlotTimerExpired,

    MSG_UTIL_MemoryUtilization,
    MSG_UTIL_FsmEvent,
    MSG_UTIL_External,
    MSG_UTIL_AbstractEvent,
    MSG_UTIL_MobilitySample,
    MSG_UTIL_NameServiceDynamicParameterUpdated,

    /* Message types for OLSRv2 NIIGATA */
    MSG_APP_OLSRv2_NIIGATA_PeriodicHello,
    MSG_APP_OLSRv2_NIIGATA_PeriodicTc,
    MSG_APP_OLSRv2_NIIGATA_PeriodicMa,
    MSG_APP_OLSRv2_NIIGATA_TimeoutForward,
    MSG_APP_OLSRv2_NIIGATA_TimeoutTuples,

    //802.15.4 timers
    MSG_SSCS_802_15_4_TimerExpired,
    MSG_CSMA_802_15_4_TimerExpired,
    MSG_MAC_802_15_4_Frame,

    // Message types for Military Radios
    MSG_EPLRS_TimerExpired,
    MSG_EPLRS_GlobalTimerExpired,
    MSG_EPLRS_BufferTimerExpired,
    MSG_ROUTING_SdrScheduleSPF,

    // Network Security
    MSG_CRYPTO_Overhead,
    MSG_NETWORK_CheckRrepAck,
    MSG_NETWORK_CheckRerrAck,
    MSG_NETWORK_CheckDataAck,

    MSG_NETWORK_ISAKMP_SchedulePhase1,
    MSG_NETWORK_ISAKMP_ScheduleNextPhase,
    MSG_NETWORK_ISAKMP_SchedulePhase2,
    MSG_NETWORK_ISAKMP_RxmtTimer,
    MSG_NETWORK_ISAKMP_RefreshTimer,

    // STATS DB CODE
    MSG_NETWORK_InsertConnectivity,
    MSG_STATS_TRANSPORT_InsertConn,
    MSG_STATS_MAC_InsertConn,
    MSG_STATS_PHY_CONN_InsertConn,
    MSG_STATS_APP_InsertConn,
    MSG_PHY_CONN_CrossPartitionSize,
    MSG_PHY_CONN_CrossPartitionMessage,
    MSG_PHY_CONN_CrossPartitionInfoField,
    MSG_STATSDB_APP_InsertAggregate,
    MSG_STATS_QOS_InsertAggregate,
    MSG_STATS_TRANSPORT_InsertAggregate,
    MSG_STATS_NETWORK_InsertAggregate,
    MSG_STATS_QUEUE_InsertAggregate,
    MSG_STATS_MAC_InsertAggregate,
    MSG_STATS_PHY_InsertAggregate,
    MSG_STATSDB_APP_InsertSummary,
    MSG_STATS_QOS_InsertSummary,
    MSG_STATS_TRANSPORT_InsertSummary,
    MSG_STATS_NETWORK_InsertSummary,
    MSG_STATS_MAC_InsertSummary,
    MSG_STATS_PHY_InsertSummary,
    MSG_STATS_NODE_InsertStatus,
    MSG_STATS_INTERFACE_InsertStatus,
    MSG_STATS_QUEUE_InsertStatus,
    MSG_STATS_QUEUE_InsertSummary,
    MSG_STATS_OSPF_InsertNeighborState,
    MSG_STATS_OSPF_InsertRouterLsa,
    MSG_STATS_OSPF_InsertNetworkLsa,
    MSG_STATS_OSPF_InsertSummaryLsa,
    MSG_STATS_OSPF_InsertExternalLsa,
    MSG_STATS_OSPF_InsertInterfaceState,
    MSG_STATS_OSPF_InsertAggregate,
    MSG_STATS_OSPF_InsertSummary,
    MSG_STATS_MULTICAST_InsertStatus,
    MSG_STATS_MULTICAST_InsertSummary,
    MSG_STATS_PIM_SM_InsertStatus,
    MSG_STATS_PIM_SM_InsertSummary,
    MSG_STATS_PIM_DM_InsertSummary,
    MSG_STATS_MOSPF_InsertSummary,
    MSG_STATS_IGMP_InsertSummary,

    // Spectrum Manager
    MSG_SPECTRUM_RealTimeAnalysis,
    MSG_SPECTRUM_PredictiveTimeAnalysis,
    MSG_SPECTRUM_ChannelReAssign,

    //CES HAIPE
    MSG_NETWORK_CES_ISAKMP_SchedulePhase1,
    MSG_NETWORK_CES_ISAKMP_RxmtTimer,
    MSG_NETWORK_CES_ISAKMP_RefreshTimer,

    //DB protocol specific
    MSG_STATS_REGION_InsertStatusTable,
    MSG_STATS_RAP_InsertStatusTable,

    MSG_STATS_MALSR_InsertAggregateTable,
    MSG_STATS_MALSR_InsertConnTable,
    MSG_STATS_LINKADAPTATION_InsertStatusTable,
    MSG_STATS_USAP_ChannelizedNeighborhoodTable,
    MSG_STATS_SRW_IslandStatusTable,
    MSG_STATS_SRW_IntranetRoutingStatusTable,
    MSG_STATS_SRW_AdjacencyStatusTable,
    MSG_STATS_SRW_IntraNetConnectivityStatusTable,
    MSG_STATS_SRW_IntraNetNbrStatusTable,
    MSG_STATS_SRW_RFNbrStatusTable,
    MSG_STATS_SRW_CallGroupStatusTable,
//receive-only
    MSG_Receive_Only_Start,
    MSG_Receive_Only_Stop,
    MSG_Receive_Only_Black_To_Red_Start,
    MSG_Receive_Only_Black_To_Red_Stop,

    MSG_NETWORK_Ip_QueueAgingTimer,
    MSG_MAC_USAP_RBS_QueueAgingTimer,
    MSG_MAC_USAP_Unicast_QueueAgingTimer,

    // WNW
    MSG_NETWORK_CES_MI_QueueAgingTimer,
    MSG_WNW_MIMDL_RequestBandwidthTimer,
    MSG_WNW_MIMDL_SetNeighborhoodTimer,
    MSG_MAC_WNW_MDL_ReportBandwidthTimer,
    MSG_MAC_WNW_MDL_ReportTxStatusTimer,
    MSG_MAC_WNW_MDL_ReportLinkStatusTimer,
    MSG_MAC_WNW_MDL_ReportQueueStatusTimer,
    MSG_RoutingCesMalsr_Hsls_T_P,
    MSG_RoutingCesMalsr_Hsls_T_E,
    MSG_RoutingCesMalsrAlsuTimer,

    // Stats DB
    MSG_STATSDB_MULTICAST_APP_InsertSummary,
    MSG_STATSDB_TellSenderAboutGroupJoin,
    MSG_STATSDB_TellSenderAboutGroupLeave,
    MSG_STATS_MULTICAST_InsertConn,
    MSG_STATS_InsertWNWLinkUtilTablePerNode,
    MSG_STATS_InsertNCWLinkUtilTablePerNode,
    MSG_STATS_InsertWNWLinkUtilTable,
    MSG_STATS_InsertNCWLinkUtilTable,
    MSG_STATS_DB_MAC_Link_Utill_Frame_End,

    // CES
    MSG_NETWORK_CES_INC_EPLRS_MPR_SendHello,
    MSG_RoutingCesMalsrClearStaleEntries,
    MSG_RoutingCesMalsrEndPauseTimer,
    MSG_STATS_MPR_InsertFirstNeighborTable,
    MSG_STATS_MPR_InsertSecondNeighborTable,
    MSG_STATS_MDL_QueueStatusTable,
    MSG_STATS_MPR_InsertMprSetTable,
    MSG_MiCesMulticastMesh_RecalcPathToSrc,
    MSG_MiCesMulticastMesh_AckTimeout,
    MSG_MiCesMulticastMesh_ResignTimer,
    MSG_MiCesMulticastMesh_RemoveStaleNbrsTimer,
    MSG_MiCesMulticastMesh_CheckNumAPlusNbrsTimer,
    MSG_MiCesMulticastMesh_RefreshTimer,
    MSG_MiCesMulticastMesh_DataCacheAgingTimer,

    //SINCGARS sip timer for buffer
    MSG_MAC_SipBufferTimer,
    MSG_APP_C2ADAPTER_ExternalFrameIndication,

    // JREAP-C support
    MSG_MAC_TADIL_JREAP_JreRequestIndication,
    MSG_MAC_TADIL_JREAP_AvatarResponseIndication,
    MSG_EXTERNAL_TADIL_JREAP_EgressFrameIndication,

    // AGI events
    MSG_EXTERNAL_AgiUpdatePosition,

    // 802.11n Timers
    MSG_MAC_INPUT_BUFFER_Timer,
    MSG_MAC_AMSDU_BUFFER_Timer,
    MSG_MAC_BAA_Timer,
    MSG_MAC_IBSS_PROBE_Timer,

    //SNMP
    MSG_SNMP_Trap,
    MSG_SNMP_Community,
    MSG_SNMP_Manager,

    //SNMPv3
    MSG_SNMPV3_TRAP,
    //CW-starts: airborne DOC timer msgs
    MSG_NetworkCesRegionGtaKeepaliveTimer,
    MSG_NetworkCesRegionGtaMaintenanceTimer,
    MSG_NetworkCesRegionGtaTeardownTimer,
    MSG_NetworkCesRegionGtaRapSupTimer,
    MSG_NetworkCesRegionGtaVolSupTimer,

    MSG_SRW_CORBA_LL_SPECIAL,
    MSG_SRW_CNR_VOICE_PKT,

    // Start JNE Module
    // JNE Configuration Agent (CA)
    MSG_JNE_CA_STATE_MACHINE_EVENT,
    
    // JNE Monitoring Agent (MA)
    MSG_JNE_MA_TIMER,

    // JNE Position Report (PR) Agent
    MSG_APP_JNE_JWNM_PRAGENT_TIMER,

    // JNE Audit Application (Audit)
    MSG_JNE_AUDIT_APPLICATION_TIMER,
    // End JNE Modules

    //Firewall Msg
    MSG_Firewall_Rule,
    MSG_Gui_Hitl_Event,

    MSG_MAC_LTE_TtiTimerExpired, // TTI Timer
    MSG_MAC_LTE_RaBackoffWaitingTimerExpired, // t-RaBackoffWaiting
    MSG_MAC_LTE_RaGrantWaitingTimerExpired, // t-RaGrantWaiting
    MSG_RRC_LTE_WaitRrcConnectedTimerExpired, // t-WaitRrcConnected Timer
    MSG_RRC_LTE_WaitRrcConnectedReconfTimerExpired, // t-WaitRrcConnectedReconf Timer
    MSG_MAC_LTE_RELOCprep,
    MSG_MAC_LTE_X2_RELOCoverall,
    MSG_MAC_LTE_X2_WaitSnStatusTransfer,
    MSG_MAC_LTE_WaitAttachUeByHo,
    MSG_MAC_LTE_X2_WaitEndMarker,
    MSG_MAC_LTE_S1_WaitPathSwitchReqAck,    MSG_PHY_LTE_TransmissionEndInEstablishment,
    MSG_PHY_LTE_StartTransmittingSignalInEstablishment,
    MSG_PHY_LTE_NonServingCellMeasurementInterval,
    MSG_PHY_LTE_NonServingCellMeasurementPeriod,
    MSG_PHY_LTE_CheckingConnection,
    MSG_PHY_LTE_InterferenceMeasurementTimerExpired,
    MSG_RRC_LTE_WaitPowerOnTimerExpired,
    MSG_RRC_LTE_WaitPowerOffTimerExpired,
    MSG_RLC_LTE_PollRetransmitTimerExpired, // t-PollRetrunsmit
    MSG_RLC_LTE_ReorderingTimerExpired, // t-Reordering
    MSG_RLC_LTE_StatusProhibitTimerExpired, // t-StatusProhibit
    MSG_RLC_LTE_ResetTimerExpired, // reset timer
    MSG_PDCP_LTE_DelayedPdcpSdu,
    MSG_RRC_LTE_PeriodicalReportTimerExpired, // periodical report timer
    MSG_PDCP_LTE_DiscardTimerExpired,
    MSG_LTE_CheckingEnbTimer,
    MSG_PHY_MODE5_Emanation,
    MSG_APP_ZIGBEEAPP_NEXT_PKT,

	// xyt : for bac backoff
	MSG_MAC_BAC_backoffTimerExpired,

    /*
     * Any other message types which have to be added should be added before
     * MSG_DEFAULT. Otherwise the program will not work correctly.
     */
    MSG_DEFAULT                                = 10000
};


// --------------------------------------------------------------------------
// FUNCTION BLOCK    Layer_*Initialize
// PURPOSE           Initialization functions for various layers.
// --------------------------------------------------------------------------

// /**
// FUNCTION    :: CHANNEL_Initialize
// LAYER       :: PHYSICAL
// PURPOSE     :: Initialization function for channel
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing all the
// configuration file details
// RETURN      :: void :
// **/
void CHANNEL_Initialize(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: PHY_Init
// LAYER       :: PHYSICAL
// PURPOSE     :: Initialization function for physical layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing config file details
// RETURN      :: void :
// **/
void PHY_Init(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: MAC_Initialize
// LAYER       :: MAC
// PURPOSE     :: Initialization function for the MAC layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void MAC_Initialize(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: NETWORK_PreInit
// LAYER       :: NETWORK
// PURPOSE     :: Pre-Initialization function for Network layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void NETWORK_PreInit(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: NETWORK_Initialize
// LAYER       :: NETWORK
// PURPOSE     :: Initialization function for Network layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void NETWORK_Initialize(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: TRANSPORT_Initialize
// LAYER       :: TRANSPORT
// PURPOSE     :: Initialization function for transport layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void TRANSPORT_Initialize(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: APP_Initialize
// LAYER       :: APPLICATION
// PURPOSE     :: Initialization function for Application layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void APP_Initialize(Node *node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: USER_Initialize
// LAYER       :: USER
// PURPOSE     :: Initialization function for User layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void USER_Initialize(Node *node, const NodeInput *nodeInput);


// /**
// FUNCTION    :: APP_InitializeApplications
// LAYER       :: APPLICATION
// PURPOSE     :: Initialization function for applications in APPLICATION
// layer
// PARAMETERS  ::
// + firstNode : Node* : first node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void APP_InitializeApplications(Node* firstNode,
                                const NodeInput *nodeInput);

// /**
// FUNCTION: ATMLAYER2_Initialize
// PURPOSE:  Initialization function for the ATM Layer2.
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void ATMLAYER2_Initialize(Node* node, const NodeInput *nodeInput);

// /**
// FUNCTION    :: ADAPTATION_Initialize
// LAYER       :: ADAPTATION
// PURPOSE     :: Initialization function for Adaptation layer
// PARAMETERS  ::
// + node      : Node* : node being intialized
// + nodeInput : const NodeInput* : structure containing input file details
// RETURN      :: void :
// **/
void ADAPTATION_Initialize(Node *node, const NodeInput *nodeInput);


// --------------------------------------------------------------------------
// FUNCTION    Layer_*Finalize
// PURPOSE     Called at the end of simulation to collect the results of
//             the simulation of the various layers.
// --------------------------------------------------------------------------

// /**
// FUNCTION    :: CHANNEL_Finalize
// LAYER       :: PHYSICAL
// PURPOSE     :: To collect results of simulation at the end
// for channels
// PARAMETERS  ::
// + node : Node * : Node for which data is collected
// RETURN      :: void :
// **/
void CHANNEL_Finalize(Node *node);

// /**
// FUNCTION    :: PHY_Finalize
// LAYER       :: PHYSICAL
// PURPOSE     :: To collect results of simulation at the end
// for the PHYSICAL layer
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void PHY_Finalize(Node *node);

// /**
// FUNCTION    :: MAC_Finalize
// LAYER       :: MAC
// PURPOSE     :: To collect results of simulation at the end
// for the mac layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void MAC_Finalize(Node *node);

// /**
// FUNCTION    :: NETWORK_Finalize
// LAYER       :: NETWORK
// PURPOSE     :: To collect results of simulation at the end
// for network layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void NETWORK_Finalize(Node *node);

// /**
// FUNCTION    :: TRANSPORT_Finalize
// LAYER       :: TRANSPORT
// PURPOSE     :: To collect results of simulation at the end
// for transport layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void TRANSPORT_Finalize(Node *node);

// /**
// FUNCTION    :: APP_Finalize
// LAYER       :: APPLICATION
// PURPOSE     :: To collect results of simulation at the end
// for application layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void APP_Finalize(Node *node);

// /**
// FUNCTION    :: USER_Finalize
// LAYER       :: USER
// PURPOSE     :: To collect results of simulation at the end
// for user layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void USER_Finalize(Node *node);


// /**
// FUNCTION    :: ATMLAYER2_Finalize
// LAYER       :: Atm Layer2
// PURPOSE     :: To collect results at the end of the simulation.
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// RETURN      :: void :
// **/
void ATMLAYER2_Finalize(Node* node);

// /**
// FUNCTION    :: ADAPTATION_Finalize
// LAYER       :: ADAPTATION
// PURPOSE     :: To collect results of simulation at the end
// for network layers
// PARAMETERS  ::
// + node : Node * : Node for which finalization function is called
// and the statistical data being collected
// RETURN      :: void :
// **/
void ADAPTATION_Finalize(Node *node);


// --------------------------------------------------------------------------
// FUNCTION    Layer_*ProcessEvent
// PURPOSE     Models the behaviour of the various layers on receiving
//             message enclosed in msgHdr.
// --------------------------------------------------------------------------

// /**
// FUNCTION    :: CHANNEL_ProcessEvent
// LAYER       :: PHYSICAL
// PURPOSE     :: Processes the message/event of physical layer received
// by the node thus simulating the PHYSICAL layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void CHANNEL_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: PHY_ProcessEvent
// LAYER       :: PHYSICAL
// PURPOSE     :: Processes the message/event of physical layer received
// by the node thus simulating the PHYSICAL layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void PHY_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: MAC_ProcessEvent
// LAYER       :: MAC
// PURPOSE     :: Processes the message/event of MAC layer received
// by the node thus simulating the MAC layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void MAC_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: NETWORK_ProcessEvent
// LAYER       :: NETWORK
// PURPOSE     :: Processes the message/event received by the node
// thus simulating the NETWORK layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void NETWORK_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: TRANSPORT_ProcessEvent
// LAYER       :: TRANSPORT
// PURPOSE     :: Processes the message/event received by the node
// thus simulating the TRANSPORT layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void TRANSPORT_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: APP_ProcessEvent
// LAYER       :: APPLICATION
// PURPOSE     :: Processes the message/event received by the node
// thus simulating the APPLICATION layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void APP_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: USER_ProcessEvent
// LAYER       :: USER
// PURPOSE     :: Processes the message/event received by the node
// thus simulating the USER layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void USER_ProcessEvent(Node *node, Message *msg);


// /**
// FUNCTION    :: ATMLAYER2_ProcessEvent
// LAYER       :: ATM_LAYER2
// PURPOSE     :: Processes the message/event of ATM_LAYER2 layer received
// by the node thus simulating the ATM_LAYER2 layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void ATMLAYER2_ProcessEvent(Node *node, Message *msg);

// /**
// FUNCTION    :: ADAPTATION_ProcessEvent
// LAYER       :: ADAPTATION
// PURPOSE     :: Processes the message/event received by the node
// thus simulating the ADAPTATION layer behaviour
// PARAMETERS  ::
// + node : Node* : node which receives the message
// + msg  : Message* : Received message structure
// RETURN      :: void :
// **/
void ADAPTATION_ProcessEvent(Node *node, Message *msg);


// --------------------------------------------------------------------------
// FUNCTION    Layer_*RunTimeStat
// PURPOSE     Prints runtime statistics for the node.
// --------------------------------------------------------------------------

// /**
// FUNCTION    :: MAC_RunTimeStat
// LAYER       :: MAC
// PURPOSE     :: To print runtime statistics for the MAC layer
// PARAMETERS  ::
// + node : Node* : node for which statistics to be printed
// RETURN      :: void :
// **/
void MAC_RunTimeStat(Node *node);

// /**
// FUNCTION    :: NETWORK_RunTimeStat
// LAYER       :: NETWORK
// PURPOSE     :: To print runtime statistics for the NETWORK layer
// PARAMETERS  ::
// + node : Node* : node for which statistics to be printed
// RETURN      :: void :
// **/
void NETWORK_RunTimeStat(Node *node);

// /**
// FUNCTION    :: TRANSPORT_RunTimeStat
// LAYER       :: TRANSPORT
// PURPOSE     :: To print runtime statistics for the TRANSPORT layer
// PARAMETERS  ::
// + node : Node* : node for which statistics to be printed
// RETURN      :: void :
// **/
void TRANSPORT_RunTimeStat(Node *node);

// /**
// FUNCTION    :: APP_RunTimeStat
// LAYER       :: APPLICATION
// PURPOSE     :: To print runtime statistics for the APPLICATION layer
// PARAMETERS  ::
// + node : Node* : node for which statistics to be printed
// RETURN      :: void :
// **/
void APP_RunTimeStat(Node *node);


// /**
// STRUCT      :: PhyBatteryPower
// DESCRIPTION :: Used by App layer and Phy layer to exchange battery power
// **/
struct PhyBatteryPower
{
    short layerType;
    short protocolType;
    double power;
};


// /**
// STRUCT      :: PacketNetworkToApp
// DESCRIPTION :: Network to application layer packet structure
// **/
struct PacketNetworkToApp
{
    NodeAddress sourceAddr;    /* previous hop */
};


// /**
// Define      :: TTL_NOT_SET
// DESCRIPTION :: TTL value for which we consider TTL not set.
//                Used in TCP/UDP app and transport layer TTL
// **/
#define TTL_NOT_SET 0

// /**
// STRUCT      :: NetworkToTransportInfo
// DESCRIPTION :: Network To Transport layer Information structure
// **/
struct NetworkToTransportInfo
{
    Address sourceAddr;
    Address destinationAddr;
    TosType priority;
    int incomingInterfaceIndex;
    unsigned receivingTtl;
    BOOL isCongestionExperienced; /* ECN related variables */
};

// /**
// STRUCT      :: PacketTransportNetwork
// DESCRIPTION :: Transport to network layer packet structure
// **/
struct PacketTransportNetwork
{
    NodeAddress sourceId;
    NodeAddress destId;
    Int32 packetSize;
    Int32 agenttype;
    short sourcePort;
    short destPort;
    TosType priority;
    void *pkt;
};

// /**
// STRUCT      :: TcpTimerPacket
// DESCRIPTION :: TCP timer packet
// **/
struct TcpTimerPacket {
    int timerId;
    int timerType;
    int connectionId;
};

// /**
// STRUCT      :: AppToUdpSend
// DESCRIPTION :: Additional information given to UDP from applications.
// This information is saved in the info field of a message.
// **/
struct AppToUdpSend
{
    Address sourceAddr;
    short sourcePort;
    Address destAddr;
    short destPort;
    TosType priority;
    TosType origPriority;
    int outgoingInterface;
    UInt8 ttl;
};

struct AppDriveToUdpSend
{
	NodeAddress sourceNode;
	NodeAddress destNode;
	short		sourceport;
	int   originatingAppType;   //YG
	int   SequenceOfApp;
	bool isReTrans;
	bool firstTrans;

	//xyt
	int priority;

	 bool  operator==(const AppDriveToUdpSend b)
	 {
		 return(sourceNode==b.sourceNode&&destNode==b.destNode&&sourceport==b.sourceport
			 &&originatingAppType==b.originatingAppType);
	 }
	 friend bool operator< (const AppDriveToUdpSend a,const AppDriveToUdpSend b)  
	 {  
		 if (a.sourceNode != b.sourceNode)  
		 {  
			 return a.sourceNode < b.sourceNode;  
		 }  
		 if (a.originatingAppType!=b.originatingAppType)
		 {
			 return a.originatingAppType<b.originatingAppType;
		 }
		 if (a.sourceport != b.sourceport)  
		 {  
			 return a.sourceport < b.sourceport;  
		 }  
		 if (a.destNode != b.destNode)  
		 {  
			 return a.destNode < b.destNode;  
		 }  
		 return false; 	
	 }
	   AppDriveToUdpSend()
	   {
		  	 NodeAddress sourceNode=0;
			 NodeAddress destNode=0;
			 short		sourceport=0;
			 int   originatingAppType=0;   //YG
			 int   SequenceOfApp=0;
			 bool isReTrans=false;
			 bool firstTrans=true;
			 int priority = 0;
	   }

};


// /**
// STRUCT      :: ZigbeeAppInfo
// DESCRIPTION :: Structure used for zigbee GTS implementation
// **/
struct ZigbeeAppInfo
{
    TosType priority;
    clocktype endTime;
    D_Clocktype zigbeeAppInterval;
    UInt32 zigbeeAppPktSize;
    NodeAddress srcAddress;
    Int16 srcPort;
    UInt32 ipFragUnit;
};

// /**
// STRUCT      :: UdpToAppRecv
// DESCRIPTION :: Additional information given to applications from UDP.
// This information is saved in the info field of a message.
// **/
struct UdpToAppRecv
{
    Address sourceAddr;
    unsigned short sourcePort;
    Address destAddr;
    unsigned short destPort;
    int incomingInterfaceIndex;
    TosType priority;
};

// /**
// STRUCT      :: AppToRsvpSend
// DESCRIPTION :: send response structure from application layer
// **/
struct AppToRsvpSend {
    NodeAddress sourceAddr;
    NodeAddress destAddr;
    char*    upcallFunctionPtr;
};

// /**
// STRUCT      :: TransportToAppListenResult
// DESCRIPTION :: Report the result of application's listen request.
// **/
struct TransportToAppListenResult
{
    Address localAddr;
    short localPort;
    int connectionId;     /* -1 - listen failed, >=0 - connection id */
};

// /**
// STRUCT      :: TransportToAppOpenResult
// DESCRIPTION :: Report the result of opening a connection.
// **/
struct TransportToAppOpenResult
{
    int type;             /* 1 - active open, 2 - passive open */
    Address localAddr;
    short localPort;
    Address remoteAddr;
    short remotePort;
    int connectionId;     /* -1 - open failed, >=0 - connection id */

    Int32 uniqueId;
    Int32 clientUniqueId; // uniqueId of client application
};

// /**
// STRUCT      :: TransportToAppDataSent
// DESCRIPTION :: Report the result of sending application data.
// **/
struct TransportToAppDataSent
{
    int connectionId;
    Int32 length;               /* length of data sent */
};

// /**
// STRUCT      :: TransportToAppDataReceived
// DESCRIPTION :: Deliver data to application.
// **/
struct TransportToAppDataReceived
{
    int connectionId;
    TosType priority;
};

// /**
// STRUCT      :: TransportToAppCloseResult
// DESCRIPTION :: Report the result of closing a connection.
// **/
struct TransportToAppCloseResult
{
    int type;                /* 1 - active close, 2 - passive close */
    int connectionId;
};

// /**
// STRUCT      :: AppToTcpListen
// DESCRIPTION :: Application announces willingness to accept connections
// on given port.
// **/
struct AppToTcpListen
{
    AppType appType;
    Address localAddr;
    short localPort;
    TosType priority;
    int uniqueId;
};


// /**
// STRUCT      :: AppToTcpOpen
// DESCRIPTION :: Application attempts to establish a connection
// **/
struct AppToTcpOpen
{
    AppType appType;
    Address localAddr;
    short localPort;
    Address remoteAddr;
    short remotePort;

    Int32 uniqueId;
    TosType priority;
    int outgoingInterface;
};

// /**
// STRUCT      :: AppToTcpSend
// DESCRIPTION :: Application wants to send some data over the connection
// **/
struct AppToTcpSend
{
    int connectionId;
    UInt8 ttl;
};

// /**
// STRUCT      :: AppToTcpClose
// DESCRIPTION :: Application wants to release the connection
// **/
struct AppToTcpClose
{
    int connectionId;
};

// /**
// STRUCT      :: AppToTcpConnSetup
// DESCRIPTION :: Application sets up connection at the local end
// Needed for NS TCP to fake connection setup
// **/
struct AppToTcpConnSetup
{
    NodeAddress localAddr;
    int       localPort;
    NodeAddress remoteAddr;
    int       remotePort;
    int       connectionId;
    int       agentType;
};

// /**
// STRUCT      :: AppQosToNetworkSend
// DESCRIPTION :: Application uses this structure in its info field to
// perform the initialization of a new QoS connection with
// its QoS requirements.
// **/
struct AppQosToNetworkSend
{
    int   clientType;
    short sourcePort;
    short destPort;
    NodeAddress sourceAddress;
    NodeAddress destAddress;
    TosType priority;
    int bandwidthRequirement;
    int delayRequirement;
};

// /**
// STRUCT      :: NetworkToAppQosConnectionStatus
// DESCRIPTION :: Q-OSPF uses this structure to report status of a session
// requested by the application for Quality of Service.
// **/
struct NetworkToAppQosConnectionStatus
{
    short sourcePort;
    BOOL  isSessionAdmitted;
};


// STRUCT      :: TadilToAppRecv
// DESCRIPTION :: Additional information given to applications from TADIL.
//                This information is saved in the info field of a message.
//                Used in Link-16
// **/
struct TadilToAppRecv
{
    NodeAddress             sourceId;
    unsigned short          sourcePort;
    NodeAddress             dstId;
    unsigned short          destPort;
};

// /**
// ENUM        ::TransportType
// DESCRIPTION :: Transport type to check reliable, unreliable or
//                TADIL network for Link16 or Link11
// **/
enum TransportType {
    TRANSPORT_TYPE_RELIABLE     = 1,
    TRANSPORT_TYPE_UNRELIABLE   = 2,
    TRANSPORT_TYPE_MAC          = 3 // For LINK16 and LINK11
};

// /**
// ENUM        ::DestinationType
// DESCRIPTION :: Interface IP address type
// **/
enum DestinationType {
    DEST_TYPE_UNICAST   = 0,
    DEST_TYPE_MULTICAST = 1,
    DEST_TYPE_BROADCAST = 2
};

#endif /*API_H*/
