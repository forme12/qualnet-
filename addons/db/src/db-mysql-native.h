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

#ifndef _DBMYSQL_NATIVE_H_
#define _DBMYSQL_NATIVE_H_

#include <string>
#include <vector>
#include <iostream>
#include <list>
#include <stdio.h> 
#include <stdlib.h>

#ifdef WIN32
#include <windows.h>
#include <winsock.h>
#pragma warning(disable: 4514 4786)
#pragma warning(push, 3)
#endif /* WIN32 */

#if defined (__unix) || defined(__APPLE__)
#include <dlfcn.h>
#endif

#include <mysql.h>
#include <mysqld_error.h>

// #ifdef WIN32
// #include <unistd.h>
// #endif /* WIN32 */

#include "fileio.h"
#include "node.h"
#include "gestalt.h"

#include "util_interlock.h"
#include "product_info.h"
#include "db-core.h"
#include "db-mum.h"

// #define DB_PERFORMANCE_MONITOR
// #define DB_TRACE_SQL_STATEMENTS

namespace UTIL { namespace Database {

//mysql exported function callbacks 

#ifdef WINDOWS_OS
typedef MYSQL* (CALLBACK* mysql_init_cb)(MYSQL*);
typedef void (CALLBACK* mysql_close_cb)(MYSQL*);
typedef int (CALLBACK* mysql_real_query_cb)(MYSQL*, const char*, unsigned long);
typedef MYSQL* (CALLBACK* mysql_real_connect_cb)(MYSQL*, const char*, const char*,
                const char*, const char*, unsigned int, const char*, unsigned long);
typedef int (CALLBACK* mysql_select_db_cb)(MYSQL*, const char*);
typedef MYSQL_RES* (CALLBACK* mysql_store_result_cb)(MYSQL*);
typedef void (CALLBACK* mysql_free_result_cb)(MYSQL_RES*);
typedef MYSQL_ROW (CALLBACK* mysql_fetch_row_cb)(MYSQL_RES*);
typedef MYSQL_FIELD* (CALLBACK* mysql_fetch_fields_cb)(MYSQL_RES*);
typedef unsigned int (CALLBACK* mysql_num_fields_cb)(MYSQL_RES*);
typedef my_ulonglong (CALLBACK* mysql_affected_rows_cb)(MYSQL*);
typedef char* (CALLBACK* mysql_error_cb)(MYSQL*);
typedef unsigned int (CALLBACK* mysql_errno_cb)(MYSQL*);
#else
typedef MYSQL* (*mysql_init_cb)(MYSQL*);
typedef void (*mysql_close_cb)(MYSQL*);
typedef int (*mysql_real_query_cb)(MYSQL*, const char*, unsigned long);
typedef MYSQL* (*mysql_real_connect_cb)(MYSQL*, const char*, const char*,
                const char*, const char*, unsigned int, const char*,unsigned long);
typedef int (*mysql_select_db_cb)(MYSQL*, const char*);
typedef MYSQL_RES* (*mysql_store_result_cb)(MYSQL*);
typedef void (*mysql_free_result_cb)(MYSQL_RES*);
typedef MYSQL_ROW (*mysql_fetch_row_cb)(MYSQL_RES*);
typedef MYSQL_FIELD* (*mysql_fetch_fields_cb)(MYSQL_RES*);
typedef unsigned int (*mysql_num_fields_cb)(MYSQL_RES*);
typedef my_ulonglong (*mysql_affected_rows_cb)(MYSQL*);
typedef char* (*mysql_error_cb)(MYSQL*);
typedef unsigned int (*mysql_errno_cb)(MYSQL*);
#endif

struct MysqlNativeDriver : public DatabaseDriver
{
    MYSQL conn;

    std::string server;
    std::string username;
    std::string password;
    std::string database;

#ifdef WINDOWS_OS
    HINSTANCE dllHandle;
#else
    void* handle;
#endif

    mysql_init_cb mysql_init_ptr;
    mysql_close_cb mysql_close_ptr;
    mysql_real_query_cb mysql_real_query_ptr;
    mysql_real_connect_cb mysql_real_connect_ptr;
    mysql_select_db_cb mysql_select_db_ptr;
    mysql_store_result_cb mysql_store_result_ptr;
    mysql_free_result_cb mysql_free_result_ptr;
    mysql_fetch_row_cb mysql_fetch_row_ptr;
    mysql_fetch_fields_cb mysql_fetch_fields_ptr;
    mysql_num_fields_cb mysql_num_fields_ptr;
    mysql_affected_rows_cb mysql_affected_rows_ptr;
    mysql_error_cb mysql_error_ptr;
    mysql_errno_cb mysql_errno_ptr;
    
    void init() {
        if (mysql_init_ptr(&conn) == NULL) {
            ERROR_ReportError("Cannot initialize MYSQL API.");
        }
    }
    
    void setup() {
        mysql_init_ptr = NULL;
        mysql_close_ptr = NULL;
        mysql_real_query_ptr = NULL;
        mysql_real_connect_ptr = NULL;
        mysql_select_db_ptr = NULL;
        mysql_store_result_ptr = NULL;
        mysql_free_result_ptr = NULL;
        mysql_fetch_row_ptr = NULL;
        mysql_fetch_fields_ptr = NULL;
        mysql_num_fields_ptr = NULL;
        mysql_affected_rows_ptr = NULL;
        mysql_error_ptr = NULL;
        mysql_errno_ptr = NULL;

        std::string productHomePath;
        BOOL success = Product::GetProductHome(productHomePath);
        if (!success)
        {
            ERROR_ReportError("Cannot determine PRODUCT_HOME path \
                              therefore cannot load MYSQL libraries");
        }
        char fullpath[MAX_STRING_LENGTH];        
        strcpy(fullpath, productHomePath.c_str());
        
#ifdef WINDOWS_OS
        HINSTANCE dllHandle = NULL;
#ifdef _WIN64
        strcat(fullpath, "/lib/windows-x64/libmysql.dll");
#else
        strcat(fullpath, "/lib/windows/libmysql.dll");
#endif

        //Load the dll and keep the handle to it

        dllHandle = LoadLibrary(fullpath);

        // If the handle is valid, try to get the function address. 
        if (!dllHandle) 
        { 
            ERROR_ReportError("Cannot find libmysql.dll: Make sure the .dll is in your PATH\n");
        }
        else
        {
            //Get pointer to desired functions using GetProcAddress:
            mysql_init_ptr = (mysql_init_cb)GetProcAddress(dllHandle, "mysql_init");
            if (!mysql_init_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_init\n");
            }
            mysql_close_ptr = (mysql_close_cb)GetProcAddress(dllHandle, "mysql_close");
            if (!mysql_close_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_close\n");
            }
            mysql_real_query_ptr = (mysql_real_query_cb)GetProcAddress(dllHandle, "mysql_real_query");
            if (!mysql_real_query_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_real_query\n");
            }
            mysql_real_connect_ptr = (mysql_real_connect_cb)GetProcAddress(dllHandle, "mysql_real_connect");
            if (!mysql_real_connect_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_real_connect\n");
            }
            mysql_select_db_ptr = (mysql_select_db_cb)GetProcAddress(dllHandle, "mysql_select_db");
            if (!mysql_select_db_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_select_db\n");
            }
            mysql_store_result_ptr = (mysql_store_result_cb)GetProcAddress(dllHandle, "mysql_store_result");
            if (!mysql_store_result_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_store_result\n");
            }
            mysql_free_result_ptr = (mysql_free_result_cb)GetProcAddress(dllHandle, "mysql_free_result");
            if (!mysql_free_result_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_free_result\n");
            }
            mysql_fetch_row_ptr = (mysql_fetch_row_cb)GetProcAddress(dllHandle, "mysql_fetch_row");
            if (!mysql_fetch_row_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_fetch_row\n");
            }
            mysql_fetch_fields_ptr = (mysql_fetch_fields_cb)GetProcAddress(dllHandle, "mysql_fetch_fields");
            if (!mysql_fetch_fields_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_fetch_fields\n");
            }
            mysql_num_fields_ptr = (mysql_num_fields_cb)GetProcAddress(dllHandle, "mysql_num_fields");
            if (!mysql_num_fields_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_num_fields\n");
            }
            mysql_affected_rows_ptr = (mysql_affected_rows_cb)GetProcAddress(dllHandle, "mysql_affected_rows");
            if (!mysql_affected_rows_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_affected_rows\n");
            }
            mysql_error_ptr = (mysql_error_cb)GetProcAddress(dllHandle, "mysql_error");
            if (!mysql_error_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_error\n");
            }
            mysql_errno_ptr = (mysql_errno_cb)GetProcAddress(dllHandle, "mysql_errno");
            if (!mysql_errno_ptr)
            {
                ERROR_ReportError("Cannot functions address to mysql_errno\n");
            }
        }
#else

        handle = NULL;
        char *error;
        
#ifdef _LP64
        strcat(fullpath, "/lib/linux-common-x64/libmysql.so");
#else
        strcat(fullpath, "/lib/linux-common/libmysql.so");
#endif
        //Load the dll and keep the handle to it
        handle = dlopen(fullpath, RTLD_LAZY);

        // If the handle is valid, try to get the function address. 
        if (!handle) 
        { 
            ERROR_ReportError(dlerror());
        }
        else
        {
            dlerror(); /* Clear any existing error */

            //Get pointer to desired functions using GetProcAddress:
            mysql_init_ptr = (mysql_init_cb) dlsym(handle, "mysql_init");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_close_ptr = (mysql_close_cb) dlsym(handle, "mysql_close");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_real_query_ptr =
                (mysql_real_query_cb) dlsym(handle, "mysql_real_query");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_real_connect_ptr =
                (mysql_real_connect_cb) dlsym(handle, "mysql_real_connect");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_select_db_ptr =
                (mysql_select_db_cb) dlsym(handle, "mysql_select_db");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_store_result_ptr =
                (mysql_store_result_cb) dlsym(handle, "mysql_store_result");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_free_result_ptr =
                (mysql_free_result_cb) dlsym(handle, "mysql_free_result");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_fetch_row_ptr =
                (mysql_fetch_row_cb) dlsym(handle, "mysql_fetch_row");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_fetch_fields_ptr =
                (mysql_fetch_fields_cb) dlsym(handle, "mysql_fetch_fields");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_num_fields_ptr =
                (mysql_num_fields_cb) dlsym(handle, "mysql_num_fields");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_affected_rows_ptr =
                (mysql_affected_rows_cb) dlsym(handle, "mysql_affected_rows");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_error_ptr = (mysql_error_cb) dlsym(handle, "mysql_error");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
            mysql_errno_ptr = (mysql_errno_cb) dlsym(handle, "mysql_errno");
            if ((error = dlerror()) != NULL)
            {                
                ERROR_ReportError(error);
            }
        }
#endif

    }



    MysqlNativeDriver(MysqlNativeDriver& db) {
        server = db.server;
        username = db.username;
        password = db.password;
        database = db.database;

        setup();
        engineType = dbMySQL;
    }


    MysqlNativeDriver(std::string p_server,
                      std::string p_username,
                      std::string p_password,
                      std::string p_database)
    : server(p_server), username(p_username),
      password(p_password), database(p_database)
    {
        setup();
        engineType = dbMySQL;
    }


    ~MysqlNativeDriver()
    {
#ifdef WINDOWS_OS
        if (dllHandle)
        {
            //Free the library:
            FreeLibrary(dllHandle);
        }
#else
        if (handle)
        {
            //Free the library:
            dlclose(handle);
        }
        
#endif
    }


    const char* error()
    {     
        return mysql_error_ptr(&conn);
    }


    unsigned int errorNum()
    {
        return mysql_errno_ptr(&conn);
    }
    
    void open (bool dropDatabase) {
        init();

        char errBuf[BUFSIZ];

        if (!mysql_real_connect_ptr(&conn,
                                server.c_str(),
                                username.c_str(),
                                password.c_str(),
                                NULL, 0, NULL, 0))
        {
            sprintf(errBuf, "Cannot connect to MYSQL server: %s.", error());
            ERROR_ReportError(errBuf);
        }


        // Drop the database before opening it
        // The database may not exist, so ignore errors
        std::string sql;
        if (dropDatabase)
        {
            sql = "DROP DATABASE " + database + ";";
            mysql_real_query_ptr(&conn,
                                 sql.c_str(),
                                 sql.size());
        }

        sql = "CREATE DATABASE IF NOT EXISTS " 
            + database + ";" ;       

        if (mysql_real_query_ptr(&conn, 
                                 sql.c_str(),
                                 sql.size())) {
            sprintf(errBuf, "Cannot create simulation database: %s.", error());
            close();
            ERROR_ReportError(errBuf);
        }

        if (mysql_select_db_ptr(&conn, database.c_str())) {
            sprintf(errBuf, "Cannot create simulation database: %s.", error());
            close();
            ERROR_ReportError(errBuf);
        }
    }
    
    void exec(std::string in, std::string& out)
    {
        char errBuf[BUFSIZ];
        const char* query_c_str = in.c_str();

#ifdef DB_TRACE_SQL_STATEMENTS
        printf("Expect Response: %s\n", query_c_str);
#endif /* DB_TRACE_SQL_STATEMENTS */

#ifdef DB_PERFORMANCE_MONITOR
        clocktype start = WallClock::getTrueRealTime();
#endif /* DB_PERFORMANCE_MONITOR */
        
        int err = mysql_real_query_ptr(&conn, 
                                       query_c_str,
                                       in.size());

        while (err == ER_TABLE_NOT_LOCKED_FOR_WRITE ||
                err == ER_LOCK_OR_ACTIVE_TRANSACTION ||
                err == ER_CANT_UPDATE_WITH_READLOCK)
        {
            sleepCounter++;

            int err = mysql_real_query_ptr(&conn, 
                                           query_c_str,
                                           in.size());
        
            if (err == 0) {
                break;
            }
        
            if (sleepCounter > MAX_DB_SLEEP_COUNTER)
            {
                sprintf(errBuf, "Sleep Timeout: Cannot execute MYSQL statement: %s.", error());
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
    
        if (err > 0)
        {
            sprintf(errBuf, "Cannot execute MYSQL statement: %s.", error());
            close();
            ERROR_ReportError(errBuf);
        }

        MYSQL_RES *result = mysql_store_result_ptr(&conn);
        int nrow = (int)mysql_affected_rows_ptr(&conn);

        if (nrow < 1) {
            out = "";
        } 
        else {
            int ncol = mysql_num_fields_ptr(result);
            MYSQL_FIELD *fields = mysql_fetch_fields_ptr(result);

            int tblsize = (nrow+1) * ncol;
            char **tbl;

            tbl = new char*[tblsize];
            memset(tbl, 0, sizeof(char *) * tblsize) ;

            for (int c = 0; c < ncol; c++) {
                tbl[c] = fields[c].name;
            }

            MYSQL_ROW row;

            int r(0);
            while ((row = mysql_fetch_row_ptr(result))) {
                for (int c = 0; c < ncol; c++) {
                    if (row[c]!=NULL)
                    tbl[(r + 1) * ncol + c] = row[c];
                }
                ++r;
            }

            out = marshall(tbl, nrow, ncol);

            delete[] tbl;
        }

        mysql_free_result_ptr(result);

#ifdef DB_PERFORMANCE_MONITOR
        clocktype end = WallClock::getTrueRealTime();
        clocktype diff = end - start;

        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        
        printf ("@S %s\n", temp);
#endif /* DB_PERFORMANCE_MONITOR */
    }

    void exec(std::string query_str) {
        char errBuf[BUFSIZ];
        const char* query_c_str = query_str.c_str();

#ifdef DB_TRACE_SQL_STATEMENTS
        printf("Expect NO Response: %s\n", query_c_str);
#endif /* DB_TRACE_SQL_STATEMENTS */

#ifdef DB_PERFORMANCE_MONITOR
        clocktype start = WallClock::getTrueRealTime();
#endif /* DB_PERFORMANCE_MONITOR */

        sleepCounter = 0;

        int err = mysql_real_query_ptr(&conn, 
                                       query_c_str,
                                       query_str.size());

        while (err == ER_TABLE_NOT_LOCKED_FOR_WRITE ||
                err == ER_LOCK_OR_ACTIVE_TRANSACTION ||
                err == ER_CANT_UPDATE_WITH_READLOCK)
        {
            sleepCounter++;

            int err = mysql_real_query_ptr(&conn, 
                                           query_c_str,
                                           query_str.size());
        
            if (err == 0) {
                break;
            }
        
            if (sleepCounter > MAX_DB_SLEEP_COUNTER)
            {
                sprintf(errBuf, "Sleep Timeout: Cannot execute MYSQL statement: %s.", error());            
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
    
        if (err > 0)
        {
            sprintf(errBuf, "Cannot execute MYSQL statement: %s.", error()); 
            close();
            ERROR_ReportError(errBuf);
        }

#ifdef DB_PERFORMANCE_MONITOR
        clocktype end = WallClock::getTrueRealTime();
        clocktype diff = end - start;

        char temp[MAX_STRING_LENGTH];
        TIME_PrintClockInSecond(diff, temp);
        
        printf ("@I %s\n", temp);
#endif /* DB_PERFORMANCE_MONITOR */
    }

    void close()  {
        mysql_close_ptr(&conn);
    }
    
} ;

}}

#endif
