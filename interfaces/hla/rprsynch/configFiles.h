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

#ifndef _CONFIG_FILES_H_
#define _CONFIG_FILES_H_

#include <string>
#include "sim_types.h"

namespace SNT_HLA
{

class ConfigFileWriter
{
    private:
        void copyFile(const std::string& src, const std::string& dst);
    public:
        void createScenarioDir(std::string scenarioPath);
        void writeConfigFile(std::string filename, NodeSet& ns);
        void writeEntitiesFile(std::string filename, FedAmb* fed);
        void writeRadiosFile(std::string filename, NodeSet& ns);
        void writeNetworksFile(std::string filename, NodeSet& ns);
        void writeNodesFile(std::string filename, NodeSet& ns);
        void writeSlotsFile(std::ostream& configFile, Config& config, NodeSet& ns);
        void writeRouterModelFile(std::string filename, NodeSet& ns);
        void copyFederationFile();
        void copyIconFiles();
        

};

};

#endif

