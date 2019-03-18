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

#ifndef _DBSQLITE3_H_
#define _DBSQLITE3_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "fileio.h"
#include "node.h"

#include "sqlite3.h"

#include "db-core.h"
#include "db-mum.h"

#if !defined(STATS_DEBUG)
#define STATS_DEBUG false
#endif

namespace UTIL { namespace Database {

struct Sqlite3Driver : public DatabaseDriver {
    sqlite3* dbFile;
    std::string dbFileName;

    Sqlite3Driver(std::string p_dbFileName)
    : dbFileName(p_dbFileName)
    {
        engineType = dbSqlite;
    }

    Sqlite3Driver(Sqlite3Driver& db) {
        dbFileName = db.dbFileName;
        engineType = dbSqlite;
    }

    const char* error() {
        return sqlite3_errmsg(dbFile);
    }

    
    void close() {
        if (STATS_DEBUG) {
            printf("SQLITE3:closing\n");
        }

        sqlite3_close(dbFile);

        if (STATS_DEBUG) 
        {
            printf("SQLITE3:closed\n");
        }
    }

    void open (bool dropDatabase) {
        int err;
        char errBuf[BUFSIZ];
    
        err = sqlite3_enable_shared_cache(1);
    
        if (err != SQLITE_OK) {
            sprintf(errBuf, 
                    "Problem setting Shared Cache "
                    "mode, error %d\n", 
                    err);
        
            ERROR_ReportError(errBuf);
        }

        // Remove a db file before opening it
        // The file may not exist, so ignore errors
        if (dropDatabase)
        {
            remove(dbFileName.c_str());
        }
    
        //err = sqlite3_open(partition->statsDbFileName, db);
        err = sqlite3_open(dbFileName.c_str(), &dbFile);
    
        if (err) {
            sprintf(errBuf, 
                    "Can't open database: %s", error());
        
            close();
        
            ERROR_ReportError(errBuf);
        }

        exec("PRAGMA synchronous=OFF;");
     }
    
    void exec(std::string query) {
        char* errMsg = NULL;
        char errBuf[BUFSIZ];
    
        int err;
        sleepCounter = 0;

        err = sqlite3_exec(dbFile, 
                           query.c_str(), 
                           0, 
                           0, 
                           &errMsg);
    
        while (err == SQLITE_LOCKED || err == SQLITE_BUSY) {
            sleepCounter++;

            err = sqlite3_exec(dbFile, 
                               query.c_str(), 
                               0, 
                               0, 
                               &errMsg);
        
            if (err == SQLITE_OK) {
                break;
            }
        
            if (sleepCounter > MAX_DB_SLEEP_COUNTER)
            {
                sprintf(errBuf, "Sleep Timeout: Cannot execute SQLite statement: %s.", error());
                close();
                ERROR_ReportError(errBuf);
            }
            else if (STATS_DEBUG_LOCK)
            {
                sprintf(errBuf, "Error in SQL Read: %s, count %d\n", error(), sleepCounter);
                ERROR_ReportWarning(errBuf);
            }

            EXTERNAL_Sleep(1 * SECOND);
        }
    
        if (err != SQLITE_OK) {
            std::string queryStr = errMsg;
            int location;
        
            location = queryStr.find_first_of(":", 0);
            std::string temp;
        
            temp = queryStr.substr(0, location);
        
            printf ("String is %s %s \n", 
                    queryStr.c_str(), 
                    temp.c_str());
        
            if (temp.compare("duplicate column name") != 0) {        
                sprintf(errBuf, "SQL error: %s\n", errMsg);
                close();
                printf ("Query is %s\n", query.c_str());
                ERROR_ReportError(errBuf);
            }
        }
    }
    
    void exec(char* query) {
        std::string strQuery = query;
        exec(strQuery);
    }

    void exec(std::string in, std::string& out)
    {
        int err(0);
        char** tbl;
        int nrow(0);
        int ncol(0);
        char *errMsg(NULL);
        char errBuf[BUFSIZ];

#if defined(TRACE_SQL)
        printf("ExecuteAsString receives query: \"%s\"\n", in.c_str());
#endif /* TRACE_SQL */

        out = "";
        sleepCounter = 0;
        err = sqlite3_get_table(dbFile,
                                in.c_str(),
                                &tbl,
                                &nrow,
                                &ncol,
                                &errMsg);

         while (err == SQLITE_LOCKED || err == SQLITE_BUSY) {
             sleepCounter++;

             err = sqlite3_get_table(dbFile,
                 in.c_str(),
                 &tbl,
                 &nrow,
                 &ncol,
                 &errMsg);
    
            if (err == SQLITE_OK) { 
                // we are done. Move on.  
                break; 
            }
            
            if (sleepCounter > MAX_DB_SLEEP_COUNTER)
            {
                sprintf(errBuf, "Sleep Timeout: Cannot execute SQLite statement: %s.", errMsg);            
                close();
                ERROR_ReportError(errBuf);
            }
            else if (STATS_DEBUG_LOCK)
            {
                sprintf(errBuf, "Error in SQL Read: %s, count %d\n", error(), sleepCounter);
                ERROR_ReportWarning(errBuf);
            }

            EXTERNAL_Sleep(1 * SECOND);
        } 
        
        if (err != SQLITE_OK) { 
            char errBuf[MAX_STRING_LENGTH]; 
    
            // We have a problem.  
            sprintf(errBuf, "SQL error: %s\n", errMsg); 
            sqlite3_close(dbFile); ERROR_ReportError(errBuf); 
        }

#if defined(DEBUG_MARSHALL)
        printf("ExecuteAsString found [%d, %d]\n",
               nrow, ncol);
#endif /* DEBUG_MARSHALL */

        // The result data structure captures the result from the database.
        // We now use these results to update the stats.

        if (ncol == 0 && nrow == 0) {
            // We have no data to process.
            out = "";
        } 
        else { 
            out = marshall(tbl, nrow, ncol); 
        } 
        
        sqlite3_free_table(tbl); 
        
#if defined(DEBUG_MARSHALL) 
        printf("ExecuteAsString returns response \"%s\"\n", 
               out.c_str()); 
#endif /* DEBUG_MARSHALL */ 
    }
} ;

}}

#endif

