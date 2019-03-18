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

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <string>
#include <map>
#include <vector>
#include "rpr_types.h"
#include "lic.h"
#include <iostream>

namespace SNT_HLA
{

class Parameter
{
    public:
        static Parameter unknownParameter;
        std::string name;
        std::string value;

        Parameter( std::string Name, std::string Value ) :
                name(Name), value(Value) {}
        bool operator==(const Parameter &p) const
        {
            if( this == &p )
                return true;
            else if( name == p.name && value == p.value )
                return true;
            else
                return false;
        }
        bool operator!=(const Parameter &p) const
        {
            return !operator==(p);
        }
};

class ParameterMap : public std::map<std::string, Parameter>
{
    public:
        const Parameter& getParameter(std::string name)
        {
            iterator it = find(name);
            if( it != end() )
                return it->second;
            else
                return Parameter::unknownParameter;
        }
        void setParameter(Parameter para)
        {
            iterator it = find(para.name);
            if( it != end() )
            {
                it->second = para;
            }
            else
            {
                std::pair<std::string, Parameter> p(para.name, para);
                insert(p);
            }
        }
        ParameterMap* Intersection(const ParameterMap* pm) const
        {
            ParameterMap* result = new ParameterMap;
            const_iterator it1 = begin();
            while( it1 != end() )
            {
                const_iterator it2 = pm->find(it1->first);
                if( it2 != pm->end() && it2->second == it1->second )
                {
                    result->setParameter(it1->second);
                }
                it1++;
            }
            return result;
        }
        ParameterMap* operator-(const ParameterMap& pm) const
        {
            ParameterMap* result = new ParameterMap;
            const_iterator it1 = begin();
            while( it1 != end() )
            {
                const_iterator it2 = pm.find(it1->first);
                if( it2 == pm.end() || it2->second != it1->second )
                {
                    result->setParameter(it1->second);
                }
                it1++;
            }
            return result;
        }
};

class Config
{
    private:
#ifdef FLEXLM
        LM_HANDLE *lm_job;
#endif
        ParameterMap scenarioParameters;
public:
        std::map<std::string, ParameterMap*> models;
        RadioTypeMapWithWildCards<ParameterMap*> radioTypeToRouterModel;
private:
        Config();
        ~Config();
        int processFlag( const char* flag, const char* values[] );
        void licenseCheckout();
        void licenseCheckin();

    public:
        std::string dirSep;
        std::string scenarioName;
        std::string scenarioDir;
        std::string scenarioPath;
        std::string externalInterfaceType;
        std::string federationName;
        std::string fedPath;
        std::string federateName;
        std::string rprVersion;
        std::string defaultRouterModel;
        std::string exeHome;
        std::string qualnetHome;
        int firstNodeId;
        unsigned char networkPrefix;
        int timeout;
        bool merge;
        bool allEntitiesHaveRadios;
        bool debug;
        bool debugSetOnCmdLine;
        RadioTypeStruct defaultRadioSystemType;

        static Config& instance()
        {
            static Config cfg;
            return cfg;
        }
        bool parseCommandLine(int argc, const char* argv[]);
        void Usage(const char* program_name);
        void initializeScenarioParameters();
        void readParameterFiles();
        void readParameterFile(std::string filename);
        void readParameterFile(std::istream& in);
        void readModelFiles();
        void readModelFile(std::string filename);
        void readModelFile(std::istream& in);
        const Parameter& getParameter(std::string name)
        {
            return scenarioParameters.getParameter(name);
        }
        void setParameter(Parameter para)
        {
            scenarioParameters.setParameter(para);
        }
        ParameterMap* getModelParameterList(std::string name);
        ParameterMap* getModelParameterList(RadioTypeStruct radioType);
        typedef enum {ANY, PREFIX, SUFFIX} matchType;
        std::vector<Parameter> getParameterList(std::string name, matchType match= ANY);
        const std::string& getIconForType(const EntityTypeStruct& type);
        std::string getFileName(const std::string& path);
        const std::string findIconFile(const std::string& iconName);
};


std::ostream& operator<<(std::ostream& out, const Parameter& para);
};

void findFilesByExtension( const std::string extension, const std::string rootDir, std::vector<std::string> &filenameList);


#endif
