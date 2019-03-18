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

#include <string>
#include <vector>
#include <iostream>

#include "api.h"
#include "partition.h"
#include "WallClock.h"
#include "external_util.h"

#ifdef _WIN32
#include <windows.h>
#endif
#include "mapping.h"
#include "dbapi.h"
#include "db.h"
#include "stats_global.h"
#include "db_statsapi_bridge.h"

#include "db-core.h"

#include "sqlite3.h"
#include "mysqld_error.h"
#include "fileio.h"
#include "network_ip.h"
#include <iomanip>

#ifdef SOCKET_INTERFACE
#include "socket-interface.h"
#endif

#ifdef ADDON_BOEINGFCS
#include "entity.h"
#include "messageapi.h"
#include "network_ces_subnet.h"
#endif

#if defined(STATS_DEBUG)
# undef STATS_DEBUG
#endif /* STATS_DEBUG */

#define STATS_DEBUG (false)

#include "db-interlock.h"

DatabaseInterlock dbInterlock;

const char StatsDBNetworkEventContent::formatString[] =
    "('%f', '%d', '%s', '%s', '%s', '%d', '%s'"
    "%s%s%s%s%s%s%s%s)";

static void FlushQueryBufferStatsDb_WT(StatsDb* db)
{
    dbInterlock.sync();
}

static void AddInsertQueryToBufferStatsDb__WT(StatsDb* db, const std::string &queryStr)
{
    if (STATS_DEBUG)
    {
        printf("Inserting query %s\n", queryStr.c_str());
    }

    dbInterlock.push_back(db, queryStr);
}


static void StatsDbFinalize__WT(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;

    if (db == NULL)
    {
        return;
    }

    if (db->networkEventsBytesUsed)
    {
        //db->networkEventsString+=";";
        strcat(db->networkEventsString, ";");
        AddInsertQueryToBuffer(db, std::string(db->networkEventsString));
    }
    if (db->statsAggregateTable->endSimulation)
    {
        HandleStatsDBAppAggregateInsertion(partition->firstNode);
        HandleStatsDBTransAggregateInsertion(partition->firstNode);
        HandleStatsDBNetworkAggregateInsertion(partition->firstNode);
        HandleStatsDBMacAggregateInsertion(partition->firstNode);
        HandleStatsDBPhyAggregateInsertion(partition->firstNode);
        HandleStatsDBQueueAggregateInsertion(partition->firstNode);
        // protocol specific
        if (db->statsOspfTable != NULL)
        {
            if (db->statsOspfTable->createOspfAggregateTable)
            {
                HandleStatsDBOspfAggregateTableInsertion(
                    partition->firstNode);
            }
        }
#ifdef ADDON_BOEINGFCS
        if (db->statsMalsrTableParams != NULL)
        {
            if (db->statsMalsrTableParams->createMalsrAggregateTable)
            {
                HandleStatsDBMalsrAggregateInsertion(partition->firstNode);
            }
        }
#endif
    }

    if (db->statsOspfTable != NULL)
    {
        if (db->statsOspfTable->createOspfNeighborStateTable)
        {
            HandleStatsDBOspfNeighborStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfInterfaceStateTable)
        {
            HandleStatsDBOspfInterfaceStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfNetworkLsaTable)
        {
            HandleStatsDBOspfNetworkLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfRouterLsaTable)
        {
            HandleStatsDBOspfRouterLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfExternalLsaTable)
        {
            HandleStatsDBOspfExternalLsaTableInsertion(
                partition->firstNode);
        }
    }

    if (db->statsSummaryTable->endSimulation)
    {
        HandleStatsDBAppSummaryInsertion(partition->firstNode);
        HandleStatsDBMulticastAppSummaryInsertion(partition->firstNode);

        //multicast network summary handling. Check if the Table exists.
        if (db->statsSummaryTable->createMulticastNetSummaryTable)
        {
            HandleStatsDBIpMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBPimMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBMospfMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
        }
        //multicast protocol specific calling start
        HandleStatsDBPimSmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBPimDmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBMospfSummaryTableInsertion(partition->firstNode);
        HandleStatsDBIgmpSummaryTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBTransSummaryInsertion(partition->firstNode);
        HandleStatsDBNetworkSummaryInsertion(partition->firstNode);
        HandleStatsDBMacSummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertionForMacProtocols(partition->firstNode);
        HandleStatsDBQueueSummaryInsertion(partition->firstNode);
        if (db->statsOspfTable != NULL)
        {
           if (db->statsOspfTable->createOspfSummaryTable)
            {
                HandleStatsDBOspfSummaryTableInsertion(
                    partition->firstNode);
            }
            if (db->statsOspfTable->createOspfSummaryLsaTable)
            {
                HandleStatsDBOspfSummaryLsaTableInsertion(
                    partition->firstNode);
            }
        }
    }
    if (db->statsStatusTable->endSimulation)
    {
        // nodeStatus table
        Node * nextNode = partition->firstNode;
        while (nextNode)
        {
            StatsDBNodeStatus nodeStatus(nextNode, FALSE);
            // Add this node's status information to the database
            STATSDB_HandleNodeStatusTableInsert(nextNode, nodeStatus);

            STATSDB_HandleInterfaceStatusTableInsert(nextNode, FALSE);

            nextNode = nextNode->nextNodeData;
        }

        // the following functions will handle all nodes on one partition
        STATSDB_HandleMulticastStatusTableInsert(partition->firstNode);
        //multicast protocol specific calling start
        HandleStatsDBPimSmStatusTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBQueueStatusInsertion(partition->firstNode);

#ifdef ADDON_BOEINGFCS
        HandleStatsDBRegionStatusInsertion(partition->firstNode);
        HandleStatsDBRapStatusInsertion(partition->firstNode);
        HandleStatsDBMprSetInsertion(partition->firstNode);
#endif
    }

    if (db->statsConnTable->endSimulation)
    {
        HandleStatsDBAppConnInsertion(partition->firstNode);
        HandleStatsDBTransConnInsertion(partition->firstNode);
        HandleStatsDBNetworkConnInsertion(partition->firstNode);
        HandleStatsDBMacConnInsertion(partition->firstNode);
        StatsDBHandleMulticastConnInsertion(partition->firstNode, NULL);
        HandleStatsDBPhyConnInsertion(partition);
        //protocol specific
#ifdef ADDON_BOEINGFCS
        if (db->statsMalsrTableParams != NULL)
        {
            if (db->statsMalsrTableParams->createMalsrConnTable)
            {
                HandleStatsDBMalsrConnInsertion(partition->firstNode);
            }
        }
#endif
    }

#ifdef ADDON_BOEINGFCS
       StatsDBSrwFinalize(partition);
#endif
    //if (db->createDbFile)
    //{
    //    db->driver->close();
    //}

    //dbInterlock.finalize();

#ifdef ADDON_BOEINGFCS
    //LinkUtilizationFinalize
    std::string str;
    if (db->StatsDBLinkUtilTable != NULL
                 && db->StatsDBLinkUtilTable->createWNWLinkUtilizationTable)
    {
        str = "WNW";
        HandleStatsDBLinkUtilizationPerNodeInsertion(
                        partition->firstNode, &str);
        if (partition->partitionId == 0)
        {
            HandleStatsDBLinkUtilizationInsertion(partition->firstNode,
                            &str);
            DropTable(db, "WNW_Link_Utilization_Per_Node");
        }

    }
    if (db->StatsDBLinkUtilTable != NULL
                && db->StatsDBLinkUtilTable->createNCWLinkUtilizationTable)
    {
        str = "NCW";

        HandleStatsDBLinkUtilizationPerNodeInsertion(
                        partition->firstNode, &str);
        if (partition->partitionId == 0)
        {
            HandleStatsDBLinkUtilizationInsertion(partition->firstNode,
                            &str);
            DropTable(db, "NCW_Link_Utilization_Per_Node");
        }
    }
#endif 
}

static void StatsDbFinalize__WT(void)
{
    dbInterlock.finalize();
}
static void FlushQueryBufferStatsDb_ST(StatsDb* db)
{
    if (db->engineType == UTIL::Database::dbSqlite)
    {
        if (!db->queryBuffer.empty())
        {
            std::string tempStr = "BEGIN EXCLUSIVE;";
            tempStr += db->queryBuffer;
            tempStr += "COMMIT;";

            db->driver->exec(tempStr);
            db->queryBuffer.clear();
        }

    }
    else if (db->engineType == UTIL::Database::dbMySQL)
    {
        for (list<std::string>::iterator it = db->buffer.begin(); it != db->buffer.end(); it++)
        {
            db->driver->exec(*it);
        }
        
        db->buffer.clear();
    }
    db->numQueryBuffer = 0;
}
static void AddInsertQueryToBufferStatsDb__ST(StatsDb* db,
                                   const std::string &queryStr)
{
    clocktype start = 0;
    clocktype end = 0;

    db->numQueryBuffer++;
       
    if (db->engineType == UTIL::Database::dbSqlite)
    {               
         // Insert the query into the buffer.
        db->queryBuffer += queryStr;
    }
    else if (db->engineType == UTIL::Database::dbMySQL)
    {
        // Insert the query into the buffer.
        db->buffer.push_back(queryStr);
    }

    if (db->numQueryBuffer < db->minQueryBuffer)
    {
        // Do not insert. Wait
        return;
    }
    
    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }

    FlushQueryBufferStatsDb(db);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
    }
}

static void StatsDbFinalize__ST(PartitionData* partition)
{
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (db->statsConnTable->endSimulation)
    {
        HandleStatsDBPhyConnInsertion(partition);
    }
    if (db->statsAggregateTable->endSimulation)
    {
        HandleStatsDBAppAggregateInsertion(partition->firstNode);
        HandleStatsDBTransAggregateInsertion(partition->firstNode);
        HandleStatsDBNetworkAggregateInsertion(partition->firstNode);
        HandleStatsDBMacAggregateInsertion(partition->firstNode);
        HandleStatsDBPhyAggregateInsertion(partition->firstNode);
        HandleStatsDBQueueAggregateInsertion(partition->firstNode);
        if (db->statsOspfTable != NULL)
        {
            if (db->statsOspfTable->createOspfAggregateTable)
            {
                HandleStatsDBOspfAggregateTableInsertion(
                    partition->firstNode);
            }
        }
    }

    if (db->statsOspfTable != NULL)
    {
        if (db->statsOspfTable->createOspfNeighborStateTable)
        {
            HandleStatsDBOspfNeighborStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfInterfaceStateTable)
        {
            HandleStatsDBOspfInterfaceStateTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfNetworkLsaTable)
        {
            HandleStatsDBOspfNetworkLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfRouterLsaTable)
        {
            HandleStatsDBOspfRouterLsaTableInsertion(
                partition->firstNode);
        }
        if (db->statsOspfTable->createOspfExternalLsaTable)
        {
            HandleStatsDBOspfExternalLsaTableInsertion(
                partition->firstNode);
        }
    }

    if (db->statsSummaryTable->endSimulation)
    {
        HandleStatsDBAppSummaryInsertion(partition->firstNode);
        HandleStatsDBMulticastAppSummaryInsertion(partition->firstNode);
        //multicast network summary handling. Check if the Table exists.
        if (db->statsSummaryTable->createMulticastNetSummaryTable)
        {
            HandleStatsDBIpMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBPimMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
            HandleStatsDBMospfMulticastNetSummaryTableInsertion(
                                                       partition->firstNode);
        }
        //multicast protocol specific calling start
        HandleStatsDBPimSmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBPimDmSummaryTableInsertion(partition->firstNode);
        HandleStatsDBMospfSummaryTableInsertion(partition->firstNode);
        HandleStatsDBIgmpSummaryTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBTransSummaryInsertion(partition->firstNode);
        HandleStatsDBMacSummaryInsertion(partition->firstNode);
        HandleStatsDBNetworkSummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertion(partition->firstNode);
        HandleStatsDBPhySummaryInsertionForMacProtocols(partition->firstNode);
        HandleStatsDBQueueSummaryInsertion(partition->firstNode);
        if (db->statsOspfTable != NULL)
        {
           if (db->statsOspfTable->createOspfSummaryTable)
            {
                HandleStatsDBOspfSummaryTableInsertion(
                    partition->firstNode);
            }
            if (db->statsOspfTable->createOspfSummaryLsaTable)
            {
                HandleStatsDBOspfSummaryLsaTableInsertion(
                    partition->firstNode);
            }
        }
    }

    if (db->statsStatusTable->endSimulation)
    {
        // nodeStatus table
        Node * nextNode = partition->firstNode;
        while (nextNode)
        {
            if (db->statsStatusTable->createNodeStatusTable)
            {
                StatsDBNodeStatus nodeStatus(nextNode, FALSE);
                // Add this node's status information to the database
                STATSDB_HandleNodeStatusTableInsert(nextNode, nodeStatus);
            }

            STATSDB_HandleInterfaceStatusTableInsert(nextNode, FALSE);

            nextNode = nextNode->nextNodeData;
        }

        // the following functions will handle all nodes on one partition
        STATSDB_HandleMulticastStatusTableInsert(partition->firstNode);
        //multicast protocol specific calling start
        HandleStatsDBPimSmStatusTableInsertion(partition->firstNode);
        //multicast protocol specific calling end
        HandleStatsDBQueueStatusInsertion(partition->firstNode);
    }
    if (db->statsConnTable->endSimulation)
    {

        HandleStatsDBAppConnInsertion(partition->firstNode);
        HandleStatsDBTransConnInsertion(partition->firstNode);
        HandleStatsDBNetworkConnInsertion(partition->firstNode);
        HandleStatsDBMacConnInsertion(partition->firstNode);
        StatsDBHandleMulticastConnInsertion(partition->firstNode, NULL) ;

        //Node * nextNode = partition->firstNode;
        //while (nextNode)
        //{
            // Add this node's app connectivity info to the database


          //  nextNode = nextNode->nextNodeData;
        //}
    }
#ifdef ADDON_BOEINGFCS
       StatsDBSrwFinalize(partition);
#endif

    if (db->createDbFile)
    {
        FlushQueryBufferStatsDb(db);
    }
}

static void StatsDbFinalize__ST(void)
{

}

void StatsDbDriverClose(PartitionData* partition)
{

    StatsDb* db = partition->statsDb;
    if (db && db->createDbFile)
    {
        db->driver->close();
    }

}
void StatsDbFinalize(void)
{
    bool useWorkerThreads = UTIL::Database::useWorkerThread();

    if (useWorkerThreads)
    {
        StatsDbFinalize__WT();
    }
    else
    {
        StatsDbFinalize__ST();
    }
}

void
StatsDbFinishEventTablesInsertion(PartitionData* partition)
{
    StatsDb* db = NULL;
    db = partition->statsDb;
    if (db != NULL && db->appEventsString != "")
    {
        db->appEventsString += ";";
        AddInsertQueryToBuffer(db, db->appEventsString);
    }
}



void StatsDbFinalize(PartitionData* partition)
{
    StatsDbFinishEventTablesInsertion(partition);

    bool useWorkerThreads = UTIL::Database::useWorkerThread();

    if (useWorkerThreads)
    {
        StatsDbFinalize__WT(partition);
    }
    else
    {
        StatsDbFinalize__ST(partition);
    }
}

void AddInsertQueryToBufferStatsDb(StatsDb* db,
                        const std::string &queryStr)
{
    bool useWorkerThreads = UTIL::Database::useWorkerThread();
    if (queryStr == "") { return; }

    if (useWorkerThreads)
    {
        AddInsertQueryToBufferStatsDb__WT(db, queryStr);
    }
    else
    {
        AddInsertQueryToBufferStatsDb__ST(db, queryStr);
    }
}

void FlushQueryBufferStatsDb(StatsDb* db)
{
    bool useWorkerThreads = UTIL::Database::useWorkerThread();

    if (useWorkerThreads)
    {
        FlushQueryBufferStatsDb_WT(db);
    }
    else
    {
        FlushQueryBufferStatsDb_ST(db);
    }
}

static Int32 callback(void* NotUsed,
                      Int32 argc,
                      char** argv,
                      char** azColName)
{
    Int32 print = 0;
    Int32 i = 0;
    if (!print)
    {
        printf("%s",azColName[0]);
        for (i = 1; i < argc; i++)
        {
            printf("%20s", azColName[i]);
        }
        printf("\n");
    }

    /* Iterate throw result rows */
    printf("%s",argv[0] ? argv[0] : "NULL");
    for (i = 1; i < argc; i++)
    {
        //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
        printf("%17s", argv[i] ? argv[i] : "NULL");
        //printf ();
    }
    printf ("\n");
    return (EXIT_SUCCESS);
}

void InitializePartitionStatsDb(StatsDb* statsDb)
{

    statsDb->createDbFile = FALSE;
    statsDb->driver = NULL;

    statsDb->numQueryBuffer = 0;
    statsDb->maxQueryBuffer = STATSDB_MAX_BUFFER_QUERY;
    statsDb->minQueryBuffer = STATSDB_MIN_BUFFER_QUERY;

    statsDb->queryBuffer = "";
    statsDb->statsTable = (StatsDBTable*) MEM_malloc(sizeof(StatsDBTable));
    statsDb->statsTable->createDescriptionTable = FALSE;
    statsDb->statsTable->createStatusTable = FALSE;
    statsDb->statsTable->createAggregateTable = FALSE;
    statsDb->statsTable->createSummaryTable = FALSE;
    statsDb->statsTable->createEventsTable = FALSE;
    statsDb->statsTable->createConnectivityTable = FALSE;
    statsDb->statsTable->createUrbanPropTable = FALSE;

    statsDb->statsDescTable = (StatsDBDescTable*) MEM_malloc(sizeof(StatsDBDescTable));
    statsDb->statsDescTable->createNodeDescTable = FALSE;
    statsDb->statsDescTable->createQueueDescTable = FALSE;
    statsDb->statsDescTable->createSchedulerDescTable = FALSE;
    statsDb->statsDescTable->createSessionDescTable = FALSE;
    statsDb->statsDescTable->createConnectionDescTable = FALSE;
    statsDb->statsDescTable->createInterfaceDescTable = FALSE;
    statsDb->statsDescTable->createPhyDescTable = FALSE;

    // constructor in data structure
    statsDb->statsInterfaceDesc = new StatsDBInterfaceDescContent;

    statsDb->statsQueueDesc = (StatsDBQueueDescContent*) MEM_malloc(sizeof(StatsDBQueueDescContent));
    statsDb->statsQueueDesc->isDiscipline = FALSE;
    statsDb->statsQueueDesc->isSize = FALSE;
    statsDb->statsQueueDesc->isPriority = FALSE;
    statsDb->statsSchedulerAlgo = new StatsDBSchedulerDescContent;

    // constructor in data structure
    statsDb->statsSessionDesc = new StatsDBSessionDescContent;
    // constructor in data structure
    statsDb->statsConnectionDesc = new StatsDBConnectionDescContent;

    statsDb->statsStatusTable = new StatsDBStatusTable ;
    statsDb->statsStatusTable->createNodeStatusTable = FALSE;
    statsDb->statsStatusTable->createInterfaceStatusTable = FALSE;
    statsDb->statsStatusTable->createQueueStatusTable = FALSE;
    statsDb->statsStatusTable->createMulticastStatusTable = FALSE;
    statsDb->statsStatusTable->createMalsrStatusTable = FALSE;
    statsDb->statsStatusTable->statusInterval = STATSDB_DEFAULT_STATUS_INTERVAL;
    statsDb->statsStatusTable->endSimulation = TRUE;

    statsDb->statsNodeStatus = (StatsDBNodeStatusContent*) MEM_malloc(sizeof(StatsDBNodeStatusContent));
    statsDb->statsNodeStatus->isActiveState = FALSE;
    statsDb->statsNodeStatus->isDamageState = FALSE;
    statsDb->statsNodeStatus->isPosition = FALSE;
    statsDb->statsNodeStatus->isVelocity = FALSE;
    statsDb->statsNodeStatus->isGateway = FALSE;

    statsDb->statsAggregateTable = (StatsDBAggregateTable*) MEM_malloc(sizeof(StatsDBAggregateTable));
    memset(statsDb->statsAggregateTable, 0, sizeof(StatsDBAggregateTable));
    statsDb->statsAggregateTable->createAppAggregateTable = FALSE;
    statsDb->statsAggregateTable->createTransAggregateTable = FALSE;
    statsDb->statsAggregateTable->createNetworkAggregateTable = FALSE;
    statsDb->statsAggregateTable->createMacAggregateTable = FALSE;
    statsDb->statsAggregateTable->createPhyAggregateTable = FALSE;
    statsDb->statsAggregateTable->aggregateInterval = STATSDB_DEFAULT_AGGREGATE_INTERVAL;
    statsDb->statsAggregateTable->endSimulation = TRUE;

    statsDb->statsAppAggregate = (StatsDBAppAggregateContent*) MEM_malloc(sizeof(StatsDBAppAggregateContent));
    statsDb->statsAppAggregate->isUnicastDelay = FALSE;
    statsDb->statsAppAggregate->isUnicastJitter = FALSE;
    statsDb->statsAppAggregate->isUnicastHopCount = FALSE;
    statsDb->statsAppAggregate->isMulticastDelay = FALSE;
    statsDb->statsAppAggregate->isMulticastJitter = FALSE;
    statsDb->statsAppAggregate->isMulticastHopCount = FALSE;
    statsDb->statsAppAggregate->isAvgDelay = FALSE;
    statsDb->statsAppAggregate->isAvgJitter = FALSE;
    statsDb->statsAppAggregate->isAvgThroughput = FALSE;
    statsDb->statsAppAggregate->isAvgOfferload = FALSE;

    statsDb->statsTransAggregate = new StatsDBTransportAggregateContent;

    statsDb->statsNetAggregate = new StatsDBNetworkAggregateContent;
    //statsDb->statsNetAggregate->isDelay = FALSE;
    //statsDb->statsNetAggregate->isJitter = FALSE;
    //statsDb->statsNetAggregate->isIpOutNoRoutes = FALSE;

    statsDb->statsMacAggregate = (StatsDBMacAggregateContent*) MEM_malloc(sizeof(StatsDBMacAggregateContent));
    Int32 i;
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsMacAggregate->addrTypes[i].isAvgQueuingDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgMediumAccessDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgMediumDelay = FALSE;
        statsDb->statsMacAggregate->addrTypes[i].isAvgJitter = FALSE;
    }
    statsDb->statsPhyAggregate = (StatsDBPhyAggregateContent*) MEM_malloc(sizeof(StatsDBPhyAggregateContent));
    statsDb->statsPhyAggregate->isAvgPathLoss = FALSE;
    statsDb->statsPhyAggregate->isAvgSignalPower = FALSE;
    statsDb->statsPhyAggregate->isAvgDelay = FALSE;

    statsDb->statsSummaryTable = (StatsDBSummaryTable*) MEM_malloc(sizeof(StatsDBSummaryTable));
    memset(statsDb->statsSummaryTable, 0, sizeof(StatsDBSummaryTable));
    statsDb->statsSummaryTable->createAppSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMulticastAppSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMulticastNetSummaryTable = FALSE;
    statsDb->statsSummaryTable->createTransSummaryTable = FALSE;
    statsDb->statsSummaryTable->createNetworkSummaryTable = FALSE;
    statsDb->statsSummaryTable->createMacSummaryTable = FALSE;
    statsDb->statsSummaryTable->createPhySummaryTable = FALSE;
    statsDb->statsSummaryTable->summaryInterval = STATSDB_DEFAULT_SUMMARY_INTERVAL;
    statsDb->statsSummaryTable->endSimulation = TRUE;

    statsDb->statsAppSummary = (StatsDBAppSummaryContent*) MEM_malloc(sizeof(StatsDBAppSummaryContent));
    statsDb->statsAppSummary->isDelay = FALSE;
    statsDb->statsAppSummary->isJitter = FALSE;
    statsDb->statsAppSummary->isHopCount = FALSE;
    statsDb->statsMulticastAppSummary = (StatsDBMulticastAppSummaryContent*) MEM_malloc(sizeof(StatsDBMulticastAppSummaryContent));
    statsDb->statsMulticastAppSummary->isDelay = FALSE;
    statsDb->statsMulticastAppSummary->isJitter = FALSE;
    statsDb->statsMulticastAppSummary->isHopCount = FALSE;

    statsDb->statsNetSummary = (StatsDBNetworkSummaryContent*) MEM_malloc(sizeof(StatsDBNetworkSummaryContent));
    statsDb->statsNetSummary->isDataDelay = FALSE;
    statsDb->statsNetSummary->isControlDelay = FALSE;
    statsDb->statsNetSummary->isDataJitter = FALSE;
    statsDb->statsNetSummary->isControlJitter = FALSE;

    statsDb->statsTransSummary = (StatsDBTransSummaryContent*) MEM_malloc(sizeof(StatsDBTransSummaryContent));
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsTransSummary->addrTypes[i].isDelay = FALSE;
        statsDb->statsTransSummary->addrTypes[i].isJitter = FALSE;
    }

    statsDb->statsMacSummary = (StatsDBMacSummaryContent*) MEM_malloc(sizeof(StatsDBMacSummaryContent));
    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        statsDb->statsMacSummary->addrTypes[i].isAvgQueuingDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgMediumAccessDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgMediumDelay = FALSE;
        statsDb->statsMacSummary->addrTypes[i].isAvgJitter = FALSE;
    }
    statsDb->statsPhySummary = (StatsDBPhySummaryContent*) MEM_malloc(sizeof(StatsDBPhySummaryContent));
    statsDb->statsPhySummary->isAvgDelay = FALSE;
    statsDb->statsPhySummary->isAvgPathLoss = FALSE;
    statsDb->statsPhySummary->isAvgSignalPower = FALSE;

    statsDb->statsEventsTable = (StatsDBEventsTable*) MEM_malloc(sizeof(StatsDBEventsTable));
    memset(statsDb->statsEventsTable, 0, sizeof(StatsDBEventsTable));
    statsDb->statsEventsTable->createAppEventsTable = FALSE;
    statsDb->statsEventsTable->createTransEventsTable = FALSE;
    statsDb->statsEventsTable->createNetworkEventsTable = FALSE;
    statsDb->statsEventsTable->createMacEventsTable = FALSE;
    statsDb->statsMacEvents = new StatsDBMacEventContent ;
    statsDb->statsEventsTable->createPhyEventsTable = FALSE;
    statsDb->statsEventsTable->createQueueEventsTable = FALSE;

    statsDb->statsConnTable = new StatsDBConnTable;
    statsDb->statsConnTable->createAppConnTable = FALSE;
    statsDb->statsConnTable->createTransConnTable = FALSE;
    statsDb->statsConnTable->createNetworkConnTable = FALSE;
    statsDb->statsConnTable->createMacConnTable = FALSE;
    statsDb->statsConnTable->createPhyConnTable = FALSE;
    statsDb->statsConnTable->createMulticastConnTable = FALSE;
    statsDb->statsConnTable->v_AppConnParam = NULL;
    statsDb->statsConnTable->v_TransConnParam = NULL;

    statsDb->statsAppEvents = new StatsDBAppEventContent;

    statsDb->statsNetEvents = new StatsDBNetworkEventContent;
    statsDb->statsNetEvents->isMsgSeqNum = FALSE;
    statsDb->statsNetEvents->isFragId = FALSE;
    statsDb->statsNetEvents->isMacProtocol = FALSE;
    statsDb->statsNetEvents->isControlSize = FALSE;
    statsDb->statsNetEvents->isPriority = FALSE;
    statsDb->statsNetEvents->isProtocolType = FALSE;
    statsDb->statsNetEvents->isPktFailureType = FALSE;
    statsDb->statsNetEvents->isPktType = FALSE;
    statsDb->statsNetEvents->isInterfaceIndex = FALSE;
    statsDb->statsNetEvents->isHopCount = FALSE;
    statsDb->statsNetEvents->networkStatsDBControl = FALSE;
    statsDb->statsNetEvents->networkStatsDBIncoming = FALSE;
    statsDb->statsNetEvents->networkStatsDBOutgoing = FALSE;
    statsDb->statsNetEvents->bufferSizeInBytes = DB_LONG_BUFFER_LENGTH;
    statsDb->statsNetEvents->multipleValues = FALSE;

    statsDb->statsPhyEvents = (StatsDBPhyEventContent*) MEM_malloc(sizeof(StatsDBPhyEventContent));
    statsDb->statsPhyEvents->isChannelIndex = FALSE;
    statsDb->statsPhyEvents->isControlSize = FALSE;
    statsDb->statsPhyEvents->isInterference = FALSE;
    statsDb->statsPhyEvents->isMessageFailureType = FALSE;
    statsDb->statsPhyEvents->isPathLoss = FALSE;
    statsDb->statsPhyEvents->isSignalPower = FALSE;

    statsDb->statsTransEvents = (StatsDBTransEventContent*)
        new(StatsDBTransEventContent);

    statsDb->statsNetConn = (StatsDBNetworkConnContent*) MEM_malloc(sizeof(StatsDBNetworkConnContent));
    statsDb->statsNetConn->isDstMaskAddr = FALSE;
    statsDb->statsNetConn->isOutgoingInterfaceIndex = FALSE;
    statsDb->statsNetConn->isNextHopAddr = FALSE;
    statsDb->statsNetConn->isRoutingProtocol = FALSE;
    statsDb->statsNetConn->isAdminDistance = FALSE;

    statsDb->statsMacConn = new StatsDBMacConnContent ;

    statsDb->statsPhyConn = (StatsDBPhyConnContent*) MEM_malloc(sizeof(StatsDBPhyConnContent));
    statsDb->statsPhyConn->isPhyIndex = FALSE;
    statsDb->statsPhyConn->isChannelIndex = FALSE;
    statsDb->statsOspfTable = NULL;
    statsDb->statsPimTable = NULL;
    statsDb->statsIgmpTable = NULL;
}

// Meta data code
void STATSDB_CreateNodeMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.

    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;

    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "NODE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "NODE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "NODE_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreateQueueMetaDataColumns
// PURPOSE  : Update the QUEUE_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreateQueueMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the queue content on to the database.

    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;

    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "QUEUE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        numMetaData++;
    }

    // Loop through the defined meta data columns
    // and add each one to the QUEUE_Description table
    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "QUEUE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "QUEUE_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreateConnectionMetaDataColumns
// PURPOSE  : Update the SCHEDULER_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreateConnectionMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.

    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;

    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "CONNECTION-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        numMetaData++;
    }

    // Loop through the defined meta data columns
    // and add each one to the CONNECTION_Description table
    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "CONNECTION-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "CONNECTION_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreateSchedulerMetaDataColumns
// PURPOSE  : Update the SCHEDULER_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreateSchedulerMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.

    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;

    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SCHEDULER-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        numMetaData++;
    }

    // Loop through the defined meta data columns
    // and add each one to the SCHEDULER_Description table
    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SCHEDULER-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "SCHEDULER_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreateSessionMetaDataColumns
// PURPOSE  : Update the SCHEDULER_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreateSessionMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.

    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;

    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            //&db->metaDataInput,
            "SESSION-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        ++numMetaData;
    }

    // Loop through the defined meta data columns
    // and add each one to the SESSION_Description table
    for (i = 0; i < numMetaData; ++i)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            //&db->metaDataInput,
            "SESSION-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "SESSION_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreateInterfaceMetaDataColumns
// PURPOSE  : Update the INTERFACE_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreateInterfaceMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "INTERFACE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        numMetaData++;
    }

    // Loop through the defined meta data columns
    // and add each one to the INTERFACE_Description table
    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "INTERFACE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "INTERFACE_Description",
            std::string(colName),
            "TEXT");
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_CreatePhyMetaDataColumns
// PURPOSE  : Update the PHY_Desc table to add columns for any user-
//            defined meta-data columns
// PARAMETERS :
// + partition : Current partition
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_CreatePhyMetaDataColumns(PartitionData* partition,
                                       NodeInput* nodeInput)
{
    // Add the new columns.
    // In this table we insert the node content on to the database.
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    char *errMsg = 0;
    std::string queryStr = "";

    Int32 err = 0;
    Int32 i = 0;
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;

    if (partition->partitionId) {return ;}
    StatsDb* db = NULL;
    db = partition->statsDb;

    while (TRUE)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "PHY-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound)
        {
            break;
        }
        ++numMetaData;
    }

    // Loop through the defined meta data columns
    // and add each one to the INTERFACE_Description table
    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            ANY_NODEID,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "PHY-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        // Now to add the column
        AddColumn(db,
            "PHY_Description",
            std::string(colName),
            "TEXT");
    }
}
void MetaDataStruct::AddNodeMetaData(Node* node,
                                     PartitionData* partition,
                                     NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "NODE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "NODE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node->nodeId,
                ANY_ADDRESS,
                //&db->metaDataInput,
                nodeInput,
                "NODE-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}

//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddInterfaceMetaData
// PURPOSE  : Fill in values for the meta-data columns of an interface
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddInterfaceMetaData(Node* node,
                                          Int32 interfaceIndex,
                                          PartitionData* partition,
                                          const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node,
            node->nodeId,
            interfaceIndex,
            //&db->metaDataInput,
            nodeInput,
            "INTERFACE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node,
            node->nodeId,
            interfaceIndex,
            //&db->metaDataInput,
            nodeInput,
            "INTERFACE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node,
                node->nodeId,
                interfaceIndex,
                //&db->metaDataInput,
                nodeInput,
                "INTERFACE-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}


//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddQueueMetaData
// PURPOSE  : Fill in values for the meta-data columns of a queue
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddQueueMetaData(Node* node,
                                     PartitionData* partition,
                                     const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "QUEUE-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "QUEUE-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node->nodeId,
                ANY_ADDRESS,
                //&db->metaDataInput,
                nodeInput,
                "QUEUE-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}

//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddSessionMetaData
// PURPOSE  : Fill in values for the meta-data columns of a session
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddSessionMetaData(Node* node,
                                     PartitionData* partition,
                                     const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SESSION-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SESSION-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node->nodeId,
                ANY_ADDRESS,
                //&db->metaDataInput,
                nodeInput,
                "SESSION-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}

//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddConnectionMetaData
// PURPOSE  : Fill in values for the meta-data columns of a connection
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddConnectionMetaData(Node* node,
                                     PartitionData* partition,
                                     const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "CONNECTION-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "CONNECTION-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node->nodeId,
                ANY_ADDRESS,
                //&db->metaDataInput,
                nodeInput,
                "CONNECTION-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}
//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddSchedulerMetaData
// PURPOSE  : Fill in values for the meta-data columns of a scheduler
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddSchedulerMetaData(Node* node,
                                     PartitionData* partition,
                                     const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SCHEDULER-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node->nodeId,
            ANY_ADDRESS,
            //&db->metaDataInput,
            nodeInput,
            "SCHEDULER-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node->nodeId,
                ANY_ADDRESS,
                //&db->metaDataInput,
                nodeInput,
                "SCHEDULER-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}


//--------------------------------------------------------------------//
// NAME     : MetaDataStruct::AddPhyMetaData
// PURPOSE  : Fill in values for the meta-data columns of a phy
// PARAMETERS :
// + node : Node to which the interface belongs
// + partition : Partition on which the node resides
// + nodeInput : Configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void MetaDataStruct::AddPhyMetaData(Node* node,
                                    Int32 interfaceIndex,
                                    PartitionData* partition,
                                    const NodeInput* nodeInput)
{
    Int32 numMetaData = 0;
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    char colName[MAX_STRING_LENGTH];
    std::string columnName ="";
    std::string content = "";

    StatsDb* db = NULL;
    db = partition->statsDb;
    Int32 i = 0;

    // Check for metaData in the configuration file,
    //
    // Scan how many metaData are defined
    //
    while (TRUE)
    {
        IO_ReadStringInstance(
            node,
            node->nodeId,
            interfaceIndex,
            //&db->metaDataInput,
            nodeInput,
            "PHY-META-DATA-COLUMN-NAME",
            numMetaData,
            (numMetaData == 0),
            &wasFound,
            buf);

        if (!wasFound) {
            break;
        }
        numMetaData++;
    }

    for (i = 0; i < numMetaData; i++)
    {
        IO_ReadStringInstance(
            node,
            node->nodeId,
            interfaceIndex,
            //&db->metaDataInput,
            nodeInput,
            "PHY-META-DATA-COLUMN-NAME",
            i,
            (numMetaData == 0),
            &wasFound,
            colName);

        if (wasFound)
        {
            columnName = colName;
            IO_ReadStringInstance(
                node,
                node->nodeId,
                interfaceIndex,
                //&db->metaDataInput,
                nodeInput,
                "PHY-META-DATA",
                i,
                (numMetaData == 0),
                &retVal,
                buf);
            if (retVal)
            {
                // Add to the metaData map.
                content = buf;
                m_MetaData[columnName] = buf;
            }
        }
    }
    std::string colNames = "";
    std::map<std::string, std::string>::iterator iter;
    iter = m_MetaData.begin();
}

////////////////////////////////////////////////////////////////////////
// Description tables definitions
////////////////////////////////////////////////////////////////////////

//--------------------------------------------------------------------//
// NAME     : StatsDBQueueDesc::StatsDBQueueDesc
// PURPOSE  : StatsDBQueueDesc constructor. Initialize QUEUE_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node on which the queue resides
// + index : index of interface on which the queue resides
// + queueIndex : Index of the queue, which is merely its position in
//                the list of queues it belongs to
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBQueueDesc::StatsDBQueueDesc(Int32 nodeId,
                                   Int32 index,
                                   Int32 queueIndex,
                                   std::string queueType)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = index;
    m_QueueIndex = queueIndex;
    m_QueueType = queueType;

    m_QueueDiscipline = "";
    m_QueueDisciplineSpecified = FALSE;

    m_QueueSize = 0;
    m_QueueSizeSpecified = FALSE;

    // Default / no queue priority is -1
    m_QueuePriority = -1;
    m_QueuePrioritySpecified = FALSE;
}


//--------------------------------------------------------------------//
// NAME     : StatsDBQueueDesc::SetQueueSize
// PURPOSE  : Set the value of a queue's size for the purpose of
//            inserting into the QUEUE_Description table
// PARAMETERS :
// + size : size (in bytes) of the queue
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBQueueDesc::SetQueueSize(Int32 size)
{
    m_QueueSize = size;
    m_QueueSizeSpecified = TRUE;
}


//--------------------------------------------------------------------//
// NAME     : StatsDBQueueDesc::SetQueueDiscipline
// PURPOSE  : Set the value of a queue's discpline for the purpose of
//            inserting into the QUEUE_Description table
// PARAMETERS :
// + discipline : A string identifying the type of queue (e.g. FIFO)
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBQueueDesc::SetQueueDiscipline(std::string discipline)
{
    m_QueueDiscipline = discipline;
    m_QueueDisciplineSpecified = TRUE;
}


//--------------------------------------------------------------------//
// NAME     : StatsDBQueueDesc::SetQueuePriority
// PURPOSE  : Set the value of a queue's priority number for the purpose of
//            inserting into the QUEUE_Description table
// PARAMETERS :
// + priority : The queue's priority number
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBQueueDesc::SetQueuePriority(Int32 priority)
{
    m_QueuePriority = priority;
    m_QueuePrioritySpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBSchedulerDesc::StatsDBSchedulerDesc
// PURPOSE  : StatsDBSchedulerDesc constructor. Initialize SCHEDULER_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node on which the scheduler resides
// + index : index of interface on which the scheduler resides
// + type : type of queue (e.g. network input, MAC output)
// + algorithm : name of the scheduling algorithm (e.g. "Strict Priority")
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBSchedulerDesc::StatsDBSchedulerDesc(Int32 nodeId,
                                           Int32 index,
                                           std::string type,
                                           std::string algorithm)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = index;
    m_SchedulerType = type;
    m_SchedulingAlgorithm = algorithm;
}

StatsDBSessionDesc::StatsDBSessionDesc(Int32 sessionId,
                                       Int32 senderId,
                                       Int32 receiverId)
{
    m_SessionId =  sessionId;
    m_SenderId = senderId;
    m_ReceiverId = receiverId;
    m_AppType = "";
    m_AppTypeSpecified = FALSE;

    m_SenderAddr = "";
    m_SenderAddrSpecified = FALSE;

    m_ReceiverAddr = "";
    m_ReceiverAddrSpecified = FALSE;

    m_SenderPort = 0;
    m_SrcPortSpecified = FALSE;

    m_RecvPort = 0;
    m_RecvPortSpecified = FALSE;

    m_TransportProtocol = "";
    m_TransportPortocolSpecified = FALSE;
}


void StatsDBSessionDesc::SetSenderAddr(std::string sdrAddr)
{
    m_SenderAddr = sdrAddr;
    m_SenderAddrSpecified = TRUE;
}

void StatsDBSessionDesc::SetAppType(std::string AppType)
{
    m_AppType = AppType;
    m_AppTypeSpecified = TRUE;
}

void StatsDBSessionDesc::SetReceiverAddr(std::string recvAddr)
{
    m_ReceiverAddr = recvAddr;
    m_ReceiverAddrSpecified = TRUE;
}

void StatsDBSessionDesc::SetRecvPort(short recvPort)
{
    m_RecvPort = recvPort;
    m_RecvPortSpecified = TRUE;
}

void StatsDBSessionDesc::SetSenderPort(short sdrPort)
{
    m_SenderPort = sdrPort;
    m_SrcPortSpecified = TRUE;
}

void StatsDBSessionDesc::SetTransportProtocol(const std::string &protocol)
{
    m_TransportProtocol = protocol;
    m_TransportPortocolSpecified = TRUE;
}

StatsDBConnectionDesc::StatsDBConnectionDesc(double timeValue,
                                             Int32 senderId,
                                             Int32 receiverId)
{
    m_timeValue = timeValue;
    m_SenderId = senderId;
    m_ReceiverId = receiverId;

    //m_AppSessionId = appSessionId;

    m_SenderAddr = "";
    m_SenderAddrSpecified = FALSE;

    m_ReceiverAddr = "";
    m_ReceiverAddrSpecified = FALSE;

    m_SenderPort = 0;
    m_SrcPortSpecified = FALSE;

    m_RecvPort = 0;
    m_RecvPortSpecified = FALSE;

    m_ConnectionType = "";
    m_ConnTypeSpecified = FALSE;

    m_NetworkProtocol = "";
    m_NetworkPortocolSpecified = FALSE;
}

void StatsDBConnectionDesc::SetConnectionType(std::string connType)
{
    m_ConnectionType = connType;
    m_ConnTypeSpecified = TRUE;
}

void StatsDBConnectionDesc::SetReceiverAddr(std::string recvAddr)
{
    m_ReceiverAddr = recvAddr;
    m_ReceiverAddrSpecified = TRUE;
}

void StatsDBConnectionDesc::SetSenderAddr(std::string sdrAddr)
{
    m_SenderAddr = sdrAddr;
    m_SenderAddrSpecified = TRUE;
}

void StatsDBConnectionDesc::SetRecvPort(short recvPort)
{
    m_RecvPort = recvPort;
    m_RecvPortSpecified = TRUE;
}

void StatsDBConnectionDesc::SetSenderPort(short sdrPort)
{
    m_SenderPort = sdrPort;
    m_SrcPortSpecified = TRUE;
}

void StatsDBConnectionDesc::SetNetworkProtocol(std::string protocol)
{
    m_NetworkProtocol = protocol;
    m_NetworkPortocolSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::StatsDBInterfaceDesc
// PURPOSE  : StatsDBInterfaceDesc constructor. Initialize INTERFACE_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node to which the interface belongs
// + interfaceIndex : Index of interface
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBInterfaceDesc::StatsDBInterfaceDesc(Int32 nodeId,
                                           Int32 interfaceIndex)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = interfaceIndex;

    m_InterfaceAddr = "";
    m_InterfaceAddrSpecified = FALSE;

    m_InterfaceName = "";
    m_InterfaceNameSpecified = FALSE;

    m_SubnetMask = "";
    m_SubnetMaskSpecified = FALSE;

    m_NetworkType = "";
    m_NetworkTypeSpecified = FALSE;

    m_SubnetId = 0;
    m_SubetIdSpecififed = FALSE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetInterfaceAddr
// PURPOSE  : Set the value of an interface's network address for the purpose of
//            inserting into the INTERFACE_Description table
// PARAMETERS :
// + addr : The interface's network address, as a string
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetInterfaceAddr(std::string addr)
{
    m_InterfaceAddr = addr;
    m_InterfaceAddrSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetInterfaceName
// PURPOSE  : Set the value of an interface's name for the purpose of
//            inserting into the INTERFACE_Description table
// PARAMETERS :
// + name : The interface's name, as a string
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetInterfaceName(std::string name)
{
    m_InterfaceName = name;
    m_InterfaceNameSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetNetworkType
// PURPOSE  : Set the value of an interface's network type (i.e. routing protocol)
//            for the purpose of inserting into the INTERFACE_Description table
// PARAMETERS :
// + type : A string identifying the network type
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetNetworkType(std::string type)
{
    m_NetworkType = type;
    m_NetworkTypeSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetMulticastProtocol
// PURPOSE  : Set the value of an interface's multicast protocol for
//            the purpose of inserting into the INTERFACE_Description table
// PARAMETERS :
// + protocol : A string identifying the multicast routing protocol
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetMulticastProtocol(std::string protocol)
{
    m_MulticastProtocol = protocol;
    m_MulticastProtocolSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetSubnetMask
// PURPOSE  : Set the value of an interface's subnet mask for
//            the purpose of inserting into the INTERFACE_Description table
// PARAMETERS :
// + mask : The interface's subnet mask, as a string
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetSubnetMask(std::string mask)
{
    m_SubnetMask = mask;
    m_SubnetMaskSpecified = TRUE;
}

//--------------------------------------------------------------------//
// NAME     : StatsDBInterfaceDesc::SetSubnetId
// PURPOSE  : Set the value of an interface's subnet id for
//            the purpose of inserting into the INTERFACE_Description table
// PARAMETERS :
// + id : An int identifying the subnet id
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBInterfaceDesc::SetSubnetId(Int32 id)
{
    m_SubnetId = id;
    m_SubetIdSpecififed = TRUE;
}


//--------------------------------------------------------------------//
// NAME     : StatsDBPhyDesc::StatsDBPhyDesc
// PURPOSE  : StatsDBPhyDesc constructor. Initialize PHY_Description
//            table values
// PARAMETERS :
// + nodeId : ID of node to which the radio belongs
// + interfaceIndex : Index of interface to which the radio channel belongs
// + phyIndex : Index of the radio channel (i.e. its index into the list of
//              channels)
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBPhyDesc::StatsDBPhyDesc(Int32 nodeId,
                               Int32 interfaceIndex,
                               Int32 phyIndex)
{
    m_NodeId = nodeId;
    m_InterfaceIndex = interfaceIndex;
    m_PhyIndex = phyIndex;
}


///////////////////////////////////////////////////////////////////////
// Status Tables definitions
///////////////////////////////////////////////////////////////////////



//--------------------------------------------------------------------//
// NAME     : StatsDBNodeStatus::StatsDBNodeStatus
// PURPOSE  : StatsDBNodeStatus constructor. Initialize NODE_Status
//            table values
// PARAMETERS :
// + node : Current node
// + triggered : TRUE if this object was instantiated as the result of
//               a 'triggered' (event-initiated) update. FALSE otherwise.
//
// RETURN   : None.
//--------------------------------------------------------------------//
StatsDBNodeStatus::StatsDBNodeStatus(Node* node,
                                     BOOL triggered)
{
    m_NodeId = node->nodeId;
    m_TriggeredUpdate = triggered;

    m_PositionUpdated = TRUE;
    m_ActiveStateUpdated = TRUE;
    m_DamageStateUpdated = TRUE;
    m_VelocityUpdated = TRUE;

    /* Fill in all of the member fields */

    PartitionData* partition = node->partitionData;

    // Position
    if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
    {
        m_DimensionOnePosition = node->mobilityData->current->position.cartesian.x;
        m_DimensionTwoPosition = node->mobilityData->current->position.cartesian.y;
        m_DimensionThreePosition = node->mobilityData->current->position.cartesian.z;
    }
    else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
    {
        m_DimensionOnePosition = node->mobilityData->current->position.latlonalt.latitude;
        m_DimensionTwoPosition = node->mobilityData->current->position.latlonalt.longitude;
        m_DimensionThreePosition = node->mobilityData->current->position.latlonalt.altitude;
    }

    // Active state
    // A node is functionally defined to be active (enabled) if at least one of its
    // interfaces is currently enabled. If a node has no enabled interfaces, the
    // node is considered to be inactive (disabled).
    m_Active = STATS_DB_Disabled;
    Int32 i;
    for (i = 0; i < node->numberInterfaces; i++)
    {
        if (NetworkIpInterfaceIsEnabled(node, i))
        {
            m_Active = STATS_DB_Enabled;
            break;
        }
    }

    // Damage state and Gatway
    // NOTE: Damage state only carries any meaning when using the CES socket
    // interface. When the socket interface is not in use, all nodes are set
    // to undamaged by default.
#ifdef ADDON_BOEINGFCS

    // Gateway first
    m_IsGateway = NetworkCesSubnetIsTrueGateway(node);
#endif 
#ifdef SOCKET_INTERFACE
    EXTERNAL_Interface* socketIface;
    socketIface = node->partitionData->interfaceTable[EXTERNAL_SOCKET];
    if (socketIface != NULL)
    {
        EntityMapping* mapping;
        mapping = SocketInterface_GetEntityMappingStructure(socketIface);
        EntityData* entity = NULL;
        entity = EntityMappingLookup(mapping, node->nodeId);
        if (entity != NULL)
        {
            if (entity->damageState == SocketInterface_PlatformStateType_Undamaged)
            {
                m_DamageState = STATS_DB_Undamaged;
            }
            else
            {
                m_DamageState = STATS_DB_Damaged;
            }
        }
        else
        {
            m_DamageState = STATS_DB_Undamaged;
        }
    }
    else
    {
        m_DamageState = STATS_DB_Undamaged;
    }
#else
    m_DamageState = STATS_DB_Undamaged;
#endif
    // Velocity

    // Check that CES-SOCKET is present and set to YES
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        partition->nodeInput,
        "CES-SOCKET",
        &wasFound,
        buf);
    if (!wasFound)
    {
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            partition->nodeInput,
            "SOCKET-INTERFACE",
            &wasFound,
            buf);
    }
    if (wasFound && strcmp(buf, "YES") == 0)
    {

        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            m_DimensionOneVelocity = node->mobilityData->lastExternalVelocity.cartesian.x;
            m_DimensionTwoVelocity = node->mobilityData->lastExternalVelocity.cartesian.y;
            m_DimensionThreeVelocity = node->mobilityData->lastExternalVelocity.cartesian.z;
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            m_DimensionOneVelocity = node->mobilityData->lastExternalVelocity.latlonalt.latitude;
            m_DimensionTwoVelocity = node->mobilityData->lastExternalVelocity.latlonalt.longitude;
            m_DimensionThreeVelocity = node->mobilityData->lastExternalVelocity.latlonalt.altitude;
        }
    }
    else
    {
        // When not using the socket interface, velocity is not explicity specified.
        // Rather, each mobile node has a speed, a destination, and an arrival time. The
        // directional velocities are computed here.
        double c1 = node->mobilityData->current->position.common.c1;
        double c2 = node->mobilityData->current->position.common.c2;
        double c3 = node->mobilityData->current->position.common.c3;
        double currTime = (double) node->mobilityData->current->time / SECOND;

        double c1_next = node->mobilityData->next->position.common.c1;
        double c2_next = node->mobilityData->next->position.common.c2;
        double c3_next = node->mobilityData->next->position.common.c3;
        double nextTime = (double) node->mobilityData->next->time / SECOND;

        double v_c1 = (c1_next - c1) / (nextTime - currTime);
        double v_c2 = (c2_next - c2) / (nextTime - currTime);
        double v_c3 = (c3_next - c3) / (nextTime - currTime);

        m_DimensionOneVelocity = v_c1;
        m_DimensionTwoVelocity = v_c2;
        m_DimensionThreeVelocity = v_c3;
    }
}

///////////////////////////////////////////////////////////////////////
// Aggregate Tables definitons
///////////////////////////////////////////////////////////////////////


StatsDBAppAggregateParam::StatsDBAppAggregateParam()
{
    m_UnicastMessageSent = 0;
    m_UnicastMessageRecd = 0;
    m_MulticastMessageSent = 0;
    m_EffMulticastMessageSent = 0;
    m_MulticastMessageRecd = 0;
    m_UnicastByteSent = 0;
    m_UnicastByteRecd = 0;
    m_UnicastFragmentSent = 0;
    m_UnicastFragmentRecd = 0;
    m_UnicastMessageCompletionRate = 0;
    m_MulticastMessageCompletionRate = 0;
    m_UnicastOfferedLoad = 0;
    m_UnicastThroughput = 0;
    m_MulticastOfferedLoad = 0;
    m_MulticastThroughput = 0;

    m_MulticastByteSent = 0;
    m_EffMulticastByteSent = 0;
    m_MulticastByteRecd = 0;
    m_MulticastFragmentSent = 0;
    m_EffMulticastFragmentSent = 0;
    m_MulticastFragmentRecd = 0;

    m_UnicastDelay = 0;
    m_UnicastDelaySpecified = FALSE;

    m_MulticastDelay = 0;
    m_MulticastDelaySpecified = FALSE;

    m_UnicastJitter = 0;
    m_UnicastJitterSpecified = FALSE;

    m_MulticastJitter = 0;
    m_MulticastJitterSpecified = FALSE;

    m_UnicastHopCount = 0;
    m_UnicastHopCountSpecified = FALSE;

    m_MulticastHopCount = 0;
    m_MulticastHopCountSpecified = FALSE;
}

void StatsDBAppAggregateParam::SetMulticastDelay(double delay)
{
    m_MulticastDelay = delay;
    m_MulticastDelaySpecified = TRUE;
}

void StatsDBAppAggregateParam::SetUnicastDelay(double delay)
{
    m_UnicastDelay = delay;
    m_UnicastDelaySpecified = TRUE;
}

void StatsDBAppAggregateParam::SetUnicastHopCount(double hopCount)
{
    m_UnicastHopCount = hopCount;
    m_UnicastHopCountSpecified = TRUE;
}

void StatsDBAppAggregateParam::SetMulticastHopCount(double hopCount)
{
    m_MulticastHopCount = hopCount;
    m_MulticastHopCountSpecified = TRUE;
}

void StatsDBAppAggregateParam::SetUnicastJitter(double jitter)
{
    m_UnicastJitter = jitter;
    m_UnicastJitterSpecified = TRUE;
}

void StatsDBAppAggregateParam::SetMulticastJitter(double jitter)
{
    m_MulticastJitter = jitter;
    m_MulticastJitterSpecified = TRUE;
}

// Network Aggregate Table definitions

StatsDBNetworkAggregateParam::StatsDBNetworkAggregateParam()
{
    m_UDataPacketsSent = 0;
    m_UDataPacketsRecd = 0;
    m_UDataPacketsForward = 0;
    m_UControlPacketsSent = 0;
    m_UControlPacketsRecd = 0;
    m_UControlPacketsForward = 0;

    m_MDataPacketsSent = 0;
    m_MDataPacketsRecd = 0;
    m_MDataPacketsForward = 0;
    m_MControlPacketsSent = 0;
    m_MControlPacketsRecd = 0;
    m_MControlPacketsForward = 0;

    m_BDataPacketsSent = 0;
    m_BDataPacketsRecd = 0;
    m_BDataPacketsForward = 0;
    m_BControlPacketsSent = 0;
    m_BControlPacketsRecd = 0;
    m_BControlPacketsForward = 0;

    m_UDataBytesSent = 0;
    m_UDataBytesRecd = 0;
    m_UDataBytesForward = 0;
    m_UControlBytesSent = 0;
    m_UControlBytesRecd = 0;
    m_UControlBytesForward = 0;

    m_MDataBytesSent = 0;
    m_MDataBytesRecd = 0;
    m_MDataBytesForward = 0;
    m_MControlBytesSent = 0;
    m_MControlBytesRecd = 0;
    m_MControlBytesForward = 0;

    m_BDataBytesSent = 0;
    m_BDataBytesRecd = 0;
    m_BDataBytesForward = 0;
    m_BControlBytesSent = 0;
    m_BControlBytesRecd = 0;
    m_BControlBytesForward = 0;

    Int32 t;
    for (t = 0; t < StatsDBNetworkAggregateContent::s_numTrafficTypes; ++t)
    {
        m_CarrierLoad.push_back(0);

        m_Delay.push_back(0);
        m_DelaySpecified.push_back(FALSE);

        m_Jitter.push_back(0);
        m_JitterSpecified.push_back(FALSE);

        m_ipOutNoRoutes.push_back(0);
        m_ipOutNoRoutesSpecified.push_back(FALSE);

        m_totalJitter.push_back(0) ;
        m_jitterDataPoints.push_back(0);
        m_jitterDataPointsSpecified.push_back(FALSE) ;
    }
}

void StatsDBNetworkAggregateParam::SetDelay(double delay,
                                            StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST &&
        t <= StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_Delay[t] = delay;
    m_DelaySpecified[t] = TRUE;
}

void StatsDBNetworkAggregateParam::SetJitter(double jitter,
    StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST &&
        t <= StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_Jitter[t] = jitter;
    m_JitterSpecified[t] = TRUE;
}

void StatsDBNetworkAggregateParam::SetIpOutNoRoutes(
    Int32 ipOutNoRoutes,
    StatsDBNetworkAggregateContent::NetAggrTrafficType t)
{
    ERROR_Assert(t>= StatsDBNetworkAggregateContent::UNICAST
        && t < StatsDBNetworkAggregateContent::BROADCAST,
        "ERROR in StatsDBNetworkAggregateParam traffic type");
    m_ipOutNoRoutes[t] = ipOutNoRoutes;
    m_ipOutNoRoutesSpecified[t] = TRUE;
}

// Phy Aggregate Table definitions

StatsDBPhyAggregateParam::StatsDBPhyAggregateParam()
{
    toInsert = FALSE;
    m_NumTransmittedSignals = 0;
    m_NumLockedSignals = 0;
    m_NumReceivedSignals = 0;
    m_NumDroppedSignals = 0;
    m_NumDroppedInterferenceSignals = 0;
    //m_NumPhys = 0;
    m_AvgInterference = 0;
    m_AvgInterferenceSpecified = FALSE;

    m_Utilization = 0;
    m_UtilizationSpecified = FALSE;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_PathLoss = 0;
    m_PathLossSpecified = 0;

    m_SignalPower = 0;
    m_SignalPowerSpecified = FALSE;
}

void StatsDBPhyAggregateParam::SetDelay(double avgDelay)
{
    m_Delay = avgDelay;
    m_DelaySpecified = TRUE;
}

void StatsDBPhyAggregateParam::SetUtilization(double u)
{
    m_Utilization = u;
    m_UtilizationSpecified = TRUE;
}
void StatsDBPhyAggregateParam::SetPathLoss(double avgPathLoss)
{
    m_PathLoss = avgPathLoss;
    m_PathLossSpecified = TRUE;
}

void StatsDBPhyAggregateParam::SetSignalPower(double avgSignalPower)
{
    m_SignalPower = avgSignalPower;
    m_SignalPowerSpecified = TRUE;
}

void StatsDBPhyAggregateParam::SetAvgInterference(double avgInterference)
{
    m_AvgInterference = avgInterference;
    m_AvgInterferenceSpecified = TRUE;
}


StatsDBMacAggregateParam::StatsDBMacAggregateParam()
{
    toInsert = FALSE;

    m_DataFramesSent = 0;
    m_DataFramesReceived = 0;
    m_DataBytesSent = 0;
    m_DataBytesReceived = 0;
    m_ControlFramesSent = 0;
    m_ControlFramesReceived = 0;
    m_ControlBytesSent = 0;
    m_ControlBytesReceived  = 0;

    m_AvgDelay = 0;
    m_AvgDelaySpecified = FALSE;

    m_AvgJitter = 0;
    m_AvgJitterSpecified = FALSE;
}

void StatsDBMacAggregateParam::SetAvgDelay(double delay)
{
    m_AvgDelay = delay;
    m_AvgDelaySpecified = TRUE;
}

void StatsDBMacAggregateParam::SetAvgJitter(double jitter)
{
    m_AvgJitter = jitter;
    m_AvgJitterSpecified = TRUE;
}

StatsDBAppJitterAggregateParam::StatsDBAppJitterAggregateParam()
{
    m_PartitionId = -1;
    m_TotalUnicastJitter = 0;
    m_TotalMulticastJitter = 0;
    m_UnicastMessageReceived = 0;
    m_MulticastMessageReceived =0;
}

///////////////////////////////////////////////////////////////////////
// Summary Tables definitions
///////////////////////////////////////////////////////////////////////

StatsDBAppSummaryParam::StatsDBAppSummaryParam()
{
    m_InitiatorId = 0;
    m_ReceiverId = 0;
    m_TargetAddr[0] = '\0';
    m_SessionId = 0;
    m_Tos = 0;
    m_MessageSent = 0;
    m_EffMessageSent = 0;
    m_MessageRecd = 0;
    m_ByteSent = 0;
    m_EffByteSent = 0;
    m_ByteRecd = 0;
    m_FragmentSent = 0;
    m_EffFragmentSent = 0;
    m_FragmentRecd = 0;
    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0' ;

    m_MessageCompletionRate = 0;
    m_MessageCompletionRateSpecified = FALSE;

    m_OfferedLoad = 0;
    m_OfferedLoadSpecified = FALSE;

    m_Throughput = 0;
    m_ThroughputSpecified = FALSE;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_Jitter = 0;
    m_JitterSpecified = FALSE;

    m_HopCount = 0;
    m_HopCountSpecified = FALSE;

#ifdef ADDON_NGCNMS
    isRetrieved = FALSE ;
#endif
}

StatsDBAppJitterSummaryParam::StatsDBAppJitterSummaryParam()
{
    m_InitiatorId = 0;
    m_SessionId = 0;
    m_TotalJitter = 0;
    m_MessageRecd = 0;
}

StatsDBMulticastAppSummaryParam::StatsDBMulticastAppSummaryParam()
{
    m_InitiatorId = 0;
    m_ReceiverId = 0;
    m_GroupAddr[0] = '\0';
    m_SessionId = 0;
    m_Tos = 0;
    m_MessageSent = 0;
    m_MessageRecd = 0;
    m_ByteSent = 0;
    m_ByteRecd = 0;
    m_FragmentSent = 0;
    m_FragmentRecd = 0;
    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0' ;

    m_MessageCompletionRate = 0;
    m_MessageCompletionRateSpecified = FALSE;

    m_OfferedLoad = 0;
    m_OfferedLoadSpecified = FALSE;

    m_Throughput = 0;
    m_ThroughputSpecified = FALSE;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_Jitter = 0;
    m_JitterSpecified = FALSE;

    m_HopCount = 0;
    m_HopCountSpecified = FALSE;

#ifdef ADDON_NGCNMS
    isRetrieved = FALSE ;
#endif
}
void StatsDBAppSummaryParam::SetCompletionRate(double rate)
{
    m_MessageCompletionRate = rate;
    m_MessageCompletionRateSpecified = TRUE;
}

void StatsDBMulticastAppSummaryParam::SetCompletionRate(double rate)
{
    m_MessageCompletionRate = rate;
    m_MessageCompletionRateSpecified = TRUE;
}

void StatsDBAppSummaryParam::SetOfferedLoad(double load)
{
    m_OfferedLoad = load;
    m_OfferedLoadSpecified = TRUE;
}
void StatsDBMulticastAppSummaryParam::SetOfferedLoad(double load)
{
    m_OfferedLoad = load;
    m_OfferedLoadSpecified = TRUE;
}
void StatsDBAppSummaryParam::SetThroughput(double t)
{
    m_Throughput = t;
    m_ThroughputSpecified = TRUE;
}
void StatsDBMulticastAppSummaryParam::SetThroughput(double t)
{
    m_Throughput = t;
    m_ThroughputSpecified = TRUE;
}
void StatsDBAppSummaryParam::SetDelay(double delay)
{
    m_Delay = delay;
    m_DelaySpecified = TRUE;
}

void StatsDBMulticastAppSummaryParam::SetDelay(double delay)
{
    m_Delay = delay;
    m_DelaySpecified = TRUE;
}

void StatsDBAppSummaryParam::SetHopCount(double hopCount)
{
    m_HopCount = hopCount;
    m_HopCountSpecified = TRUE;
}

void StatsDBMulticastAppSummaryParam::SetHopCount(double hopCount)
{
    m_HopCount = hopCount;
    m_HopCountSpecified = TRUE;
}
void StatsDBAppSummaryParam::SetJitter(double jitter)
{
    m_Jitter = jitter;
    m_JitterSpecified = TRUE;
}

void StatsDBMulticastAppSummaryParam::SetJitter(double jitter)
{
    m_Jitter = jitter;
    m_JitterSpecified = TRUE;
}

// Network Summary
StatsDBNetworkSummaryParam::StatsDBNetworkSummaryParam()
{
    m_UDataPacketsSent = 0;
    m_UDataPacketsRecd = 0;
    m_UDataPacketsForward = 0;
    m_UControlPacketsSent = 0;
    m_UControlPacketsRecd = 0;
    m_UControlPacketsForward = 0;

    m_UDataBytesSent = 0;
    m_UDataBytesRecd = 0;
    m_UDataBytesForward = 0;
    m_UControlBytesSent = 0;
    m_UControlBytesRecd = 0;
    m_UControlBytesForward = 0;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_Jitter = 0;
    m_JitterSpecified = FALSE;

    m_DataDelay = 0 ;
    m_DataDelaySpecified = FALSE;

    m_DataJitter = 0;
    m_DataJitterSpecified = FALSE;

    m_ControlDelay = 0;
    m_ControlDelaySpecified = FALSE;

    m_ControlJitter = 0;
    m_ControlJitterSpecified = FALSE;


}

void StatsDBNetworkSummaryParam::SetDelay(double delay)
{
    m_Delay = delay;
    m_DelaySpecified = TRUE;
}

void StatsDBNetworkSummaryParam::SetJitter(double jitter)
{
    m_Jitter = jitter;
    m_JitterSpecified = TRUE;
}


void StatsDBNetworkSummaryParam::SetDelay(double delay,
                                          NETSUMPARAM_DataType t)
{
    if (t == StatsDBNetworkSummaryParam::DATA)
    {
        m_DataDelay = delay;
        m_DataDelaySpecified = TRUE;
    }else {
        m_ControlDelay = delay;
        m_ControlDelaySpecified = TRUE;
    }
}

void StatsDBNetworkSummaryParam::SetJitter(double jitter,
                                           NETSUMPARAM_DataType t)
{
    if (t == StatsDBNetworkSummaryParam::DATA)
    {
        m_DataJitter = jitter;
        m_DataJitterSpecified = TRUE;
    }else {
        m_ControlJitter = jitter;
        m_ControlJitterSpecified = TRUE;
    }
}


StatsDBMacSummaryParam::StatsDBMacSummaryParam()
{
    m_BroadcastDataFramesSent = 0;
    m_UnicastDataFramesSent = 0;
    m_BroadcastDataFramesReceived = 0;
    m_UnicastDataFramesReceived = 0;
    m_BroadcastDataBytesSent = 0;
    m_UnicastDataBytesSent = 0;
    m_BroadcastDataBytesReceived = 0;
    m_UnicastDataBytesReceived = 0;

    m_ControlFramesSent = 0;
    m_ControlFramesReceived = 0;
    m_ControlBytesSent = 0;
    m_ControlBytesReceived  = 0;
    m_FramesDropped = 0;
    m_BytesDropped = 0;

    m_AvgDelay = 0;
    m_AvgDelaySpecified = FALSE;

    m_AvgJitter = 0;
    m_AvgJitterSpecified = FALSE;
}
void StatsDBMacSummaryParam::SetAvgDelay(double delay)
{
    m_AvgDelay = delay;
    m_AvgDelaySpecified = TRUE;
}

void StatsDBMacSummaryParam::SetAvgJitter(double jitter)
{
    m_AvgJitter = jitter;
    m_AvgJitterSpecified = TRUE;
}

StatsDBPhySummaryParam::StatsDBPhySummaryParam()
{
    m_SenderId = 0;
    m_RecieverId = 0;
    m_PhyIndex = -1;
    m_Utilization = 0;
    m_NumSignals = 0;
    m_NumErrorSignals = 0;

    m_AvgInterference = 0;
    m_AvgInterferenceSpecified = FALSE;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_PathLoss = 0;
    m_PathLossSpecified = FALSE;

    m_SignalPower = 0;
    m_SignalPowerSpecified = FALSE;
}

void StatsDBPhySummaryParam::SetDelay(double avgDelay)
{
    m_Delay = avgDelay;
    m_DelaySpecified = TRUE;
}

void StatsDBPhySummaryParam::SetPathLoss(double avgPathLoss)
{
    m_PathLoss = avgPathLoss;
    m_PathLossSpecified = TRUE;
}

void StatsDBPhySummaryParam::SetSignalPower(double avgSignalPower)
{
    m_SignalPower = avgSignalPower;
    m_SignalPowerSpecified = TRUE;
}

void StatsDBPhySummaryParam::SetAvgInterference(double avgInterference)
{
    m_AvgInterference = avgInterference;
    m_AvgInterferenceSpecified = TRUE;
}


///////////////////////////////////////////////////////////////////////
//Events Tables definitions
///////////////////////////////////////////////////////////////////////


StatsDBAppEventParam::StatsDBAppEventParam()
{
    m_NodeId = 0;
    m_ReceiverId = 0;
    m_TargetAddr.networkType = NETWORK_IPV4;
    m_TargetAddr.interfaceAddr.ipv4 = ANY_DEST;
    m_TargetAddrSpecified = TRUE;
    //m_MessageId = "";
    //m_EventType = "";

    m_ApplicationType[0] = '\0';
    m_ApplicationName[0] = '\0';

    m_MsgSize = 0;
    m_MsgSizeSpecified = FALSE;

    m_MsgSeqNum = 0;
    m_MsgSeqNumSpecified = FALSE;

    m_FragId = 0;
    m_FragIdSpecified = FALSE;

    m_SessionId = 0;
    m_SessionIdSpecified = FALSE;

    m_Priority = 0;
    m_PrioritySpecified = FALSE;

    //m_MsgFailureType = "";
    m_MsgFailureTypeSpecified = FALSE;
    m_fragEnabled = FALSE;

    m_Delay = 0;
    m_DelaySpecified = FALSE;

    m_Jitter = 0;
    m_JitterSpecified = FALSE;

    m_TotalMsgSize = 0;
    m_IsFragmentation = FALSE;

    m_PktCreationTime = 0;
    m_PktCreationTimeSpecified = FALSE;

    m_SocketInterfaceMsgIdSpecified = FALSE;
}

void StatsDBAppEventParam::SetReceiverAddr(Address* receiverAddr)
{
    MAPPING_AddressCopy(&m_TargetAddr, receiverAddr);
    m_TargetAddrSpecified = TRUE;
}

void StatsDBAppEventParam::SetReceiverAddr(NodeAddress receiverAddr)
{
    MAPPING_SetAddress(NETWORK_IPV4, &m_TargetAddr, &receiverAddr);
    m_TargetAddrSpecified = TRUE;
}

void StatsDBAppEventParam::SetDelay(clocktype delay)
{
    m_Delay = delay;
    m_DelaySpecified = TRUE;
}

void StatsDBAppEventParam::SetPacketCreateTime(clocktype time)
{
    m_PktCreationTime = time;
    m_PktCreationTimeSpecified = TRUE;
}

void StatsDBAppEventParam::SetFragNum(Int32 fragNum)
{
    m_FragId = fragNum;
    m_FragIdSpecified = TRUE;
}

void StatsDBAppEventParam::SetJitter(clocktype jitter)
{
    m_Jitter = jitter;
    m_JitterSpecified = TRUE;
}

void StatsDBAppEventParam::SetMessageFailure(char* failure)
{
    strncpy(m_MsgFailureType, failure, MAX_STRING_LENGTH);
    m_MsgFailureTypeSpecified = TRUE;
}

void StatsDBAppEventParam::SetMsgSeqNum(Int32 msgSeqNum)
{
    m_MsgSeqNum = msgSeqNum;
    m_MsgSeqNumSpecified = TRUE;
}

void StatsDBAppEventParam::SetMsgSize(Int32 size)
{
    m_MsgSize = size;
    m_MsgSizeSpecified = TRUE;
}

void StatsDBAppEventParam::SetPriority(Int32 priority)
{
    m_Priority = priority;
    m_PrioritySpecified = TRUE;
}

void StatsDBAppEventParam::SetSessionId(Int32 id)
{
    m_SessionId = id;
    m_SessionIdSpecified = TRUE;
}

void StatsDBAppEventParam::SetAppType(const char* appType)
{
    if (appType)
    {
        strncpy(m_ApplicationType, appType, MAX_STRING_LENGTH);
    }
}

void StatsDBAppEventParam::SetAppName(const char* appName)
{
    if (appName)
    {
        strncpy(m_ApplicationName, appName, MAX_STRING_LENGTH);
    }
}

StatsDBTransportEventParam::StatsDBTransportEventParam(Int32 nodeId,
                                                       char* msgId,
                                                       Int32 size)
{
    m_NodeId = nodeId;
    strcpy(m_MessageId, msgId);

    m_MsgSize = size;

    m_MsgSeqNum = 0;
    m_MsgSeqNumSpecified = FALSE;

//    m_AppFragId = 0;
//    m_AppFragIdSpecified = FALSE;

//    m_TcpSeqNumber = 0;
//    m_TcpSeqNumberSpecified = FALSE;

    m_ConnTypeSpecified = FALSE;

    m_HeaderSize = 0;
    m_HdrSizeSpecified = FALSE;

    //m_Priority = 0;
    //m_PrioritySpecified = FALSE;

    m_FlagsSpecified = FALSE;
    m_FailureTypeSpecified = FALSE;

}

StatsDBTransportEventParam::StatsDBTransportEventParam(
    Int32 nodeId,
    const std::string& msgId,
    Int32 size)
{
    m_NodeId = nodeId;
    //strcpy(m_MessageId, msgId);
    strcpy(m_MessageId, msgId.c_str());

    m_MsgSize = size;

    m_MsgSeqNum = 0;
    m_MsgSeqNumSpecified = FALSE;

//    m_AppFragId = 0;
//    m_AppFragIdSpecified = FALSE;

//    m_TcpSeqNumber = 0;
//    m_TcpSeqNumberSpecified = FALSE;

    m_ConnTypeSpecified = FALSE;

    m_HeaderSize = 0;
    m_HdrSizeSpecified = FALSE;

//    m_Priority = 0;
//    m_PrioritySpecified = FALSE;

    m_FlagsSpecified = FALSE;
    m_FailureTypeSpecified = FALSE;

}

void StatsDBTransportEventParam::SetConnectionType(
    const std::string & type)
{
    m_ConnectionType = type;
    m_ConnTypeSpecified = TRUE;
}

void StatsDBTransportEventParam::SetHdrSize(Int32 size)
{
    m_HeaderSize = size;
    m_HdrSizeSpecified = TRUE;
}

void StatsDBTransportEventParam::SetMsgSeqNum(Int32 msgSeqNum)
{
    m_MsgSeqNum = msgSeqNum;
    m_MsgSeqNumSpecified = TRUE;
}

//void StatsDBTransportEventParam::SetAppFragNum(int appFragNum)
//{
//    m_AppFragId = appFragNum;
//    m_AppFragIdSpecified = TRUE;
//}

//void StatsDBTransportEventParam::SetTcpSeqNum(int tcpSeqNum)
//{
//    m_TcpSeqNumber = tcpSeqNum;
//    m_TcpSeqNumberSpecified = TRUE;
//}

//void StatsDBTransportEventParam::SetPriority(int priority)
//{
//    m_Priority = priority;
//    m_PrioritySpecified = TRUE;
//}

void StatsDBTransportEventParam::SetFlags(const std::string &flags)
{
    m_Flags = flags;
    m_FlagsSpecified = TRUE;
}

void StatsDBTransportEventParam::SetEventType(const std::string &eventType)
{
    m_EventType = eventType;
    m_EventTypeSpecified = TRUE;
}

void StatsDBTransportEventParam::SetMessageFailure(char* failure)
{
    strncpy(m_FailureType, failure, MAX_STRING_LENGTH);
    m_FailureTypeSpecified = TRUE;
}

void StatsDBTransportEventParam::SetPktSendTime(clocktype time)
{
    transPktSendTime = time;
    transPktSendTimeSpecified = TRUE;
}

StatsDBNetworkEventParam::StatsDBNetworkEventParam()
{
    m_NodeId = 0;
    m_SenderAddr = 0;
    m_ReceiverAddr = 0;
    m_MsgSize = 0;

    m_MsgSeqNum = 0;
    m_MsgSeqNumSpecified = FALSE;


    m_HeaderSize = 0;
    m_HdrSizeSpecified = FALSE;

    m_Priority = 0;
    m_PrioritySpecified = FALSE;

    //m_ProtocolType = 0;
    m_ProtocolTypeSpecified = FALSE;

    //m_PktType = "";
    m_PktTypeSpecified = FALSE;

    m_InterfaceIndex = 0;
    m_InterfaceIndexSpecified = FALSE;

    m_HopCount = 0;
    m_HopCountSpecified = FALSE;

}


void StatsDBNetworkEventParam::SetHdrSize(Int32 size)
{
    m_HeaderSize = size;
    m_HdrSizeSpecified = TRUE;
}

void StatsDBNetworkEventParam::SetHopCount(double count)
{
    m_HopCount = count;
    m_HopCountSpecified = TRUE;
}

void StatsDBNetworkEventParam::SetInterfaceIndex(Int32 index)
{
    m_InterfaceIndex = index;
    m_InterfaceIndexSpecified = TRUE;
}

void StatsDBNetworkEventParam::SetMsgSeqNum(Int32 msgSeqNum)
{
    m_MsgSeqNum = msgSeqNum;
    m_MsgSeqNumSpecified = TRUE;
}

void StatsDBNetworkEventParam::SetPktType(char type)
{
    m_PktType = type;
    m_PktTypeSpecified = TRUE;
}

void StatsDBNetworkEventParam::SetPriority(Int32 priority)
{
    m_Priority = priority;
    m_PrioritySpecified = TRUE;
}

void StatsDBNetworkEventParam::SetProtocolType(char type)
{
    m_ProtocolType = type;
    m_ProtocolTypeSpecified = TRUE;
}

StatsDBMacEventParam::StatsDBMacEventParam(
    Int32 nodeId,
    const std::string& msgId,
    Int32 interfaceIndex,
    Int32 size,
    const std::string& eventType)
{
    m_NodeId = nodeId;
    m_MessageId = msgId;
    m_InterfaceIndex = interfaceIndex;
    m_MsgSize = size;
    m_EventType = eventType;

    m_MsgSeqNum = 0;
    m_MsgSeqNumSpecified = FALSE;

    //m_AppFragId = 0;
    //m_AppFragIdSpecified = FALSE;

    //m_TransportSeqNumber = 0;
    //m_TransportSeqNumberSpecified = FALSE;

    //m_NetworkFragNumber = 0;
    //m_NetworkFragNumSpecified = FALSE;

    m_ChannelIndex = 0;
    m_ChannelIndexSpecified = FALSE;

    m_FailureType = "";
    m_FailureTypeSpecified = FALSE;

    m_HeaderSize = 0;
    m_HdrSizeSpecified = FALSE;

    //m_Priority = 0;
    //m_PrioritySpecified = FALSE;

    m_FrameType = "";
    m_FrameTypeSpecified = FALSE;

    m_DstAddrStr = "";
    m_DstAddrSpecified = FALSE;

    m_SrcAddrStr = "";
    m_SrcAddrSpecified = FALSE;
}
void StatsDBMacEventParam::SetMsgId(const std::string &msgId)
{
    m_MessageId = msgId;
}
void StatsDBMacEventParam::SetMsgSize(Int32 msgSize)
{
    m_MsgSize = msgSize;
}
void StatsDBMacEventParam::SetMsgSeqNum(Int32 msgSeqNum)
{
    m_MsgSeqNum = msgSeqNum;
    m_MsgSeqNumSpecified = TRUE;
}
void StatsDBMacEventParam::SetMsgEventType(const std::string &eventType)
{
    m_EventType = eventType;
}
//void StatsDBMacEventParam::SetAppFragId(int appFragId)
//{
//    m_AppFragId = appFragId;
//    m_AppFragIdSpecified = TRUE;
//}

//void StatsDBMacEventParam::SetTransportSeqNumber(int tcpSeqNumber)
//{
//    m_TransportSeqNumber = tcpSeqNumber;
//    m_TransportSeqNumberSpecified = TRUE;
//}

//void StatsDBMacEventParam::SetNetworkFragId(int netFragId)
//{
//    m_NetworkFragNumber = netFragId;
//    m_NetworkFragNumSpecified = TRUE;
//}

void StatsDBMacEventParam::SetChannelIndex(Int32 index)
{
    m_ChannelIndex = index;
    m_ChannelIndexSpecified = TRUE;
}

void StatsDBMacEventParam::SetFailureType(const std::string &type)
{
    m_FailureType = type;
    m_FailureTypeSpecified = TRUE;
}

void StatsDBMacEventParam::SetFrameType(const std::string &type)
{
    m_FrameType = type;
    m_FrameTypeSpecified = TRUE;
}

void StatsDBMacEventParam::SetHdrSize(Int32 size)
{
    m_HeaderSize = size;
    m_HdrSizeSpecified = TRUE;
}

void StatsDBMacEventParam::SetDstAddr(const std::string &dstAddrStr)
{
    m_DstAddrStr = dstAddrStr;
    m_DstAddrSpecified = TRUE;
}

void StatsDBMacEventParam::SetSrcAddr(const std::string &srcAddrStr)
{
    m_SrcAddrStr = srcAddrStr;
    m_SrcAddrSpecified = TRUE;
}

StatsDBPhyEventParam::StatsDBPhyEventParam(Int32 nodeId,
                                           std::string messageId,
                                           Int32 phyIndex,
                                           Int32 msgSize,
                                           std::string eventType)
{
    m_NodeId = nodeId;
    m_MessageId = messageId;
    m_PhyIndex = phyIndex;
    m_MsgSize = msgSize;
    m_EventType = eventType;

    m_ChannelIndexSpecified = FALSE;
    m_ControlSizeSpecified = FALSE;
    m_MessageFailureTypeSpecified = FALSE;
    m_SignalPowerSpecified = FALSE;
    m_InterferenceSpecified = FALSE;
    m_PathLossSpecified = FALSE;
}

void StatsDBPhyEventParam::SetChannelIndex(Int32 channel)
{
    m_ChannelIndex = channel;
    m_ChannelIndexSpecified = TRUE;
}

void StatsDBPhyEventParam::SetControlSize(Int32 size)
{
    m_ControlSize = size;
    m_ControlSizeSpecified = TRUE;
}

void StatsDBPhyEventParam::SetInterference(double interference)
{
    m_Interference = interference;
    m_InterferenceSpecified = TRUE;
}

void StatsDBPhyEventParam::SetMessageFailureType(std::string type)
{
    m_MessageFailureType = type;
    m_MessageFailureTypeSpecified = TRUE;
}

void StatsDBPhyEventParam::SetPathLoss(double pathLoss)
{
    m_PathLoss = pathLoss;
    m_PathLossSpecified = TRUE;
}

void StatsDBPhyEventParam::SetSignalPower(double signalPower)
{
    m_SignalPower = signalPower;
    m_SignalPowerSpecified = TRUE;
}


///////////////////////////////////////////////////////////////////////
// Connectivity table definitions
///////////////////////////////////////////////////////////////////////


StatsDBNetworkConnParam::StatsDBNetworkConnParam()
{
    m_NodeId = 0;
    m_DstAddress = "";
    m_Cost = 0;

    m_DstNetMask = "";
    m_DstMaskSpecified = FALSE;

    m_OutgoingIntIndex = 0;
    m_OutgoingIntIndexSpecified = FALSE;

    m_NextHopAddr = "";
    m_NextHopAddrSpecified = FALSE;

    m_RoutingProtocolType = "";
    m_RoutingProtocolSpecified = FALSE;

    m_AdminDistance = 0;
    m_AdminDistanceSpecified = FALSE;
}

void StatsDBNetworkConnParam::SetAdminDistance(Int32 distance)
{
    m_AdminDistance = distance;
    m_AdminDistanceSpecified = TRUE;
}

void StatsDBNetworkConnParam::SetDstnetworkMask(std::string mask)
{
    m_DstNetMask = mask;
    m_DstMaskSpecified = TRUE;
}

void StatsDBNetworkConnParam::SetOutgoingInterface(Int32 index)
{
    m_OutgoingIntIndex = index;
    m_OutgoingIntIndexSpecified = TRUE;
}

void StatsDBNetworkConnParam::SetNextHopAddr(std::string addr)
{
    m_NextHopAddr = addr;
    m_NextHopAddrSpecified = TRUE;
}

void StatsDBNetworkConnParam::SetRoutingProtocol(std::string protocol)
{
    m_RoutingProtocolType = protocol;
    m_RoutingProtocolSpecified = TRUE;
}

StatsDBPhyConnParam::StatsDBPhyConnParam()
{
    m_SenderId = 0;
    m_ReceiverId = 0;

    m_PhyIndex = 0;
    m_PhyIndexSpecified = FALSE;

    senderListening = FALSE;
    receiverListening = FALSE;

    reachableWorst = FALSE ;
}

void StatsDBPhyConnParam::SetChannelIndex(Int32 index)
{
    m_ChannelIndex = index;
    m_ChannelIndexSpecified = TRUE;
}

void StatsDBPhyConnParam::SetPhyIndex(Int32 index)
{
    m_PhyIndex = index;
    m_PhyIndexSpecified = TRUE;
}


//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleNodeDescTableInsert
// PURPOSE  : Insert a row into the NODE_Description table
// PARAMETERS :
// + node : Current node
// + partition : Current partition
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleNodeDescTableInsert(Node* node, PartitionData* partition)
{
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createNodeDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the node content on to the database.
    std::string queryStr = "";
    clocktype start = 0;
    double timeVal = 0.0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    
    std::vector<std::string> newValues;
    newValues.reserve(3);
    std::vector<std::string> columns;
    columns.reserve(3);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("HostName");
    newValues.push_back(std::string(node->hostname));

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        node->meta_data->m_MetaData.begin();

    while (iter != node->meta_data->m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "NODE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueDescTableInsert
// PURPOSE  : Insert a row into the QUEUE_Description table
// PARAMETERS :
// + node : Current node
// + queueDesc : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueDescTableInsert(
    Node* node,
    StatsDBQueueDesc queueDesc)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createQueueDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the node content on to the database.
    std::string queryStr = "";
    clocktype start = 0;
    double timeVal = 0.0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    
    std::vector<std::string> newValues;
    newValues.reserve(8);
    std::vector<std::string> columns;
    columns.reserve(8);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(queueDesc.m_InterfaceIndex));
    columns.push_back("QueueType");
    newValues.push_back(queueDesc.m_QueueType);
    columns.push_back("QueueIndex");
    newValues.push_back(STATSDB_IntToString(queueDesc.m_QueueIndex));
    
    if (queueDesc.m_QueueDisciplineSpecified &&
        db->statsQueueDesc->isDiscipline)
    {
        columns.push_back("QueueDiscipline");
        newValues.push_back(queueDesc.m_QueueDiscipline);
    }
    if (queueDesc.m_QueuePrioritySpecified &&
        db->statsQueueDesc->isPriority)
    {
        columns.push_back("QueuePriority");
        newValues.push_back(STATSDB_IntToString(queueDesc.m_QueuePriority));
    }
    if (queueDesc.m_QueueSizeSpecified && db->statsQueueDesc->isSize)
    {
        columns.push_back("QueueSize");
        newValues.push_back(STATSDB_IntToString(queueDesc.m_QueueSize));
    }
    std::map<std::string, std::string>::iterator iter =
        queueDesc.m_QueueMetaData.m_MetaData.begin();
    while (iter != queueDesc.m_QueueMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "QUEUE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceDescTableInsert
// PURPOSE  : Insert a new row into the INTERFACE_Description table
// PARAMETERS :
// + node : The node that the interface belongs to
// + interfaceDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceDescTableInsert(
    Node* node,
    StatsDBInterfaceDesc interfaceDesc)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createInterfaceDescTable)
    {
        // Table does not exist
        return;
    }
    // In this table we insert the interface content on to the database.
    clocktype start = 0;
    size_t i;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    
    std::vector<std::string> newValues;
    std::vector<std::string> columns;
    if (db->statsInterfaceDesc->interfaceDescTableDef.size() > 0)
    {
        newValues.reserve(
            db->statsInterfaceDesc->interfaceDescTableDef.size() * 9);
        columns.reserve(
            db->statsInterfaceDesc->interfaceDescTableDef.size() * 9);
    }

    for (i = 0; i < db->statsInterfaceDesc->interfaceDescTableDef.size(); i++)
    {
        if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "Timestamp")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(
                STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "NodeId")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(STATSDB_IntToString(node->nodeId));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceIndex")
        {
            columns.push_back(
                db->statsInterfaceDesc->interfaceDescTableDef[i].first);
            newValues.push_back(
                STATSDB_IntToString(interfaceDesc.m_InterfaceIndex));
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceName")
        {
            if (interfaceDesc.m_InterfaceNameSpecified &&
                db->statsInterfaceDesc->isName)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_InterfaceName);
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "InterfaceAddress")
        {
            if (interfaceDesc.m_InterfaceAddrSpecified &&
                db->statsInterfaceDesc->isAddress)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_InterfaceAddr);
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "SubnetMask")
        {
            if (interfaceDesc.m_SubnetMaskSpecified &&
                db->statsInterfaceDesc->isSubnetMask)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_SubnetMask);
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "RoutingProtocol")
        {
            if (interfaceDesc.m_NetworkTypeSpecified &&
                db->statsInterfaceDesc->isNetworkProtocol)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_NetworkType);
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "MulticastProtocol")
        {
            if (interfaceDesc.m_MulticastProtocolSpecified &&
                db->statsInterfaceDesc->isMulticastProtocol)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(interfaceDesc.m_MulticastProtocol);
            }
        }
        else if (db->statsInterfaceDesc->interfaceDescTableDef[i].first ==
            "SubnetId")
        {
#ifdef ADDON_BOEINGFCS
            if (interfaceDesc.m_SubetIdSpecififed &&
                db->statsInterfaceDesc->isSubnetId)
            {
                columns.push_back(
                    db->statsInterfaceDesc->interfaceDescTableDef[i].first);
                newValues.push_back(
                    STATSDB_IntToString(interfaceDesc.m_SubnetId));
            }
#endif /* ADDON_BOEINGFCS */
        }
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        interfaceDesc.m_InterfaceMetaData.m_MetaData.begin();

    while (iter != interfaceDesc.m_InterfaceMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }
    
    InsertValues(db, "INTERFACE_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSchedulerDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSchedulerDescTableInsert(
    Node* node,
    Int32 interfaceIndex,
    const string& schedulerType,
    const string& schedulerAlgorithm)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createSchedulerDescTable)
    {
        // Table does not exist
        return;
    }

    StatsDBSchedulerDesc schedulerDesc(node->nodeId, interfaceIndex,
        schedulerType, schedulerAlgorithm);

    schedulerDesc.m_SchedulerMetaData.AddSchedulerMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    clocktype start = 0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(
        STATSDB_InterfaceToString(schedulerDesc.m_InterfaceIndex));
    columns.push_back("SchedulerType");
    newValues.push_back(schedulerDesc.m_SchedulerType);

    if (db->statsSchedulerAlgo->isSchedulerAlgo)
    {
        columns.push_back("SchedulerAlgorithm");
        newValues.push_back(schedulerDesc.m_SchedulingAlgorithm);
    }
    
    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        schedulerDesc.m_SchedulerMetaData.m_MetaData.begin();

    while (iter != schedulerDesc.m_SchedulerMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    // Now to insert the content into the table.
    InsertValues(db, "SCHEDULER_Description", columns, newValues);
}

string STATSDB_ConvertAddressToString(const Address &address)
{
    //std::string addressStr;
    char addressString[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((Address *)&address, addressString);

    return addressString;
}

string STATSDB_ConvertAddressToString(const NodeAddress &address)
{
    //std::string addressStr;
    char addressString[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((NodeAddress )address, addressString);

    return addressString;
}

std::string STATSDB_InterfaceToString(Int32 index)
{
    if (index >= 0)
    {
        return STATSDB_IntToString(index);
    }
    else if (index == CPU_INTERFACE)
    {
        return "CPU";
    }else
        return "BACKPLANE";
}

std::string STATSDB_ChannelToString(
        Node* node,
        Int32 interfaceIndex,
        Int32 channelIndex)
{
    if (channelIndex >= 0)
    {
        return STATSDB_IntToString(channelIndex);
    }

    std::string buf;
    switch(node->macData[interfaceIndex]->macProtocol )
    {
#ifdef ADDON_BOEINGFCS
    case MAC_PROTOCOL_CES_WINTNCW:
        buf = "WINTNCW";
        break;
    case MAC_PROTOCOL_CES_WINTGBS:
        buf = "WINTGBS";
        break;
#endif
    case MAC_PROTOCOL_SATCOM:
        buf = "SATCOM";
        break ;
    case MAC_PROTOCOL_LINK:
        buf = "LINK";
        break;
    case MAC_PROTOCOL_802_3:
        buf = "ETHERNET";
        break ;
    default:
        ERROR_Assert(FALSE, "ERROR in STATSDB_ChannelToString.");
    }
    return buf;
}
string STATSDB_IntToString(Int32 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%d", num);
    return (string) buf;
}

string STATSDB_DoubleToString(double f)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%.9f", f + 1.0e-10);
    return (string) buf;
}

string STATSDB_Int64ToString(Int64 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%lld", num);
    return (string) buf;
}

string STATSDB_UInt64ToString(UInt64 num)
{
    char buf[MAX_STRING_LENGTH];
    sprintf(buf, "%llu", num);
    return (string) buf;
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSessionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (!db ||!db->statsDescTable->createSessionDescTable)
    {
        // Table does not exist
        return;
    }

    Int32 senderId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                           clientAddr);

    Int32 receiverId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                             serverAddr);

    // actually sessionDesc seems useless...
    StatsDBSessionDesc sessionDesc(sessionId, senderId, receiverId);

    sessionDesc.m_SessionMetaData.AddSessionMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    sessionDesc.SetSenderAddr(STATSDB_ConvertAddressToString(clientAddr));
    sessionDesc.SetReceiverAddr(STATSDB_ConvertAddressToString(serverAddr));
    sessionDesc.SetSenderPort(clientPort);
    sessionDesc.SetRecvPort(serverPort);
    sessionDesc.SetTransportProtocol(transportProtocol);
    sessionDesc.SetAppType(appType);

    // In this table we insert the node content on to the database.
    clocktype start = 0;
    double timeVal = 0.0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    std::vector<std::string> newValues;
    newValues.reserve(10);
    std::vector<std::string> columns;
    columns.reserve(10);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SessionId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_SessionId));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(sessionDesc.m_ReceiverId));

    if (sessionDesc.m_SenderAddrSpecified &&
        db->statsSessionDesc->isSenderAddr)
    {
        columns.push_back("SenderAddr");
        newValues.push_back(sessionDesc.m_SenderAddr);
    }
    if (sessionDesc.m_ReceiverAddrSpecified &&
        db->statsSessionDesc->isReceiverAddr)
    {
        columns.push_back("ReceiverAddr");
        newValues.push_back(sessionDesc.m_ReceiverAddr);
    }
    if (sessionDesc.m_SrcPortSpecified &&
        db->statsSessionDesc->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(STATSDB_IntToString(sessionDesc.m_SenderPort));
    }
    if (sessionDesc.m_RecvPortSpecified &&
        db->statsSessionDesc->isRecvPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(STATSDB_IntToString(sessionDesc.m_RecvPort));
    }
    if (sessionDesc.m_TransportPortocolSpecified &&
        db->statsSessionDesc->isTransportProtocol)
    {
        columns.push_back("TransportProtocol");
        newValues.push_back(sessionDesc.m_TransportProtocol);
    }
    if (sessionDesc.m_AppTypeSpecified &&
        db->statsSessionDesc->isAppType)
    {
        columns.push_back("AppType");
        newValues.push_back(sessionDesc.m_AppType);
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        sessionDesc.m_SessionMetaData.m_MetaData.begin();

    while (iter != sessionDesc.m_SessionMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    // Now to insert the content into the table.
    InsertValues(db, "SESSION_Description", columns, newValues);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleSessionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Int32 sessionId,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    NodeAddress convertedClientAddr = clientAddr.interfaceAddr.ipv4;
    NodeAddress convertedServerAddr = serverAddr.interfaceAddr.ipv4;

    STATSDB_HandleSessionDescTableInsert(
        node, sessionId, convertedClientAddr, convertedServerAddr,
        clientPort, serverPort, appType, transportProtocol);

}

void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const NodeAddress &clientAddr,
    const NodeAddress &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    Int32* sessionIdInfo =  (Int32 *)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbAppSessionId);
    ERROR_Assert(sessionIdInfo,
        "Errror in STATSDB_HandleSessionDescTableInsert.");

    STATSDB_HandleSessionDescTableInsert(
        node, *sessionIdInfo, clientAddr, serverAddr,
        clientPort, serverPort, appType, transportProtocol);

}
void STATSDB_HandleSessionDescTableInsert(
    Node* node,
    Message* msg,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 clientPort,
    Int32 serverPort,
    const std::string& appType,
    const std::string& transportProtocol)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || !db->statsDescTable->createSessionDescTable)
    {
        return ;
    }

    Int32* sessionIdInfo =  (Int32 *)
        MESSAGE_ReturnInfo(msg, INFO_TYPE_StatsDbAppSessionId);
    ERROR_Assert(sessionIdInfo,
        "Errror in STATSDB_HandleSessionDescTableInsert.");
    NodeAddress convertedClientAddr = clientAddr.interfaceAddr.ipv4;
    NodeAddress convertedServerAddr = serverAddr.interfaceAddr.ipv4;

    STATSDB_HandleSessionDescTableInsert(
        node, *sessionIdInfo, convertedClientAddr, convertedServerAddr,
        clientPort, serverPort, appType, transportProtocol);

}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleConnectionDescTableInsert
// PURPOSE  : Insert a new row into the SCHEDULER_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleConnectionDescTableInsert(Node* node,
    /*int appSessionId,*/
    const Address & local_addr,
    const Address & remote_addr,
    short local_port,
    short remote_port,
    const std::string &connection_type)
{

    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    double timeVal = 0.0;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createConnectionDescTable)
    {
        // Table does not exist
        return;
    }

    timeVal = (double) getSimTime(node) / SECOND;
    // actually connectionDesc seems useless...
    Int32 senderId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                           local_addr);

    Int32 receiverId = MAPPING_GetNodeIdFromInterfaceAddress(node,
                                                             remote_addr);
    StatsDBConnectionDesc connectionDesc(timeVal,/* appSessionId,*/ senderId, receiverId);

    connectionDesc.SetSenderAddr(STATSDB_ConvertAddressToString(local_addr));
    connectionDesc.SetReceiverAddr(STATSDB_ConvertAddressToString(remote_addr));
    connectionDesc.SetSenderPort(local_port);
    connectionDesc.SetRecvPort(remote_port);
    connectionDesc.SetConnectionType(connection_type);
    if (local_addr.networkType == NETWORK_IPV4)
    {
        connectionDesc.SetNetworkProtocol("NETWORK_IPV4");
    }
    else {
        connectionDesc.SetNetworkProtocol("NETWORK_IPV6");
    }

    connectionDesc.m_ConnectionMetaData.AddConnectionMetaData(node,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    clocktype start = 0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    
    std::vector<std::string> columns;
    columns.reserve(9);
    std::vector<std::string> newValues;
    newValues.reserve(9);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(connectionDesc.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(connectionDesc.m_ReceiverId));

    if (connectionDesc.m_ReceiverAddrSpecified &&
        db->statsConnectionDesc->isSenderAddr)
    {
        columns.push_back("SenderAddr");
        newValues.push_back(connectionDesc.m_SenderAddr);
    }
    if (connectionDesc.m_ReceiverAddrSpecified &&
        db->statsConnectionDesc->isReceiverAddr)
    {
        columns.push_back("ReceiverAddr");
        newValues.push_back(connectionDesc.m_ReceiverAddr);
    }
    if (connectionDesc.m_SrcPortSpecified &&
        db->statsConnectionDesc->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(STATSDB_IntToString(connectionDesc.m_SenderPort));
    }
    if (connectionDesc.m_RecvPortSpecified &&
        db->statsConnectionDesc->isRecvPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(STATSDB_IntToString(connectionDesc.m_RecvPort));
    }
    if (connectionDesc.m_ConnTypeSpecified &&
        db->statsConnectionDesc->isConnectionType)
    {
        columns.push_back("ConnectionType");
        newValues.push_back(connectionDesc.m_ConnectionType);
    }
    if (connectionDesc.m_NetworkPortocolSpecified &&
        db->statsConnectionDesc->isNetworkProtocol)
    {
        columns.push_back("NetworkProtocol");
        newValues.push_back(connectionDesc.m_NetworkProtocol);
    }

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        connectionDesc.m_ConnectionMetaData.m_MetaData.begin();

    while (iter != connectionDesc.m_ConnectionMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "CONNECTION_Description", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyDescTableInsert
// PURPOSE  : Insert a new row into the PHY_Description table
// PARAMETERS :
// + node : The node that the scheduler belongs to
// + schedulerDesc : Structure containing the values to insert into the
//                   table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyDescTableInsert(
    Node* node,
    Int32 interfaceIndex,
    Int32 phyIndex)
{
    PartitionData* partition = node->partitionData;
    StatsDb* db = NULL;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsDescTable->createPhyDescTable)
    {
        // Table does not exist
        return;
    }

    StatsDBPhyDesc phyDesc(node->nodeId, interfaceIndex,
        phyIndex);

    phyDesc.m_PhyMetaData.AddPhyMetaData(node,
        interfaceIndex,
        node->partitionData,
        node->partitionData->nodeInput);

    // In this table we insert the node content on to the database.
    clocktype start = 0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    std::vector<std::string> newValues;
    newValues.reserve(4);
    std::vector<std::string> columns;
    columns.reserve(4);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(phyDesc.m_InterfaceIndex));
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyDesc.m_PhyIndex));

    // Add the meta data columns.
    std::map<std::string, std::string>::iterator iter =
        phyDesc.m_PhyMetaData.m_MetaData.begin();

    while (iter != phyDesc.m_PhyMetaData.m_MetaData.end())
    {
        columns.push_back(iter->first);
        newValues.push_back(iter->second);
        iter++;
    }

    InsertValues(db, "PHY_Description", columns, newValues);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleNodeStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleNodeStatusTableInsert(Node* node, StatsDBNodeStatus nodeStatus)
{
    StatsDb* db = NULL;
    PartitionData* partition = node->partitionData;
    db = partition->statsDb;

    // Check if the Table exists.
    if (db == NULL || !db->statsStatusTable->createNodeStatusTable)
    {
        // Table does not exist
        return;
    }

    // In this table we insert the node content on to the database.
    clocktype start = 0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("TriggeredUpdate");
    if (nodeStatus.m_TriggeredUpdate)
    {
        newValues.push_back("TRUE");
    }
    else {
        newValues.push_back("FALSE");
    }

    if (db->statsNodeStatus->isPosition)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            columns.push_back("X");
            columns.push_back("Y");
            columns.push_back("Z");
            if (nodeStatus.m_PositionUpdated)
            {
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOnePosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoPosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreePosition));
            }
            else
            {
                newValues.push_back("NULL");
                newValues.push_back("NULL");
                newValues.push_back("NULL");
            }
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            columns.push_back("Lat");
            columns.push_back("Lon");
            columns.push_back("Alt");
            if (nodeStatus.m_PositionUpdated)
            {
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOnePosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoPosition));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreePosition));
            }
            else
            {
                newValues.push_back("NULL");
                newValues.push_back("NULL");
                newValues.push_back("NULL");
            }
        }
    }
    if (db->statsNodeStatus->isVelocity)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            columns.push_back("XVelocity");
            columns.push_back("YVelocity");
            columns.push_back("ZVelocity");
            if (nodeStatus.m_PositionUpdated)
            {
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOneVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreeVelocity));
            }
            else
            {
                newValues.push_back("NULL");
                newValues.push_back("NULL");
                newValues.push_back("NULL");
            }
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            columns.push_back("LatVelocity");
            columns.push_back("LonVelocity");
            columns.push_back("AltVelocity");
            if (nodeStatus.m_PositionUpdated)
            {
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionOneVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionTwoVelocity));
                newValues.push_back(
                    STATSDB_DoubleToString(nodeStatus.m_DimensionThreeVelocity));
            }
            else
            {
                newValues.push_back("NULL");
                newValues.push_back("NULL");
                newValues.push_back("NULL");
            }
        }
    }
    if (db->statsNodeStatus->isActiveState)
    {
        columns.push_back("ActiveState");
        if (nodeStatus.m_ActiveStateUpdated)
        {
            if (nodeStatus.m_Active == STATS_DB_Enabled)
            {
                newValues.push_back("Enabled");
            }
            else
            {
                newValues.push_back("Disabled");
            }
        }
        else
        {
            newValues.push_back("NULL");
        }
    }
    if (db->statsNodeStatus->isDamageState)
    {
        columns.push_back("DamageState");
        if (nodeStatus.m_DamageStateUpdated)
        {
            if (nodeStatus.m_DamageState == STATS_DB_Damaged)
            {
                newValues.push_back("Damaged");
            }
            else if (nodeStatus.m_DamageState == STATS_DB_Undamaged)
            {
                newValues.push_back("Undamaged");
            }
        }
        else
        {
            newValues.push_back("NULL");
        }
    }
    if (db->statsNodeStatus->isGateway)
    {
        columns.push_back("IsGateway");
        if (nodeStatus.m_IsGateway)
        {
            newValues.push_back("YES");
        }
        else
        {
            newValues.push_back("NO");
        }
    }
    // Now to insert the content into the table.
    InsertValues(db, "NODE_Status", columns, newValues);
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMulticastStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMulticastStatusTableInsert(Node *node)
{
    if (node != node->partitionData->firstNode)
    {
        return ;
    }

    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsStatusTable->createMulticastStatusTable)
    {
        return ;
    }

    StatsDBStatusTable::Const_MultiStatusIter citer =
        db->statsStatusTable->map_MultiStatus.begin() ;

    for (; citer != db->statsStatusTable->map_MultiStatus.end(); ++citer)
    {
        /*if (citer->second->joinPrint && citer->second->leavePrint )
        {
            continue ;
        }
        if (citer->second->joinPrint &&
            !citer->second->leavePrint && citer->second->timeLeft.empty())
        {
            continue ;
        }
        */

        std::vector<std::string> newValues;
        newValues.reserve(6);
        std::vector<std::string> columns;
        columns.reserve(6);

        columns.push_back("Timestamp");
        newValues.push_back(
            STATSDB_DoubleToString((double) getSimTime(node) / SECOND));

        char srcAddrStr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(citer->first.first, srcAddrStr);
        columns.push_back("NodeAddress");
        newValues.push_back(std::string(srcAddrStr));
        
        char grpAddrStr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(citer->first.second, grpAddrStr);
        columns.push_back("GroupAddress");
        newValues.push_back(std::string(grpAddrStr));

        columns.push_back("JoiningTime");
        newValues.push_back(std::string(citer->second->timeJoined));
        columns.push_back("LeavingTime");
        newValues.push_back(std::string(citer->second->timeLeft));
        columns.push_back("GroupName");
        newValues.push_back(std::string(citer->second->groupName));

        /*if (!citer->second->timeJoined.empty() && !citer->second->timeLeft.empty())
        {
            citer->second->joinPrint = TRUE;
            citer->second->leavePrint = TRUE;
        }else if (!citer->second->timeJoined.empty() )
        {
            citer->second->joinPrint = TRUE;
        }else
            ERROR_Assert(FALSE, "Error in STATSDB_HandleMulticastStatusTableInsert.");
            */
        // Now to insert the content into the table.

        InsertValues(db, "MULTICAST_Status", columns, newValues);
    }
    return ;
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMulticastStatusTableInsert
// PURPOSE  : Insert a row into the NODE_Status table
// PARAMETERS :
// + node : Current node
// + nodeStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMulticastStatusTableInsert(Node* node, Message *msg)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        /*ERROR_ReportWarning("Unable to extract DB information for Node Status\n");*/
        return ;
    }

    // Time to insert the status stats in the database.

    if (db->statsStatusTable->createMulticastStatusTable)
    {
        STATSDB_HandleMulticastStatusTableInsert(node);
    }
}

void STATSDB_HandleInterfaceStatusTableInsert(Node *node,
    BOOL triggeredUpdate)
{
    Int32 i;
    for (i = 0; i < node->numberInterfaces; ++i)
    {
        STATSDB_HandleInterfaceStatusTableInsert(node,
            triggeredUpdate, i);
    }
}
void STATSDB_HandleInterfaceStatusTableInsert(
    Node* node,
    BOOL triggeredUpdate,
    Int32 interfaceIndex)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return ;
    }
    // Time to insert the status stats in the database.
    if (db->statsStatusTable->createInterfaceStatusTable == FALSE)
    {
        return ;
    }
    StatsDBInterfaceStatus interfaceStatus;
    char interfaceAddrStr[100];
    NetworkIpGetInterfaceAddressString(node, interfaceIndex, interfaceAddrStr);
    interfaceStatus.m_triggeredUpdate = triggeredUpdate;
    interfaceStatus.m_address = interfaceAddrStr;
    interfaceStatus.m_interfaceEnabled =
        NetworkIpInterfaceIsEnabled(node, interfaceIndex);

    STATSDB_HandleInterfaceStatusTableInsert(node, interfaceStatus);
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceStatusTableInsert
// PURPOSE  : Insert a row into the INTERFACE_Status table
// PARAMETERS :
// + node : Current node
// + interfaceStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceStatusTableInsert(PartitionData* partition,
    BOOL triggeredUpdate, Message *msg)
{
    StatsDb* db = partition->statsDb;
    Int32 i;
    if (db == NULL)
    {
        //ERROR_ReportWarning("Unable to extract DB information for Interface Status\n");
        return ;
    }
    // Time to insert the status stats in the database.
    if (db->statsStatusTable->createInterfaceStatusTable)
    {
        Node * nextNode = partition->firstNode;
        while (nextNode != NULL)
        {
            for (i = 0; i < nextNode->numberInterfaces; ++i)
            {
                STATSDB_HandleInterfaceStatusTableInsert(nextNode,
                    triggeredUpdate, i);
            }
            nextNode = nextNode->nextNodeData;
        }
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleInterfaceStatusTableInsert
// PURPOSE  : Insert a row into the INTERFACE_Status table
// PARAMETERS :
// + node : Current node
// + interfaceStatus : Structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleInterfaceStatusTableInsert(Node* node,
    StatsDBInterfaceStatus interfaceStatus)
{
    StatsDb* db = NULL;    
    PartitionData* partition = node->partitionData;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Check if the Table exists.
    if (db == NULL || !db->statsStatusTable->createInterfaceStatusTable)
    {
        // Table does not exist
        return;
    }

    // In this table we insert the node content on to the database.
    clocktype start = 0;

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeID");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("InterfaceAddress");
    newValues.push_back(interfaceStatus.m_address);

    columns.push_back("InterfaceEnabled");
    if (interfaceStatus.m_interfaceEnabled)
    {    
        newValues.push_back("TRUE");
    }
    else
    {
        newValues.push_back("FALSE");
    }
    columns.push_back("TriggeredUpdate");
    if (interfaceStatus.m_triggeredUpdate)
    {    
       newValues.push_back("TRUE");
    }
    else
    {
        newValues.push_back("FALSE");
    }

    InsertValues(db, "INTERFACE_Status", columns, newValues);
}


void STATSDB_HandleTransAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.transportBridge == NULL)
        {
            node->partitionData->stats->global.transportBridge =
                 new STAT_GlobalTransportStatisticsBridge(
                    &node->partitionData->stats->global.transportAggregate,
                    node->partitionData);
        }        
        // Compose and Insert SQL
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.transportBridge->
            composeGlobalTransportStatisticsInsertSQLString(node, node->partitionData));
    }
}

////////////////////////////////////////////////////////////////////////
// Aggregate Table Insertion
////////////////////////////////////////////////////////////////////////
void STATSDB_HandleAppAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_GlobalAppStatisticsBridge *appBridge =
        node->partitionData->stats->global.appBridge;

    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (appBridge == NULL)
        {
            node->partitionData->stats->global.appBridge
                = new STAT_GlobalAppStatisticsBridge(
                                node->partitionData->stats->global.appAggregate,
                                node->partitionData);

            appBridge = node->partitionData->stats->global.appBridge;
        }
        else
        {
            appBridge->copyFromGlobalApp(
                        node->partitionData->stats->global.appAggregate);
        }
        // Compose SQL string
        AddInsertQueryToBuffer(db,
            appBridge->composeGlobalAppStatisticsInsertSQLString(node,
                                                     node->partitionData));
    }
}


// Network Aggregate table update
void STATSDB_HandleNetworkAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_GlobalNetStatisticsBridge *netBridge =
        node->partitionData->stats->global.netBridge;

    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (netBridge == NULL)
        {
            node->partitionData->stats->global.netBridge
                = new STAT_GlobalNetStatisticsBridge(
                                node->partitionData->stats->global.netAggregate,
                                node->partitionData);

            netBridge = node->partitionData->stats->global.netBridge;
        }
        else
        {
            netBridge->copyFromGlobalNet(
                        node->partitionData->stats->global.netAggregate);
        }
        // Compose SQL string
        AddInsertQueryToBuffer(db,
            netBridge->composeGlobalNetStatisticsInsertSQLString(node,
                                                     node->partitionData));
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleMacAggregateTableInsert
// PURPOSE  : Insert a new row into the MAC_Aggregate table.
// PARAMETERS :
// + node : current node
// + macParam : a structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleMacAggregateTableInsert(Node* node)    
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.macBridge == NULL)
        {
            node->partitionData->stats->global.macBridge =
                 new STAT_GlobalMacStatisticsBridge(
                    &node->partitionData->stats->global.macAggregate,
                    node->partitionData);
        }        
        // Compose and Insert SQL
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.macBridge->
            composeGlobalMacStatisticsInsertSQLString(node, node->partitionData));
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyAggregateTableInsert
// PURPOSE  : Insert a new row into the PHY_Aggregate table.
// PARAMETERS :
// + node : current node
// + phyParam : a structure containing the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyAggregateTableInsert(
    Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsAggregateTable->createPhyAggregateTable)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.phyBridge == NULL)
        {
            node->partitionData->stats->global.phyBridge
                = new STAT_GlobalPhysicalStatisticsBridge(
                                &node->partitionData->stats->global.phyAggregate,
                                node->partitionData);
        }

        // Compose and exectue insert SQL string
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.phyBridge->composeGlobalPhysicalStatisticsInsertSQLString(
            node, node->partitionData));
    }
}

// Summary Table update
void STATSDB_HandleAppSummaryTableInsert(Node* node)
{
    std::vector<std::string> insertList;
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Pointer accessing appBridge
    STAT_AppSummaryBridge *appSummaryBridge =
        node->partitionData->stats->global.appSummaryBridge;

    node->partitionData->stats->SummarizeApp(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (appSummaryBridge == NULL)
        {
            node->partitionData->stats->global.appSummaryBridge
                = new STAT_AppSummaryBridge(
                    node->partitionData->stats->global.appUnicastSummary,
                    node->partitionData->stats->global.appMulticastSummary,
                    node->partitionData);

            appSummaryBridge =
                        node->partitionData->stats->global.appSummaryBridge;
        }
        else
        {
            appSummaryBridge->copyFromGlobalAppSummary(
                    node->partitionData->stats->global.appUnicastSummary,
                    node->partitionData->stats->global.appMulticastSummary);
        }
        // Compose SQL strings
        appSummaryBridge->composeAppSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList,
                                                    STAT_Unicast);

        appSummaryBridge->composeAppSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList,
                                                    STAT_Multicast);
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

void STATSDB_HandleMulticastAppSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check if the Table exists.
    if (!db)
    {
        return;
    }

    node->partitionData->stats->SummarizeMulticastApp(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.multicastAppSummaryBridge == NULL)
        {
                node->partitionData->stats->global.multicastAppSummaryBridge
                    = new STAT_MulticastAppSummaryBridge(
                        &node->partitionData->stats->global.appMulticastSessionSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.multicastAppSummaryBridge->composeMutlicastAppSummaryInsertSQLString(
            node, node->partitionData, &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Multicast Network Summary Table Insert
void STATSDB_HandleMulticastNetSummaryTableInsert(Node* node,
                        const StatsDBMulticastNetworkSummaryContent & stats)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check if the Table exists.
    if (!db || !db->statsSummaryTable ||
        !db->statsSummaryTable->createMulticastNetSummaryTable)
    {
        // Table does not exist
        return;
    }
    std::vector<std::string> newValues;
    newValues.reserve(7);
    std::vector<std::string> columns;
    columns.reserve(7);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));

    columns.push_back("ProtocolType");
    if (!strcmp(stats.m_ProtocolType,"PIM-SM"))
    {
        newValues.push_back("PIM-SM");
    }
    else if (!strcmp(stats.m_ProtocolType,"PIM-DM"))
    {
        newValues.push_back("PIM-DM");
    }
    else if (!strcmp(stats.m_ProtocolType,"MOSPF"))
    {
        newValues.push_back("MOSPF");
    }
    else if (!strcmp(stats.m_ProtocolType,"OTHER"))
    {
        newValues.push_back("OTHER");
    }
    else
    {
        newValues.push_back("");
    }
    columns.push_back("DataSent");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataSent));
    columns.push_back("DataReceived");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataRecvd));
    columns.push_back("DataForwarded");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataForwarded));
    columns.push_back("DataDiscarded");
    newValues.push_back(STATSDB_IntToString(stats.m_NumDataDiscarded));

    InsertValues(db, "MULTICAST_NETWORK_Summary", columns, newValues);
}

// Summary Table update
// Transport Summary
void STATSDB_HandleTransSummaryTableInsert(Node *node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeTransport(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        //TCP first
        if (node->partitionData->stats->global.transportSummaryBridge == NULL)
        {
                node->partitionData->stats->global.transportSummaryBridge
                    = new STAT_TransportSummaryBridge(
                        &node->partitionData->stats->global.transportSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.transportSummaryBridge->composeTransportSummaryInsertSQLString(
            node, node->partitionData, &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Network Summary Table Insert
void STATSDB_HandleNetworkSummaryTableInsert(Node* node)
{
    std::vector<std::string> insertList;
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    STAT_NetSummaryBridge *netSummaryBridge =
        node->partitionData->stats->global.netSummaryBridge;

    node->partitionData->stats->SummarizeNet(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (netSummaryBridge == NULL)
    {
            node->partitionData->stats->global.netSummaryBridge
                = new STAT_NetSummaryBridge(
                    node->partitionData->stats->global.netSummary,
                    node->partitionData);

            netSummaryBridge =
                        node->partitionData->stats->global.netSummaryBridge;
    }
    else
    {
            netSummaryBridge->copyFromGlobalNetSummary(
                    node->partitionData->stats->global.netSummary);
    }
        // Compose SQL strings
        netSummaryBridge->composeNetSummaryInsertSQLString(
                                                    node,
                                                    node->partitionData,
                                                    &insertList);
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

// Mac Summary Table Insert
void STATSDB_HandleMacSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeMac(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.macSummaryBridge == NULL)
        {
                node->partitionData->stats->global.macSummaryBridge
                    = new STAT_MacSummaryBridge(
                        &node->partitionData->stats->global.macSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.macSummaryBridge->composeMacSummaryInsertSQLString(
            node, node->partitionData, &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueAggregateTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Aggregate table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueAggregateTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsAggregateTable->createQueueAggregateTable)
    {
        return;
    }
    node->partitionData->stats->Aggregate(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueBridge == NULL)
        {
            node->partitionData->stats->global.queueBridge
                = new STAT_GlobalQueueStatisticsBridge(
                                &node->partitionData->stats->global.queueAggregate,
                                node->partitionData);
        }

        // Compose and exectue insert SQL string
        AddInsertQueryToBuffer(db, node->partitionData->stats->global.queueBridge->composeGlobalQueueStatisticsInsertSQLString(
            node, node->partitionData));
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueSummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueSummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeQueue(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueSummaryBridge == NULL)
        {
                node->partitionData->stats->global.queueSummaryBridge
                    = new STAT_QueueSummaryBridge(
                        &node->partitionData->stats->global.queueSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.queueSummaryBridge->composeQueueSummaryInsertSQLString(
            node, node->partitionData, &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandleQueueStatusTableInsertion
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            QUEUE_Status table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandleQueueStatusTableInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    node->partitionData->stats->SummarizeQueue(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.queueStatusBridge == NULL)
        {
                node->partitionData->stats->global.queueStatusBridge
                    = new STAT_QueueStatusBridge(
                        &node->partitionData->stats->global.queueSummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.queueStatusBridge->composeQueueStatusInsertSQLString(
            node, node->partitionData, &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}
//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhySummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            PHY_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhySummaryTableInsert(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsSummaryTable->createPhySummaryTable)
    {
        return;
    }

    node->partitionData->stats->SummarizePhy(node->partitionData);

    if (node->partitionData->partitionId == 0)
    {
        std::vector<std::string> insertList;

        // Would enter here only once(during first time of insertion)
        if (node->partitionData->stats->global.phySummaryBridge == NULL)
        {
                node->partitionData->stats->global.phySummaryBridge
                    = new STAT_PhySummaryBridge(
                        &node->partitionData->stats->global.phySummary,
                        node->partitionData);
        }

        // Compose SQL strings
        node->partitionData->stats->global.phySummaryBridge->composePhysicalSummaryInsertSQLString(
            node,
            node->partitionData,
            &insertList);
        
        ExecuteMultipleNoReturnQueries(db, insertList);
    }
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhySummaryTableInsert
// PURPOSE  : Prepare the SQL INSERT statement for a row in the
//            PHY_Summary table
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhySummaryTableInsert(Node* node, 
    const StatsDBPhySummaryParam &phyParam)
{
    char buf[MAX_STRING_LENGTH];
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderID");
    newValues.push_back(STATSDB_IntToString(phyParam.m_SenderId));
    columns.push_back("ReceiverID");
    newValues.push_back(STATSDB_IntToString(phyParam.m_RecieverId));
    // channel index is always -1 for link,satcom
    
    if (phyParam.m_ChannelIndex != -1)
    {    
        columns.push_back("ChannelIndex");
        newValues.push_back(STATSDB_IntToString(phyParam.m_ChannelIndex));
    }
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_PhyIndex));
    columns.push_back("NumRcvdSignals");
    newValues.push_back(STATSDB_UInt64ToString(phyParam.m_NumSignals));
    columns.push_back("NumRcvdErrorSignals");
    newValues.push_back(STATSDB_UInt64ToString(phyParam.m_NumErrorSignals));
    
    //utilization will be zero only for the satellite node of satcom.
    //In the case when no data is sent also it can be zero, but
    //in this case, there will be no entry for that sender/receiver pair
    
    if (phyParam.m_Utilization != 0)
    {
        columns.push_back("Utilization");
        sprintf(buf, "%.10f", phyParam.m_Utilization);
        newValues.push_back(buf);
    } 
    
    if (phyParam.m_AvgInterferenceSpecified)
    {
        columns.push_back("AverageInterference");
        newValues.push_back(
            STATSDB_DoubleToString(phyParam.m_AvgInterference));
    }
    
    if (phyParam.m_DelaySpecified)
    {
        columns.push_back("AverageDelay");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_Delay));
    }
    if (phyParam.m_PathLossSpecified)
    {
        columns.push_back("AveragePathLoss");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_PathLoss));
    }
    if (phyParam.m_SignalPowerSpecified)
    {
        columns.push_back("AverageSignalPower");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_SignalPower));
    }

    InsertValues(db, "PHY_Summary", columns, newValues);
}

void STATSDB_HandleAppEventsTableUpdate(Node* node,
                                        void* data,
                                        const StatsDBAppEventParam & appParam)
{
    // In this table we insert the application layer 
    // content on to the database.

    char addrBuf[MAX_STRING_LENGTH];
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    StatsDBAppEventContent *appEvent = db->statsAppEvents;

    if (appEvent->multipleValues)
    {
        if (db->engineType == UTIL::Database::dbMySQL)
        {
            std::string bufferForAppEventsTbColsList;
            char buf[MAX_STRING_LENGTH] = "";

            sprintf(buf,
                    "('%f', '%d', '%d'",
                    ((double) getSimTime(node) / SECOND),
                    node->nodeId,
                    appParam.m_SessionInitiator);
            bufferForAppEventsTbColsList += buf;

            if (appParam.m_ReceiverId == 0 || appParam.m_ReceiverId == -1)
            {
                bufferForAppEventsTbColsList += ", NULL";
            }
            else
            {
                sprintf(buf, ", '%d'", appParam.m_ReceiverId);
                bufferForAppEventsTbColsList += buf;
            }

            if (appParam.m_TargetAddrSpecified == FALSE)
            {
                bufferForAppEventsTbColsList += ", NULL";
            }
            else
            {
                char addrBuf[MAX_STRING_LENGTH];
                IO_ConvertIpAddressToString(
                    const_cast<Address *>(&appParam.m_TargetAddr),
                    addrBuf);
                sprintf(buf, ", '%s'", addrBuf);
                bufferForAppEventsTbColsList += buf;
            }

            sprintf(buf,
                    ", '%s', '%d', '%s'",
                    appParam.m_MessageId,
                    appParam.m_MsgSize,
                    appParam.m_EventType);
            bufferForAppEventsTbColsList += buf;

            if (appParam.m_SessionIdSpecified && appEvent->isSession)
            {
                sprintf(buf, ", '%d'", appParam.m_SessionId);
                bufferForAppEventsTbColsList += buf;
            }
            else
            {
                bufferForAppEventsTbColsList += ", NULL";
            }

            sprintf(buf,
                    ", '%s', '%s'",
                    appParam.m_ApplicationType,
                    appParam.m_ApplicationName);
            bufferForAppEventsTbColsList += buf;

            if (appEvent->isMsgSeqNum)
            {
                if (appParam.m_MsgSeqNumSpecified)
                {
                    sprintf(buf, ", '%d'", appParam.m_MsgSeqNum);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL";
                }
            }

            if (appEvent->isSocketInterfaceMsgIds)
            {
                if (appParam.m_SocketInterfaceMsgIdSpecified)
                {
                    sprintf(buf,
                            ", '%s', '%s'",
                            STATSDB_UInt64ToString(appParam.m_SocketInterfaceMsgId1).c_str(),
                            STATSDB_UInt64ToString(appParam.m_SocketInterfaceMsgId2).c_str());
                        bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL, NULL";
                }
            }

            if (appEvent->isPriority)
            {
                if (appParam.m_PrioritySpecified && appEvent->isPriority)
                {
                    sprintf(buf, ", '%d'", appParam.m_Priority);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL";
                }
            }

            if (appEvent->isMsgFailureType)
            {
                if (appParam.m_MsgFailureTypeSpecified &&
                    appEvent->isMsgFailureType)
                {
                    sprintf(buf, ", '%s'", appParam.m_MsgFailureType);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL";
                }
            }

            if (appEvent->isDelay)
            {
                if (appParam.m_DelaySpecified && appEvent->isDelay)
                {
                    sprintf(buf, ", '%f'", (double) appParam.m_Delay / SECOND);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL";
                }
            }

            if (appEvent->isJitter)
            {
                if (appParam.m_JitterSpecified && appEvent->isJitter)
                {
                    sprintf(buf, ", '%f'", (double) appParam.m_Jitter / SECOND);
                    bufferForAppEventsTbColsList += buf;
                }
                else
                {
                    bufferForAppEventsTbColsList += ", NULL";
                }
            }

            

            bufferForAppEventsTbColsList += ")";

            Int32 requiredBytes = bufferForAppEventsTbColsList.length();

            if (db->appEventsBytesUsed == 0)
            {
                // in this case, there is no stored app events
                // strings
                requiredBytes += appEvent->appEventsTbColsName.length();
            }
            else
            {
                requiredBytes += 1; // strlen(",");
            }

            if (requiredBytes + 1 >
                appEvent->bufferSizeInBytes - db->appEventsBytesUsed)
            {
                ERROR_Assert(requiredBytes + 1 <= appEvent->bufferSizeInBytes,
                    "STATS-DB-APPLICATION-EVENTS-BUFFER-SIZE"
                    "is too small for the statement. \n");

                // if coming here, then there must be a stored string
                // to insert.
                // insert the stored string first
                db->appEventsString += ";";
                AddInsertQueryToBuffer(db, db->appEventsString);

                db->appEventsBytesUsed = 0;
                // minus one because "," was accounted in the requiredBytes
                // earlier since the stored string was sent already, the ","
                // is not needed for now
                requiredBytes += appEvent->appEventsTbColsName.length() - 1;
            }

            if (db->appEventsBytesUsed)
            {
                db->appEventsString += ",";
            }
            else
            {
                db->appEventsString = appEvent->appEventsTbColsName;
            }

            db->appEventsString += bufferForAppEventsTbColsList;
            db->appEventsBytesUsed += requiredBytes;
            // keep in the buffer
            return;
        }
    }
    // Following code is executed in the following cases:
    // 1) With NATIVE MYSQL and appEvent->multipleValues is FALSE.
    // 2) With SQLlite.
    std::vector<std::string> newValues;
    newValues.reserve(18);
    std::vector<std::string> columns;
    columns.reserve(18);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("SessionInitiator");
    newValues.push_back(STATSDB_IntToString(appParam.m_SessionInitiator));
    columns.push_back("ReceiverId");
    if (appParam.m_ReceiverId == 0 || appParam.m_ReceiverId == -1)
    {
        newValues.push_back("0");
    }
    else
    {
        newValues.push_back(STATSDB_IntToString(appParam.m_ReceiverId));
    }
    columns.push_back("ReceiverAddress");
    if (appParam.m_TargetAddrSpecified == FALSE)
    {
        newValues.push_back("NULL");
    }
    else
    {
        IO_ConvertIpAddressToString(
            const_cast<Address *>(&appParam.m_TargetAddr), addrBuf);
        newValues.push_back(std::string(addrBuf));
    }
    columns.push_back("MessageId");
    newValues.push_back(appParam.m_MessageId);
    columns.push_back("Size");
    newValues.push_back(STATSDB_IntToString(appParam.m_MsgSize));
    columns.push_back("EventType");
    newValues.push_back(appParam.m_EventType);

    if (appParam.m_MsgSeqNumSpecified && appEvent->isMsgSeqNum)
    {
        columns.push_back("MessageSeqNum");
        newValues.push_back(STATSDB_IntToString(appParam.m_MsgSeqNum));
    }
    if (appParam.m_SessionIdSpecified && appEvent->isSession)
    {
        columns.push_back("SessionId");
        newValues.push_back(STATSDB_IntToString(appParam.m_SessionId));
    }

    columns.push_back("ApplicationType");
    newValues.push_back(std::string(appParam.m_ApplicationType));

    columns.push_back("ApplicationName");
    newValues.push_back(std::string(appParam.m_ApplicationName));

    if (appParam.m_PrioritySpecified && appEvent->isPriority)
    {
        columns.push_back("Priority");
        newValues.push_back(STATSDB_IntToString(appParam.m_Priority));
    }
    if (appParam.m_MsgFailureTypeSpecified && appEvent->isMsgFailureType)
    {
        columns.push_back("MessageFailureType");
        newValues.push_back(appParam.m_MsgFailureType);
    }
    if (appParam.m_DelaySpecified && appEvent->isDelay)
    {
        columns.push_back("Delay");
        newValues.push_back(
            STATSDB_DoubleToString((double) appParam.m_Delay / SECOND));
    }
    if (appParam.m_JitterSpecified && appEvent->isJitter)
    {
        columns.push_back("Jitter");
        newValues.push_back(
            STATSDB_DoubleToString((double) appParam.m_Jitter / SECOND));
    }
    if (appParam.m_SocketInterfaceMsgIdSpecified && appEvent->isSocketInterfaceMsgIds)
    {
        columns.push_back("SocketInterfaceMessageId1");
        newValues.push_back(
            STATSDB_UInt64ToString(appParam.m_SocketInterfaceMsgId1));
        columns.push_back("SocketInterfaceMessageId2");
        newValues.push_back(
            STATSDB_UInt64ToString(appParam.m_SocketInterfaceMsgId2));
    }

    InsertValues(db, "APPLICATION_Events", columns, newValues);
}


void STATSDB_HandleNetworkEventsTableUpdate(Node* node,
        void* data,
        const StatsDBNetworkEventParam &networkParam,
        Message *msg,
        const char *failure,
        BOOL failureSpecified,
        const char *eventType)
{
    // In this table we insert the network layer content on to the database.
    char buf[DB_LONG_BUFFER_LENGTH];
    char senderAddr[MAX_STRING_LENGTH];
    char receiverAddr[MAX_STRING_LENGTH];

    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    StatsDBNetworkEventContent * ipEvent = db->statsNetEvents;

    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);


    ERROR_Assert(mapParamInfo,
        "Error in HandleStatsDBNetworkEventsInsertion.");

    // Now to add the values for these parameters.

    double timeVal = (double) getSimTime(node) / SECOND;
    IO_ConvertIpAddressToString(networkParam.m_SenderAddr, senderAddr);
    IO_ConvertIpAddressToString(networkParam.m_ReceiverAddr, receiverAddr);

    // Now to add the optional stuff
    char buf3[32] = ",null";
    if (networkParam.m_MsgSeqNumSpecified && ipEvent->isMsgSeqNum)
    {
        sprintf(buf3, ", '%d'", networkParam.m_MsgSeqNum);
    }

    char buf4[32] = ",null";
    if (networkParam.m_HdrSizeSpecified && ipEvent->isControlSize)
    {
        sprintf(buf4, ", '%d'", networkParam.m_HeaderSize);
    }
    char buf7[32] = ",null";
    if (networkParam.m_PrioritySpecified && ipEvent->isPriority)
    {
        sprintf(buf7, ", '%d'", networkParam.m_Priority);
    }
    char buf6[MAX_STRING_LENGTH] = ",null";
    if (networkParam.m_ProtocolTypeSpecified && ipEvent->isProtocolType)
    {
        std::string protocolType;
        NetworkIpConvertIpProtocolNumToString(
            networkParam.m_ProtocolType, &protocolType);
        sprintf(buf6, ", '%s'", protocolType.c_str());
    }

    char buf8[MAX_STRING_LENGTH] = ",null";
    if (failureSpecified && ipEvent->isPktFailureType)
    {
        sprintf(buf8, ", '%s'", failure);
    }
    char buf5[16] = ",null";
    if (networkParam.m_PktTypeSpecified && ipEvent->isPktType)
    {
        if (networkParam.m_PktType == StatsDBNetworkEventParam::DATA)
        {
            sprintf(buf5, ", '%s'", "Data");
        }

        else sprintf(buf5, ", '%s'", "Control");
    }
    char buf2[32] = ",null";
    if (networkParam.m_InterfaceIndexSpecified && ipEvent->isInterfaceIndex)
    {

        if (networkParam.m_InterfaceIndex >= 0)
        {
            sprintf(buf2, ", '%d'", networkParam.m_InterfaceIndex);
        }
        else if (networkParam.m_InterfaceIndex == CPU_INTERFACE)
        {
            sprintf(buf2, ", '%s'", "CPU");

        }else{
            sprintf(buf2, ", '%s'", "BACKPLANE");
        }
    }
    char buf9[64] = ",null";
    if (networkParam.m_HopCountSpecified && ipEvent->isHopCount)
    {
        sprintf(buf9, ", '%f'", networkParam.m_HopCount);
    }

    if (db->statsNetEvents->multipleValues)
    {
        if (db->engineType == UTIL::Database::dbMySQL)
        {
            // trying to come up the insert statement in the following format
            // INSERT INTO NETWORK_Events VALUES('0.017438', '25', 'N25S0', '0.0.0.25', '255.25
            // 5.255.255', '16', 'NetworkReceiveFromUpper', '0', '0', '192', 'IPPROTO_ICMP',nul
            // l, 'Control', '0', '0.000000'),('0.017438', '25', 'N25S0', '0.0.0.25', '255.255.
            // 255.255', '36', 'NetworkSendToLower', '0', '20', '192', 'IPPROTO_ICMP',null, 'Co
            // ntrol', '0', '0.000000');
            // the above is stored in db->networkEventsString, whose size
            // is user configured. If the remaining buffer can not take the
            // current events after convrted to strings, the stored will be
            // inserted first
            // currently we do not do buffer reallocation, which can cause
            // performance change

            // buf has the value list, for example
            // ('0.017438', '25', 'N25S0', '0.0.0.25', '255.255.
            // 255.255', '36', 'NetworkSendToLower', '0', '20', '192', 'IPPROTO_ICMP',null, 'Co
            // ntrol', '0', '0.000000')
            sprintf(buf,
                    ipEvent->formatString,
                    timeVal,
                    node->nodeId,
                    mapParamInfo->msgId,
                    senderAddr,
                    receiverAddr,
                    networkParam.m_MsgSize,
                    eventType,
                    buf2, buf3, buf4,
                    buf5, buf6,
                    buf7, buf8, buf9);

            // requiredBytes keeping track of the required number of Bytes for
            // converting these events to strings
            // if there is no stored network events string, then it's
            // strlen of buf appended above plus strlen of
            // lengthOfNetworkEventsTbName("INSERT INTO NETWORK_Events VALUES")
            // if there is stored network events string, then it's
            // strlen of the buf appened above plus a ","

            Int32 requiredBytes = strlen(buf);

            if (db->networkEventsBytesUsed == 0)
            {
                // in this case, there is no stored network events
                // strings
                requiredBytes += ipEvent->lengthOfNetworkEventsTbName;

            }else
            {
                requiredBytes += 1; // strlen(",");
            }

            // if the number of unused bytes are not enough for this
            // statement, insert the exisiting statements first,
            // then insert the new statement
            // the extra 1 is for ";"

            if (requiredBytes + 1 >
                ipEvent->bufferSizeInBytes - db->networkEventsBytesUsed)
            {
                ERROR_Assert(requiredBytes + 1 <= ipEvent->bufferSizeInBytes,
                    "STATS-DB-NETWORK-EVENTS-BUFFER-SIZE"
                    "is too small for the statement.");

                // if coming here, then there must be a stored string
                // to insert
                // insert the stored string first
                strcat(db->networkEventsString, ";");
                AddInsertQueryToBuffer(db, std::string(db->networkEventsString));

                db->networkEventsBytesUsed = 0;
                // minus one because "," was accounted in the requiredBytes earlier
                // since the stored string was sent already, the "," is not needed
                // for now
                requiredBytes += strlen(ipEvent->networkEventsTbColsName) - 1;
            }

            if (db->networkEventsBytesUsed)
            {
                strcat(db->networkEventsString, ",");
            }else
            {
                sprintf(db->networkEventsString,
                    "%s", ipEvent->networkEventsTbColsName);
            }

            strcat(db->networkEventsString, buf);
            db->networkEventsBytesUsed += requiredBytes;
            if (db->networkEventsBytesUsed + 1 == ipEvent->bufferSizeInBytes)
            {
                // reached the limit of buffer, insert into database
                strcat(db->networkEventsString, ";");
                AddInsertQueryToBuffer(db, std::string(db->networkEventsString));

                db->networkEventsBytesUsed = 0;
            }
            // otherwise keep in the buffer
            return;
        }
    }
     sprintf(buf,
         "%s('%f', '%d', '%s', '%s', '%s', '%d', '%s'"
            "%s%s%s%s%s%s%s%s);",
            db->statsNetEvents->networkEventsTbColsName,
            timeVal,
            node->nodeId,
            mapParamInfo->msgId,
            senderAddr,
            receiverAddr,
            networkParam.m_MsgSize,
            eventType,
            buf2, buf3, buf4,
            buf5, buf6,
            buf7, buf8, buf9);


     AddInsertQueryToBuffer(db, std::string(buf));
}

//--------------------------------------------------------------------//
// NAME     : STATSDB_HandlePhyEventsTableInsert
// PURPOSE  : Insert a new row into the PHY_Events table
// PARAMETERS :
// + node : current node
// + phyParam : the values to insert into the table
//
// RETURN   : None.
//--------------------------------------------------------------------//
void STATSDB_HandlePhyEventsTableInsert(Node* node,
                                        const StatsDBPhyEventParam &phyParam)
{
    // In this table we insert the phy layer content on to the database.
    StatsDBPhyEventContent *phyEvent;

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    if (!db->statsEventsTable->createPhyEventsTable)
    {
        return;
    }
    std::vector<std::string> columns;
    columns.reserve(12);
    std::vector<std::string> newValues;
    newValues.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(phyParam.m_MessageId);
    columns.push_back("PhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_PhyIndex));    
    columns.push_back("Size");
    newValues.push_back(STATSDB_IntToString(phyParam.m_MsgSize));
    columns.push_back("EventType");
    newValues.push_back(phyParam.m_EventType);

    phyEvent = db->statsPhyEvents;

    if (phyParam.m_ChannelIndexSpecified && phyEvent->isChannelIndex)
    {
        columns.push_back("ChannelIndex");
        newValues.push_back(STATSDB_IntToString(phyParam.m_ChannelIndex));
    }
    if (phyParam.m_ControlSizeSpecified && phyEvent->isControlSize)
    {
        columns.push_back("OverheadSize");
        newValues.push_back(STATSDB_IntToString(phyParam.m_ControlSize));
    }
    if (phyParam.m_InterferenceSpecified && phyEvent->isInterference)
    {
        columns.push_back("Interference");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_Interference));
    }
    if (phyParam.m_MessageFailureTypeSpecified && phyEvent->isMessageFailureType)
    {
        columns.push_back("FailureType");
        newValues.push_back(phyParam.m_MessageFailureType);
    }
    if (phyParam.m_PathLossSpecified && phyEvent->isPathLoss)
    {
        columns.push_back("PathLoss");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_PathLoss));
    }
    if (phyParam.m_SignalPowerSpecified && phyEvent->isSignalPower)
    {
        columns.push_back("SignalPower");
        newValues.push_back(STATSDB_DoubleToString(phyParam.m_SignalPower));
    }

    InsertValues(db, "PHY_Events", columns, newValues);
}

void STATSDB_HandleMacEventsTableInsert(Node* node,
        void* data,
        const StatsDBMacEventParam & macParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    std::vector<std::string> newValues;
    newValues.reserve(17);
    std::vector<std::string> columns;
    columns.reserve(17);

    columns.push_back("Timestamp");
    newValues.push_back(STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(macParam.m_MessageId);
    columns.push_back("InterfaceIndex");
    newValues.push_back(STATSDB_IntToString(macParam.m_InterfaceIndex));
    columns.push_back("MessageSize");
    newValues.push_back(STATSDB_IntToString(macParam.m_MsgSize));
    columns.push_back("EventType");
    newValues.push_back(macParam.m_EventType);

    if (macParam.m_MsgSeqNumSpecified)
    {
        columns.push_back("SequenceNumber");
        newValues.push_back(STATSDB_IntToString(macParam.m_MsgSeqNum));
    }
    if (macParam.m_ChannelIndexSpecified)
    {
        columns.push_back("ChannelIndex");
        newValues.push_back(STATSDB_IntToString(macParam.m_ChannelIndex));
    }
    if (macParam.m_FailureTypeSpecified)
    {
        columns.push_back("FailureType");
        newValues.push_back(macParam.m_FailureType);
    }
    if (macParam.m_HdrSizeSpecified)
    {
        columns.push_back("OverheadSize");
        newValues.push_back(STATSDB_IntToString(macParam.m_HeaderSize));
    }
    if (macParam.m_FrameTypeSpecified)
    {
        columns.push_back("FrameType");
        newValues.push_back(macParam.m_FrameType); 
    }
    if (macParam.m_DstAddrSpecified)
    {
        columns.push_back("DestAddress");
        newValues.push_back(macParam.m_DstAddrStr);
    }

    if (macParam.m_SrcAddrSpecified)
    {
        columns.push_back("SrcAddress");
        newValues.push_back(macParam.m_SrcAddrStr);
    }
    //if (macParam.m_PrioritySpecified)
    //{
    //    columns.push_back("Priority");
    //    newValues.push_back(STATSDB_IntToString(macParam.m_Priority));
    //}
    //if (macParam.m_AppFragIdSpecified)
    //{
    //    columns.push_back("ApplicationFragId");
    //    newValues.push_back(STATSDB_IntToString(macParam.m_AppFragId));
    //}

    //if (macParam.m_TransportSeqNumberSpecified)
    //{
    //    columns.push_back("TransportSeqNum");
    //    newValues.push_back(STATSDB_IntToString(macParam.m_TransportSeqNumber));
    //}
    //if (macParam.m_NetworkFragNumSpecified)
    //{
    //    columns.push_back("NetworkFragId");
    //    newValues.push_back(STATSDB_IntToString(macParam.m_NetworkFragNumber));
    //}

    InsertValues(db, "MAC_Events", columns, newValues);
}

void HandleStatsDBMessageIdMappingInsert(Node *node,
    const std::string &MsgIdIn,
    const std::string &MsgIdOut,
    const std::string &protocol)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL ||
        db->statsEventsTable == NULL ||
        !db->statsEventsTable->createPhyEventsTable)
    {
        return;
    }

    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageIdIn");
    newValues.push_back(MsgIdIn);
    columns.push_back("MessageIdOut");
    newValues.push_back(MsgIdOut);
    columns.push_back("Protocol");
    newValues.push_back(protocol);

    InsertValues(db, "Message_Id_Mapping", columns, newValues);
}

void HandleStatsDBMessageIdMappingInsert(
         Node *node,
         const Message* oldMsg,
         const Message* newMsg,
         const std::string &protocol)
{
    StatsDBMappingParam* inMapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo(oldMsg, INFO_TYPE_StatsDbMapping);
    StatsDBMappingParam* outMapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo(newMsg, INFO_TYPE_StatsDbMapping);

    HandleStatsDBMessageIdMappingInsert(
        node,
        inMapParamInfo->msgId,
        outMapParamInfo->msgId,
        protocol);
}

void HandleStatsDBTransportEventsInsert(Node* node,
        Message *msg,
        const StatsDBTransportEventParam & transParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    StatsDBTransEventContent *transEvent = db->statsTransEvents;

    std::vector<std::string> newValues;
    newValues.reserve(12);
    std::vector<std::string> columns;
    columns.reserve(12);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("MessageId");
    newValues.push_back(transParam.m_MessageId);
    columns.push_back("Size");
    newValues.push_back(STATSDB_IntToString(transParam.m_MsgSize));

    if (transEvent->isSenderPort)
    {
        columns.push_back("SenderPort");
        newValues.push_back(STATSDB_IntToString(transParam.m_SenderPort));
    }
    if (transEvent->isReceiverPort)
    {
        columns.push_back("ReceiverPort");
        newValues.push_back(STATSDB_IntToString(transParam.m_ReceiverPort));
    }
    if (transParam.m_MsgSeqNumSpecified && transEvent->isMsgSeqNum)
    {
        columns.push_back("MessageSeqNum");
        newValues.push_back(STATSDB_IntToString(transParam.m_MsgSeqNum));
    }
    if (transParam.m_ConnTypeSpecified && transEvent->isConnType)
    {
        columns.push_back("ConnectionType");
        newValues.push_back(transParam.m_ConnectionType);
    }
    if (transParam.m_HdrSizeSpecified && transEvent->isHdrSize)
    {
        columns.push_back("OverheadSize");
        newValues.push_back(STATSDB_IntToString(transParam.m_HeaderSize));
    }
    if (transParam.m_FlagsSpecified && transEvent->isFlags)
    {
        columns.push_back("SegmentType");
        newValues.push_back(transParam.m_Flags);
    }
    if (transParam.m_EventTypeSpecified && transEvent->isEventType)
    {
        columns.push_back("EventType");
        newValues.push_back(transParam.m_EventType);
    }
    if (transParam.m_FailureTypeSpecified && transEvent->isMsgFailureType)
    {
        columns.push_back("MessageFailureType");
        newValues.push_back(std::string(transParam.m_FailureType));
    }

    InsertValues(db, "TRANSPORT_Events", columns, newValues);
}

void STATSDB_HandleAppConnTableInsert(Node *node,
    const StatsDBAppConnParam * appConnParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    std::vector<std::string> newValues;
    newValues.reserve(4);
    std::vector<std::string> columns;
    columns.reserve(4);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderAddr");
    newValues.push_back(appConnParam->m_SrcAddress);
    columns.push_back("ReceiverAddr");
    newValues.push_back(appConnParam->m_DstAddress);
    columns.push_back("SessionId");
    newValues.push_back(STATSDB_IntToString(appConnParam->sessionId));

    InsertValues(db, "APPLICATION_Connectivity", columns, newValues);
}

void STATSDB_HandleTransConnTableInsert(Node *node,
    const StatsDBTransConnParam * transConnParam)
{
    // In this table we insert the network layer content on to the database.
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderAddr");
    newValues.push_back(transConnParam->m_SrcAddress);
    columns.push_back("SenderPort");
    newValues.push_back(STATSDB_IntToString(transConnParam->m_SrcPort));
    columns.push_back("ReceiverAddr");
    newValues.push_back(transConnParam->m_DstAddress);
    columns.push_back("ReceiverPort");
    newValues.push_back(STATSDB_IntToString(transConnParam->m_DstPort));

    InsertValues(db, "TRANSPORT_Connectivity", columns, newValues);
}

void STATSDB_HandleMulticastConnTableInsert(Node *node,
    StatsDBConnTable::MulticastConnectivity multicastConnParam)
{
    // In this table we insert the multicast connectivity content, that is,
    // the forwarding table content
    char destAddress[20];
    double timeVal = 0.0;

    // the main stats db handle
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    // fill up our sql query
    std::vector<std::string> newValues;
    newValues.reserve(8);
    std::vector<std::string> columns;
    columns.reserve(8);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(multicastConnParam.nodeId));
    columns.push_back("DestAddr");
    // convert our addresses into strings
    IO_ConvertIpAddressToString(multicastConnParam.destAddr,destAddress);
    newValues.push_back(std::string(destAddress));
    columns.push_back("RootNodeType");
    newValues.push_back(std::string(multicastConnParam.rootNodeType));
    columns.push_back("RootNodeId");
    newValues.push_back(STATSDB_IntToString(multicastConnParam.rootNodeId));

    columns.push_back("OutgoingInterfaceIndex");
    if (multicastConnParam.outgoingInterface >= 0)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.outgoingInterface));
    }
    else
    {
        newValues.push_back("null");
    }
    columns.push_back("UpstreamNodeId");
    if (multicastConnParam.rootNodeId != multicastConnParam.nodeId)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.upstreamNeighborId));
    }
    else
    {
        newValues.push_back("null");
    }
    columns.push_back("UpstreamInterface");
    if (multicastConnParam.upstreamInterface >= 0)
    {
        newValues.push_back(
            STATSDB_IntToString(multicastConnParam.upstreamInterface));
    }
    else
    {
       newValues.push_back("null");
    }
    InsertValues(db, "MULTICAST_Connectivity", columns, newValues);
}

void STATSDB_HandleNetworkConnTableUpdate(Node* node,
        StatsDBNetworkConnParam networkParam)
{
    // In this table we insert the network connectivity content, that is,
    // the forwarding table content
    StatsDBNetworkConnContent *ipConn = NULL;
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    ipConn = db->statsNetConn;

    std::vector<std::string> newValues;
    newValues.reserve(9);
    std::vector<std::string> columns;
    columns.reserve(9);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("NodeId");
    newValues.push_back(STATSDB_IntToString(node->nodeId));
    columns.push_back("DestinationAddr");
    newValues.push_back(networkParam.m_DstAddress);
    columns.push_back("Cost");
    newValues.push_back(STATSDB_IntToString(networkParam.m_Cost));

    if (networkParam.m_DstMaskSpecified && ipConn->isDstMaskAddr)
    {
        columns.push_back("DestMaskAddr");
        newValues.push_back(networkParam.m_DstNetMask);
    }
    if (networkParam.m_OutgoingIntIndexSpecified && ipConn->isOutgoingInterfaceIndex)
    {
        columns.push_back("OutgoingInterfaceIndex");
        newValues.push_back(
            STATSDB_IntToString(networkParam.m_OutgoingIntIndex));
    }
    if (networkParam.m_NextHopAddrSpecified && ipConn->isNextHopAddr)
    {
        columns.push_back("NextHopAddr");
        newValues.push_back(networkParam.m_NextHopAddr);
    }
    if (networkParam.m_RoutingProtocolSpecified && ipConn->isRoutingProtocol)
    {
        columns.push_back("RoutingProtocolType");
        newValues.push_back(networkParam.m_RoutingProtocolType);
    }
    if (networkParam.m_AdminDistanceSpecified && ipConn->isAdminDistance)
    {
        columns.push_back("AdminDistance");
        newValues.push_back(
            STATSDB_IntToString(networkParam.m_AdminDistance));
    }

    InsertValues(db, "NETWORK_Connectivity", columns, newValues);
}

void STATSDB_HandleMacConnTableUpdate(Node *node,
                                      const StatsDBMacConnParam &macParam)
{
    // In this table we insert the phy connectivity content,
    StatsDb* db = NULL;
    db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    StatsDBMacConnContent *macConn = db->statsMacConn;

    std::vector<std::string> newValues;
    newValues.reserve(5);
    std::vector<std::string> columns;
    columns.reserve(5);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(macParam.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(macParam.m_ReceiverId));
    columns.push_back("SenderInterfaceIndex");
    newValues.push_back(STATSDB_IntToString(macParam.m_InterfaceIndex));
    columns.push_back("ChannelIndex");
    newValues.push_back(macParam.channelIndex_str);

    InsertValues(db, "MAC_Connectivity", columns, newValues);
}

void STATSDB_HandlePhyConnTableUpdate(Node* node,
    const StatsDBPhyConnParam & phyParam)
{
    // In this table we insert the phy connectivity content,
    StatsDBPhyConnContent *phyConn;

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    if (!db || !db->statsConnTable->createPhyConnTable)
    {
        return ;
    }
    if (node->partitionData->partitionId != 0)
    {
        return ;
    }
    if (node->partitionData->maxSimClock == TIME_getSimTime(node)
        && db->statsConnTable->endSimulation == FALSE)
    {
        return ;
    }

    phyConn = db->statsPhyConn;

    std::vector<std::string> newValues;
    newValues.reserve(11);
    std::vector<std::string> columns;
    columns.reserve(11);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("SenderId");
    newValues.push_back(STATSDB_IntToString(phyParam.m_SenderId));
    columns.push_back("ReceiverId");
    newValues.push_back(STATSDB_IntToString(phyParam.m_ReceiverId));
    columns.push_back("ChannelIndex");
    newValues.push_back(phyParam.m_ChannelIndex);
    columns.push_back("SenderPhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_PhyIndex));
    columns.push_back("ReceiverPhyIndex");
    newValues.push_back(STATSDB_IntToString(phyParam.m_ReceiverPhyIndex));

    columns.push_back("SenderListening");
    columns.push_back("ReceiverListening");
    columns.push_back("AntennaType");
    columns.push_back("BestAngle");
    columns.push_back("WorstAngle");
    if (isdigit(phyParam.m_ChannelIndex.c_str()[0]))
    {
        if (phyParam.senderListening)
        {
            newValues.push_back("TRUE");
        }
        else
        {
            newValues.push_back("FALSE");
        }        
        if (phyParam.receiverListening)
        {
            newValues.push_back("TRUE");
        }
        else
        {
            newValues.push_back("FALSE");
        }        
        if (phyParam.antennaType == ANTENNA_SWITCHED_BEAM)
        {
            newValues.push_back("SwitchedBeam");
        }
        else if (phyParam.antennaType == ANTENNA_STEERABLE)
        {
            newValues.push_back("Steerable");
        }
        else if (phyParam.antennaType == ANTENNA_PATTERNED)
        {
            newValues.push_back("Patterned");
        }
        else if (phyParam.antennaType == ANTENNA_OMNIDIRECTIONAL)
        {
            newValues.push_back("Omnidirectional");
        }
        else
        {
            newValues.push_back("None");
        }
        
        newValues.push_back("TRUE");
        if (phyParam.reachableWorst)
        {
            newValues.push_back("TRUE");
        }
        else
        {
            newValues.push_back("FALSE");
        }
    } 
    else
    {
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
        newValues.push_back("");
    }
    InsertValues(db, "PHY_Connectivity", columns, newValues);
}

#include "sqlite3.h"

// API to retrieve data from the database
void StatsDBRetrieveDataDatabase(Node* node,
                                 StatsDb* db,
                                 std::string sqlQueryStr,
                                 char*** data,
                                 Int32* ncol,
                                 Int32* nrow)
{
    // In this API we would retrieve data from the database
    // for the query and update the stats in the param structure.
    // If the database does not have an entry then that is an error.
    // This condition should not happen as a node cannot receive data
    // without the data being sent from another node. For now we will
    // print an error to std out in this case and move on with the simulation
    // without asserting.
    std::string queryStr = "";
    char *errMsg = 0;
    Int32 err = 0;

    std::string out;

    db->driver->exec(sqlQueryStr, out);

    db->driver->unmarshall(out, *data, *nrow, *ncol);

    // The result data structure captures the result from the database.
    // We now use these results to update the stats.
    if (ncol == 0 && nrow == 0)
    {
        // We have no data to process.
        data = NULL;
        return;
    }

    //data = &result;
}
#if 0// for possible future use
static void
STATSDB_HandleLinkUtilizationPerFrameTableInsert(Node* node,StatDBLinkUtilizationFrameDescriptor * statDBLinkUtilFrameDesp)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::vector<std::string> insertList;
    vectorSlotDesp* slotDespTmp;
    slotDespTmp = &(statDBLinkUtilFrameDesp->slotDesp);
    vectorSlotDesp::iterator iterSlotTmp;
    iterSlotTmp = slotDespTmp->begin();

    while (iterSlotTmp != slotDespTmp->end())
    {
        std::vector<std::string> newValues;
        newValues.reserve(7);
        std::vector<std::string> columns;
        columns.reserve(7);

        columns.push_back("Timestamp");
        newValues.push_back(
            STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
        columns.push_back("FrameId");
        newValues.push_back(
            STATSDB_UInt64ToString(statDBLinkUtilFrameDesp->frameId));
        columns.push_back("ChannelIndex");
        newValues.push_back(
            STATSDB_IntToString(statDBLinkUtilFrameDesp->channelIndex));
        columns.push_back("SubnetId");
        newValues.push_back(
            STATSDB_IntToString(statDBLinkUtilFrameDesp->subnetIndex));
        columns.push_back("SlotId";
        newValues.push_back(STATSDB_IntToString(iterSlotTmp->slotId));

        vectorNodeIfId::iterator iterNode;
        iterNode = iterSlotTmp->nodeIdentifier.begin();
        std::string queryStrTmp = "";
        while (iterNode != iterSlotTmp->nodeIdentifier.end())
        {
            queryStrTmp += "(" + STATSDB_IntToString(iterNode->nodeId) +
                "," + STATSDB_IntToString(iterNode->interfaceIndex) + ")";
            iterNode++;
        }
        columns.push_back("SlotUsedByNode");
        newValues.push_back(queryStrTmp);

        columns.push_back("SlotType");
        newValues.push_back(iterSlotTmp->slotType);
        
        insertList.push_back(
            GetInsertValuesSQL("Link_Utilization_Per_Frame", columns, newValues));
        iterSlotTmp++;
   }
   ExecuteMultipleNoReturnQueries(db, insertList);
}
#endif
//--------------------------------------------------------------------------
// FUNCTION:  STATSDB_HandleLinkUtilizationPerNodeTableInsert
// PURPOSE:   to put node buffer for link utilization in to lupernodetable.
// PARAMETERS
// + node : Node* : Pointer to a node
//
//
// RETURN void.
//--------------------------------------------------------------------------

void
STATSDB_HandleLinkUtilizationPerNodeTableInsert(Node* node, const std::string* str)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::string queryStr = "";
    Int32 i;
    Node* traverseNode = node;
    MAC_PROTOCOL macProtocol;

    vectorSlotDesp*  slotDespTmp;
    vectorFrameDesp* linkUtilBufferPerNode ;
    vectorFrameDesp::iterator   iterTmp;
    vectorSlotDesp::iterator    iterSlotTmp;
    macProtocol = MAC_PROTOCOL_NONE;
    if (!str->compare("WNW"))
    {
#ifdef ADDON_BOEINGFCS
        macProtocol = MAC_PROTOCOL_CES_WNW_MDL;
#endif
    }
    else if (!str->compare("NCW"))
    {
#ifdef ADDON_BOEINGFCS
        macProtocol = MAC_PROTOCOL_CES_WINTNCW;
#endif
    }
    std::vector<std::string> insertList;
    while (traverseNode != NULL)
    {
        for (i = 0; i < traverseNode->numberInterfaces; i++)
        {
          linkUtilBufferPerNode = traverseNode->macData[i]->statDBGULBuffer;
            if (traverseNode->macData[i]->macProtocol == macProtocol)
            {
               iterTmp = linkUtilBufferPerNode->begin();
                while (iterTmp != linkUtilBufferPerNode->end())
                {
                    slotDespTmp = &iterTmp->slotDesp;

                    iterSlotTmp = slotDespTmp->begin();
                    while (iterSlotTmp != slotDespTmp->end())
                    {
                        std::vector<std::string> columns;
                        columns.reserve(10);
                        std::vector<std::string> newValues;
                        newValues.reserve(10);

                        columns.push_back("Timestamp");
                        newValues.push_back(
                            STATSDB_DoubleToString((double) getSimTime(traverseNode) / SECOND));
                        columns.push_back("NodeId");
                        newValues.push_back(STATSDB_IntToString(iterTmp->nodeId));
                        columns.push_back("InterfaceIndex");
                        newValues.push_back(
                            STATSDB_IntToString(iterTmp->interfaceIndex));
                        columns.push_back("FrameId");
                        newValues.push_back(
                            STATSDB_UInt64ToString(iterTmp->frameId));
                        columns.push_back("ChannelIndex");
                        newValues.push_back(
                            STATSDB_IntToString(iterTmp->channelIndex));
                        columns.push_back("SubnetId");
                        newValues.push_back(STATSDB_IntToString(iterTmp->subnetId));
                        columns.push_back("RegionId");
                        newValues.push_back(STATSDB_IntToString(iterTmp->regionId));
                        columns.push_back("NumSlotsPerFrame");
                        newValues.push_back(
                            STATSDB_IntToString(iterTmp->numSlotsPerFrame));
                        columns.push_back("SlotId");
                        newValues.push_back(
                            STATSDB_IntToString(iterSlotTmp->slotId));
                        columns.push_back("SlotType");
                        newValues.push_back(iterSlotTmp->slotType);

                        insertList.push_back(GetInsertValuesSQL(
                                    *str + "_Link_Utilization_Per_Node",
                                    columns,
                                    newValues));
                        iterSlotTmp++;
                    }
                    iterTmp++;
                }

                linkUtilBufferPerNode->erase(
                                linkUtilBufferPerNode->begin(),
                                linkUtilBufferPerNode->end());
            }
        }
        traverseNode = traverseNode->nextNodeData;
    }
    ExecuteMultipleNoReturnQueries(db, insertList);
}

//--------------------------------------------------------------------------
// FUNCTION:  HandleLinkUtilizationTableInsert
// PURPOSE:  to process the Stats DB event inputa.
// PARAMETERS
// + node : Node* : Pointer to a node
// + LinkUtilizationParam : vector <StatsDBLinkUtilizationParam>*
//                       : pointer to the parameters.
// RETURN void.
//--------------------------------------------------------------------------


void
STATSDB_HandleLinkUtilizationTableInsert(Node* node, vector<
                StatsDBLinkUtilizationParam>* LinkUtilizationParam,
                const std::string* str)
{
    vector<StatsDBLinkUtilizationParam>::iterator iterTmp =
                    LinkUtilizationParam->begin();
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::vector<std::string> insertList;
    while (iterTmp != LinkUtilizationParam->end())
    {
        std::vector<std::string> columns;
        columns.reserve(10);
        std::vector<std::string> newValues;
        newValues.reserve(10);

        columns.push_back("Timestamp");
        newValues.push_back(
            STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
        columns.push_back("ChannelIndex");
        newValues.push_back(STATSDB_IntToString(iterTmp->channelIndex));
        columns.push_back("SubnetId");
        newValues.push_back(STATSDB_IntToString(iterTmp->subnetId));
        columns.push_back("RegionId");
        newValues.push_back(STATSDB_IntToString(iterTmp->regionId));
        columns.push_back("NumAllocSlotsPerInterval");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->numAllocSlots));
        columns.push_back("NumIdleSlotsPerInterval");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->numIdleSlots));
        columns.push_back("NumSlotsForDataPerInterval");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->numSlotsForData));
        columns.push_back("MinNumAllocSlotsPerFrame");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->minNumSlots));
        columns.push_back("MaxNumAllocSlotsPerFrame");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->maxNumSlots));
        columns.push_back("AvgNumAllocSlotsPerFrame");
        newValues.push_back(STATSDB_UInt64ToString(iterTmp->averageNumSlots));

        insertList.push_back(
            GetInsertValuesSQL(*str + "_Link_Utilization", columns, newValues));
        iterTmp++;
    }
    ExecuteMultipleNoReturnQueries(db, insertList);
}
void
STATSDB_HandleLinkUtilTableCalculate(Node* node,
                vector<StatsDBLinkUtilizationParam>* linkUtilizationParam,
                const std::string* str)
{
    vectorLinkUtilFrameDesp statDBLinkUtilFrameDescriptor;
    vectorLinkUtilFrameDesp::iterator Iter;

    std::string strSlot1 = "DATA";
    std::string strSlot2 = "RBS";
    std::string strSlot3 = "FRS";
    char** resultp = NULL;
    Int32 nrow;
    Int32 ncol;
    UInt64 frameId;
    UInt32 channelIndex;
    UInt32 subnetIndex;
    UInt32 regionId;
    BOOL flagFound;
    Int32 slotId;
    BOOL slotFound;
    
    std::vector<std::string> qualifierColumns;
    qualifierColumns.reserve(1);
    std::vector<std::string> qualifierValues;
    qualifierValues.reserve(1);
    qualifierColumns.push_back("Timestamp");
    qualifierValues.push_back(
        STATSDB_DoubleToString((double) node->partitionData->theCurrentTime / SECOND));

    std::vector<std::string> columns;
    columns.reserve(9);
    columns.push_back("NodeId");
    columns.push_back("FrameId");
    columns.push_back("ChannelIndex");
    columns.push_back("SubnetId");
    columns.push_back("SlotId");
    columns.push_back("SlotType");
    columns.push_back("InterfaceIndex");
    columns.push_back("NumSlotsPerFrame");
    columns.push_back("RegionId");
    Int32 expectedCol = (Int32) columns.size();

    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    std::string out = Select(db,       
        *str + "_Link_Utilization_Per_Node",
        columns,
        qualifierColumns,
        qualifierValues);
    
    db->driver->unmarshall(out, resultp, nrow, ncol);

    //conver resultp to StatDBLinkUtilizationFrameDescriptor

    if (nrow < 1 || ncol < expectedCol)
    {
       // ERROR_ReportWarning("Error during Link Utilization"
       //                 " collection, existing stats not found");
    }
    else
    {
        Int32 i = 0;
        for (; i < nrow; i++)
        {
            frameId = atol(resultp[expectedCol * (i + 1) + 1]);
            channelIndex = atoi(resultp[expectedCol * (i + 1) + 2]);
            subnetIndex = atoi(resultp[expectedCol * (i + 1) + 3]);
            regionId = atoi(resultp[expectedCol * (i + 1) + 8]);
            // printf("%d ",channelIndex);
            BOOL flagFound = false;
            Iter = statDBLinkUtilFrameDescriptor.begin();

            while (Iter != statDBLinkUtilFrameDescriptor.end())
            {
                if (channelIndex == Iter->channelIndex
                                && frameId == Iter->frameId
                                && subnetIndex == Iter->subnetIndex
                                && regionId == Iter->regionId)
                {
                    slotId = atoi(resultp[expectedCol * (i + 1) + 4]);
                    slotFound = false;
                    vectorSlotDesp::iterator slotTmp;
                    slotTmp = Iter->slotDesp.begin();
                    while (slotTmp != Iter->slotDesp.end())
                    {//to check wether the same slot is already exists in iter
                        if (slotTmp->slotId == slotId)
                        {
                            StatDBLinkUtilNodeInterfaceIdentifier
                                            nodeIdenTmp;
                            // need to update node table to record interfaceIndex
                            nodeIdenTmp.interfaceIndex = atoi(
                                            resultp[expectedCol * (i + 1)
                                                            + 6]);
                            nodeIdenTmp.nodeId = atoi(resultp[expectedCol
                                            * (i + 1) + 0]);

                            slotTmp->nodeIdentifier.push_back(nodeIdenTmp);
                            slotFound = true;
                            break;
                        }
                        slotTmp++;
                    }
                    if (!slotFound)
                    {
                        StatDBLinkUtilizationSlotDescriptor slotDespTmp;

                        slotDespTmp.slotId = atoi(resultp[expectedCol
                                        * (i + 1) + 4]);
                        slotDespTmp.slotType = (resultp[expectedCol * (i
                                        + 1) + 5]);
                        //  printf("slot type is %s", slotDespTmp.slotType.c_str());
                        if (!slotDespTmp.slotType.compare(strSlot1)
                                        || !slotDespTmp.slotType.compare(
                                                        strSlot2)
                                        || !slotDespTmp.slotType.compare(
                                                        strSlot3))
                        {
                            Iter->numSlotsForData++;
                        }

                        StatDBLinkUtilNodeInterfaceIdentifier nodeIdenTmp;
                        // need to update node table to record interfaceIndex
                        nodeIdenTmp.interfaceIndex
                                        = atoi(resultp[expectedCol * (i
                                                        + 1) + 6]);
                        nodeIdenTmp.nodeId = atoi(resultp[expectedCol
                                        * (i + 1) + 0]);

                        slotDespTmp.nodeIdentifier.push_back(nodeIdenTmp);
                        Iter->slotDesp.push_back(slotDespTmp);
                    }
                    flagFound = true;
                }
                Iter++;
            }

            if (!flagFound)
            {

                StatDBLinkUtilizationFrameDescriptor tmp;
                StatDBLinkUtilizationSlotDescriptor slotTmp;
                StatDBLinkUtilNodeInterfaceIdentifier nodeInfId;
                tmp.frameId = atol(resultp[expectedCol * (i + 1) + 1]);
                tmp.channelIndex = atoi(
                                resultp[expectedCol * (i + 1) + 2]);
                tmp.subnetIndex
                                = atoi(resultp[expectedCol * (i + 1) + 3]);
                tmp.regionId = atoi(resultp[expectedCol * (i + 1) + 8]);
                tmp.numSlotsPerFrame = atoi(resultp[expectedCol * (i + 1)
                                + 7]);
                slotTmp.slotId = atoi(resultp[expectedCol * (i + 1) + 4]);

                slotTmp.slotType = resultp[expectedCol * (i + 1) + 5];
                if (!slotTmp.slotType.compare(strSlot1)
                                || !slotTmp.slotType.compare(strSlot2)
                                || !slotTmp.slotType.compare(strSlot3))
                {
                    tmp.numSlotsForData = 1;
                }
                else
                {
                    tmp.numSlotsForData = 0;
                }
                nodeInfId.nodeId = atoi(
                                resultp[expectedCol * (i + 1) + 0]);
                nodeInfId.interfaceIndex = atoi(resultp[expectedCol * (i
                                + 1) + 6]);
                slotTmp.nodeIdentifier.push_back(nodeInfId);

                (tmp.slotDesp).push_back(slotTmp);

                statDBLinkUtilFrameDescriptor.push_back(tmp);

            }

        }
    }

    vectorLinkUtilFrameDesp::iterator iter3;
    iter3 = statDBLinkUtilFrameDescriptor.begin();
    vector<StatsDBLinkUtilizationParam>::iterator iter4;
    StatsDBLinkUtilizationParam tmp;
    while (iter3 != statDBLinkUtilFrameDescriptor.end())
    {
        iter4 = linkUtilizationParam->begin();
        flagFound = false;
        while (iter4 != linkUtilizationParam->end())
        {
            if (iter4->channelIndex == iter3->channelIndex
                            && iter4->subnetId == iter3->subnetIndex
                            && iter4->regionId == iter3->regionId)
            {
                iter4->numFrameCounted++;
                iter4->numAllocSlots += iter3->slotDesp.size();
                iter4->numIdleSlots += iter3->numSlotsPerFrame
                                - iter3->slotDesp.size();
                iter4->numSlotsForData += iter3->numSlotsForData;
                if (iter3->slotDesp.size() > iter4->maxNumSlots)
                {
                    iter4->maxNumSlots = iter3->slotDesp.size();
                }
                if (iter3->slotDesp.size() < iter4->minNumSlots)
                {
                    iter4->minNumSlots = iter3->slotDesp.size();
                }

                flagFound = true;
                break;
            }
            iter4++;
        }
        if (!flagFound)
        {
            //insert new
            tmp.numAllocSlots = iter3->slotDesp.size();
            tmp.numFrameCounted = 1;
            tmp.maxNumSlots = iter3->slotDesp.size();
            tmp.minNumSlots = iter3->slotDesp.size();
            tmp.numIdleSlots = iter3->numSlotsPerFrame
                            - iter3->slotDesp.size();
            tmp.channelIndex = iter3->channelIndex;
            tmp.subnetId = iter3->subnetIndex;
            tmp.numSlotsForData = iter3->numSlotsForData;
            tmp.regionId = iter3->regionId;

            //hope doesn't run out of memory for vector.
            linkUtilizationParam->push_back(tmp);
        }
        iter3++;
    }

    vector<StatsDBLinkUtilizationParam>::iterator iter5;
    iter5 = linkUtilizationParam->begin();
    while (iter5 != linkUtilizationParam->end())
    {
        iter5->averageNumSlots = iter5->numAllocSlots
                        / iter5->numFrameCounted;
        iter5++;
    }
    //delete used rows in pernode table.

    qualifierValues[0] = 
        STATSDB_DoubleToString((double) node->partitionData->theCurrentTime / SECOND);

    DeleteValues(db,
        *str + "_Link_Utilization_Per_Node",
        qualifierColumns, qualifierValues);
}
