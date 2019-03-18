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

#ifndef _DBCORE_H_
#define _DBCORE_H_

#include <string>

#include "node.h"
#include "gestalt.h"

#include "db-mum.h"

namespace UTIL { namespace Database {

#define MAX_DB_SLEEP_COUNTER 10

enum dbEngineType
{
    dbSqlite,
    dbMySQL
};
struct DatabaseDriver : public QualNet::DynamicAPI::SimpleMarshaller {
    virtual void open(bool dropDatabase) = 0;
    virtual void close() = 0;

    virtual const char* error() = 0;

    virtual void exec(std::string query) = 0;
    virtual void exec(std::string in, std::string& out) = 0;

    dbEngineType engineType;
    unsigned sleepCounter;
} ;

static bool useWorkerThread() {
    bool request=UTIL::Gestalt::get_b("GESTALT-USE-WORKER-THREAD");

    return request;
}

}}

#endif
