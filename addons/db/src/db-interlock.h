// Copyright (c) 2001-2007, Scalable Network Technologies, Inc.  All Rights Reserved.
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

#ifndef _DB_INTERLOCK_H_
#define _DB_INTERLOCK_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include "fileio.h"
#include "node.h"
#include "db.h"

// #define STATS_DEBUG false
#include "util_interlock.h"

#include "db-core.h"
#include "db-sqlite3.h"
#include "db-mysql-native.h"

#include "dbapi.h"

class DatabaseInterlock : public UTIL::Interlock<std::string>,
public UTIL::Worker<std::string>
{
    UTIL::Database::DatabaseDriver* driver_;
    int minQueryBuffer_;
    bool isOpen_;
    
    std::string listToQuery(std::list<std::string>& list)
    {
        // std::string query = "BEGIN EXCLUSIVE;\n";
        std::string query;
        std::list<std::string>::iterator pos = list.begin();
        
        for (; pos != list.end(); pos++)
        {
            query += *pos + "\n";
        }
        
        // query += "COMMIT;\n";
        
        return query;             
    }
    
public:
    
    DatabaseInterlock()
    : UTIL::Interlock<std::string>("Database Interlock",
                                   false), driver_(NULL),
                                   minQueryBuffer_(-1), 
                                   isOpen_(false)
    {

    }

    void start()
    {
        setWorker(this);
    }
    
    void run(std::list<std::string>& list)
    {
        if (driver_ != NULL)
        {
            if (isOpen_ == false) {
                driver_->open(false);
                isOpen_ = true;
            }

            // std::string query = listToQuery(list);

            std::list<std::string>::iterator pos = list.begin();

            if (driver_->engineType == UTIL::Database::dbSqlite)
            {
                driver_->exec("BEGIN;");
            }
            for (; pos != list.end(); pos++)
            {
                std::string query = *pos;
                driver_->exec(query);
            }
            if (driver_->engineType == UTIL::Database::dbSqlite)
            {
                driver_->exec("COMMIT;");
            }

            list.clear();

        }
    }
    
    void close()
    {
        if (driver_ != NULL && isOpen_) 
        {
            driver_->close();
            driver_ = NULL;
            isOpen_ = false;
        }
    }

    int push_back(StatsDb* db, const std::string &query)
    {
        take();

        if (driver_ == NULL) {
            if (db->engineType == UTIL::Database::dbMySQL)
            {
                driver_ = (UTIL::Database::DatabaseDriver*)
                    new UTIL::Database::MysqlNativeDriver(
                    *(UTIL::Database::MysqlNativeDriver*)db->driver);
            }
            else if (db->engineType == UTIL::Database::dbSqlite)
            {
                driver_ = (UTIL::Database::DatabaseDriver*)
                    new UTIL::Database::Sqlite3Driver(
                    *(UTIL::Database::Sqlite3Driver*)db->driver);
            }
            minQueryBuffer_ = db->minQueryBuffer;
        }
        
        int size = UTIL::Interlock<std::string>::push_back_xxx(query);
        
        give();

        if (size >= minQueryBuffer_) 
        {
            force_wake();
        }

        return size;
    }
} ;

#endif
