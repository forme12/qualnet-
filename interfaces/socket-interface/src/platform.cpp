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

#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#endif
#include "api.h"
#include "platform.h"

void PrintTimeStamp(FILE *f)
{
#ifdef _WIN32
    SYSTEMTIME t;
    
    // Break down the time into seconds, minutes, etc...
    GetSystemTime(&t);

    fprintf(f, "[%02d:%02d:%02d] ", t.wHour,
                                    t.wMinute,
                                    t.wSecond);
    fflush(f);
#else /* unix/linux */
    time_t now;
    struct tm t;

    // Break down the time into seconds, minutes, etc...
    now = time(NULL);
    localtime_r(&now, &t);

    fprintf(f, "[%02d:%02d:%02d] ", t.tm_hour,
                                   t.tm_min,
                                   t.tm_sec);
    fflush(f);
#endif
}

void MakeDirectories(const char *prefix, char *dirName, int partitionId)
{
#ifdef _WIN32
    SYSTEMTIME t;
    BOOL success;
    char errString[MAX_STRING_LENGTH];

    GetLocalTime(&t);

    if (partitionId != 0)
    {
        sprintf(dirName, "%s%04d%02d%02d_%02d%02d%02d_p%d",
                        prefix,
                        t.wYear,
                        t.wMonth,
                        t.wDay,
                        t.wHour,
                        t.wMinute,
                        t.wSecond, partitionId);
    }
    else
    {
        sprintf(dirName, "%s%04d%02d%02d_%02d%02d%02d",
                        prefix,
                        t.wYear,
                        t.wMonth,
                        t.wDay,
                        t.wHour,
                        t.wMinute,
                        t.wSecond);
    }

    success = CreateDirectory(dirName, NULL);
    if (!success)
    {
        sprintf(errString, "Error creating log directory \"%s\" err = %d",
                           dirName, GetLastError());
        ERROR_ReportError(errString);
    }
#else /* unix/linux */
    time_t now;
    struct tm t;
    int err;

    // Break down the time into seconds, minutes, etc...
    now = time(NULL);
    localtime_r(&now, &t);

    // Create the directory
    if (partitionId != 0)
    {
        sprintf(dirName, "%s%04d%02d%02d_%02d%02d%02d_p%d",
                        prefix,
                        t.tm_year + 1900,
                        t.tm_mon + 1,
                        t.tm_mday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec, partitionId);
    }
    else
    {
        sprintf(dirName, "%s%04d%02d%02d_%02d%02d%02d",
                        prefix,
                        t.tm_year + 1900,
                        t.tm_mon + 1,
                        t.tm_mday,
                        t.tm_hour,
                        t.tm_min,
                        t.tm_sec);
    }
    err = mkdir(dirName, S_IRUSR | S_IWUSR | S_IXUSR
                         | S_IRGRP | S_IWGRP | S_IXGRP
                         | S_IROTH | S_IWOTH | S_IXOTH);
    if (err == -1)
    {
        ERROR_ReportError("Error creating log directory");
    }
#endif
}

void ExecNewProcess(std::vector<std::string>& command)
{
    int i;

    std::string processName = command[0];

#ifdef _WIN32
    BOOL success;
    char* str;
    PROCESS_INFORMATION processInfo;
    STARTUPINFO startup;
    char err[MAX_STRING_LENGTH];

    std::string commandLine;
    commandLine = processName;
    for (i = 1; i < command.size(); i++)
    {
        commandLine += " " + command[i];
    }

    // Convert command line string to regular non-const c string
    str = new char[commandLine.size() + 1];
    for (i = 0; i < commandLine.size(); i++)
    {
        str[i] = commandLine[i];
    }
    str[commandLine.size()] = 0;
    
    // Zero start-up info
    memset(&startup, 0, sizeof(STARTUPINFO));

    // Create the process
    success = CreateProcess(
        NULL,
        str,
        NULL,
        NULL,
        FALSE,
        CREATE_NEW_CONSOLE,
        NULL,
        NULL,
        &startup,
        &processInfo);
    if (!success)
    {
        sprintf(err, "Error %d beginning new QualNet process\n", GetLastError());
        ERROR_ReportError(err);
    }

#else // unix/linux
    pid_t pid;
    int err;
    char errStr[MAX_STRING_LENGTH];

    // Create new process
    pid = fork();

    // If it's the child
    if (pid == 0)
    {
        char** args;

        // Create args array
        args = new char*[command.size() + 1];
        for (i = 0; i < command.size(); i++)
        {
            args[i] = new char[command[i].size() + 1];
            strcpy(args[i], command[i].c_str());
        }
        args[command.size()] = NULL;

        err = execvp(processName.c_str(), args);
        if (err == -1)
        {
            sprintf(errStr, "Error execing %s", processName.c_str());
            perror(errStr);
        }
    }
#endif
}

