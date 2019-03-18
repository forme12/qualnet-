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

#ifndef _DB_STATSAPI_BRIDGE_H_
#define _DB_STATSAPI_BRIDGE_H_

#include <string>

#include "partition.h"
#include "stats_app.h"
#include "stats_net.h"
#include "stats_transport.h"
#include "stats_phy.h"

#include "stats_global.h"

// /**
// ENUM        :: AppAggregateTable Column Constants
// DESCRIPTION :: AppAggregateTable column constants values
// **/
enum
{
    // For AppAggregateTable
    APP_AGGREGATE_TABLE_TIME                      = 0,
    APP_AGGREGATE_TABLE_UNI_MSG_SENT              = 1,
    APP_AGGREGATE_TABLE_UNI_MSG_RCVD              = 2,
    APP_AGGREGATE_TABLE_MULTI_MSG_SENT            = 3,
    APP_AGGREGATE_TABLE_EFCTV_MULTI_MSG_SENT      = 4,
    APP_AGGREGATE_TABLE_MULTI_MSG_RCVD            = 5,
    APP_AGGREGATE_TABLE_UNI_BYTES_SENT            = 6,
    APP_AGGREGATE_TABLE_UNI_BYTES_RCVD            = 7,
    APP_AGGREGATE_TABLE_MULTI_BYTES_SENT          = 8,
    APP_AGGREGATE_TABLE_EFCTV_MULTI_BYTES_SENT    = 9,
    APP_AGGREGATE_TABLE_MULTI_MSG_BYTES_RCVD      = 10,
    APP_AGGREGATE_TABLE_UNI_FRAG_SENT             = 11,
    APP_AGGREGATE_TABLE_UNI_FRAG_RCVD             = 12,
    APP_AGGREGATE_TABLE_MULTI_FRAGSENT            = 13,
    APP_AGGREGATE_TABLE_EFCTV_MULTI_FRAG_SENT     = 14,
    APP_AGGREGATE_TABLE_MULTI_FRAG_RCVD           = 15,
    APP_AGGREGATE_TABLE_UNI_COMPLETION_RATE       = 16,
    APP_AGGREGATE_TABLE_MULTI_COMPLETION_RATE     = 17,
    APP_AGGREGATE_TABLE_UNI_OFFERLOAD             = 18,
    APP_AGGREGATE_TABLE_UNI_THROUGHPUT            = 19,
    APP_AGGREGATE_TABLE_MULTI_OFFERLOAD           = 20,
    APP_AGGREGATE_TABLE_MULTI_THROUGHPUT          = 21,
    APP_AGGREGATE_TABLE_UNI_DELAY                 = 22,
    APP_AGGREGATE_TABLE_MULTI_DELAY               = 23,
    APP_AGGREGATE_TABLE_UNI_HOPCOUNT              = 24,
    APP_AGGREGATE_TABLE_MULTI_HOPCOUNT            = 25,
    APP_AGGREGATE_TABLE_AVG_DELAY                 = 26,
    APP_AGGREGATE_TABLE_AVG_THROUGHPUT            = 27,
    APP_AGGREGATE_TABLE_AVG_OFFERLOAD             = 28,
    APP_AGGREGATE_TABLE_UNI_JITTER                = 29,
    APP_AGGREGATE_TABLE_MULTI_JITTER              = 30,
    APP_AGGREGATE_TABLE_AVG_JITTER                = 31,

    APP_AGGREGATE_TABLE_MAX_COLUMNS               = 32,
    APP_AGGREGATE_TABLE_MAX_FIXED_COLUMNS         = 22
};


//
// ENUM        :: AppSummaryTable Column Constants
// DESCRIPTION :: AppSummaryTable column constants values
//
enum
{
    // For AppSummaryTable
    APP_SUMMARY_TABLE_TIME                      = 0,
    APP_SUMMARY_TABLE_SENDER_ID                 = 1,
    APP_SUMMARY_TABLE_RECEIVER_ID               = 2,
    APP_SUMMARY_TABLE_RECEIVER_ADDR             = 3,
    APP_SUMMARY_TABLE_SESSION_ID                = 4,
    APP_SUMMARY_TABLE_TOS                       = 5,
    APP_SUMMARY_TABLE_MSG_SENT                  = 6,
    APP_SUMMARY_TABLE_EFCTV_MSG_SENT            = 7,
    APP_SUMMARY_TABLE_MSG_RCVD                  = 8,
    APP_SUMMARY_TABLE_BYTES_SENT                = 9,
    APP_SUMMARY_TABLE_EFCTV_BYTES_SENT          = 10,
    APP_SUMMARY_TABLE_BYTES_RCVD                = 11,
    APP_SUMMARY_TABLE_FRAG_SENT                 = 12,
    APP_SUMMARY_TABLE_EFCTV_FRAG_SENT           = 13,
    APP_SUMMARY_TABLE_FRAG_RCVD                 = 14,
    APP_SUMMARY_TABLE_APP_TYPE                  = 15,
    APP_SUMMARY_TABLE_APP_NAME                  = 16,
    APP_SUMMARY_TABLE_MSG_COMP_RATE             = 17,
    APP_SUMMARY_TABLE_OFFERLOAD                 = 18,
    APP_SUMMARY_TABLE_THROUGHPUT                = 19,
    APP_SUMMARY_TABLE_AVG_DELAY                 = 20,
    APP_SUMMARY_TABLE_AVG_HOPCOUNT              = 21,
    APP_SUMMARY_TABLE_AVG_JITTER                = 22,

    APP_SUMMARY_TABLE_MAX_COLUMNS               = 23,
    APP_SUMMARY_TABLE_MAX_FIXED_COLUMNS         = 20
};
//
// ENUM        :: MulticastAppSummaryTable Column Constants
// DESCRIPTION :: MulticastAppSummaryTable column constants values
enum
{
    // For MutlicastAppSummaryTable
    MULTI_APP_SUMMARY_TABLE_TIME                      = 0,
    MULTI_APP_SUMMARY_TABLE_SENDER_ID                 = 1,
    MULTI_APP_SUMMARY_TABLE_RECEIVER_ID               = 2,
    MULTI_APP_SUMMARY_TABLE_RECEIVER_ADDR             = 3,
    MULTI_APP_SUMMARY_TABLE_SESSION_ID                = 4,
    MULTI_APP_SUMMARY_TABLE_TOS                       = 5,
    MULTI_APP_SUMMARY_TABLE_MSG_SENT                  = 6,
    MULTI_APP_SUMMARY_TABLE_MSG_RCVD                  = 7,
    MULTI_APP_SUMMARY_TABLE_BYTES_SENT                = 8,
    MULTI_APP_SUMMARY_TABLE_BYTES_RCVD                = 9,
    MULTI_APP_SUMMARY_TABLE_FRAG_SENT                 = 10,
    MULTI_APP_SUMMARY_TABLE_FRAG_RCVD                 = 11,
    MULTI_APP_SUMMARY_TABLE_APP_TYPE                  = 12,
    MULTI_APP_SUMMARY_TABLE_APP_NAME                  = 13,
    MULTI_APP_SUMMARY_TABLE_MSG_COMP_RATE             = 14,
    MULTI_APP_SUMMARY_TABLE_OFFERLOAD                 = 15,
    MULTI_APP_SUMMARY_TABLE_THROUGHPUT                = 16,
    MULTI_APP_SUMMARY_TABLE_AVG_DELAY                 = 17,
    MULTI_APP_SUMMARY_TABLE_AVG_JITTER                = 18,
    MULTI_APP_SUMMARY_TABLE_AVG_HOPCOUNT              = 19,

    MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS               = 20,
    MULTI_APP_SUMMARY_TABLE_MAX_FIXED_COLUMNS         = 17
};

enum
{
    // For NetAggregateTable
    NET_AGGREGATE_TABLE_TIME                    = 0,
    NET_AGGREGATE_TABLE_UNI_OFFERLOAD           = 1,
    NET_AGGREGATE_TABLE_MULTI_OFFERLOAD         = 2,
    NET_AGGREGATE_TABLE_BROAD_OFFERLOAD         = 3,
    NET_AGGREGATE_TABLE_UNI_DATA_PKT_SENT       = 4,
    NET_AGGREGATE_TABLE_UNI_DATA_PKT_RCVD       = 5,
    NET_AGGREGATE_TABLE_UNI_DATA_PKT_FORW       = 6,
    NET_AGGREGATE_TABLE_MULTI_DATA_PKT_SENT     = 7,
    NET_AGGREGATE_TABLE_MULTI_DATA_PKT_RCVD     = 8,
    NET_AGGREGATE_TABLE_MULTI_DATA_PKT_FORW     = 9,
    NET_AGGREGATE_TABLE_BROAD_DATA_PKT_SENT     = 10,
    NET_AGGREGATE_TABLE_BROAD_DATA_PKT_RCVD     = 11,
    NET_AGGREGATE_TABLE_BROAD_DATA_PKT_FORW     = 12,
    NET_AGGREGATE_TABLE_UNI_CTRL_PKT_SENT       = 13,
    NET_AGGREGATE_TABLE_UNI_CTRL_PKT_RCVD       = 14,
    NET_AGGREGATE_TABLE_UNI_CTRL_PKT_FORW       = 15,
    NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_SENT     = 16,
    NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_RCVD     = 17,
    NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_FORW     = 18,
    NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_SENT     = 19,
    NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_RCVD     = 20,
    NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_FORW     = 21,
    NET_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT     = 22,
    NET_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD     = 23,
    NET_AGGREGATE_TABLE_UNI_DATA_BYTES_FORW     = 24,
    NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT   = 25,
    NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD   = 26,
    NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_FORW   = 27,
    NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT   = 28,
    NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD   = 29,
    NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_FORW   = 30,
    NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_SENT     = 31,
    NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_RCVD     = 32,
    NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_FORW     = 33,
    NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_SENT   = 34,
    NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_RCVD   = 35,
    NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_FORW   = 36,
    NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_SENT   = 37,
    NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_RCVD   = 38,
    NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_FORW   = 39,
    NET_AGGREGATE_TABLE_UNI_DELAY               = 40,
    NET_AGGREGATE_TABLE_MULTI_DELAY             = 41,
    NET_AGGREGATE_TABLE_BROAD_DELAY             = 42,

    NET_AGGREGATE_TABLE_UNI_IP_OUT_NO_ROUTE     = 43,
    NET_AGGREGATE_TABLE_MULTI_IP_OUT_NO_ROUTE   = 44,
    NET_AGGREGATE_TABLE_UNI_JITTER              = 45,
    NET_AGGREGATE_TABLE_MULTI_JITTER            = 46,
    NET_AGGREGATE_TABLE_BROAD_JITTER            = 47,

    NET_AGGREGATE_TABLE_MAX_COLUMNS             = 48,
    NET_AGGREGATE_TABLE_MAX_FIXED_COLUMNS       = 40
};

enum
{
    // For NetSummaryTable
    NET_SUMMARY_TABLE_TIME                      = 0,
    NET_SUMMARY_TABLE_SENDER_ADDR               = 1,
    NET_SUMMARY_TABLE_RECEIVER_ADDR             = 2,
    NET_SUMMARY_TABLE_DATA_PACKET_SENT          = 3,
    NET_SUMMARY_TABLE_DATA_PACKET_RCVD          = 4,
    NET_SUMMARY_TABLE_DATA_PACKET_FRWD          = 5,
    NET_SUMMARY_TABLE_CONTROL_PACKET_SENT       = 6,
    NET_SUMMARY_TABLE_CONTROL_PACKET_RCVD       = 7,
    NET_SUMMARY_TABLE_CONTROL_PACKET_FRWD       = 8,
    NET_SUMMARY_TABLE_DATA_BYTES_SENT           = 9,
    NET_SUMMARY_TABLE_DATA_BYTES_RCVD           = 10,
    NET_SUMMARY_TABLE_DATA_BYTES_FRWD           = 11,
    NET_SUMMARY_TABLE_CONTROL_BYTES_SENT        = 12,
    NET_SUMMARY_TABLE_CONTROL_BYTES_RCVD        = 13,
    NET_SUMMARY_TABLE_CONTROL_BYTES_FRWD        = 14,
    NET_SUMMARY_TABLE_DATA_DELAY                = 15,
    NET_SUMMARY_TABLE_CONTROL_DELAY             = 16,
    NET_SUMMARY_TABLE_DATA_JITTER               = 17,
    NET_SUMMARY_TABLE_CONTROL_JITTER            = 18,

    NET_SUMMARY_TABLE_MAX_COLUMNS               = 19,
    NET_SUMMARY_TABLE_MAX_FIXED_COLUMNS         = 15
};

enum
{
    // For TransSummaryTable
    TRANS_SUMMARY_TABLE_TIME                    = 0,
    TRANS_SUMMARY_TABLE_SENDER_ADDR             = 1,
    TRANS_SUMMARY_TABLE_RECEIVER_ADDR           = 2,
    TRANS_SUMMARY_TABLE_SENDER_PORT             = 3,
    TRANS_SUMMARY_TABLE_RECEIVER_PORT           = 4,
    TRANS_SUMMARY_TABLE_UNI_SEGMENTS_SENT       = 5,
    TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_SENT     = 6,
    TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_SENT     = 7,
    TRANS_SUMMARY_TABLE_UNI_SEGMENTS_RECV       = 8,
    TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_RECV     = 9,
    TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_RECV     = 10,
    TRANS_SUMMARY_TABLE_UNI_BYTES_SENT          = 11,
    TRANS_SUMMARY_TABLE_MULTI_BYTES_SENT        = 12,
    TRANS_SUMMARY_TABLE_BROAD_BYTES_SENT        = 13,
    TRANS_SUMMARY_TABLE_UNI_BYTES_RECV          = 14,
    TRANS_SUMMARY_TABLE_MULTI_BYTES_RECV        = 15,
    TRANS_SUMMARY_TABLE_BROAD_BYTES_RECV        = 16,
    TRANS_SUMMARY_TABLE_UNI_OFFERLOAD           = 17,
    TRANS_SUMMARY_TABLE_MULTI_OFFERLOAD         = 18,
    TRANS_SUMMARY_TABLE_BROAD_OFFERLOAD         = 19,
    TRANS_SUMMARY_TABLE_UNI_THROUGHPUT          = 20,
    TRANS_SUMMARY_TABLE_MULTI_THROUGHPUT        = 21,
    TRANS_SUMMARY_TABLE_BROAD_THROUGHPUT        = 22,
    TRANS_SUMMARY_TABLE_UNI_AVG_DELAY           = 23,
    TRANS_SUMMARY_TABLE_MULTI_AVG_DELAY         = 24,
    TRANS_SUMMARY_TABLE_BROAD_AVG_DELAY         = 25,
    TRANS_SUMMARY_TABLE_UNI_AVG_JITTER          = 26,
    TRANS_SUMMARY_TABLE_MULTI_AVG_JITTER        = 27,
    TRANS_SUMMARY_TABLE_BROAD_AVG_JITTER        = 28,

    TRANS_SUMMARY_TABLE_MAX_COLUMNS             = 29,
    TRANS_SUMMARY_TABLE_MAX_FIXED_COLUMNS       = 23
};

enum
{
    // For TransAggregateTable
    TRANS_AGGREGATE_TABLE_TIME                  = 0,
    TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_SENT     = 1,
    TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_SENT   = 2,
    TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_SENT   = 3,
    TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_RECV     = 4,
    TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_RECV   = 5,
    TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_RECV   = 6,
    TRANS_AGGREGATE_TABLE_UNI_BYTES_SENT        = 7,
    TRANS_AGGREGATE_TABLE_MULTI_BYTES_SENT      = 8,
    TRANS_AGGREGATE_TABLE_BROAD_BYTES_SENT      = 9,
    TRANS_AGGREGATE_TABLE_UNI_BYTES_RECV        = 10,
    TRANS_AGGREGATE_TABLE_MULTI_BYTES_RECV      = 11,
    TRANS_AGGREGATE_TABLE_BROAD_BYTES_RECV      = 12,
    TRANS_AGGREGATE_TABLE_UNI_OFFERLOAD         = 13,
    TRANS_AGGREGATE_TABLE_MULTI_OFFERLOAD       = 14,
    TRANS_AGGREGATE_TABLE_BROAD_OFFERLOAD       = 15,
    TRANS_AGGREGATE_TABLE_UNI_THROUGHPUT        = 16,
    TRANS_AGGREGATE_TABLE_MULTI_THROUGHPUT      = 17,
    TRANS_AGGREGATE_TABLE_BROAD_THROUGHPUT      = 18,
    TRANS_AGGREGATE_TABLE_UNI_AVG_DELAY         = 19,
    TRANS_AGGREGATE_TABLE_MULTI_AVG_DELAY       = 20,
    TRANS_AGGREGATE_TABLE_BROAD_AVG_DELAY       = 21,
    TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER        = 22,
    TRANS_AGGREGATE_TABLE_MULTI_AVG_JITTER      = 23,
    TRANS_AGGREGATE_TABLE_BROAD_AVG_JITTER      = 24,


    TRANS_AGGREGATE_TABLE_MAX_COLUMNS           = 25,
    TRANS_AGGREGATE_TABLE_MAX_FIXED_COLUMNS     = 19
};

enum
{
    // For PhySummaryTable
    PHY_SUMMARY_TABLE_TIME                    = 0,
    PHY_SUMMARY_TABLE_SENDER_ID               = 1,
    PHY_SUMMARY_TABLE_RECEIVER_ID             = 2,
    PHY_SUMMARY_TABLE_CHANNEL_INDEX           = 3,
    PHY_SUMMARY_TABLE_PHY_INDEX               = 4,
    PHY_SUMMARY_TABLE_NUM_RECV_SIGS           = 5,
    PHY_SUMMARY_TABLE_NUM_RECV_ERR_SIGS       = 6,
    PHY_SUMMARY_TABLE_UTILIZATION             = 7,
    PHY_SUMMARY_TABLE_AVG_INTERFERENCE        = 8,
    PHY_SUMMARY_TABLE_PATHLOSS                = 9,
    PHY_SUMMARY_TABLE_DELAY                   = 10,
    PHY_SUMMARY_TABLE_SIG_POWER               = 11,

    PHY_SUMMARY_TABLE_MAX_COLUMNS             = 12,
    PHY_SUMMARY_TABLE_MAX_FIXED_COLUMNS       = 9
};

enum
{
    // For PhyAggregateTable
    PHY_AGGREGATE_TABLE_TIME                    = 0,
    PHY_AGGREGATE_TABLE_SIGS_TRANS              = 1,
    PHY_AGGREGATE_TABLE_SIGS_LOCKED             = 2,
    PHY_AGGREGATE_TABLE_SIGS_RECV               = 3,
    PHY_AGGREGATE_TABLE_SIGS_DROPPED            = 4,
    PHY_AGGREGATE_TABLE_SIGS_DROPPED_INTERFERENCE   = 5,
    PHY_AGGREGATE_TABLE_UTILIZATION             = 6,
    PHY_AGGREGATE_TABLE_AVG_INTERFERENCE        = 7,
    PHY_AGGREGATE_TABLE_PATHLOSS                = 8,
    PHY_AGGREGATE_TABLE_DELAY                   = 9,
    PHY_AGGREGATE_TABLE_SIG_POWER               = 10,

    PHY_AGGREGATE_TABLE_MAX_COLUMNS             = 11,
    PHY_AGGREGATE_TABLE_MAX_FIXED_COLUMNS       = 8
};


enum
{
    // For MacAggregateTable
    MAC_AGGREGATE_TABLE_TIME                      = 0,
    MAC_AGGREGATE_TABLE_CARRIED_LOAD              = 1,
    MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_SENT      = 2,
    MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_RCVD      = 3,
    MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_SENT   = 4,
    MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_RCVD   = 5,
    MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT       = 6,
    MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD       = 7,
    MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_SENT    = 8,
    MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_RCVD    = 9,
    MAC_AGGREGATE_TABLE_UNI_AVG_QUEUING_DELAY     = 10,
    MAC_AGGREGATE_TABLE_UNI_AVG_MED_ACCESS_DELAY  = 11,
    MAC_AGGREGATE_TABLE_UNI_AVG_MED_DELAY         = 12,
    MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_SENT    = 13,
    MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_RCVD    = 14,
    MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_SENT = 15,
    MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_RCVD = 16,
    MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT     = 17,
    MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD     = 18,
    MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_SENT  = 19,
    MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_RCVD  = 20,
    MAC_AGGREGATE_TABLE_BROAD_AVG_QUEUING_DELAY   = 21,
    MAC_AGGREGATE_TABLE_BROAD_AVG_MED_ACCESS_DELAY  = 22,
    MAC_AGGREGATE_TABLE_BROAD_AVG_MED_DELAY       = 23,
    MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_SENT    = 24,
    MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_RCVD    = 25,
    MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_SENT = 26,
    MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_RCVD = 27,
    MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT     = 28,
    MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD     = 29,
    MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_SENT  = 30,
    MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_RCVD  = 31,
    MAC_AGGREGATE_TABLE_MULTI_AVG_QUEUING_DELAY   = 32,
    MAC_AGGREGATE_TABLE_MULTI_AVG_MED_ACCESS_DELAY  = 33,
    MAC_AGGREGATE_TABLE_MULTI_AVG_MED_DELAY       = 34,

    MAC_AGGREGATE_TABLE_UNI_AVG_JITTER            = 35,
    MAC_AGGREGATE_TABLE_BROAD_AVG_JITTER          = 36,
    MAC_AGGREGATE_TABLE_MULTI_AVG_JITTER          = 37,
    MAC_AGGREGATE_TABLE_MAX_COLUMNS               = 38,
    MAC_AGGREGATE_TABLE_MAX_FIXED_COLUMNS         = 26
};

enum
{
    // For MacSummaryTable
    MAC_SUMMARY_TABLE_TIME                      = 0,
    MAC_SUMMARY_TABLE_SENDER_ID                 = 1,
    MAC_SUMMARY_TABLE_RECEIVER_ID               = 2,
    MAC_SUMMARY_TABLE_INTERFACE_INDEX           = 3,
    MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_SENT      = 4,
    MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_RCVD      = 5,
    MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_SENT   = 6,
    MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_RCVD   = 7,
    MAC_SUMMARY_TABLE_UNI_DATA_BYTES_SENT       = 8,
    MAC_SUMMARY_TABLE_UNI_DATA_BYTES_RCVD       = 9,
    MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_SENT    = 10,
    MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_RCVD    = 11,
    MAC_SUMMARY_TABLE_UNI_AVG_QUEUING_DELAY     = 12,
    MAC_SUMMARY_TABLE_UNI_AVG_MED_ACCESS_DELAY  = 13,
    MAC_SUMMARY_TABLE_UNI_AVG_MED_DELAY         = 14,
    MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_SENT    = 15,
    MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_RCVD    = 16,
    MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_SENT = 17,
    MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_RCVD = 18,
    MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_SENT     = 19,
    MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_RCVD     = 20,
    MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_SENT  = 21,
    MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_RCVD  = 22,
    MAC_SUMMARY_TABLE_BROAD_AVG_QUEUING_DELAY   = 23,
    MAC_SUMMARY_TABLE_BROAD_AVG_MED_ACCESS_DELAY = 24,
    MAC_SUMMARY_TABLE_BROAD_AVG_MED_DELAY       = 25,
    MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_SENT    = 26,
    MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_RCVD    = 27,
    MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_SENT = 28,
    MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_RCVD = 29,
    MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_SENT     = 30,
    MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_RCVD     = 31,
    MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_SENT  = 32,
    MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_RCVD  = 33,
    MAC_SUMMARY_TABLE_MULTI_AVG_QUEUING_DELAY   = 34,
    MAC_SUMMARY_TABLE_MULTI_AVG_MED_ACCESS_DELAY  = 35,
    MAC_SUMMARY_TABLE_MULTI_AVG_MED_DELAY       = 36,
    MAC_SUMMARY_TABLE_FRAMES_DROP_SENDER        = 37,
    MAC_SUMMARY_TABLE_FRAMES_DROP_RECEIVER      = 38,
    MAC_SUMMARY_TABLE_BYTES_DROP_SENDER         = 39,
    MAC_SUMMARY_TABLE_BYTES_DROP_RECEIVER       = 40,

    MAC_SUMMARY_TABLE_UNI_AVG_JITTER            = 41,
    MAC_SUMMARY_TABLE_BROAD_AVG_JITTER          = 42,
    MAC_SUMMARY_TABLE_MULTI_AVG_JITTER          = 43,
    MAC_SUMMARY_TABLE_MAX_COLUMNS               = 44,
    MAC_SUMMARY_TABLE_MAX_FIXED_COLUMNS         = 32
};

// Bridge Base Class for all STATS DB TABLES
class DB_API_Bridge {
    public:
      // pure virtual function to get total number of fields in the table
      virtual Int32 numFields(PartitionData* partition) = 0;

      // pure virtual function to get name and value
      // for column = index, in the table
      virtual double valueForIndex(Node* node,
                                   Int32 index) = 0;
};

enum
{
    //For QueueSummaryTable
    QUEUE_SUMMARY_TABLE_TIME                      = 0,
    QUEUE_SUMMARY_INTERFACE_INDEX                 = 1,
    QUEUE_SUMMARY_QUEUE_INDEX                     = 2,
    QUEUE_SUMMARY_QUEUE_POSITION                  = 3,
    QUEUE_SUMMARY_NODE_ID                         = 4,
    QUEUE_SUMMARY_QUEUE_TYPE                      = 5,
    QUEUE_SUMMARY_QUEUE_SIZE                      = 6,
    QUEUE_SUMMARY_PACKETS_ENQUEUED                = 7,
    QUEUE_SUMMARY_PACKETS_DEQUEUED                = 8,
    QUEUE_SUMMARY_PACKETS_DROPPED                 = 9,
    QUEUE_SUMMARY_PACKETS_DROPPED_FORCE           = 10,
    QUEUE_SUMMARY_BYTES_ENQUEUED                  = 11,
    QUEUE_SUMMARY_BYTES_DEQUEUED                  = 12,
    QUEUE_SUMMARY_BYTES_DROPPED                   = 13,
    QUEUE_SUMMARY_BYTES_DROPPED_FORCE             = 14,
    QUEUE_SUMMARY_AVG_LENGTH                      = 15,
    QUEUE_SUMMARY_PEAK_LENGTH                     = 16,
    QUEUE_SUMMARY_AVG_TIME_IN_QUEUE               = 17,
    QUEUE_SUMMARY_LONG_TIME_IN_QUEUE              = 18,
    QUEUE_SUMMARY_AVG_FREE_SPACE                  = 19,
    QUEUE_SUMMARY_MIN_FREE_SPACE                  = 20,

    QUEUE_SUMMARY_TABLE_MAX_COLUMNS               = 21,
    QUEUE_SUMMARY_TABLE_MAX_FIXED_COLUMNS         = 21
};

enum
{
    //For QueueAggreagateTable
    QUEUE_AGGREGATE_TABLE_TIME                    = 0,
    QUEUE_AGGREGATE_PACKETS_ENQUEUED              = 1,
    QUEUE_AGGREGATE_PACKETS_DEQUEUED              = 2,
    QUEUE_AGGREGATE_PACKETS_DROPPED               = 3,
    QUEUE_AGGREGATE_PACKETS_DROPPED_FORCE         = 4,
    QUEUE_AGGREGATE_BYTES_ENQUEUED                = 5,
    QUEUE_AGGREGATE_BYTES_DEQUEUED                = 6,
    QUEUE_AGGREGATE_BYTES_DROPPED                 = 7,
    QUEUE_AGGREGATE_BYTES_DROPPED_FORCE           = 8,

    QUEUE_AGGREGATE_TABLE_MAX_COLUMNS             = 9,
    QUEUE_AGGREGATE_TABLE_MAX_FIXED_COLUMNS       = 9,
};

enum
{
    //For QueueStatusTable
    QUEUE_STATUS_TABLE_TIME                      = 0,
    QUEUE_STATUS_INTERFACE_INDEX                 = 1,
    QUEUE_STATUS_QUEUE_INDEX                     = 2,
    QUEUE_STATUS_FREE_SPACE                      = 3,
    QUEUE_STATUS_QUEUE_POSITION                  = 4,
    QUEUE_STATUS_NODE_ID                         = 5,
    QUEUE_STATUS_PACKETS_ENQUEUED                = 6,
    QUEUE_STATUS_PACKETS_DEQUEUED                = 7,
    QUEUE_STATUS_PACKETS_DROPPED                 = 8,
    QUEUE_STATUS_PACKETS_DROPPED_FORCE           = 9,
    QUEUE_STATUS_AVG_SERVICE_TIME                = 10,

    QUEUE_STATUS_TABLE_MAX_COLUMNS               = 11,
    QUEUE_STATUS_TABLE_MAX_FIXED_COLUMNS         = 11
};

// Forward Declaration
class STAT_GlobalAppStatistics;
class STAT_GlobalNetStatistics;
class STAT_GlobalTransStatistics;
class STAT_GlobalPhyStatistics;
class STAT_GlobalMacStatistics;
class STAT_GlobalQueueStatistics;

class STAT_GlobalAppStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalAppStatistics globalApp;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[APP_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[APP_AGGREGATE_TABLE_MAX_COLUMNS];


    protected:

    public:
        STAT_GlobalAppStatisticsBridge(STAT_GlobalAppStatistics globalApp,
                                                PartitionData* partition);
        // New Destructor to resolve memory leak issue
        ~STAT_GlobalAppStatisticsBridge();
        Int32 numFields(PartitionData* partition);
        std::string composeGlobalAppStatisticsInsertSQLString(Node* node,
           PartitionData* partition);  // compose the sql string;
        double valueForIndex(Node* node, Int32 index);
        void initializeNameTypeConfigurations();
        void copyFromGlobalApp(STAT_GlobalAppStatistics globalApp);
};


//For Application Summary Table

class STAT_AppSummaryBridge : public DB_API_Bridge
{
    private:
            STAT_AppSummarizer appUnicastSummary;
            STAT_AppMulticastSummarizer appMulticastSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[APP_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[APP_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_AppSummaryBridge();

            STAT_AppSummaryBridge(STAT_AppSummarizer appGlobalUnicastSummary,
                    STAT_AppMulticastSummarizer appGlobalMulticastSummary,
                    PartitionData* partition);

            Int32 numFields(PartitionData* partition);

            // compose SQL string
            void composeAppSummaryInsertSQLString(
                Node* node,
                PartitionData* partition,
                std::vector<std::string>* insertList,
                STAT_DestAddressType type = STAT_Unicast);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            // overloaded valueForIndex
            double valueForIndex(Node* node,
                                 Int32 index,
                                 STAT_AppSummaryStatistics& appSummary);

            std::string valueForIndexInString(
                Node* node,
                Int32 index,
                STAT_AppSummaryStatistics& appSummary);

            Int32 valueForIndexInInteger(
                Node* node,
                Int32 index,
                STAT_AppSummaryStatistics& appSummary);

            void initializeNameTypeConfigurations();

            void copyFromGlobalAppSummary(
                    STAT_AppSummarizer appGlobalUnicastSummary,
                    STAT_AppMulticastSummarizer appGlobalMulticastSummary);
};



//For MulticastApplication Summary Table

class STAT_MulticastAppSummaryBridge : public DB_API_Bridge
{
    private:
            STAT_MulticastAppSessionSummarizer* sessionSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_MulticastAppSummaryBridge();

            STAT_MulticastAppSummaryBridge(
                STAT_MulticastAppSessionSummarizer* appMulticastSessionSummary,
                PartitionData* partition);

            Int32 numFields(PartitionData* partition);

            // compose SQL string
            void composeMutlicastAppSummaryInsertSQLString(
                                Node* node,
                                PartitionData* partition,
                                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            // overloaded valueForIndex
            double valueForIndex(
                Node* node,
                Int32 index,
                STAT_MulticastAppSessionSummaryStatistics& sessionSummary);

            std::string valueForIndexInString(
                Node* node,
                Int32 index,
                STAT_MulticastAppSessionSummaryStatistics& sessionSummary);

            Int32 valueForIndexInInteger(
                Node* node,
                Int32 index,
                STAT_MulticastAppSessionSummaryStatistics& sessionSummary);

            void initializeNameTypeConfigurations();
};



// For Network Agrgegate
class STAT_GlobalNetStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalNetStatistics globalNet;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[NET_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[NET_AGGREGATE_TABLE_MAX_COLUMNS];


    protected:

    public:
        ~STAT_GlobalNetStatisticsBridge();
        STAT_GlobalNetStatisticsBridge(STAT_GlobalNetStatistics globalApp,
                                                PartitionData* partition);
        Int32 numFields(PartitionData* partition);

        // compose the sql string;
        std::string composeGlobalNetStatisticsInsertSQLString(Node* node,
                                               PartitionData* partition);

        double valueForIndex(Node* node, Int32 index);

        void initializeNameTypeConfigurations();

        void copyFromGlobalNet(STAT_GlobalNetStatistics globalNet);
};

//For Network Summary Table
class STAT_NetSummaryBridge : public DB_API_Bridge
{
    private:
            STAT_NetSummarizer netSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[NET_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[NET_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_NetSummaryBridge();

            STAT_NetSummaryBridge(STAT_NetSummarizer netSummary,
                    PartitionData* partition);

            Int32 numFields(PartitionData* partition);

            // compose SQL string
            void composeNetSummaryInsertSQLString(
                Node* node,
                PartitionData* partition,
                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            double valueForIndex(Node* node,
                                 Int32 index,
                                 STAT_NetSummaryStatistics& summary);

           std::string valueForIndexInString(
                Node* node,
                Int32 index,
                STAT_NetSummaryStatistics& summary);

            void initializeNameTypeConfigurations();

            void copyFromGlobalNetSummary(STAT_NetSummarizer netSummary);
};


// For Transport Agrgegate
class STAT_GlobalTransportStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalTransportStatistics* globalTransport;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[TRANS_AGGREGATE_TABLE_MAX_COLUMNS];


    protected:

    public:
        ~STAT_GlobalTransportStatisticsBridge();
        STAT_GlobalTransportStatisticsBridge(STAT_GlobalTransportStatistics* globalTrans,
                                                PartitionData* partition);
        Int32 numFields(PartitionData* partition);

        // compose the sql string;
        std::string composeGlobalTransportStatisticsInsertSQLString(Node* node,
                                               PartitionData* partition);
        double valueForIndex(Node* node, Int32 index);
        UInt64 valueForIndexInUInt64(Node* node, Int32 index);

        void initializeNameTypeConfigurations();
};

//For Transport Summary Table
class STAT_TransportSummaryBridge : public DB_API_Bridge
{
    private:
            STAT_TransportSummarizer* transportSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[TRANS_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[TRANS_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_TransportSummaryBridge();

            STAT_TransportSummaryBridge();
            STAT_TransportSummaryBridge(STAT_TransportSummarizer* transportSummary,
                PartitionData* partition);

            Int32 numFields(PartitionData* partition);

            // compose SQL string
            void composeTransportSummaryInsertSQLString(
                Node* node,
                PartitionData* partition,
                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            double valueForIndex(
                Node* node,
                Int32 index,
                STAT_TransportSummaryStatistics& summary);
            
            UInt64 valueForIndexInUInt64(
                Node* node,
                Int32 index,
                STAT_TransportSummaryStatistics& summary);

            std::string valueForIndexInString(
                Node* node,
                Int32 index,
                STAT_TransportSummaryStatistics& summary);

            void initializeNameTypeConfigurations();
};

//For Physical Summary Table
class STAT_PhySummaryBridge : public DB_API_Bridge
{
    private:
            STAT_PhySummarizer* phySummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[PHY_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration 
                tableConfigurations[PHY_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_PhySummaryBridge();

            STAT_PhySummaryBridge(STAT_PhySummarizer* phySummary,
                PartitionData* partition);

            Int32 numFields(PartitionData* partition);

            // compose SQL string
            void composePhysicalSummaryInsertSQLString(
                Node* node,
                PartitionData * partition,
                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            double valueForIndex(
                Node* node,
                Int32 index,
                STAT_PhySessionStatistics& summary);
            
            UInt64 valueForIndexInUInt64(
                Node* node,
                Int32 index,
                STAT_PhySessionStatistics& summary);

            void initializeNameTypeConfigurations();
};

// For Physical Agrgegate
class STAT_GlobalPhysicalStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalPhyStatistics* globalPhysical;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[PHY_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[PHY_AGGREGATE_TABLE_MAX_COLUMNS];

    protected:

    public:
        ~STAT_GlobalPhysicalStatisticsBridge();
        STAT_GlobalPhysicalStatisticsBridge(STAT_GlobalPhyStatistics* globalPhy,
                                                PartitionData * partition);
        Int32 numFields(PartitionData* partition);

        // compose the sql string;
        std::string composeGlobalPhysicalStatisticsInsertSQLString(Node* node,
                                               PartitionData * partition);
        double valueForIndex(Node* node, Int32 index);
        UInt64 valueForIndexInUInt64(Node* node, Int32 index);

        void initializeNameTypeConfigurations();
};
// For MAC Agrgegate
class STAT_GlobalMacStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalMacStatistics* globalMac;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[MAC_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[MAC_AGGREGATE_TABLE_MAX_COLUMNS];

    protected:

    public:
        ~STAT_GlobalMacStatisticsBridge();
        STAT_GlobalMacStatisticsBridge(STAT_GlobalMacStatistics* globalMac,
                                                PartitionData* partition);
        Int32 numFields(PartitionData* partition);

        // compose the sql string;
        std::string composeGlobalMacStatisticsInsertSQLString(Node* node,
                                               PartitionData* partition);

        double valueForIndex(Node* node, Int32 index);
        UInt64 valueForIndexInUInt64(Node* node, Int32 index);

        void initializeNameTypeConfigurations();
};

// For MAC Summary
class STAT_MacSummaryBridge : public DB_API_Bridge{
    private :
            STAT_MacSummarizer* macSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[MAC_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[MAC_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
        ~STAT_MacSummaryBridge();
        STAT_MacSummaryBridge(STAT_MacSummarizer* SUMMARY,
                                                PartitionData* partition);
        Int32 numFields(PartitionData* partition);

        // compose the sql string;
        void composeMacSummaryInsertSQLString(
                Node* node,
                PartitionData* partition,
                std::vector<std::string>* insertList);

        double valueForIndex(Node* node, Int32 index);
        double valueForIndex(Node* node,
                             Int32 index,
                             STAT_MacSummaryStatistics& summary);
        Int32 valueForIndexInInt(Node* node,
                                 Int32 index,
                                 STAT_MacSummaryStatistics& summary);
        UInt64 valueForIndexInUInt64(Node* node,
                                     Int32 index,
                                     STAT_MacSummaryStatistics& summary);


        void initializeNameTypeConfigurations();
};

class STAT_GlobalQueueStatisticsBridge : public DB_API_Bridge{
    private :
            STAT_GlobalQueueStatistics* globalQueue;
            Int32 numFixedFields;
            Int32 numOptionalFields;

            BOOL requiredFieldsIndex[QUEUE_AGGREGATE_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name ; // column name
                string type ; // column type
                string configuration ; // configuration, "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration
                    tableConfigurations[APP_AGGREGATE_TABLE_MAX_COLUMNS];


    protected:

    public:
        STAT_GlobalQueueStatisticsBridge(STAT_GlobalQueueStatistics* globalApp,
                                                PartitionData* partition);
        // New Destructor to resolve memory leak issue
        ~STAT_GlobalQueueStatisticsBridge();
        std::string composeGlobalQueueStatisticsInsertSQLString(Node* node,
           PartitionData* partition);  // compose the sql string;
        double valueForIndex(Node* node, Int32 index);
        UInt64 valueForIndexInUInt64(Node* node, Int32 index);
        Int32 numFields(PartitionData* partition);
        void initializeNameTypeConfigurations();
};


//For Queue Summary Table
class STAT_QueueSummaryBridge : public DB_API_Bridge
{
    private:
            STAT_QueueSummarizer* queueSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[QUEUE_SUMMARY_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration 
                tableConfigurations[QUEUE_SUMMARY_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_QueueSummaryBridge();

            STAT_QueueSummaryBridge(STAT_QueueSummarizer* queueSummary,
                PartitionData *partition);

            // compose SQL string
            void composeQueueSummaryInsertSQLString(
                Node* node,
                PartitionData * partition,
                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            double valueForIndex(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            Int32 valueForIndexInInt(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            std::string valueForIndexAsString(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);
            
            UInt64 valueForIndexInUInt64(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            Int32 numFields(PartitionData* partition);
            void initializeNameTypeConfigurations();
};

//For Queue Status Table
class STAT_QueueStatusBridge : public DB_API_Bridge
{
    private:
            STAT_QueueSummarizer* queueSummary;
            Int32 numFixedFields;
            Int32 numOptionalFields;
            BOOL requiredFieldsIndex[QUEUE_STATUS_TABLE_MAX_COLUMNS];

            struct StatsDB_NameTypeConfiguration{
                string name; // column name
                string type; // column type
                string configuration ; // "" means required column
                StatsDB_NameTypeConfiguration(string n, string t, string c):
                name(n), type(t), configuration(c) {}
                StatsDB_NameTypeConfiguration() {}
            };
            StatsDB_NameTypeConfiguration 
                tableConfigurations[QUEUE_STATUS_TABLE_MAX_COLUMNS];

    protected:

    public:
            ~STAT_QueueStatusBridge();

            STAT_QueueStatusBridge(STAT_QueueSummarizer* queueSummary,
                PartitionData* partition);

            // compose SQL string
            void composeQueueStatusInsertSQLString(
                Node* node,
                PartitionData * partition,
                std::vector<std::string>* insertList);

            // retrieving values for field index
            double valueForIndex(Node* node,
                                 Int32 index);

            double valueForIndex(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            Int32 valueForIndexInInt(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            std::string valueForIndexAsString(
                Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);
            
            UInt64 valueForIndexInUInt64
                (Node* node,
                Int32 index,
                STAT_QueueSummaryStatistics& summary);

            Int32 numFields(PartitionData* partition);
            void initializeNameTypeConfigurations();
};
#endif
