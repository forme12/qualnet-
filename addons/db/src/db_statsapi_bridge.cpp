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


#include <iostream>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#endif


#include "db_statsapi_bridge.h"
#include "dbapi.h"
#include "db.h"
#include "network_ip.h"

//#include "stats_app.h"

#define UNICAST 0
#define MULTICAST 1
#define  BROADCAST 2


// Bridge constructor
STAT_GlobalAppStatisticsBridge::STAT_GlobalAppStatisticsBridge(
                                    STAT_GlobalAppStatistics globalApp,
                                    PartitionData* partition)
{
    Int32 i;
    this->globalApp =  globalApp;
    numFixedFields = APP_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < APP_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// New Destructor to resolve memory leak issue

STAT_GlobalAppStatisticsBridge::~STAT_GlobalAppStatisticsBridge()
{
    Int32 i;
    for (i = 0; i < APP_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}

// Function to delegate data from STAT_GlobalAppStatistics object to
// bridge object

void
STAT_GlobalAppStatisticsBridge::copyFromGlobalApp(
                                    STAT_GlobalAppStatistics app)
{
    this->globalApp = app;
}

/* Function to enable/disable Application Aggregate table column indexes
   in array
*/
Int32 STAT_GlobalAppStatisticsBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsAppAggregate->isUnicastDelay)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isUnicastJitter)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isUnicastHopCount)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_HOPCOUNT] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isMulticastDelay)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isMulticastJitter)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isMulticastHopCount)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_HOPCOUNT] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isAvgDelay)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isAvgJitter)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isAvgThroughput)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_AVG_THROUGHPUT] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppAggregate->isAvgOfferload)
    {
        requiredFieldsIndex[APP_AGGREGATE_TABLE_AVG_OFFERLOAD] = TRUE;
        numOptionalFields++;
    }

    return numFixedFields + numOptionalFields;
}

// Function to compose SQL insertion string

std::string
STAT_GlobalAppStatisticsBridge::composeGlobalAppStatisticsInsertSQLString(
                                                Node* node,
                                                PartitionData* partition)
{
    std::vector<std::string> newValues;
    newValues.reserve(numFixedFields + numOptionalFields);
    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    Int32 i;

    columns.push_back(tableConfigurations[APP_AGGREGATE_TABLE_TIME].name);
    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = APP_AGGREGATE_TABLE_UNI_MSG_SENT;
         i < APP_AGGREGATE_TABLE_UNI_JITTER;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
            if (tableConfigurations[i].type == "real" ||
                tableConfigurations[i].type == "float" ||
                tableConfigurations[i].type == "bigint")
            {
                newValues.push_back(STATSDB_DoubleToString(
                    valueForIndex(node,i)));
            }
        }
    }
    for (i = APP_AGGREGATE_TABLE_UNI_JITTER;
         i < APP_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            double val = valueForIndex(node, i);
            if (val >= 0)
            {
                columns.push_back(tableConfigurations[i].name);
                newValues.push_back(STATSDB_DoubleToString(val));
            }
        }
    }

    return GetInsertValuesSQL("APPLICATION_Aggregate", columns, newValues);
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string
double STAT_GlobalAppStatisticsBridge::valueForIndex(Node* node,
                                                     Int32 index)
{
    double totalUnicastJitter = 0;
    double totalMulticastJitter = 0;
    double totalUnicastDelay = 0;
    double totalMulticastDelay = 0;
    double totalDelay = 0.0;
    double totalMsgReceived = 0;
    double totalUnicastBytesSent = 0;
    double totalUnicastBytesReceived = 0;
    double totalMulticastBytesReceived = 0;
    double currTime = (double) getSimTime(node)/ SECOND;

    switch (index)
    {
        case APP_AGGREGATE_TABLE_TIME:
            return currTime;

        case APP_AGGREGATE_TABLE_UNI_MSG_SENT:
            return globalApp.GetMessagesSent(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_MSG_RCVD:
            return globalApp.GetMessagesReceived(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_MSG_SENT:
            return globalApp.GetMessagesSent(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_EFCTV_MULTI_MSG_SENT:
            return globalApp.
                GetAggregatedEffectiveMessagesSent(STAT_Multicast).
                GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_MSG_RCVD:
            return globalApp.GetMessagesReceived(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_BYTES_SENT:
            return globalApp.GetDataSent(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_BYTES_RCVD:
            return globalApp.GetDataReceived(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_BYTES_SENT:
            return globalApp.GetDataSent(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_EFCTV_MULTI_BYTES_SENT:
            return globalApp.
                GetAggregatedEffectiveDataSent(STAT_Multicast).
                GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_MSG_BYTES_RCVD:
            return globalApp.GetDataReceived(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_FRAG_SENT:
            return globalApp.GetFragmentsSent(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_FRAG_RCVD:
            return globalApp.GetFragmentsReceived(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_FRAGSENT:
            return globalApp.GetFragmentsSent(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_EFCTV_MULTI_FRAG_SENT:
            return globalApp.
                GetAggregatedEffectiveFragmentsSent(STAT_Multicast).
                GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_FRAG_RCVD:
            return globalApp.GetFragmentsReceived(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_COMPLETION_RATE:
            return globalApp.GetMessageCompletionRate(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_COMPLETION_RATE:
          return globalApp.GetMessageCompletionRate(STAT_Multicast).
                                            GetValue(getSimTime(node));

        // Unicast offerload
        case APP_AGGREGATE_TABLE_UNI_OFFERLOAD:
            return (globalApp.GetDataSent(STAT_Unicast).
                                GetValue(getSimTime(node)) / currTime) * 8;

        // Unicast throughput
        case APP_AGGREGATE_TABLE_UNI_THROUGHPUT:
            return (globalApp.GetDataReceived(STAT_Unicast).
                                GetValue(getSimTime(node)) / currTime) * 8;

        // Multicast offerload
        case APP_AGGREGATE_TABLE_MULTI_OFFERLOAD:
            return globalApp.GetOfferedLoad(STAT_Multicast).
                                GetValue(getSimTime(node));

        // Multicast throughput
        case APP_AGGREGATE_TABLE_MULTI_THROUGHPUT:
            return (globalApp.GetDataReceived(STAT_Multicast).
                                GetValue(getSimTime(node)) / currTime) * 8;

        case APP_AGGREGATE_TABLE_UNI_DELAY:
            return globalApp.GetAverageDelay(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_DELAY:
            return globalApp.GetAverageDelay(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_UNI_JITTER:
        {
            if (globalApp.GetAverageJitter(STAT_Unicast).
                                            GetNumDataPoints() > 0)
            {
                return globalApp.GetAverageJitter(STAT_Unicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case APP_AGGREGATE_TABLE_MULTI_JITTER:
        {
            if (globalApp.GetAverageJitter(STAT_Multicast).
                                            GetNumDataPoints() > 0)
            {
                return globalApp.GetAverageJitter(STAT_Multicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case APP_AGGREGATE_TABLE_UNI_HOPCOUNT:
            return globalApp.GetAverageHopCount(STAT_Unicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_MULTI_HOPCOUNT:
            return globalApp.GetAverageHopCount(STAT_Multicast).
                                            GetValue(getSimTime(node));

        case APP_AGGREGATE_TABLE_AVG_JITTER:
            totalUnicastJitter = globalApp.GetTotalJitter(STAT_Unicast).
                                                GetValue(getSimTime(node));

            totalMulticastJitter = globalApp.GetTotalJitter(STAT_Multicast).
                                                GetValue(getSimTime(node));

            totalMsgReceived = globalApp.GetMessagesReceived(STAT_Unicast).
                                                GetValue(getSimTime(node))
                               +
                               globalApp.GetMessagesReceived(STAT_Multicast).
                                                GetValue(getSimTime(node));

            if (totalMsgReceived > 1)
            {
                return ((totalUnicastJitter + totalMulticastJitter)
                                                / (totalMsgReceived - 1));
            }
            else
            {
                return -1;
            }

        case APP_AGGREGATE_TABLE_AVG_DELAY:
            totalUnicastDelay = globalApp.GetAverageDelay(STAT_Unicast).
                                                GetValue(getSimTime(node))
                                *
                                globalApp.GetMessagesReceived(STAT_Unicast).
                                                GetValue(getSimTime(node));

            totalMulticastDelay = globalApp.GetAverageDelay(STAT_Multicast).
                                                GetValue(getSimTime(node))
                                  *
                               globalApp.GetMessagesReceived(STAT_Multicast).
                                                GetValue(getSimTime(node));

            totalDelay = totalUnicastDelay + totalMulticastDelay;

            totalMsgReceived = globalApp.GetMessagesReceived(STAT_Unicast).
                                                GetValue(getSimTime(node))
                               +
                               globalApp.GetMessagesReceived(STAT_Multicast).
                                                GetValue(getSimTime(node));

            if (totalMsgReceived > 0)
            {
                return (totalDelay / totalMsgReceived);
            }
            else
            {
                return 0;
            }

        case APP_AGGREGATE_TABLE_AVG_THROUGHPUT:
            return (globalApp.GetThroughput(STAT_Unicast).
                                                GetValue(getSimTime(node))
                    +
                    globalApp.GetThroughput(STAT_Multicast).
                                                GetValue(getSimTime(node)));

        case APP_AGGREGATE_TABLE_AVG_OFFERLOAD:
            return (globalApp.GetOfferedLoad(STAT_Unicast).
                                                GetValue(getSimTime(node))
                    +
                    globalApp.GetOfferedLoad(STAT_Multicast).
                                                GetValue(getSimTime(node)));

        default:
            return 0;
    }
}

// Function to initialize Name and Types of columns of
// Application Aggregate Table
void STAT_GlobalAppStatisticsBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[APP_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[APP_AGGREGATE_TABLE_TIME] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_MSG_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastMessagesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_MSG_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_MSG_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastMessagesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_MSG_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_MSG_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastMessagesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_MSG_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_EFCTV_MULTI_MSG_SENT] =
                    StatsDB_NameTypeConfiguration(
                                            "EffectiveMulticastMessagesSent",
                                            "bigint",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_EFCTV_MULTI_MSG_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_MSG_RCVD] =
                    StatsDB_NameTypeConfiguration(
                                            "MulticastMessagesReceived",
                                            "bigint",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_MSG_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastBytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_BYTES_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastBytesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_BYTES_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastBytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_BYTES_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_EFCTV_MULTI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration(
                                            "EffectiveMulticastBytesSent",
                                            "bigint",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_EFCTV_MULTI_BYTES_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_MSG_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastBytesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_MSG_BYTES_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_FRAG_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastFragSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_FRAG_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_FRAG_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastFragReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_FRAG_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_FRAGSENT] =
                    StatsDB_NameTypeConfiguration("MulticastFragSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_FRAGSENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_EFCTV_MULTI_FRAG_SENT] =
                    StatsDB_NameTypeConfiguration(
                                            "EffectiveMulticastFragSent",
                                            "bigint",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_EFCTV_MULTI_FRAG_SENT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_FRAG_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastFragReceived",
                                            "bigint",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_FRAG_RCVD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_COMPLETION_RATE] =
                    StatsDB_NameTypeConfiguration(
                                            "UnicastMessageCompletionRate",
                                            "float",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_COMPLETION_RATE] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_COMPLETION_RATE] =
                    StatsDB_NameTypeConfiguration(
                                            "MulticastMessageCompletionRate",
                                            "float",
                                            "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_COMPLETION_RATE] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("UnicastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_OFFERLOAD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_UNI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("UnicastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_UNI_THROUGHPUT] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("MulticastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_OFFERLOAD] = TRUE;


    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("MulticastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_AGGREGATE_TABLE_MULTI_THROUGHPUT] = TRUE;


    // now optional fields
    tableConfigurations[APP_AGGREGATE_TABLE_UNI_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastDelay",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-UNICAST-DELAY");

    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastDelay",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-DELAY");

    tableConfigurations[APP_AGGREGATE_TABLE_UNI_JITTER] =
                    StatsDB_NameTypeConfiguration("UnicastJitter",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-UNICAST-JITTER");

    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_JITTER] =
                    StatsDB_NameTypeConfiguration("MulticastJitter",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-JITTER");

    tableConfigurations[APP_AGGREGATE_TABLE_UNI_HOPCOUNT] =
                    StatsDB_NameTypeConfiguration("UnicastHopCount",
                         "float",
                         "STATS-DB-APPLICATION-AGGREGATE-UNICAST-HOP-COUNT");

    tableConfigurations[APP_AGGREGATE_TABLE_MULTI_HOPCOUNT] =
                    StatsDB_NameTypeConfiguration("MulticastHopCount",
                       "float",
                       "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-HOP-COUNT");

    tableConfigurations[APP_AGGREGATE_TABLE_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("AverageJitter",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-JITTER");

    tableConfigurations[APP_AGGREGATE_TABLE_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("AverageDelay",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-DELAY");

    tableConfigurations[APP_AGGREGATE_TABLE_AVG_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("AverageThroughput",
                        "float",
                        "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-THROUGHPUT");

    tableConfigurations[APP_AGGREGATE_TABLE_AVG_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("AverageOfferedload",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-OFFEREDLOAD");
}




// For App Summary Table 
// Bridge constructor
STAT_AppSummaryBridge::STAT_AppSummaryBridge(
                    STAT_AppSummarizer appGlobalUnicastSummary,
                    STAT_AppMulticastSummarizer appGlobalMulticastSummary,
                    PartitionData* partition)
{
    Int32 i;
    this->appUnicastSummary = appGlobalUnicastSummary;
    this->appMulticastSummary = appGlobalMulticastSummary;

    numFixedFields = APP_SUMMARY_TABLE_MAX_FIXED_COLUMNS;
    numOptionalFields = 0;

    for (i = 0; i < APP_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// Function to delegate data to bridge objects

void STAT_AppSummaryBridge::copyFromGlobalAppSummary(
                    STAT_AppSummarizer appGlobalUnicastSummary,
                    STAT_AppMulticastSummarizer appGlobalMulticastSummary)
{
    this->appUnicastSummary = appGlobalUnicastSummary;
    this->appMulticastSummary = appGlobalMulticastSummary;
}

// Function to enable/disable Application Summary table column indexes
// in array.

Int32 STAT_AppSummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsAppSummary->isDelay)
    {
        requiredFieldsIndex[APP_SUMMARY_TABLE_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppSummary->isJitter)
    {
        requiredFieldsIndex[APP_SUMMARY_TABLE_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsAppSummary->isHopCount)
    {
        requiredFieldsIndex[APP_SUMMARY_TABLE_AVG_HOPCOUNT] = TRUE;
        numOptionalFields++;
    }

    return numFixedFields + numOptionalFields;
}

// Function to compose SQL insertion string for APP Summary table for
// both unicast and multicast
void 
STAT_AppSummaryBridge::composeAppSummaryInsertSQLString(
                                        Node* node,
                                        PartitionData* partition,
                                        std::vector<std::string>* insertList,
                                        STAT_DestAddressType type)
{
    map<STAT_AppSummaryTag11, STAT_AppSummaryStatistics>::iterator it1;
    map<STAT_AppSummaryTag1N, STAT_AppSummaryStatistics>::iterator it2;

    Int32 applicationCount = 0;
    Int32 i;

    if (type == STAT_Unicast)
    {
        applicationCount = appUnicastSummary.size();
        it1 = appUnicastSummary.begin();
    }
    else if (type == STAT_Multicast)
    {
        applicationCount = appMulticastSummary.size();
        it2 = appMulticastSummary.begin();
    }

    //start a loop for every entry

    Int32 countOfApplications = 0;
    STAT_AppSummaryStatistics temp;

    for (;
        countOfApplications < applicationCount;
        countOfApplications++)
    {
        std::vector<std::string> columns;
        columns.reserve(numFixedFields + numOptionalFields);
        columns.push_back(tableConfigurations[APP_SUMMARY_TABLE_TIME].name);
        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);

        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));

        if (type == STAT_Unicast)
        {
            temp = it1->second;
            ++it1;
        }
        else if (type == STAT_Multicast)
        {
            temp = it2->second;
            ++it2;
        }

        for (i = APP_SUMMARY_TABLE_SENDER_ID;
             i < APP_SUMMARY_TABLE_AVG_JITTER;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                columns.push_back(tableConfigurations[i].name);
                if (tableConfigurations[i].type == "real" ||
                        tableConfigurations[i].type == "float" ||
                        tableConfigurations[i].type == "bigint")
                {
                    newValues.push_back(STATSDB_DoubleToString(
                        valueForIndex(node, i,temp)));
                }
                else if (tableConfigurations[i].type == "integer")
                {
                    newValues.push_back(STATSDB_IntToString(
                        valueForIndexInInteger(node, i,temp)));
                }
                else if (tableConfigurations[i].type == "string")
                {
                    newValues.push_back(valueForIndexInString(
                        node, i,temp));
                }
            }
        }
        for (i = APP_SUMMARY_TABLE_AVG_JITTER;
             i < APP_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                double val = valueForIndex(node, i, temp);
                if (val >= 0)
                {
                    columns.push_back(tableConfigurations[i].name);
                    newValues.push_back(STATSDB_DoubleToString(val));
                }
            }
        }

        insertList->push_back(GetInsertValuesSQL(
            "APPLICATION_Summary", columns, newValues));
    }
}


// Function to call Stat APIs to get various values to be inserted in
// SQL string for AppSummary table

double
STAT_AppSummaryBridge::valueForIndex(Node* node,
                                     Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double 
STAT_AppSummaryBridge::valueForIndex(Node* node,
                                     Int32 index,
                                     STAT_AppSummaryStatistics& appSummary)
{
    switch (index)
    {
        case APP_SUMMARY_TABLE_MSG_SENT:
            return appSummary.messagesSent;
            break;
        case APP_SUMMARY_TABLE_EFCTV_MSG_SENT:
            return appSummary.effectiveMessagesSent;
            break;
        case APP_SUMMARY_TABLE_MSG_RCVD:
            return appSummary.messagesReceived;
            break;
        case APP_SUMMARY_TABLE_BYTES_SENT:
            return appSummary.dataSent;
            break;
        case APP_SUMMARY_TABLE_EFCTV_BYTES_SENT:
            return appSummary.effectiveDataSent;
            break;
        case APP_SUMMARY_TABLE_BYTES_RCVD:
            return appSummary.dataReceived;
            break;
        case APP_SUMMARY_TABLE_FRAG_SENT:
            return appSummary.fragmentsSent;
            break;
        case APP_SUMMARY_TABLE_EFCTV_FRAG_SENT:
            return appSummary.effectiveFragmentsSent;
            break;
        case APP_SUMMARY_TABLE_FRAG_RCVD:
            return appSummary.fragmentsReceived;
            break;
        case APP_SUMMARY_TABLE_MSG_COMP_RATE:
            if (appSummary.dataSent != 0)
            {
                return (appSummary.dataReceived/appSummary.dataSent);
            }
            else
            {
                return 0;
            }
            break;
        case APP_SUMMARY_TABLE_OFFERLOAD:
            return appSummary.offeredLoad;
            break;
        case APP_SUMMARY_TABLE_THROUGHPUT:
            return appSummary.throughput;
            break;
        case APP_SUMMARY_TABLE_AVG_DELAY:
            return appSummary.averageDelay;
            break;
        case APP_SUMMARY_TABLE_AVG_JITTER:
            if (appSummary.numJitterDataPoints > 0)
            {
                return appSummary.averageJitter;
            }
            else
            {
                return -1;
            }
            break;
        case APP_SUMMARY_TABLE_AVG_HOPCOUNT:
            return appSummary.averageHopCount;
            break;
        default:
            return 0;
    }
    return 0;
}

std::string 
STAT_AppSummaryBridge::valueForIndexInString(
    Node* node,
    Int32 index,
    STAT_AppSummaryStatistics& appSummary)
{
    char tempAddr[MAX_STRING_LENGTH];
    std:string tempStr;

    switch(index)
    {
        case APP_SUMMARY_TABLE_RECEIVER_ADDR:
            
            sprintf(tempAddr, "%u.%u.%u.%u",
            (appSummary.receiverAddress & 0xff000000) >> 24,
            (appSummary.receiverAddress & 0xff0000) >> 16,
            (appSummary.receiverAddress & 0xff00) >> 8,
            appSummary.receiverAddress & 0xff);
            tempStr = tempAddr;
            return tempStr;
            break;
        case APP_SUMMARY_TABLE_APP_TYPE:
            return appSummary.type;
            break;
        case APP_SUMMARY_TABLE_APP_NAME:
            return appSummary.name;
            break;
        default:
            return "";
    }
    return "";


}

Int32 
STAT_AppSummaryBridge::valueForIndexInInteger(
    Node* node,
    Int32 index,
    STAT_AppSummaryStatistics& appSummary)
{
    switch(index)
    {
        case APP_SUMMARY_TABLE_SENDER_ID:
            return (Int32)appSummary.senderId;
            break;
        case APP_SUMMARY_TABLE_RECEIVER_ID:
            return (Int32)appSummary.receiverId;
            break;
        case APP_SUMMARY_TABLE_SESSION_ID:
            return (Int32)appSummary.sessionId;
            break;
        case APP_SUMMARY_TABLE_TOS:
            return (Int32)appSummary.tos;
            break;
        default:
            return 0;
    }
    return 0;
}

// Function to initialize Name and Types of columns of
// Application Summary Table
void STAT_AppSummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[APP_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[APP_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[APP_SUMMARY_TABLE_SENDER_ID] =
                    StatsDB_NameTypeConfiguration("SenderId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_SENDER_ID] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_RECEIVER_ID] =
                    StatsDB_NameTypeConfiguration("ReceiverId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_RECEIVER_ID] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_RECEIVER_ADDR] =
                    StatsDB_NameTypeConfiguration("ReceiverAddress",
                                                  "string",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_RECEIVER_ADDR] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_SESSION_ID] =
                    StatsDB_NameTypeConfiguration("SessionId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_SESSION_ID] = TRUE;
    

    tableConfigurations[APP_SUMMARY_TABLE_TOS] =
                    StatsDB_NameTypeConfiguration("Tos",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_TOS] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_MSG_SENT] =
                    StatsDB_NameTypeConfiguration("MessagesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_MSG_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_EFCTV_MSG_SENT] =
                    StatsDB_NameTypeConfiguration("EffectiveMessagesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_EFCTV_MSG_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_MSG_RCVD] =
                    StatsDB_NameTypeConfiguration("MessagesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_MSG_RCVD] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_BYTES_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_EFCTV_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("EffectiveBytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_EFCTV_BYTES_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BytesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_BYTES_RCVD] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_FRAG_SENT] =
                    StatsDB_NameTypeConfiguration("FragSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_FRAG_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_EFCTV_FRAG_SENT] =
                    StatsDB_NameTypeConfiguration("EffectiveFragSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_EFCTV_FRAG_SENT] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_FRAG_RCVD] =
                    StatsDB_NameTypeConfiguration("FragReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_FRAG_RCVD] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_APP_TYPE] =
                    StatsDB_NameTypeConfiguration("ApplicationType",
                                                  "string",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_APP_TYPE] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_APP_NAME] =
                    StatsDB_NameTypeConfiguration("ApplicationName",
                                                  "string",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_APP_NAME] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_MSG_COMP_RATE] =
                    StatsDB_NameTypeConfiguration("MessageCompletionRate",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_MSG_COMP_RATE] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("OfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_OFFERLOAD] = TRUE;


    tableConfigurations[APP_SUMMARY_TABLE_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("Throughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[APP_SUMMARY_TABLE_THROUGHPUT] = TRUE;

    // now optional fields
    tableConfigurations[APP_SUMMARY_TABLE_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("Delay",
                          "float",
                          "STATS-DB-APPLICATION-AGGREGATE-UNICAST-DELAY");

    tableConfigurations[APP_SUMMARY_TABLE_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("Jitter",
                          "float",
                          "STATS-DB-APPLICATION-SUMMARY-JITTER");

    tableConfigurations[APP_SUMMARY_TABLE_AVG_HOPCOUNT] =
                    StatsDB_NameTypeConfiguration("HopCount",
                         "float",
                         "STATS-DB-APPLICATION-SUMMARY-HOP-COUNT");

}

STAT_AppSummaryBridge::~STAT_AppSummaryBridge()
{
    Int32 i;
    for (i = 0; i < APP_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}

// Network Aggregate

STAT_GlobalNetStatisticsBridge::STAT_GlobalNetStatisticsBridge(
                                    STAT_GlobalNetStatistics globalNet,
                                    PartitionData* partition)
{
    Int32 i;
    this->globalNet =  globalNet;
    numFixedFields = NET_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < NET_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// Function to delegate data from STAT_GlobalAppStatistics object to
// bridge object

void
STAT_GlobalNetStatisticsBridge::copyFromGlobalNet(
                                    STAT_GlobalNetStatistics net)
{
    this->globalNet = net;
}

/* Function to enable/disable Network Aggregate table column indexes
   in array
*/
Int32 STAT_GlobalNetStatisticsBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsNetAggregate->isDelay[UNICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isDelay[MULTICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isDelay[BROADCAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isJitter[UNICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isJitter[MULTICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isJitter[BROADCAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isIpOutNoRoutes[UNICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_IP_OUT_NO_ROUTE] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetAggregate->isIpOutNoRoutes[MULTICAST])
    {
        requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_IP_OUT_NO_ROUTE] = TRUE;
        numOptionalFields++;
    }

    return numFixedFields + numOptionalFields;
}

// Function to compose SQL insertion string

std::string
STAT_GlobalNetStatisticsBridge::composeGlobalNetStatisticsInsertSQLString(
                                                Node* node,
                                                PartitionData* partition)
{
    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    columns.push_back(tableConfigurations[NET_AGGREGATE_TABLE_TIME].name);
    std::vector<std::string> newValues;
    newValues.reserve(numFixedFields + numOptionalFields);
    Int32 i;

    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = NET_AGGREGATE_TABLE_UNI_OFFERLOAD;
         i < NET_AGGREGATE_TABLE_UNI_JITTER;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
            if (tableConfigurations[i].type == "real" ||
                    tableConfigurations[i].type == "float" ||
                    tableConfigurations[i].type == "bigint")
            {
                    newValues.push_back(STATSDB_DoubleToString(
                                            valueForIndex(node, i)));
            }
        }
    }
    for (i = NET_AGGREGATE_TABLE_UNI_JITTER; 
         i < NET_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            double val = valueForIndex(node, i);
            if (val >= 0)
            {
                columns.push_back(tableConfigurations[i].name);
                newValues.push_back(STATSDB_DoubleToString(val));
            }
        }
    }
    return GetInsertValuesSQL("NETWORK_Aggregate", columns, newValues);
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string
double STAT_GlobalNetStatisticsBridge::valueForIndex(Node* node,
                                                     Int32 index)
{
    double currTime = (double) getSimTime(node)/ SECOND;

    switch (index)
    {
        case NET_AGGREGATE_TABLE_TIME:
            return currTime;

        case NET_AGGREGATE_TABLE_UNI_OFFERLOAD:
            return globalNet.GetCarriedLoadUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_OFFERLOAD:
            return globalNet.GetCarriedLoadMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_OFFERLOAD:
            return globalNet.GetCarriedLoadBroadcast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_DATA_PKT_SENT:
            return globalNet.
                GetDataPacketsSentUnicast().
                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_DATA_PKT_RCVD:
            return globalNet.GetDataPacketsReceivedUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_DATA_PKT_FORW:
            return globalNet.GetDataPacketsForwardedUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_PKT_SENT:
            return globalNet.GetDataPacketsSentMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_PKT_RCVD:
            return globalNet.GetDataPacketsReceivedMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_PKT_FORW:
            return globalNet.
                GetDataPacketsForwardedMulticast().
                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_PKT_SENT:
            return globalNet.GetDataPacketsSentBroadcast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_PKT_RCVD:
            return globalNet.GetDataPacketsReceivedBroadcast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_PKT_FORW:
            return globalNet.GetDataPacketsForwardedBroadcast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_CTRL_PKT_SENT:
            return globalNet.GetControlPacketsSentUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_CTRL_PKT_RCVD:
            return globalNet.
                GetControlPacketsReceivedUnicast().
                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_CTRL_PKT_FORW:
            return globalNet.GetControlPacketsForwardedUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_SENT:
            return globalNet.GetControlPacketsSentMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_RCVD:
          return globalNet.GetControlPacketsReceivedMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_FORW:
            return (globalNet.GetControlPacketsForwardedMulticast().
                                GetValue(getSimTime(node)));

        case NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_SENT:
            return (globalNet.GetControlPacketsSentBroadcast().
                                GetValue(getSimTime(node)));

        // Multicast offerload
        case NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_RCVD:
            return globalNet.GetControlPacketsReceivedBroadcast().
                                GetValue(getSimTime(node));

        // Multicast throughput
        case NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_FORW:
            return (globalNet.GetControlPacketsForwardedBroadcast().
                                GetValue(getSimTime(node)));

        case NET_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT:
            return globalNet.GetDataBytesSentUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD:
            return globalNet.GetDataBytesReceivedUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_DATA_BYTES_FORW:
            return globalNet.GetDataBytesForwardedUnicast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT:
            return globalNet.GetDataBytesSentMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD:
            return globalNet.GetDataBytesReceivedMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_FORW:
            return globalNet.GetDataBytesForwardedMulticast().
                                            GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT:
            return globalNet.GetDataBytesSentBroadcast().
                                                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD:
            return globalNet.GetDataBytesReceivedBroadcast().
                                                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_FORW:
            return globalNet.GetDataBytesForwardedBroadcast().
                                                GetValue(getSimTime(node));

        case NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_SENT:
            return globalNet.GetControlBytesSentUnicast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_RCVD:
            return globalNet.GetControlBytesReceivedUnicast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_FORW:
            return globalNet.GetControlBytesForwardedUnicast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_SENT:
            return globalNet.GetControlBytesSentMulticast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_RCVD:
            return globalNet.GetControlBytesReceivedMulticast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_FORW:
            return globalNet.GetControlBytesForwardedMulticast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_SENT:
            return globalNet.GetControlBytesSentBroadcast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_RCVD:
            return globalNet.GetControlBytesReceivedBroadcast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_FORW:
            return globalNet.GetControlBytesForwardedBroadcast().
                                            GetValue(getSimTime(node));


        case NET_AGGREGATE_TABLE_UNI_DELAY:
            return globalNet.GetAverageDelayUnicast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_MULTI_DELAY:
            return globalNet.GetAverageDelayMulticast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_BROAD_DELAY:
            return globalNet.GetAverageDelayBroadcast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_UNI_JITTER:
        {
            if (globalNet.GetAverageJitterUnicast().GetNumDataPoints() > 0)
            {
                return globalNet.GetAverageJitterUnicast().
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case NET_AGGREGATE_TABLE_MULTI_JITTER:
        {
            if (globalNet.GetAverageJitterMulticast().
                                            GetNumDataPoints() > 0)
            {
                return globalNet.GetAverageJitterMulticast().
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case NET_AGGREGATE_TABLE_BROAD_JITTER:
        {
            if (globalNet.GetAverageJitterBroadcast().
                                            GetNumDataPoints() > 0)
            {
                return globalNet.GetAverageJitterBroadcast().
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case NET_AGGREGATE_TABLE_UNI_IP_OUT_NO_ROUTE:
            return globalNet.GetPacketsDroppedNoRouteUnicast().
                                            GetValue(getSimTime(node));
        case NET_AGGREGATE_TABLE_MULTI_IP_OUT_NO_ROUTE:
            return globalNet.GetPacketsDroppedNoRouteMulticast().
                                            GetValue(getSimTime(node));

        default:
            return 0;
    }
}

// Function to initialize Name and Types of columns of
// Network Aggregate Table
void STAT_GlobalNetStatisticsBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[NET_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp","real","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_TIME] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_OFFERLOAD] =
        StatsDB_NameTypeConfiguration("UnicastOfferedLoad","float","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_OFFERLOAD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_OFFERLOAD] =
            StatsDB_NameTypeConfiguration("MulticastOfferedLoad","float","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_OFFERLOAD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_OFFERLOAD] =
           StatsDB_NameTypeConfiguration("BroadcastOfferedLoad","float","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_OFFERLOAD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_PKT_SENT] =
         StatsDB_NameTypeConfiguration("UnicastDataPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_PKT_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_PKT_RCVD] =
        StatsDB_NameTypeConfiguration("UnicastDataPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_PKT_FORW] =
      StatsDB_NameTypeConfiguration("UnicastDataPacketsForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_PKT_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_SENT] =
       StatsDB_NameTypeConfiguration("MulticastDataPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_RCVD] =
        StatsDB_NameTypeConfiguration("MulticastDataPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_FORW] =
     StatsDB_NameTypeConfiguration("MulticastDataPacketsForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_PKT_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_SENT] =
        StatsDB_NameTypeConfiguration("BroadcastDataPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_RCVD] =
        StatsDB_NameTypeConfiguration("BroadcastDataPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_FORW] =
      StatsDB_NameTypeConfiguration("BroadcastDataPacketsForward",
                                    "bigint",
                                    "");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_PKT_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_SENT] =
      StatsDB_NameTypeConfiguration("UnicastControlPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_RCVD] =
      StatsDB_NameTypeConfiguration("UnicastControlPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_FORW] =
        StatsDB_NameTypeConfiguration(
                                    "UnicastControlPacketsForward",
                                    "bigint",
                                    "");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_PKT_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_SENT] =
        StatsDB_NameTypeConfiguration("MulticastControlPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_RCVD] =
        StatsDB_NameTypeConfiguration("MulticastControlPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_FORW] =
        StatsDB_NameTypeConfiguration("MulticastControlPacketsForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_PKT_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_SENT] =
        StatsDB_NameTypeConfiguration("BroadcastControlPacketsSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_SENT] = TRUE;

    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_RCVD] =
        StatsDB_NameTypeConfiguration("BroadcastControlPacketsRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_FORW] =
        StatsDB_NameTypeConfiguration("BroadcastControlPacketsForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_PKT_FORW] = TRUE;

    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT] = 
        StatsDB_NameTypeConfiguration("UnicastDataBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("UnicastDataBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_FORW] =
        StatsDB_NameTypeConfiguration("UnicastDataBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_DATA_BYTES_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT] =
        StatsDB_NameTypeConfiguration("MulticastDataBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("MulticastDataBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_FORW] =
      StatsDB_NameTypeConfiguration("MulticastDataBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_DATA_BYTES_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT] =
        StatsDB_NameTypeConfiguration("BroadcastDataBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("BroadcastDataBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_FORW] =
        StatsDB_NameTypeConfiguration("BroadcastDataBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_DATA_BYTES_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_SENT] =
        StatsDB_NameTypeConfiguration("UnicastControlBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("UnicastControlBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_FORW] =
     StatsDB_NameTypeConfiguration("UnicastControlBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_UNI_CTRL_BYTES_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_SENT] =
        StatsDB_NameTypeConfiguration("MulticastControlBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_SENT] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("MulticastControlBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_FORW] =
        StatsDB_NameTypeConfiguration("MulticastControlBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_MULTI_CTRL_BYTES_FORW] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_SENT] =
        StatsDB_NameTypeConfiguration("BroadcastControlBytesSent","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_SENT] = TRUE;

    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_RCVD] =
        StatsDB_NameTypeConfiguration("BroadcastControlBytesRecd","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_FORW] =
        StatsDB_NameTypeConfiguration("BroadcastControlBytesForward","bigint","");
    requiredFieldsIndex[NET_AGGREGATE_TABLE_BROAD_CTRL_BYTES_FORW] = TRUE;

    // now optional fields
    tableConfigurations[NET_AGGREGATE_TABLE_UNI_DELAY] =
          StatsDB_NameTypeConfiguration(
          "UnicastDelay",
          "float",
          "STATS-DB-NETWORK-AGGREGATE-UNICAST-DELAY");

    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_DELAY] =
        StatsDB_NameTypeConfiguration(
        "MulticastDelay",
        "float",
        "STATS-DB-NETWORK-AGGREGATE-MULTICAST-DELAY");


    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_DELAY] =
        StatsDB_NameTypeConfiguration(
        "BroadcastDelay",
        "float",
        "STATS-DB-NETWORK-AGGREGATE-BROADCAST-DELAY");

    tableConfigurations[NET_AGGREGATE_TABLE_UNI_JITTER] =
        StatsDB_NameTypeConfiguration(
        "UnicastJitter",
        "float",
        "STATS-DB-APPLICATION-AGGREGATE-UNICAST-JITTER");

    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_JITTER] =
        StatsDB_NameTypeConfiguration(
        "MulticastJitter",
        "float",
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-JITTER");

    tableConfigurations[NET_AGGREGATE_TABLE_BROAD_JITTER] =
        StatsDB_NameTypeConfiguration(
        "BroadcastJitter",
        "float",
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-JITTER");

    tableConfigurations[NET_AGGREGATE_TABLE_UNI_IP_OUT_NO_ROUTE] =
        StatsDB_NameTypeConfiguration(
        "UnicastIpOutNoRoutes",
        "bigint",
        "STATS-DB-APPLICATION-AGGREGATE-UNICAST-HOP-COUNT");

    tableConfigurations[NET_AGGREGATE_TABLE_MULTI_IP_OUT_NO_ROUTE] =
        StatsDB_NameTypeConfiguration(
        "MulticastIpOutNoRoutes",
        "bigint",
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-HOP-COUNT");
}


STAT_GlobalNetStatisticsBridge::~STAT_GlobalNetStatisticsBridge()
{
    Int32 i;
    for (i = 0; i < NET_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}

// For Net Summary Table, Bridge constructor
STAT_NetSummaryBridge::STAT_NetSummaryBridge(
                    STAT_NetSummarizer netGlobalSummary,
                    PartitionData* partition)
{
    this->netSummary = netGlobalSummary;
    numFixedFields = NET_SUMMARY_TABLE_MAX_FIXED_COLUMNS;
    numOptionalFields = 0;
    Int32 i;
    for (i = 0; i < NET_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// Function to delegate data to bridge objects

void STAT_NetSummaryBridge::copyFromGlobalNetSummary(
                    STAT_NetSummarizer netSummary)
{
    this->netSummary = netSummary;

}

// Function to enable/disable Network Summary table column indexes
// in array.

Int32 STAT_NetSummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsNetSummary->isDataDelay)
    {
        requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetSummary->isControlDelay)
    {
        requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetSummary->isDataJitter)
    {
        requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsNetSummary->isControlJitter)
    {
        requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_JITTER] = TRUE;
        numOptionalFields++;
    }

    return numFixedFields + numOptionalFields;
}

// Function to compose SQL insertion string for NET Summary table for
// both unicast and multicast
void
STAT_NetSummaryBridge::composeNetSummaryInsertSQLString(
                                        Node* node,
                                        PartitionData* partition,
                                        std::vector<std::string>* insertList)
{
    Int32 countOfNetSummary;
    Int32 netSummarySize = netSummary.size();
    Int32 i;

    map<STAT_NetSummaryTag11, STAT_NetSummaryStatistics>::iterator it;
    it = netSummary.begin();

    for (countOfNetSummary = 0;
        countOfNetSummary < netSummarySize;
        countOfNetSummary++)
    {
        std::vector<std::string> columns;
        columns.reserve(numFixedFields + numOptionalFields);
        columns.push_back(tableConfigurations[APP_SUMMARY_TABLE_TIME].name);

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);
        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));

        STAT_NetSummaryStatistics& temp = it->second;
        ++it;

        for (i = NET_SUMMARY_TABLE_SENDER_ADDR;
             i < NET_SUMMARY_TABLE_DATA_JITTER;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                columns.push_back(tableConfigurations[i].name);
                if (tableConfigurations[i].type == "real"
                    || tableConfigurations[i].type == "bigint")
                {
                    newValues.push_back(STATSDB_DoubleToString(
                        valueForIndex(node, i,temp)));
                }
                else if (tableConfigurations[i].type == "string")
                {
                    newValues.push_back(valueForIndexInString
                                                    (node, i,temp));
                }
            }
        }
        for (i = NET_SUMMARY_TABLE_DATA_JITTER;
             i < NET_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                double val = valueForIndex(node, i, temp);
                if (val >= 0)
                {
                    columns.push_back(tableConfigurations[i].name);
                    newValues.push_back(STATSDB_DoubleToString(val));
                }
            }
        }

        insertList->push_back(GetInsertValuesSQL(
                                "NETWORK_Summary", columns, newValues));
    }
}


// Function to call Stat APIs to get various values to be inserted in
// SQL string for NetSummary table

double
STAT_NetSummaryBridge::valueForIndex(Node* node,
                                     Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double
STAT_NetSummaryBridge::valueForIndex(Node* node,
                                     Int32 index,
                                     STAT_NetSummaryStatistics& summary)
{
    switch(index)
    {
        case NET_SUMMARY_TABLE_DATA_PACKET_SENT:
            return summary.dataPacketsSent;
            break;
        case NET_SUMMARY_TABLE_DATA_PACKET_RCVD:
            return summary.dataPacketsRecd;
            break;
        case NET_SUMMARY_TABLE_DATA_PACKET_FRWD:
            return summary.dataPacketsForward;
            break;
        case NET_SUMMARY_TABLE_CONTROL_PACKET_SENT:
            return summary.controlPacketsSent;
            break;
        case NET_SUMMARY_TABLE_CONTROL_PACKET_RCVD:
            return summary.controlPacketsRecd;
            break;
        case NET_SUMMARY_TABLE_CONTROL_PACKET_FRWD:
            return summary.controlPacketsForward;
            break;
        case NET_SUMMARY_TABLE_DATA_BYTES_SENT:
            return summary.dataBytesSent;
            break;
        case NET_SUMMARY_TABLE_DATA_BYTES_RCVD:
            return summary.dataBytesRecd;
            break;
        case NET_SUMMARY_TABLE_DATA_BYTES_FRWD:
            return summary.dataBytesForward;
            break;
        case NET_SUMMARY_TABLE_CONTROL_BYTES_SENT:
            return summary.controlBytesSent;
            break;
        case NET_SUMMARY_TABLE_CONTROL_BYTES_RCVD:
            return summary.controlBytesRecd;
            break;
        case NET_SUMMARY_TABLE_CONTROL_BYTES_FRWD:
            return summary.controlBytesForward;
            break;
        case NET_SUMMARY_TABLE_DATA_DELAY:
            return summary.dataDelay;
            break;
        case NET_SUMMARY_TABLE_CONTROL_DELAY:
            return summary.controlDelay;
            break;
        case NET_SUMMARY_TABLE_DATA_JITTER:
            if (summary.dataJitterDataPoints > 0)
            {
                return summary.dataJitter;
            }
            else
            {
                return -1;
            }
            break;
        case NET_SUMMARY_TABLE_CONTROL_JITTER:
            if (summary.controlJitterDataPoints > 0)
            {
                return summary.controlJitter;
            }
            else
            {
                return -1;
            }
            break;
        default:
            return 0;
    }
    return 0;
}

std::string 
STAT_NetSummaryBridge::valueForIndexInString(
    Node* node,
    Int32 index,
    STAT_NetSummaryStatistics& summary)
{
    char tempAddr[MAX_STRING_LENGTH];
    std:string tempStr;

    switch(index)
    {
        case NET_SUMMARY_TABLE_SENDER_ADDR:

            sprintf(tempAddr, "%u.%u.%u.%u",
            (summary.senderAddr & 0xff000000) >> 24,
            (summary.senderAddr & 0xff0000) >> 16,
            (summary.senderAddr & 0xff00) >> 8,
            summary.senderAddr & 0xff);
            tempStr = tempAddr;
            return tempStr;
            break;

        case NET_SUMMARY_TABLE_RECEIVER_ADDR:

            sprintf(tempAddr, "%u.%u.%u.%u",
            (summary.receiverAddr & 0xff000000) >> 24,
            (summary.receiverAddr & 0xff0000) >> 16,
            (summary.receiverAddr & 0xff00) >> 8,
            summary.receiverAddr & 0xff);
            tempStr = tempAddr;
            return tempStr;
            break;

        default:
            return "";
    }
    return "";
}

// Function to initialize Name and Types of columns of
// Network Summary Table
void STAT_NetSummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[NET_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[NET_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[NET_SUMMARY_TABLE_SENDER_ADDR] =
                    StatsDB_NameTypeConfiguration("SenderAddr",
                                                  "string",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_SENDER_ADDR] = TRUE;

    tableConfigurations[NET_SUMMARY_TABLE_RECEIVER_ADDR] =
                    StatsDB_NameTypeConfiguration("ReceiverAddr",
                                                  "string",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_RECEIVER_ADDR] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_DATA_PACKET_SENT] =
                    StatsDB_NameTypeConfiguration("DataPacketsSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_PACKET_SENT] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_DATA_PACKET_RCVD] =
                    StatsDB_NameTypeConfiguration("DataPacketsRecd",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_PACKET_RCVD] = TRUE;
    

    tableConfigurations[NET_SUMMARY_TABLE_DATA_PACKET_FRWD] =
                    StatsDB_NameTypeConfiguration("DataPacketsForward",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_PACKET_FRWD] = TRUE;

    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_PACKET_SENT] =
                    StatsDB_NameTypeConfiguration("ControlPacketsSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_PACKET_SENT] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_PACKET_RCVD] =
                    StatsDB_NameTypeConfiguration("ControlPacketsRecd",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_PACKET_RCVD] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_PACKET_FRWD] =
                    StatsDB_NameTypeConfiguration("ControlPacketsForward",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_PACKET_FRWD] = TRUE;

    tableConfigurations[NET_SUMMARY_TABLE_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("DataBytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_BYTES_SENT] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("DataBytesRecd",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_BYTES_RCVD] = TRUE;
    

    tableConfigurations[NET_SUMMARY_TABLE_DATA_BYTES_FRWD] =
                    StatsDB_NameTypeConfiguration("DataBytesForward",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_DATA_BYTES_FRWD] = TRUE;

        tableConfigurations[NET_SUMMARY_TABLE_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("ControlBytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_BYTES_SENT] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("ControlBytesRecd",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_BYTES_RCVD] = TRUE;


    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_BYTES_FRWD] =
                    StatsDB_NameTypeConfiguration("ControlBytesForward",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[NET_SUMMARY_TABLE_CONTROL_BYTES_FRWD] = TRUE;



    // now optional fields
    tableConfigurations[NET_SUMMARY_TABLE_DATA_DELAY] =
                    StatsDB_NameTypeConfiguration("DataDelay",
                          "real",
                          "STATS-DB-NETWORK-SUMMARY-DATA-DELAY");

    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_DELAY] =
                    StatsDB_NameTypeConfiguration("ControlDelay",
                          "real",
                          "STATS-DB-NETWORK-SUMMARY-CONTROL-DELAY");

    tableConfigurations[NET_SUMMARY_TABLE_DATA_JITTER] =
                    StatsDB_NameTypeConfiguration("DataJitter",
                          "real",
                          "STATS-DB-NETWORK-SUMMARY-DATA-JITTER");

    tableConfigurations[NET_SUMMARY_TABLE_CONTROL_JITTER] =
                    StatsDB_NameTypeConfiguration("ControlJitter",
                          "real",
                          "STATS-DB-NETWORK-SUMMARY-CONTROL-JITTER");

}
// Bridge constructor
STAT_GlobalTransportStatisticsBridge::STAT_GlobalTransportStatisticsBridge(
    STAT_GlobalTransportStatistics* globalTransport,
    PartitionData* partition)
{
    Int32 i;
    this->globalTransport =  globalTransport;
    numFixedFields = TRANS_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < TRANS_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// New Destructor to resolve memory leak issue
STAT_GlobalTransportStatisticsBridge::~STAT_GlobalTransportStatisticsBridge()
{
    Int32 i;
    for (i = 0; i < TRANS_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}

// Function to initialize Name and Types of columns of
// Transport Aggregate Table
void STAT_GlobalTransportStatisticsBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[TRANS_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_TIME] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_SENT] = TRUE;
    
    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_SENT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_SENT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("UnicastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_RECV] = TRUE;
    
    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("MulticastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_RECV] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("BroadcastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_RECV] = TRUE;


    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_BYTES_SENT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_BYTES_SENT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_BYTES_SENT] = TRUE;


    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("UnicastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_BYTES_RECV] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("MulticastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_BYTES_RECV] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("BroadcastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_BYTES_RECV] = TRUE;


    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("UnicastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("MulticastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("BroadcastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("UnicastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_THROUGHPUT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("MulticastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_THROUGHPUT] = TRUE;

    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("BroadcastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_THROUGHPUT] = TRUE;

    // now optional fields
    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-DELAY");
    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-DELAY");
    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-DELAY");

    tableConfigurations[TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("UnicastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-JITTER");
    tableConfigurations[TRANS_AGGREGATE_TABLE_MULTI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("MulticastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-JITTER");
    tableConfigurations[TRANS_AGGREGATE_TABLE_BROAD_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-AGGREGATE-JITTER");
}

std::string
STAT_GlobalTransportStatisticsBridge::
           composeGlobalTransportStatisticsInsertSQLString(
    Node* node,
    PartitionData* partition)
{
    Int32 i;
    string combinedQueryForAllAddressTypes = "";
    std::vector<std::string> newValues;
    newValues.reserve(numFixedFields + numOptionalFields);
    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);

    columns.push_back(tableConfigurations[TRANS_AGGREGATE_TABLE_TIME].name);
    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_SENT;
         i < TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
            if (tableConfigurations[i].type == "real"
                || tableConfigurations[i].type == "float")
            {
                newValues.push_back(STATSDB_DoubleToString(
                    valueForIndex(node,i)));
            }            
            else if (tableConfigurations[i].type == "bigint unsigned")
            {
                newValues.push_back(STATSDB_UInt64ToString(
                    valueForIndexInUInt64(node,i)));
            }
        }
    }
    for (i = TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER;
         i < TRANS_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            double val = valueForIndex(node,i);
            if (val > 0)
            {
                columns.push_back(tableConfigurations[i].name);
                newValues.push_back(STATSDB_DoubleToString(val));
            }
        }
    }
    combinedQueryForAllAddressTypes +=
        GetInsertValuesSQL("TRANSPORT_Aggregate", columns, newValues);

    return combinedQueryForAllAddressTypes;
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string
double STAT_GlobalTransportStatisticsBridge::valueForIndex(Node* node,
                                                           Int32 index)
{
    double sum = 0;
    double totalReceived = 0;
    double currTime = (double) getSimTime(node)/ SECOND;

    switch (index)
    {
        case TRANS_AGGREGATE_TABLE_TIME:
            return currTime;        
        case TRANS_AGGREGATE_TABLE_UNI_OFFERLOAD:
            return globalTransport->GetOfferedLoad(STAT_Unicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_MULTI_OFFERLOAD:
            return globalTransport->GetOfferedLoad(STAT_Multicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_BROAD_OFFERLOAD:
            return globalTransport->GetOfferedLoad(STAT_Broadcast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_UNI_THROUGHPUT:
            return globalTransport->GetThroughput(STAT_Unicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_MULTI_THROUGHPUT:
            return globalTransport->GetThroughput(STAT_Multicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_BROAD_THROUGHPUT:
            return globalTransport->GetThroughput(STAT_Broadcast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER:
        {
            if (globalTransport->GetAverageJitter(STAT_Unicast).
                                            GetNumDataPoints() > 0)
            {
                return globalTransport->GetAverageJitter(STAT_Unicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case TRANS_AGGREGATE_TABLE_MULTI_AVG_JITTER:
        {
            if (globalTransport->GetAverageJitter(STAT_Multicast).
                                            GetNumDataPoints() > 0)
            {
                return globalTransport->GetAverageJitter(STAT_Multicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case TRANS_AGGREGATE_TABLE_BROAD_AVG_JITTER:
        {
            if (globalTransport->GetAverageJitter(STAT_Broadcast).
                                            GetNumDataPoints() > 0)
            {
                return globalTransport->GetAverageJitter(STAT_Broadcast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case TRANS_AGGREGATE_TABLE_UNI_AVG_DELAY:
            return globalTransport->GetAverageDelay(STAT_Unicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_MULTI_AVG_DELAY:
            return globalTransport->GetAverageDelay(STAT_Multicast).GetValue(getSimTime(node));
        case TRANS_AGGREGATE_TABLE_BROAD_AVG_DELAY:
            return globalTransport->GetAverageDelay(STAT_Broadcast).GetValue(getSimTime(node));
        default:
            return 0;
    }
}

/* Function to enable/disable Transport Aggregate table column indexes
   in array
*/
Int32 
STAT_GlobalTransportStatisticsBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;
    
    if (db->statsTransAggregate->isDelay[UNICAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransAggregate->isDelay[MULTICAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransAggregate->isDelay[BROADCAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransAggregate->isJitter[UNICAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_UNI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransAggregate->isJitter[MULTICAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_MULTI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransAggregate->isJitter[BROADCAST])
    {
        requiredFieldsIndex[TRANS_AGGREGATE_TABLE_BROAD_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}

UInt64 STAT_GlobalTransportStatisticsBridge::valueForIndexInUInt64(Node* node,
                                                                   Int32 index)
{
    switch (index)
    {
        case TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_SENT:
            return (UInt64)(globalTransport->GetDataSegmentsSent(STAT_Unicast).GetValue(getSimTime(node)) + 
                globalTransport->GetControlSegmentsSent(STAT_Unicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_SENT:
            return (UInt64)(globalTransport->GetDataSegmentsSent(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlSegmentsSent(STAT_Multicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_SENT:
            return (UInt64)(globalTransport->GetDataSegmentsSent(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetControlSegmentsSent(STAT_Broadcast).GetValue(getSimTime(node)));        
        case TRANS_AGGREGATE_TABLE_UNI_SEGMENTS_RECV:
            return (UInt64)(UInt64)(globalTransport->GetDataSegmentsReceived(STAT_Unicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlSegmentsReceived(STAT_Unicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_MULTI_SEGMENTS_RECV:
            return (UInt64)(globalTransport->GetDataSegmentsReceived(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlSegmentsReceived(STAT_Multicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_BROAD_SEGMENTS_RECV:
            return (UInt64)(globalTransport->GetDataSegmentsReceived(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetControlSegmentsReceived(STAT_Broadcast).GetValue(getSimTime(node)));  
        case TRANS_AGGREGATE_TABLE_UNI_BYTES_SENT:
            return (UInt64)(globalTransport->GetDataBytesSent(STAT_Unicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesSent(STAT_Unicast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesSent(STAT_Unicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_MULTI_BYTES_SENT:
            return (UInt64)(globalTransport->GetDataBytesSent(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesSent(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesSent(STAT_Multicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_BROAD_BYTES_SENT:
            return (UInt64)(globalTransport->GetDataBytesSent(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesSent(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesSent(STAT_Broadcast).GetValue(getSimTime(node)));     
        case TRANS_AGGREGATE_TABLE_UNI_BYTES_RECV:
            return (UInt64)(globalTransport->GetDataBytesReceived(STAT_Unicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesReceived(STAT_Unicast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesReceived(STAT_Unicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_MULTI_BYTES_RECV:
            return (UInt64)(globalTransport->GetDataBytesReceived(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesReceived(STAT_Multicast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesReceived(STAT_Multicast).GetValue(getSimTime(node)));
        case TRANS_AGGREGATE_TABLE_BROAD_BYTES_RECV:
            return (UInt64)(globalTransport->GetDataBytesReceived(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetControlBytesReceived(STAT_Broadcast).GetValue(getSimTime(node)) +
                globalTransport->GetOverheadBytesReceived(STAT_Broadcast).GetValue(getSimTime(node)));
        default:
            return 0;
    }
}

// Function to initialize Name and Types of columns of
// Transport Summary Table
void STAT_TransportSummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[TRANS_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_SENDER_ADDR] =
                    StatsDB_NameTypeConfiguration("SenderAddr",
                                                  "VARCHAR(64)",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_SENDER_ADDR] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_RECEIVER_ADDR] =
                    StatsDB_NameTypeConfiguration("ReceiverAddr",
                                                  "VARCHAR(64)",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_RECEIVER_ADDR] = TRUE;


    tableConfigurations[TRANS_SUMMARY_TABLE_SENDER_PORT] =
                    StatsDB_NameTypeConfiguration("SenderPort",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_SENDER_PORT] = TRUE;


    tableConfigurations[TRANS_SUMMARY_TABLE_RECEIVER_PORT] =
                    StatsDB_NameTypeConfiguration("ReceiverPort",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_RECEIVER_PORT] = TRUE;
    

    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_SEGMENTS_SENT] = TRUE;
    
    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_SENT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastSegmentsSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_SENT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("UnicastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_SEGMENTS_RECV] = TRUE;
    
    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("MulticastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_RECV] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_RECV] =
                    StatsDB_NameTypeConfiguration("BroadcastSegmentsRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_RECV] = TRUE;


    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_BYTES_SENT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_BYTES_SENT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_BYTES_SENT] = TRUE;


    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("UnicastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_BYTES_RECV] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("MulticastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_BYTES_RECV] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_BYTES_RECV] =
                    StatsDB_NameTypeConfiguration("BroadcastBytesRcvd",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_BYTES_RECV] = TRUE;


    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("UnicastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("MulticastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("BroadcastOfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_OFFERLOAD] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("UnicastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_THROUGHPUT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("MulticastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_THROUGHPUT] = TRUE;

    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("BroadcastThroughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_THROUGHPUT] = TRUE;

    // now optional fields
    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-DELAY");
    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-DELAY");
    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageDelay",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-DELAY");

    tableConfigurations[TRANS_SUMMARY_TABLE_UNI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("UnicastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-JITTER");
    tableConfigurations[TRANS_SUMMARY_TABLE_MULTI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("MulticastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-JITTER");
    tableConfigurations[TRANS_SUMMARY_TABLE_BROAD_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageJitter",
                          "float",
                          "STATS-DB-TRANSPORT-SUMMARY-JITTER");
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string for TransSummary table
double STAT_TransportSummaryBridge::valueForIndex(Node* node, Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double STAT_TransportSummaryBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_TransportSummaryStatistics& summary)
{
    switch (index)
    {
        case TRANS_SUMMARY_TABLE_SENDER_PORT:
            return summary.sourcePort;
        case TRANS_SUMMARY_TABLE_RECEIVER_PORT:
            return summary.destPort; 
        case TRANS_SUMMARY_TABLE_UNI_OFFERLOAD:
            return summary.m_AddrStats[STAT_Unicast].offeredLoad;
        case TRANS_SUMMARY_TABLE_UNI_THROUGHPUT:
            return summary.m_AddrStats[STAT_Unicast].throughput;
        case TRANS_SUMMARY_TABLE_UNI_AVG_DELAY:
            return summary.m_AddrStats[STAT_Unicast].averageDelay;
        case TRANS_SUMMARY_TABLE_UNI_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Unicast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Unicast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        case TRANS_SUMMARY_TABLE_MULTI_OFFERLOAD:
            return summary.m_AddrStats[STAT_Multicast].offeredLoad;
        case TRANS_SUMMARY_TABLE_MULTI_THROUGHPUT:
            return summary.m_AddrStats[STAT_Multicast].throughput;
        case TRANS_SUMMARY_TABLE_MULTI_AVG_DELAY:
            return summary.m_AddrStats[STAT_Multicast].averageDelay;
        case TRANS_SUMMARY_TABLE_MULTI_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Multicast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Multicast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        case TRANS_SUMMARY_TABLE_BROAD_OFFERLOAD:
            return summary.m_AddrStats[STAT_Broadcast].offeredLoad;
        case TRANS_SUMMARY_TABLE_BROAD_THROUGHPUT:
            return summary.m_AddrStats[STAT_Broadcast].throughput;
        case TRANS_SUMMARY_TABLE_BROAD_AVG_DELAY:
            return summary.m_AddrStats[STAT_Broadcast].averageDelay;
        case TRANS_SUMMARY_TABLE_BROAD_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Broadcast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Broadcast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        default:
            return 0;
    }
}

// Function to compose SQL insertion string for TRANS Summary table
void STAT_TransportSummaryBridge::composeTransportSummaryInsertSQLString(
    Node* node,
    PartitionData* partition,
    std::vector<std::string>* insertList)
{

    Int32 countOfTransportSummary;
    Int32 transportSummarySize = transportSummary->size();
    Int32 i;

    map<STAT_TransportSummaryTag, STAT_TransportSummaryStatistics>
                                                        ::iterator it;
    it = transportSummary->begin();
    
    for (countOfTransportSummary = 0;
        countOfTransportSummary < transportSummarySize;
        countOfTransportSummary++)
    {
        STAT_TransportSummaryStatistics&  temp = it->second;
        ++it;

        std::vector<std::string> columns;
        columns.reserve(numFixedFields + numOptionalFields);
        columns.push_back(tableConfigurations
                                [TRANS_SUMMARY_TABLE_TIME].name);
        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);

        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));

        for (i = TRANS_SUMMARY_TABLE_SENDER_ADDR;
             i < TRANS_SUMMARY_TABLE_UNI_AVG_JITTER;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                columns.push_back(tableConfigurations[i].name);
                if (tableConfigurations[i].type == "real" ||
                    tableConfigurations[i].type == "float" ||
                    tableConfigurations[i].type == "integer")
                {
                    newValues.push_back(STATSDB_DoubleToString(
                        valueForIndex(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "bigint unsigned")
                {
                    newValues.push_back(STATSDB_UInt64ToString(
                        valueForIndexInUInt64(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "VARCHAR(64)")
                {
                    newValues.push_back(
                        valueForIndexInString(node, i, temp));
                }
            }
        }
        for (i = TRANS_SUMMARY_TABLE_UNI_AVG_JITTER;
             i < TRANS_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                double val = valueForIndex(node, i, temp);
                if (val >= 0)
                {
                    columns.push_back(tableConfigurations[i].name);
                    newValues.push_back(STATSDB_DoubleToString(val));
                }
            }
        }
        insertList->push_back(GetInsertValuesSQL(
            "TRANSPORT_Summary", columns, newValues));
    }
}
STAT_TransportSummaryBridge::STAT_TransportSummaryBridge()
{
    this->transportSummary = NULL;
}
// For Trans Summary Table, Bridge constructor
STAT_TransportSummaryBridge::STAT_TransportSummaryBridge(
    STAT_TransportSummarizer* transGlobalSummary,
    PartitionData* partition)
{
    Int32 i;
    this->transportSummary = transGlobalSummary;
    numFixedFields = TRANS_SUMMARY_TABLE_MAX_FIXED_COLUMNS;
    numOptionalFields = 0;

    for (i = 0; i < TRANS_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

STAT_TransportSummaryBridge::~STAT_TransportSummaryBridge()
{
    Int32 i;
    for (i = 0; i < TRANS_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}


// Function to enable/disable Transport Summary table column indexes
// in array.
Int32 STAT_TransportSummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsTransSummary->addrTypes[UNICAST].isDelay)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransSummary->addrTypes[MULTICAST].isDelay)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransSummary->addrTypes[BROADCAST].isDelay)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransSummary->addrTypes[UNICAST].isJitter)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_UNI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransSummary->addrTypes[MULTICAST].isJitter)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_MULTI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsTransSummary->addrTypes[BROADCAST].isJitter)
    {
        requiredFieldsIndex[TRANS_SUMMARY_TABLE_BROAD_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}

std::string STAT_TransportSummaryBridge::valueForIndexInString(
    Node* node,
    Int32 index,
    STAT_TransportSummaryStatistics& summary)
{
    char tempAddr[MAX_STRING_LENGTH];
    std:string tempStr;

    switch(index)
    {
        case TRANS_SUMMARY_TABLE_SENDER_ADDR:
            IO_ConvertIpAddressToString((Address*)&summary.sourceAddr, tempAddr);
            tempStr = tempAddr;
            return tempStr;
        case TRANS_SUMMARY_TABLE_RECEIVER_ADDR:
            IO_ConvertIpAddressToString((Address*)&summary.destAddr, tempAddr);
            tempStr = tempAddr;
            return tempStr;
        default:
            return "";
    }
}

UInt64 STAT_TransportSummaryBridge::valueForIndexInUInt64(
    Node* node,
    Int32 index,
    STAT_TransportSummaryStatistics& summary)
{
    switch (index)
    {
        case TRANS_SUMMARY_TABLE_UNI_SEGMENTS_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Unicast].totalDataSegmentsSent +
                summary.m_AddrStats[STAT_Unicast].totalControlSegmentsSent);
        case TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Multicast].totalDataSegmentsSent +
                summary.m_AddrStats[STAT_Multicast].totalControlSegmentsSent);
        case TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Broadcast].totalDataSegmentsSent +
                summary.m_AddrStats[STAT_Broadcast].totalControlSegmentsSent);
        case TRANS_SUMMARY_TABLE_UNI_SEGMENTS_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Unicast].totalDataSegmentsReceived +
                summary.m_AddrStats[STAT_Unicast].totalControlSegmentsReceived);
        case TRANS_SUMMARY_TABLE_MULTI_SEGMENTS_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Multicast].totalDataSegmentsReceived +
                summary.m_AddrStats[STAT_Multicast].totalControlSegmentsReceived);
        case TRANS_SUMMARY_TABLE_BROAD_SEGMENTS_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Broadcast].totalDataSegmentsReceived +
                summary.m_AddrStats[STAT_Broadcast].totalControlSegmentsReceived);
        case TRANS_SUMMARY_TABLE_UNI_BYTES_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Unicast].totalDataBytesSent +
                summary.m_AddrStats[STAT_Unicast].totalControlBytesSent +
                summary.m_AddrStats[STAT_Unicast].totalOverheadBytesSent);
        case TRANS_SUMMARY_TABLE_MULTI_BYTES_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Multicast].totalDataBytesSent +
                summary.m_AddrStats[STAT_Multicast].totalControlBytesSent +
                summary.m_AddrStats[STAT_Multicast].totalOverheadBytesSent);
        case TRANS_SUMMARY_TABLE_BROAD_BYTES_SENT:
            return (UInt64) (summary.m_AddrStats[STAT_Broadcast].totalDataBytesSent +
                summary.m_AddrStats[STAT_Broadcast].totalControlBytesSent +
                summary.m_AddrStats[STAT_Broadcast].totalOverheadBytesSent);
        case TRANS_SUMMARY_TABLE_UNI_BYTES_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Unicast].totalDataBytesReceived +
                summary.m_AddrStats[STAT_Unicast].totalControlBytesReceived +
                summary.m_AddrStats[STAT_Unicast].totalOverheadBytesReceived);
        case TRANS_SUMMARY_TABLE_MULTI_BYTES_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Multicast].totalDataBytesReceived +
                summary.m_AddrStats[STAT_Multicast].totalControlBytesReceived +
                summary.m_AddrStats[STAT_Multicast].totalOverheadBytesReceived);
        case TRANS_SUMMARY_TABLE_BROAD_BYTES_RECV:
            return (UInt64) (summary.m_AddrStats[STAT_Broadcast].totalDataBytesReceived +
                summary.m_AddrStats[STAT_Broadcast].totalControlBytesReceived +
                summary.m_AddrStats[STAT_Broadcast].totalOverheadBytesReceived);
        default:
            return 0;
    }
}
STAT_GlobalPhysicalStatisticsBridge::STAT_GlobalPhysicalStatisticsBridge(
    STAT_GlobalPhyStatistics* globalPhysical,
    PartitionData * partition)
{
    Int32 i;
    this->globalPhysical =  globalPhysical;
    numFixedFields = PHY_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < PHY_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}
// New Destructor to resolve memory leak issue
STAT_GlobalPhysicalStatisticsBridge::~STAT_GlobalPhysicalStatisticsBridge()
{
    Int32 i;
    for (i = 0; i < PHY_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}
// Function to initialize Name and Types of columns of
// Physical Aggregate Table
void STAT_GlobalPhysicalStatisticsBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[PHY_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_TIME] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_SIGS_TRANS] =
                    StatsDB_NameTypeConfiguration("SignalsTransmitted",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIGS_TRANS] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_SIGS_LOCKED] =
                    StatsDB_NameTypeConfiguration("SignalsLocked",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIGS_LOCKED] = TRUE;



    tableConfigurations[PHY_AGGREGATE_TABLE_SIGS_RECV] =
                    StatsDB_NameTypeConfiguration("SignalsReceived",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIGS_RECV] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_SIGS_DROPPED] =
                    StatsDB_NameTypeConfiguration("SignalsDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIGS_DROPPED] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_SIGS_DROPPED_INTERFERENCE] =
                    StatsDB_NameTypeConfiguration("SignalsDroppedDueToInterference",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIGS_DROPPED_INTERFERENCE] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_UTILIZATION] =
                    StatsDB_NameTypeConfiguration("Utilization",
                                                  "float",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_UTILIZATION] = TRUE;

    tableConfigurations[PHY_AGGREGATE_TABLE_AVG_INTERFERENCE] =
                    StatsDB_NameTypeConfiguration("AverageInterference",
                                                  "float",
                                                  ""); // required field
    requiredFieldsIndex[PHY_AGGREGATE_TABLE_AVG_INTERFERENCE] = TRUE;

    // now optional fields
    tableConfigurations[PHY_AGGREGATE_TABLE_PATHLOSS] =
        StatsDB_NameTypeConfiguration("AveragePathLoss",
              "float",
              "STATS-DB-PHY-AGGREGATE-PATHLOSS");

    tableConfigurations[PHY_AGGREGATE_TABLE_DELAY] =
        StatsDB_NameTypeConfiguration("AverageDelay",
              "float",
              "STATS-DB-PHY-AGGREGATE-DELAY");

    tableConfigurations[PHY_AGGREGATE_TABLE_SIG_POWER] =
        StatsDB_NameTypeConfiguration("AverageSignalPower",
              "float",
              "STATS-DB-PHY-AGGREGATE-SIGNAL-POWER");
}

std::string
STAT_GlobalPhysicalStatisticsBridge::composeGlobalPhysicalStatisticsInsertSQLString(
    Node* node,
    PartitionData * partition)
{
    Int32 i;
    string combinedQueryForAllAddressTypes = "";

    std::vector<std::string> newValues;
    newValues.reserve(25);
    std::vector<std::string> columns;
    columns.reserve(25);

    columns.push_back(tableConfigurations[PHY_AGGREGATE_TABLE_TIME].name);
    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = PHY_AGGREGATE_TABLE_SIGS_TRANS;
         i < PHY_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
            if (tableConfigurations[i].type == "real" ||
                tableConfigurations[i].type == "float")
            {
                newValues.push_back(STATSDB_DoubleToString(valueForIndex(node,i)));
            }            
            else if (tableConfigurations[i].type == "bigint unsigned")
            {
                newValues.push_back(STATSDB_UInt64ToString(valueForIndexInUInt64(node,i)));
            }
        }
    }
    combinedQueryForAllAddressTypes +=
        GetInsertValuesSQL("PHY_Aggregate", columns, newValues);

    return combinedQueryForAllAddressTypes;
}
/* Function to enable/disable Physical Aggregate table column indexes
   in array
*/
Int32 
STAT_GlobalPhysicalStatisticsBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;
    
    if (db->statsPhyAggregate->isAvgPathLoss)
    {
        requiredFieldsIndex[PHY_AGGREGATE_TABLE_PATHLOSS] = TRUE;
        numOptionalFields++;
    }
    if (db->statsPhyAggregate->isAvgDelay)
    {
        requiredFieldsIndex[PHY_AGGREGATE_TABLE_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsPhyAggregate->isAvgSignalPower)
    {
        requiredFieldsIndex[PHY_AGGREGATE_TABLE_SIG_POWER] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}

UInt64 STAT_GlobalPhysicalStatisticsBridge::valueForIndexInUInt64(Node* node,
                                                                  Int32 index)
{
    switch (index)
    {
        case PHY_AGGREGATE_TABLE_SIGS_TRANS:
            return (UInt64) globalPhysical->GetSignalsTransmitted().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_SIGS_LOCKED:
            return (UInt64) globalPhysical->GetSignalsLocked().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_SIGS_RECV:
            return (UInt64) globalPhysical->GetSignalsToMac().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_SIGS_DROPPED:
            return (UInt64) globalPhysical->GetSignalsWithErrors().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_SIGS_DROPPED_INTERFERENCE:
            return (UInt64) globalPhysical->GetSignalsInterferenceErrors().GetValue(getSimTime(node));
    
        default:
            return 0;
    }
}

double STAT_GlobalPhysicalStatisticsBridge::valueForIndex(Node* node,
                                                          Int32 index)
{
    double currTime = (double) getSimTime(node)/ SECOND;

    switch (index)
    {
        case PHY_AGGREGATE_TABLE_TIME:
            return currTime;
        case PHY_AGGREGATE_TABLE_UTILIZATION:
            return globalPhysical->GetAverageUtilization().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_AVG_INTERFERENCE:
            if (globalPhysical->GetAverageInterference_dBm().GetValue(getSimTime(node)) == 0)
            {
                return -120;
            }
            else
            {
                return globalPhysical->GetAverageInterference_dBm().GetValue(getSimTime(node));
            }
        case PHY_AGGREGATE_TABLE_PATHLOSS:
            return globalPhysical->GetAveragePathloss_dB().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_DELAY:
            return globalPhysical->GetAverageDelay().GetValue(getSimTime(node));
        case PHY_AGGREGATE_TABLE_SIG_POWER:
            return globalPhysical->GetAverageSignalPower_dBm().GetValue(getSimTime(node));
    
        default:
            return 0;
    }
}
// For Physical Summary Table, Bridge constructor
STAT_PhySummaryBridge::STAT_PhySummaryBridge(
    STAT_PhySummarizer* phyGlobalSummary,
    PartitionData* partition)
{
    Int32 i;
    this->phySummary = phyGlobalSummary;
    numFixedFields = PHY_SUMMARY_TABLE_MAX_FIXED_COLUMNS;
    numOptionalFields = 0;

    for (i = 0; i < PHY_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}
STAT_PhySummaryBridge::~STAT_PhySummaryBridge()
{
    Int32 i;
    for (i = 0; i < PHY_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}
// Function to enable/disable Phy Summary table column indexes
// in array.
Int32 STAT_PhySummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsPhySummary->isAvgPathLoss)
    {
        requiredFieldsIndex[PHY_SUMMARY_TABLE_PATHLOSS] = TRUE;
        numOptionalFields++;
    }
    if (db->statsPhySummary->isAvgSignalPower)
    {
        requiredFieldsIndex[PHY_SUMMARY_TABLE_SIG_POWER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsPhySummary->isAvgDelay)
    {
        requiredFieldsIndex[PHY_SUMMARY_TABLE_DELAY] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}
// Function to initialize Name and Types of columns of
// Phy Summary Table
void STAT_PhySummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[PHY_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[PHY_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_SENDER_ID] =
                    StatsDB_NameTypeConfiguration("SenderID",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_SENDER_ID] = TRUE;


    tableConfigurations[PHY_SUMMARY_TABLE_RECEIVER_ID] =
                    StatsDB_NameTypeConfiguration("ReceiverID",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_RECEIVER_ID] = TRUE;
    
    tableConfigurations[PHY_SUMMARY_TABLE_CHANNEL_INDEX] =
                    StatsDB_NameTypeConfiguration("ChannelIndex",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_CHANNEL_INDEX] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_PHY_INDEX] =
                    StatsDB_NameTypeConfiguration("PhyIndex",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_PHY_INDEX] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_NUM_RECV_SIGS] =
                    StatsDB_NameTypeConfiguration("NumRcvdSignals",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_NUM_RECV_SIGS] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_NUM_RECV_ERR_SIGS] =
                    StatsDB_NameTypeConfiguration("NumRcvdErrorSignals",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_NUM_RECV_ERR_SIGS] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_UTILIZATION] =
                    StatsDB_NameTypeConfiguration("Utilization",
                                                  "float",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_UTILIZATION] = TRUE;

    tableConfigurations[PHY_SUMMARY_TABLE_AVG_INTERFERENCE] =
                    StatsDB_NameTypeConfiguration("AverageInterference",
                                                  "float",
                                                  "");
    requiredFieldsIndex[PHY_SUMMARY_TABLE_AVG_INTERFERENCE] = TRUE;

    // now optional fields
    tableConfigurations[PHY_SUMMARY_TABLE_PATHLOSS] =
                    StatsDB_NameTypeConfiguration("AveragePathLoss",
                          "real",
                          "STATS-DB-PHY-SUMMARY-PATHLOSS");

    tableConfigurations[PHY_SUMMARY_TABLE_DELAY] =
                    StatsDB_NameTypeConfiguration("AverageDelay",
                          "real",
                          "STATS-DB-PHY-SUMMARY-DELAY");

    tableConfigurations[PHY_SUMMARY_TABLE_SIG_POWER] =
                    StatsDB_NameTypeConfiguration("AverageSignalPower",
                          "real",
                          "STATS-DB-PHY-SUMMARY-SIGNAL-POWER");

}

// Function to compose SQL insertion string for PHY Summary table
void STAT_PhySummaryBridge::composePhysicalSummaryInsertSQLString(
    Node* node,
    PartitionData* partition,
    std::vector<std::string>* insertList)
{
    Int32 countOfPhysicalSummary;
    Int32 phySummarySize = phySummary->size();
    Int32 i;

    map<STAT_PhySummaryTag, STAT_PhySessionStatistics>::iterator it;
    it = phySummary->begin();

    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    columns.push_back(tableConfigurations[PHY_SUMMARY_TABLE_TIME].name);
    
    for (i = PHY_SUMMARY_TABLE_SENDER_ID;
         i < PHY_SUMMARY_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
        }
    }
    for (countOfPhysicalSummary = 0;
        countOfPhysicalSummary < phySummarySize;
        countOfPhysicalSummary++)
    {
        STAT_PhySessionStatistics& temp = it->second;
        ++it;

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);
        
        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));

        for (i = PHY_SUMMARY_TABLE_SENDER_ID;
             i < PHY_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
            {
                if (tableConfigurations[i].type == "real" ||
                    tableConfigurations[i].type == "float" ||
                    tableConfigurations[i].type == "integer")
                {
                    if (i == PHY_SUMMARY_TABLE_PATHLOSS
                        || i == PHY_SUMMARY_TABLE_DELAY
                        || i == PHY_SUMMARY_TABLE_SIG_POWER)
                    {
                        if (temp.numSignals == 0)
                        {
                            std::string str = " ";
                            newValues.push_back(str);
                        }
                        else
                        {
                            newValues.push_back(STATSDB_DoubleToString(
                                                 valueForIndex(node,
                                                               i,
                                                               temp)));
                        }
                    }
                    else
                    {
                        newValues.push_back(STATSDB_DoubleToString(
                                                 valueForIndex(node,
                                                               i,
                                                               temp)));
                    }
                }
                else if (tableConfigurations[i].type == "bigint unsigned")
                {
                    newValues.push_back(STATSDB_UInt64ToString(valueForIndexInUInt64(node, i, temp)));
                }
            }
        }
        insertList->push_back(GetInsertValuesSQL("PHY_Summary", columns, newValues));
    }
}
double STAT_PhySummaryBridge::valueForIndex(Node* node, Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double STAT_PhySummaryBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_PhySessionStatistics& summary)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case PHY_SUMMARY_TABLE_TIME:
            return currTime;
        case PHY_SUMMARY_TABLE_SENDER_ID:
            return summary.senderId; 
        case PHY_SUMMARY_TABLE_RECEIVER_ID:
            return summary.receiverId;
        case PHY_SUMMARY_TABLE_CHANNEL_INDEX:
            return summary.channelIndex;
        case PHY_SUMMARY_TABLE_PHY_INDEX:
            return summary.phyIndex;
        case PHY_SUMMARY_TABLE_UTILIZATION:
            return summary.utilization / getSimTime(node);
        case PHY_SUMMARY_TABLE_AVG_INTERFERENCE:
            {
                if (summary.totalInterference == 0
                    || summary.numSignals == 0)
                {
                    return -120;
                }
                else
                {
                    return IN_DB(summary.totalInterference / summary.numSignals);
                }
            }
        case PHY_SUMMARY_TABLE_PATHLOSS:
            return summary.totalPathLoss / summary.numSignals;
        case PHY_SUMMARY_TABLE_DELAY:
            return summary.totalDelay / summary.numSignals;
        case PHY_SUMMARY_TABLE_SIG_POWER:
            return summary.totalSignalPower / summary.numSignals;
        default:
            return 0;
    }
}
UInt64 STAT_PhySummaryBridge::valueForIndexInUInt64(
    Node* node,
    Int32 index,
    STAT_PhySessionStatistics& summary)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case PHY_SUMMARY_TABLE_NUM_RECV_SIGS:
            return summary.numSignals;
        case PHY_SUMMARY_TABLE_NUM_RECV_ERR_SIGS:
            return summary.numErrorSignals; 
        
        default:
            return 0;
    }
}
// Bridge constructor for MAC Aggregrate
STAT_GlobalMacStatisticsBridge::STAT_GlobalMacStatisticsBridge(
                                    STAT_GlobalMacStatistics* globalMac,
                                    PartitionData * partition)
{
    Int32 i;
    this->globalMac =  globalMac;
    numFixedFields = MAC_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < MAC_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}
/* Function to enable/disable MAC Aggregate table column indexes
   in array
*/
Int32 STAT_GlobalMacStatisticsBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsMacAggregate->addrTypes[0].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[0].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[0].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[0].isAvgJitter)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
 
    if (db->statsMacAggregate->addrTypes[1].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[1].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[1].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[1].isAvgJitter)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }

    if (db->statsMacAggregate->addrTypes[2].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[2].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[2].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacAggregate->addrTypes[2].isAvgJitter)
    {
        requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}
// Function to compose SQL insertion string
std::string STAT_GlobalMacStatisticsBridge::
        composeGlobalMacStatisticsInsertSQLString(
                                                Node* node,
                                                PartitionData* partition)
{
    Int32 i;
    std::vector<std::string> newValues;
    newValues.reserve(25);
    std::vector<std::string> columns;
    columns.reserve(25);

    columns.push_back(tableConfigurations[MAC_AGGREGATE_TABLE_TIME].name);
    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = MAC_AGGREGATE_TABLE_CARRIED_LOAD;
         i < MAC_AGGREGATE_TABLE_UNI_AVG_JITTER;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
            if (tableConfigurations[i].type == "real" ||
                tableConfigurations[i].type == "float")
            {
                newValues.push_back(STATSDB_DoubleToString(
                                    valueForIndex(node,i)));
            }            
            else if (tableConfigurations[i].type == "bigint unsigned")
            {
                newValues.push_back(STATSDB_UInt64ToString(
                                   valueForIndexInUInt64(node,i)));
            }
        }
    }
    for (i = MAC_AGGREGATE_TABLE_UNI_AVG_JITTER;
         i < MAC_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == ""
            || requiredFieldsIndex[i])
        {
            double val = valueForIndex(node,i);
            if (val >= 0)
            {
                columns.push_back(tableConfigurations[i].name);
                newValues.push_back(STATSDB_DoubleToString(val));
            }
        }
    }

    return GetInsertValuesSQL("MAC_Aggregate", columns, newValues);
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string
UInt64 STAT_GlobalMacStatisticsBridge::valueForIndexInUInt64(Node* node,
                                                             Int32 index)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_SENT:
            return (UInt64) globalMac->GetDataFramesSent(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_RCVD:
            return (UInt64) globalMac->GetDataFramesReceived(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_SENT:
            return (UInt64) globalMac->GetControlFramesSent(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_RCVD:
            return (UInt64) globalMac->GetControlFramesReceived(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT:
            return (UInt64) globalMac->GetDataBytesSent(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD:
            return (UInt64) globalMac->GetDataBytesReceived(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_SENT:
            return (UInt64) globalMac->GetControlBytesSent(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_RCVD:
            return (UInt64) globalMac->GetControlBytesReceived(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_SENT:
            return (UInt64) globalMac->GetDataFramesSent(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_RCVD:
            return (UInt64) globalMac->GetDataFramesReceived(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_SENT:
            return (UInt64) globalMac->GetControlFramesSent(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_RCVD:
            return (UInt64) globalMac->GetControlFramesReceived(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT:
            return (UInt64) globalMac->GetDataBytesSent(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD:
            return (UInt64) globalMac->GetDataBytesReceived(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_SENT:
            return (UInt64) globalMac->GetControlBytesSent(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_RCVD:
            return (UInt64) globalMac->GetControlBytesReceived(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_SENT:
            return (UInt64) globalMac->GetDataFramesSent(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_RCVD:
            return (UInt64) globalMac->GetDataFramesReceived(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_SENT:
            return (UInt64) globalMac->GetControlFramesSent(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_RCVD:
            return (UInt64) globalMac->GetControlFramesReceived(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT:
            return (UInt64) globalMac->GetDataBytesSent(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD:
            return (UInt64) globalMac->GetDataBytesReceived(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_SENT:
            return (UInt64) globalMac->GetControlBytesSent(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_RCVD:
            return (UInt64) globalMac->GetControlBytesReceived(STAT_Multicast).GetValue(getSimTime(node));
        default:
            return 0;
    }
}
double STAT_GlobalMacStatisticsBridge::valueForIndex(Node* node,
                                                     Int32 index)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case APP_AGGREGATE_TABLE_TIME:
            return currTime;
        case MAC_AGGREGATE_TABLE_CARRIED_LOAD:
            return globalMac->GetCarriedLoad().GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_AVG_QUEUING_DELAY:
            return globalMac->GetAverageQueuingDelay(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_AVG_MED_ACCESS_DELAY:
            return globalMac->GetAverageMediumAccessDelay(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_AVG_MED_DELAY:
            return globalMac->GetAverageMediumDelay(STAT_Unicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_UNI_AVG_JITTER:
        {
            if (globalMac->GetAverageJitter(STAT_Unicast).
                                            GetNumDataPoints() > 0)
            {
                return globalMac->GetAverageJitter(STAT_Unicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case MAC_AGGREGATE_TABLE_BROAD_AVG_QUEUING_DELAY:
            return globalMac->GetAverageQueuingDelay(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_AVG_MED_ACCESS_DELAY:
            return globalMac->GetAverageMediumAccessDelay(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_AVG_MED_DELAY:
            return globalMac->GetAverageMediumDelay(STAT_Broadcast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_BROAD_AVG_JITTER:
        {
            if (globalMac->GetAverageJitter(STAT_Broadcast).
                                             GetNumDataPoints() > 0)
            {
                return globalMac->GetAverageJitter(STAT_Broadcast).
                                             GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        case MAC_AGGREGATE_TABLE_MULTI_AVG_QUEUING_DELAY:
            return globalMac->GetAverageQueuingDelay(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_AVG_MED_ACCESS_DELAY:
            return globalMac->GetAverageMediumAccessDelay(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_AVG_MED_DELAY:
            return globalMac->GetAverageMediumDelay(STAT_Multicast).GetValue(getSimTime(node));
        case MAC_AGGREGATE_TABLE_MULTI_AVG_JITTER:
        {
            if (globalMac->GetAverageJitter(STAT_Multicast).
                                            GetNumDataPoints() > 0)
            {
                return globalMac->GetAverageJitter(STAT_Multicast).
                                            GetValue(getSimTime(node));
            }
            else
            {
                return -1;
            }
        }
        default:
            return 0;
    }
}

// Function to initialize Name and Types of columns of
// MAC Aggregate Table
void STAT_GlobalMacStatisticsBridge::initializeNameTypeConfigurations()
{
    tableConfigurations[MAC_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_TIME] = TRUE;


    tableConfigurations[MAC_AGGREGATE_TABLE_CARRIED_LOAD] =
                    StatsDB_NameTypeConfiguration("Throughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_CARRIED_LOAD] = TRUE;


    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_CONTROL_FRAMES_RCVD] = TRUE;
    

    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_UNI_CONTROL_BYTES_RCVD] = TRUE;

    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_UNI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("UnicastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-JITTER");
        

    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_CONTROL_FRAMES_RCVD] = TRUE;    

    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_BROAD_CONTROL_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_BROAD_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-JITTER");

    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_CONTROL_FRAMES_RCVD] = TRUE;
    

    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_AGGREGATE_TABLE_MULTI_CONTROL_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-DELAY");
    
    tableConfigurations[MAC_AGGREGATE_TABLE_MULTI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("MulticastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-AGGREGATE-JITTER");
}

// Bridge constructor for MAC Summary
STAT_MacSummaryBridge::STAT_MacSummaryBridge(
                                    STAT_MacSummarizer* summary,
                                    PartitionData * partition)
{
    Int32 i;
    this->macSummary =  summary;
    numFixedFields = MAC_SUMMARY_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < MAC_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}
STAT_MacSummaryBridge::~STAT_MacSummaryBridge()
{
    Int32 i;
    for (i = 0; i < MAC_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}
/* Function to enable/disable MAC Aggregate table column indexes
   in array
*/
Int32 STAT_MacSummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsMacSummary->addrTypes[0].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[0].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[0].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[0].isAvgJitter)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
 
    if (db->statsMacSummary->addrTypes[1].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[1].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[1].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[1].isAvgJitter)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }

    if (db->statsMacSummary->addrTypes[2].isAvgQueuingDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_AVG_QUEUING_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[2].isAvgMediumAccessDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_AVG_MED_ACCESS_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[2].isAvgMediumDelay)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_AVG_MED_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMacSummary->addrTypes[2].isAvgJitter)
    {
        requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    return numFixedFields + numOptionalFields;
}
// Function to compose SQL insertion string
void STAT_MacSummaryBridge::composeMacSummaryInsertSQLString(
    Node* node,
    PartitionData* partition,
    std::vector<std::string>* insertList)
{
    Int32 countOfMacSummary;
    Int32 macSummarySize = macSummary->size();
    Int32 i;

    map<STAT_MacSummaryTag, STAT_MacSummaryStatistics>::iterator it;
    it = macSummary->begin();
    
    for (countOfMacSummary = 0;
        countOfMacSummary < macSummarySize;
        countOfMacSummary++)
    {
        STAT_MacSummaryStatistics& temp = it->second;
        ++it;

        std::vector<std::string> columns;
        columns.reserve(numFixedFields + numOptionalFields);
        columns.push_back(tableConfigurations[MAC_SUMMARY_TABLE_TIME].name);

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);
        
        double timeVal = (double) getSimTime(node) / SECOND;        
        newValues.push_back(STATSDB_DoubleToString(timeVal));

        for (i = MAC_SUMMARY_TABLE_SENDER_ID;
             i < MAC_SUMMARY_TABLE_UNI_AVG_JITTER;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                columns.push_back(tableConfigurations[i].name);
                if (tableConfigurations[i].type == "real" ||
                    tableConfigurations[i].type == "float")
                {
                    newValues.push_back(STATSDB_DoubleToString(
                                        valueForIndex(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "bigint unsigned")
                {
                    newValues.push_back(STATSDB_UInt64ToString(
                                        valueForIndexInUInt64(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "int")
                {
                    newValues.push_back(STATSDB_IntToString(
                                        valueForIndexInInt(node, i, temp)));
                }
            }
        }
        for (i = MAC_SUMMARY_TABLE_UNI_AVG_JITTER;
             i < MAC_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == ""
                || requiredFieldsIndex[i])
            {
                double val = valueForIndex(node, i, temp);
                if (val >= 0)
                {
                    columns.push_back(tableConfigurations[i].name);
                    newValues.push_back(STATSDB_DoubleToString(val));
                }
            }           
        }
        insertList->push_back(GetInsertValuesSQL(
                                    "MAC_Summary", columns, newValues));
    }
}

// Function to initialize Name and Types of columns of
// MAC Summary Table
void STAT_MacSummaryBridge::initializeNameTypeConfigurations()
{
    tableConfigurations[MAC_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[MAC_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_SENDER_ID] =
                    StatsDB_NameTypeConfiguration("SenderId",
                                                  "int",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_SENDER_ID] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_RECEIVER_ID] =
                    StatsDB_NameTypeConfiguration("ReceiverId",
                                                  "int",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_RECEIVER_ID] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_INTERFACE_INDEX] =
                    StatsDB_NameTypeConfiguration("InterfaceIndex",
                                                  "int",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_INTERFACE_INDEX] = TRUE;


    tableConfigurations[MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_RCVD] = TRUE;
    

    tableConfigurations[MAC_SUMMARY_TABLE_UNI_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("UnicastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("UnicastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_RCVD] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_UNI_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("UnicastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_UNI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("UnicastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-JITTER");
        

    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_RCVD] = TRUE;    

    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BroadcastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BroadcastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_BROAD_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("BroadcastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-JITTER");

    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastDataFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_SENT] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastDataFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastControlFramesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastControlFramesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_RCVD] = TRUE;
    

    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastDataBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastDataBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("MulticastControlBytesSent",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_SENT] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("MulticastControlBytesReceived",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_RCVD] = TRUE;
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_AVG_QUEUING_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageQueuingDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_AVG_MED_ACCESS_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageMediumAccessDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_AVG_MED_DELAY] =
                    StatsDB_NameTypeConfiguration("MulticastAverageMediumDelay",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-DELAY");
    
    tableConfigurations[MAC_SUMMARY_TABLE_MULTI_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("MulticastAverageJitter",
                                                  "float",
                                                  "STATS-DB-MAC-SUMMARY-JITTER");

    tableConfigurations[MAC_SUMMARY_TABLE_FRAMES_DROP_SENDER] =
                    StatsDB_NameTypeConfiguration("FramesDroppedSender",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_FRAMES_DROP_SENDER] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_FRAMES_DROP_RECEIVER] =
                    StatsDB_NameTypeConfiguration("FramesDroppedReceiver",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_FRAMES_DROP_RECEIVER] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_BYTES_DROP_SENDER] =
                    StatsDB_NameTypeConfiguration("BytesDroppedSender",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BYTES_DROP_SENDER] = TRUE;

    tableConfigurations[MAC_SUMMARY_TABLE_BYTES_DROP_RECEIVER] =
                    StatsDB_NameTypeConfiguration("BytesDroppedReceiver",
                                                  "bigint unsigned",
                                                  "");
    requiredFieldsIndex[MAC_SUMMARY_TABLE_BYTES_DROP_RECEIVER] = TRUE;
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string for MacSummary table

double
STAT_MacSummaryBridge::valueForIndex(Node* node,
                                     Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}
Int32 
STAT_MacSummaryBridge::valueForIndexInInt(
    Node* node,
    Int32 index,
    STAT_MacSummaryStatistics& summary)
{
    switch (index)
    {
        case MAC_SUMMARY_TABLE_SENDER_ID:
            return summary.senderId;
        case MAC_SUMMARY_TABLE_RECEIVER_ID:
            return summary.receiverId;
        case MAC_SUMMARY_TABLE_INTERFACE_INDEX:
            return summary.interfaceIndex;
        default:
            return 0;
    }
}
double 
STAT_MacSummaryBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_MacSummaryStatistics& summary)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case MAC_SUMMARY_TABLE_TIME:
            return currTime;
        case MAC_SUMMARY_TABLE_UNI_AVG_QUEUING_DELAY:
            return summary.m_AddrStats[STAT_Unicast].averageQueuingDelay;
        case MAC_SUMMARY_TABLE_UNI_AVG_MED_ACCESS_DELAY:
            return summary.m_AddrStats[STAT_Unicast].averageMediumAccessDelay;
        case MAC_SUMMARY_TABLE_UNI_AVG_MED_DELAY:
            return summary.m_AddrStats[STAT_Unicast].averageMediumDelay;
        case MAC_SUMMARY_TABLE_UNI_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Unicast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Unicast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        case MAC_SUMMARY_TABLE_BROAD_AVG_QUEUING_DELAY:
            return summary.m_AddrStats[STAT_Broadcast].averageQueuingDelay;
        case MAC_SUMMARY_TABLE_BROAD_AVG_MED_ACCESS_DELAY:
            return summary.m_AddrStats[STAT_Broadcast].averageMediumAccessDelay;
        case MAC_SUMMARY_TABLE_BROAD_AVG_MED_DELAY:
            return summary.m_AddrStats[STAT_Broadcast].averageMediumDelay;
        case MAC_SUMMARY_TABLE_BROAD_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Broadcast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Broadcast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        case MAC_SUMMARY_TABLE_MULTI_AVG_QUEUING_DELAY:
            return summary.m_AddrStats[STAT_Multicast].averageQueuingDelay;
        case MAC_SUMMARY_TABLE_MULTI_AVG_MED_ACCESS_DELAY:
            return summary.m_AddrStats[STAT_Multicast].averageMediumAccessDelay;
        case MAC_SUMMARY_TABLE_MULTI_AVG_MED_DELAY:
            return summary.m_AddrStats[STAT_Multicast].averageMediumDelay;
        case MAC_SUMMARY_TABLE_MULTI_AVG_JITTER:
        {
            if (summary.m_AddrStats[STAT_Multicast].numJitterDataPoints > 0)
            {
                return summary.m_AddrStats[STAT_Multicast].averageJitter;
            }
            else
            {
                return -1;
            }
        }
        default:
            return 0;
    }
}

UInt64 
STAT_MacSummaryBridge::valueForIndexInUInt64(
    Node* node,
    Int32 index,
    STAT_MacSummaryStatistics& summary)
{
    switch (index)
    {
        case MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalDataFramesSent;
        case MAC_SUMMARY_TABLE_UNI_DATA_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalDataFramesReceived;
        case MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalControlFramesSent;
        case MAC_SUMMARY_TABLE_UNI_CONTROL_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalControlFramesReceived;
        case MAC_SUMMARY_TABLE_UNI_DATA_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalDataBytesSent;
        case MAC_SUMMARY_TABLE_UNI_DATA_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalDataBytesReceived;
        case MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalControlBytesSent;
        case MAC_SUMMARY_TABLE_UNI_CONTROL_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Unicast].totalControlBytesReceived;
        case MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalDataFramesSent;
        case MAC_SUMMARY_TABLE_BROAD_DATA_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalDataFramesReceived;
        case MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalControlFramesSent;
        case MAC_SUMMARY_TABLE_BROAD_CONTROL_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalControlFramesReceived;
        case MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalDataBytesSent;
        case MAC_SUMMARY_TABLE_BROAD_DATA_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalDataBytesReceived;
        case MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalControlBytesSent;
        case MAC_SUMMARY_TABLE_BROAD_CONTROL_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Broadcast].totalControlBytesReceived;
        case MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalDataFramesSent;
        case MAC_SUMMARY_TABLE_MULTI_DATA_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalDataFramesReceived;
        case MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalControlFramesSent;
        case MAC_SUMMARY_TABLE_MULTI_CONTROL_FRAMES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalControlFramesReceived;
        case MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalDataBytesSent;
        case MAC_SUMMARY_TABLE_MULTI_DATA_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalDataBytesReceived;
        case MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_SENT:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalControlBytesSent;
        case MAC_SUMMARY_TABLE_MULTI_CONTROL_BYTES_RCVD:
            return (UInt64) summary.m_AddrStats[STAT_Multicast].totalControlBytesReceived;
        case MAC_SUMMARY_TABLE_FRAMES_DROP_SENDER:
            return (UInt64) summary.totalFramesDroppedSender;
        case MAC_SUMMARY_TABLE_FRAMES_DROP_RECEIVER:
            return (UInt64) summary.totalFramesDroppedReceiver;
        case MAC_SUMMARY_TABLE_BYTES_DROP_SENDER:
            return (UInt64) summary.totalBytesDroppedSender;
        case MAC_SUMMARY_TABLE_BYTES_DROP_RECEIVER:
            return (UInt64) summary.totalBytesDroppedReceiver;
        default:
            return 0;
    }
}

STAT_GlobalQueueStatisticsBridge::STAT_GlobalQueueStatisticsBridge(
    STAT_GlobalQueueStatistics* globalQueue,
    PartitionData * partition)
{
    Int32 i;
    this->globalQueue =  globalQueue;
    numFixedFields = QUEUE_AGGREGATE_TABLE_MAX_FIXED_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < QUEUE_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    
    initializeNameTypeConfigurations();
}
// New Destructor to resolve memory leak issue
STAT_GlobalQueueStatisticsBridge::~STAT_GlobalQueueStatisticsBridge()
{
    Int32 i;
    for (i = 0; i < QUEUE_AGGREGATE_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}
UInt64 STAT_GlobalQueueStatisticsBridge::valueForIndexInUInt64(Node* node,
                                                               Int32 index)
{    
    switch (index)
    {
        case QUEUE_AGGREGATE_PACKETS_ENQUEUED:
            return (UInt64) globalQueue->GetPacketsEnqueued().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_PACKETS_DEQUEUED:
            return (UInt64) globalQueue->GetPacketsDequeued().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_PACKETS_DROPPED:
            return (UInt64) globalQueue->GetPacketsDropped().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_PACKETS_DROPPED_FORCE:
            return (UInt64) globalQueue->GetPacketsDroppedForcefully().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_BYTES_ENQUEUED:
            return (UInt64) globalQueue->GetBytesEnqueued().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_BYTES_DEQUEUED:
            return (UInt64) globalQueue->GetBytesDequeued().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_BYTES_DROPPED:
            return (UInt64) globalQueue->GetBytesDropped().GetValue(getSimTime(node));
        case QUEUE_AGGREGATE_BYTES_DROPPED_FORCE:
            return (UInt64) globalQueue->GetBytesDroppedForcefully().GetValue(getSimTime(node));
        default:
            return 0;
    }
}
double STAT_GlobalQueueStatisticsBridge::valueForIndex(Node* node,
                                                       Int32 index)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case QUEUE_AGGREGATE_TABLE_TIME:
            return currTime;
    }
    return 0;
}

std::string
STAT_GlobalQueueStatisticsBridge::composeGlobalQueueStatisticsInsertSQLString(
    Node* node,
    PartitionData * partition)
{
    Int32 i;
    std::vector<std::string> newValues;
    newValues.reserve(9);
    std::vector<std::string> columns;
    columns.reserve(9);

    columns.push_back(tableConfigurations[QUEUE_AGGREGATE_TABLE_TIME].name);
    double timeVal = (double) getSimTime(node) / SECOND;
    newValues.push_back(STATSDB_DoubleToString(timeVal));

    for (i = QUEUE_AGGREGATE_PACKETS_ENQUEUED;
         i < QUEUE_AGGREGATE_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {            
            if (tableConfigurations[i].type == "bigint unsigned")
            {
                columns.push_back(tableConfigurations[i].name);
                newValues.push_back(STATSDB_UInt64ToString(valueForIndexInUInt64(node,i)));
            }
        }
    }
    return GetInsertValuesSQL("QUEUE_Aggregate", columns, newValues);
}

// Function to initialize Name and Types of columns of
// QUEUE Aggregate Table
void STAT_GlobalQueueStatisticsBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[QUEUE_AGGREGATE_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_TABLE_TIME] = TRUE;

    tableConfigurations[QUEUE_AGGREGATE_PACKETS_ENQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsEnqueued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_PACKETS_ENQUEUED] = TRUE;

    tableConfigurations[QUEUE_AGGREGATE_PACKETS_DEQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsDequeued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_PACKETS_DEQUEUED] = TRUE;
    
    tableConfigurations[QUEUE_AGGREGATE_PACKETS_DROPPED] =
                    StatsDB_NameTypeConfiguration("PacketsDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_PACKETS_DROPPED] = TRUE;
    
    tableConfigurations[QUEUE_AGGREGATE_PACKETS_DROPPED_FORCE] =
                    StatsDB_NameTypeConfiguration("PacketsDroppedForcefully",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_PACKETS_DROPPED_FORCE] = TRUE;
    
    tableConfigurations[QUEUE_AGGREGATE_BYTES_ENQUEUED] =
                    StatsDB_NameTypeConfiguration("BytesEnqueued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_BYTES_ENQUEUED] = TRUE;

    tableConfigurations[QUEUE_AGGREGATE_BYTES_DEQUEUED] =
                    StatsDB_NameTypeConfiguration("BytesDequeued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_BYTES_DEQUEUED] = TRUE;
    
    tableConfigurations[QUEUE_AGGREGATE_BYTES_DROPPED] =
                    StatsDB_NameTypeConfiguration("BytesDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_BYTES_DROPPED] = TRUE;
    
    tableConfigurations[QUEUE_AGGREGATE_BYTES_DROPPED_FORCE] =
                    StatsDB_NameTypeConfiguration("BytesDroppedForcefully",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_AGGREGATE_BYTES_DROPPED_FORCE] = TRUE;
    
}
// Function to enable/disable Queue Aggregate table column indexes
// in array.
Int32 STAT_GlobalQueueStatisticsBridge::numFields(PartitionData* partition)
{
    return numFixedFields + numOptionalFields;
}
// Bridge constructor for Queue Summary
STAT_QueueSummaryBridge::STAT_QueueSummaryBridge(
    STAT_QueueSummarizer* summary,
    PartitionData * partition)
{
    Int32 i;
    this->queueSummary =  summary;
    numFixedFields = QUEUE_SUMMARY_TABLE_MAX_COLUMNS;

    numOptionalFields = 0;
    for (i = 0; i < QUEUE_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    initializeNameTypeConfigurations();
}
STAT_QueueSummaryBridge::~STAT_QueueSummaryBridge()
{
    Int32 i;
    for (i = 0; i < QUEUE_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}

double STAT_QueueSummaryBridge::valueForIndex(Node* node, Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double STAT_QueueSummaryBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    double currTime = (double) getSimTime(node)/ SECOND;
    switch (index)
    {
        case QUEUE_SUMMARY_TABLE_TIME:
            return currTime;
        case QUEUE_SUMMARY_AVG_LENGTH:
            return summary.averageQueueLength;
        case QUEUE_SUMMARY_AVG_TIME_IN_QUEUE:
            {
                if (summary.totalPacketsDequeued > 0 ||
                    summary.totalPacketsDroppedForcefully > 0 ||
                    summary.totalPacketsDroppedForcefullyFromAging > 0)
                {
                    return summary.totalDelays /
                        (summary.totalPacketsDequeued +
                        summary.totalPacketsDroppedForcefully +
                        summary.totalPacketsDroppedForcefullyFromAging);
                }
                else
                {
                    return 0;
                }
            }
        case QUEUE_SUMMARY_AVG_FREE_SPACE:
            return (summary.queueSize - summary.averageQueueLength);
        case QUEUE_SUMMARY_LONG_TIME_IN_QUEUE:
            return summary.longestTimeInQueue;
        default:
            return 0;
    }
}
std::string STAT_QueueSummaryBridge::valueForIndexAsString(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_SUMMARY_QUEUE_POSITION:
            return summary.queuePosition;
        case QUEUE_SUMMARY_QUEUE_TYPE:
            return summary.queueType;
        default:
            return 0;
    }
}
Int32 STAT_QueueSummaryBridge::valueForIndexInInt(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_SUMMARY_NODE_ID:
            return summary.nodeId;
        case QUEUE_SUMMARY_QUEUE_INDEX:
            return summary.queueIndex;
        default:
            return 0;
    }
}
UInt64 STAT_QueueSummaryBridge::valueForIndexInUInt64(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_SUMMARY_PACKETS_ENQUEUED:
            return summary.totalPacketsEnqueued;
        case QUEUE_SUMMARY_PACKETS_DEQUEUED:
            return summary.totalPacketsDequeued;
        case QUEUE_SUMMARY_PACKETS_DROPPED:
            return summary.totalPacketsDropped;
        case QUEUE_SUMMARY_PACKETS_DROPPED_FORCE:
            return summary.totalPacketsDroppedForcefully;
        case QUEUE_SUMMARY_BYTES_ENQUEUED:
            return summary.totalBytesEnqueued;
        case QUEUE_SUMMARY_BYTES_DEQUEUED:
            return summary.totalBytesDequeued;
        case QUEUE_SUMMARY_BYTES_DROPPED:
            return summary.totalBytesDropped;
        case QUEUE_SUMMARY_BYTES_DROPPED_FORCE:
            return summary.totalBytesDroppedForcefully;
        case QUEUE_SUMMARY_PEAK_LENGTH:
            return summary.peakQueueLength;
        case QUEUE_SUMMARY_MIN_FREE_SPACE:
            return (summary.queueSize - summary.peakQueueLength);
        case QUEUE_SUMMARY_QUEUE_SIZE:
            return summary.queueSize;
        default:
            return 0;
    }
}

void STAT_QueueSummaryBridge::composeQueueSummaryInsertSQLString(
    Node* node,
    PartitionData * partition,
    std::vector<std::string>* insertList)
{
    Int32 countOfQueueSummary;
    Int32 queueSummarySize = queueSummary->size();
    Int32 i;

    map<STAT_QueueSummaryTag, STAT_QueueSummaryStatistics>::iterator it;
    it = queueSummary->begin();

    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    columns.push_back(tableConfigurations[QUEUE_SUMMARY_TABLE_TIME].name);
    columns.push_back(tableConfigurations[QUEUE_SUMMARY_INTERFACE_INDEX].name);
    
    for (i = QUEUE_SUMMARY_QUEUE_INDEX;
         i < QUEUE_SUMMARY_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
        }
    }

    for (countOfQueueSummary = 0;
        countOfQueueSummary < queueSummarySize;
        countOfQueueSummary++)
    {
        STAT_QueueSummaryStatistics& temp = it->second;
        ++it;

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);
        
        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));      
        newValues.push_back(STATSDB_InterfaceToString(temp.interfaceIndex));
        
        for (i = QUEUE_SUMMARY_QUEUE_INDEX;
             i < QUEUE_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
            {                
                if (tableConfigurations[i].type == "real")
                {
                    newValues.push_back(STATSDB_DoubleToString(valueForIndex(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "bigint unsigned")
                {
                    newValues.push_back(STATSDB_UInt64ToString(valueForIndexInUInt64(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "integer")
                {
                    newValues.push_back(STATSDB_IntToString(valueForIndexInInt(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "string")
                {
                    newValues.push_back(valueForIndexAsString(node, i, temp));
                }
            }          
        }
        insertList->push_back(GetInsertValuesSQL("QUEUE_Summary", columns, newValues));
    }
}



// Function to initialize Name and Types of columns of
// Queue Summary Table

void STAT_QueueSummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[QUEUE_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_NODE_ID] =
                    StatsDB_NameTypeConfiguration("NodeId",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_NODE_ID] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_INTERFACE_INDEX] =
                    StatsDB_NameTypeConfiguration("InterfaceIndex",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_INTERFACE_INDEX] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_QUEUE_POSITION] =
                    StatsDB_NameTypeConfiguration("QueuePosition",
                                                  "string",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_QUEUE_POSITION] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_QUEUE_INDEX] =
                    StatsDB_NameTypeConfiguration("QueueIndex",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_QUEUE_INDEX] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_QUEUE_TYPE] =
                    StatsDB_NameTypeConfiguration("QueueType",
                                                  "string",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_QUEUE_TYPE] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_QUEUE_SIZE] =
                    StatsDB_NameTypeConfiguration("QueueSize",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_QUEUE_SIZE] = TRUE;


    tableConfigurations[QUEUE_SUMMARY_PACKETS_ENQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsEnqueued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_PACKETS_ENQUEUED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_PACKETS_DEQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsDequeued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_PACKETS_DEQUEUED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_PACKETS_DROPPED] =
                    StatsDB_NameTypeConfiguration("PacketsDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_PACKETS_DROPPED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_PACKETS_DROPPED_FORCE] =
                    StatsDB_NameTypeConfiguration("PacketsDroppedForcefully",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_PACKETS_DROPPED_FORCE] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_BYTES_ENQUEUED] =
                    StatsDB_NameTypeConfiguration("BytesEnqueued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_BYTES_ENQUEUED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_BYTES_DEQUEUED] =
                    StatsDB_NameTypeConfiguration("BytesDequeued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_BYTES_DEQUEUED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_BYTES_DROPPED] =
                    StatsDB_NameTypeConfiguration("BytesDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_BYTES_DROPPED] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_BYTES_DROPPED_FORCE] =
                    StatsDB_NameTypeConfiguration("BytesDroppedForcefully",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_BYTES_DROPPED_FORCE] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_PEAK_LENGTH] =
                    StatsDB_NameTypeConfiguration("PeakQueueLength",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_PEAK_LENGTH] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_LONG_TIME_IN_QUEUE] =
                    StatsDB_NameTypeConfiguration("LongestTimeInQueue",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_LONG_TIME_IN_QUEUE] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_MIN_FREE_SPACE] =
                    StatsDB_NameTypeConfiguration("MinimumFreeSpace",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_MIN_FREE_SPACE] = TRUE;


    tableConfigurations[QUEUE_SUMMARY_AVG_LENGTH] =
                    StatsDB_NameTypeConfiguration("AverageQueueLength",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_AVG_LENGTH] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_AVG_TIME_IN_QUEUE] =
                    StatsDB_NameTypeConfiguration("AverageTimeInQueue",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_AVG_TIME_IN_QUEUE] = TRUE;

    tableConfigurations[QUEUE_SUMMARY_AVG_FREE_SPACE] =
                    StatsDB_NameTypeConfiguration("AverageFreeSpace",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_SUMMARY_AVG_FREE_SPACE] = TRUE;
}
// Function to enable/disable Queue Summary table column indexes
// in array.

Int32 STAT_QueueSummaryBridge::numFields(PartitionData* partition)
{
    return numFixedFields + numOptionalFields;
}

// Function to initialize Name and Types of columns of
// Queue Status Table

void STAT_QueueStatusBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[QUEUE_STATUS_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_TABLE_TIME] = TRUE;

    tableConfigurations[QUEUE_STATUS_NODE_ID] =
                    StatsDB_NameTypeConfiguration("NodeId",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_NODE_ID] = TRUE;

    tableConfigurations[QUEUE_STATUS_INTERFACE_INDEX] =
                    StatsDB_NameTypeConfiguration("InterfaceIndex",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_INTERFACE_INDEX] = TRUE;

    tableConfigurations[QUEUE_STATUS_QUEUE_POSITION] =
                    StatsDB_NameTypeConfiguration("QueuePosition",
                                                  "string",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_QUEUE_POSITION] = TRUE;

    tableConfigurations[QUEUE_STATUS_QUEUE_INDEX] =
                    StatsDB_NameTypeConfiguration("QueueIndex",
                                                  "integer",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_QUEUE_INDEX] = TRUE;

    tableConfigurations[QUEUE_STATUS_PACKETS_ENQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsEnqueued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_PACKETS_ENQUEUED] = TRUE;

    tableConfigurations[QUEUE_STATUS_PACKETS_DEQUEUED] =
                    StatsDB_NameTypeConfiguration("PacketsDequeued",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_PACKETS_DEQUEUED] = TRUE;

    tableConfigurations[QUEUE_STATUS_PACKETS_DROPPED] =
                    StatsDB_NameTypeConfiguration("PacketsDropped",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_PACKETS_DROPPED] = TRUE;

    tableConfigurations[QUEUE_STATUS_PACKETS_DROPPED_FORCE] =
                    StatsDB_NameTypeConfiguration("PacketsDroppedForcefully",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_PACKETS_DROPPED_FORCE] = TRUE;

    tableConfigurations[QUEUE_STATUS_FREE_SPACE] =
                    StatsDB_NameTypeConfiguration("FreeSpace",
                                                  "bigint unsigned",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_FREE_SPACE] = TRUE;

    tableConfigurations[QUEUE_STATUS_AVG_SERVICE_TIME] =
                    StatsDB_NameTypeConfiguration("AverageServiceTime",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[QUEUE_STATUS_AVG_SERVICE_TIME] = TRUE;

}
// Function to enable/disable Queue Status table column indexes
// in array.

Int32 STAT_QueueStatusBridge::numFields(PartitionData* partition)
{
    return numFixedFields + numOptionalFields;
}


void STAT_QueueStatusBridge::composeQueueStatusInsertSQLString(
    Node* node,
    PartitionData* partition,
    std::vector<std::string>* insertList)
{
    Int32 countOfQueueSummary;
    Int32 queueSummarySize = queueSummary->size();
    Int32 i;

    map<STAT_QueueSummaryTag, STAT_QueueSummaryStatistics>::iterator it;
    it = queueSummary->begin();

    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    columns.push_back(tableConfigurations[QUEUE_SUMMARY_TABLE_TIME].name);
    columns.push_back(tableConfigurations[QUEUE_SUMMARY_INTERFACE_INDEX].name);
    
    for (i = QUEUE_STATUS_QUEUE_INDEX;
         i < QUEUE_STATUS_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
        }
    }

    for (countOfQueueSummary = 0;
        countOfQueueSummary < queueSummarySize;
        countOfQueueSummary++)
    {
        STAT_QueueSummaryStatistics& temp = it->second;
        ++it;

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);

        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));        
        newValues.push_back(STATSDB_InterfaceToString(temp.interfaceIndex));

        for (i = QUEUE_STATUS_QUEUE_INDEX;
             i < QUEUE_STATUS_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
            {
                if (tableConfigurations[i].type == "real")
                {
                    newValues.push_back(STATSDB_DoubleToString(valueForIndex(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "bigint unsigned")
                {
                    newValues.push_back(STATSDB_UInt64ToString(valueForIndexInUInt64(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "integer")
                {
                    newValues.push_back(STATSDB_IntToString(valueForIndexInInt(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "string")
                {
                    newValues.push_back(valueForIndexAsString(node, i, temp));
                }
            }
        }
        insertList->push_back(GetInsertValuesSQL("QUEUE_Status", columns, newValues));
    }
}

UInt64 STAT_QueueStatusBridge::valueForIndexInUInt64(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_STATUS_PACKETS_ENQUEUED:
            return summary.totalPacketsEnqueued;
        case QUEUE_STATUS_PACKETS_DEQUEUED:
            return summary.totalPacketsDequeued;
        case QUEUE_STATUS_PACKETS_DROPPED:
            return summary.totalPacketsDropped;
        case QUEUE_STATUS_PACKETS_DROPPED_FORCE:
            return summary.totalPacketsDroppedForcefully;
        case QUEUE_STATUS_FREE_SPACE:
            return summary.curFreeSpace;
        default:
            return 0;
    }
}

double STAT_QueueStatusBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {  
        case QUEUE_STATUS_AVG_SERVICE_TIME:
            {
                if (summary.totalPacketsDequeued > 0 || summary.totalPacketsDroppedForcefully > 0)
                {
                    return summary.totalDelays /
                        (summary.totalPacketsDequeued +
                        summary.totalPacketsDroppedForcefully);
                }
                else
                {
                    return 0;
                }
            }
        default:
            return 0;
    }
}

std::string STAT_QueueStatusBridge::valueForIndexAsString(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_STATUS_QUEUE_POSITION:
            return summary.queuePosition;
        default:
            return 0;
    }
}

Int32 STAT_QueueStatusBridge::valueForIndexInInt(
    Node* node,
    Int32 index,
    STAT_QueueSummaryStatistics& summary)
{
    switch (index)
    {
        case QUEUE_STATUS_QUEUE_INDEX:
            return summary.queueIndex;        
        case QUEUE_STATUS_NODE_ID:
            return summary.nodeId;
        default:
            return 0;
    }
}

// Bridge constructor for Queue Summary
STAT_QueueStatusBridge::STAT_QueueStatusBridge(
    STAT_QueueSummarizer* summary,
    PartitionData * partition)
{
    Int32 i;
    this->queueSummary =  summary;
    numFixedFields = QUEUE_STATUS_TABLE_MAX_COLUMNS;
    numOptionalFields = 0;
    for (i = 0; i < QUEUE_STATUS_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    initializeNameTypeConfigurations();
}

STAT_QueueStatusBridge::~STAT_QueueStatusBridge()
{
    Int32 i;
    for (i = 0; i < QUEUE_STATUS_TABLE_MAX_COLUMNS; i++)
    {
        tableConfigurations[i].name.clear();
        tableConfigurations[i].type.clear();
        tableConfigurations[i].configuration.clear();
    }
}


double STAT_QueueStatusBridge::valueForIndex(Node* node, Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}


Int32 STAT_MulticastAppSummaryBridge::numFields(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db->statsMulticastAppSummary->isDelay)
    {
        requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_AVG_DELAY] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMulticastAppSummary->isJitter)
    {
        requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_AVG_JITTER] = TRUE;
        numOptionalFields++;
    }
    if (db->statsMulticastAppSummary->isHopCount)
    {
        requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_AVG_HOPCOUNT] = TRUE;
        numOptionalFields++;
    }

    return numFixedFields + numOptionalFields;
}

// For MulticastApp Summary Table 
// Bridge constructor
STAT_MulticastAppSummaryBridge::STAT_MulticastAppSummaryBridge(
    STAT_MulticastAppSessionSummarizer* sessionSummary,
    PartitionData* partition)
{
    Int32 i;
    this->sessionSummary = sessionSummary;

    numFixedFields = MULTI_APP_SUMMARY_TABLE_MAX_FIXED_COLUMNS;
    numOptionalFields = 0;

    for (i = 0; i < MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS; i++)
    {
        requiredFieldsIndex[i] = FALSE;
    }
    numFields(partition);
    initializeNameTypeConfigurations();
}

// Function to initialize Name and Types of columns of
// Multicast Application Summary Table
void STAT_MulticastAppSummaryBridge::initializeNameTypeConfigurations()
{
    // first fixed fields
    tableConfigurations[MULTI_APP_SUMMARY_TABLE_TIME] =
                    StatsDB_NameTypeConfiguration("Timestamp",
                                                  "real",
                                                  ""); // required field
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_TIME] = TRUE;

    tableConfigurations[MULTI_APP_SUMMARY_TABLE_SENDER_ID] =
                    StatsDB_NameTypeConfiguration("SenderId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_SENDER_ID] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_RECEIVER_ID] =
                    StatsDB_NameTypeConfiguration("ReceiverId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_RECEIVER_ID] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_RECEIVER_ADDR] =
                    StatsDB_NameTypeConfiguration("ReceiverAddress",
                                                  "string",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_RECEIVER_ADDR] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_SESSION_ID] =
                    StatsDB_NameTypeConfiguration("SessionId",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_SESSION_ID] = TRUE;
    

    tableConfigurations[MULTI_APP_SUMMARY_TABLE_TOS] =
                    StatsDB_NameTypeConfiguration("Tos",
                                                  "integer",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_TOS] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_MSG_SENT] =
                    StatsDB_NameTypeConfiguration("MessagesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_MSG_SENT] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_MSG_RCVD] =
                    StatsDB_NameTypeConfiguration("MessagesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_MSG_RCVD] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_BYTES_SENT] =
                    StatsDB_NameTypeConfiguration("BytesSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_BYTES_SENT] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_BYTES_RCVD] =
                    StatsDB_NameTypeConfiguration("BytesReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_BYTES_RCVD] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_FRAG_SENT] =
                    StatsDB_NameTypeConfiguration("FragSent",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_FRAG_SENT] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_FRAG_RCVD] =
                    StatsDB_NameTypeConfiguration("FragReceived",
                                                  "bigint",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_FRAG_RCVD] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_APP_TYPE] =
                    StatsDB_NameTypeConfiguration("ApplicationType",
                                                  "string",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_APP_TYPE] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_APP_NAME] =
                    StatsDB_NameTypeConfiguration("ApplicationName",
                                                  "string",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_APP_NAME] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_MSG_COMP_RATE] =
                    StatsDB_NameTypeConfiguration("MessageCompletionRate",
                                                  "float",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_MSG_COMP_RATE] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_OFFERLOAD] =
                    StatsDB_NameTypeConfiguration("OfferedLoad",
                                                  "float",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_OFFERLOAD] = TRUE;


    tableConfigurations[MULTI_APP_SUMMARY_TABLE_THROUGHPUT] =
                    StatsDB_NameTypeConfiguration("Throughput",
                                                  "float",
                                                  "");
    requiredFieldsIndex[MULTI_APP_SUMMARY_TABLE_THROUGHPUT] = TRUE;

    // now optional fields
    tableConfigurations[MULTI_APP_SUMMARY_TABLE_AVG_DELAY] =
                    StatsDB_NameTypeConfiguration("Delay",
                          "float",
                          "STATS-DB-MULTICAST-APPLICATION-SUMMARY-DELAY");

    tableConfigurations[MULTI_APP_SUMMARY_TABLE_AVG_JITTER] =
                    StatsDB_NameTypeConfiguration("Jitter",
                          "float",
                          "STATS-DB-MULTICAST-APPLICATION-SUMMARY-JITTER");

    tableConfigurations[MULTI_APP_SUMMARY_TABLE_AVG_HOPCOUNT] =
                    StatsDB_NameTypeConfiguration("HopCount",
                         "float",
                         "STATS-DB-MULTICAST-APPLICATION-SUMMARY-HOP-COUNT");

}


// Function to compose SQL insertion string for MULTICAST-APP Summary table 
void 
STAT_MulticastAppSummaryBridge::composeMutlicastAppSummaryInsertSQLString(
                                        Node* node,
                                        PartitionData* partition,
                                        std::vector<std::string>* insertList)
{    
    Int32 countOfMutlicastSessionSummary;
    Int32 mutlicastSessionSummarySize = sessionSummary->size();
    Int32 i;

    map<STAT_MulticastAppSummaryTag, STAT_MulticastAppSessionSummaryStatistics>::iterator it;
    it = sessionSummary->begin();

    std::vector<std::string> columns;
    columns.reserve(numFixedFields + numOptionalFields);
    columns.push_back(tableConfigurations[MULTI_APP_SUMMARY_TABLE_TIME].name);
    
    for (i = MULTI_APP_SUMMARY_TABLE_SENDER_ID;
         i < MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS;
         i++)
    {
        if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
        {
            columns.push_back(tableConfigurations[i].name);
        }
    }

    for (countOfMutlicastSessionSummary = 0;
        countOfMutlicastSessionSummary < mutlicastSessionSummarySize;
        countOfMutlicastSessionSummary++)
    {
        STAT_MulticastAppSessionSummaryStatistics& temp = it->second;
        ++it;

        std::vector<std::string> newValues;
        newValues.reserve(numFixedFields + numOptionalFields);

        double timeVal = (double) getSimTime(node) / SECOND;
        newValues.push_back(STATSDB_DoubleToString(timeVal));        

        for (i = MULTI_APP_SUMMARY_TABLE_SENDER_ID;
             i < MULTI_APP_SUMMARY_TABLE_MAX_COLUMNS;
             i++)
        {
            if (tableConfigurations[i].configuration == "" || requiredFieldsIndex[i])
            {
                if (tableConfigurations[i].type == "real" || 
                    tableConfigurations[i].type == "float" || 
                    tableConfigurations[i].type == "bigint")
                {
                    newValues.push_back(STATSDB_DoubleToString(valueForIndex(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "integer")
                {
                    newValues.push_back(STATSDB_IntToString(valueForIndexInInteger(node, i, temp)));
                }
                else if (tableConfigurations[i].type == "string")
                {
                    newValues.push_back(valueForIndexInString(node, i, temp));
                }
            }
        }
        insertList->push_back(GetInsertValuesSQL("MULTICAST_APPLICATION_Summary", columns, newValues));
    }
}

// Function to call Stat APIs to get various values to be inserted in
// SQL string for AppSummary table

double
STAT_MulticastAppSummaryBridge::valueForIndex(Node* node,
                                              Int32 index)
{
    // do nothing. It is just to define it
    return 0;
}

double 
STAT_MulticastAppSummaryBridge::valueForIndex(
    Node* node,
    Int32 index,
    STAT_MulticastAppSessionSummaryStatistics& sessionSummary)
{
    switch (index)
    {
        case MULTI_APP_SUMMARY_TABLE_MSG_SENT:
            return sessionSummary.m_messagesSent;
            break;
        case MULTI_APP_SUMMARY_TABLE_MSG_RCVD:
            return sessionSummary.m_messagesReceived;
            break;
        case MULTI_APP_SUMMARY_TABLE_BYTES_SENT:
            return sessionSummary.m_bytesSent;
            break;
        case MULTI_APP_SUMMARY_TABLE_BYTES_RCVD:
            return sessionSummary.m_bytesReceived;
            break;
        case MULTI_APP_SUMMARY_TABLE_FRAG_SENT:
            return sessionSummary.m_fragmentsSent;
            break;
        case MULTI_APP_SUMMARY_TABLE_FRAG_RCVD:
            return sessionSummary.m_fragmentsReceived;
            break;
        case MULTI_APP_SUMMARY_TABLE_MSG_COMP_RATE:
            if (sessionSummary.m_bytesSent != 0)
            {
                return (sessionSummary.m_bytesReceived / sessionSummary.m_bytesSent);
            }
            else
            {
                return 0;
            }
            break;
        case MULTI_APP_SUMMARY_TABLE_OFFERLOAD:
            return sessionSummary.m_offerload;
            break;
        case MULTI_APP_SUMMARY_TABLE_THROUGHPUT:
            return sessionSummary.m_throughput;
            break;
        case MULTI_APP_SUMMARY_TABLE_AVG_DELAY:
            return sessionSummary.m_averageDelay;
            break;
        case MULTI_APP_SUMMARY_TABLE_AVG_JITTER:
            return sessionSummary.m_avgJitter;
            break;
        case MULTI_APP_SUMMARY_TABLE_AVG_HOPCOUNT:
            return sessionSummary.m_avgHopCount;
            break;
        default:
            return 0;
    }
    return 0;
}

std::string 
STAT_MulticastAppSummaryBridge::valueForIndexInString(
    Node* node,
    Int32 index,
    STAT_MulticastAppSessionSummaryStatistics& sessionSummary)
{
    char tempAddr[MAX_STRING_LENGTH];
    std:string tempStr;

    switch(index)
    {
        case MULTI_APP_SUMMARY_TABLE_RECEIVER_ADDR:
            
            sprintf(tempAddr, "%u.%u.%u.%u",
            (sessionSummary.receiverAddress & 0xff000000) >> 24,
            (sessionSummary.receiverAddress & 0xff0000) >> 16,
            (sessionSummary.receiverAddress & 0xff00) >> 8,
            sessionSummary.receiverAddress & 0xff);
            tempStr = tempAddr;
            return tempStr;
            break;
        case MULTI_APP_SUMMARY_TABLE_APP_TYPE:
            return sessionSummary.type;
            break;
        case MULTI_APP_SUMMARY_TABLE_APP_NAME:
            return sessionSummary.name;
            break;
        default:
            return "";
    }
    return "";
}

Int32 
STAT_MulticastAppSummaryBridge::valueForIndexInInteger(
    Node* node,
    Int32 index,
    STAT_MulticastAppSessionSummaryStatistics& sessionSummary)
{
    switch(index)
    {
        case MULTI_APP_SUMMARY_TABLE_SENDER_ID:
            return (Int32)sessionSummary.m_senderId;
            break;
        case MULTI_APP_SUMMARY_TABLE_RECEIVER_ID:
            return (Int32)sessionSummary.m_DestNodeId;
            break;
        case MULTI_APP_SUMMARY_TABLE_SESSION_ID:
            return (Int32)sessionSummary.m_SessionId;
            break;
        case MULTI_APP_SUMMARY_TABLE_TOS:
            return (Int32)sessionSummary.tos;
            break;
        default:
            return 0;
    }
    return 0;
}
