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
#include <math.h>
#include <stdio.h>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#define strcasecmp strcmpi
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#endif

#include "api.h"
#include "partition.h"
#include "WallClock.h"
#include "external_util.h"
#include "db.h"
#include "dbapi.h"
#include "db-core.h"
#include "db-sqlite3.h"
#include "application.h"
#include "network_ip.h"
#include "network_dualip.h"

#include "phy_802_11.h"
#include "phy_abstract.h"
#include "antenna.h"

#include "mac.h"
#include "mac_dot11.h"
#include "mac_satcom.h"

#ifdef PARALLEL //Parallel
#include "parallel.h"
#endif

#ifdef USE_MPI
#include <mpi.h>
#endif

#ifdef JNE_LIB
// JWNM overhead instrumentation
#include "jne_stats.h"
#endif //JNE_LIB

#include "db-mysql-native.h"

#include "db-dynamic.h"
#include "transport_tcp.h"
#include "transport_tcpip.h"
#include "transport_udp.h"

#ifdef ADDON_BOEINGFCS
#include "routing_ces_malsr.h"
#include "mi_ces_forwarding.h"
#include "db_srw.h"
#endif

#ifdef LTE_LIB
#include "phy_lte.h"
#endif

#include "phy_connectivity.h"
// #define TRACE_SQL
// #define DEBUG_MARSHALL
#define NETWORK_SUMMARY_BROADCAST_DEBUG 0

#ifdef PARALLEL
static char dbFileName[MAX_STRING_LENGTH] = "";
#endif

const Int32 StatsDBNetworkAggregateContent::s_numTrafficTypes = 3;
const Int32 StatsDBTransportAggregateContent::s_numAddressTypes = 3;

void STATSDB_Initialize(PartitionData* partition,
                       NodeInput* nodeInput,
                       char* prefix)
{
    // In this function, we create the database. Start with creating the
    // database name.
    char buf[MAX_STRING_LENGTH];
    Int32 intBuf = 0;
    BOOL wasFound = FALSE;
    BOOL value = FALSE;
    memset(buf, 0, MAX_STRING_LENGTH);
    BOOL createDataBase = FALSE;

    // Now create the database only if the user wants.
    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-COLLECTION",
        &wasFound,
        &value);

    createDataBase = (wasFound && value);

    if (createDataBase)
    {
        StatsDb* db = new StatsDb();

        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-ENGINE",
            &wasFound,
            buf);

        if (wasFound)
        {
            IO_ConvertStringToUpperCase(buf);
            if (strcmp(buf, "SQLITE") == 0)
            {
                // Create the dataBase.
                db->engineType = UTIL::Database::dbSqlite;
            }
            else if (strcmp(buf, "MYSQL") == 0)
            {
                db->engineType = UTIL::Database::dbMySQL;
            }
            else
            {
                // We have invalid values.
                ERROR_ReportError
                    ("Invalid Value for STATS-DB-ENGINE parameter\n");
            }
        }
        else
        {
            // default do create the database using sqlite.
            db->engineType = UTIL::Database::dbSqlite;
        }

        //Do a parallel check, sqlite cannot operate in parallel mode
        if (db->engineType != UTIL::Database::dbMySQL &&
            partition->isRunningInParallel())
        {
            ERROR_ReportError
                    ("STATS-DB-ENGINE must be set to MYSQL when in running in parallel\n");
        }

        InitializePartitionStatsDb(db);
        db->createDbFile = TRUE;

        db->levelSetting = dbLevelNone;

        partition->statsDb = db;

        // Now create the database only if the user wants.
        BOOL addTimestamp;
        IO_ReadBool(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-DBNAME-TIMESTAMP",
            &wasFound,
            &addTimestamp);


        // Now to add the name, Date and Time.
        if (!wasFound || addTimestamp)
        {
#ifdef _WIN32
            SYSTEMTIME t;
            GetLocalTime(&t);

            sprintf(buf, "%s%04d%02d%02d_%02d%02d%02d",
                prefix,
                t.wYear,
                t.wMonth,
                t.wDay,
                t.wHour,
                t.wMinute,
                t.wSecond);
#else /* unix/linux */
            time_t now;
            struct tm t;

            // Break down the time into seconds, minutes, etc...
            now = time(NULL);
            localtime_r(&now, &t);

            sprintf(buf, "%s%04d%02d%02d_%02d%02d%02d",
                prefix,
                t.tm_year + 1900,
                t.tm_mon + 1,
                t.tm_mday,
                t.tm_hour,
                t.tm_min,
                t.tm_sec);
#endif
        }
        else
        {
            strcpy(buf, prefix);
        }

        // Now we have the database name. Copy to the partitionData.
#ifdef PARALLEL
#ifdef USE_MPI
        strcpy(db->statsDatabase, buf);

        if (partition->isRunningInParallel())
        {
            // mpi to spray partition 0's changes out to all other partitions
            MPI_Bcast ((void*)db->statsDatabase, MAX_STRING_LENGTH, MPI_BYTE,
                0, MPI_COMM_WORLD);
        }
#else
        // Shared memory
        if (partition->partitionId == 0)
        {
            strcpy(db->statsDatabase, buf);
            strcpy(dbFileName, buf);
        }
        else
        {
            strcpy(db->statsDatabase, dbFileName);
        }
#endif
#else
        // Sequential
        strcpy(db->statsDatabase, buf);
#endif // PARALLEL

        if (db->engineType == UTIL::Database::dbMySQL)
        {
            char database[MAX_STRING_LENGTH];
            Int32 i = 0;
            Int32 j = 0;
            while (db->statsDatabase[i] != '\0')
            {
                if (db->statsDatabase[i] != '-')
                {
                    database[j++] = db->statsDatabase[i++];
                }
                else
                {
                    i++;
                }
            }
            //copy terminating character
            database[j] = db->statsDatabase[i];

            IO_ReadString(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "STATS-DB-USERNAME",
                &wasFound,
                buf);

            std::string username = "statsdb";
            if (wasFound)
            {
                username = buf;
            }

            IO_ReadString(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "STATS-DB-PASSWORD",
                &wasFound,
                buf);

            std::string password = "statsdb";
            if (wasFound)
            {
                password = buf;
            }

            IO_ReadString(
                ANY_NODEID,
                ANY_ADDRESS,
                nodeInput,
                "STATS-DB-SERVER",
                &wasFound,
                buf);
            std::string server = "localhost";
            if (wasFound)
            {
                server = buf;
            }
            db->driver = (UTIL::Database::DatabaseDriver*)
                     new UTIL::Database::MysqlNativeDriver(server,
                                                           username,
                                                           password,
                                                           std::string(database));
        }
        else if (db->engineType == UTIL::Database::dbSqlite)
        {
            std::string database = db->statsDatabase
                + std::string(".db");

            db->driver = (UTIL::Database::DatabaseDriver*)
                new UTIL::Database::Sqlite3Driver(database);
        }
        // printf("using database: \"%s\"\n", database.c_str());

        db->driver->open(partition->partitionId == 0);

        // Check for the max and min Query buffer
        IO_ReadInt(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-MAX-QUERY-BUFFER",
            &wasFound,
            &intBuf);
        if (wasFound)
        {
            db->maxQueryBuffer = intBuf;
        }

        IO_ReadInt(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-MIN-QUERY-BUFFER",
            &wasFound,
            &intBuf);
        if (wasFound)
        {
            db->minQueryBuffer = intBuf;
        }

        /*/ Check for the existence of a separate meta data file
        IO_ReadString(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-META-DATA-FILE",
            &wasFound,
            buf);

        if (wasFound)
        {
            IO_InitializeNodeInput(&db->metaDataInput, true);
            IO_ReadNodeInput(&db->metaDataInput, buf);
        }
        else
        {
            db->metaDataInput = *nodeInput;
        }*/

        // Now to initialize the tables in the database based on the user inputs.
        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "STATS-DB-DETAIL", &wasFound, buf);
        if (wasFound)
        {
            IO_ConvertStringToUpperCase(buf);
            if (strcmp(buf, "HIGH") == 0)
            {
                db->levelSetting = dbLevelHigh;
                db->statsTable->createEventsTable = TRUE;
                db->statsTable->createConnectivityTable = TRUE;
                db->statsTable->createSummaryTable = TRUE;
                db->statsTable->createStatusTable = TRUE;
                db->statsTable->createDescriptionTable = TRUE;
                db->statsTable->createAggregateTable = TRUE;
            }
            else if (strcmp(buf, "MEDIUM") == 0)
            {
                db->levelSetting = dbLevelMedium;
                db->statsTable->createConnectivityTable = TRUE;
                db->statsTable->createSummaryTable = TRUE;
                db->statsTable->createStatusTable = TRUE;
                db->statsTable->createDescriptionTable = TRUE;
                db->statsTable->createAggregateTable = TRUE;
            }
            else if (strcmp(buf, "LOW") == 0)
            {
                db->levelSetting = dbLevelLow;
                db->statsTable->createDescriptionTable = TRUE;
                db->statsTable->createAggregateTable = TRUE;
            }
            else if (strcmp(buf, "CUSTOM") != 0)
            {
                // We have invalid values.
                ERROR_ReportWarning("Invalid Value for STATS-DB-LEVEL parameter\n");
            }
        }
        else
        {
            //default init values
            db->statsTable->createEventsTable = FALSE;
            db->statsTable->createConnectivityTable = FALSE;
            db->statsTable->createSummaryTable = FALSE;
            db->statsTable->createStatusTable = FALSE;
            db->statsTable->createDescriptionTable = FALSE;
            db->statsTable->createAggregateTable = FALSE;
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-DESCRIPTION-TABLE", &wasFound,
            &db->statsTable->createDescriptionTable);
        if ((wasFound && db->statsTable->createDescriptionTable) || !wasFound)
        {
            // Create the Decription tables.
            InitializeStatsDBDescTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-STATUS-TABLE", &wasFound, &db->statsTable->createStatusTable);
        if ((wasFound && db->statsTable->createStatusTable) || !wasFound)
        {
            //Create the status table
            InitializeStatsDBStatusTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-AGGREGATE-TABLE", &wasFound,
            &db->statsTable->createAggregateTable);
        if ((wasFound && db->statsTable->createAggregateTable) || !wasFound)
        {
            // Create the aggregate tables.
            InitializeStatsDBAggregateTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-SUMMARY-TABLE", &wasFound, &db->statsTable->createSummaryTable);
        if ((wasFound && db->statsTable->createSummaryTable) || !wasFound)
        {
            // Create the summary tables.
            InitializeStatsDBSummaryTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-EVENTS-TABLE", &wasFound, &db->statsTable->createEventsTable);
        if ((wasFound && db->statsTable->createEventsTable) || !wasFound)
        {
            // Create the events tables.
            InitializeStatsDBEventsTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-CONNECTIVITY-TABLE", &wasFound,
            &db->statsTable->createConnectivityTable);
        if ((wasFound && db->statsTable->createConnectivityTable) || !wasFound)
        {
            // Do not create the connectivity tables.
            InitializeStatsDBConnTable(partition, nodeInput);
        }

        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-URBAN-PROP-TABLE", &wasFound,
            &db->statsTable->createUrbanPropTable);
        if (partition->partitionId == 0)
        {
            if ((wasFound && db->statsTable->createUrbanPropTable) ||
                (!wasFound && db->statsTable->createUrbanPropTable))
            {
                // Do create the urban prop table.
                InitializeStatsDBUrbanPropTable(partition, nodeInput);
            }
        }

        // Protocol Specific Code
        InitializeStatsDBOspfTable(partition, nodeInput);
        InitializeStatsDBPimTable(partition, nodeInput);
        InitializeStatsDBIgmpTable(partition, nodeInput);
#ifdef ADDON_BOEINGFCS
        // Protocol Specific Code
        InitializeStatsDBRegionTable(partition, nodeInput);
        InitializeStatsDBRapTable(partition, nodeInput);
        InitializeStatsDBMalsrTable(partition, nodeInput);
        InitializeStatsDBUsapTable(partition, nodeInput);
        InitializeStatsDBLinkAdaptationTable(partition, nodeInput);
        InitializeStatsDBMiMdlTable(partition, nodeInput);
        InitializeStatsDBUsapDbaTable(partition, nodeInput);
        InitializeStatsDBMdlQueueTable(partition, nodeInput);
        InitializeStatsDBMprTable(partition, nodeInput);

        // initialize DB tables for SRW
        StatsDBSrwTablesInitialize(partition, nodeInput);
#endif
        InitializeStatsDBLinkUtilizationTable(partition, nodeInput);
    }
}

//--------------------------------------------------------------------------
// FUNCTION: STATSDB_GetTables
// PURPOSE : Return a set of all tables in the database
// PARAMETERS
// + driver : Sqlite3Driver* : Pointer to sqlite driver
// + tables : std::set<std::string>& : A set of all table names
// RETURN void.
//--------------------------------------------------------------------------
static void STATSDB_GetTables(
    UTIL::Database::Sqlite3Driver* driver,
    std::set<std::string>& tables)
{
    std::string out;
    char** table;
    Int32 numRow;
    Int32 numCol;
    Int32 i;

    // Query for all table names
    driver->exec("SELECT * FROM sqlite_master WHERE type='table';", out);
    driver->unmarshall(out, table, numRow, numCol);

    // Add table names to set
    tables.clear();
    for (i = 1; i < numRow + 1; i++)
    {
        // Ignore sqlite_sequence table
        if (strcmp(table[i * numCol + 2], "sqlite_sequence") != 0)
        {
            tables.insert(table[i * numCol + 2]);
        }
    }
}

//--------------------------------------------------------------------------
// FUNCTION: STATSDB_RegressionRow
// PURPOSE : Contains one sortable row of a table
//--------------------------------------------------------------------------
struct STATSDB_RegressionRow
{
    std::vector<std::string> m_cols;
    Int32 i;

    STATSDB_RegressionRow(char** row, Int32 numCols, Int32 ignoreColumn)
    {
        for (i = 0; i < numCols; i++)
        {
            if (i != ignoreColumn)
            {
                m_cols.push_back(row[i]);
            }
        }
    }

    bool equals (const STATSDB_RegressionRow& rhs,
        std::vector<bool> floatColumns) const
    {
        // First compare by count of columns
        if (m_cols.size() != rhs.m_cols.size())
        {
            return false;
        }

        // Next compare column by column
        for (size_t i = 0; i < m_cols.size(); i++)
        {
            // Try comparing as floating point numbers
            // If not floating point (or both are equal to 0) then fall back
            // to string compare
            if (floatColumns[i])
            {
                if (fabs(atof(m_cols[i].c_str()) - atof(rhs.m_cols[i].c_str())) > 1.1e-9)
                {
                    return false;
                }
            }
            else if (m_cols[i] != rhs.m_cols[i])
            {
                return false;
            }
        }

        // Equal to rhs
        return true;
    }
};

struct STATSDB_RegressionRowCompare
{
    std::vector<bool> floatColumns;

    bool operator() (const STATSDB_RegressionRow* lhs, const STATSDB_RegressionRow* rhs) const
    {
        double lhsFloat;
        double rhsFloat;

        // First compare by count of columns
        if (lhs->m_cols.size() < rhs->m_cols.size())
        {
            return true;
        }
        else if (lhs->m_cols.size() > rhs->m_cols.size())
        {
            return false;
        }

        // Next compare column by column
        for (size_t i = 0; i < lhs->m_cols.size(); i++)
        {
            if (floatColumns[i])
            {
                lhsFloat = atof(lhs->m_cols[i].c_str());
                rhsFloat = atof(lhs->m_cols[i].c_str());

                if (lhsFloat < rhsFloat)
                {
                    return true;
                }
                else if (lhsFloat > rhsFloat)
                {
                    return false;
                }
            }
            else if (lhs->m_cols[i] < rhs->m_cols[i])
            {
                return true;
            }
            else if (lhs->m_cols[i] > rhs->m_cols[i])
            {
                return false;
            }
        }

        // Equal to rhs
        return false;
    }
};

//--------------------------------------------------------------------------
// FUNCTION: STATSDB_CompareTables
// PURPOSE : Compare two tables.  If an error is found this function will
//     call exit(-1).
// PARAMETERS
// + expected  : DatabaseDriver* : Driver to the expected database
// + compare   : DatabaseDriver* : Driver to the results database
// + tableName : const char*     : The name of the table to compare
// RETURN void.
//----------------------------------
static void STATSDB_CompareTables(
    UTIL::Database::DatabaseDriver* expected,
    UTIL::Database::DatabaseDriver* compare,
    const char* tableName)
{
    char query[MAX_STRING_LENGTH];
    std::string out;
    Int32 i;
    char** table;
    Int32 numRow;
    Int32 numCol;
    bool err = false;
    Int32 ignoreCol = -1; // Ignore the RowId column
    STATSDB_RegressionRowCompare comp;

    // Get columns from each table, verify values are the same
    // Result is index, column name, type (int, bigint, float/real, null allowed, default value)
    std::vector<std::string> expectedColumns;
    std::vector<std::string> expectedTypes;
    std::vector<std::string> compareColumns;
    std::vector<std::string> compareTypes;
    sprintf(query, "PRAGMA TABLE_INFO(%s)", tableName);
    expected->exec(query, out);
    expected->unmarshall(out, table, numRow, numCol);
    for (i = 1; i <= numRow; i++)
    {
        if (strcmp(table[i * numCol + 1], "RowId") == 0)
        {
            ignoreCol = i - 1;
        }
        else
        {
            // Check if float - ignore rowid column though
            comp.floatColumns.push_back(strcasecmp(table[i * numCol + 2], "float") == 0
                || strcasecmp(table[i * numCol + 2], "real") == 0);
        }

        expectedColumns.push_back(table[i * numCol + 1]);
        expectedTypes.push_back(table[i * numCol + 2]);        
    }

    compare->exec(query, out);
    compare->unmarshall(out, table, numRow, numCol);
    for (i = 1; i <= numRow; i++)
    {
        compareColumns.push_back(table[i * numCol + 1]);
        compareTypes.push_back(table[i * numCol + 2]);
    }

    // Now verify that columns match
    if (expectedColumns.size() != compareColumns.size())
    {
        printf("Db has different number of columns in %s table\n", tableName);
        exit(-1);
    }
    for (size_t j = 0; j < expectedColumns.size(); j++)
    {
        if (expectedColumns[j] != compareColumns[j])
        {
            printf("Db has different column name in %s table (got %s, expected %s)\n",
                tableName,
                expectedColumns[j].c_str(),
                compareColumns[j].c_str());
            exit(-1);
        }

        if (expectedTypes[j] != compareTypes[j])
        {
            printf("Db has different column type in %s table (got %s, expected %s)\n",
                tableName,
                expectedTypes[j].c_str(),
                compareTypes[j].c_str());
            exit(-1);
        }
    }

    // Extract rows from expected db
    sprintf(query, "SELECT * FROM %s", tableName);
    expected->exec(query, out);
    expected->unmarshall(out, table, numRow, numCol);
    std::vector<STATSDB_RegressionRow*> expectedRows;
    for (i = 1; i < numRow + 1; i++)
    {
        STATSDB_RegressionRow* row = new STATSDB_RegressionRow(
            table + (i * numCol),
            numCol,
            ignoreCol);
        expectedRows.push_back(row);
    }

    // Extract rows from compare db
    compare->exec(query, out);
    compare->unmarshall(out, table, numRow, numCol);
    std::vector<STATSDB_RegressionRow*> compareRows;
    for (i = 1; i < numRow + 1; i++)
    {
        STATSDB_RegressionRow* row = new STATSDB_RegressionRow(
            table + (i * numCol),
            numCol,
            ignoreCol);
        compareRows.push_back(row);
    }

    // Sort rows
    std::sort(expectedRows.begin(), expectedRows.end(), comp);
    std::sort(compareRows.begin(), compareRows.end(), comp);

    // Now check presence of rows in each side
    // Algorithm:
    // Start itLeft and itRight at begging of LHS (expected) and RHS (compare)
    // While itLeft != end and itRight != end:
    //     If itLeft < itRight or itRight == end, print itLeft error, itLeft++
    //     Else if itRight < itLeft or itLeft == end, print itRight error, itRight++
    //     Else itLeft == itRight, increment both
    // Treat all remaining itLeft and itRight values as errors
    Int32 errCount = 0;
    std::vector<STATSDB_RegressionRow*>::iterator itLeft = expectedRows.begin();
    std::vector<STATSDB_RegressionRow*>::iterator itRight = compareRows.begin();
    while (itLeft != expectedRows.end() && itRight != expectedRows.end())
    {
        if (itLeft != expectedRows.end()
            && itRight != compareRows.end()
            && (**itLeft).equals(**itRight, comp.floatColumns))
        {
            // Equal
            ++itLeft;
            ++itRight;
        }
        else if (itLeft != expectedRows.end()
            && (itRight == compareRows.end() || comp(*itLeft, *itRight)))
        {
            // Error left
            printf("Did not find row %s", (**itLeft).m_cols[0].c_str());
            for (size_t j = 1; j < (**itLeft).m_cols.size(); j++)
            {
                printf("|%s", (**itLeft).m_cols[j].c_str());
            }
            printf(" in db table %s\n", tableName);

            ++errCount;
            ++itLeft;
        }
        else if (itRight != compareRows.end()
            && (itLeft == expectedRows.end() || comp(*itRight, *itLeft)))
        {
            // Error Right
            printf("Found extra row  %s", (**itRight).m_cols[0].c_str());
            for (size_t j = 1; j < (**itRight).m_cols.size(); j++)
            {
                printf("|%s", (**itRight).m_cols[j].c_str());
            }
            printf(" in db table %s\n", tableName);

            ++errCount;
            ++itRight;
        }
        else
        {
            // Should never arrive here
            printf("Not < or > or == %s", (**itLeft).m_cols[0].c_str());
            for (size_t j = 1; j < (**itLeft).m_cols.size(); j++)
            {
                printf("|%s", (**itLeft).m_cols[j].c_str());
            }
            printf("\n            and  %s", (**itRight).m_cols[0].c_str());
            for (size_t j = 1; j < (**itRight).m_cols.size(); j++)
            {
                printf("|%s", (**itRight).m_cols[j].c_str());
            }
            printf(" in db table %s\n", tableName);

            ++errCount;
            ++itRight;
            ++itLeft;
        }

        if (errCount >= 10)
        {
            printf("Further errors suppressed\n");
            break;
        }
    }

    // Exit with error code -1 if an error was encountered
    if (errCount > 0)
    {
        exit(-1);
    }
}

//--------------------------------------------------------------------------
// FUNCTION: STATSDB_Regression
// PURPOSE : Run regression on database output.  This function does not
//     return.  It will exit with a code of 0 if the regression matches and
//     a code of -1 if regression does not match.
// PARAMETERS
// + prefix : char* : The name of the scenario.  There should be prefix.db
//     and prefix.expected.db database files.
// RETURN void.
//----------------------------------
void STATSDB_Regression(char* prefix)
{
    // Get experiment name, open .expected.db file
    std::string dbFilename = std::string(prefix) + ".expected.db";
    UTIL::Database::Sqlite3Driver expectedDb(dbFilename);
    expectedDb.open(false);

    // Open .db file
    dbFilename = std::string(prefix) + ".db";
    UTIL::Database::Sqlite3Driver compareDb(dbFilename);
    compareDb.open(false);

    // Get tables in expected and compare
    std::set<std::string> expectedTables;
    STATSDB_GetTables(&expectedDb, expectedTables);

    std::set<std::string> compareTables;
    STATSDB_GetTables(&compareDb, compareTables);

    // Verify each database contains the same tables
    std::set<std::string>::iterator it;
    for (it = expectedTables.begin(); it != expectedTables.end(); ++it)
    {
        if (compareTables.find(*it) == compareTables.end())
        {
            printf("Db is missing table \"%s\"\n", it->c_str());
            exit(-1);
        }
    }
    for (it = compareTables.begin(); it != compareTables.end(); ++it)
    {
        if (expectedTables.find(*it) == expectedTables.end())
        {
            printf("Db has extra table \"%s\"\n", it->c_str());
            exit(-1);
        }
    }

    // Compare the content of each table
    for (it = expectedTables.begin(); it != expectedTables.end(); ++it)
    {
        STATSDB_CompareTables(&expectedDb, &compareDb, it->c_str());
    }

    // No error was found, exit with error code 0
    exit(0);
}

void InitializeStatsDBDescTable(PartitionData* partition,
                                NodeInput* nodeInput)
{
    BOOL wasFound = FALSE;
    StatsDb* db = partition->statsDb;

    // Check which tables are to be defined in the Description table
    // category. Check for the user configurations.

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-NODE-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createNodeDescTable);
    if ((wasFound && db->statsDescTable->createNodeDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the node description table.
        db->statsDescTable->createNodeDescTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBNodeDescTable(partition, nodeInput);
        }
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-QUEUE-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createQueueDescTable);
    if ((wasFound && db->statsDescTable->createQueueDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the queue description table.
        db->statsDescTable->createQueueDescTable = TRUE;
        InitializeStatsDBQueueDescTable(partition, nodeInput);
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-SCHEDULER-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createSchedulerDescTable);
    if ((wasFound && db->statsDescTable->createSchedulerDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the scheduler description table.
        db->statsDescTable->createSchedulerDescTable = TRUE;
        InitializeStatsDBSchedulerDescTable(partition, nodeInput);
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createSessionDescTable);
    if ((wasFound && db->statsDescTable->createSessionDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the session description table.
        db->statsDescTable->createSessionDescTable = TRUE;
        InitializeStatsDBSessionDescTable(partition, nodeInput);
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-CONNECTION-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createConnectionDescTable);
    if ((wasFound && db->statsDescTable->createConnectionDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the connection description table.
        db->statsDescTable->createConnectionDescTable = TRUE;
        InitializeStatsDBConnectionDescTable(partition, nodeInput);
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createInterfaceDescTable);
    if ((wasFound && db->statsDescTable->createInterfaceDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the interface description table.
        db->statsDescTable->createInterfaceDescTable = TRUE;
        InitializeStatsDBInterfaceDescTable(partition, nodeInput);
    }

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-PHY-DESCRIPTION-TABLE",
        &wasFound,
        &db->statsDescTable->createPhyDescTable);
    if ((wasFound && db->statsDescTable->createPhyDescTable) ||
        (!wasFound && db->statsTable->createDescriptionTable))
    {
        // Create the phy description table.
        db->statsDescTable->createPhyDescTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBPhyDescTable(partition, nodeInput);
        }
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBStatusTable
// PURPOSE  : Determine which status tables are to be created
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBStatusTable(PartitionData* partition,
                                NodeInput* nodeInput)
{
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound = FALSE;

    StatsDb* db = partition->statsDb;

    // Check which tables are to be defined in the Status table
    // category. Check for the user configurations.
    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NODE-STATUS-TABLE", &wasFound,
        &db->statsStatusTable->createNodeStatusTable);
    if ((wasFound && db->statsStatusTable->createNodeStatusTable) ||
        (!wasFound && db->statsTable->createStatusTable))
    {
        // Create the node status table.
        db->statsStatusTable->createNodeStatusTable = TRUE;
        InitializeStatsDBNodeStatusTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-STATUS-TABLE", &wasFound,
        &db->statsStatusTable->createInterfaceStatusTable);
    if ((wasFound && db->statsStatusTable->createInterfaceStatusTable) ||
        (!wasFound && db->statsTable->createStatusTable))
    {
        // Create the interface status table.
        db->statsStatusTable->createInterfaceStatusTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBInterfaceStatusTable(partition, nodeInput);
        }
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-STATUS-TABLE", &wasFound,
        &db->statsStatusTable->createMulticastStatusTable);
    if ((wasFound && db->statsStatusTable->createMulticastStatusTable) ||
        (!wasFound && db->statsTable->createStatusTable))
    {
        // Create the interface status table.
        db->statsStatusTable->createMulticastStatusTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBMulticastStatusTable(partition, nodeInput);
        }
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-STATUS-TABLE", &wasFound,
        &db->statsStatusTable->createQueueStatusTable);
    if ((wasFound && db->statsStatusTable->createQueueStatusTable) ||
        (!wasFound && db->statsTable->createStatusTable))
    {
        // Create the interface status table.
        db->statsStatusTable->createQueueStatusTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBQueueStatusTable(partition, nodeInput);
        }
    }

    // Set Status Interval timer.
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-STATUS-INTERVAL",
        &wasFound,
        buf);

    if (wasFound)
    {
        db->statsStatusTable->statusInterval =
            TIME_ConvertToClock(buf);
    }
    else
    {
        db->statsStatusTable->statusInterval =
            STATSDB_DEFAULT_STATUS_INTERVAL;
    }

    if (db->statsStatusTable->statusInterval <= 0)
    {
        ERROR_ReportWarning("STATS-DB-STATUS-INTERVAL should be greater "
                            "than 0. Continue with default value.");
        db->statsStatusTable->statusInterval =
                                            STATSDB_DEFAULT_STATUS_INTERVAL;
    }

    // Check for end of simulation parameter
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-STATUS-PRINT-END-SIMULATION",
        &wasFound,
        buf);
    if (wasFound && strcmp(buf, "NO") == 0)
    {
        db->statsStatusTable->endSimulation = FALSE;
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBNodeStatusTable
// PURPOSE  : Determine which columns should appear in the NODE_Status
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBNodeStatusTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Node Status Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(12);

    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("TriggeredUpdate", "VARCHAR(16)"));

    // Check for Position
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "STATS-DB-NODE-STATUS-TABLE-POSITION", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNodeStatus->isPosition = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NODE-STATUS-TABLE-POSITION parameter,"
                "using Default\n");
            db->statsNodeStatus->isPosition = TRUE;
        }
    }
    else
    {
        // Default case. We add the position.
        db->statsNodeStatus->isPosition = TRUE;
    }
    if (db->statsNodeStatus->isPosition)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            columns.push_back(string_pair("X", "real"));
            columns.push_back(string_pair("Y", "real"));
            columns.push_back(string_pair("Z", "real"));
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            columns.push_back(string_pair("Lat", "real"));
            columns.push_back(string_pair("Lon", "real"));
            columns.push_back(string_pair("Alt", "real"));
        }
        else
        {
            ERROR_Assert(0, "Unsupported STATS DB coordinate system");
        }
    }

    // Check for Velocity
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput, "STATS-DB-NODE-STATUS-TABLE-VELOCITY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNodeStatus->isVelocity = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NODE-STATUS-TABLE-VELOCITY parameter,"
                "using Default\n");
            db->statsNodeStatus->isVelocity = TRUE;
        }
    }
    else
    {
        // Default case. We add the velocity.
        db->statsNodeStatus->isVelocity = TRUE;
    }
    if (db->statsNodeStatus->isVelocity)
    {
        if (partition->terrainData->getCoordinateSystem() == CARTESIAN)
        {
            columns.push_back(string_pair("XVelocity", "real"));
            columns.push_back(string_pair("YVelocity", "real"));
            columns.push_back(string_pair("ZVelocity", "real"));
        }
        else if (partition->terrainData->getCoordinateSystem() == LATLONALT)
        {
            columns.push_back(string_pair("LatVelocity", "real"));
            columns.push_back(string_pair("LonVelocity", "real"));
            columns.push_back(string_pair("AltVelocity", "real"));
        }
        else
        {
            ERROR_Assert(0, "Unsupported STATS DB coordinate system");
        }
    }

    // Check for Active State
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NODE-STATUS-TABLE-ACTIVE-STATE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNodeStatus->isActiveState = TRUE;
            columns.push_back(string_pair("ActiveState", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NODE-STATUS-TABLE-ACTIVE-STATE parameter,"
                "using Default\n");
            db->statsNodeStatus->isActiveState = TRUE;
            columns.push_back(string_pair("ActiveState", "text"));
        }
    }
    else
    {
        // Default case. We add the active state
        db->statsNodeStatus->isActiveState = TRUE;
        columns.push_back(string_pair("ActiveState", "text"));
    }

    // Check for Damage State
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NODE-STATUS-TABLE-DAMAGE-STATE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNodeStatus->isDamageState = TRUE;
            columns.push_back(string_pair("DamageState", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NODE-STATUS-TABLE-DAMAGE-STATE parameter,"
                "using Default\n");
            db->statsNodeStatus->isDamageState = TRUE;
            columns.push_back(string_pair("DamageState", "text"));
        }
    }
    else
    {
        // Default case. We add the damage state
        db->statsNodeStatus->isDamageState = TRUE;
        columns.push_back(string_pair("DamageState", "text"));
    }

#ifdef ADDON_BOEINGFCS
    // Check for Gateway Node
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NODE-STATUS-TABLE-GATEWAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNodeStatus->isGateway = TRUE;
            columns.push_back(string_pair("IsGateway", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NODE-STATUS-TABLE-GATEWAY parameter,"
                "using Default\n");
            db->statsNodeStatus->isGateway = TRUE;
            columns.push_back(string_pair("IsGateway", "text"));
        }
    }
    else
    {
        // Default case. We add the Gateway
        db->statsNodeStatus->isGateway = TRUE;
        columns.push_back(string_pair("IsGateway", "text"));
    }
#endif
    if (partition->partitionId == 0)
    {
        
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "NODE_Status", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Node Status table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}


//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBInterfaceStatusTable
// PURPOSE  : Determine which columns should appear in the INTERFACE_Status
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBInterfaceStatusTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Interface Status Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(5);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeID", "integer"));
    columns.push_back(string_pair("InterfaceAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("InterfaceEnabled", "VARCHAR(16)"));
    columns.push_back(string_pair("TriggeredUpdate", "VARCHAR(16)"));

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    CreateTable(db, "INTERFACE_Status", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Interface Status table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBMulticastStatusTable
// PURPOSE  : Determine which columns should appear in the MULTICAST_Status
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBMulticastStatusTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Interface Status Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(6);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("GroupAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("JoiningTime", "text"));
    columns.push_back(string_pair("LeavingTime", "text"));
    columns.push_back(string_pair("GroupName", "text"));

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    CreateTable(db, "MULTICAST_Status", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Multicast Status table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

void InitializeStatsDBAggregateTable(PartitionData* partition,
                                     NodeInput* nodeInput)
{
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound = FALSE;

    StatsDb* db = partition->statsDb;

    // Check which tables are to be defined in the Aggregate table
    // category. Check for the user configurations.

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createAppAggregateTable);
    if ((wasFound && db->statsAggregateTable->createAppAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create the application aggregate table.
        db->statsAggregateTable->createAppAggregateTable = TRUE;
        InitializeStatsDBAppAggregateTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createTransAggregateTable);
    if ((wasFound && db->statsAggregateTable->createTransAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create the transport Aggregate table.
        db->statsAggregateTable->createTransAggregateTable = TRUE;
        InitializeStatsDBTransAggregateTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createNetworkAggregateTable);
    if ((wasFound && db->statsAggregateTable->createNetworkAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create the network Aggregate table.
        db->statsAggregateTable->createNetworkAggregateTable = TRUE;
        InitializeStatsDBNetworkAggregateTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createMacAggregateTable);
    if ((wasFound && db->statsAggregateTable->createMacAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create the mac Aggregate table.
        db->statsAggregateTable->createMacAggregateTable = TRUE;
        InitializeStatsDBMacAggregateTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createPhyAggregateTable);
    if ((wasFound && db->statsAggregateTable->createPhyAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create the phy Aggregate table.
        db->statsAggregateTable->createPhyAggregateTable = TRUE;
        InitializeStatsDBPhyAggregateTable(partition, nodeInput);
    }

    /*Queue DB------------------------------------------------------*/

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-AGGREGATE-TABLE", &wasFound,
        &db->statsAggregateTable->createQueueAggregateTable);
    if ((wasFound && db->statsAggregateTable->createQueueAggregateTable) ||
        (!wasFound && db->statsTable->createAggregateTable))
    {
        // Create and Initialize the Queue Aggregate tbale
        db->statsAggregateTable->createQueueAggregateTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBQueueAggregateTable(partition, nodeInput);
        }
    }

    /*Queue DB------------------------------------------------------*/

     // Set Aggregate Interval timer.
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-AGGREGATE-INTERVAL",
        &wasFound,
        buf);
    if (wasFound)
    {
        db->statsAggregateTable->aggregateInterval =
            TIME_ConvertToClock(buf);
    }
    else
    {
        db->statsAggregateTable->aggregateInterval =
            600 * SECOND;
    }

    ERROR_Assert(db->statsAggregateTable->aggregateInterval > 0,
        "STATS-DB-AGGREGATE-INTERVAL should be greater than 0.");
    // Check for end of simulation parameter
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-AGGREGATE-PRINT-END-SIMULATION",
        &wasFound,
        buf);
    if (wasFound && strcmp(buf, "NO") == 0)
    {
        db->statsAggregateTable->endSimulation = FALSE;
    }
}

// Summary Table
void InitializeStatsDBSummaryTable(PartitionData* partition,
                                   NodeInput* nodeInput)
{
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound = FALSE;

    StatsDb* db = partition->statsDb;

    // Check which tables are to be defined in the Summary table
    // category. Check for the user configurations.

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createAppSummaryTable);
    if ((wasFound && db->statsSummaryTable->createAppSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the application summary table.
        db->statsSummaryTable->createAppSummaryTable = TRUE;
        InitializeStatsDBAppSummaryTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-APPLICATION-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createMulticastAppSummaryTable);
    if ((wasFound && db->statsSummaryTable->createMulticastAppSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the multicast application summary table.
        db->statsSummaryTable->createMulticastAppSummaryTable = TRUE;
        InitializeStatsDBMulticastAppSummaryTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-NETWORK-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createMulticastNetSummaryTable);
    if ((wasFound && db->statsSummaryTable->createMulticastNetSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the multicast network summary table.
        db->statsSummaryTable->createMulticastNetSummaryTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBMulticastNetSummaryTable(partition, nodeInput);
        }
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createTransSummaryTable);
    if ((wasFound && db->statsSummaryTable->createTransSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the transport Summary table.
        db->statsSummaryTable->createTransSummaryTable = TRUE;
        InitializeStatsDBTransSummaryTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createNetworkSummaryTable);
    if ((wasFound && db->statsSummaryTable->createNetworkSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the network Summary table.
        db->statsSummaryTable->createNetworkSummaryTable = TRUE;
        InitializeStatsDBNetworkSummaryTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createMacSummaryTable);
    if ((wasFound && db->statsSummaryTable->createMacSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the mac Summary table.
        db->statsSummaryTable->createMacSummaryTable = TRUE;
        InitializeStatsDBMacSummaryTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createPhySummaryTable);
    if ((wasFound && db->statsSummaryTable->createPhySummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the phy summary table.
        db->statsSummaryTable->createPhySummaryTable = TRUE;
        InitializeStatsDBPhySummaryTable(partition, nodeInput);
    }

    /*Queue DB------------------------------------------------------*/
    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-SUMMARY-TABLE", &wasFound,
        &db->statsSummaryTable->createQueueSummaryTable);
    if ((wasFound && db->statsSummaryTable->createQueueSummaryTable) ||
        (!wasFound && db->statsTable->createSummaryTable))
    {
        // Create the queue summary table.
        db->statsSummaryTable->createQueueSummaryTable = TRUE;
        if (partition->partitionId == 0)
        {
           InitializeStatsDBQueueSummaryTable(partition, nodeInput);
        }
    }
    /*Queue DB------------------------------------------------------*/

    // Set Summary Interval timer.
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-SUMMARY-INTERVAL",
        &wasFound,
        buf);
    if (wasFound)
    {
        db->statsSummaryTable->summaryInterval = TIME_ConvertToClock(buf);
    }
    else
    {
        db->statsSummaryTable->summaryInterval =
                                            STATSDB_DEFAULT_SUMMARY_INTERVAL;
    }

    if (db->statsSummaryTable->summaryInterval <= 0)
    {
        ERROR_ReportWarning("STATS-DB-SUMMARY-INTERVAL should be greater "
                            "than 0. Continue with default value.");
        db->statsSummaryTable->summaryInterval =
                                            STATSDB_DEFAULT_SUMMARY_INTERVAL;
    }

    // Check for end of simulation parameter
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-SUMMARY-PRINT-END-SIMULATION",
        &wasFound,
        buf);
    if (wasFound && strcmp(buf, "NO") == 0)
    {
        db->statsSummaryTable->endSimulation = FALSE;
    }
}
void InitializeStatsDBEventsTable(PartitionData* partition,
                                  NodeInput* nodeInput)
{
    BOOL wasFound = FALSE;

    StatsDb* db = partition->statsDb;

    // Check which tables are to be defined in the Events table
    // category. Check for the user configurations.

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createAppEventsTable);
    if ((wasFound && db->statsEventsTable->createAppEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the application message Events table.
        db->statsEventsTable->createAppEventsTable = TRUE;
        InitializeStatsDBAppEventsTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createTransEventsTable);
    if ((wasFound && db->statsEventsTable->createTransEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the transport Events table.
        db->statsEventsTable->createTransEventsTable = TRUE;
        InitializeStatsDBTransEventsTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createNetworkEventsTable);
    if ((wasFound && db->statsEventsTable->createNetworkEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the network packet Events table.
        db->statsEventsTable->createNetworkEventsTable = TRUE;
        InitializeStatsDBNetworkEventsTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createMacEventsTable);
    if ((wasFound && db->statsEventsTable->createMacEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the mac frame Events table.
        db->statsEventsTable->createMacEventsTable = TRUE;
        InitializeStatsDBMacEventsTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createPhyEventsTable);
    if ((wasFound && db->statsEventsTable->createPhyEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the phy signal Events table.
        db->statsEventsTable->createPhyEventsTable = TRUE;
        InitializeStatsDBPhyEventsTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-EVENTS-TABLE", &wasFound,
        &db->statsEventsTable->createQueueEventsTable);
    if ((wasFound && db->statsEventsTable->createQueueEventsTable) ||
       (!wasFound && db->statsTable->createEventsTable))
    {
        // Create the queue Events table.
        db->statsEventsTable->createQueueEventsTable = TRUE;
        if (db->queueDbPtr == NULL)
        {
            db->queueDbPtr = new StatsQueueDB::StatsQueueDb;
        }
        if (partition->partitionId == 0)
        {
            db->queueDbPtr->STATSDB_InitializeQueueEventTable
                (partition, nodeInput);\
        }
    }


    // If any event table is created, we also create the message ID
    // mapping table
    db->statsEventsTable->createMessageIdMappingTable =
        db->statsEventsTable->createAppEventsTable ||
        db->statsEventsTable->createMacEventsTable ||
        db->statsEventsTable->createNetworkEventsTable ||
        db->statsEventsTable->createPhyEventsTable ||
        db->statsEventsTable->createQueueEventsTable ||
        db->statsEventsTable->createTransEventsTable;

    if (partition->partitionId == 0)
    {
        if (db->statsEventsTable->createMessageIdMappingTable)
        {
            InitializeStatsDBMessageIdMappingTable(partition, nodeInput);
        }
    }
}

void InitializeStatsDBConnTable(PartitionData* partition,
                                NodeInput* nodeInput)
{
    char buf[MAX_STRING_LENGTH];
    BOOL wasFound = FALSE;

    StatsDb* db = partition->statsDb;

     // Check for end of simulation parameter
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-CONNECTIVITY-PRINT-END-SIMULATION",
        &wasFound,
        buf);
    if (wasFound && strcmp(buf, "NO") == 0)
    {
        db->statsConnTable->endSimulation = FALSE;
    }

    // Check which tables are to be defined in the Connectivity table
    // category. Check for the user configurations.

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createAppConnTable);
    if ((wasFound && db->statsConnTable->createAppConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the application connectivity table.
        db->statsConnTable->createAppConnTable = TRUE;
        InitializeStatsDBAppConnTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createTransConnTable);
    if ((wasFound && db->statsConnTable->createTransConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the transport Connectivity table.
        db->statsConnTable->createTransConnTable = TRUE;
        InitializeStatsDBTransConnTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createNetworkConnTable);
    if ((wasFound && db->statsConnTable->createNetworkConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the network Connectivity table.
        db->statsConnTable->createNetworkConnTable = TRUE;
        InitializeStatsDBNetworkConnTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createMacConnTable);
    if ((wasFound && db->statsConnTable->createMacConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the mac Connectivity table.
        db->statsConnTable->createMacConnTable = TRUE;
        InitializeStatsDBMacConnTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createPhyConnTable);
    if ((wasFound && db->statsConnTable->createPhyConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the phy Connectivity table.
        db->statsConnTable->createPhyConnTable = TRUE;
        InitializeStatsDBPhyConnTable(partition, nodeInput);
    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-CONNECTIVITY-TABLE", &wasFound,
        &db->statsConnTable->createMulticastConnTable);
    if ((wasFound && db->statsConnTable->createMulticastConnTable) ||
        (!wasFound && db->statsTable->createConnectivityTable))
    {
        // Create the phy Connectivity table.
        db->statsConnTable->createMulticastConnTable = TRUE;
        if (partition->partitionId == 0)
        {
            InitializeStatsDBMulticastConnTable(partition, nodeInput);
        }
        // read the multicast connectivity time interval
        IO_ReadTime(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-MULTICAST-CONNECTIVITY-INTERVAL",
            &wasFound,
            &(db->statsConnTable->multicastConnSampleTimeInterval));

        if (wasFound)
        {
            ERROR_Assert(db->statsConnTable->multicastConnSampleTimeInterval > 0,
                "STATS-DB-MULTICAST-CONNECTIVITY-INTERVAL");
        }
        else
        {
            // set to default value
            db->statsConnTable->multicastConnSampleTimeInterval =
                STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
        }
    }
}


void InitializeStatsDBUrbanPropTable(PartitionData* partition,
                                     NodeInput* nodeInput)
{
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    clocktype start = 0;
    clocktype end = 0;

    // Now to create the table.
    DBColumns columns;
    columns.reserve(30);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("TxNodeId", "integer"));
    columns.push_back(string_pair("RxNodeId", "integer"));

    if (partition->terrainData->getCoordinateSystem() == LATLONALT) {
        columns.push_back(string_pair("TxLat", "real"));
        columns.push_back(string_pair("TxLon", "real"));
        columns.push_back(string_pair("TxAlt", "real"));
        columns.push_back(string_pair("RxLat", "real"));
        columns.push_back(string_pair("RxLon", "real"));
        columns.push_back(string_pair("RxAlt", "real"));
    }
    else
    {
        columns.push_back(string_pair("TxX", "real"));
        columns.push_back(string_pair("TxY", "real"));
        columns.push_back(string_pair("TxZ", "real"));
        columns.push_back(string_pair("RxX", "real"));
        columns.push_back(string_pair("RxY", "real"));
        columns.push_back(string_pair("RxZ", "real"));
    }
    columns.push_back(string_pair("Frequency", "real"));
    columns.push_back(string_pair("Distance", "real"));
    columns.push_back(string_pair("FreeSpace", "integer"));
    columns.push_back(string_pair("LoS", "integer"));
    columns.push_back(string_pair("TxInCanyon", "integer"));
    columns.push_back(string_pair("RxInCanyon", "integer"));
    columns.push_back(string_pair("TxIndoors", "integer"));
    columns.push_back(string_pair("RxIndoors", "integer"));
    columns.push_back(string_pair("ModelSelected", "text"));
    columns.push_back(string_pair("UrbanPathloss", "real"));
    columns.push_back(string_pair("FreeSpacePathloss", "real"));
    columns.push_back(string_pair("TwoRayPathloss", "real"));
    columns.push_back(string_pair("ItmPathloss", "real"));
    columns.push_back(string_pair("NumWalls", "integer"));
    columns.push_back(string_pair("NumFloors", "integer"));
    columns.push_back(string_pair("OutsideDistance", "real"));
    columns.push_back(string_pair("InsideDistance1", "real"));
    columns.push_back(string_pair("InsideDistance2", "real"));
    columns.push_back(string_pair("OutsidePathloss", "real"));
    columns.push_back(string_pair("InsidePathloss1", "real"));
    columns.push_back(string_pair("InsidePathloss2", "real"));

    CreateTable(db, "URBAN_Propagation", columns);
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBNodeDescTable
// PURPOSE  : Determine which columns should appear in the NODE_Description
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBNodeDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Node Description Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(3);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("HostName", "text"));

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    CreateTable(db, "NODE_Description", columns);

      // Take care of the Meta-data Columns.
    STATSDB_CreateNodeMetaDataColumns(partition, nodeInput);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Node Description table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}


//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBQueueDescTable
// PURPOSE  : Determine which columns should appear in the QUEUE_Description
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBQueueDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Queue Description Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(8);

    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("InterfaceIndex", "integer"));
    columns.push_back(string_pair("QueueType", "VARCHAR(64)"));
    columns.push_back(string_pair("QueueIndex", "integer"));

    // Check for Discipline

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-DESCRIPTION-TABLE-DISCIPLINE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsQueueDesc->isDiscipline = TRUE;
            columns.push_back(string_pair("QueueDiscipline", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-QUEUE-DESCRIPTION-TABLE-DISCIPLINE parameter,"
                "using Default\n");
            db->statsQueueDesc->isDiscipline = TRUE;
            columns.push_back(string_pair("QueueDiscipline", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsQueueDesc->isDiscipline = TRUE;
        columns.push_back(string_pair("QueueDiscipline", "text"));
    }
    // Check for Priority

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-DESCRIPTION-TABLE-PRIORITY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsQueueDesc->isPriority = TRUE;
            columns.push_back(
                string_pair("QueuePriority", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-QUEUE-DESCRIPTION-TABLE-PRIORITY parameter,"
                "using Default\n");
            db->statsQueueDesc->isPriority = TRUE;
            columns.push_back(
                string_pair("QueuePriority", "integer"));
        }
    }
    else
    {
        // Default case. We add the Priority.
        db->statsQueueDesc->isPriority = TRUE;
        columns.push_back(string_pair("QueuePriority", "integer"));
    }
    // Check for Size

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-DESCRIPTION-TABLE-QUEUE-SIZE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsQueueDesc->isSize = TRUE;
            columns.push_back(string_pair("QueueSize", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-QUEUE-DESCRIPTION-TABLE-QUEUE-SIZE parameter,"
                "using Default\n");
            db->statsQueueDesc->isSize = TRUE;
            columns.push_back(string_pair("QueueSize", "integer"));
        }
    }
    else
    {
        // Default case. We add the Size.
        db->statsQueueDesc->isSize = TRUE;
        columns.push_back(string_pair("QueueSize", "integer"));
    }
   
    if (partition->partitionId == 0)
    {
        clocktype start = 0;
        clocktype end = 0;
        
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
        CreateTable(db, "QUEUE_Description", columns);

          // Take care of the Meta-data Columns.
        STATSDB_CreateQueueMetaDataColumns(partition, nodeInput);
     
        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Queue Description table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }    
}

void InitializeStatsDBSessionDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Scheduler Description Table.
    // That is we create the table with the columns based on the
    // user input.
    //char schedulerTable[MAX_INSERT_SIZE];
    std::string schedulerTable;
    BOOL metaData = FALSE;
    char *errMsg = 0;
    clocktype start = 0;
    clocktype end = 0;
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(10);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SessionId", "integer"));
    columns.push_back(string_pair("SenderId", "integer"));
    columns.push_back(string_pair("ReceiverId", "integer"));

    // Check for sender addr
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-APP-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isAppType = TRUE;
            columns.push_back(string_pair("AppType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-SESSION-DESCRIPTION-APP-TYPE parameter,"
                "using Default\n");
            db->statsSessionDesc->isAppType = TRUE;
            columns.push_back(string_pair("AppType", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isAppType = TRUE;
        columns.push_back(string_pair("AppType", "text"));
    }

    // Check for receiver addr
    wasFound = FALSE;
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-SENDER-ADDRESS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isSenderAddr = TRUE;
            columns.push_back(string_pair("SenderAddr", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-SESSION-DESCRIPTION-SENDER-ADDRESS parameter,"
                "using Default\n");
            db->statsSessionDesc->isSenderAddr = TRUE;
            columns.push_back(string_pair("SenderAddr", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isSenderAddr = TRUE;
        columns.push_back(string_pair("SenderAddr", "text"));
    }

    // Check for receiver addr
    wasFound = FALSE;

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-RECEIVER-ADDRESS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isReceiverAddr = TRUE;
            columns.push_back(string_pair("ReceiverAddr", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-SESSION-DESCRIPTION-RECEIVER-ADDRESS parameter,"
                "using Default\n");
            db->statsSessionDesc->isReceiverAddr = TRUE;
            columns.push_back(string_pair("ReceiverAddr", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isReceiverAddr = TRUE;
        columns.push_back(string_pair("ReceiverAddr", "text"));
    }

    // Check for sender port
    wasFound = FALSE;

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-SENDER-PORT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isSenderPort = TRUE;
            columns.push_back(string_pair("SenderPort", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-SESSION-DESCRIPTION-SENDER-PORT parameter,"
                "using Default\n");
            db->statsSessionDesc->isSenderPort = TRUE;
            columns.push_back(string_pair("SenderPort", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isSenderPort = TRUE;
        columns.push_back(string_pair("SenderPort", "text"));
    }

    // Check for receiver port
    wasFound = FALSE;

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-RECEIVER-PORT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isRecvPort = TRUE;
            columns.push_back(string_pair("ReceiverPort", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-SESSION-DESCRIPTION-RECEIVER-PORT parameter,"
                "using Default\n");
            db->statsSessionDesc->isRecvPort = TRUE;
            columns.push_back(string_pair("ReceiverPort", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isRecvPort = TRUE;
        columns.push_back(string_pair("ReceiverPort", "text"));
    }

    // Check for transport protocol
    wasFound = FALSE;

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SESSION-DESCRIPTION-TRANSPORT-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSessionDesc->isTransportProtocol = TRUE;
            columns.push_back(
                string_pair("TransportProtocol", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-SESSION-DESCRIPTION-TRANSPORT-PROTOCOL parameter,"
                "using Default\n");
            db->statsSessionDesc->isTransportProtocol = TRUE;
            columns.push_back(
                string_pair("TransportProtocol", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsSessionDesc->isTransportProtocol = TRUE;
        columns.push_back(string_pair("TransportProtocol", "text"));
    }
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "SESSION_Description", columns);

        // Take care of the Meta-data Columns.
        STATSDB_CreateSessionMetaDataColumns(partition, nodeInput);
    
        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Session Description table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBConnectionDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{

    // In this function we initialize the Scheduler Description Table.
    // That is we create the table with the columns based on the
    // user input.
    //char schedulerTable[MAX_INSERT_SIZE];
    std::string schedulerTable;
    BOOL metaData = FALSE;
    char *errMsg = 0;
    clocktype start = 0;
    clocktype end = 0;
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(13);

    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderId", "integer"));
    columns.push_back(string_pair("ReceiverId", "integer"));
    columns.push_back(string_pair("SenderAddr", "VARCHAR(64)"));
    columns.push_back(string_pair("ReceiverAddr", "VARCHAR(64)"));
    columns.push_back(string_pair("SenderPort", "VARCHAR(64)"));
    columns.push_back(string_pair("ReceiverPort", "VARCHAR(64)"));

    // Check for transport protocol
    wasFound = FALSE;

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-CONNECTION-DESCRIPTION-CONNECTION-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsConnectionDesc->isConnectionType = TRUE;
            columns.push_back(string_pair("ConnectionType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-CONNECTION-DESCRIPTION-CONNECTION-TYPE parameter,"
                "using Default\n");
            db->statsConnectionDesc->isConnectionType = TRUE;
            columns.push_back(string_pair("ConnectionType", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsConnectionDesc->isConnectionType = TRUE;
        columns.push_back(
            string_pair("ConnectionType", "text"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-CONNECTION-DESCRIPTION-NETWORK-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsConnectionDesc->isNetworkProtocol = TRUE;
            columns.push_back(string_pair("NetworkProtocol", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-CONNECTION-DESCRIPTION-NETWORK-PROTOCOL parameter,"
                "using Default\n");
            db->statsConnectionDesc->isNetworkProtocol = TRUE;
            columns.push_back(string_pair("NetworkProtocol", "text"));
        }
    }
    else
    {
        // Default case. We add the Discipline.
        db->statsConnectionDesc->isNetworkProtocol = TRUE;
        columns.push_back(string_pair("NetworkProtocol", "text"));
    }
   
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "CONNECTION_Description", columns);

        // Take care of the Meta-data Columns.
        STATSDB_CreateConnectionMetaDataColumns(partition, nodeInput);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Connection Description table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBSchedulerDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Scheduler Description Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(5);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("InterfaceIndex", "VARCHAR(16)"));
    columns.push_back(string_pair("SchedulerType", "VARCHAR(64)"));

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-SCHEDULER-DESCRIPTION-SCHEDULER-ALGORITHM", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsSchedulerAlgo->isSchedulerAlgo = TRUE;
            columns.push_back(
                string_pair("SchedulerAlgorithm", "VARCHAR(64)"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-SCHEDULER-DESCRIPTION-SCHEDULER-ALGORITHM parameter,"
                "using Default\n");
            db->statsSchedulerAlgo->isSchedulerAlgo = TRUE;
            columns.push_back(
                string_pair("SchedulerAlgorithm", "VARCHAR(64)"));
        }
    }
    else
    {
        // Default case. We add the Name.
        db->statsSchedulerAlgo->isSchedulerAlgo = TRUE;
        columns.push_back(
            string_pair("SchedulerAlgorithm", "VARCHAR(64)"));
    }

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "SCHEDULER_Description", columns);

        // Take care of the Meta-data Columns.
        STATSDB_CreateSchedulerMetaDataColumns(partition, nodeInput);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Scheduler Description table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBInterfaceDescTable
// PURPOSE  : Determine which columns should appear in the INTERFACE_Description
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBInterfaceDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Interface Description Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    db->statsInterfaceDesc->interfaceDescTableDef.push_back(
        string_pair("Timestamp", "real"));
    db->statsInterfaceDesc->interfaceDescTableDef.push_back(
        string_pair("NodeId", "int"));
    db->statsInterfaceDesc->interfaceDescTableDef.push_back(
        string_pair("InterfaceIndex", "int"));
    // Check for Name
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-INTERFACE-NAME", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isName = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("InterfaceName", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-INTERFACE-DESCRIPTION-INTERFACE-NAME parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isName = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("InterfaceName", "text"));
        }
    }
    else
    {
        // Default case. We add the Name.
        db->statsInterfaceDesc->isName = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("InterfaceName", "text"));
    }

    // Check for Address
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-INTERFACE-ADDRESS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isAddress = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("InterfaceAddress", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-INTERFACE-DESCRIPTION-INTERFACE-ADDRESS parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isAddress = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("InterfaceAddress", "text"));
        }
    }
    else
    {
        // Default case. We add the Address.
        db->statsInterfaceDesc->isAddress = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("InterfaceAddress", "text"));
    }

    // Check for Subnet Mask
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-SUBNET-MASK", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isSubnetMask = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("SubnetMask", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-INTERFACE-DESCRIPTION-SUBNET-MASK parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isSubnetMask = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("SubnetMask", "text"));
        }
    }
    else
    {
        // Default case. We add the subnet mask.
        db->statsInterfaceDesc->isSubnetMask = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("SubnetMask", "text"));
    }

    // Check for Network Type
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-ROUTING-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isNetworkProtocol = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("RoutingProtocol", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-INTERFACE-DESCRIPTION-ROUTING-PROTOCOL parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isNetworkProtocol = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("RoutingProtocol", "text"));
        }
    }
    else
    {
        // Default case. We add the network type.
        db->statsInterfaceDesc->isNetworkProtocol = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("RoutingProtocol", "text"));

    }

    // Check for Multicast Protocol
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-MULTICAST-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isMulticastProtocol = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("MulticastProtocol", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-INTERFACE-DESCRIPTION-MULTICAST-PROTOCOL parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isMulticastProtocol = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("MulticastProtocol", "text"));
        }
    }
    else
    {
        // Default case. We add the multicast protocol.
        db->statsInterfaceDesc->isMulticastProtocol = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("MulticastProtocol", "text"));
    }

#ifdef ADDON_BOEINGFCS
    // Check for Subnet Id
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-INTERFACE-DESCRIPTION-SUBNET-ID", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsInterfaceDesc->isSubnetId = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("SubnetId", "int"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-INTERFACE-DESCRIPTION-SUBNET-ID parameter,"
                "using Default\n");
            db->statsInterfaceDesc->isSubnetId = TRUE;
            db->statsInterfaceDesc->interfaceDescTableDef.push_back(
                string_pair("SubnetId", "int"));
        }
    }
    else
    {
        // Default case. We add the Subnet Id.
        db->statsInterfaceDesc->isSubnetId = TRUE;
        db->statsInterfaceDesc->interfaceDescTableDef.push_back(
            string_pair("SubnetId", "int"));
    }
#endif

    if (partition->partitionId == 0)
    {
        clocktype start = 0;
        clocktype end = 0;
        
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
        CreateTable(db,
            "INTERFACE_Description",
            db->statsInterfaceDesc->interfaceDescTableDef);

          // Take care of the Meta-data Columns.
        STATSDB_CreateInterfaceMetaDataColumns(partition, nodeInput);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Interface Description table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBPhyDescTable
// PURPOSE  : Determine which columns should appear in the PHY_Description
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBPhyDescTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Interface Description Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(4);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("InterfaceIndex", "integer"));
    columns.push_back(string_pair("PhyIndex", "integer"));

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    CreateTable(db, "PHY_Description", columns);

      // Take care of the Meta-data Columns.
    STATSDB_CreatePhyMetaDataColumns(partition, nodeInput);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create PHY Description table partition %d, %s\n",
                partition->partitionId,
                temp);
    }

}

void InitializeStatsDBAppAggregateTable(PartitionData* partition,
                                        NodeInput* nodeInput)
{
    // In this function we initialize the Application Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    char *errMsg = 0;

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application aggregate table.
    DBColumns columns;
    columns.reserve(32);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(
        string_pair("UnicastMessagesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastMessagesReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastMessagesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveMulticastMessagesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastMessagesReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveMulticastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastFragSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastFragReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastFragSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveMulticastFragSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastFragReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastMessageCompletionRate", "float"));
    columns.push_back(
        string_pair("MulticastMessageCompletionRate", "float"));
    columns.push_back(string_pair("UnicastOfferedLoad", "float"));
    columns.push_back(string_pair("UnicastThroughput", "float"));
    columns.push_back(string_pair("MulticastOfferedLoad", "float"));
    columns.push_back(string_pair("MulticastThroughput", "float"));

    // Check for Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-UNICAST-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isUnicastDelay = TRUE;
            columns.push_back(string_pair("UnicastDelay", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-UNICAST-DELAY parameter,"
                "using Default\n");
            db->statsAppAggregate->isUnicastDelay = TRUE;
            columns.push_back(string_pair("UnicastDelay", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isUnicastDelay = TRUE;
        columns.push_back(string_pair("UnicastDelay", "float"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isMulticastDelay = TRUE;
            columns.push_back(string_pair("MulticastDelay", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-MULTICAST-DELAY parameter,"
                "using Default\n");
            db->statsAppAggregate->isMulticastDelay = TRUE;
            columns.push_back(string_pair("MulticastDelay", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isMulticastDelay = TRUE;
        columns.push_back(
            string_pair("MulticastDelay", "float"));
    }
    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-DELAY",
                  &wasFound,
                  buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isAvgDelay = TRUE;
            columns.push_back(
                string_pair("AverageDelay",  "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for"
                " STATS-DB-APPLICATION-AGGREGATE-AVERAGE-DELAY parameter,"
                "using Default\n");
            db->statsAppAggregate->isAvgDelay = TRUE;
            columns.push_back(
                string_pair("AverageDelay", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isAvgDelay = TRUE;
        columns.push_back(
            string_pair("AverageDelay", "float"));
    }

    // Check for JITTER
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-UNICAST-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isUnicastJitter = TRUE;
            columns.push_back(string_pair("UnicastJitter", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-UNICAST-JITTER parameter,"
                "using Default\n");
            db->statsAppAggregate->isUnicastJitter = TRUE;
            columns.push_back(string_pair("UnicastJitter", "float"));
        }
    }
    else
    {
        // Default case. We add the Jitter.
        db->statsAppAggregate->isUnicastJitter = TRUE;
        columns.push_back(string_pair("UnicastJitter", "float"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isMulticastJitter = TRUE;
            columns.push_back(
                string_pair("MulticastJitter", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-MULTICAST-JITTER parameter,"
                "using Default\n");
            db->statsAppAggregate->isMulticastJitter = TRUE;
            columns.push_back(
                string_pair("MulticastJitter", "float"));
        }
    }
    else
    {
        db->statsAppAggregate->isMulticastJitter = TRUE;
        columns.push_back(
            string_pair("MulticastJitter", "float"));
    }

    // Reading new parameters from Config file
    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-JITTER",
                  &wasFound,
                  buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isAvgJitter = TRUE;
            columns.push_back(
                string_pair("AverageJitter", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for"
                " STATS-DB-APPLICATION-AGGREGATE-AVERAGE-JITTER parameter,"
                "using Default TRUE\n");
            db->statsAppAggregate->isAvgJitter = TRUE;
            columns.push_back(
                string_pair("AverageJitter", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isAvgJitter = TRUE;
        columns.push_back(
            string_pair("AverageJitter", "float"));
    }

    // Check for HopCount

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-UNICAST-HOP-COUNT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isUnicastHopCount = TRUE;
            columns.push_back(
                string_pair("UnicastHopCount", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-UNICAST-HOP-COUNT parameter,"
                "using Default\n");
            db->statsAppAggregate->isUnicastHopCount = TRUE;
            columns.push_back(
                string_pair("UnicastHopCount", "float"));
        }
    }
    else
    {
        // Default case. We add the hop count.
        db->statsAppAggregate->isUnicastHopCount = TRUE;
        columns.push_back(
            string_pair("UnicastHopCount", "float"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-AGGREGATE-MULTICAST-HOP-COUNT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isMulticastHopCount = TRUE;
            columns.push_back(
                string_pair("MulticastHopCount", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-AGGREGATE-MULTICAST-HOP-COUNT parameter,"
                "using Default\n");
            db->statsAppAggregate->isMulticastHopCount = TRUE;
            columns.push_back(
                string_pair("MulticastHopCount", "float"));
        }
    }
    else
    {
        // Default case. We add the hop count.
        db->statsAppAggregate->isMulticastHopCount = TRUE;
        columns.push_back(
            string_pair("MulticastHopCount", "float"));
    }
    
        

    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-THROUGHPUT",
                  &wasFound,
                  buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isAvgThroughput = TRUE;
            columns.push_back(
                string_pair("AverageThroughput", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
              "Invalid Value for"
              " STATS-DB-APPLICATION-AGGREGATE-AVERAGE-THROUGHPUT parameter,"
              "using Default\n");
            db->statsAppAggregate->isAvgThroughput = TRUE;
            columns.push_back(
                string_pair("AverageThroughput", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isAvgThroughput = TRUE;
        columns.push_back(
            string_pair("AverageThroughput", "float"));
    }

    IO_ReadString(ANY_NODEID,
                  ANY_ADDRESS,
                  nodeInput,
                  "STATS-DB-APPLICATION-AGGREGATE-AVERAGE-OFFEREDLOAD",
                  &wasFound,
                  buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppAggregate->isAvgOfferload = TRUE;
            columns.push_back(
                string_pair("AverageOfferedload", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for"
                " STATS-DB-APPLICATION-AGGREGATE-AVERAGE-OFFEREDLOAD parameter,"
                "using Default\n");
            db->statsAppAggregate->isAvgOfferload = TRUE;
            columns.push_back(
                string_pair("AverageOfferedload", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppAggregate->isAvgOfferload = TRUE;
        columns.push_back(
            string_pair("AverageOfferedload", "float"));
    }

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "APPLICATION_Aggregate", columns);

        BOOL useApplAggregateIndex = TRUE ;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-APPLICATION-AGGREGATE-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useApplAggregateIndex = readVal;
        }

        if (useApplAggregateIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(1);
            columnsToUse.push_back("Timestamp");

            CreateIndex(db,
                "AppAggregateIndex",
                "APPLICATION_Aggregate",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create App Aggregate table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}


// Transport Aggregate Table
void InitializeStatsDBTransAggregateTable(PartitionData* partition,
                                    NodeInput* nodeInput)
{
    // In this function we initialize the Network Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the transport aggregate table.
    DBColumns columns;
    columns.reserve(25);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(
        string_pair("UnicastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastBytesRcvd", "bigint unsigned"));

    columns.push_back(string_pair("UnicastOfferedLoad", "float"));
    columns.push_back(string_pair("MulticastOfferedLoad", "float"));
    columns.push_back(string_pair("BroadcastOfferedLoad", "float"));

    columns.push_back(string_pair("UnicastThroughput", "float"));
    columns.push_back(string_pair("MulticastThroughput", "float"));
    columns.push_back(string_pair("BroadcastThroughput", "float"));

    vector<string> addressString;
    addressString.reserve(3);
    addressString.push_back("Unicast");
    addressString.push_back("Multicast");
    addressString.push_back("Broadcast");

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-AGGREGATE-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransAggregate->isDelay[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageDelay"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-AGGREGATE-DELAY parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransAggregate->isDelay[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageDelay"), "float"));
            }
        }
    }
    else
    {
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsTransAggregate->isDelay[t] = TRUE;
            columns.push_back(
                string_pair(
                    (addressString[t] + "AverageDelay"), "float"));
        }
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-AGGREGATE-JITTER", &wasFound, buf);

    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransAggregate->isJitter[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageJitter"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-AGGREGATE-JITTER parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransAggregate->isJitter[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageJitter"), "float"));
            }
        }
    }
    else
    {
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsTransAggregate->isJitter[t] = TRUE;
            columns.push_back(
                string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
        }
    }

  
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
        
        CreateTable(db, "TRANSPORT_Aggregate", columns);

        BOOL useTransAggregateIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-TRANSPORT-AGGREGATE-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useTransAggregateIndex = readVal;
        }
        if (useTransAggregateIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(1);
            columnsToUse.push_back("Timestamp");

            CreateIndex(db,
                "TransAggregateIndex",
                "TRANSPORT_Aggregate",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Transport Aggregate "
                "table partition %d, %s\n", partition->partitionId, temp);
        }
    }
}

// Network Aggregate Table
void InitializeStatsDBNetworkAggregateTable(PartitionData* partition,
                                            NodeInput* nodeInput)
{
    // In this function we initialize the Network Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;
    Int32 t = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application aggregate table.
    DBColumns columns;
    columns.reserve(40);

    columns.push_back(
        string_pair("Timestamp", "real"));
    columns.push_back(
        string_pair("UnicastOfferedLoad", "float"));
    columns.push_back(
        string_pair("MulticastOfferedLoad", "float"));
    columns.push_back(
        string_pair("BroadcastOfferedLoad", "float"));
    columns.push_back(
        string_pair("UnicastDataPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastDataPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastDataPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastDataBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastDataBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastDataBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastDataBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastDataBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastControlBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastControlBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastControlBytesForward", "bigint unsigned"));

    // Check for Delay
    //string trafficDelayBuf = "STATS-DB-NETWORK-AGGREGATE-"
    //    + ;
    vector<string> trafficString;
    trafficString.reserve(3);
    trafficString.push_back("UNICAST");
    trafficString.push_back("MULTICAST");
    trafficString.push_back("BROADCAST");
    
    // Check for ipOutNoRoutes
    for (t = 0;
         t < StatsDBNetworkAggregateContent::s_numTrafficTypes - 1;
         ++t)
    {
        string trafficNoRouteStr = "STATS-DB-NETWORK-AGGREGATE-" +
            trafficString[t] + "-IP-NO-ROUTES" ;

        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
            trafficNoRouteStr.c_str(), &wasFound, buf);

        if (wasFound)
        {            
            if (strcmp(buf, "YES") == 0)
            {
                db->statsNetAggregate->isIpOutNoRoutes[t] = TRUE;
                columns.push_back(
                    string_pair(
                    (db->statsNetAggregate->m_trafficStr[t] + "IpOutNoRoutes"), "bigint unsigned"));
            }
            else if (strcmp(buf, "NO") != 0)
            {
                // We have invalid values.
                ERROR_ReportWarning(
                    "Invalid Value for STATS-DB-NETWORK-AGGREGATE-IP-NO-ROUTES parameter,"
                    "using Default\n");
                db->statsNetAggregate->isIpOutNoRoutes[t] = TRUE;
                columns.push_back(
                    string_pair(
                    (db->statsNetAggregate->m_trafficStr[t] + "IpOutNoRoutes"), "bigint unsigned"));
            }           
        }
        else
        {
            db->statsNetAggregate->isIpOutNoRoutes[t] = TRUE;
            columns.push_back(
                string_pair(
                    (db->statsNetAggregate->m_trafficStr[t] + "IpOutNoRoutes"), "bigint unsigned"));
        }

    }

    for (t = 0; t < StatsDBNetworkAggregateContent::s_numTrafficTypes; ++t)
    {
        string trafficDelayStr = "STATS-DB-NETWORK-AGGREGATE-" +
            trafficString[t] + "-DELAY" ;
        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
            trafficDelayStr.c_str(), &wasFound, buf);

        if (wasFound)
        {
            if (strcmp(buf, "YES") == 0)
            {
                db->statsNetAggregate->isDelay[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (db->statsNetAggregate->m_trafficStr[t] + "Delay"), "float"));
            }
            else if (strcmp(buf, "NO") != 0)
            {
                // We have invalid values.
                ERROR_ReportWarning(
                    "Invalid Value for STATS-DB-NETWORK-AGGREGATE-DELAY parameter,"
                    "using Default\n");
                db->statsNetAggregate->isDelay[t] = TRUE;
                columns.push_back(
                    string_pair(
                    (db->statsNetAggregate->m_trafficStr[t] + "Delay"), "float"));
            }
        }
        else
        {
            db->statsNetAggregate->isDelay[t] = TRUE;
            columns.push_back(
                string_pair(
                    (db->statsNetAggregate->m_trafficStr[t] + "Delay"), "float"));
        }
            
    }

    // Check for JITTER
    for (t = 0; t < StatsDBNetworkAggregateContent::s_numTrafficTypes; ++t)
    {
        string trafficJitterStr = "STATS-DB-NETWORK-AGGREGATE-" +
            trafficString[t] + "-JITTER" ;
        IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
            trafficJitterStr.c_str(), &wasFound, buf);

        if (wasFound)
        {
            if (strcmp(buf, "YES") == 0)
            {
                db->statsNetAggregate->isJitter[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (db->statsNetAggregate->m_trafficStr[t] + "Jitter"), "float"));
            }
            else if (strcmp(buf, "NO") != 0)
            {
                // We have invalid values.
                ERROR_ReportWarning(
                    "Invalid Value for STATS-DB-NETWORK-AGGREGATE-JITTER parameter,"
                    "using Default\n");
                db->statsNetAggregate->isJitter[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (db->statsNetAggregate->m_trafficStr[t] + "Jitter"), "float"));
            }
        }
        else
        {
            // Default case. We add the Jitter.
            db->statsNetAggregate->isJitter[t] = TRUE;
                columns.push_back(
                    string_pair(
                        (db->statsNetAggregate->m_trafficStr[t] + "Jitter"), "float"));
        }
    }
  
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
        
        CreateTable(db, "NETWORK_Aggregate", columns);

        BOOL useNetAggregateIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-NETWORK-AGGREGATE-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useNetAggregateIndex = readVal;
        }

        if (useNetAggregateIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(1);
            columnsToUse.push_back("Timestamp");

            CreateIndex(db,
                "NetAggregateIndex",
                "NETWORK_Aggregate",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Network Aggregate table partition %d, %s\n",
                partition->partitionId,
                temp);
        }
    }

    
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBMacEventsTable
// PURPOSE  : Determine which columns should appear in the MAC_Events
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBMacEventsTable(PartitionData* partition,
                                     NodeInput* nodeInput)
{
    // In this function we initialize the Mac Event Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the mac event table.
    DBColumns columns;
    columns.reserve(13);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("MessageId", "VARCHAR(64)"));
    columns.push_back(string_pair("InterfaceIndex", "integer"));
    columns.push_back(string_pair("MessageSize", "integer"));
    columns.push_back(string_pair("EventType", "VARCHAR(64)"));
    columns.push_back(string_pair("OverheadSize", "integer"));
    columns.push_back(string_pair("DestAddress", "text"));
    columns.push_back(string_pair("SrcAddress", "text"));

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-EVENTS-MSG-SEQUENCE-NUM", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMacEvents->isMsgSeqNum = TRUE;
            columns.push_back(
                string_pair("SequenceNumber", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-EVENTS-MSG-SEQUENCE-NUM parameter,"
                "using Default\n");
            db->statsMacEvents->isMsgSeqNum = TRUE;
            columns.push_back(
                string_pair("SequenceNumber", "integer"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsMacEvents->isMsgSeqNum = TRUE;
        columns.push_back(
            string_pair("SequenceNumber", "integer"));
    }

    wasFound = FALSE;
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-EVENTS-CHANNEL-INDEX", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMacEvents->isChannelIndex = TRUE;
            columns.push_back(
                string_pair("ChannelIndex", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-EVENTS-CHANNEL-INDEX parameter,"
                "using Default\n");
            db->statsMacEvents->isChannelIndex = TRUE;
            columns.push_back(
                string_pair("ChannelIndex", "integer"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsMacEvents->isChannelIndex = TRUE;
        columns.push_back(
            string_pair("ChannelIndex", "integer"));
    }
    wasFound = FALSE;
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-EVENTS-MSG-FRAME-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMacEvents->isFrameType = TRUE;
            columns.push_back(string_pair("FrameType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-EVENTS-MSG-FRAME-TYPE parameter,"
                "using Default\n");
            db->statsMacEvents->isFrameType = TRUE;
            columns.push_back(string_pair("FrameType", "text"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsMacEvents->isFrameType = TRUE;
        columns.push_back(string_pair("FrameType", "text"));
    }
    wasFound = FALSE;
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-EVENTS-MSG-FAILURE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMacEvents->isMessageFailureType = TRUE;
            columns.push_back(string_pair("FailureType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-EVENTS-MSG-FAILURE-TYPE parameter,"
                "using Default\n");
            db->statsMacEvents->isMessageFailureType = TRUE;
            columns.push_back(string_pair("FailureType", "text"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsMacEvents->isMessageFailureType = TRUE;
        columns.push_back(string_pair("FailureType", "text"));
    }


    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
    
        CreateTable(db, "MAC_Events", columns);
    
        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Mac Events table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBMacAggregateTable
// PURPOSE  : Determine which columns should appear in the MAC_Aggregate
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBMacAggregateTable(PartitionData* partition,
                                            NodeInput* nodeInput)
{
    // In this function we initialize the Mac Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;
    Int32 i;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy aggregate table.
    DBColumns columns;
    columns.reserve(24);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("Throughput", "float"));
    
    vector<string> addressString;
    addressString.reserve(3);
    addressString.push_back("Unicast");
    addressString.push_back("Multicast");
    addressString.push_back("Broadcast");

    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        std::string addrType = STAT_AddrToString(i);

        columns.push_back(
            string_pair(addressString[i] + "DataFramesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "DataFramesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "ControlFramesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "ControlFramesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "DataBytesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "DataBytesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "ControlBytesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addressString[i] + "ControlBytesReceived", "bigint unsigned"));
    }

    // Check for Average Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-AGGREGATE-DELAY", &wasFound, buf);

    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacAggregate->addrTypes[t].isAvgQueuingDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageQueuingDelay"), "float"));
                db->statsMacAggregate->addrTypes[t].isAvgMediumAccessDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumAccessDelay"), "float"));
                db->statsMacAggregate->addrTypes[t].isAvgMediumDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumDelay"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-AGGREGATE-DELAY "
                "parameter using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacAggregate->addrTypes[t].isAvgQueuingDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageQueuingDelay"), "float"));
                db->statsMacAggregate->addrTypes[t].isAvgMediumAccessDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumAccessDelay"), "float"));
                db->statsMacAggregate->addrTypes[t].isAvgMediumDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumDelay"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the average delays.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsMacAggregate->addrTypes[t].isAvgQueuingDelay = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageQueuingDelay"), "float"));
            db->statsMacAggregate->addrTypes[t].isAvgMediumAccessDelay = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageMediumAccessDelay"), "float"));
            db->statsMacAggregate->addrTypes[t].isAvgMediumDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumDelay"), "float"));
        }
    }

    // Check for Average Jitter
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-AGGREGATE-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacAggregate->addrTypes[t].isAvgJitter = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-AGGREGATE-JITTER "
                "parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacAggregate->addrTypes[t].isAvgJitter = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the average jitter.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsMacAggregate->addrTypes[t].isAvgJitter = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageJitter"), "float"));
        }
    }



    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }
        
        CreateTable(db, "MAC_Aggregate", columns);
        
        BOOL useMacAggregateIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-MAC-AGGREGATE-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useMacAggregateIndex = readVal;
        }

        if (useMacAggregateIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(1);

            columnsToUse.push_back("Timestamp");

            CreateIndex(db,
                "MacAggregateIndex",
                "MAC_Aggregate",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Mac Aggregate table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }    
}



//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBPhyAggregateTable
// PURPOSE  : Determine which columns should appear in the PHY_Aggregate
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBPhyAggregateTable(PartitionData* partition,
                                            NodeInput* nodeInput)
{
    // In this function we initialize the Phy Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy aggregate table.
    DBColumns columns;
    columns.reserve(11);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(
        string_pair("SignalsTransmitted", "bigint unsigned"));
    columns.push_back(
        string_pair("SignalsLocked", "bigint unsigned"));
    columns.push_back(
        string_pair("SignalsReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("SignalsDropped", "bigint unsigned"));
    columns.push_back(
        string_pair("SignalsDroppedDueToInterference", "bigint unsigned"));
    columns.push_back(string_pair("Utilization", "text"));
    columns.push_back(string_pair("AverageInterference", "float"));

    // Check for Average Loss
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-AGGREGATE-PATHLOSS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyAggregate->isAvgPathLoss = TRUE;
            columns.push_back(string_pair("AveragePathLoss", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-AGGREGATE-PATHLOSS parameter,"
                "using Default\n");
            db->statsPhyAggregate->isAvgPathLoss = TRUE;
            columns.push_back(string_pair("AveragePathLoss", "float"));
        }
    }
    else
    {
        // Default case. We add the average loss.
        db->statsPhyAggregate->isAvgPathLoss = TRUE;
        columns.push_back(string_pair("AveragePathLoss", "float"));
    }

    // Check for Average Signal Power

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-AGGREGATE-SIGNAL-POWER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyAggregate->isAvgSignalPower = TRUE;
            columns.push_back(
                string_pair("AverageSignalPower", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-AGGREGATE-SIGNAL-POWER parameter,"
                "using Default\n");
            db->statsPhyAggregate->isAvgSignalPower = TRUE;
            columns.push_back(
                string_pair("AverageSignalPower", "float"));
        }
    }
    else
    {
        // Default case. We add the average signal power.
        db->statsPhyAggregate->isAvgSignalPower = TRUE;
        columns.push_back(string_pair("AverageSignalPower", "float"));
    }

    // Check for Average Delay

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-AGGREGATE-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyAggregate->isAvgDelay = TRUE;
            columns.push_back(string_pair("AverageDelay", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-AGGREGATE-DELAY parameter,"
                "using Default\n");
            db->statsPhyAggregate->isAvgDelay = TRUE;
            columns.push_back(string_pair("AverageDelay", "float"));
        }
    }
    else
    {
        // Default case. We add the average delay.
        db->statsPhyAggregate->isAvgDelay = TRUE;
        columns.push_back(string_pair("AverageDelay", "float"));
    }
   
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "PHY_Aggregate", columns);

        BOOL usePhyAggregateIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-PHY-AGGREGATE-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            usePhyAggregateIndex = readVal;
        }

        if (usePhyAggregateIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(1);
            columnsToUse.push_back("Timestamp");
            CreateIndex(db,
                "PhyAggregateIndex",
                "PHY_Aggregate",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Phy Aggregate table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }    
}


//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBQueueAggregateTable
// PURPOSE  : Determine which columns should appear in the QUEUE_Aggregate
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBQueueAggregateTable(PartitionData* partition, NodeInput* nodeInput)
{
    BOOL wasFound = FALSE;

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy aggregate table.
    DBColumns columns;
    columns.reserve(9);
    columns.push_back(string_pair("Timestamp",
        "real"));
    columns.push_back(string_pair("PacketsEnqueued",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDequeued",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDropped",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDroppedForcefully",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesEnqueued",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDequeued",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDropped",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDroppedForcefully",
        "bigint unsigned"));
   
    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }

    CreateTable(db, "QUEUE_Aggregate", columns);

    BOOL useQueueAggregateIndex = TRUE ;
    BOOL readVal;
    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-QUEUE-AGGREGATE-INDEX", &wasFound, &readVal);
    if (wasFound)
    {
        useQueueAggregateIndex = readVal;
    }
    if (useQueueAggregateIndex)
    {
        std::vector<std::string> columnsToUse;
        columnsToUse.push_back("Timestamp");

        CreateIndex(db,
            "QueueAggregateIndex",
            "QUEUE_Aggregate",
            columnsToUse);
    }

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create "
            "Queue Aggregate table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}


//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBQueueSummaryTable
// PURPOSE  : Determine which columns should appear in the QUEUE_Summary
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBQueueSummaryTable(PartitionData* partition, NodeInput* nodeInput)
{
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy aggregate table.
    DBColumns columns;
    columns.reserve(21);
    columns.push_back(string_pair("Timestamp",
        "real"));
    columns.push_back(string_pair("NodeId",
        "integer"));
    columns.push_back(string_pair("InterfaceIndex",
        "text"));
#ifdef ADDON_BOEINGFCS
    columns.push_back(string_pair("ReservationId",
        "integer"));
#endif
    columns.push_back(string_pair("QueuePosition",
        "VARCHAR(64)"));
    columns.push_back(string_pair("QueueIndex",
        "integer"));
    columns.push_back(string_pair("QueueType",
        "VARCHAR(64)"));
    columns.push_back(string_pair("QueueSize",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsEnqueued",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDequeued",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDropped",
        "bigint unsigned"));
    columns.push_back(string_pair("PacketsDroppedForcefully",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesEnqueued",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDequeued",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDropped",
        "bigint unsigned"));
    columns.push_back(string_pair("BytesDroppedForcefully",
        "bigint unsigned"));
    columns.push_back(string_pair("AverageQueueLength",
        "real"));
    columns.push_back(string_pair("PeakQueueLength",
        "bigint unsigned"));
    columns.push_back(string_pair("AverageTimeInQueue",
        "real"));
    columns.push_back(string_pair("LongestTimeInQueue ",
        "bigint unsigned"));
    columns.push_back(string_pair("AverageFreeSpace",
        "real"));
    columns.push_back(string_pair("MinimumFreeSpace",
        "bigint unsigned"));

    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }

    CreateTable(db, "QUEUE_Summary", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Queue "
            "Summary table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDbPhySummaryStats
// PURPOSE  : Initialize the summary stats for a single phy transmit/
//            receive pair
// PARAMETERS :
// + stats : pointer to the stats structure to initialize
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDbPhySummaryStats(PhyOneHopNeighborStats* stats)
{
    stats->totalDelay               = 0;
    stats->totalInterference        = 0;
    stats->totalPathLoss            = 0;
    stats->totalSignalPower         = 0;
    stats->utilization              = 0;
    stats->lastSignalStartTime      = 0;
    stats->numSignals               = 0;
    stats->numErrorSignals          = 0;
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDbPhySummaryStatsForMacProtocols
// PURPOSE  : Initialize the summary stats for a single mac transmit/
//            receive pair
// PARAMETERS :
// + stats : pointer to the stats structure to initialize
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDbPhySummaryStatsForMacProtocols(MacOneHopNeighborStats* stats)
{
    stats->totalDelay               = 0;
    stats->utilization              = 0;
    stats->numSignals               = 0;
    stats->numErrorSignals          = 0;
}

void InitializeStatsDbNetworkAggregateStats(
    StatsDBNetworkAggregate* netAggregateStats)
{
    netAggregateStats->ipUnicastOutNoRoutes = 0 ;
    netAggregateStats->ipMulticastOutNoRoutes = 0 ;
}

void InitializeStatsDbNetworkSummaryStats(OneHopNeighborStats* stats)
{
    stats->uDataPacketsSent = 0;
    stats->uDataPacketsRecd = 0;
    stats->uDataPacketsForward = 0;
    stats->uControlPacketsSent = 0;
    stats->uControlPacketsRecd = 0;
    stats->uControlPacketsForward = 0;
    stats->uDataBytesSent = 0;
    stats->uDataBytesRecd = 0;
    stats->uDataBytesForward = 0;
    stats->uControlBytesSent = 0;
    stats->uControlBytesRecd = 0;
    stats->uControlBytesForward = 0;
    stats->firstPacketSendTime = 0;
    stats->lastPacketSendTime = 0;

    stats->dataPacketDelay = 0;
    stats->dataPacketJitter = 0;
    stats->lastDataPacketDelayTime = CLOCKTYPE_MAX;
    stats->controlPacketDelay = 0;
    stats->controlPacketJitter = 0;
    stats->lastControlPacketDelayTime = CLOCKTYPE_MAX;

    // for the aggr table
    stats->totalJitter = 0 ;
    stats->lastDelayTime = CLOCKTYPE_MAX;
}
// Appplication Summary Table
void InitializeStatsDBAppSummaryTable(PartitionData* partition,
                                      NodeInput* nodeInput)
{
    // In this function we initialize the Application Summary Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application summary table.
    DBColumns columns;
    columns.reserve(23);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderId", "integer"));
    columns.push_back(string_pair("ReceiverId", "integer"));
    columns.push_back(
        string_pair("ReceiverAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("SessionId", "integer"));
    columns.push_back(string_pair("Tos", "integer"));
    columns.push_back(
        string_pair("MessagesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveMessagesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MessagesReceived", "bigint unsigned"));
    columns.push_back(
        string_pair("BytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BytesReceived", "bigint unsigned"));
    columns.push_back(string_pair("FragSent", "bigint unsigned"));
    columns.push_back(
        string_pair("EffectiveFragSent", "bigint unsigned"));
    columns.push_back(
        string_pair("FragReceived", "bigint unsigned"));
    columns.push_back(string_pair("ApplicationType", "text"));
    columns.push_back(string_pair("ApplicationName", "text"));
    columns.push_back(string_pair("MessageCompletionRate", "float"));
    columns.push_back(string_pair("OfferedLoad", "float"));
    columns.push_back(string_pair("Throughput", "float"));

    // Check for Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-SUMMARY-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppSummary->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-SUMMARY-DELAY parameter,"
                "using Default\n");
            db->statsAppSummary->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppSummary->isDelay = TRUE;
        columns.push_back(string_pair("Delay", "float"));
    }
    // Check for JITTER

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-SUMMARY-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppSummary->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-SUMMARY-JITTER parameter,"
                "using Default\n");
            db->statsAppSummary->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "float"));
        }
    }
    else
    {
        // Default case. We add the Jitter.
        db->statsAppSummary->isJitter = TRUE;
        columns.push_back(string_pair("Jitter", "float"));
    }
    // Check for HopCount
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-SUMMARY-HOP-COUNT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppSummary->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-SUMMARY-HOP-COUNT parameter,"
                "using Default\n");
            db->statsAppSummary->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "float"));
        }
    }
    else
    {
        // Default case. We add the hop count.
        db->statsAppSummary->isHopCount = TRUE;
        columns.push_back(string_pair("HopCount", "float"));
    }

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "APPLICATION_Summary", columns);

        BOOL useAppSummaryIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-APPLICATION-SUMMARY-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useAppSummaryIndex = readVal;
        }
        if (useAppSummaryIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(4);
            columnsToUse.push_back("Timestamp");
            columnsToUse.push_back("SenderId");
            columnsToUse.push_back("ReceiverId");
            columnsToUse.push_back("SessionId");
            CreateIndex(db,
                "AppSummaryIndex",
                "APPLICATION_Summary",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create App Summary table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}
// Multicast Application Summary Table
void InitializeStatsDBMulticastAppSummaryTable(PartitionData* partition,
                                               NodeInput* nodeInput)
{
    // In this function we initialize the Multicast Application Summary Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the multicast application summary table.
    DBColumns columns;
    columns.reserve(20);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderId", "integer"));
    columns.push_back(string_pair("ReceiverId", "integer"));
    columns.push_back(string_pair("ReceiverAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("SessionId", "integer"));
    columns.push_back(string_pair("Tos", "integer"));
    columns.push_back(string_pair("MessagesSent", "bigint unsigned"));
    columns.push_back(string_pair("MessagesReceived", "bigint unsigned"));
    columns.push_back(string_pair("BytesSent", "bigint unsigned"));
    columns.push_back(string_pair("BytesReceived", "bigint unsigned"));
    columns.push_back(string_pair("FragSent", "bigint unsigned"));
    columns.push_back(string_pair("FragReceived", "bigint unsigned"));
    columns.push_back(string_pair("ApplicationType", "text"));
    columns.push_back(string_pair("ApplicationName", "text"));
    columns.push_back(string_pair("MessageCompletionRate", "float"));
    columns.push_back(string_pair("OfferedLoad", "float"));
    columns.push_back(string_pair("Throughput", "float"));

    // Check for Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-APPLICATION-SUMMARY-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMulticastAppSummary->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MULTICAST-APPLICATION-SUMMARY-DELAY parameter,"
                "using Default\n");
            db->statsMulticastAppSummary->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "float"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsMulticastAppSummary->isDelay = TRUE;
        columns.push_back(string_pair("Delay", "float"));
    }
    // Check for JITTER

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-APPLICATION-SUMMARY-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMulticastAppSummary->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MULTICAST-APPLICATION-SUMMARY-JITTER parameter,"
                "using Default\n");
            db->statsMulticastAppSummary->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "float"));
        }
    }
    else
    {
        // Default case. We add the Jitter.
        db->statsMulticastAppSummary->isJitter = TRUE;
        columns.push_back(string_pair("Jitter", "float"));
    }
    
    // Check for HopCount
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MULTICAST-APPLICATION-SUMMARY-HOP-COUNT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsMulticastAppSummary->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "float"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MULTICAST-APPLICATION-SUMMARY-HOP-COUNT parameter,"
                "using Default\n");
            db->statsMulticastAppSummary->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "float"));
        }
    }
    else
    {
        // Default case. We add the hop count.
        db->statsMulticastAppSummary->isHopCount = TRUE;
        columns.push_back(string_pair("HopCount", "float"));
    }    

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "MULTICAST_APPLICATION_Summary", columns);

        BOOL useMultiAppSummaryIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-MULTICAST-APPLICATION-SUMMARY-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useMultiAppSummaryIndex = readVal;
        }

        if (useMultiAppSummaryIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(4);
            columnsToUse.push_back("Timestamp");
            columnsToUse.push_back("SenderId");
            columnsToUse.push_back("ReceiverId");
            columnsToUse.push_back("SessionId");
            CreateIndex(db,
                "MulticastAppSummaryIndex",
                "MULTICAST_APPLICATION_Summary",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Multicast App Summary table partition %d, %s\n",
                partition->partitionId,
                temp);
        }
    }

}

// Multicast Network Summary Table
void InitializeStatsDBMulticastNetSummaryTable(PartitionData* partition,
                                               NodeInput* nodeInput)
{
    // In this function we initialize the MULTICAST_NETWORK_Summary Table.
    // That is we create the table with the columns based on the
    // user input.  We also create the data sructure.

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the multicast network summary table.
    DBColumns columns;
    columns.reserve(7);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeID", "integer"));
    columns.push_back(string_pair("ProtocolType", "VARCHAR(64)"));
    columns.push_back(string_pair("DataSent", "bigint unsigned"));
    columns.push_back(string_pair("DataReceived", "bigint unsigned"));
    columns.push_back(string_pair("DataForwarded", "bigint unsigned"));
    columns.push_back(string_pair("DataDiscarded", "bigint unsigned"));

    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }

    CreateTable(db, "MULTICAST_NETWORK_Summary", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create MULTICAST_NETWORK_Summary table "
                "partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}


// Transport Summary Table
void InitializeStatsDBTransSummaryTable(PartitionData* partition,
                                          NodeInput* nodeInput)
{
    // In this function we initialize the Network Summary Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application summary table.
    DBColumns columns;
    columns.reserve(23);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderAddr", "VARCHAR(64)"));
    columns.push_back(string_pair("ReceiverAddr", "VARCHAR(64)"));
    columns.push_back(string_pair("SenderPort", "integer"));
    columns.push_back(string_pair("ReceiverPort", "integer"));

    columns.push_back(
        string_pair("UnicastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastSegmentsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastSegmentsRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("UnicastBytesRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("MulticastBytesRcvd", "bigint unsigned"));
    columns.push_back(
        string_pair("BroadcastBytesRcvd", "bigint unsigned"));

    columns.push_back(string_pair("UnicastOfferedLoad", "float"));
    columns.push_back(string_pair("MulticastOfferedLoad", "float"));
    columns.push_back(string_pair("BroadcastOfferedLoad", "float"));

    columns.push_back(string_pair("UnicastThroughput", "float"));
    columns.push_back(string_pair("MulticastThroughput", "float"));
    columns.push_back(string_pair("BroadcastThroughput", "float"));

    vector<string> addressString;
    addressString.reserve(3);
    addressString.push_back("Unicast");
    addressString.push_back("Multicast");
    addressString.push_back("Broadcast");

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-TRANSPORT-SUMMARY-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransSummary->addrTypes[t].isDelay = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageDelay"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-SUMMARY-DELAY parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransSummary->addrTypes[t].isDelay = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageDelay"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the Delay.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsTransSummary->addrTypes[t].isDelay = TRUE;
            columns.push_back(
                string_pair(
                    (addressString[t] + "AverageDelay"), "float"));
        }
    }
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-TRANSPORT-SUMMARY-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransSummary->addrTypes[t].isJitter = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageJitter"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-SUMMARY-JITTER parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsTransSummary->addrTypes[t].isJitter = TRUE;
                columns.push_back(
                    string_pair(
                        (addressString[t] + "AverageJitter"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the Delay.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsTransSummary->addrTypes[t].isJitter = TRUE;
            columns.push_back(
                string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
        }
    }
    
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "TRANSPORT_Summary", columns);
    
        BOOL useTransSummaryIndex = TRUE ;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-TRANSPORT-SUMMARY-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useTransSummaryIndex = readVal;
        }

        if (useTransSummaryIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(5);
            columnsToUse.push_back("Timestamp");
            columnsToUse.push_back("SenderAddr");
            columnsToUse.push_back("ReceiverAddr");
            columnsToUse.push_back("SenderPort");
            columnsToUse.push_back("ReceiverPort");

            CreateIndex(db,
                "TransSummaryIndex",
                "TRANSPORT_Summary",
                columnsToUse);
        }

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Transport Summary table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }    
}

// Network Summary Table
void InitializeStatsDBNetworkSummaryTable(PartitionData* partition,
                                          NodeInput* nodeInput)
{
    // In this function we initialize the Network Summary Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH] = {'\0'};

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application summary table.
    DBColumns columns;
    columns.reserve(19);
    columns.push_back(
        string_pair("Timestamp", "real"));
    columns.push_back(
        string_pair("SenderAddr", "VARCHAR(64)"));
    columns.push_back(
        string_pair("ReceiverAddr", "VARCHAR(64)"));
    columns.push_back(
        string_pair("DataPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("DataPacketsRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("DataPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("ControlPacketsSent", "bigint unsigned"));
    columns.push_back(
        string_pair("ControlPacketsRecd",  "bigint unsigned"));
    columns.push_back(
        string_pair("ControlPacketsForward", "bigint unsigned"));
    columns.push_back(
        string_pair("DataBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("DataBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("DataBytesForward", "bigint unsigned"));
    columns.push_back(
        string_pair("ControlBytesSent", "bigint unsigned"));
    columns.push_back(
        string_pair("ControlBytesRecd", "bigint unsigned"));
    columns.push_back(
        string_pair("ControlBytesForward", "bigint unsigned"));

    // Check for Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-SUMMARY-DATA-DELAY", &wasFound, buf);

    if (!wasFound || strcmp(buf, "NO"))
    {
        if (wasFound && strcmp(buf, "NO")&& strcmp(buf, "YES"))
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-SUMMARY-DATA-DELAY parameter,"
                "using Default\n");
        }
        db->statsNetSummary->isDataDelay = TRUE;
        columns.push_back(string_pair("DataDelay", "float"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-SUMMARY-CONTROL-DELAY", &wasFound, buf);

    if (!wasFound || strcmp(buf, "NO"))
    {
        if (wasFound && strcmp(buf, "NO")&& strcmp(buf, "YES"))
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-SUMMARY-CONTROL-DELAY parameter,"
                "using Default\n");
        }
        db->statsNetSummary->isControlDelay = TRUE;
        columns.push_back(string_pair("ControlDelay", "float"));
    }

    // Check for JITTER
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-SUMMARY-DATA-JITTER", &wasFound, buf);

    if (!wasFound || strcmp(buf, "NO"))
    {
        if (wasFound && strcmp(buf, "NO") &&  strcmp(buf, "YES"))
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-SUMMARY-DATA-JITTER parameter,"
                "using Default\n");
        }
        db->statsNetSummary->isDataJitter = TRUE;
        columns.push_back(string_pair("DataJitter", "float"));
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-SUMMARY-CONTROL-JITTER", &wasFound, buf);

    if (!wasFound || strcmp(buf, "NO"))
    {
        if (wasFound && strcmp(buf, "NO") &&  strcmp(buf, "YES"))
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-SUMMARY-CONTROL-JITTER parameter,"
                "using Default\n");
        }
        db->statsNetSummary->isControlJitter = TRUE;
        columns.push_back(string_pair("ControlJitter", "float"));
    }

    

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "NETWORK_Summary", columns);

        BOOL useNetSummaryIndex = TRUE;
        BOOL readVal;
        IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
            "STATS-DB-NETWORK-SUMMARY-INDEX", &wasFound, &readVal);
        if (wasFound)
        {
            useNetSummaryIndex = readVal;
        }
        if (useNetSummaryIndex)
        {
            std::vector<std::string> columnsToUse;
            columnsToUse.reserve(3);
            columnsToUse.push_back("Timestamp");
            columnsToUse.push_back("SenderAddr");
            columnsToUse.push_back("ReceiverAddr");

            CreateIndex(db,
                "NetSummaryIndex",
                "NETWORK_Summary",
                columnsToUse);
        }
        
        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Network Summary table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }  
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBMacSummaryTable
// PURPOSE  : Determine which columns should appear in the MAC_Summary
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBMacSummaryTable(PartitionData* partition,
                                NodeInput* nodeInput)
{
    // In this function we initialize the Mac Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;
    Int32 i;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy aggregate table.
    DBColumns columns;
    columns.reserve(19);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderId", "int"));
    columns.push_back(string_pair("ReceiverId", "int"));
    columns.push_back(string_pair("InterfaceIndex", "int"));

    for (i = 0; i < STAT_NUM_ADDRESS_TYPES; i++)
    {
        std::string addrType = STAT_AddrToString(i);

        columns.push_back(
            string_pair(addrType + "DataFramesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "DataFramesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "ControlFramesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "ControlFramesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "DataBytesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "DataBytesReceived", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "ControlBytesSent", "bigint unsigned"));
        columns.push_back(
            string_pair(addrType + "ControlBytesReceived", "bigint unsigned"));
    }

    vector<string> addressString;
    addressString.reserve(3);
    addressString.push_back("Unicast");
    addressString.push_back("Multicast");
    addressString.push_back("Broadcast");

    // Check for Average Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-SUMMARY-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacSummary->addrTypes[t].isAvgQueuingDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageQueuingDelay"), "float"));
                db->statsMacSummary->addrTypes[t].isAvgMediumAccessDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumAccessDelay"), "float"));
                db->statsMacSummary->addrTypes[t].isAvgMediumDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumDelay"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-SUMMARY-DELAY \
                parameter using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacSummary->addrTypes[t].isAvgQueuingDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageQueuingDelay"), "float"));
                db->statsMacSummary->addrTypes[t].isAvgMediumAccessDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumAccessDelay"), "float"));
                db->statsMacSummary->addrTypes[t].isAvgMediumDelay = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageMediumDelay"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the average delays.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsMacSummary->addrTypes[t].isAvgQueuingDelay = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageQueuingDelay"), "float"));
            db->statsMacSummary->addrTypes[t].isAvgMediumAccessDelay = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageMediumAccessDelay"), "float"));
            db->statsMacSummary->addrTypes[t].isAvgMediumDelay = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageMediumDelay"), "float"));
        }
    }

    // Check for Average Jitter
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-MAC-SUMMARY-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacSummary->addrTypes[t].isAvgJitter = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
            }
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-MAC-SUMMARY-JITTER \
                parameter, using Default\n");
            for (size_t t = 0; t < addressString.size(); ++t)
            {
                db->statsMacSummary->addrTypes[t].isAvgJitter = TRUE;
                columns.push_back(string_pair(
                    (addressString[t] + "AverageJitter"), "float"));
            }
        }
    }
    else
    {
        // Default case. We add the average jitter.
        for (size_t t = 0; t < addressString.size(); ++t)
        {
            db->statsMacSummary->addrTypes[t].isAvgJitter = TRUE;
            columns.push_back(string_pair(
                (addressString[t] + "AverageJitter"), "float"));
        }
    }

    columns.push_back(
        string_pair("FramesDroppedSender", "bigint unsigned"));
    columns.push_back(
        string_pair("FramesDroppedReceiver", "bigint unsigned"));
    columns.push_back(
        string_pair("BytesDroppedSender", "bigint unsigned"));
    columns.push_back(
        string_pair("BytesDroppedReceiver", "bigint unsigned"));

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = partition->wallClock->getRealTime();
        }

        CreateTable(db, "MAC_Summary", columns);

        if (STATS_DEBUG)
        {
            end = partition->wallClock->getRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create MAC Summary table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBPhySummaryTable
// PURPOSE  : Determine which columns should appear in the PHY_Summary
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBPhySummaryTable(PartitionData* partition,
                                          NodeInput* nodeInput)
{
    // In this function we initialize the Phy Summary Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy summary table.
    DBColumns columns;
    columns.reserve(12);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderID", "integer"));
    columns.push_back(string_pair("ReceiverID", "integer"));
    columns.push_back(string_pair("ChannelIndex", "integer"));
    columns.push_back(string_pair("PhyIndex", "integer"));
    columns.push_back(
        string_pair("NumRcvdSignals", "bigint unsigned"));
    columns.push_back(
        string_pair("NumRcvdErrorSignals", "bigint unsigned"));
    columns.push_back(string_pair("Utilization", "float"));
    columns.push_back(string_pair("AverageInterference", "float"));

    // Check for Path Loss
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-SUMMARY-PATHLOSS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhySummary->isAvgPathLoss = TRUE;
            columns.push_back(string_pair("AveragePathLoss", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-SUMMARY-PATHLOSS parameter,"
                "using Default\n");
            db->statsPhySummary->isAvgPathLoss = TRUE;
            columns.push_back(string_pair("AveragePathLoss", "real"));
        }
    }
    else
    {
        // Default case. We add the path loss.
        db->statsPhySummary->isAvgPathLoss = TRUE;
        columns.push_back(string_pair("AveragePathLoss", "real"));
    }

    // Check for Signal Power
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-SUMMARY-SIGNAL-POWER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhySummary->isAvgSignalPower = TRUE;
            columns.push_back(
                string_pair("AverageSignalPower", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-SUMMARY-SIGNAL-POWER parameter,"
                "using Default\n");
            db->statsPhySummary->isAvgSignalPower = TRUE;
            columns.push_back(
                string_pair("AverageSignalPower", "real"));
        }
    }
    else
    {
        // Default case. We add the signal power.
        db->statsPhySummary->isAvgSignalPower = TRUE;
        columns.push_back(
            string_pair("AverageSignalPower", "real"));
    }

    // Check for Delay
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-SUMMARY-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhySummary->isAvgDelay = TRUE;
            columns.push_back(string_pair("AverageDelay", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-SUMMARY-DELAY parameter,"
                "using Default\n");
            db->statsPhySummary->isAvgDelay = TRUE;
            columns.push_back(string_pair("AverageDelay", "real"));
        }
    }
    else
    {
        // Default case. We add the delay.
        db->statsPhySummary->isAvgDelay = TRUE;
        columns.push_back(string_pair("AverageDelay", "real"));
    }    

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "PHY_Summary", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Phy Summary table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}



void InitializeStatsDBAppEventsTable(PartitionData* partition,
                                     NodeInput* nodeInput)
{
    // In this function we initialize the Application Events Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    BOOL retVal = FALSE;
    char buf[MAX_STRING_LENGTH];
    std::string bufferForAppEventsTbColsName;
    clocktype start = 0;
    clocktype end = 0;
    Int32 intVal = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(19);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("SessionInitiator", "integer"));
    columns.push_back(string_pair("ReceiverId", "integer"));
    columns.push_back(string_pair("ReceiverAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("MessageId", "VARCHAR(64)"));
    columns.push_back(string_pair("Size", "integer"));
    columns.push_back(string_pair("EventType", "VARCHAR(64)"));
    db->statsAppEvents->isSession = TRUE;
    columns.push_back(string_pair("SessionId", "integer"));
    columns.push_back(string_pair("ApplicationType", "text"));
    columns.push_back(string_pair("ApplicationName", "text"));

    // Check for Message Sequence Number
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-MSG-SEQUENCE-NUM", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-EVENTS-MSG-SEQUENCE-NUM parameter,"
                "using Default\n");
            db->statsAppEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsAppEvents->isMsgSeqNum = TRUE;
        columns.push_back(string_pair("MessageSeqNum", "integer"));
    }

#ifdef SOCKET_INTERFACE
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-APPLICATION-EVENTS-SOCKET-INTERFACE-MESSAGE-IDS",
        &wasFound,
        buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isSocketInterfaceMsgIds = TRUE;
            columns.push_back(
                string_pair("SocketInterfaceMessageId1", "bigint unsigned"));
            columns.push_back(
                string_pair("SocketInterfaceMessageId2", "bigint unsigned"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-EVENTS-SOCKET-INTERFACE-MESSAGE-IDS parameter,"
                "using Default\n");
            db->statsAppEvents->isSocketInterfaceMsgIds = FALSE;
        }
    }
    else
    {
        // Default case. We do not add the external msgids.
        db->statsAppEvents->isSocketInterfaceMsgIds = FALSE;
    }
#else
    // Default case. We do not add the external msgids.
    db->statsAppEvents->isSocketInterfaceMsgIds = FALSE;
#endif

    // Check for Priority

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-PRIORITY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isPriority = TRUE;
            columns.push_back(string_pair("Priority", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-EVENTS-PRIORITY parameter,"
                "using Default\n");
            db->statsAppEvents->isPriority = TRUE;
            columns.push_back(string_pair("Priority", "integer"));
        }
    }
    else
    {
        // Default case. We add the Fragment Id number.
        db->statsAppEvents->isPriority = TRUE;
        columns.push_back(string_pair("Priority", "integer"));
    }
    // Check for Message Failure type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-FAILURE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isMsgFailureType = TRUE;
            columns.push_back(
                string_pair("MessageFailureType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-APPLICATION-EVENTS-FAILURE-TYPE parameter,"
                "using Default\n");
            db->statsAppEvents->isMsgFailureType = TRUE;
            columns.push_back(
                string_pair("MessageFailureType", "text"));
        }
    }
    else
    {
        // Default case. We add the Message Failure type.
        db->statsAppEvents->isMsgFailureType = TRUE;
        columns.push_back(string_pair("MessageFailureType", "text"));
    }
    // Check for Delay

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-DELAY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-EVENTS-DELAY parameter,"
                "using Default\n");
            db->statsAppEvents->isDelay = TRUE;
            columns.push_back(string_pair("Delay", "real"));
        }
    }
    else
    {
        // Default case. We add the Delay.
        db->statsAppEvents->isDelay = TRUE;
        columns.push_back(string_pair("Delay", "real"));
    }
    // Check for JITTER

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-APPLICATION-EVENTS-JITTER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsAppEvents->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-APPLICATION-EVENTS-JITTER parameter,"
                "using Default\n");
            db->statsAppEvents->isJitter = TRUE;
            columns.push_back(string_pair("Jitter", "real"));
        }
    }
    else
    {
        // Default case. We add the Jitter.
        db->statsAppEvents->isJitter = TRUE;
        columns.push_back(string_pair("Jitter", "real"));
    }


    // Read whether to record duplicate messages
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
                  "STATS-DB-APPLICATION-EVENTS-RECORD-DUPLICATE",
                  &retVal,
                  buf);
    if (retVal && strcmp(buf, "YES") == 0)
    {
        db->statsAppEvents->recordDuplicate = TRUE;
    }
    else if (!retVal || strcmp(buf, "NO") == 0)
    {
        db->statsAppEvents->recordDuplicate = FALSE;
    }
    else
    {
        ERROR_ReportWarning(
            "Wrong value of parameter "
            "STATS-DB-APPLICATION-EVENTS-RECORD-DUPLICATE. Expect YES/NO");
    }

    
    // Read whether to record out of order messages
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
                  "STATS-DB-APPLICATION-EVENTS-RECORD-OUT-OF-ORDER",
                  &retVal,
                  buf);
    if (retVal && strcmp(buf, "YES") == 0)
    {
        db->statsAppEvents->recordOutOfOrder = TRUE;
    }
    else if (!retVal || strcmp(buf, "NO") == 0)
    {
        db->statsAppEvents->recordOutOfOrder = FALSE;
    }
    else
    {
        ERROR_ReportWarning(
            "Wrong value of parameter "
            "STATS-DB-APPLICATION-EVENTS-RECORD-OUT-OF-ORDER. Expect YES/NO");
    }

    // Read whether to record fragments
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
                  "STATS-DB-APPLICATION-EVENTS-RECORD-FRAGMENT",
                  &retVal,
                  buf);
    if (retVal && strcmp(buf, "YES") == 0)
    {
        db->statsAppEvents->recordFragment = TRUE;
    }
    else if (!retVal || strcmp(buf, "NO") == 0)
    {
        db->statsAppEvents->recordFragment = FALSE;
    }
    else
    {
        ERROR_ReportWarning(
            "Wrong value of parameter "
            "STATS-DB-APPLICATION-EVENTS-RECORD-FRAGMENT. Expect YES/NO");
    }

    // Read the limit of the sequence number cache size
    IO_ReadInt(ANY_NODEID, ANY_ADDRESS, nodeInput,
               "STATS-DB-APPLICATION-SEQUENCE-NUMBER-CACHE-SIZE",
               &retVal,
               &intVal);
    if (retVal && (intVal == 0 || intVal > 1))
    {
        db->statsAppEvents->seqCacheSize = intVal;
    }
    else if (!retVal)
    {
        db->statsAppEvents->seqCacheSize = 0;
    }
    else
    {
        ERROR_ReportWarning(
            "Wrong value of parameter "
            "STATS-DB-APPLICATION-SEQUENCE-NUMBER-CACHE-SIZE. "
            "Must be 0 or larger than 1.");
    }
    // we should use receiver Id instead of the session Id in the long run

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }
    
        CreateTable(db, "APPLICATION_Events", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create App table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }

    wasFound = FALSE;
    BOOL readValue = FALSE;

    IO_ReadBool(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-APPLICATION-EVENTS-MULTIPLE-VALUES",
        &wasFound,
        &readValue);

    if (wasFound)
    {
        if (readValue)
        {
            db->statsAppEvents->multipleValues = TRUE;
        }
    }
    else
    {
        // Default case.
        db->statsAppEvents->multipleValues = TRUE;
    }

    Int32 values = 0;
    IO_ReadInt(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-APPLICATION-EVENTS-BUFFER-SIZE",
        &wasFound,
        &values);

    if (db->statsAppEvents->multipleValues == FALSE && wasFound)
    {
        ERROR_ReportError("STATS-DB-APPLICATION-EVENTS-BUFFER-SIZE should be"
            " configured only when"
            " STATS-DB-APPLICATION-EVENTS-MULTIPLE-VALUES is enabled.\n");
    }

    if (db->statsAppEvents->multipleValues == TRUE && wasFound)
    {
        ERROR_Assert(values >= MIN_EVENT_TABLE_BUFFER,
            "Please specifiy a valid "
            "STATS-DB-APPLICATION-EVENTS-BUFFER-SIZE. \n"
            "The minimum required buffer size is 500 bytes. \n");
            db->statsAppEvents->bufferSizeInBytes = values;
    }

    bufferForAppEventsTbColsName += columns[0].first;
    for (size_t i = 1; i < columns.size(); i++)
    {
        if (columns[i].first.length() > 0)
        {
            bufferForAppEventsTbColsName += ",";
            bufferForAppEventsTbColsName += columns[i].first;
        }
    }
    db->statsAppEvents->appEventsTbColsName =
                        "INSERT INTO APPLICATION_Events ("
                        + bufferForAppEventsTbColsName
                        + ") VALUES";

}

void InitializeStatsDBNetworkEventsTable(PartitionData* partition,
        NodeInput* nodeInput)
{
    // In this function we initialize the Network events Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    string nonAutoCols;
    // Now to create the network packet table.
    DBColumns columns;
    columns.reserve(15);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("MessageId", "VARCHAR(64)"));
    columns.push_back(
        string_pair("SenderAddress", "VARCHAR(64)"));
    columns.push_back(
        string_pair("ReceiverAddress", "VARCHAR(64)"));
    columns.push_back(string_pair("PacketSize", "integer"));
    columns.push_back(string_pair("EventType", "VARCHAR(64)"));
    nonAutoCols += "Timestamp,";
    nonAutoCols += "NodeId,";
    nonAutoCols += "MessageId,";
    nonAutoCols += "SenderAddress,";
    nonAutoCols += "ReceiverAddress,";
    nonAutoCols += "PacketSize,";
    nonAutoCols += "EventType";
    // Check for Interface Index
    // Interface Index is a required column since it composes the
    // primary key
    db->statsNetEvents->isInterfaceIndex = TRUE;
    columns.push_back(string_pair("InterfaceIndex", "VARCHAR(16)"));
    nonAutoCols += ",InterfaceIndex";

    // Check for Message Sequence Number
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-MSG-SEQUENCE-NUM", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-NETWORK-EVENTS-MSG-SEQUENCE-NUM parameter,"
                "using Default\n");
            db->statsNetEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsNetEvents->isMsgSeqNum = TRUE;
        columns.push_back(string_pair("MessageSeqNum", "integer"));
    }

    if (db->statsNetEvents->isMsgSeqNum)
    {
        nonAutoCols += ",MessageSeqNum";
    }

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-DATA-HEADER-SIZE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isControlSize = TRUE;
            columns.push_back(
                string_pair("OverheadSize", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-DATA-HEADER-SIZE parameter,"
                "using Default\n");
            db->statsNetEvents->isControlSize = TRUE;
            columns.push_back(
                string_pair("OverheadSize", "integer"));
        }

    }
    else
    {
        // Default case. We add the Fragment Id number.
        db->statsNetEvents->isControlSize = TRUE;
        columns.push_back(string_pair("OverheadSize", "integer"));
    }
    if (db->statsNetEvents->isControlSize)
    {
        nonAutoCols += ",OverheadSize";
    }
    // Check for Packet Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-MESSAGE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isPktType = TRUE;
            columns.push_back(string_pair("PacketType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-MESSAGE-TYPE parameter,"
                "using Default\n");
            db->statsNetEvents->isPktType = TRUE;
            columns.push_back(string_pair("PacketType", "text"));
        }
    }
    else
    {
        // Default case. We add the packet type.
        db->statsNetEvents->isPktType = TRUE;
        columns.push_back(string_pair("PacketType", "text"));
    }
    if (db->statsNetEvents->isPktType)
    {
        nonAutoCols += ",PacketType";

    // Check for Protocol Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isProtocolType = TRUE;
            columns.push_back(string_pair("ProtocolType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-PROTOCOL parameter,"
                "using Default\n");
            db->statsNetEvents->isProtocolType = TRUE;
            columns.push_back(string_pair("ProtocolType", "text"));
        }
    }
    else
    {
        // Default case. We add the Protocol Type.
        db->statsNetEvents->isProtocolType = TRUE;
        columns.push_back(string_pair("ProtocolType", "text"));
    }

    if (db->statsNetEvents->isProtocolType)
    {
        nonAutoCols += ",ProtocolType";
    }
    // Check for Priority

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-PRIORITY", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isPriority = TRUE;
            columns.push_back(string_pair("Priority", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-PRIORITY parameter,"
                "using Default\n");
            db->statsNetEvents->isPriority = TRUE;
            columns.push_back(string_pair("Priority", "integer"));
        }
    }
    else
    {
        // Default case. We add the priority number.
        db->statsNetEvents->isPriority = TRUE;
        columns.push_back(string_pair("Priority", "integer"));
    }

    if (db->statsNetEvents->isPriority)
    {
        nonAutoCols += ",Priority";
    }

    

    // Check for Message Failure type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-FAILURE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isPktFailureType = TRUE;
            columns.push_back(string_pair("PacketFailureType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-FAILURE-TYPE parameter,"
                "using Default\n");
            db->statsNetEvents->isPktFailureType = TRUE;
            columns.push_back(string_pair("PacketFailureType", "text"));
        }
    }
    else
    {
        // Default case. We add the packet Failure type.
        db->statsNetEvents->isPktFailureType = TRUE;
        columns.push_back(string_pair("PacketFailureType", "text"));
    }

    if (db->statsNetEvents->isPktFailureType)
    {
        nonAutoCols += ",PacketFailureType";
    }
    
    }
 
    // Check for Hop Count

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-EVENTS-HOP-COUNT", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-HOP-COUNT parameter,"
                "using Default\n");
            db->statsNetEvents->isHopCount = TRUE;
            columns.push_back(string_pair("HopCount", "real"));
        }
    }
    else
    {
        // Default case. We add the Interface index.
        db->statsNetEvents->isHopCount = TRUE;
        columns.push_back(string_pair("HopCount", "real"));
    }
    if (db->statsNetEvents->isHopCount)
    {
        nonAutoCols += ",HopCount";
    }

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }
    
        CreateTable(db, "NETWORK_Events", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create network table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }

    // Check for Control
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "NETWORK-STATS-DB-CONTROL",
        &wasFound,
        buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->networkStatsDBControl = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for NETWORK-STATS-DB-CONTROL parameter,"
                "using Default\n");
        }

    }
    else
    {
        // Default case.
        db->statsNetEvents->networkStatsDBControl = TRUE;
    }

    // Check for Incoming
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "NETWORK-STATS-DB-INCOMING",
        &wasFound,
        buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->networkStatsDBIncoming = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for NETWORK-STATS-DB-INCOMING parameter,"
                "using Default\n");
        }
    }
    else
    {
        // Default case.
        db->statsNetEvents->networkStatsDBIncoming = TRUE;
    }

    // Check for Outgoing
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "NETWORK-STATS-DB-OUTGOING",
        &wasFound,
        buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->networkStatsDBOutgoing = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for NETWORK-STATS-DB-OUTGOING parameter,"
                "using Default\n");
        }
    }
    else
    {
        // Default case.
        db->statsNetEvents->networkStatsDBOutgoing = TRUE;
    }

    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-NETWORK-EVENTS-MULTIPLE-VALUES",
        &wasFound,
        buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetEvents->multipleValues = TRUE;
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-NETWORK-EVENTS-MULTIPLE-VALUES parameter,"
                "using Default\n");
        }
    }
    else
    {
        db->statsNetEvents->multipleValues = TRUE;
    }

    if (db->statsNetEvents->multipleValues)
    {
        Int32 values = 0;
        IO_ReadInt(
            ANY_NODEID,
            ANY_ADDRESS,
            nodeInput,
            "STATS-DB-NETWORK-EVENTS-BUFFER-SIZE",
            &wasFound,
            &values);
        if (wasFound)
        {

            ERROR_Assert(values >= 500,
                "Please specifiy a valid STATS-DB-NETWORK-EVENTS-BUFFER-SIZE.\n"
                "The minimum required buffer size is 500 bytes.");
            db->statsNetEvents->bufferSizeInBytes = values;

        }

        db->networkEventsString = (char*)
            MEM_malloc(db->statsNetEvents->bufferSizeInBytes);

        memset(db->networkEventsString,
            0, db->statsNetEvents->bufferSizeInBytes);
    }
    sprintf(db->statsNetEvents->networkEventsTbColsName,
            "INSERT INTO NETWORK_Events (%s) VALUES",
            nonAutoCols.c_str());

    db->statsNetEvents->lengthOfNetworkEventsTbName =
        strlen(db->statsNetEvents->networkEventsTbColsName);
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBPhyEventsTable
// PURPOSE  : Determine which columns should appear in the PHY_Events
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBPhyEventsTable(PartitionData* partition,
        NodeInput* nodeInput)
{
    // In this function we initialize the phy events Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the phy events table.
    DBColumns columns;
    columns.reserve(12);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeID", "integer"));
    columns.push_back(string_pair("MessageID", "VARCHAR(64)"));
    columns.push_back(string_pair("PhyIndex", "integer"));
    columns.push_back(string_pair("Size", "integer"));
    columns.push_back(string_pair("EventType", "VARCHAR(64)"));

    // Check for channel index
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-CHANNEL-INDEX", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isChannelIndex = TRUE;
            columns.push_back(string_pair("ChannelIndex", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-PHY-EVENTS-CHANNEL-INDEX parameter,"
                "using Default\n");
            db->statsPhyEvents->isChannelIndex = TRUE;
            columns.push_back(string_pair("ChannelIndex", "integer"));
        }
    }
    else
    {
        // Default case. We add the channel index
        db->statsPhyEvents->isChannelIndex = TRUE;
        columns.push_back(string_pair("ChannelIndex", "integer"));
    }

    // Check for control size
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-PREAMBLE-SIZE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isControlSize = TRUE;
            columns.push_back(string_pair("OverheadSize", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-EVENTS-PREAMBLE-SIZE parameter,"
                "using Default\n");
            db->statsPhyEvents->isControlSize = TRUE;
            columns.push_back(string_pair("OverheadSize", "integer"));
        }
    }
    else
    {
        // Default case. We add the control size
        db->statsPhyEvents->isControlSize = TRUE;
        columns.push_back(string_pair("OverheadSize", "integer"));
    }

    // Check for message failure type
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-FAILURE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isMessageFailureType = TRUE;
            columns.push_back(string_pair("FailureType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-EVENTS-FAILURE-TYPE parameter,"
                "using Default\n");
            db->statsPhyEvents->isMessageFailureType = TRUE;
            columns.push_back(string_pair("FailureType", "text"));
        }
    }
    else
    {
        // Default case. We add the failure type
        db->statsPhyEvents->isMessageFailureType = TRUE;
        columns.push_back(string_pair("FailureType", "text"));
    }

    // Check for signal power
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-SIGNAL-POWER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isSignalPower = TRUE;
            columns.push_back(string_pair("SignalPower", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-EVENTS-SIGNAL-POWER parameter,"
                "using Default\n");
            db->statsPhyEvents->isSignalPower = TRUE;
            columns.push_back(string_pair("SignalPower", "real"));
        }
    }
    else
    {
        // Default case. We add the signal power
        db->statsPhyEvents->isSignalPower = TRUE;
        columns.push_back(string_pair("SignalPower", "real"));
    }

    // Check for interference
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-INTERFERENCE-POWER", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isInterference = TRUE;
            columns.push_back(string_pair("Interference", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-PHY-EVENTS-INTERFERENCE-POWER parameter,"
                "using Default\n");
            db->statsPhyEvents->isInterference = TRUE;
            columns.push_back(string_pair("Interference", "real"));
        }
    }
    else
    {
        // Default case. We add the interference
        db->statsPhyEvents->isInterference = TRUE;
        columns.push_back(string_pair("Interference", "real"));
    }

    // Check for path loss
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-PHY-EVENTS-PATHLOSS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsPhyEvents->isPathLoss = TRUE;
            columns.push_back(string_pair("PathLoss", "real"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-PHY-EVENTS-PATHLOSS parameter,"
                "using Default\n");
            db->statsPhyEvents->isPathLoss = TRUE;
            columns.push_back(string_pair("PathLoss", "real"));
        }
    }
    else
    {
        // Default case. We add the path loss
        db->statsPhyEvents->isPathLoss = TRUE;
        columns.push_back(string_pair("PathLoss", "real"));
    }
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }
    
        CreateTable(db, "PHY_Events", columns);
        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create network table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

// This function should be generic
void InitializeStatsDBMessageIdMappingTable(
                                        PartitionData* partition,
                                        NodeInput* nodeInput)
{
    clocktype start = 0;
    clocktype end = 0;
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // In this function we initialize the Transport Mapping Table.
    // That is we create the table with the columns based on the
    // user input.

    // Now to create the application message table.
    DBColumns columns;
    columns.reserve(5);
    columns.push_back( string_pair("Timestamp", "real"));
    columns.push_back( string_pair("NodeId", "integer"));
    columns.push_back( string_pair("MessageIdIn", "text"));
    columns.push_back( string_pair("MessageIdOut", "text"));
    columns.push_back( string_pair("Protocol", "text"));

    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }

    CreateTable(db, "Message_Id_Mapping", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Message ID mapping table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBTransEventsTable
// PURPOSE  : Determine which columns should appear in the TRANSPORT_Events
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBTransEventsTable(PartitionData* partition,
                                     NodeInput* nodeInput)
{

    // In this function we initialize the Transport Events Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the application message table.
    DBColumns columns;
    columns.reserve(17);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("MessageId", "VARCHAR(64)"));
    columns.push_back(string_pair("Size", "integer"));
    db->statsTransEvents->isSenderPort = TRUE;
    columns.push_back(string_pair("SenderPort", "integer"));
    db->statsTransEvents->isReceiverPort = TRUE;
    columns.push_back(string_pair("ReceiverPort", "integer"));

    // Check for Message Sequence Number

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-MSG-SEQUENCE-NUM", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-EVENTS-MSG-SEQUENCE-NUM parameter,"
                "using Default\n");
            db->statsTransEvents->isMsgSeqNum = TRUE;
            columns.push_back(string_pair("MessageSeqNum", "integer"));
        }
    }
    else
    {
        // Default case. We add the message Sequence number.
        db->statsTransEvents->isMsgSeqNum = TRUE;
        columns.push_back(string_pair("MessageSeqNum", "integer"));
    }
    // Check for Connection Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-CONNECTION-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isConnType = TRUE;
            columns.push_back(string_pair("ConnectionType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-EVENTS-CONNECTION-TYPE parameter,"
                "using Default\n");
            db->statsTransEvents->isConnType = TRUE;
            columns.push_back(string_pair("ConnectionType", "text"));
        }
    }
    else
    {
        // Default case. We add the connection type.
        db->statsTransEvents->isConnType = TRUE;
        columns.push_back(string_pair("ConnectionType", "text"));
    }

    // Check for Header Size

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-DATA-HEADER-SIZE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isHdrSize = TRUE;
            columns.push_back(
                string_pair("OverheadSize", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for STATS-DB-TRANSPORT-EVENTS-DATA-HEADER-SIZE parameter,"
                "using Default\n");
            db->statsTransEvents->isHdrSize = TRUE;
            columns.push_back(
                string_pair("OverheadSize", "integer"));
        }
    }
    else
    {
        // Default case. We add the connection type.
        db->statsTransEvents->isHdrSize = TRUE;
        columns.push_back(string_pair("OverheadSize", "integer"));
    }

    // Check for Segment Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-SEGMENT-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isFlags = TRUE;
            columns.push_back(string_pair("SegmentType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-TRANSPORT-EVENTS-SEGMENT-TYPE parameter,"
                "using Default\n");
            db->statsTransEvents->isFlags = TRUE;
            columns.push_back(string_pair("SegmentType", "text"));
        }
    }
    else
    {
        // Default case. We add the priority.
        db->statsTransEvents->isFlags = TRUE;
        columns.push_back(string_pair("SegmentType", "text"));
    }

     // Check for Event Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-EVENT-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isEventType = TRUE;
            columns.push_back(string_pair("EventType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-TRANSPORT-EVENTS-EVENT-TYPE parameter,"
                "using Default\n");
            db->statsTransEvents->isEventType = TRUE;
            columns.push_back(string_pair("EventType", "text"));
        }
    }
    else
    {
        // Default case. We add the priority.
        db->statsTransEvents->isEventType = TRUE;
        columns.push_back(string_pair("EventType", "text"));
    }

    // Check for Message Failure type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-TRANSPORT-EVENTS-FAILURE-TYPE", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsTransEvents->isMsgFailureType = TRUE;
            columns.push_back(
                string_pair("MessageFailureType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-TRANSPORT-EVENTS-FAILURE-TYPE parameter,"
                "using Default\n");
            db->statsTransEvents->isMsgFailureType = TRUE;
            columns.push_back(
                string_pair("MessageFailureType", "text"));
        }
    }
    else
    {
        // Default case. We add the Message Failure type.
        db->statsTransEvents->isMsgFailureType = TRUE;
        columns.push_back(string_pair("MessageFailureType", "text"));
    }

    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }
    
        CreateTable(db, "TRANSPORT_Events", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Transport table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBAppConnTable(PartitionData* partition,
    NodeInput* nodeInput)
{
    // In this function we initialize the Network connectivity Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    db->statsConnTable->v_AppConnParam = new StatsDBConnTable::V_AppConnParam;
    // Check for the Connectivity table timer
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-APPLICATION-CONNECTIVITY-INTERVAL",
        &wasFound,
        buf);

    if (wasFound)
    {
       db->statsConnTable->AppConnTableInterval = TIME_ConvertToClock(buf);
    }
    ERROR_Assert(db->statsConnTable->AppConnTableInterval > 0,
        "Error in STATS-DB-APPLICATION-CONNECTIVITY-INTERVAL. ");

    if (partition->partitionId == 0)
    {
        // Now to create the network connectivity table.
        DBColumns columns;
        columns.reserve(4);
        columns.push_back(string_pair("Timestamp", "real"));
        columns.push_back(string_pair("SenderAddr", "VARCHAR(64)"));
        columns.push_back(string_pair("ReceiverAddr", "text"));
        columns.push_back(string_pair("SessionId", "integer"));
    
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "APPLICATION_Connectivity", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Application Conn table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBTransConnTable(PartitionData* partition,
    NodeInput* nodeInput)
{
    // In this function we initialize the Network connectivity Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    db->statsConnTable->v_TransConnParam = new StatsDBConnTable::V_TransConnParam;
    // Check for the Connectivity table timer
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-TRANSPORT-CONNECTIVITY-INTERVAL",
        &wasFound,
        buf);

    if (wasFound)
    {
       db->statsConnTable->TransConnTableInterval = TIME_ConvertToClock(buf);
    }
    ERROR_Assert(db->statsConnTable->TransConnTableInterval > 0,
        "Error in STATS-DB-TRANSPORT-CONNECTIVITY-INTERVAL. ");

    if (partition->partitionId == 0)
    {
        // Now to create the network connectivity table.
        DBColumns columns;
        columns.reserve(5);
        columns.push_back(string_pair("Timestamp", "real"));
        columns.push_back(string_pair("SenderAddr", "VARCHAR(64)"));
        columns.push_back(string_pair("SenderPort", "integer"));
        columns.push_back(string_pair("ReceiverAddr", "VARCHAR(64)"));
        columns.push_back(string_pair("ReceiverPort", "integer"));

        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "TRANSPORT_Connectivity", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Transport Conn table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBNetworkConnTable(
    PartitionData* partition,
    NodeInput* nodeInput)
{
    // In this function we initialize the Network connectivity Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    char buf[MAX_STRING_LENGTH];


    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the network connectivity table.
    DBColumns columns;
    columns.reserve(11);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(
        string_pair("DestinationAddr", "VARCHAR(64)"));
    columns.push_back(string_pair("Cost", "real"));
    db->statsNetConn->isDstMaskAddr = TRUE;
    columns.push_back(string_pair("DestMaskAddr", "VARCHAR(64)"));
    db->statsNetConn->isAdminDistance = TRUE;
    columns.push_back(string_pair("AdminDistance", "integer"));
    
    // Check for the Next Hop Address

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-CONNECTIVITY-NEXT-HOP-ADDRESS", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetConn->isNextHopAddr = TRUE;
            columns.push_back(string_pair("NextHopAddr", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-NETWORK-CONNECTIVITY-NEXT-HOP-ADDRESS parameter,"
                "using Default\n");
            db->statsNetConn->isNextHopAddr = TRUE;
            columns.push_back(string_pair("NextHopAddr", "text"));
        }
    }
    else
    {
        // Default case. We add the Next Hop Address.
        db->statsNetConn->isNextHopAddr = TRUE;
        columns.push_back(string_pair("NextHopAddr", "text"));
    }

    // Check for the Outgoing Interface Index
    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-CONNECTIVITY-OUTGOING-INTERFACE-INDEX", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetConn->isOutgoingInterfaceIndex = TRUE;
            columns.push_back(
                string_pair("OutgoingInterfaceIndex", "integer"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-NETWORK-CONNECTIVITY-OUTGOING-INTERFACE-INDEX parameter,"
                "using Default\n");
            db->statsNetConn->isOutgoingInterfaceIndex = TRUE;
            columns.push_back(
                string_pair("OutgoingInterfaceIndex", "integer"));
        }
    }
    else
    {
        // Default case. We add the Outgoing Interface Index.
        db->statsNetConn->isOutgoingInterfaceIndex = TRUE;
        columns.push_back(
            string_pair("OutgoingInterfaceIndex", "integer"));
    }

    

    // Check for the Routing Protocol Type

    IO_ReadString(ANY_NODEID, ANY_ADDRESS, nodeInput,
        "STATS-DB-NETWORK-CONNECTIVITY-ROUTING-PROTOCOL", &wasFound, buf);
    if (wasFound)
    {
        if (strcmp(buf, "YES") == 0)
        {
            db->statsNetConn->isRoutingProtocol = TRUE;
            columns.push_back(
                string_pair("RoutingProtocolType", "text"));
        }
        else if (strcmp(buf, "NO") != 0)
        {
            // We have invalid values.
            ERROR_ReportWarning(
                "Invalid Value for "
                "STATS-DB-NETWORK-CONNECTIVITY-ROUTING-PROTOCOL parameter,"
                "using Default\n");
            db->statsNetConn->isRoutingProtocol = TRUE;
            columns.push_back(
                string_pair("RoutingProtocolType", "text"));
        }
    }
    else
    {
        // Default case. We add the Routing protocol type.
        db->statsNetConn->isRoutingProtocol = TRUE;
        columns.push_back(
            string_pair("RoutingProtocolType", "text"));
    }
    if (partition->partitionId == 0)
    {
        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "NETWORK_Connectivity", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Net Conn table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
    IO_ReadTime(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-NETWORK-CONNECTIVITY-INTERVAL",
        &wasFound,
        &(db->statsConnTable->networkConnSampleTimeInterval));

    if (wasFound)
    {
        ERROR_Assert(db->statsConnTable->networkConnSampleTimeInterval > 0,
            "Please specify a valid STATS-DB-NETWORK-CONNECTIVITY-INTERVAL");
    }
    else
    {
        // set to default value
        db->statsConnTable->networkConnSampleTimeInterval =
            STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
    }
}

void InitializeStatsDBMacConnTable(PartitionData* partition,
                                   NodeInput* nodeInput)
{
    // In this function we initialize the Mac connectivity Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (partition->partitionId == 0)
    {
        // Now to create the network connectivity table.
        DBColumns columns;
        columns.reserve(7);
        columns.push_back(string_pair("Timestamp", "real"));
        columns.push_back(string_pair("SenderId", "integer"));
        columns.push_back(string_pair("ReceiverId", "integer"));
        db->statsMacConn->isInterfaceIndex = TRUE;
        columns.push_back(
            string_pair("SenderInterfaceIndex", "integer"));
        db->statsMacConn->isChannelIndex = TRUE;
        columns.push_back(
            string_pair("ChannelIndex", "VARCHAR(64)"));

        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }
        CreateTable(db, "MAC_Connectivity", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Mac Conn table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }

    // Check for the timer interval
    IO_ReadTime(
        partition->partitionId,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-MAC-CONNECTIVITY-INTERVAL",
        &wasFound,
        &(db->statsConnTable->macConnSampleTimeInterval));

    if (wasFound)
    {
        ERROR_Assert(db->statsConnTable->macConnSampleTimeInterval > 0,
            "Please specify a valid STATS-DB-MAC-CONNECTIVITY-INTERVAL");
    }
    else
    {
        // set to default value
        db->statsConnTable->macConnSampleTimeInterval =
            STATSDB_DEFAULT_CONNECTIVITY_INTERVAL;
    }

}

void InitializeStatsDBPhyConnTable(PartitionData* partition,
                                   NodeInput* nodeInput)
{
    // In this function we initialize the Physical connectivity Table.
    // That is we create the table with the columns based on the
    // user input.
    BOOL wasFound = FALSE;

    clocktype start = 0;
    clocktype end = 0;
    char buf[MAX_STRING_LENGTH];

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Check for the Connectivity table timer
    IO_ReadString(
        ANY_NODEID,
        ANY_ADDRESS,
        nodeInput,
        "STATS-DB-PHY-CONNECTIVITY-INTERVAL",
        &wasFound,
        buf);

    if (wasFound)
    {
       db->statsConnTable->connectSampleTimeInterval = TIME_ConvertToClock(buf);
    }
    ERROR_Assert(db->statsConnTable->connectSampleTimeInterval > 0,
        "Error in STATS-DB-PHY-CONNECTIVITY-INTERVAL. ");

    if (partition->partitionId == 0)
    {
        // Now to create the network connectivity table.
        DBColumns columns;
        columns.reserve(13);
        columns.push_back(string_pair("Timestamp", "real"));
        columns.push_back(string_pair("SenderId", "integer"));
        columns.push_back(string_pair("ReceiverId", "integer"));
        db->statsPhyConn->isPhyIndex = TRUE;
        columns.push_back(string_pair("SenderPhyIndex", "integer"));
        columns.push_back(string_pair("ReceiverPhyIndex", "integer"));
        db->statsPhyConn->isChannelIndex = TRUE;
        columns.push_back(string_pair("ChannelIndex", "text"));
        columns.push_back(string_pair("SenderListening", "text"));
        columns.push_back(string_pair("ReceiverListening", "text"));
        columns.push_back(string_pair("AntennaType", "text"));
        columns.push_back(string_pair("BestAngle", "text"));
        columns.push_back(string_pair("WorstAngle", "text"));

        if (STATS_DEBUG)
        {
            start = WallClock::getTrueRealTime();
        }

        CreateTable(db, "PHY_Connectivity", columns);

        if (STATS_DEBUG)
        {
            end = WallClock::getTrueRealTime();
            clocktype diff = end - start;
            char temp[MAX_STRING_LENGTH];
            TIME_PrintClockInSecond(diff, temp);
            printf ("Time Taken to create Phy COnn table partition %d, %s\n",
                    partition->partitionId,
                    temp);
        }
    }
}

void InitializeStatsDBMulticastConnTable(PartitionData* partition, NodeInput* nodeInput)
{
    // In this function we initialize the multicast connectivity Table.
    // That is we create the table with the columns
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create the multicast connectivity table.
    DBColumns columns;
    columns.reserve(8);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("SenderId", "integer"));
    columns.push_back(string_pair("DestAddr", "varchar(64)"));
    columns.push_back(string_pair("RootNodeType", "varchar(64)"));
    columns.push_back(string_pair("RootNodeId", "integer"));
    columns.push_back(string_pair("OutgoingInterfaceIndex", "varchar(64)"));
    columns.push_back(string_pair("UpstreamNodeId", "varchar(64)"));
    columns.push_back(string_pair("UpstreamInterface",  "varchar(64)"));

    std::vector<std::string> uniqueColumns;
    uniqueColumns.reserve(6);
    uniqueColumns.push_back("Timestamp");
    uniqueColumns.push_back("SenderId");
    uniqueColumns.push_back("DestAddr");
    uniqueColumns.push_back("RootNodeType");
    uniqueColumns.push_back("RootNodeId");
    uniqueColumns.push_back("OutgoingInterfaceIndex");

    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }
    CreateTable(db,
        "MULTICAST_Connectivity",
        columns,
        uniqueColumns,
        "uniquenessconst");

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create multicast COnn table partition %d, %s\n",
            partition->partitionId,
            temp);
    }
}

void
CreateDBStatsLinkUtilizationTable(PartitionData* partition,
                NodeInput* nodeInput, const std::string* str)
{
    // In this function we create the Link Utilization Table.
    clocktype start = 0;
    clocktype end = 0;
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    // Now to create Link Utilization Table
    DBColumns columns;
    columns.reserve(10);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("ChannelIndex", "integer"));
    columns.push_back(string_pair("SubnetId", "integer"));
    columns.push_back(string_pair("RegionId", "integer"));
    columns.push_back(
        string_pair("NumAllocSlotsPerInterval", "bigint unsigned"));
    columns.push_back(
        string_pair("NumIdleSlotsPerInterval", "bigint unsigned"));
    columns.push_back(
        string_pair("NumSlotsForDataPerInterval", "bigint unsigned"));
    columns.push_back(
        string_pair("MinNumAllocSlotsPerFrame", "bigint unsigned"));
    columns.push_back(
        string_pair("MaxNumAllocSlotsPerFrame", "bigint unsigned"));
    columns.push_back(
        string_pair("AvgNumAllocSlotsPerFrame", "bigint unsigned"));

    CreateTable(db, *str + "_Link_Utilization", columns);

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }

    columns.clear();
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("InterfaceIndex", "integer"));
    columns.push_back(string_pair("FrameId", "bigint unsigned"));
    columns.push_back(string_pair("ChannelIndex", "integer"));
    columns.push_back(string_pair("SubnetId", "integer"));
    columns.push_back(string_pair("RegionId", "integer"));
    columns.push_back(string_pair("NumSlotsPerFrame", "integer"));
    columns.push_back(string_pair("SlotId", "integer"));
    columns.push_back(string_pair("SlotType", "text"));
    CreateTable(db, *str + "_Link_Utilization_Per_Node", columns);

    //columns.clear();
    //columns.push_back(string_pair("Timestamp", "real"));
    ////columns.push_back(string_pair("NodeId", "integer"));
    //columns.push_back(string_pair("FrameId", "integer"));
    //columns.push_back(string_pair("ChannelIndex", "integer"));
    //columns.push_back(string_pair("SubnetId", "integer"));
    //columns.push_back(string_pair("RegionId", "integer"));
    //columns.push_back(string_pair("SlotId", "integer"));
    //columns.push_back(string_pair("SlotUsedByNode", "text"));
    //columns.push_back(string_pair("SlotType", "text"));
    //CreateTable(db, "Link_Utilization_Per_Frame", columns);

    if (STATS_DEBUG)
    {
        end = partition->wallClock->getRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf("Time Taken to create Link utilization Table"
            "table partition %d, %s\n", partition->partitionId, temp);
    }
    return;

}
//--------------------------------------------------------------------------
// FUNCTION:   InitializeStatsDBLinkUtilizationTable
// PURPOSE:  to create LinkUtilization table according to user input.
// PARAMETERS
// + node : Node* : Pointer to a node
// + nodeInput : NodeInput*: pointer to the node input.
// RETURN void.
//--------------------------------------------------------------------------
void
InitializeStatsDBLinkUtilizationTable(PartitionData* partition,
                NodeInput* nodeInput)
{

    BOOL wasFound = FALSE;
    BOOL createTable = FALSE;

    StatsDb* db = NULL;
    db = partition->statsDb;
    db->StatsDBLinkUtilTable = NULL;

#ifdef ADDON_BOEINGFCS
    // Check which tables are to be defined
    // Check for the user configurations.
    std::string str;
    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
                    "STATS-DB-WNW-LINK-UTILIZATION-TABLE", &wasFound,
                    &createTable);
    if (wasFound)
    {
        if (createTable)
        {
            db->StatsDBLinkUtilTable = new StatsDBLinkUtilizationTable;
            db->StatsDBLinkUtilTable->createLinkUtilizationTable = TRUE;
            db->StatsDBLinkUtilTable->createWNWLinkUtilizationTable
                            = TRUE;

            // Initilize the table
            if (partition->partitionId == 0)
            {
                str = "WNW";
                CreateDBStatsLinkUtilizationTable(partition, nodeInput, &str);
            }
            IO_ReadTime(
                            ANY_NODEID,
                            ANY_ADDRESS,
                            nodeInput,
                            "STATS-DB-WNW-LINK-UTILIZATION-TABLE-INTERVAL",
                            &wasFound,
                            &db->StatsDBLinkUtilTable->
                            wnwLinkUtilizationTableInterval);
            if (wasFound)
            {
                ERROR_Assert(db->StatsDBLinkUtilTable->
                                wnwLinkUtilizationTableInterval > 0,
                                "STATS-DB-WNW-UTILIZATION-TABLE-INTERVAL"
                                "should be greater than 0.");
            }
            else
            {
                db->StatsDBLinkUtilTable->wnwLinkUtilizationTableInterval
                                = STATSDB_DEFAULT_LINK_UTILIZATION_INTERVAL;
            }
        }

    }

    IO_ReadBool(ANY_NODEID, ANY_ADDRESS, nodeInput,
                    "STATS-DB-NCW-LINK-UTILIZATION-TABLE",
                    &wasFound,
                    &createTable);
    if (wasFound)
    {
        if (createTable)
        {
            if (!db->StatsDBLinkUtilTable)
            {
                db->StatsDBLinkUtilTable
                                = new StatsDBLinkUtilizationTable;
            }
            db->StatsDBLinkUtilTable->createLinkUtilizationTable = TRUE;
            db->StatsDBLinkUtilTable->createNCWLinkUtilizationTable
                            = TRUE;

            // Initilize the table
            if (partition->partitionId == 0)
            {
                str = "NCW";
                CreateDBStatsLinkUtilizationTable(partition, nodeInput, &str);
            }
            IO_ReadTime(
                            ANY_NODEID,
                            ANY_ADDRESS,
                            nodeInput,
                            "STATS-DB-NCW-LINK-UTILIZATION-TABLE-INTERVAL",
                            &wasFound,
                            &db->StatsDBLinkUtilTable->
                            ncwLinkUtilizationTableInterval);
            if (wasFound)
            {
                ERROR_Assert(db->StatsDBLinkUtilTable->
                                ncwLinkUtilizationTableInterval > 0,
                                "STATS-DB-NCW-UTILIZATION-TABLE-INTERVAL"
                                "should be greater than 0.");
            }
            else
            {
                db->StatsDBLinkUtilTable->ncwLinkUtilizationTableInterval
                                = STATSDB_DEFAULT_LINK_UTILIZATION_INTERVAL;
            }
        }

    }
#endif
    return;

}
// Event Hanlders from this point one =======================================================
// ==========================================================================================
//--------------------------------------------------------------------------
// FUNCTION:  HandleStatsDBLinkUtilizationInsertion
// PURPOSE:   to handle link utilization insert.
// PARAMETERS
// + node: Node* : Pointer to node.
// + msg : Message* : pointer to message
// RETURN void.
//--------------------------------------------------------------------------

void
HandleStatsDBLinkUtilizationInsertion(Node* node, const std::string* str)
{

    StatsDb* db = node->partitionData->statsDb;
    vector<StatsDBLinkUtilizationParam> linkUtilizationParam;//defined in dbapi.h

    // Check if util  stats are on.
    if (db == NULL
        || !db->StatsDBLinkUtilTable->createLinkUtilizationTable)
    {
        return;
    }


    STATSDB_HandleLinkUtilTableCalculate
                (node,&linkUtilizationParam, str);

    STATSDB_HandleLinkUtilizationTableInsert
                (node, &linkUtilizationParam, str);

 }

// ==========================================================================================
//--------------------------------------------------------------------------
// FUNCTION:  HandleStatsDBLinkUtilizationPerNodeInsertion
// PURPOSE:   to handle link utilization insert.
// PARAMETERS
// + node: Node* : Pointer to node.
// + msg : Message* : pointer to message
// RETURN void.
//--------------------------------------------------------------------------

void
HandleStatsDBLinkUtilizationPerNodeInsertion(Node* node, const std::string* str)
{

    StatsDb* db = node->partitionData->statsDb;
    vector<StatsDBLinkUtilizationParam> linkUtilizationParam;//defined in dbapi.h

    // Check if util  stats are on.
    if (db == NULL
            || !db->StatsDBLinkUtilTable->createLinkUtilizationTable)
    {
        return;
    }

    STATSDB_HandleLinkUtilizationPerNodeTableInsert(node, str);
    SynchPlusParallelBarrier(node->partitionData);

}



void STATSDB_HandleUrbanPropInsertion(
    const Node* node,
    StatsDBUrbanPropData* urbanData)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }

    // Now to add the information into the DataBase IF TABLE EXISTS
    // Check first if the table exists.
    if (!db->statsTable->createUrbanPropTable)
    {
        // Table does not exist.
        return;
    }

    std::vector<std::string> newValues;
    newValues.reserve(30);
    std::vector<std::string> columns;
    columns.reserve(30);

    columns.push_back("Timestamp");
    newValues.push_back(
        STATSDB_DoubleToString((double) getSimTime(node) / SECOND));
    columns.push_back("TxNodeId");
    newValues.push_back(STATSDB_IntToString(urbanData->txNodeId));
    columns.push_back("RxNodeId");
    newValues.push_back(STATSDB_IntToString(urbanData->rxNodeId));

    if (node->partitionData->terrainData->getCoordinateSystem() == LATLONALT)
    {
        columns.push_back("TxLat");
        columns.push_back("TxLon");
        columns.push_back("TxAlt");
        columns.push_back("RxLat");
        columns.push_back("RxLon");
        columns.push_back("RxAlt");
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c1));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c2));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c3));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c1));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c2));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c3));
    }
    else {
        columns.push_back("TxX");
        columns.push_back("TxY");
        columns.push_back("TxZ");
        columns.push_back("RxX");
        columns.push_back("RxY");
        columns.push_back("RxZ");
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c1));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c2));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->txPosition.common.c3));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c1));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c2));
        newValues.push_back(
            STATSDB_DoubleToString(urbanData->rxPosition.common.c3));
    }
    columns.push_back("Frequency");
    newValues.push_back(STATSDB_DoubleToString(urbanData->frequency));
    columns.push_back("Distance");
    newValues.push_back(STATSDB_DoubleToString(urbanData->distance));
    columns.push_back("FreeSpace");
    newValues.push_back(STATSDB_IntToString(urbanData->freeSpace));
    columns.push_back("Los");
    newValues.push_back(STATSDB_IntToString(urbanData->LoS));
    columns.push_back("TxInCanyon");
    newValues.push_back(STATSDB_IntToString(urbanData->txInCanyon));
    columns.push_back("RxInCanyon");
    newValues.push_back(STATSDB_IntToString(urbanData->rxInCanyon));
    columns.push_back("TxIndoors");
    newValues.push_back(STATSDB_IntToString(urbanData->txIndoors));
    columns.push_back("RxIndoors");
    newValues.push_back(STATSDB_IntToString(urbanData->rxIndoors));

    columns.push_back("ModelSelected");
    newValues.push_back(urbanData->modelSelected);

    columns.push_back("UrbanPathloss");
    newValues.push_back(STATSDB_DoubleToString(urbanData->pathloss));
    columns.push_back("FreeSpacePathloss");
    newValues.push_back(STATSDB_DoubleToString(urbanData->freeSpacePathloss));
    columns.push_back("TwoRayPathloss");
    newValues.push_back(STATSDB_DoubleToString(urbanData->twoRayPathloss));
    columns.push_back("ItmPathloss");
    newValues.push_back(STATSDB_DoubleToString(urbanData->itmPathloss));
    columns.push_back("NumWalls");
    newValues.push_back(STATSDB_IntToString(urbanData->numWalls));
    columns.push_back("NumFloors");
    newValues.push_back(STATSDB_IntToString(urbanData->numFloors));
    columns.push_back("OutsideDistance");
    newValues.push_back(STATSDB_DoubleToString(urbanData->outsideDistance));
    columns.push_back("InsideDistance1");
    newValues.push_back(STATSDB_DoubleToString(urbanData->insideDistance1));
    columns.push_back("InsideDistance2");
    newValues.push_back(STATSDB_DoubleToString(urbanData->insideDistance2));
    columns.push_back("OutsidePathloss");
    newValues.push_back(STATSDB_DoubleToString(urbanData->outsidePathloss));
    columns.push_back("InsidePathloss1");
    newValues.push_back(STATSDB_DoubleToString(urbanData->insidePathloss1));
    columns.push_back("InsidePathloss2");
    newValues.push_back(STATSDB_DoubleToString(urbanData->insidePathloss2));

    InsertValues(db, "URBAN_Propagation", columns, newValues);
}

void HandleStatsDBAppSendEventsInsertion(
    Node* node,
    Message* msg,
    Int32 msgSize,
    StatsDBAppEventParam* appParam)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }
    StatsDBAppEventParam* appParamInfo = NULL;


    appParamInfo = (StatsDBAppEventParam*) MESSAGE_AddInfo(
                       node,
                       msg,
                       sizeof(StatsDBAppEventParam),
                       INFO_TYPE_AppStatsDbContent);

    ERROR_Assert(appParam != NULL, "Unable to add an info field!");
    appParamInfo->m_NodeId = node->nodeId;
    appParamInfo->m_SessionInitiator = appParam->m_SessionInitiator;
    appParamInfo->m_ReceiverId = appParam->m_ReceiverId;
    appParamInfo->SetReceiverAddr(&appParam->m_TargetAddr);

    if (appParam->m_MsgSizeSpecified)
    {
        appParamInfo->m_MsgSize = appParam->m_MsgSize;
    }
    else
    {
        appParamInfo->m_MsgSize = msgSize;
    }

    if (appParam->m_SocketInterfaceMsgIdSpecified)
    {
        appParamInfo->SetSocketInterfaceMsgId(appParam->m_SocketInterfaceMsgId1,
            appParam->m_SocketInterfaceMsgId2);
    }

    // TESTING. Pre-empt the inof fields for TCP Only
    StatsDBNetworkEventParam* ipParamInfo = NULL;
    ipParamInfo = (StatsDBNetworkEventParam*) MESSAGE_AddInfo(
                      node,
                      msg,
                      sizeof(StatsDBNetworkEventParam),
                      INFO_TYPE_NetStatsDbContent);
    ipParamInfo = NULL;

    strcpy(appParamInfo->m_ApplicationType, appParam->m_ApplicationType);
    strcpy(appParamInfo->m_ApplicationName, appParam->m_ApplicationName);

    strcpy(appParamInfo->m_EventType, "AppSendToLower");
    appParamInfo->SetMsgSeqNum(msg->sequenceNumber);
    if (appParam->m_FragId == 0)
    {
       appParam->SetMsgSeqNum(msg->sequenceNumber);
    }
    if (appParam->m_FragIdSpecified)
    {
        appParamInfo->SetFragNum(appParam->m_FragId);
    }
    /*The first 2 if clauses should def be checked before the following one
    if (!node->partitionData->statsDb->statsAppEvents->recordFragment)
    {
       appParam->SetFragNum(0);
    }*/


    if (appParam->m_SessionIdSpecified)
    {
        appParamInfo->SetSessionId(appParam->m_SessionId);
    }
    if (appParam->m_PrioritySpecified)
    {
        appParamInfo->SetPriority(appParam->m_Priority);
    }
    if (appParam->m_MsgFailureTypeSpecified)
    {
        appParamInfo->SetMessageFailure(appParam->m_MsgFailureType);
    }
    if (appParam->m_DelaySpecified)
    {
        appParamInfo->SetDelay(appParam->m_Delay);
    }
    if (appParam->m_JitterSpecified)
    {
        appParamInfo->SetJitter(appParam->m_Jitter);
    }

    appParamInfo->SetPacketCreateTime(getSimTime(node));
    StatsDBAddMessageAppMsgId(node,msg,appParam);

    // Now to add the information into the DataBase IF TABLE EXISTS
    // Check first if the table exists.
    if (!db->statsEventsTable->createAppEventsTable)
    {
        // Table does not exist.
        return;
    }


    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    if (mapParamInfo == NULL)
    {
        ERROR_Assert(mapParamInfo, "SendEventsInsertion unable to find the mapparaminfo field");
    }
    sprintf(appParamInfo->m_MessageId, "%s", mapParamInfo->msgId);
    if (!node->partitionData->statsDb->statsAppEvents->recordFragment &&
        (appParam->m_IsFragmentation || appParam->m_FragId == 0))
    {
       Int32 i = 0;
       while (mapParamInfo->msgId[i] != '\0')
       {
           if (mapParamInfo->msgId[i] != 'A')
           {
              appParamInfo->m_MessageId[i] = mapParamInfo->msgId[i];
              i++;
           }
           else
           {
               break;
           }

       }
       appParamInfo->m_MessageId[i] = '\0';
       StatsDBAppEventParam tempAppParam = *appParamInfo;
       tempAppParam.m_MsgSize = appParam->m_TotalMsgSize;
       STATSDB_HandleAppEventsTableUpdate(node, NULL, tempAppParam);
    }
    else if (node->partitionData->statsDb->statsAppEvents->recordFragment)
    {
        STATSDB_HandleAppEventsTableUpdate(node, NULL, *appParamInfo);
    }
}

void HandleStatsDBAppReceiveEventsInsertion(
    Node* node,
    Message* msg,
    const char* eventType,
    clocktype delay,
    clocktype jitter,
    Int32 size,
    Int32 numReqMsgRcvd)
{
    StatsDb* db = node->partitionData->statsDb;

    // Update the application events table.
    // Check first if the table exists.
    if (!db || !db->statsEventsTable->createAppEventsTable)
    {
        // Table does not exist.
        return;
    }

    StatsDBAppEventParam appParam;
    StatsDBAppEventParam* appParamInfo = NULL;

    appParamInfo = (StatsDBAppEventParam*) MESSAGE_ReturnInfo(msg, INFO_TYPE_AppStatsDbContent);
    if (appParamInfo == NULL)
    {
        // Nothing to insert
        ERROR_ReportWarning("Application Info field not found\n");
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    if (mapParamInfo == NULL)
    {
        ERROR_Assert(mapParamInfo, "ReceiveEventsInsertion unable to find the mapparaminfo field");
    }


    appParam.m_NodeId = node->nodeId;
    appParam.m_SessionInitiator = appParamInfo->m_SessionInitiator;
    appParam.m_ReceiverId = appParamInfo->m_ReceiverId;
    appParam.SetReceiverAddr(&appParamInfo->m_TargetAddr);
    appParam.m_TargetAddrSpecified = appParamInfo->m_TargetAddrSpecified;
    if (!node->partitionData->statsDb->statsAppEvents->recordFragment)
    {
       Int32 i = 0;
       while (mapParamInfo->msgId[i] != '\0')
       {
           if (mapParamInfo->msgId[i] != 'A')
           {
              appParam.m_MessageId[i] = mapParamInfo->msgId[i];
              i++;
           }
           else
           {
               break;
           }

       }
       appParam.m_MessageId[i] = '\0';
       //strcat(appParam.m_MessageId,"F0");
    }

    else
    {
       strcpy(appParam.m_MessageId, mapParamInfo->msgId);
    }
    appParam.m_MsgSize = size;
    strcpy(appParam.m_EventType, eventType);

    strcpy(appParam.m_ApplicationType, appParamInfo->m_ApplicationType);
    strcpy(appParam.m_ApplicationName, appParamInfo->m_ApplicationName);

    appParam.SetMsgSeqNum(msg->sequenceNumber);
    if (appParamInfo->m_FragIdSpecified)
    {
        appParam.SetFragNum(appParamInfo->m_FragId);
    }

    if (appParamInfo->m_SessionIdSpecified)
    {
        appParam.SetSessionId(appParamInfo->m_SessionId);
    }
    if (appParamInfo->m_PrioritySpecified)
    {
        appParam.SetPriority(appParamInfo->m_Priority);
    }
    if (appParamInfo->m_MsgFailureTypeSpecified)
    {
        appParam.SetMessageFailure(appParamInfo->m_MsgFailureType);
    }
    appParam.SetDelay(delay);
    if (numReqMsgRcvd > 1)
    {
        appParam.SetJitter(jitter);
    }
    if (appParamInfo->m_SocketInterfaceMsgIdSpecified)
    {
        appParam.SetSocketInterfaceMsgId(appParamInfo->m_SocketInterfaceMsgId1,
            appParamInfo->m_SocketInterfaceMsgId2);
    }

    STATSDB_HandleAppEventsTableUpdate(node, NULL, appParam);

}

void HandleStatsDBAppDropEventsInsertion(
    Node* node,
    Message* msg,
    const char* eventType,
    clocktype delay,
    Int32 size,
    char* failureType)
{
    StatsDb* db = node->partitionData->statsDb;

    // Update the application events table.
    // Check first if the table exists.
    if (!db || !db->statsEventsTable->createAppEventsTable)
    {
        // Table does not exist.
        return;
    }

    StatsDBAppEventParam appParam;
    StatsDBAppEventParam* appParamInfo = NULL;

     StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    if (mapParamInfo == NULL)
    {
        ERROR_Assert(mapParamInfo, "DropEventsInsertion unable to find the mapparaminfo field");
    }

    appParamInfo = (StatsDBAppEventParam*) MESSAGE_ReturnInfo(msg, INFO_TYPE_AppStatsDbContent);
    if (appParamInfo == NULL)
    {
#ifdef EXATA
    appParam.m_NodeId = node->nodeId;
    appParam.m_SessionInitiator = msg->originatingNodeId;
    appParam.m_ReceiverId = -1;
    strcpy(appParam.m_MessageId, mapParamInfo->msgId);
    appParam.m_MsgSize = size;
    strcpy(appParam.m_EventType, eventType);

    strcpy(appParam.m_ApplicationType, "");
    strcpy(appParam.m_ApplicationName, "");

    appParam.SetMsgSeqNum(msg->sequenceNumber);
    appParam.SetMessageFailure(failureType);

    STATSDB_HandleAppEventsTableUpdate(node, NULL, appParam);
    return;

#else
        // Nothing to insert
        ERROR_ReportWarning("Application Info field not found\n");
        return;
#endif
    }


    appParam.m_NodeId = node->nodeId;
    appParam.m_SessionInitiator = appParamInfo->m_SessionInitiator;
    appParam.m_ReceiverId = appParamInfo->m_ReceiverId;
    appParam.SetReceiverAddr(&appParamInfo->m_TargetAddr);
    if (!node->partitionData->statsDb->statsAppEvents->recordFragment)
    {
       Int32 i = 0;
       while (mapParamInfo->msgId[i] != '\0')
       {
           if (mapParamInfo->msgId[i] != 'A')
           {
              appParam.m_MessageId[i] = mapParamInfo->msgId[i];
              i++;
           }
           else
           {
               break;
           }

       }
       appParam.m_MessageId[i] = '\0';
       //strcat(appParam.m_MessageId,"F0");
    }

    else
    {
       strcpy(appParam.m_MessageId, mapParamInfo->msgId);
    }
    appParam.m_MsgSize = size;
    strcpy(appParam.m_EventType, eventType);

    strcpy(appParam.m_ApplicationType, appParamInfo->m_ApplicationType);
    strcpy(appParam.m_ApplicationName, appParamInfo->m_ApplicationName);

    appParam.SetMsgSeqNum(msg->sequenceNumber);
    if (appParamInfo->m_FragIdSpecified)
    {
        appParam.SetFragNum(appParamInfo->m_FragId);
    }
    if (appParamInfo->m_SessionIdSpecified)
    {
        appParam.SetSessionId(appParamInfo->m_SessionId);
    }
    if (appParamInfo->m_PrioritySpecified)
    {
        appParam.SetPriority(appParamInfo->m_Priority);
    }
    //if (appParamInfo->m_MsgFailureTypeSpecified)
    //{
    appParam.SetMessageFailure(failureType);
    //}
    if (delay >=0)
    {
        appParam.SetDelay(delay);
    }

    STATSDB_HandleAppEventsTableUpdate(node, NULL, appParam);

}
#ifdef ADDON_NGCNMS
void HandleStatsDBNetworkEventsInsertionForOtherTables(
    Node* node,
    Message* msg,
    Int32 msgSize,
    StatsDBNetworkEventParam* ipParam)
{
    StatsDb* db = node->partitionData->statsDb;

    Int32 transportSeqId = 0;
    BOOL isTransId = FALSE;
    BOOL isAppId = FALSE;
    Int32 appFragId = 0;
    StatsDBNetworkEventParam* ipParamInfo = NULL;
    StatsDBTransportEventParam* transParamInfo = NULL;
    StatsDBAppEventParam* appParamInfo = NULL;

    // fix 09/01/08
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    sprintf(ipParam->m_MessageId, mapParamInfo->net_msgId);

    // First Extract the existing info field.
    ipParamInfo = (StatsDBNetworkEventParam*)
        MESSAGE_ReturnInfo(msg, INFO_TYPE_NetStatsDbContent);
    if (ipParamInfo == NULL)
    {
        // Info does not exist. Insert one.
        ipParamInfo = (StatsDBNetworkEventParam*) MESSAGE_AddInfo(
                          node,
                          msg,
                          sizeof(StatsDBNetworkEventParam),
                          INFO_TYPE_NetStatsDbContent);
    }
    ERROR_Assert(ipParamInfo != NULL, "Unable to add an info field!");
    //ipParamInfo->m_NodeId = node->nodeId;
    //ipParamInfo->m_MsgSize = msgSize;
    //ipParamInfo->m_SenderAddr = ipParam->m_SenderAddr;
    //ipParamInfo->m_ReceiverAddr = ipParam->m_ReceiverAddr;

    // Special case for EPLRS Waveform.
    if (strcmp(ipParamInfo->m_PktType, "Data") == 0 &&
        strcmp(ipParam->m_PktType, "Control") == 0 &&
        strcmp(ipParam->m_ProtocolType, "IPPROTO_IPIP_BOEING_ODR") == 0)
    {
        ipParam->SetHopCount(0.0);
    }
    //strcpy(ipParamInfo->m_EventType, ipParam->m_EventType);
    //strcpy(ipParamInfo->m_MessageId, ipParam->m_MessageId);

    if (ipParamInfo->m_HopCountSpecified)
    {
        ipParamInfo->m_HopCount += ipParam->m_HopCount;
    }
    else
    {
        ipParamInfo->SetHopCount(ipParam->m_HopCount);
    }
    //if (isAppId)
    //{
    //    ipParamInfo->SetAppFragId(appFragId);
    //}

    //if (isTransId)
    //{
    //    ipParamInfo->SetTransportSeqNumber(transportSeqId);
    //}

    //if (ipParam->m_NetworkFragNumSpecified)
    //{
    //    ipParamInfo->SetFragNum(ipParam->m_NetworkFragNumber);
    //}

    //ipParamInfo->SetMsgSeqNum(msg->sequenceNumber);
    //if (ipParam->m_FailureTypeSpecified)
    //{
    //    ipParamInfo->SetFailure(ipParam->m_FailureType);
    //}
    //if (ipParam->m_HdrSizeSpecified)
    //{
    //    ipParamInfo->SetHdrSize(ipParam->m_HeaderSize);
    //}
    //if (ipParam->m_InterfaceIndexSpecified)
    //{
    //    ipParamInfo->SetInterfaceIndex(ipParam->m_InterfaceIndex);
    //}
    //if (ipParam->m_PrioritySpecified)
    //{
    //    ipParamInfo->SetPriority(ipParam->m_Priority);
    //}
    if (ipParam->m_ProtocolTypeSpecified)
    {
        ipParamInfo->SetProtocolType(ipParam->m_ProtocolType);
    }
    //if (ipParam->m_MacProtocolSpecified)
    //{
    //    ipParamInfo->SetMacProtocol(ipParam->m_MacProtocol);
    //}
    if (ipParam->m_PktTypeSpecified)
    {
        ipParamInfo->SetPktType(ipParam->m_PktType);
    }
    else if (msg->originatingProtocol == TRACE_TCP ||
        msg->originatingProtocol == TRACE_UDP)
    {
        ipParamInfo->SetPktType("Data");
    }
    else
    {
        ipParamInfo->SetPktType("Control");
    }

}
#endif

void HandleStatsDBNetworkEventsInsertion(
    Node* node,
    Message* msg,
    Int32 interfaceIndex,
    IpHeaderType* ipHeader,
    const std::string& eventType,
    const std::string& failure,
    NodeAddress srcAddr,
    NodeAddress dstAddr,
    TosType priority,
    unsigned char protocol,
    Int32 ipHdrSize)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return ;
    }
#ifdef ADDON_NGCNMS
    if (!db->statsEventsTable->createNetworkEventsTable)
    {
        return HandleStatsDBNetworkEventsInsertionForOtherTables(
            node, msg, msgSize, ipParam);
    }
#endif

    // First Extract the existing info field.
    StatsDBNetworkEventParam* ipParamInfo = (StatsDBNetworkEventParam*)
        MESSAGE_ReturnInfo(msg, INFO_TYPE_NetStatsDbContent);
    if (ipParamInfo == NULL)
    {
        // Info does not exist. Insert one.
        ipParamInfo = (StatsDBNetworkEventParam*) MESSAGE_AddInfo(
                          node,
                          msg,
                          sizeof(StatsDBNetworkEventParam),
                          INFO_TYPE_NetStatsDbContent);
    }
    ERROR_Assert(ipParamInfo != NULL, "Unable to add an info field!");
    ipParamInfo->m_NodeId = node->nodeId;
    ipParamInfo->m_MsgSize = MESSAGE_ReturnPacketSize(msg); //msgSize;

    char packetType = -1;

    if (eventType.compare("NetworkSendToUpper") == 0 ||
        eventType.compare("NetworkReceiveFromUpper") == 0)
    {
        ipParamInfo->m_SenderAddr = srcAddr;
        ipParamInfo->m_ReceiverAddr = dstAddr;
        ipParamInfo->SetPriority(priority);
        ipParamInfo->SetHdrSize(ipHdrSize);
        if (protocol == IPPROTO_UDP ||
            protocol == IPPROTO_TCP
            )
        {

              packetType = StatsDBNetworkEventParam::DATA;
        }
        else
        {
            packetType = StatsDBNetworkEventParam::CONTROL;
        }

    }
    else
    {
        ipParamInfo->m_SenderAddr = ipHeader->ip_src;
        ipParamInfo->m_ReceiverAddr = ipHeader->ip_dst;
        ipParamInfo->SetPriority(IpHeaderGetTOS(ipHeader->ip_v_hl_tos_len));
        ipParamInfo->SetHdrSize(IpHeaderSize(ipHeader));
        if (ipHeader->ip_p == IPPROTO_UDP ||
            ipHeader->ip_p == IPPROTO_TCP)
        {

              packetType = StatsDBNetworkEventParam::DATA;
        }
        else
        {
            packetType = StatsDBNetworkEventParam::CONTROL;
        }
        protocol = ipHeader->ip_p;

    }

    // Hop Count.
    Int32 hopCount = 0;
    if (eventType  == "NetworkReceiveFromLower"  ||
        eventType == "NetworkMalsrForwardPacket")
    {
        hopCount = 1;
    }

    ipParamInfo->SetProtocolType(protocol);

    ipParamInfo->SetInterfaceIndex(interfaceIndex);

    // Special case for EPLRS Waveform.
#ifdef ADDON_BOEINGFCS
   if (ipParamInfo->m_PktTypeSpecified &&
       ipParamInfo->m_PktType == StatsDBNetworkEventParam::DATA &&
        packetType == StatsDBNetworkEventParam::CONTROL &&
        ipParamInfo->m_ProtocolTypeSpecified /*&&
       ipParamInfo->m_ProtocolType == IPPROTO_INC*/)
    {
        hopCount = 0;
    }
#endif

    if (ipParamInfo->m_HopCountSpecified)
    {
        ipParamInfo->m_HopCount += hopCount;
    }
    else
    {
        ipParamInfo->SetHopCount(hopCount);
    }

    ipParamInfo->SetMsgSeqNum(msg->sequenceNumber);

    if (packetType != -1)
    {
        ipParamInfo->SetPktType(packetType);
    }
    else if (msg->originatingProtocol == TRACE_TCP ||
        msg->originatingProtocol == TRACE_UDP)
    {
        ipParamInfo->SetPktType(StatsDBNetworkEventParam::DATA);
    }
    else
    {
        ipParamInfo->SetPktType(StatsDBNetworkEventParam::CONTROL);
    }

    if (ipParamInfo->m_PktType == StatsDBNetworkEventParam::CONTROL &&
            !db->statsNetEvents->networkStatsDBControl)
    {
        // Do not insert
        return;
    }
    if (!db->statsNetEvents->networkStatsDBIncoming &&
        (eventType  ==  "NetworkReceiveFromLower" ||
        eventType == "NetworkRecieveFromUpper") &&
        ipParamInfo->m_PktType != StatsDBNetworkEventParam::CONTROL)
    {
        return;
    }
    if (!db->statsNetEvents->networkStatsDBOutgoing &&
        (eventType == "NetworkSendToLower" ||
        eventType == "NetworkSendToUpper") &&
          ipParamInfo->m_PktType != StatsDBNetworkEventParam::CONTROL)
    {
        return;
    }
    // Insert on to the table
    // Check first if the table exists.
    if (!db->statsEventsTable->createNetworkEventsTable)
    {
        // Table does not exist.
        return;
    }
    BOOL failureSpecified = FALSE;
    if (failure.size() > 0) // this check should be cancelled ?
    {
        failureSpecified = TRUE ;
    }
    STATSDB_HandleNetworkEventsTableUpdate(node, NULL, *ipParamInfo, msg,
        failure.c_str(), failureSpecified, eventType.c_str());
}

// Connectivity Table Insertion

void STATSDB_HandleAppConnCreation(
    Node* node,
    NodeAddress clientAddr,
    NodeAddress serverAddr,
    Int32 sessionId)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable->createAppConnTable)
    {
        // table does not exist
        return;
    }
    Address addr_client;
    addr_client.networkType = NETWORK_IPV4;
    addr_client.interfaceAddr.ipv4 = clientAddr;

    Address addr_server;
    addr_server.networkType = NETWORK_IPV4;
    addr_server.interfaceAddr.ipv4 = serverAddr;

    STATSDB_HandleAppConnCreation(node, addr_client, addr_server, sessionId);

}
void STATSDB_HandleAppConnCreation(
    Node* node,
    const Address &clientAddr,
    const Address &serverAddr,
    Int32 sessionId)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable->createAppConnTable)
    {
        // table does not exist
        return;
    }
    StatsDBConnTable::V_AppConnParam* v_AppConnParam
        = db->statsConnTable->v_AppConnParam;

    ERROR_Assert(v_AppConnParam, "Error in STATSDB_HandleAppConnCreation.");
    StatsDBConnTable::V_AppConnParamIter iter = v_AppConnParam->find(
        std::pair<Address, Int32>(clientAddr, sessionId));

    ERROR_Assert(iter == v_AppConnParam->end(),
        "Error in STATSDB_HandleAppConnCreation.");

    char addr[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((Address *)&clientAddr, addr);
    std::string srcString = addr;

    IO_ConvertIpAddressToString((Address *)&serverAddr, addr);
    std::string dstString = addr;

    (*v_AppConnParam)[std::pair<Address, Int32>(clientAddr, sessionId)] =
        new StatsDBAppConnParam(srcString, dstString, sessionId);

    //printf("Stats DB AppConn Table insert: src %s dst %s sessionId %d \n",
    //    srcString.c_str(), dstString.c_str(), sessionId);

}

void STATSDB_HandleTransConnCreation(
    Node* node,
    const Address &clientAddr,
    Int32 clientPort,
    const Address &serverAddr,
    Int32 serverPort)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable->createTransConnTable)
    {
        // table does not exist
        return;
    }

    StatsDBConnTable::V_TransConnParam* v_TransConnParam
        = db->statsConnTable->v_TransConnParam;

    ERROR_Assert(v_TransConnParam,
        "Error in STATSDB_HandleTransConnCreation.");

    pair<Address, Int32> key = pair<Address, Int32>(clientAddr, clientPort);
    pair<StatsDBConnTable::V_TransConnParamIter,
        StatsDBConnTable::V_TransConnParamIter> pair_iter =
        v_TransConnParam->equal_range(key);
    StatsDBTransConnParam *pTransData = NULL ;

    char addr[MAX_STRING_LENGTH];
    IO_ConvertIpAddressToString((Address *)&clientAddr, addr);
    std::string srcString = addr;

    IO_ConvertIpAddressToString((Address *)&serverAddr, addr);
    std::string dstString = addr;

    //if (pair_iter.first == db->statsConnTable->v_TransConnParam->end())
    if (pair_iter.first == pair_iter.second)
    {
        pTransData = new StatsDBTransConnParam(srcString, dstString,
            clientPort, serverPort);
        db->statsConnTable->v_TransConnParam->insert(
            pair<pair<Address, Int32>,
            StatsDBTransConnParam*>(key, pTransData));
    }
    else
    {
        StatsDBConnTable::V_TransConnParamIter iter = pair_iter.first;
        for (; iter != pair_iter.second; ++iter)
        {
            if (iter->second->m_DstAddress == dstString &&
                iter->second->m_DstPort == serverPort)
            {
               ERROR_Assert(FALSE,
                    "Error in STATSDB_HandleTransConnCreation.");
            }
        }
        if (iter == pair_iter.second)
        {
            pTransData = new StatsDBTransConnParam(srcString, dstString,
                clientPort, serverPort);
            db->statsConnTable->v_TransConnParam->insert(
                pair<pair<Address, Int32>,
                StatsDBTransConnParam*>(key, pTransData));
        }
    }

    //printf("Stats DB TransConn Table insert: src %s dst %s src port %d dst port %d\n",
    //    srcString.c_str(), dstString.c_str(), clientPort, serverPort);

}

void STATSDB_HandleMulticastConnCreation(Node *node,
                       const StatsDBConnTable::MulticastConnectivity & stats)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable ||
        !db->statsConnTable->createMulticastConnTable)
    {
        // table does not exist
        return;
    }

    // Test for uniqueness
    BOOL uniqueItem = TRUE;
    std::vector<StatsDBConnTable::MulticastConnectivity>::iterator multicastIterator;
    for (multicastIterator = db->statsConnTable->MulticastConnTable.begin();
        multicastIterator != db->statsConnTable->MulticastConnTable.end();
        multicastIterator++)
    {
        if ((multicastIterator->nodeId == node->nodeId) &&
           (multicastIterator->destAddr == stats.destAddr) &&
           (!strcmp(multicastIterator->rootNodeType,stats.rootNodeType)) &&
           (multicastIterator->rootNodeId == stats.rootNodeId) &&
           (multicastIterator->outgoingInterface ==
                                                 stats.outgoingInterface)
          )
          {
              uniqueItem = FALSE;
              break;
          }
    }

    // only insert unique items
    if (uniqueItem == TRUE)
    {
        StatsDBConnTable::MulticastConnectivity multicastConnParam;
        multicastConnParam.nodeId = stats.nodeId;
        multicastConnParam.destAddr = stats.destAddr;
        strcpy(multicastConnParam.rootNodeType,stats.rootNodeType);
        multicastConnParam.rootNodeId = stats.rootNodeId;
        multicastConnParam.outgoingInterface = stats.outgoingInterface;
        multicastConnParam.upstreamNeighborId = stats.upstreamNeighborId;
        multicastConnParam.upstreamInterface = stats.upstreamInterface;

        // insert this into queue of waiting table inserts
        db->statsConnTable->MulticastConnTable.push_back(multicastConnParam);
    }
}


void STATSDB_HandleMulticastConnDeletion(Node *node,
                       const StatsDBConnTable::MulticastConnectivity & stats)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable ||
        !db->statsConnTable->createMulticastConnTable)
    {
        // table does not exist
        return;
    }

    // delete item if exists
    std::vector<StatsDBConnTable::MulticastConnectivity>::iterator multicastIterator;
    for (multicastIterator = db->statsConnTable->MulticastConnTable.begin();
        multicastIterator != db->statsConnTable->MulticastConnTable.end();
        multicastIterator++)
    {
        if ((multicastIterator->nodeId == stats.nodeId) &&
              (multicastIterator->destAddr == stats.destAddr) &&
            (!strcmp(multicastIterator->rootNodeType,stats.rootNodeType)) &&
            (multicastIterator->rootNodeId == stats.rootNodeId) &&
            (multicastIterator->outgoingInterface == stats.outgoingInterface)
          )
          {
              db->statsConnTable->MulticastConnTable.erase(multicastIterator);
              break;
          }
    }
}

void STATSDB_HandleMulticastConnUpdateUpstreamInfo(Node *node,
                       const StatsDBConnTable::MulticastConnectivity & stats,
                       BOOL createIfNotExist)
{
    BOOL wasFound = FALSE;
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable ||
        !db->statsConnTable->createMulticastConnTable)
    {
        // table does not exist
        return;
    }

    // update upstream Info for the item if exists
    std::vector<StatsDBConnTable::MulticastConnectivity>::iterator multicastIterator;
    for (multicastIterator = db->statsConnTable->MulticastConnTable.begin();
        multicastIterator != db->statsConnTable->MulticastConnTable.end();
        multicastIterator++)
    {
        if ((multicastIterator->nodeId == stats.nodeId) &&
              (multicastIterator->destAddr == stats.destAddr) &&
            (!strcmp(multicastIterator->rootNodeType,stats.rootNodeType)) &&
            (multicastIterator->rootNodeId == stats.rootNodeId) &&
            (multicastIterator->outgoingInterface == stats.outgoingInterface)
          )
          {
              multicastIterator->upstreamNeighborId =
                                                    stats.upstreamNeighborId;
              multicastIterator->upstreamInterface = stats.upstreamInterface;
              wasFound = TRUE;
              break;
          }
    }

    if (!wasFound && createIfNotExist)
    {
        STATSDB_HandleMulticastConnCreation(node, stats);
    }
}


void StatsDBHandleMulticastConnInsertion(Node* node, Message* msg)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (!db || !db->statsConnTable ||
        !db->statsConnTable->createMulticastConnTable)
    {
        // table does not exist
        return;
    }

    std::vector<StatsDBConnTable::MulticastConnectivity>::iterator multicastIterator;
    for (multicastIterator = db->statsConnTable->MulticastConnTable.begin();
        multicastIterator != db->statsConnTable->MulticastConnTable.end();
        multicastIterator++)
    {
        // Insert into the Connectivity table.
        STATSDB_HandleMulticastConnTableInsert(node, *multicastIterator);
    }
}

void HandleStatsDBAppConnInsertion(Node* node)
{

    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (db == NULL || !db->statsConnTable->createAppConnTable)
    {
        // table does not exist
        return;
    }
    StatsDBConnTable::V_AppConnParam* v_AppConnParam
        = db->statsConnTable->v_AppConnParam;

    ERROR_Assert(v_AppConnParam, "Error in HandleStatsDBAppConnInsertion.");

    StatsDBConnTable::V_AppConnParamIter iter = v_AppConnParam->begin();

    for (; iter != v_AppConnParam->end(); ++iter)
    {
        // Insert into the Connectivity table.
        STATSDB_HandleAppConnTableInsert(node, iter->second);
    }
}

void HandleStatsDBTransConnInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check first if the table exists
    if (db == NULL || !db->statsConnTable->createTransConnTable)
    {
        // table does not exist
        return;
    }
    StatsDBConnTable::V_TransConnParam* v_TransConnParam
        = db->statsConnTable->v_TransConnParam;

    ERROR_Assert(v_TransConnParam, "Error in HandleStatsDBTransConnInsertion.");

    StatsDBConnTable::V_TransConnParamIter iter = v_TransConnParam->begin();

    for (; iter != v_TransConnParam->end(); ++iter)
    {
        // Insert into the Connectivity table.
        STATSDB_HandleTransConnTableInsert(node, iter->second);
    }
}

void HandleStatsDBPhyConnInsertion(PartitionData* partitionData)
{
    StatsDb* db = partitionData->statsDb;
    if (!db || !db->statsConnTable->createPhyConnTable)
    {
        return;
    }
    PHY_CONN_CollectConnectSample(partitionData);

}


void HandleStatsDBNetworkConnInsertion(Node* firstNode)
{
    Node* node = firstNode;
    Int32 i;
    StatsDb* db = node->partitionData->statsDb;
    while (node)
    {
        i = 0;
        // Check first if the table exists
        if (!db || !db->statsConnTable->createNetworkConnTable)
        {
            // table does not exist
            return;
        }

        StatsDBNetworkConnParam ipParam;
        StatsDBNetworkConnContent* netConnContent = NULL;
        char address[20];
        NetworkDataIp* ip = (NetworkDataIp *) node->networkData.networkVar;
        NetworkForwardingTable* rt = &ip->forwardTable;
        netConnContent = db->statsNetConn;

        ERROR_Assert(ip != NULL,
            "Unable to extract the ip information\n");
        ERROR_Assert(rt != NULL,
            "Unable to get Forwarding Tabel content\n");
        ERROR_Assert(netConnContent != NULL,
            "Unable to get STATS Table information\n");

        ipParam.m_NodeId = node->nodeId;

        // Check for parallel code.
        if (node->partitionId != node->partitionData->partitionId)
        {
            // node data will not be available
            return;
        }

        for (i = 0; i < rt->size; i++)
        {
            // Set Destination Address
            IO_ConvertIpAddressToString(rt->row[i].destAddress, address);
            ipParam.m_DstAddress = address;

            // Set the hop Count
            ipParam.m_Cost = rt->row[i].cost;

            // Set the Optional columns.
            if (netConnContent->isDstMaskAddr)
            {
                IO_ConvertIpAddressToString(rt->row[i].destAddressMask,
                                            address);
                ipParam.SetDstnetworkMask(address);
            }
            if (netConnContent->isOutgoingInterfaceIndex)
            {
                ipParam.SetOutgoingInterface(rt->row[i].interfaceIndex);
            }
            if (netConnContent->isNextHopAddr)
            {
                IO_ConvertIpAddressToString(rt->row[i].nextHopAddress,
                                            address);
                ipParam.SetNextHopAddr(address);
            }
            if (netConnContent->isRoutingProtocol)
            {
                std::string temp;
                NetworkIpConvertProtocolTypeToString(
                    rt->row[i].protocolType,
                    &temp);
                ipParam.SetRoutingProtocol(temp);
            }
            if (netConnContent->isAdminDistance)
            {
                ipParam.SetAdminDistance(rt->row[i].adminDistance);
            }
            // Insert into the Connectivity table.
            STATSDB_HandleNetworkConnTableUpdate(node, ipParam);
        }
        node = node->nextNodeData;
    }


}

void HandleStatsDBMacConnInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    Int32 i = 0;
    // Check first if the table exists
    if (!db || !db->statsConnTable->createMacConnTable)
    {
        // table does not exist
        return;
    }
    StatsDBConnTable::MacConnTable_Iterator iter =
        db->statsConnTable->MacConnTable.begin();

    for (; iter != db->statsConnTable->MacConnTable.end(); ++iter)
    {
        if (iter->second->isConnected)
        {
            StatsDBMacConnParam macParam;

            macParam.m_SenderId = iter->first.first ;
            macParam.m_ReceiverId = iter->first.second ;
            macParam.m_InterfaceIndex = iter->second->interfaceIndex;
            macParam.m_ChannelIndex = iter->second->channelIndex;

            strcpy(macParam.channelIndex_str, iter->second->channelIndex_str.c_str());

            //printf("Mac Conn senderId %d [%d] to recvId %d on channel %d \n",
            //    macParam.m_SenderId, macParam.m_InterfaceIndex,
            //    macParam.m_ReceiverId, macParam.m_ChannelIndex);

            STATSDB_HandleMacConnTableUpdate(node, macParam);
        }
    }


    iter =
        db->statsConnTable->MacConnTable.begin();

    for (; iter != db->statsConnTable->MacConnTable.end(); ++iter)
    {
        delete(iter->second);
    }
    db->statsConnTable->MacConnTable.clear();
}

// Aggregate Table Insertion
void HandleStatsDBAppAggregateInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Calculate only if the app aggregate is enabled.
    if (!db || !db->statsAggregateTable->createAppAggregateTable)
    {
        return;
    }

    STATSDB_HandleAppAggregateTableInsert(node);

}

// Aggregate Table Insertion
void HandleStatsDBTransAggregateInsertion(Node *node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
       /* ERROR_ReportWarning("Unable to extract DB infotmation for Transport Aggregate\n");*/
        return ;
    }

    // Check if Transport Aggregate stats are on.
    if (!db->statsAggregateTable->createTransAggregateTable)
    {
        return;
    }

    if (node->partitionData->partitionId == 0)
    {
        STATSDB_HandleTransAggregateTableInsert(node);
    }
    else
    {
        node->partitionData->stats->Aggregate(node->partitionData);
    }

}

// Network Aggregate Stats.
void HandleStatsDBNetworkAggregateInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
       /* ERROR_ReportWarning("Unable to extract DB infotmation for Network Aggregate\n");*/
        return ;
    }

    // Check if Network Aggregate stats are on.
    if (!db->statsAggregateTable->createNetworkAggregateTable)
    {
        return;
    }

    if (node->partitionData->partitionId == 0)
    {
        STATSDB_HandleNetworkAggregateTableInsert(node);
    }
    else
    {
        node->partitionData->stats->Aggregate(node->partitionData);
    }
}
// Queue Aggregate Stats.
void HandleStatsDBQueueAggregateInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
       /* ERROR_ReportWarning("Unable to extract DB infotmation for Network Aggregate\n");*/
        return ;
    }

    // Check if Queue Aggregate stats are on.
    if (!db->statsAggregateTable->createQueueAggregateTable)
    {
        return;
    }

    if (node->partitionData->partitionId == 0)
    {
        STATSDB_HandleQueueAggregateTableInsert(node);
    }
    else
    {
        node->partitionData->stats->Aggregate(node->partitionData);
    }
}
// Queue Summary Stats.
void HandleStatsDBQueueSummaryInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsSummaryTable->createQueueSummaryTable)
    {
        return;
    }
    STATSDB_HandleQueueSummaryTableInsert(node);
}

void HandleStatsDBMacAggregateInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Check if Mac Aggregate stats are on.
    if (db == NULL || !db->statsAggregateTable->createMacAggregateTable)
    {
        return;
    }

    if (node->partitionData->partitionId == 0)
    {
        STATSDB_HandleMacAggregateTableInsert(node);
    }
    else
    {
        node->partitionData->stats->Aggregate(node->partitionData);
    }
}
//--------------------------------------------------------------------//
// NAME     : HandleStatsDBPhyAggregateInsertion
// PURPOSE  : Insert a new row into the PHY_Aggregate table. During
//            parallel operation, this requires combining the statistics
//            of all partitions.
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void HandleStatsDBPhyAggregateInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
       /* ERROR_ReportWarning("Unable to extract DB infotmation for Physical Aggregate\n");*/
        return ;
    }

    // Check if Physical Aggregate stats are on.
    if (!db->statsAggregateTable->createPhyAggregateTable)
    {
        return;
    }

    if (node->partitionData->partitionId == 0)
    {
        STATSDB_HandlePhyAggregateTableInsert(node);
    }
    else
    {
        node->partitionData->stats->Aggregate(node->partitionData);
    }
}


void StatsDBSendAggregateTimerMessage(PartitionData* partition)
{
    // In this function we send a timer message to the first node of
    // each partition. Once the message is received we calculate the
    // aggregate stats for the simulation.
    // Create the Timer message and send out.

    Message* aggregateTimer;
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (db->statsAggregateTable->createAppAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            STATSDB_APP_AGGREGATE,
            MSG_STATSDB_APP_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }

    if (db->statsAggregateTable->createTransAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_TRANSPORT_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }

    if (db->statsAggregateTable->createNetworkAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_NETWORK_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }

    if (db->statsAggregateTable->createQueueAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_QUEUE_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }

    if (db->statsAggregateTable->createMacAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
                STATSDB_LAYER,
                MAC_STATSDB_AGGREGATE,
                MSG_STATS_MAC_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }

    if (db->statsAggregateTable->createPhyAggregateTable)
    {
        aggregateTimer = MESSAGE_Alloc(partition,
                STATSDB_LAYER,
                PHY_STATSDB_AGGREGATE,
                MSG_STATS_PHY_InsertAggregate);

        aggregateTimer->eventTime = partition->theCurrentTime +
            db->statsAggregateTable->aggregateInterval;

        PARTITION_SchedulePartitionEvent(partition,
            aggregateTimer,
            aggregateTimer->eventTime,
            false);
    }
}

//--------------------------------------------------------------------//
// NAME     : StatsDBSendStatusTimerMessage
// PURPOSE  : Set a timer of duration specified by the configuation
//            parameter STATS-DB-STATUS-INTERVAL. When the timer
//            expires, updates will be made to the status tables
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
void StatsDBSendStatusTimerMessage(PartitionData* partition)
{
    // In this function we send a timer message for each node.
    // When the message is received, the node's status is
    // recorded.

    Message* statusTimer;
    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (db->statsStatusTable->createNodeStatusTable)
    {
        statusTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_NODE_InsertStatus);
        statusTimer->eventTime = partition->theCurrentTime +
            db->statsStatusTable->statusInterval;
        PARTITION_SchedulePartitionEvent(partition,
            statusTimer,
            statusTimer->eventTime,
            false);
    }
    if (db->statsStatusTable->createInterfaceStatusTable)
    {
        statusTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_INTERFACE_InsertStatus);
        statusTimer->eventTime = partition->theCurrentTime +
            db->statsStatusTable->statusInterval;
        PARTITION_SchedulePartitionEvent(partition,
            statusTimer,
            statusTimer->eventTime,
            false);
    }

    if (db->statsStatusTable->createQueueStatusTable)
    {
        statusTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_QUEUE_InsertStatus);
        statusTimer->eventTime = partition->theCurrentTime +
            db->statsStatusTable->statusInterval;
        PARTITION_SchedulePartitionEvent(partition,
            statusTimer,
            statusTimer->eventTime,
            false);
    }

    if (db->statsStatusTable->createMulticastStatusTable)
    {
        statusTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_MULTICAST_InsertStatus);
        statusTimer->eventTime = partition->theCurrentTime +
            db->statsStatusTable->statusInterval;
        PARTITION_SchedulePartitionEvent(partition,
            statusTimer,
            statusTimer->eventTime,
            false);
    }
}

void HandleStatsDBAppSummaryInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Calculate only if the app Summary is enabled.
    if (db == NULL || !db->statsSummaryTable->createAppSummaryTable)
    {
        return;
    }

    STATSDB_HandleAppSummaryTableInsert(node);
}

void HandleStatsDBMulticastAppSummaryInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    // Calculate only if the multicast app Summary is enabled.
    if (db == NULL || !db->statsSummaryTable->createMulticastAppSummaryTable)
    {
        return;
    }

    STATSDB_HandleMulticastAppSummaryTableInsert(node);
}

static
void HandleNetSumAggrStatsUpdate(
    Node* node,
    Message* msg,
    OneHopNeighborStats* stats,
    BOOL outgoing,
    BOOL isForward,
    BOOL isDataPacket)
{

    IpHeaderType* ipHeader = (IpHeaderType *) msg->packet;
    if (ipHeader == NULL)
    {
        ERROR_ReportWarning("Error in StatsDb,"
                    "Network IP Header Unavailable\n");
    }
    // Update the stats.
    if (outgoing)
    {
        if (!isForward)
        {
            if (stats->uControlPacketsSent == 0 &&
                stats->uDataPacketsSent == 0)
            {
                // First Packet
                stats->firstPacketSendTime = getSimTime(node);
            }
            stats->lastPacketSendTime = getSimTime(node);
        }

        if (isDataPacket)
        {
            if (isForward)
            {
                // Data packet forwarded
                stats->uDataPacketsForward++;
                stats->uDataBytesForward += MESSAGE_ReturnPacketSize(msg);
            }
            else
            {
                // Data packet being sent
                stats->uDataPacketsSent++;
                stats->uDataBytesSent += MESSAGE_ReturnPacketSize(msg);
            }
        }
        else
        {
            if (isForward)
            {
                // Control packet forwarded
                stats->uControlPacketsForward++;
                stats->uControlBytesForward +=
                                            MESSAGE_ReturnPacketSize(msg);
            }
            else
            {
                // Control packet sent
                stats->uControlPacketsSent++;
                stats->uControlBytesSent += MESSAGE_ReturnPacketSize(msg);
            }
        }
    }
    else
    {
        // Packets Received, calculate delay.

        clocktype* timing = (clocktype*)
            MESSAGE_ReturnInfo(msg, INFO_TYPE_IPPacketSentTime);
        if (timing == NULL)
        {
            ERROR_ReportWarning("Error of INFO_TYPE_IPPacketSentTime \n");
            return;
        }

        clocktype delay = getSimTime(node) - *timing;
        clocktype jitter = 0;
        clocktype lastDelayTime = CLOCKTYPE_MAX;
        clocktype delayDifference = 0;

        // for summary table
        if (isDataPacket)
        {
            lastDelayTime = stats->lastDataPacketDelayTime;
        }
        else
        {
            lastDelayTime = stats->lastControlPacketDelayTime;
        }

        if (lastDelayTime != CLOCKTYPE_MAX)
        {
            // Compute abs(delay difference)
            if (delay > lastDelayTime)
            {
                delayDifference = delay - lastDelayTime;
            }
            else
            {
                delayDifference = lastDelayTime - delay;
            }
            // Update jitter
            jitter += delayDifference;
        }
        if (isDataPacket)
        {
            stats->uDataPacketsRecd++;
            stats->uDataBytesRecd += MESSAGE_ReturnPacketSize(msg);

            stats->dataPacketDelay += delay;
            stats->dataPacketJitter += jitter;
            stats->lastDataPacketDelayTime = delay;
        }
        else
        {
            // Control packet sent
            stats->uControlPacketsRecd++;
            stats->uControlBytesRecd += MESSAGE_ReturnPacketSize(msg);

            stats->controlPacketDelay += delay;
            stats->controlPacketJitter += jitter;
            stats->lastControlPacketDelayTime = delay;
        }
        // for the aggr table
        clocktype aggrJitter = 0;
        if (stats->lastDelayTime != CLOCKTYPE_MAX)
        {
            // Compute abs(delay difference)
            if (delay > stats->lastDelayTime)
            {
                delayDifference = (clocktype)(delay - stats->lastDelayTime);
            }
            else
            {
                delayDifference = (clocktype)(stats->lastDelayTime - delay);
            }

            // Update jitter
            aggrJitter += delayDifference;
        }
        stats->totalJitter += aggrJitter;
        stats->lastDelayTime = delay;
    }
}


static
void HandleNetSumAggrMultiBroadcastStats(
    Node* node,
    Message* msg,
    NetworkDataIp* ip,
    IpHeaderType* ipHeader,
    BOOL isDataType,
    BOOL isForwardPacket,
    BOOL outgoing,
    Address prevHop,
    Address nextHop,
    Int32 interfaceIndex,
    const char* trafficType)
{
    if (interfaceIndex < 0)
    {
        if (NetworkIpIsMyIP(node, ipHeader->ip_src) &&
            NetworkIpIsMyIP(node, ipHeader->ip_dst) &&
            ip->isLoopbackEnabled)
        {
            // LoopBack
            return;
        }
        ERROR_ReportWarning ("Error: Interface Index must be >= 0\n");
        return;
    }

    Address tempAddr;
    MAPPING_SetAddress(NETWORK_IPV4, &tempAddr, &ipHeader->ip_dst);

    std::pair<Address, Address> key;
    if (outgoing)
    {
        key = std::pair<Address, Address>(prevHop, tempAddr);
    }
    else
    {
        key = std::pair<Address, Address>(nextHop, tempAddr);
    }

   multimap<pair<Address,Address>,
        OneHopNetworkData*, ltaddrpair>::iterator equal_iter;
                      equal_iter = ip->oneHopData->find(key);

    OneHopNetworkData* netData = NULL;
    BOOL found = FALSE;

    if (equal_iter == ip->oneHopData->end())
    {
        found = FALSE;
    }
    else
    {
        NetworkSumAggrDataIter iter = equal_iter;
        netData = iter->second;
        found = TRUE;
    }

    if (found)
    {
        OneHopNeighborStats* stats = &(netData->oneHopStats);
        HandleNetSumAggrStatsUpdate(node,
            msg,
            stats,
            outgoing,
            isForwardPacket,
            isDataType);
    }
    else
    {
        // Add a new entry.

        OneHopNetworkData* data = new OneHopNetworkData();
        InitializeStatsDbNetworkSummaryStats(&data->oneHopStats);

        HandleNetSumAggrStatsUpdate(node,
            msg,
            &data->oneHopStats,
            outgoing,
            isForwardPacket,
            isDataType);

        char addr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(&key.first, addr);
        data->srcAddr = addr;

        IO_ConvertIpAddressToString(&key.second, addr);
        data->rcvAddr = addr;

        data->destinationType = trafficType;

        if (!isForwardPacket)
        {
            data->isServer = 0;
        }
        else
        {
            data->isServer = 1;
        }
        ip->oneHopData->insert(
            pair<pair<Address, Address>, OneHopNetworkData*>(key, data));
    }

}


static
void HandleNetSumAggrUnicastStats(
    Node* node,
    Message* msg,
    NetworkDataIp* ip,
    IpHeaderType* ipHeader,
    BOOL isDataType,
    BOOL isForwardPacket,
    BOOL outgoing,
    Address prevHop,
    Address nextHop)
{

    std::pair<Address, Address> key;
    if (outgoing)
    {
        key = std::pair<Address, Address>(prevHop, nextHop);
    }
    else
    {
        key = std::pair<Address, Address>(nextHop, prevHop);
    }

    multimap<pair<Address,Address>,
        OneHopNetworkData*, ltaddrpair>::iterator equal_iter;
                      equal_iter = ip->oneHopData->find(key);

    OneHopNetworkData* netData = NULL;
    BOOL found = FALSE;

    if (equal_iter == ip->oneHopData->end())
    {
        found = FALSE;
    }
    else
    {
        NetworkSumAggrDataIter iter;
        iter = equal_iter;
        netData = iter->second;
        found = TRUE;
    }
    if (found)
    {
        // we have the entry update it.

        HandleNetSumAggrStatsUpdate(node,
            msg,
            &(netData->oneHopStats),
            outgoing,
            isForwardPacket,
            isDataType);
    }
    else
    {
        // Add a new entry.
        OneHopNetworkData* data = new OneHopNetworkData();
        InitializeStatsDbNetworkSummaryStats(&data->oneHopStats);

        HandleNetSumAggrStatsUpdate(node,
            msg,
            &data->oneHopStats,
            outgoing,
            isForwardPacket,
            isDataType);

        char addr[MAX_STRING_LENGTH];
        IO_ConvertIpAddressToString(&key.first, addr);

        data->srcAddr = addr;
        IO_ConvertIpAddressToString(&key.second, addr);
        data->rcvAddr = addr;

        data->destinationType = "Unicast";

        if (outgoing)
        {
            data->isServer = 0;
        }
        else
        {
            data->isServer = 1;
        }

        ip->oneHopData->insert(
            pair<pair<Address, Address>, OneHopNetworkData*>(key, data));
    }

}


static
void  HandleStatsDBNetworkSumAggrUpdate(Node* node,
                                        Message* msg,
                                        const Address &prevHop,
                                        const Address &nextHop,
                                        BOOL outgoing,
                                        Int32 interfaceIndex)
{
    // Sanity Check for the variables.
    NetworkDataIp* ip = (NetworkDataIp *) node->networkData.networkVar;
    ERROR_Assert(ip != NULL,
                    "Error in StatsDb, Network Data Unavailable\n");

    IpHeaderType* ipHeader = (IpHeaderType *) msg->packet;
    if (ipHeader == NULL)
    {
        ERROR_ReportWarning("Error in StatsDb,"
                            "Network IP Header Unavailable\n");
        return ;
    }

    BOOL isDataType = FALSE;
    BOOL isMulticastPacket = FALSE;
    BOOL isUnicastPacket = FALSE;
    BOOL isBroadcastPacket = FALSE;
    BOOL isForwardPacket = TRUE;
    Int32 i;

    // Check for Data or Control
    isDataType = IsDataPacket(msg, ipHeader);

    // Check for multicast packet.
    isMulticastPacket = NetworkIpIsMulticastAddress(node, ipHeader->ip_dst);

    // Check for broadcast packet
    if (!isMulticastPacket)
    {
        if (ipHeader->ip_dst == ANY_DEST )
        {
            isBroadcastPacket = TRUE;
        }
        else
        {
            for (i = 0; i < node->numberInterfaces; i++)
            {
                if (ipHeader->ip_dst ==
                    NetworkIpGetInterfaceBroadcastAddress(node, i))
                {
                    isBroadcastPacket = TRUE;
                    break ;
                }
            }
        }
        // Check for Unicast Packet
        if (!isBroadcastPacket )
        {
            isUnicastPacket = TRUE;
        }
    }


    // Check for packet forwarding
    if (NetworkIpIsMyIP(node, ipHeader->ip_src)
        || NetworkIpIsLoopbackInterfaceAddress(
                                    ipHeader->ip_src))
    {
        isForwardPacket = FALSE;
    }

    if (isUnicastPacket)
    {
        HandleNetSumAggrUnicastStats(
            node,
            msg,
            ip,
            ipHeader,
            isDataType,
            isForwardPacket,
            outgoing,
            prevHop,
            nextHop);
    }
    else if (isMulticastPacket)
    {
        HandleNetSumAggrMultiBroadcastStats(
            node,
            msg,
            ip,
            ipHeader,
            isDataType,
            isForwardPacket,
            outgoing,
            prevHop,
            nextHop,
            interfaceIndex, "Multicast");
    }
    else if (isBroadcastPacket)
    {
        HandleNetSumAggrMultiBroadcastStats(
            node,
            msg,
            ip,
            ipHeader,
            isDataType,
            isForwardPacket,
            outgoing,
            prevHop,
            nextHop,
            interfaceIndex, "Broadcast");
    }
    else
    {
        ERROR_ReportWarning(
                "Error in StatsDB HandleStatsDBNetworkSummaryUpdate.");
        return ;
    }
}

void HandleStatsDBNetworkInUpdate(Node* node,
                                  Message* msg,
                                  NodeAddress prevHop,
                                  Int32 incomingInterface)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (!db->statsSummaryTable->createNetworkSummaryTable
        && !db->statsAggregateTable->createNetworkAggregateTable)
    {
        return;
    }

    NodeAddress nextHop ;
    if (incomingInterface >= 0)
    {
        nextHop = NetworkIpGetInterfaceAddress(node,
                                               incomingInterface);
    }
    else
    {
        IpHeaderType* ipHeader = (IpHeaderType *) msg->packet;
        nextHop = ipHeader->ip_dst;
    }

    Address addressNextHop, addressPrevHop;
    MAPPING_SetAddress(NETWORK_IPV4, &addressNextHop, &nextHop);
    MAPPING_SetAddress(NETWORK_IPV4, &addressPrevHop, &prevHop);


    HandleStatsDBNetworkSumAggrUpdate(node,
                                      msg,
                                      addressPrevHop,
                                      addressNextHop,
                                      FALSE,
                                      incomingInterface); // FALSE - incoming
    // incoming includes inDeliver and inForward ...
}
void HandleStatsDBNetworkOutUpdate(Node* node,
                                   Message* msg,
                                   NodeAddress nextHop,
                                   Int32 outgoingInterface)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }
    if (!db->statsSummaryTable->createNetworkSummaryTable
        && !db->statsAggregateTable->createNetworkAggregateTable)
    {
        return;
    }

    Address addressNextHop, addressPrevHop;
    MAPPING_SetAddress(NETWORK_IPV4,
                       &addressNextHop,
                      (void *)(&nextHop));

    NodeAddress prevHop ;
    if (outgoingInterface >= 0)
    {
        prevHop = NetworkIpGetInterfaceAddress(node,
                                               outgoingInterface);
    }
    else
    {
        IpHeaderType* ipHeader = (IpHeaderType *) msg->packet;
        prevHop = ipHeader->ip_src;
    }

    MAPPING_SetAddress(NETWORK_IPV4, &addressPrevHop,
        (void *)(&prevHop));

    HandleStatsDBNetworkSumAggrUpdate(node,
                                      msg,
                                      addressPrevHop,
                                      addressNextHop,
                                      TRUE,
                                      outgoingInterface); // TRUE - outgoing

    // out includes orignating and outForwarding...
}
/*In this API, we first check if the one hop neighbor is a part of
the list of neighbors and if it is then update the stats else add the
neighbor*/


void HandleStatsDBTransSummaryInsertion(Node *node)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsSummaryTable->createTransSummaryTable)
    {
        return;
    }

    STATSDB_HandleTransSummaryTableInsert(node);
}
void HandleStatsDBNetworkSummaryInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsSummaryTable->createNetworkSummaryTable)
    {
        return;
    }
    STATSDB_HandleNetworkSummaryTableInsert(node);
}
//--------------------------------------------------------------------//
// NAME     : HandleStatsDBMacSummaryInsertion
// PURPOSE  : Calculate the statistics to be inserted into the MAC_Summary
//            for each transmit/receive pair and insert the results
// PARAMETERS :
// + node : starting node to traverse
//
// RETURN   : None.
//--------------------------------------------------------------------//

void HandleStatsDBMacSummaryInsertion(Node * node)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsSummaryTable->createMacSummaryTable)
    {
        return;
    }
    STATSDB_HandleMacSummaryTableInsert(node);
}

//--------------------------------------------------------------------//
// NAME     : HandleStatsDBPhySummaryInsert
// PURPOSE  : Calculate the statistics to be inserted into the PHY_Summary
//            for each transmit/receive pair and insert the results
// PARAMETERS :
// + node : starting node to traverse
//
// RETURN   : None.
//--------------------------------------------------------------------//
void HandleStatsDBPhySummaryInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsSummaryTable->createPhySummaryTable)
    {
        return;
    }

    STATSDB_HandlePhySummaryTableInsert(node);
}

//--------------------------------------------------------------------//
// NAME     : HandleStatsDBPhySummaryInsertionForMacProtocols
// PURPOSE  : Calculate the statistics to be inserted into the PHY_Summary
//            for each transmit/receive pair and insert the results
// PARAMETERS :
// + node : starting node to traverse
//
// RETURN   : None.
//--------------------------------------------------------------------//
void HandleStatsDBPhySummaryInsertionForMacProtocols(Node* node)
{
    StatsDBPhySummaryParam phyParam;
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsSummaryTable->createPhySummaryTable)
    {
        return;
    }
    Node* traverseNode = node;

    // Walk through all the nodes on this partition
    while (traverseNode != NULL)
    {

        // Walk through all the mac interfaces on this node
        Int32 interfaceIndex;
        for (interfaceIndex = 0; interfaceIndex < traverseNode->numberInterfaces; interfaceIndex++)
        {
            //check for virtual interface created by dual-ip, this is not supported
            if (TunnelIsVirtualInterface(traverseNode, interfaceIndex))
            {
                continue;
            }
            std::map<Int32, MacOneHopNeighborStats>* oneHopData = NULL;
            if (traverseNode->macData[interfaceIndex]->macProtocol == MAC_PROTOCOL_LINK)
            {
                LinkData *link = (LinkData*)traverseNode->macData[interfaceIndex]->macVar;
                oneHopData = link->macOneHopData;
            }
            else if (traverseNode->macData[interfaceIndex]->macProtocol == MAC_PROTOCOL_SATCOM)
            {
                MacSatComData *satCom ;
                satCom = (MacSatComData*)traverseNode->macData[interfaceIndex]->macVar;
                oneHopData = satCom->macOneHopData;
            }

            if (oneHopData == NULL)
            {
                continue ;
            }

            // Now we have to loop over the list of transmit/receive pairs, and insert
            // the data in the table.
            std::map<Int32, MacOneHopNeighborStats>::iterator iter;

            // Iterate through the list of one-hop statistics. Each element in the oneHopData
            // list represents statistics collected for a particular transmit/receive pair,
            // uniquely identified by the transmitting node's ID (the current node is the
            // receiving node).
            for (iter = oneHopData->begin(); iter != oneHopData->end(); iter++)
            {
                // Sanity check to prevent accidental divide-by-zero
                if (iter->second.numSignals <= 0)
                {
                    continue;
                }

                phyParam.m_SenderId = iter->first;
                phyParam.m_RecieverId = traverseNode->nodeId;
                phyParam.m_PhyIndex = interfaceIndex;
                // Set channel index
                phyParam.m_ChannelIndex = -1;
                phyParam.m_NumSignals =
                    iter->second.numSignals;
                phyParam.m_NumErrorSignals =
                    iter->second.numErrorSignals;
                phyParam.m_Utilization =
                    iter->second.utilization / getSimTime(node);

                // Add the optional parameters
                if (db->statsPhySummary->isAvgDelay)
                {
                    phyParam.SetDelay(
                        (iter->second.totalDelay
                        / iter->second.numSignals) / SECOND);
                }

                // Insert in the table.
                STATSDB_HandlePhySummaryTableInsert(node, phyParam);

            } // end of for iter
        }
        traverseNode = traverseNode->nextNodeData;
    }
}

void StatsDBSendSummaryTimerMessage(PartitionData* partition)
{
    // In this function we send a timer message to the first node of
    // each partition. Once the message is received we calculate the
    // Summary stats for the simulation.
    // Create the Timer message and send out.

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }

    Message* summaryTimer;

    if (db->statsSummaryTable->createAppSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            STATSDB_APP_SUMMARY,
            MSG_STATSDB_APP_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime,
            false);
    }
    if (db->statsSummaryTable->createMulticastAppSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            STATSDB_MULTICAST_APP_SUMMARY,
            MSG_STATSDB_MULTICAST_APP_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
    if (db->statsSummaryTable->createTransSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_TRANSPORT_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
    if (db->statsSummaryTable->createNetworkSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_NETWORK_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
    if (db->statsSummaryTable->createQueueSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_QUEUE_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
    if (db->statsSummaryTable->createMacSummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
                STATSDB_LAYER,
                MAC_STATSDB_SUMMARY,
                MSG_STATS_MAC_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
    if (db->statsSummaryTable->createPhySummaryTable)
    {
        summaryTimer = MESSAGE_Alloc(partition,
                STATSDB_LAYER,
                PHY_STATSDB_SUMMARY,
                MSG_STATS_PHY_InsertSummary);

        summaryTimer->eventTime = partition->theCurrentTime +
            db->statsSummaryTable->summaryInterval;

        PARTITION_SchedulePartitionEvent(partition,
            summaryTimer,
            summaryTimer->eventTime);
    }
}

void StatsDBSendConnTimerMessage(PartitionData* partition)
{
    // In this function we send a timer message to the first node of
    // each partition. Once the message is received we calculate the
    // Summary stats for the simulation.
    // Create the Timer message and send out.

    StatsDb* db = partition->statsDb;
    Message* connTimer;

    if (db == NULL)
    {
        return;
    }
    if (db->statsConnTable->createAppConnTable)
    {
        connTimer = MESSAGE_Alloc(partition,
                                     STATSDB_LAYER,
                                     0,
                                     MSG_STATS_APP_InsertConn);
        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->AppConnTableInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }

    if (db->statsConnTable->createTransConnTable)
    {
        connTimer = MESSAGE_Alloc(partition,
                                     STATSDB_LAYER,
                                     0,
                                     MSG_STATS_TRANSPORT_InsertConn);

        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->TransConnTableInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }

    if (db->statsConnTable->createNetworkConnTable)
    {
        connTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_NETWORK_InsertConnectivity);

        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->networkConnSampleTimeInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }

    if (db->statsConnTable->createMacConnTable)
    {
        connTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_MAC_InsertConn);

        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->macConnSampleTimeInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }
    if (db->statsConnTable->createMulticastConnTable)
    {
        connTimer = MESSAGE_Alloc(partition,
            STATSDB_LAYER,
            0,
            MSG_STATS_MULTICAST_InsertConn);

        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->multicastConnSampleTimeInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }
    if (db->statsConnTable->createPhyConnTable)
    {

        connTimer = MESSAGE_Alloc(partition,
                                    STATSDB_LAYER,
                                    0,
                                    MSG_STATS_PHY_CONN_InsertConn);

        connTimer->eventTime = partition->theCurrentTime +
            db->statsConnTable->connectSampleTimeInterval;

        PARTITION_SchedulePartitionEvent(partition,
            connTimer,
            connTimer->eventTime);
    }
}

//--------------------------------------------------------------------------
// FUNCTION:  StatsDBSendLinkUtilizationTimerMessage
// PURPOSE:  to start  timer for updating LinkUtilization.
// PARAMETERS
// + partitionData : PartitionData* : Pointer to partition data.
// RETURN void.
//--------------------------------------------------------------------------
void
StatsDBSendLinkUtilizationTimerMessage(PartitionData* partition)
{
    Message* LinkUtilizationPerNodeTimer;
    Message* LinkUtilizationTimer;
    StatsDb* db = partition->statsDb;
    clocktype currentTime = getSimTime(partition->firstNode);
    if (db == NULL || db->StatsDBLinkUtilTable == NULL)
    {
        return;
    }

    if (db->StatsDBLinkUtilTable->createWNWLinkUtilizationTable)
    {

        LinkUtilizationPerNodeTimer = MESSAGE_Alloc(partition,
                        STATSDB_LAYER, STATSDB_MAC_LINK_UTILIZATION,//protocol defined in db.h
                        MSG_STATS_InsertWNWLinkUtilTablePerNode);

        LinkUtilizationPerNodeTimer->eventTime = partition->theCurrentTime +
            db->StatsDBLinkUtilTable->wnwLinkUtilizationTableInterval;

        PARTITION_SchedulePartitionEvent(partition,
            LinkUtilizationPerNodeTimer,
            LinkUtilizationPerNodeTimer->eventTime);

        if (partition->partitionId == 0)
        {// for linktuil table and linkutil per frame tanble.

            LinkUtilizationTimer = MESSAGE_Alloc(partition,
                            STATSDB_LAYER, STATSDB_MAC_LINK_UTILIZATION,//protocol defined in db.h
                            MSG_STATS_InsertWNWLinkUtilTable);//event type define in api.h

            LinkUtilizationTimer->eventTime = partition->theCurrentTime +
                db->StatsDBLinkUtilTable->wnwLinkUtilizationTableInterval;

            PARTITION_SchedulePartitionEvent(partition,
                LinkUtilizationTimer,
                LinkUtilizationTimer->eventTime);
        }
    }

    if (db->StatsDBLinkUtilTable->createNCWLinkUtilizationTable)
    {

        LinkUtilizationPerNodeTimer = MESSAGE_Alloc(partition,
                        STATSDB_LAYER, STATSDB_MAC_LINK_UTILIZATION,//protocol defined in db.h
                        MSG_STATS_InsertNCWLinkUtilTablePerNode);

        LinkUtilizationPerNodeTimer->eventTime = partition->theCurrentTime +
            db->StatsDBLinkUtilTable->ncwLinkUtilizationTableInterval;

        PARTITION_SchedulePartitionEvent(partition,
            LinkUtilizationPerNodeTimer,
            LinkUtilizationPerNodeTimer->eventTime);

        //eventtype comes before nodeid to break the tie.
        if (partition->partitionId == 0)
        {// for linktuil table and linkutil per frame table.

            LinkUtilizationTimer = MESSAGE_Alloc(partition,
                            STATSDB_LAYER, STATSDB_MAC_LINK_UTILIZATION,//protocol defined in db.h
                            MSG_STATS_InsertNCWLinkUtilTable);//event type define in api.h

            LinkUtilizationTimer->eventTime = partition->theCurrentTime +
                db->StatsDBLinkUtilTable->ncwLinkUtilizationTableInterval;

            PARTITION_SchedulePartitionEvent(partition,
                LinkUtilizationTimer,
                LinkUtilizationTimer->eventTime);
        }
    }

    return;
}

//--------------------------------------------------------------------//
// NAME     : HandlePhySummaryStatsUpdateForMacProtocols
// PURPOSE  : Update the summary statistics for a given mac interface. This
//            function is called whenever a frame is received.
// PARAMETERS :
// + node : current node
// + stats : pointer to the summary stat object corresponding to the
//           current senderID, receiverID, and phyIndex
// + interfaceIndex : current mac interface index
// + txDelay : the transmission delay of the current frame
// + totalDelay : the total delay encountered by the current frame
// + isErrorMessage : indicates if the current frame is received in error
// RETURN   : None.
//--------------------------------------------------------------------//
static void HandlePhySummaryStatsUpdateForMacProtocols(
    Node* node,
    MacOneHopNeighborStats* stats,
    Int32 interfaceIndex,
    clocktype txDelay,
    clocktype totalDelay,
    BOOL isErrorMessage)
{
    if (node->macData[interfaceIndex]->macProtocol == MAC_PROTOCOL_LINK
        || node->macData[interfaceIndex]->macProtocol == MAC_PROTOCOL_SATCOM)
    {
        // the satellite node of satcom does not have any utilization
        // i.e. the txDelay for the satellite node is inserted as -1.
        if (txDelay != -1)
        {
           stats->utilization += txDelay;
        }
        stats->totalDelay += totalDelay;
        if (!isErrorMessage)
        {
            stats->numSignals++;
        }else {
            // For now, this is always 0, but once microwave is supported
            // it might have some value
            stats->numErrorSignals++;
        }
    }
    else
    {
        return;
    }
}


//--------------------------------------------------------------------//
// NAME     : HandleStatsDBPhySummaryUpdateForMacProtocols
// PURPOSE  : Update the summary statistics for a given mac interface. This
//            function is called whenever a node's interface recieves a frame
//            and creates new stats objects as needed
// PARAMETERS :
// + node : current node
// + interfaceIndex : current interface index
// + sourceId : the sender of this frame
// + macOneHopData : pointer to the vector storing neighbor information
// + txDelay : the transmission delay of the current frame
// + totalDelay : the total delay encountered by the current frame
// + isErrorMessage : indicates if the current frame is received in error
// RETURN   : None.
//--------------------------------------------------------------------//
void HandleStatsDBPhySummaryUpdateForMacProtocols(
    Node* node,
    Int32 interfaceIndex,
    NodeAddress sourceId,
    std::map<Int32,MacOneHopNeighborStats>* macOneHopData,
    clocktype txDelay,
    clocktype totalDelay,
    BOOL isErrorMessage)
{

    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL || !db->statsSummaryTable->createPhySummaryTable)
    {
        return;
    }
    //the key is the senderId
    std::map<Int32,MacOneHopNeighborStats>::iterator iter;
    iter = macOneHopData->find(sourceId);
    if (iter == macOneHopData->end())
    {
        //create a new entry with updated stats if
        //there is no entry for this sender
        MacOneHopNeighborStats stats;
        InitializeStatsDbPhySummaryStatsForMacProtocols(&stats);
        HandlePhySummaryStatsUpdateForMacProtocols(node,
            &stats,
            interfaceIndex,
            txDelay,
            totalDelay,
            isErrorMessage);
        (*(macOneHopData))[sourceId] =
            stats;

    }
    else
    {
        // Update the existing entry
        MacOneHopNeighborStats* stats = &(iter->second);

        HandlePhySummaryStatsUpdateForMacProtocols(node,
            stats,
            interfaceIndex,
            txDelay,
            totalDelay,
            isErrorMessage);
    }
}
/*====================================================================*/
/*====================================================================*/
using namespace StatsQueueDB;

string StatsQueueDB::UInt64ToString(UInt64 num)
{
    ostringstream myStream;
    myStream << num << flush;

    return myStream.str();
}

string StatsQueueDB::Int64ToString(Int64 num)
{
    ostringstream myStream;
    myStream << num << flush;

    return myStream.str();
}


string StatsQueueDB::IntToString(Int32 num)
{
    ostringstream myStream;
    myStream << num << flush;

    return myStream.str();
}

string StatsQueueDB::DoubleToString(double f)
{
    ostringstream myStream;
    //myStream << f << flush;
    myStream<<setprecision(6) <<std::fixed<<f << flush;

    return myStream.str();
}

#ifdef ADDON_BOEINGFCS
#ifdef ADDON_NGCNMS
void QueueAggregateTbBuilder::AddDscpColumnsToTableStr(Node* node,
                                                       unsigned char ds)
{
    NetworkDataIp *ip
            = (NetworkDataIp *) node->networkData.networkVar;

    char dscpbuf[MAX_STRING_LENGTH];
    sprintf(dscpbuf, "DSCP_%d_Queued", ds);
    aggregateTableStr.push_back(string_pair(dscpbuf, " integer"));

    sprintf(dscpbuf, "DSCP_%d_Dequeued", ds);
    aggregateTableStr.push_back(string_pair(dscpbuf, " integer"));

    sprintf(dscpbuf, "DSCP_%d_Dropped", ds);
    aggregateTableStr.push_back(string_pair(dscpbuf, " integer"));

}
#endif
#endif

#ifndef __APPLE__
string & operator+=(string &inputS, Int32 d)
{
    inputS = IntToString(atoi(inputS.c_str()) + d);
    return inputS;
}

string & operator+=(string &inputS, Int64 d)
{
    inputS = Int64ToString(atoi(inputS.c_str()) + d);
    return inputS;
}

string & operator+=(string &inputS, double f)
{
    inputS = DoubleToString(atof(inputS.c_str()) + f);
    return inputS;
}
#endif

void QueueEventTbBuilder::InitializeQueueEventTable(
    PartitionData* partition,
    NodeInput* nodeInput)
{
    // In this function we initialize the Network Aggregate Table.
    // That is we create the table with the columns based on the
    // user input.
    string tableStr;
    BOOL wasFound = FALSE;

    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
#ifdef ADDON_BOEINGFCS
    eventTableStr.push_back(string_pair("Timestamp", "real"));
    eventTableStr.push_back(string_pair("MessageId", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("NodeId", "integer"));
    eventTableStr.push_back(string_pair("InterfaceIndex", "VARCHAR(16)"));
    eventTableStr.push_back(string_pair("ReservationId", "integer"));
    eventTableStr.push_back(string_pair("QueuePosition", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("QueueIndex", "integer"));
    eventTableStr.push_back(string_pair("SizeofPacket", "integer"));
    eventTableStr.push_back(string_pair("NextHop", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("PacketPriority", "integer"));
    eventTableStr.push_back(string_pair("EventType", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("FailureType", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("TimeinQueue", "real"));
    eventTableStr.push_back(string_pair("FreeSpace", "integer"));
    eventTableStr.push_back(string_pair("Dscp", "integer"));
#else
    eventTableStr.push_back(string_pair("Timestamp", "real"));
    eventTableStr.push_back(string_pair("MessageId", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("NodeId", "integer"));
    eventTableStr.push_back(string_pair("InterfaceIndex", "VARCHAR(16)"));
    eventTableStr.push_back(string_pair("QueuePosition", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("QueueIndex", "integer"));
    eventTableStr.push_back(string_pair("SizeofPacket", "integer"));
    eventTableStr.push_back(string_pair("PacketPriority", "integer"));
    eventTableStr.push_back(string_pair("EventType", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("FailureType", "VARCHAR(64)"));
    eventTableStr.push_back(string_pair("TimeinQueue", "real"));
    eventTableStr.push_back(string_pair("FreeSpace", "integer"));
#endif

    if (STATS_DEBUG)
    {
        start = WallClock::getTrueRealTime();
    }
    CreateTable(db, "QUEUE_Events", eventTableStr);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Queue Event table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

void QueueEventTbBuilder::BuildEvent(StatsDb* db,
    const vector<string>& eventPara)
{
    std::vector<std::string> columns;
    columns.reserve(eventTableStr.size());

    for (size_t i = 0; i < eventTableStr.size(); ++i)
    {
        columns.push_back(eventTableStr[i].first);
    }
    InsertValues(db, "QUEUE_Events", columns, eventPara);
}

//--------------------------------------------------------------------//
// NAME     : InitializeStatsDBQueueStatusTable
// PURPOSE  : Determine which columns should appear in the QUEUE_Status
//            table, then create it
// PARAMETERS :
// + partition : current partition
// + nodeInput : configuration file contents
//
// RETURN   : None.
//--------------------------------------------------------------------//
void InitializeStatsDBQueueStatusTable(
    PartitionData* partition, NodeInput* nodeInput)
{
    // In this function we initialize the Interface Status Table.
    // That is we create the table with the columns based on the
    // user input.
    clocktype start = 0;
    clocktype end = 0;

    StatsDb* db = partition->statsDb;
    if (db == NULL)
    {
        return;
    }
    DBColumns columns;
    columns.reserve(11);
    columns.push_back(string_pair("Timestamp", "real"));
    columns.push_back(string_pair("NodeId", "integer"));
    columns.push_back(string_pair("InterfaceIndex", "VARCHAR(16)"));
    columns.push_back(string_pair("QueuePosition", "VARCHAR(64)"));
    columns.push_back(string_pair("QueueIndex", "integer"));
    columns.push_back(string_pair("PacketsEnqueued", "integer"));
    columns.push_back(string_pair("PacketsDequeued", "integer"));
    columns.push_back(string_pair("PacketsDropped", "integer"));
    columns.push_back(string_pair("PacketsDroppedForcefully", "integer"));
    columns.push_back(string_pair("FreeSpace", "integer"));
    columns.push_back(string_pair("AverageServiceTime", "real"));

    if (STATS_DEBUG)
    {
        start = partition->wallClock->getRealTime();
    }
    CreateTable(db, "QUEUE_Status", columns);

    if (STATS_DEBUG)
    {
        end = WallClock::getTrueRealTime();
        clocktype diff = end - start;
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        printf ("Time Taken to create Queue Status table partition %d, %s\n",
                partition->partitionId,
                temp);
    }
}

// Queue Aggregate Table
#ifdef ADDON_BOEINGFCS
#ifdef ADDON_NGCNMS
void StatsQueueDb::STATSDB_UpdateQueueAggregateTableForDscp(
    Node* node,
    unsigned char ds)
{
    aggregateBuilder.AddDscpColumnsToTableStr(node, ds);
}
#endif
#endif

// Queue Event Table
void StatsQueueDb::STATSDB_InitializeQueueEventTable(
    PartitionData* partition,
    NodeInput* nodeInput)
{

    eventBuilder.InitializeQueueEventTable(partition, nodeInput);
}

void StatsQueueEventHook::eventHook(Message* msg,
                                    BOOL returnValue,
                                    Int32 queuePriority,
                                    StatsQueueEventType eventType,
                                    clocktype insertTime)
{
    vector<string> eventPara;

    // timestamp
    clocktype currentTime = getSimTime(node) ;
    eventPara.push_back(STATSDB_DoubleToString((double)currentTime/SECOND));


    // fix 09/01/08
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    ERROR_Assert(mapParamInfo ,
        "ERROR in StatsQueueEventHook::eventHook.");

    eventPara.push_back(std::string(mapParamInfo->msgId));

    // node id
    eventPara.push_back(STATSDB_IntToString(node->nodeId));

    //interfaceIndex
    eventPara.push_back(STATSDB_InterfaceToString(interfaceIndex));

#ifdef ADDON_BOEINGFCS
    // queue reservation Id
    QueuedPacketInfo *infoPtr = (QueuedPacketInfo *) MESSAGE_ReturnInfo(msg);
    eventPara.push_back(STATSDB_InterfaceToString(infoPtr->rsvnId));
#endif

    // queue position
    eventPara.push_back(queuePosition);

    // queue priority
    eventPara.push_back(STATSDB_IntToString(queuePriority));

    // size of packet
    eventPara.push_back(STATSDB_IntToString(MESSAGE_ReturnPacketSize(msg)));
#ifdef ADDON_BOEINGFCS
    char nextHopStr[MAX_STRING_LENGTH];
    QueuedPacketInfo * infoQueuePtr = (QueuedPacketInfo *) MESSAGE_ReturnInfo(msg);
    if (infoPtr)
    {
        IO_ConvertIpAddressToString(infoQueuePtr->nextHopAddress,
                                    nextHopStr);
    }
    else
    {
        sprintf(nextHopStr, "N/A");
    }

    eventPara.push_back(std::string(nextHopStr));
#endif

    // priority of packet
    char *ipHeaderPtr = msg->packet;

#ifdef ADDON_BOEINGFCS
    Int32 offset = RoutingCesMalsrGetIpHeaderOffset(node, msg);
    ipHeaderPtr += offset;
#endif
    IpHeaderType* ipHeader = (IpHeaderType*) ipHeaderPtr;
    eventPara.push_back(IntToString(
        IpHeaderGetTOS(ipHeader->ip_v_hl_tos_len)));

#ifdef ADDON_BOEINGFCS
    Int32 priority = IpHeaderGetTOS(ipHeader->ip_v_hl_tos_len);
    priority = priority >> 2;
#endif


    // event type
    switch(eventType)
    {
    case STATS_INSERT_PACKET:
        if (returnValue == FALSE)
        {
            eventPara.push_back("SuccessfulEnqueue");

            eventPara.push_back("");
        }else
        {

            eventPara.push_back("FailedEnqueue");
            eventPara.push_back("Enqueue Failed because Queue is Full");
        }

        break;
    case STATS_DEQUEUE_PACKET:
    case STATS_DISCARD_PACKET:
    case STATS_DROP_PACKET:
    case STATS_DROP_AGED_PACKET:
        eventPara.push_back("DeQueue");
        eventPara.push_back("");
        break;
        /*
    case STATS_DISCARD_PACKET:
        eventPara.push_back("Protocol Dequeues for Discard");
        break;
    case STATS_DROP_PACKET:
        eventPara.push_back("Protocol Dequeues for Drop");
        break;
    case STATS_DROP_AGED_PACKET:
        eventPara.push_back("Protocol Dequeues for Drop Aged");
        break ;*/
    case STATS_QUEUE_DROP_PACKET:
        eventPara.push_back("QueueDrop");
        eventPara.push_back("Simulation Resets or Ends");
        break ;
    default:
        ERROR_Assert(FALSE, "Error in Stat DB Queue Event Table.");
    }

    // time in queue
    if ((eventType == STATS_INSERT_PACKET) && (returnValue == FALSE))
    {
        eventPara.push_back("0");
    }else if (eventType != STATS_INSERT_PACKET)
    {
        ERROR_Assert(currentTime >= insertTime,
            "Error in Stat DB Queue Event Table.");
        eventPara.push_back(
            STATSDB_DoubleToString((double)(currentTime - insertTime)/SECOND));
    }
    else {
        eventPara.push_back("0");
    }

    eventPara.push_back(STATSDB_IntToString(_queue->freeSpaceInQueue()));

#ifdef ADDON_BOEINGFCS
    eventPara.push_back(STATSDB_IntToString(priority));
#endif

    StatsDb* db = node->partitionData->statsDb;
    db->queueDbPtr->STATSDB_HandleQueueEventTableUpdate(node, eventPara);
}

void StatsQueueDb::STATSDB_HandleQueueEventTableUpdate(Node*node,
        const vector<string>& eventParam)
{

    // In this table we insert the queue layer content on to the database.

    StatsDb* db = NULL;
    db = node->partitionData->statsDb;

    db->queueDbPtr->eventBuilder.BuildEvent(db, eventParam);

}

//--------------------------------------------------------------------//
// NAME     : HandleStatsDBQueueStatusInsertion
// PURPOSE  : Calculate the statistics to be inserted into the QUEUE_Status
//            for each queue
// PARAMETERS :
// + node : starting node to traverse
//
// RETURN   : None.
//--------------------------------------------------------------------//
void HandleStatsDBQueueStatusInsertion(Node* node)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL || !db->statsStatusTable->createQueueStatusTable)
    {
        return;
    }

    STATSDB_HandleQueueStatusTableInsertion(node);
}

//--------------------------------------------------------------------//
// NAME     : StatsDBGetTimestamp
// PURPOSE  : This function returns a timestamp for insertion into the
//            database that may differ from the internal simulation time.
//            The primary motiviation for this function is during CES
//            operation when the "warm-up time" should be subtracted from
//            the total simulation time.
// PARAMETERS :
// + node : current node
//
// RETURN   : None.
//--------------------------------------------------------------------//
clocktype StatsDBGetTimestamp(Node* node)
{
    // NOTE: This function is currently only a stub that returns the
    // sim time by default, but will eventually be updated to return
    // a custom timestamp.
    return getSimTime(node);
}

void StatsDBAddMessageAppMsgId(Node *node, Message *msg, StatsDBAppEventParam *appParam)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    if (mapParamInfo == NULL)
    {
        // Info does not exist. Insert one.
        mapParamInfo = (StatsDBMappingParam*) MESSAGE_AddInfo(
                          node,
                          msg,
                          sizeof(StatsDBMappingParam),
                          INFO_TYPE_StatsDbMapping);
    }
    ERROR_Assert(mapParamInfo != NULL, "Unable to add an info field!");

    std::string app_msgId = "N"+STATSDB_IntToString(node->nodeId);
    app_msgId += "S"+STATSDB_IntToString(appParam->m_MsgSeqNum);
    if (appParam->m_fragEnabled)
    {
        app_msgId += "AF"+STATSDB_IntToString(appParam->m_FragId);
    }
    //mapParamInfo->app_msgId.push_back(app_msgId);
    sprintf(mapParamInfo->msgId, "%s", app_msgId.c_str());
}
// needs to consider the fragmentation Id
static
void StatsDBAddMessageMsgId(Node *node, Message *msg)
{

    StatsDBMappingParam *mapParamInfo =
        (StatsDBMappingParam*) MESSAGE_AddInfo(
                      node,
                      msg,
                      sizeof(StatsDBMappingParam),
                      INFO_TYPE_StatsDbMapping);
    ERROR_Assert(mapParamInfo != NULL, "Unable to add an info field!");

    std::string net_msgId;
    net_msgId = "N"+STATSDB_IntToString(node->nodeId);
    net_msgId += "S"+STATSDB_IntToString(msg->sequenceNumber);
    //net_msgId += "NF"+STATSDB_IntToString(0);
    sprintf(mapParamInfo->msgId, "%s",net_msgId.c_str());

}

void StatsDBAddMessageMsgIdIfNone(Node *node, Message *msg)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    if (mapParamInfo == NULL)
    {
        StatsDBAddMessageMsgId(node, msg) ;
    }

}

void StatsDBAddMessageNextPrevHop(Node *node,
                                  Message *msg,
                                  NodeAddress nextHopAddr,
                                  NodeAddress prevHopAddr)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }
    StatsDBMessageNextPrevHop *nextPrevHopInfo = (StatsDBMessageNextPrevHop*)
            MESSAGE_ReturnInfo(msg, INFO_TYPE_MessageNextPrevHop);

    if (nextPrevHopInfo == NULL)
    {
        nextPrevHopInfo = (StatsDBMessageNextPrevHop*)
                          MESSAGE_AddInfo(node,
                                           msg,
                                           sizeof(StatsDBMessageNextPrevHop),
                                           INFO_TYPE_MessageNextPrevHop);
        ERROR_Assert(nextPrevHopInfo != NULL, "Unable to add an info field!");
    }

    nextPrevHopInfo->nextHopId = MAPPING_GetNodeIdFromInterfaceAddress(
                                     node,
                                     nextHopAddr);;
    nextPrevHopInfo->prevHopAddr = prevHopAddr;
}

//
// Add a info field for source IP address and destination IP address
//
void StatsDBAddMessageAddrInfo(Node *node,
                               Message *msg,
                               NodeAddress srcAddr,
                               NodeAddress dstAddr)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        return;
    }

    StatsDBMessageAddrInfo *msgAddrInfo = (StatsDBMessageAddrInfo*)
            MESSAGE_ReturnInfo( msg, INFO_TYPE_MessageAddrInfo);

    if (msgAddrInfo == NULL)
    {
        msgAddrInfo = (StatsDBMessageAddrInfo*)
                      MESSAGE_AddInfo(node,
                                      msg,
                                      sizeof(StatsDBMessageAddrInfo),
                                      INFO_TYPE_MessageAddrInfo);
        ERROR_Assert(msgAddrInfo != NULL, "Unable to add an info field!");
    }

    msgAddrInfo->srcAddr = srcAddr;
    msgAddrInfo->dstAddr = dstAddr;
}

//
//  This function is for message packing. Usually, the packed
//  message doesn't contain info fields of messages being packed in.
//  Calling this API will help put a copy of the address info field
//  of all messages being packed as info fields of the packed message.
void StatsDBCopyMessageAddrInfo(Node *node,
                                Message *dstMsg,
                                Message *srcMsg)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        return;
    }

    // loop thru all info fields in case the srcMsg is already a packed
    // message, thus contains multiple info fields of this type already
    UInt32 i = 0;
    for (i = 0; i < srcMsg->infoArray.size(); i ++)
    {
        MessageInfoHeader *hdrPtr = (MessageInfoHeader*)&(srcMsg->infoArray[i]);

        if (hdrPtr->infoType == INFO_TYPE_MessageAddrInfo)
        {
            StatsDBMessageAddrInfo *msgAddrInfo =
                (StatsDBMessageAddrInfo*) hdrPtr->info;

            if (msgAddrInfo != NULL)
            {
                // append this info to the dstMsg
                StatsDBMessageAddrInfo* infoPtr = (StatsDBMessageAddrInfo*)
                    MESSAGE_AppendInfo(node,
                                       dstMsg,
                                       sizeof(StatsDBMessageAddrInfo),
                                       INFO_TYPE_MessageAddrInfo);
                if (infoPtr != NULL)
                {
                    infoPtr->srcAddr = msgAddrInfo->srcAddr;
                    infoPtr->dstAddr = msgAddrInfo->dstAddr;
                }
            }
        }
    }
}

void StatsDBAppendMessageNetworkMsgId(Node* node,
                                      Message* msg,
                                      Int32 fragId)
{
    StatsDb* db = node->partitionData->statsDb;
    Int32 i;

    if (db == NULL)
    {
        // return if db is not enabled
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    ERROR_Assert(mapParamInfo, "Error in StatsDBAppendMessageNetworkMsgId.") ;
    // find the first non-digit character before F
    // append fragId depend on the character
    BOOL ipFragmentAgain = FALSE ;

    Int32 stringLength = (Int32)strlen(mapParamInfo->msgId);
    if (strchr(mapParamInfo->msgId, 'F') != NULL)
    {

        i = stringLength - 1;

        //ERROR_Assert(isdigit(mapParamInfo->msgId[i]),
        //    "Error in StatsDBAppendMessageNetworkMsgId.") ;
        //ERROR_Assert(mapParamInfo->msgId[0] == 'N',
        //    "Error in StatsDBAppendMessageNetworkMsgId.") ;
        if (!isdigit(mapParamInfo->msgId[i]))
        { return ;}

        while (i >= 0)
        {
            if (mapParamInfo->msgId[i] == 'F')
            {
                i-- ;

                if (mapParamInfo->msgId[i] == 'N')
                {
                    ipFragmentAgain = TRUE ;
                }
                break ;
            }else {
                i-- ;
            }

        }

    }
    if (ipFragmentAgain)
    {
        //  'NF'
        // apend the fragId in the format of "-(fragId)
        char buf[MAX_STRING_LENGTH] ;
        sprintf(buf, "-%d", fragId) ;
        ERROR_Assert(stringLength + strlen(buf) < MAX_STRING_LENGTH,
            "Error in StatsDBAppendMessageNetworkMsgId.") ;
        strcpy((char *)(mapParamInfo->msgId+stringLength), buf) ;
        //printf("new msgId is %s \n", mapParamInfo->msgId) ;
    }
    else {
        // no 'NF'
        // apend the fragId in the format of "NF(fragId)
        char buf[MAX_STRING_LENGTH] ;
        sprintf(buf, "NF%d", fragId) ;
        ERROR_Assert(stringLength + strlen(buf) < MAX_STRING_LENGTH,
            "Error in StatsDBAppendMessageNetworkMsgId.") ;
        strcpy((char *)(mapParamInfo->msgId+stringLength), buf) ;
        //printf("new msgId is %s \n", mapParamInfo->msgId) ;
    }

}

void StatsDBTrimMessageNetworkMsgId(Node *node, Message*msg)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        // return if db is not enabled
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    ERROR_Assert(mapParamInfo,
        "Error in StatsDBAppendMessageNetworkMsgId.") ;
    //ERROR_Assert(strchr(mapParamInfo->msgId, 'F'),
    //    "Error in StatsDBAppendMessageNetworkMsgId.") ;
    if (!strchr(mapParamInfo->msgId, 'F'))
    { return ;}

    Int32 stringLength = (Int32)strlen(mapParamInfo->msgId);
    Int32 i = stringLength - 1;
    while (i >= 0)
    {
        if (mapParamInfo->msgId[i] == 'F')
        {
            i-- ;

            if (mapParamInfo->msgId[i] == 'N')
            {
                break ;
            }
            ERROR_Assert(FALSE,
                "Error in StatsDBAppendMessageNetworkMsgId.") ;
        }else {
            i-- ;
        }
    }

    //ERROR_Assert(i, "Error in StatsDBAppendMessageNetworkMsgId.") ;
    if (i == 0)
    { return ;}
    mapParamInfo->msgId[i] = '\0';

    //printf("new msgId is %s \n", mapParamInfo->msgId) ;

}

void StatsDBAppendMessageMsgId(Node* node,
                               Message* msg,
                               Int32 fragId,
                               char l)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        // return if db is not enabled
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    ERROR_Assert(mapParamInfo, "Error in StatsDBAppendMessageMsgId.") ;

    // find the first non-digit character before F
    // append fragId depend on the character

    Int32 stringLength = (Int32)strlen(mapParamInfo->msgId);

    //ERROR_Assert(isdigit(mapParamInfo->msgId[stringLength-1]),
    //    "Error in StatsDBAppendMessageMsgId.") ;
    if (!isdigit(mapParamInfo->msgId[stringLength-1]))
    {return ;}
    ERROR_Assert(mapParamInfo->msgId[0] == 'N',
        "Error in StatsDBAppendMessageMsgId.") ;


    // no 'lF'
    // apend the fragId in the format of "lF(fragId)
    char buf[MAX_STRING_LENGTH] ;
    sprintf(buf, "%cF%d", l, fragId) ;
    ERROR_Assert(stringLength + strlen(buf) < MAX_STRING_LENGTH,
        "Error in StatsDBAppendMessageMsgId.") ;
    strcpy((char *)(mapParamInfo->msgId+stringLength), buf) ;
    //printf("new msgId is %s \n", mapParamInfo->msgId) ;

}

void StatsDBTrimMessageMsgId(Node *node, Message*msg, char l)
{
    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL)
    {
        // return if db is not enabled
        return;
    }
    StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
        MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);

    ERROR_Assert(mapParamInfo,
        "Error in StatsDBAppendMessageNetworkMsgId.") ;
    //ERROR_Assert(strchr(mapParamInfo->msgId, 'F'),
    //    "Error in StatsDBAppendMessageNetworkMsgId.") ;
    if (!strchr(mapParamInfo->msgId, 'F'))
    { return ;}

    Int32 stringLength = (Int32)strlen(mapParamInfo->msgId);
    Int32 i = stringLength - 1;
    while (i >= 0)
    {
        if (mapParamInfo->msgId[i] == 'F')
        {
            i-- ;

            if (mapParamInfo->msgId[i] == l)
            {
                break ;
            }
            ERROR_Assert(FALSE,
                "Error in StatsDBTrimMessageMsgId.") ;
        }else {
            i-- ;
        }
    }

    //ERROR_Assert(i, "Error in StatsDBTrimMessageMsgId.") ;
    if (i == 0) { return ;}
    mapParamInfo->msgId[i] = '\0';

    //printf("new msgId is %s \n", mapParamInfo->msgId) ;

}
void HandleTransportDBEvents(Node* node,
                             Message* msg,
                             Int32 interfaceIndex,
                             const std::string& eventType,
                             const Address &srcAddr,
                             const Address &dstAddr,
                             short sourcePort,
                             short destPort,
                             Int32 hdrSize,
                             char* failureType)
{

    PartitionData* partition = node->partitionData;
    StatsDb* db = partition->statsDb;

    // Check if the Table exists.
    if (!db || !db->statsEventsTable->createTransEventsTable)
    {
        // Table does not exist
        return;
    }
    StatsDBTransEventContent *transEvent = db->statsTransEvents;

    if (eventType == "UDPSendToLower" || eventType == "UDPReceiveFromUpper"
         || eventType == "UDPReceiveFromLower" || eventType == "UDPSendToUpper" || eventType == "UDPDrop")
    {
        // Note: cbr traffic does not have this info field

         StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
            MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);
        ERROR_Assert(mapParamInfo, "Error in HandleTransportDBEvents.") ;

        StatsDBTransportEventParam transParam(node->nodeId,
                    mapParamInfo->msgId, MESSAGE_ReturnPacketSize(msg));

        transParam.SetMsgSeqNum(msg->sequenceNumber);

        transParam.SetConnectionType("UDP");
        transParam.SetHdrSize(hdrSize);
        transParam.SetEventType(eventType);
        transParam.SetFlags("Data");
        transParam.m_SenderPort = sourcePort;
        transParam.m_ReceiverPort = destPort;
        if (eventType == "UDPDrop")
        {
           transParam.SetMessageFailure(failureType);
        }

        HandleStatsDBTransportEventsInsert(node, msg, transParam);
    }
    else ERROR_Assert(FALSE, "unknown type in HandleTransportDBEvents.");

}

std::string STATSDB_TypeToString(BOOL isControl)
{
    if (isControl) {
        return "Control";
    }else {
        return "Data";
    }
}
void HandleTransportDBEvents(Node* node,
                             Message* msg,
                             Int32 interfaceIndex,
                             const std::string& eventType,
                             unsigned short sourcePort,
                             unsigned short destPort,
                             Int32 hdrSize,
                             Int32 msgSize)
{

    PartitionData* partition = node->partitionData;
    StatsDb* db = partition->statsDb;
    Int32 i;

    // Check if the Table exists.
    if (!db || !db->statsEventsTable->createTransEventsTable)
    {
        // Table does not exist
        return;
    }
    StatsDBTransEventContent *transEvent = db->statsTransEvents;

    if (eventType == "TCPSendToLower" || eventType == "TCPSendToUpper"
          || eventType == "TCPReceiveFromUpper" || eventType == "TCPReceiveFromLower" )
    {
        StatsDBMappingParam* tcpMapParamInfo = (StatsDBMappingParam*)
                    MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping) ;
        ERROR_Assert(tcpMapParamInfo, "Error in HandleTransportDBEvents.") ;

        Int32 numFrags = MESSAGE_ReturnNumFrags(msg);

        if (numFrags == 0)
        {
            // seqNum
            // tcp control segment

            StatsDBTransportEventParam transParam(node->nodeId,
                tcpMapParamInfo->msgId, msgSize);
            transParam.SetMsgSeqNum(msg->sequenceNumber);
            transParam.SetConnectionType("TCP");
            //transParam.SetFlags(tcp_flags); // flags
            transParam.SetFlags(static_cast<const string> (
                STATSDB_TypeToString(hdrSize == msgSize)));
            if (hdrSize == msgSize)
            {
                // control packet. No overhead
                transParam.SetHdrSize(0);
            }
            else
            {
                // data packet. Overhead is headersize
                transParam.SetHdrSize(hdrSize);
            }
            transParam.SetEventType(eventType);
            transParam.m_SenderPort = sourcePort;
            transParam.m_ReceiverPort = destPort;

            HandleStatsDBTransportEventsInsert(node, msg, transParam);

            return ;
        }

        for (i = 0; i < numFrags; ++i)
        {
            StatsDBMappingParam* mapParamInfo = (StatsDBMappingParam*)
                    MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping, i);

            ERROR_Assert(mapParamInfo ,
                "Error in HandleTransportDBEvents.") ;

            if (eventType == "TCPSendToUpper")
            {
                HandleStatsDBMessageIdMappingInsert(node,
                    tcpMapParamInfo->msgId,
                    mapParamInfo->msgId,
                    "TCP");

            }
            else if (eventType == "TCPSendToLower") { //TCPSendToLower

                HandleStatsDBMessageIdMappingInsert(node,
                    mapParamInfo->msgId,
                    tcpMapParamInfo->msgId,
                     "TCP");
            }
        }
        char msgId[MAX_STRING_LENGTH];
        sprintf(msgId, tcpMapParamInfo->msgId) ;

        StatsDBTransportEventParam transParam(node->nodeId,
            msgId, msgSize);

        transParam.SetMsgSeqNum(msg->sequenceNumber);
        transParam.SetConnectionType("TCP");
        transParam.SetHdrSize(hdrSize);
        //transParam.SetFlags(tcp_flags); // tcp flags
        transParam.SetFlags(static_cast<const string>
            (STATSDB_TypeToString(hdrSize == msgSize)));
        transParam.SetEventType(eventType);
        transParam.m_SenderPort = sourcePort;
        transParam.m_ReceiverPort = destPort;

        // First Extract the existing info field.
        HandleStatsDBTransportEventsInsert(node, msg, transParam);
    }
    else
    ERROR_Assert(FALSE, "unknown type in HandleTransportDBEvents.");
}

void HandleTransportDBSummary(Node* node,
                              Message* msg,
                              Int32 interfaceIndex,
                              const std::string& eventType,
                              const Address &srcAddr,
                              const Address &dstAddr,
                              short sport,
                              short dport,
                              Int32 msgSize)
{
    //depracated, models still using this routine need to edited to use transport stats api
}

void HandleTransportDBAggregate(
       Node* node,
       Message* msg,
       Int32 interfaceIndex,
       const std::string& eventType,
       Int32 msgSize)
{
    //depracated, models still using this routine need to edited to use transport stats api
}

void HandleMulticastDBStatus(
       Node* node,
       Int32 interfaceIndex,
       const std::string& eventType,
       NodeAddress srcAddr,
       NodeAddress groupAddr)
{

    StatsDb* db = node->partitionData->statsDb;
    if (!db || db->statsStatusTable->createMulticastStatusTable == FALSE)
    {
        return ;
    }

    std::pair<Int32, NodeAddress> key(srcAddr, groupAddr);

    StatsDBStatusTable::Const_MultiStatusIter iter =
        db->statsStatusTable->map_MultiStatus.find(key);
    StatsDBMulticastStatus *pStatus = NULL;

    if (iter == db->statsStatusTable->map_MultiStatus.end())
    {
        db->statsStatusTable->map_MultiStatus[key] =
            new StatsDBMulticastStatus;
        pStatus = db->statsStatusTable->map_MultiStatus[key];
    }
    else pStatus = iter->second;

    ERROR_Assert(pStatus, "Error in HandleMulticastDBStatus. ");

    if (eventType == "Join")
    {
        pStatus->timeJoined = STATSDB_DoubleToString(
            (double)getSimTime(node)/SECOND);

        if (!pStatus->timeLeft.empty() &&
            atof(pStatus->timeLeft.c_str()) < atof(pStatus->timeJoined.c_str()))
        {
            pStatus->timeLeft.clear();
            /*pStatus->joinPrint = FALSE;
            pStatus->leavePrint = FALSE;*/
        }
    }
    else if (eventType == "Leave")
    {

        pStatus->timeLeft = STATSDB_DoubleToString(
            (double)getSimTime(node)/SECOND);
    }
    else ERROR_Assert(FALSE, "Uknown eventType in HandleMulticastDBEvent. ");

}

void StatsDBInitializeMacStructure(Node* node, Int32 interfaceIndex)
{
    BOOL alreadyEnabled = NetworkIpInterfaceIsEnabled(node, interfaceIndex);
    if (!alreadyEnabled)
    {
        StatsDb* db = node->partitionData->statsDb;
        if (db == NULL)
        {
            return;
        }

        // If MAC summary table is enabled, we need to allocate memory for
        // the summary stats
        if (db->statsSummaryTable->createMacSummaryTable)
        {
            node->macData[interfaceIndex]->summaryStats
                = new StatsDBMacSummary;
        }

        if (db->statsAggregateTable->createMacAggregateTable)
        {
            node->macData[interfaceIndex]->aggregateStats
                = new StatsDBMacAggregate;
        }
    }
}

// initialize the one-hop data structure in ip for the network summary
void StatsDBInitializeNetSummaryStructure(
    Node *node)
{

    StatsDb* db = node->partitionData->statsDb;

    if (db == NULL) return ;
    if (db->statsSummaryTable->createNetworkSummaryTable ||
        db->statsAggregateTable->createNetworkAggregateTable)
    {

        NetworkDataIp *ip = (NetworkDataIp *) node->networkData.networkVar;
        ip->oneHopData = new NetworkSumAggrData;
    }
}

/*
* NAME:        HandleMulticastGroupInfoMessage.
* PURPOSE:     call proper event to process messages.
* PARAMETERS:  node - pointer to the node,
*              msg - pointer to the received message.
* RETURN:      none.
*/
void
HandleMulticastGroupInfoMessage(Node* node, Message* msg)
{

    GroupEventInfo* groupmemberInfo = (GroupEventInfo*)
                                        MESSAGE_ReturnInfo(msg);

    // fill the multicast map only by the first node
    if (node->nodeId == node->partitionData->firstNode->nodeId)
    {
        if (NetworkIpIsMulticastAddress(node, groupmemberInfo->mcastAddr))
        {
            STATSDB_BuildMulticastEffective(node,
                                            msg);
        }
    }

}

/*
* NAME:        STATSDB_SendMulticastGroupInfo
* PURPOSE:     Send the member join/leave info to the first node of each partition.
* PARAMETERS:  node - pointer to the node,
*              msg - pointer to the received message.
* RETURN:      none.
*/
void
STATSDB_SendMulticastGroupInfo(Node* node,
                               Message* msg,
                               NodeAddress groupAddr)
{
    short eventType;
    Int32 i;
    if (msg->eventType == MSG_NETWORK_JoinGroup)
    {
        eventType = MSG_STATSDB_TellSenderAboutGroupJoin;
    }
    else if (msg->eventType == MSG_NETWORK_LeaveGroup)
    {
        eventType = MSG_STATSDB_TellSenderAboutGroupLeave;
    }

    for (i = 0; i < node->partitionData->numPartitions; i++)
    {
        Message* memberInfoMsg;
        GroupEventInfo* groupMemberInfo;
        clocktype currTime = getSimTime(node);
        memberInfoMsg = MESSAGE_Alloc(node->partitionData,
                                      STATSDB_LAYER,
                                      STATSDB_NETWORK_MULTICAST,
                                      eventType);
        memberInfoMsg->eventTime = currTime;
        MESSAGE_InfoAlloc(node, memberInfoMsg, sizeof(GroupEventInfo));
        groupMemberInfo =
            (GroupEventInfo*)MESSAGE_ReturnInfo(memberInfoMsg);
        groupMemberInfo->mcastAddr = groupAddr;
        groupMemberInfo->memberId = node->nodeId;

        if (msg->eventType == MSG_NETWORK_JoinGroup)
        {
            groupMemberInfo->joinTime = currTime;
            groupMemberInfo->leaveTime = CLOCKTYPE_MAX;
        }
        else if (msg->eventType == MSG_NETWORK_LeaveGroup)
        {
            groupMemberInfo->joinTime =  CLOCKTYPE_MAX;
            groupMemberInfo->leaveTime = currTime;
        }

        if (i == node->partitionData->partitionId)
        {
            PARTITION_SchedulePartitionEvent(node->partitionData,
                                             memberInfoMsg,
                                             memberInfoMsg->eventTime);
        }
        else
        {

#ifdef PARALLEL
            memberInfoMsg->nodeId = ANY_DEST;
            memberInfoMsg->eventTime = node->partitionData->safeTime;
            PARALLEL_SendRemoteMessages(memberInfoMsg,
                                        node->partitionData,
                                        i);

#ifdef USE_MPI

            MESSAGE_Free(node->partitionData->firstNode, memberInfoMsg);
#endif
#endif
        }
    }
}

/*
* NAME:        STATSDB_MulticastMembershipCount
* PURPOSE:     Return the number of members in a multicast group
* PARAMETERS:  node - pointer to the node,
*              groupAddr - The multicast group
*              nodeMap - double pointer to AppData::APPL_MULTICAST_NODE_MAP
*                        map, used to return the node map list
* RETURN:      none.
*/
Int32 STATSDB_MulticastMembershipCount(
    Node* node,
    NodeAddress groupAddr,
    AppData::APPL_MULTICAST_NODE_MAP** nodeMap)
{
    AppData::APPL_MULTICAST_ADDR_ITER it;

    // If map is empty, then nobody has joined the group,
    // so return.
    if (node->appData.multicastMap == NULL)
    {
        return 0;
    }

    // Go through groupAddr row of the map
    it = node->appData.multicastMap->find(groupAddr);
    if (it == node->appData.multicastMap->end())
    {
        return 0;
    }

    *nodeMap = it->second;

    if (*nodeMap == NULL)
    {
        char errStr[MAX_STRING_LENGTH];
        char addrStr[MAX_STRING_LENGTH];

        IO_ConvertIpAddressToString(groupAddr, addrStr);
        sprintf(errStr, "NodeId map list is not allocated in "
                   "APPL_MULTICAST_MAP for multicast group %s \n",
                   addrStr);
        ERROR_ReportWarning(errStr);
    }

    return (*nodeMap)->size();
}

void STATSDB_ProcessEvent(PartitionData *partition, Message *msg)
{
    Node* node = partition->firstNode;
    switch (msg->eventType)
    {
        // STATSDB_NETWORK_MULTICAST:
        case MSG_STATSDB_TellSenderAboutGroupJoin:
        case MSG_STATSDB_TellSenderAboutGroupLeave:
        {
            // When a node joins a group, call the below function to
            // inform the first nodes of every partition about this join
            HandleMulticastGroupInfoMessage(node, msg);
            return;
        }
    }
    StatsDb* db = node->partitionData->statsDb;
    if (db == NULL)
    {
        ERROR_ReportWarning("Error in StatsDB\n");
        return ;
    }

    switch (msg->eventType)
    {
        case MSG_STATSDB_APP_InsertSummary:
        {
            HandleStatsDBAppSummaryInsertion(partition->firstNode);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATSDB_APP_InsertAggregate:
        {
            HandleStatsDBAppAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }

        case MSG_STATS_APP_InsertConn:
        {
            // Check for parallel code.
            if (node->partitionId != node->partitionData->partitionId)
            {
                // node data will not be available
                return;
            }

            HandleStatsDBAppConnInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsConnTable->AppConnTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_TRANSPORT_InsertSummary:
        {
            HandleStatsDBTransSummaryInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }

        case MSG_STATS_TRANSPORT_InsertAggregate:
        {
            HandleStatsDBTransAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }

        case MSG_STATS_TRANSPORT_InsertConn:
        {
            if (node->partitionId != node->partitionData->partitionId)
            {
                // node data will not be available
                return;
            }

            HandleStatsDBTransConnInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsConnTable->TransConnTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_MAC_InsertSummary:
        {
            HandleStatsDBMacSummaryInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_MAC_InsertAggregate:
        {
            HandleStatsDBMacAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_MAC_InsertConn:
        {
            HandleStatsDBMacConnInsertion(node) ;
            msg->eventTime = partition->theCurrentTime +
                db->statsConnTable->macConnSampleTimeInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_PHY_InsertSummary:
        {
            HandleStatsDBPhySummaryInsertion(node);
            HandleStatsDBPhySummaryInsertionForMacProtocols(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_PHY_InsertAggregate:
        {
            HandleStatsDBPhyAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }

        case MSG_STATS_PHY_CONN_InsertConn:
        {
            PHY_CONN_CollectConnectSample(partition);
            msg->eventTime =
                partition->theCurrentTime +
                db->statsConnTable->connectSampleTimeInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_NETWORK_InsertSummary:
        {
            HandleStatsDBNetworkSummaryInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_NETWORK_InsertAggregate:
        {
            HandleStatsDBNetworkAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_NETWORK_InsertConnectivity:
        {
            HandleStatsDBNetworkConnInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsConnTable->networkConnSampleTimeInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_QUEUE_InsertSummary:
        {
            HandleStatsDBQueueSummaryInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_QUEUE_InsertAggregate:
        {
            HandleStatsDBQueueAggregateInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsAggregateTable->aggregateInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_QUEUE_InsertStatus:
        {
            HandleStatsDBQueueStatusInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsStatusTable->statusInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATSDB_MULTICAST_APP_InsertSummary:
        {
            HandleStatsDBMulticastAppSummaryInsertion(node);
            msg->eventTime = partition->theCurrentTime +
                db->statsSummaryTable->summaryInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_MULTICAST_InsertConn:
        {
            StatsDBHandleMulticastConnInsertion(node, msg);
            msg->eventTime = partition->theCurrentTime +
                db->statsConnTable->multicastConnSampleTimeInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_MULTICAST_InsertStatus:
        {
            STATSDB_HandleMulticastStatusTableInsert(node, msg);
            msg->eventTime = partition->theCurrentTime +
                db->statsStatusTable->statusInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }

        case MSG_STATS_INTERFACE_InsertStatus:
        {
            STATSDB_HandleInterfaceStatusTableInsert(node->partitionData,
                FALSE, msg);
            msg->eventTime = partition->theCurrentTime +
                db->statsStatusTable->statusInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_NODE_InsertStatus:
        {
            if (db->statsStatusTable->createNodeStatusTable)
            {
                Node * nextNode = node;
                while (nextNode != NULL)
                {
                    StatsDBNodeStatus nodeStatus(nextNode, FALSE);
                    // Add this node's status information to the database
                    STATSDB_HandleNodeStatusTableInsert(nextNode, nodeStatus);
                    nextNode = nextNode->nextNodeData;
                }
                msg->eventTime = partition->theCurrentTime +
                    db->statsStatusTable->statusInterval;
                PARTITION_SchedulePartitionEvent(
                    partition, msg, msg->eventTime, false);
            }

            break;
        }
        case MSG_STATS_InsertWNWLinkUtilTable:
        {
            std::string str = "WNW";
            HandleStatsDBLinkUtilizationInsertion(node, &str);
            msg->eventTime = partition->theCurrentTime +
                    db->StatsDBLinkUtilTable->wnwLinkUtilizationTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_InsertWNWLinkUtilTablePerNode:
        {
            std::string str = "WNW";
            HandleStatsDBLinkUtilizationPerNodeInsertion(node, &str);
            msg->eventTime = partition->theCurrentTime +
                    db->StatsDBLinkUtilTable->wnwLinkUtilizationTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_InsertNCWLinkUtilTable:
        {
            std::string str = "NCW";
            HandleStatsDBLinkUtilizationInsertion(node, &str);
            msg->eventTime = partition->theCurrentTime +
                    db->StatsDBLinkUtilTable->ncwLinkUtilizationTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
        case MSG_STATS_InsertNCWLinkUtilTablePerNode:
        {
            std::string str = "NCW";
            HandleStatsDBLinkUtilizationPerNodeInsertion(node, &str);
            msg->eventTime = partition->theCurrentTime +
                    db->StatsDBLinkUtilTable->ncwLinkUtilizationTableInterval;
            PARTITION_SchedulePartitionEvent(
                partition, msg, msg->eventTime, false);
            break;
        }
    }
}


/*
* NAME:        STATSDB_BuildMulticastEffective
* PURPOSE:     Build the multicast map table required by stats db
*              to fill in the effective number of packets sent by a
*              multicast sender, whenever a receiver node joins the group.
* PARAMETERS:  node - pointer to the node who is the multicast sender,
*              msg - pointer to the message
*              eventType - specifies either a node join or leave event.
* RETURN:      none.
*/
void
STATSDB_BuildMulticastEffective(Node* node,
                                Message* msg)
{
    GroupEventInfo* groupMemberInfo = (GroupEventInfo*)
                                       MESSAGE_ReturnInfo(msg);
    NodeAddress mcastAddr = groupMemberInfo->mcastAddr;
    NodeAddress memberNodeId = groupMemberInfo->memberId;
    AppData::APPL_MULTICAST_NODE_MAP* nodeMap = NULL;

    if (NetworkIpGetNetworkProtocolType(node, memberNodeId)
                                                    == INVALID_NETWORK_TYPE)
    {
        char errStr[MAX_STRING_LENGTH];
        sprintf(errStr, "Node%u that is trying to join a group is not a "
            "valid node in this scenario\n", memberNodeId);
        ERROR_ReportWarning(errStr);
    }

    if (node->appData.multicastMap == NULL)
    {
        node->appData.multicastMap = new AppData::APPL_MULTICAST_MAP;
    }

    nodeMap = (*node->appData.multicastMap)[mcastAddr];

    // check whether this is the first time
    if (nodeMap == NULL)
    {
        nodeMap = new AppData::APPL_MULTICAST_NODE_MAP;
        (*node->appData.multicastMap)[mcastAddr] = nodeMap;
    }

    Int32& counter = (*nodeMap)[memberNodeId];

    if (msg->eventType == MSG_STATSDB_TellSenderAboutGroupJoin)
    {
        // Increment the NodeId counter value for every Join received
        counter++;
    }
    else
    {
        // Decrement the NodeId counter value for every Leave received
        counter--;

        if (counter == 0)
        {
            // remove memberNodeId from the map
            AppData::APPL_MULTICAST_ADDR_ITER itMap;
            AppData::APPL_MULTICAST_NODE_ITER nodeIt;

            itMap = node->appData.multicastMap->find(mcastAddr);
            if (itMap == node->appData.multicastMap->end())
            {
                char errStr[MAX_STRING_LENGTH];
                char addrStr[MAX_STRING_LENGTH];
                IO_ConvertIpAddressToString(mcastAddr, addrStr);
                sprintf(errStr, "Entry not found in the multicast map for "
                    "Node %u for the multicast group %\n", memberNodeId);
                ERROR_ReportWarning(errStr);
                return;
            }

            // Find the memberNodeId entry for this group and erase it
            nodeIt = itMap->second->find(memberNodeId);
            itMap->second->erase(nodeIt++);
        }

        else if (counter < 0)
        {
            char errStr[MAX_STRING_LENGTH];
            sprintf(errStr, "For member node %u, multicast leave event "
                            "occur before join\n", memberNodeId);
            ERROR_ReportWarning(errStr);
        }
    }
}


void StatsDB_PhyRecordStartTransmittingSignal(
    Node* node,
    Int32 phyIndex,
    Message* packet)
{

    StatsDb* db = node->partitionData->statsDb;
    if (db && db->statsEventsTable->createPhyEventsTable)
    {
        PhyData* thisPhy = node->phyData[phyIndex];
        StatsDBMappingParam *mapParamInfo = (StatsDBMappingParam*)
                       MESSAGE_ReturnInfo( packet, INFO_TYPE_StatsDbMapping);
        Int32 msgSize = 0;
        Int32 channelIndex = 0;

        if (!packet->isPacked)
        {
            msgSize = MESSAGE_ReturnPacketSize(packet);
        }
        else
        {
            msgSize = MESSAGE_ReturnActualPacketSize(packet);
        }
        StatsDBPhyEventParam phyParam(node->nodeId,
                                      mapParamInfo->msgId,
                                      phyIndex,
                                      msgSize,
                                      "PhyReceiveFromUpper");

        PHY_GetTransmissionChannel(node, phyIndex, &channelIndex);
        phyParam.SetChannelIndex(channelIndex);
        phyParam.SetControlSize(0);

        switch(thisPhy->phyModel)
        {
            case PHY_ABSTRACT:
            {
                PhyDataAbstract* phy_abstract =
                                  (PhyDataAbstract*)thisPhy->phyVar;
                phyParam.SetSignalPower(phy_abstract->txPower_dBm);
                break;
            }
            case PHY802_11a:
            case PHY802_11b:
            {
                PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
                phyParam.SetSignalPower(phy802_11->txPower_dBm);
                break;
            }
#ifdef LTE_LIB
            case PHY_LTE:
            {
                phyParam.SetSignalPower(PhyLteGetTxPower(node,
                                                         phyIndex,
                                                         packet));
                break;
            }
#endif
            default:
            {
                return;
            }
        }
        STATSDB_HandlePhyEventsTableInsert(node,
                                           phyParam);
    }
}

void StatsDB_PROPRecordSignalRelease(
    Node* node,
    Message* msg,
    Int32 phyIndex,
    Int32 channelIndex,
    float txPower_dBm)
{
    StatsDb* db = node->partitionData->statsDb;
    if (db && db->statsEventsTable->createPhyEventsTable)
    {
        // currently we support PHY_ABSTRACT, PHY_SRW_ABSTRACT, and PHY_802.11
        PhyData* thisPhy = node->phyData[phyIndex];
        StatsDBMappingParam* mapParamInfo = (StatsDBMappingParam*)
            MESSAGE_ReturnInfo( msg, INFO_TYPE_StatsDbMapping);
        Int32 msgSize = 0;
        Int32 controlSize = 0;

        if (!msg->isPacked)
        {
            msgSize = MESSAGE_ReturnPacketSize(msg);
        }
        else
        {
            msgSize = MESSAGE_ReturnActualPacketSize(msg);
        }
        StatsDBPhyEventParam phyParam(node->nodeId,
                                      mapParamInfo->msgId,
                                      phyIndex,
                                      msgSize,
                                      "PhySendSignal");
        phyParam.SetChannelIndex(channelIndex);

        switch (thisPhy->phyModel)
        {
            case PHY_ABSTRACT:
            case PHY_SRW_ABSTRACT:
            {
                PhyDataAbstract* phy_abstract =
                                      (PhyDataAbstract*)thisPhy->phyVar;
                phyParam.SetSignalPower(phy_abstract->txPower_dBm);
                controlSize = PHY_ABSTRACT_DEFAULT_HEADER_SIZE;
                break;
            }
            case PHY802_11a:
            {
                PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
                phyParam.SetSignalPower(phy802_11->txPower_dBm);
                controlSize = PHY802_11a_CONTROL_OVERHEAD_SIZE;
                break;
            }
            case PHY802_11b:
            {
                PhyData802_11* phy802_11 = (PhyData802_11*)thisPhy->phyVar;
                phyParam.SetSignalPower(phy802_11->txPower_dBm);
                controlSize = PHY802_11b_CONTROL_OVERHEAD_SIZE;
                break;
            }
#ifdef LTE_LIB
            case PHY_LTE:
            {
                phyParam.SetSignalPower(txPower_dBm);
                controlSize = PhyLteGetPhyControlInfoSize(node,
                                                          phyIndex,
                                                          channelIndex,
                                                          msg);
                break;
            }
#endif
            default:
            {
                return;
            }
        }
        phyParam.SetControlSize(controlSize);
        STATSDB_HandlePhyEventsTableInsert(node,
                                           phyParam);
    }
}

void StatsDB_CheckPacketPhyDrop(
    PartitionData* partition,
    PropChannel* propChannel,
    Int32 numNodes,
    Int32 channelIndex,
    Message* msg)
{
#ifdef ADDON_DB
    StatsDb* db = partition->statsDb;
    if (db  &&
        db->statsEventsTable  &&
        db->statsEventsTable->createPhyEventsTable)
    {

        Int32 i = 0;
        Node *thisNode = NULL ;
        if (numNodes)
        {
            thisNode = propChannel->nodeList[i];
        }
        Node *dbNextNode = partition->firstNode ;
        while (dbNextNode)
        {
            if (dbNextNode != thisNode)
            {
                PHY_NotificationOfPacketDrop(
                    dbNextNode,
                    -1,
                    channelIndex,
                    msg,
                    "PHY not Listenable to Channel",
                    0.0,
                    0.0,
                    0.0);
            }
            else
            {
                ++i;
                if (i < numNodes)
                {
                    if (i < propChannel->numNodes)
                    {
                        thisNode = propChannel->nodeList[i];
                    }
                    else
                    {
                        thisNode = propChannel->nodeListWithLI[i - propChannel->numNodes];
                    }
                }
            }
            dbNextNode = dbNextNode->nextNodeData;
        }
    }

#endif
}

std::string GatherWhereClause(
    const std::vector<std::string>& qualifierColumns,
    const std::vector<std::string>& qualifierValues,
    const std::string& joinOperator)
{
    if (qualifierColumns.size() > 0)
    {
        std::string query = " WHERE ";

        query += qualifierColumns[0];
        query += "='";
        query += qualifierValues[0];
        query += "'";

        for (size_t i = 1; i < qualifierColumns.size(); i++)
        {
            if (qualifierColumns[i].length() > 0)
            {
                query += " ";
                query += joinOperator;
                query += " ";
                query += qualifierColumns[i];
                query += "='";
                query += qualifierValues[i];
                query += "'";
            }
        }
        return query;
    }
    else
    {
        return "";
    }
}

void CreateTable(
    StatsDb* db,
    const std::string& name,
    const DBColumns& columns,
    const std::vector<std::string>& uniqueColumns,
    const std::string& constraintName,
    bool overwrite)
{
    if (columns.size() > 0)
    {
        if (overwrite)
        {
            DropTable(db, name);
        }

        std::string query = "CREATE TABLE IF NOT EXISTS ";
        query += name;
        query += "(";

        if (db->engineType == UTIL::Database::dbMySQL)
        {
            query += "RowId bigint auto_increment primary key,";
        }
        else if (db->engineType == UTIL::Database::dbSqlite)
        {
            query += "RowId INTEGER PRIMARY KEY AUTOINCREMENT,";
        }
        query += columns[0].first;
        query += " ";
        query += columns[0].second;

        for (size_t i = 1; i < columns.size(); i++ )
        {
            if (columns[i].first.length() > 0)
            {
                query += ("," + columns[i].first + " " + columns[i].second);
            }
        }

        if (constraintName != "")
        {
            query += ", CONSTRAINT ";
            query += constraintName;
            query += " ";
        }

        if (uniqueColumns.size() > 0)
        {
            query += " UNIQUE(";
            query += uniqueColumns[0];
            for (size_t i = 1; i<uniqueColumns.size(); i++)
            {
                query += ",";
                query += uniqueColumns[i];
            }
        }
        query += "));";

        db->driver->exec(query);   //Execute immediately, not an insert query
    }
}

void CreateTable(
    StatsDb* db,
    const std::string& name,
    const DBColumns& columns,
    const std::string& constraintName,
    bool overwrite)
{
    if (columns.size() > 0)
    {
        if (overwrite)
        {
            DropTable(db, name);
        }

        std::string query = "CREATE TABLE IF NOT EXISTS ";
        query += name;
        query += "(";

        if (db->engineType == UTIL::Database::dbMySQL)
        {
            query += "RowId bigint auto_increment primary key,";
        }
        else if (db->engineType == UTIL::Database::dbSqlite)
        {
            query += "RowId INTEGER PRIMARY KEY AUTOINCREMENT,";
        }
        query += columns[0].first;
        query += " ";
        query += columns[0].second;

        for (size_t i = 1; i < columns.size(); i++ )
        {
            if (!columns[i].first.empty())
            {
                query += ",";
                query += columns[i].first;
                query += " ";
                query += columns[i].second;
            }
        }
        query += ")";

        if (constraintName != "")
        {
            query += "CONSTRAINT ";
            query += constraintName;
            query += " ";
        }
        query += ";";

        db->driver->exec(query);  //Execute immediately, not an insert query
    }
}
void DropTable(
    StatsDb* db,
    const std::string& name)
{
    std::string query = "DROP TABLE IF EXISTS ";
    query += name;
    query += ";";
    db->driver->exec(query);  //Execute immediately, not an insert query
}

void AddColumn(
    StatsDb* db,
    const std::string& table,
    const std::string& name,
    const std::string& valueType)
{
    std::string query = "ALTER TABLE ";
    query += table;
    query += " ADD COLUMN ";
    query += name;
    query += " ";
    query += valueType;
    query += ";";

    db->driver->exec(query);  //Execute immediately, not an insert query
}
void InsertValues(
    StatsDb* db,
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& newValues)
{
    if (columns.size() > 0)
    {
        std::string query = "INSERT INTO ";
        query += table;
        query += "(";
        std::string values = "VALUES(";

        query += columns[0];
        values += newValues[0];

        for (size_t i = 1; i<newValues.size(); i++)
        {
            if (columns[i].length() > 0)
            {
                query += ",";
                query += columns[i];
                values += ",'";
                values += newValues[i];
                values += "'";
            }
        }
        query += ") ";
        query += values;
        query += ");";

        AddInsertQueryToBufferStatsDb(db, query);
    }
}

std::string GetInsertValuesSQL(
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& newValues)
{
    if (columns.size() > 0)
    {
        std::string query = "INSERT INTO ";
        query += table;
        query += "(";
        std::string values = "VALUES(";

        query += columns[0];
        values += "'";
        values += newValues[0];
        values += "'";

        for (size_t i = 1; i < newValues.size(); i++)
        {
            if (columns[i].length() > 0)
            {
                query += ",";
                query += columns[i];
                values += ",'";
                values += newValues[i];
                values += "'";
            }
        }
        query += ") ";
        query += values;
        query += ");";
        return query;
    }
    else
    {
        return "";
    }
}

void UpdateValues(
    StatsDb* db,
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& values,
    const std::vector<std::string>& qualifierColumns,
    const std::vector<std::string>& qualifierValues,
    const std::string &joinOperator)
{
    if (columns.size() > 0)
    {
        std::string query = "UPDATE ";
        query += table;
        query += " SET ";
        query += columns[0];
        query += "='";
        query += values[0];
        query += "'";

        for (size_t i = 1; i < columns.size(); i++)
        {
            if (columns[i].length() > 0)
            {
                query += ", ";
                query += columns[i];
                query += "='";
                query += values[i];
                query += "'";
            }
        }
        query += GatherWhereClause(qualifierColumns,
            qualifierValues,
            joinOperator);
        query += ";";
        db->driver->exec(query);
    }
}

std::string GetUpdateValuesSQL(
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& values,
    const std::vector<std::string>& qualifierColumns,
    const std::vector<std::string>& qualifierValues,
    const std::string &joinOperator)
{
    if (columns.size() > 0)
    {
        std::string query = "UPDATE ";
        query += table;
        query += " SET ";
        query += columns[0];
        query += "='";
        query += values[0];
        query += "'";

        for (size_t i = 1; i < columns.size(); i++)
        {
            if (columns[i].length() > 0)
            {
                query += ", ";
                query += columns[i];
                query += "='";
                query += values[i];
                query += "'";
            }
        }
        query += GatherWhereClause(qualifierColumns,
            qualifierValues,
            joinOperator);
        query += ";";
        return query;
    }
    return "";
}

void DeleteValues(
    StatsDb* db,
    const std::string &table,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string &joinOperator)
{
    if (db->engineType == UTIL::Database::dbMySQL)
    {
        std::string query = "DELETE FROM ";
        query += table;
        query += GatherWhereClause(qualifiersColumns, qualifiersValues, joinOperator);
        query += ";";
        db->driver->exec(query);
    }
    else if (db->engineType == UTIL::Database::dbSqlite)
    {
        std::string query = "BEGIN EXCLUSIVE;DELETE FROM ";
        query += table;
        query +=
            GatherWhereClause(
                qualifiersColumns,
                qualifiersValues,
                joinOperator);
        query += ";COMMIT;";
        db->driver->exec(query);  //Execute immediately, not an insert query
    }
}

std::string Select(
    StatsDb* db,
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string &joinOperator)
{
    std::string returnString;
    std::string sources = "SELECT ";

    if (columns.empty())
    {
         sources = "SELECT * FROM ";
    }
    else
    {
        sources+=columns[0];
        for (size_t i = 1; i < columns.size(); i++)
        {
            sources += ",";
            sources += columns[i];
        }
        sources += " FROM ";
    }
    sources += table;
    sources +=
        GatherWhereClause(qualifiersColumns, qualifiersValues, joinOperator);
    sources += ";";

    db->driver->exec(sources, returnString);  //Execute immediately, not an insert query

    return returnString;

}

std::string GetSelectSQL(
    const std::string& table,
    const std::vector<std::string>& columns,
    const std::vector<std::string>& qualifiersColumns,
    const std::vector<std::string>& qualifiersValues,
    const std::string& joinOperator)
{
    std::string returnString;
    std::string sources = "SELECT ";

    if (columns.empty())
    {
         sources = "SELECT * FROM ";
    }
    else
    {
        sources+=columns[0];
        for (size_t i = 1; i < columns.size(); i++)
        {
            sources += ",";
            sources += columns[i];
        }
        sources += " FROM ";
    }
    sources +=table;
    sources +=
        GatherWhereClause(qualifiersColumns, qualifiersValues, joinOperator);
    sources +=";";
    return sources;
}

void CreateIndex(
    StatsDb* db,
    const std::string& name,
    const std::string& table,
    const std::vector<std::string>& columns)
{
    if (columns.size() > 0)
    {
        std::string query = "CREATE INDEX ";
        query += name;
        query += " ON ";
        query += table;
        query += " (";

        query += columns[0];
        for (size_t i = 1; i < columns.size(); i++)
        {
            query += ",";
            query += columns[i];
        }
        query+=");";

        db->driver->exec(query); //Execute immediately, not an insert query
    }
}

void Synch(StatsDb* db)
{
    FlushQueryBufferStatsDb(db);
}

void SynchPlusParallelBarrier(PartitionData* partition)
{
    Synch(partition->statsDb);
    ParallelBarrier(partition);
}

void ParallelBarrier(PartitionData* partition)
{
#ifdef PARALLEL
    PARALLEL_SynchronizePartitions(partition);
#else
    //NOP
#endif /* PARALLEL */
}

void LockTable(const std::string& tableName, PartitionData* partitionData, bool read)
{
    if (partitionData->statsDb->engineType == UTIL::Database::dbMySQL)
    {
        if (partitionData->isRunningInParallel())
        {
            string str = "LOCK TABLES ";
            str += tableName;
            if (read)
            {
                str += " READ;";
            }
            else
            {
                str += " WRITE;";
            }
            partitionData->statsDb->driver->exec(str);
        }
    }
}

void UnlockTables(PartitionData* partitionData)
{
    if (partitionData->statsDb->engineType == UTIL::Database::dbMySQL)
    {
        if (partitionData->isRunningInParallel())
        {
            string str = "UNLOCK TABLES;";
            partitionData->statsDb->driver->exec(str);
        }
    }
}

void AddInsertQueryToBuffer(StatsDb* db,
                      const std::string& query)
{
    AddInsertQueryToBufferStatsDb(db, query);
}

void ExecuteMultipleNoReturnQueries(
    StatsDb* db,
    std::vector<std::string>& insertList)
{
    if (db->engineType == UTIL::Database::dbSqlite)
    {
        std::string tmpStr = "BEGIN EXCLUSIVE;";
        for (size_t i = 0; i < insertList.size(); i++)
        {
            tmpStr += insertList[i];
        }
        tmpStr+="COMMIT;";
        db->driver->exec(tmpStr);
    }
    else
    {
        for (size_t i = 0; i < insertList.size(); i++)
        {
            db->driver->exec(insertList[i]);
        }
    }
}

std::string Select(
    StatsDb* db,
    const std::string &query)
{
    string out;
    db->driver->exec(query, out);
    return out;
}
