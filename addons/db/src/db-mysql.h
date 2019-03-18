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

#ifndef _DBMYSQL_H_
#define _DBMYSQL_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>

#include <mysql++/mysql++.h>

#include "fileio.h"
#include "node.h"
#include "gestalt.h"

#include "util_interlock.h"

#include "db-core.h"
#include "db-mum.h"


namespace UTIL { namespace Database {

struct MysqlDriver : public DatabaseDriver
{
    mysqlpp::Connection conn;

    std::string server;
    std::string username;
    std::string password;
    std::string database;

    MysqlDriver(MysqlDriver& db) {
        server = db.server;
        username = db.username;
        password = db.password;
        database = db.database;
    }

    MysqlDriver(std::string p_server,
                std::string p_username,
                std::string p_password,
                std::string p_database)
    : server(p_server), username(p_username),
      password(p_password), database(p_database),
      conn(false)
    {

    }

    const char* error() { return (const char*)conn.error(); }
    const char* error(mysqlpp::Query& query) { return (const char*)query.error(); }
    
    void open () {        
        try {
            conn.connect("",
                         server.c_str(),
                         username.c_str(),
                         password.c_str());
        
            std::string sql;
            
            sql = "CREATE DATABASE IF NOT EXISTS " 
                + database;
            (conn.query(sql)).execute();

            sql = "USE " + database + ";";
            (conn.query(sql)).execute();

        } catch (mysqlpp::BadQuery& er) {
            std::string dberr = "Database query error: " 
                + std::string(er.what()) + ".";
            
            ERROR_ReportError((char*)dberr.c_str());
        } catch (mysqlpp::Exception& er) {
            std::string dberr = "Database general error: " 
                + std::string(er.what()) + ".";
            
            ERROR_ReportError((char*)dberr.c_str());
        }
    }
    
    void exec(std::string in, std::string& out)
    {
#if STATS_DEBUG
        // Not supported on Windows VC7 platform
        clocktype start = EXTERNAL_QueryRealTime();
#endif

        // printf("Executing \"%s\"\n", in.c_str());

        std::string query_error;
        
        try {
            mysqlpp::Query query = conn.query(in);
            mysqlpp::StoreQueryResult res = query.store();

            int nrow = res.size();
            int ncol = res.num_fields();

            if (nrow > 0) {
                std::string strtbl[nrow+1][ncol];

                for (int c = 0; c < ncol; c++) {
                    strtbl[0][c] = res.field_name(c);
                }

                for (int r = 0; r < nrow; r++) {
                    mysqlpp::Row row = res[r];

                    for (int c = 0; c < ncol; c++) {
                        strtbl[r+1][c] = std::string(row[c]);
                    }
                }

                int tblsize = (nrow+1) * ncol;
                char* tbl[tblsize];

                for (int k = 0; k < (nrow+1)*ncol; k++) {
                    tbl[k] = (char*)strtbl[k / ncol][k % ncol].c_str();
                }

                out = marshall(tbl, nrow, ncol);
            }
            else {
                out = "";
            }
        } catch (mysqlpp::BadQuery& er) {
            std::string query_error = "Database query error: " 
                + std::string(er.what()) + ".";
            
            ERROR_ReportError((char*)query_error.c_str());
        } catch (mysqlpp::Exception& er) {
            std::string dberr = "Database general error: " 
            + std::string(er.what()) + ".";
            
            ERROR_ReportError((char*)dberr.c_str());
        }
                
#if STATS_DEBUG
        // Not supported on Windows VC7 platform
        
        clocktype end = EXTERNAL_QueryRealTime();
        clocktype diff = end - start;
        
        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        
        printf ("Time Taken to exec query table, %s\n",
                temp);
#endif /* STATS_DEBUG */

    // printf("SELECT returns \"%s\"\n", out.c_str());
        
    }

    void exec(std::string query_str) {
#if STATS_DEBUG
        // Not supported on Windows VC7 platform
        clocktype start = EXTERNAL_QueryRealTime();
#endif

        // printf("Executing \"%s\"\n", query_str.c_str());

        std::string query_error;
        
        try {
            mysqlpp::Query query = conn.query(query_str);
            query.execute();
        } catch (mysqlpp::BadQuery& er) {
            std::string query_error = "Database query error: " 
                + std::string(er.what()) + ".";
            
            ERROR_ReportError((char*)query_error.c_str());
        } catch (mysqlpp::Exception& er) {
            std::string dberr = "Database general error: " 
                + std::string(er.what()) + ".";
        }
    }

    void close()  { conn.disconnect(); }
    
} ;

}}
    
#endif
