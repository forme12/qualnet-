// Copyright (c) 2001-2008, Scalable Network Technologies, Inc.  All Rights Reserved.
//                          6100 Center Drive West
//                          Suite 1250 
//                          Los Angeles, CA 90045
//                          sales@scalable-networks.com
//
// This source code is licensed, not sold, and is subject to a written
// license agreement.  Among other things, no portion of this source
// code may be copied, transmitted, disclosed, displayed, distributed,
// translated, used as the basis for a derivative work, or used, in
// whole or in part, for any program or purpose other than its intended
// use in compliance with the license agreement as part of the EXata
// software.  This source code and certain of the algorithms contained
// within it are confidential trade secrets of Scalable Network
// Technologies, Inc. and may not be used as the basis for any other
// software, hardware, product or service.

#include <RTI.hh>
#include <iostream>
#include "FedAmb.h"
#include "HLAbase.h"
#include "hla_types.h"
#include "rpr_types.h"
#include "sim_types.h"
#include "Config.h"
#include "configFiles.h"

using namespace SNT_HLA;

int main(int argc, const char* argv[])
{
    if( !Config::instance().parseCommandLine(argc, argv) )
    {
        Config::instance().Usage(argv[0]);
        return 1;
    }

    Config::instance().readParameterFiles();
    Config::instance().readModelFiles();
    // find the fed file
    // look in the following
    // specified on command line
    // current directory
    // RTI_CONFIG
    // QUALNET_HOME/addons/hla/rprsynch/data
    FedAmb *fed = new FedAmb;
    try
    {
        fed->Init(Config::instance().federationName, Config::instance().fedPath, Config::instance().federateName);
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "Error initializing Federation: " << &e << std::endl;
        return 1;
    }

    try
    {
        fed->Subscribe(Factory<BaseEntity>::instance().getClass());
        fed->Subscribe(Factory<PhysicalEntity>::instance().getClass());
        fed->Subscribe(Factory<EmbeddedSystem>::instance().getClass());
        fed->Subscribe(Factory<RadioTransmitter>::instance().getClass());
    }
    catch (RTI::Exception& e)
    {
        std::cerr << "Error subscribing : " << &e << std::endl;
        return 1;
    }

    try
    {
        fed->Subscribe(Factory<AggregateEntity>::instance().getClass());
    }
    catch (RTI::Exception& e)
    {
        ;
    }

    fed->Collect(Config::instance().timeout);

    fed->Cleanup(Config::instance().federationName);

    NodeSet ns;
    ns.extractNodes(fed);

    ConfigFileWriter cfWriter;
    cfWriter.createScenarioDir(Config::instance().scenarioDir);
    std::string path = Config::instance().scenarioDir + Config::instance().dirSep;
    cfWriter.writeRadiosFile(path+Config::instance().getParameter("HLA-RADIOS-FILE-PATH").value, ns);
    cfWriter.writeEntitiesFile(path+Config::instance().getParameter("HLA-ENTITIES-FILE-PATH").value, fed);
    cfWriter.writeNetworksFile(path+Config::instance().getParameter("HLA-NETWORKS-FILE-PATH").value, ns);
    cfWriter.writeNodesFile(path+Config::instance().getParameter("NODE-POSITION-FILE").value, ns);
    cfWriter.writeConfigFile(path+Config::instance().getParameter("CONFIG-FILE-PATH").value, ns);
    cfWriter.writeRouterModelFile(path+Config::instance().getParameter("ROUTER-MODEL-CONFIG-FILE").value, ns);
    cfWriter.copyFederationFile();
    cfWriter.copyIconFiles();

    return 0;
}
