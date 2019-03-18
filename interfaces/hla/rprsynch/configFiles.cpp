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

#include "configFiles.h"
#include "Config.h"
#include "hla_gcclatlon.h"
#include "hla_orientation.h"

#include <fstream>
#include <sstream>
#include <string.h>

using namespace SNT_HLA;

const unsigned short HLA_COUNTRYCODE_OTHER = 0;
const unsigned short HLA_COUNTRYCODE_AUS   = 13;
const unsigned short HLA_COUNTRYCODE_CAN   = 39;
const unsigned short HLA_COUNTRYCODE_FRA   = 71;
const unsigned short HLA_COUNTRYCODE_CIS   = 222;
const unsigned short HLA_COUNTRYCODE_GBR   = 224;
const unsigned short HLA_COUNTRYCODE_USA   = 225;

std::string getCountryString(unsigned short countryCode)
{
    switch (countryCode)
    {
    case HLA_COUNTRYCODE_AUS: return "AUS";
    case HLA_COUNTRYCODE_CAN: return "CAN";
    case HLA_COUNTRYCODE_FRA: return "FRA";
    case HLA_COUNTRYCODE_CIS: return "CIS";
    case HLA_COUNTRYCODE_GBR: return "GBR";
    case HLA_COUNTRYCODE_USA: return "USA";
    default:
        std::stringstream code;
        code << (unsigned int) countryCode;
        return code.str();
    }
}

void ConfigFileWriter::writeEntitiesFile(std::string filename, FedAmb* fed)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);

    std::map<RTI::ObjectHandle, HLAObject*>::iterator it =  fed->allObjects.begin();
    while( it != fed->allObjects.end() )
    {
        PhysicalEntity* ent = dynamic_cast<PhysicalEntity*>(it->second);
        if( ent )
        {
            int len = strlen((char*)ent->Marking.MarkingData);
            if( len == 0 || len > 11 )
            {
                it++;
                continue;
            }
            std::stringstream text;
            text << ent->Marking << ", ";
            text << ent->ForceIdentifier << ", ";
            text << getCountryString(ent->EntityType.countryCode) << ", ";
            text << ent->WorldLocation << ", ";
            text << ent->EntityType;
            out << ent->Marking << ", ";
            out << ent->ForceIdentifier << ", ";
            out << getCountryString(ent->EntityType.countryCode) << ", ";
            out << ent->WorldLocation << ", ";
            out << ent->EntityType;
            out << std::endl;
        }
        it++;
    }
    out.close();
}

void ConfigFileWriter::createScenarioDir(std::string scenarioPath)
{
    size_t pos = 1;
    while( pos != scenarioPath.npos )
    {
        pos = scenarioPath.find('\\', pos+1);
        std::string dir = scenarioPath.substr(0,pos);
        _mkdir(dir.c_str());
    }
}

void ConfigFileWriter::writeRadiosFile(std::string filename, NodeSet& ns)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);

    NodeSet::iterator it =  ns.begin();
    while( it != ns.end() )
    {
        PhysicalEntity* ent = (*it)->entity;
        int len = strlen((char*)ent->Marking.MarkingData);
        if( len == 0 || len > 11 )
        {
            it++;
            continue;
        }

        std::set<RadioTransmitter*>& radios = (*it)->radios;
        std::set<RadioTransmitter*>::iterator rit = radios.begin();
        if( rit != radios.end() )
        {
            RadioTransmitter* radio = *rit;
            out << (*it)->NodeId << ", ";
            out << ent->Marking << ", ";
            out << radio->RadioIndex << ", ";
            out << radio->RelativePosition << ", ";
            out << radio->RadioSystemType ;
            out << std::endl;
            //rit++;
        }
        it++;
    }
    out.close();
}

void ConfigFileWriter::writeNetworksFile(std::string filename, NodeSet& ns)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);

    NodeSet::iterator it =  ns.begin();
    out << "N1;3000000000;";
    while( it != ns.end() )
    {
        out << (*it)->NodeId;
        it++;
        if( it != ns.end() )
            out << ", ";
    }
    out << ";255.255.255.255";
    out.close();
}
void ConfigFileWriter::writeNodesFile(std::string filename, NodeSet& ns)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);
    out.precision(10);

    NodeSet::iterator it =  ns.begin();
    while( it != ns.end() )
    {
        PhysicalEntity* ent = (*it)->entity;
        double x = ent->WorldLocation.x;
        double y = ent->WorldLocation.y;
        double z = ent->WorldLocation.z;
        short azimuth, elevation;

        double lat = ent->WorldLocation.lat;
        double lon = ent->WorldLocation.lon;
        if( fabs(lat) > 90.0 || fabs(lon) > 180.0 )
        {
            lat = 0.0;
            lon = 0.0;
        }

        HlaConvertRprFomOrientationToQualNetOrientation( lat, lon,
            ent->Orientation.Phi, ent->Orientation.Theta, ent->Orientation.Psi,
            azimuth, elevation );
        std::set<RadioTransmitter*>& radios = (*it)->radios;
        std::set<RadioTransmitter*>::iterator rit = radios.begin();
        while( rit != radios.end() )
        {
            if( (*rit)->RadioIndex == 0 )
                break;
            rit++;
        }
        if( rit != radios.end() )
        {
            RadioTransmitter* radio = *rit;
            double lat, lon, alt;
            HlaConvertGccToLatLonAlt( x+radio->RelativePosition.BodyXDistance,
                y+radio->RelativePosition.BodyYDistance,
                z+radio->RelativePosition.BodyZDistance,
                lat, lon, alt );
            out << (*it)->NodeId << " 0 ";
            out << "(" << lat << ", " << lon << ", " << alt << ") ";
            out << azimuth << " ";
            out << elevation;
            out << std::endl;
        }
        it++;
    }
    out.close();
}
void ConfigFileWriter::writeSlotsFile(std::ostream& configFile, Config& config, NodeSet& ns)
{
    const Parameter &slotParm = config.getParameter("LINK-16-SLOT-FILE");
    if( ns.usesSlots() && slotParm != Parameter::unknownParameter )
    {
        configFile << slotParm << std::endl;

        std::string filename = slotParm.value;
        std::fstream out;
        out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);
    }
}
void ConfigFileWriter::writeRouterModelFile(std::string filename, NodeSet& ns)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);

    std::set<std::string>::iterator it = ns.modelsUsed().begin();
    while( it != ns.modelsUsed().end() )
    {
        ParameterMap* parameters = Config::instance().getModelParameterList(*it);
        const Parameter& model = parameters->getParameter("ROUTER-MODEL");
        if( model != Parameter::unknownParameter )
        {
            out << model << std::endl;
            ParameterMap::iterator pit = parameters->begin();
            while( pit != parameters->end() )
            {
                if( pit->first != "ROUTER-MODEL" )
                    out << pit->second << std::endl;
                pit++;
            }
            out << std::endl;
        }
        it++;
    }
}

void writeGeneralSection(std::ostream& out, Config& config)
{
    out << "# General" << std::endl;
    out << config.getParameter("VERSION") << std::endl;
    out << config.getParameter("EXPERIMENT-NAME") << std::endl;
    out << config.getParameter("SIMULATION-TIME") << std::endl;
    out << config.getParameter("SEED") << std::endl;
    out << std::endl;
}

void writeTerrainSection(std::ostream& out, Config& config)
{
    out << "# Terrain" << std::endl;
    out << config.getParameter("COORDINATE-SYSTEM") << std::endl;
    out << config.getParameter("TERRAIN-SOUTH-WEST-CORNER") << std::endl;
    out << config.getParameter("TERRAIN-NORTH-EAST-CORNER") << std::endl;
    out << config.getParameter("TERRAIN-DATA-BOUNDARY-CHECK") << std::endl;
    out << std::endl;
}

void writePositionSection(std::ostream& out, Config& config)
{
    out << "# Node positions" << std::endl;
    out << config.getParameter("NODE-PLACEMENT") << std::endl;
    out << config.getParameter("NODE-POSITION-FILE") << std::endl;
    out << std::endl;
}

void writeMobilitySection(std::ostream& out, Config& config)
{
    out << "# Mobility" << std::endl;
    out << config.getParameter("MOBILITY") << std::endl;
    out << config.getParameter("MOBILITY-POSITION-GRANULARITY") << std::endl;
    out << config.getParameter("MOBILITY-GROUND-NODE") << std::endl;
    out << std::endl;
}

void writeProgagationSection(std::ostream& out, Config& config, ChannelSet& channels)
{
    out << "# Propagation" << std::endl;
    for( size_t i=0; i<channels.size(); i++ )
    {
        std::stringstream channel;
        channel << "[" << i << "]";
        out << config.getParameter(std::string("PROPAGATION-CHANNEL-FREQUENCY")+channel.str()) << std::endl;
        out << config.getParameter(std::string("PROPAGATION-LIMIT")+channel.str()) << std::endl;
        out << config.getParameter(std::string("PROPAGATION-PATHLOSS-MODEL")+channel.str()) << std::endl;
        out << config.getParameter(std::string("PROPAGATION-SHADOWING-MODEL")+channel.str()) << std::endl;
        out << config.getParameter(std::string("PROPAGATION-SHADOWING-MEAN")+channel.str()) << std::endl;
        out << config.getParameter(std::string("PROPAGATION-FADING-MODEL")+channel.str()) << std::endl;
        out << std::endl;
    }
}
void writeParameters(std::ostream& out, std::string grouping, ParameterMap* parameters)
{
    ParameterMap::iterator pit = parameters->begin();
    while( pit != parameters->end() )
    {
        if( !grouping.empty() )
            out << grouping << " ";
        out << pit->second << std::endl;
        pit++;
    }
    out << std::endl;
}
void writePhysicalSection(std::ostream& out, Config& config, NetworkSet& nets)
{
    out << "# Physical" << std::endl;
    
    std::string defaultModel = config.getParameter("HLA-DEFAULT-ROUTER-MODEL").value;
    out << config.getParameter("ROUTER-MODEL-CONFIG-FILE") << std::endl;
    
    std::set<std::string> models;
    NetworkSet::iterator nit = nets.begin();
    while( nit != nets.end() )
    {
        if( (*nit)->models.size() == 0 )
            models.insert(defaultModel);
        else
            models.insert((*nit)->models.begin(), (*nit)->models.end());
        nit++;
    }
    if( models.size() == 1 )
    {
        ParameterMap* parameters = config.getModelParameterList(*models.begin());
        if(parameters)
            writeParameters(out, "", parameters);
    }
    else if( models.size() > 1 )
    {
        nit = nets.begin();
        while( nit != nets.end() )
        {
            Network* n = *nit;
            ParameterMap* common = n->getCommonParameters();
            std::stringstream subnet;
            subnet << "[" << n->address << "]";
            writeParameters(out, subnet.str(), common);

            std::set<std::string>::iterator mit = n->models.begin();
            while( mit != n->models.end() )
            {
                ParameterMap* parameters = config.getModelParameterList(*mit);
                ParameterMap* specfic = *parameters - *common;
                std::string group = n->getGroup(*mit);
                if( parameters && !group.empty() )
                    writeParameters(out, group, specfic);
                delete specfic;
                mit++;
            }
            nit++;
        }
    }
}

void writeStatisticsSection(std::ostream& out, Config& config)
{
    out << "# Statistics" << std::endl;
    std::vector<Parameter> parameters = config.getParameterList("-STATISTICS", Config::SUFFIX);
    for( size_t i=0; i<parameters.size(); i++ )
    {
        const std::string pn = parameters[i].name;
        if( parameters[i].name.find("HLA-") != 0 )
            out << parameters[i] << std::endl;
    }
    out << std::endl;
}
void writeNetworksSection(std::ostream& out, NodeSet& ns)
{
    out << "# Networks" << std::endl;
    NetworkSet& nets = ns.networksUsed();
    NetworkSet::iterator it = nets.begin();
    while( it != nets.end() )
    {
        Network* n = *it;
        out << "SUBNET " << n->address;
        out << " { ";
        std::set<Node*>::iterator nit = n->nodes.begin();
        double lat = 0.0;
        double lon = 0.0;
        double alt = 0.0;
        while( nit != n->nodes.end() )
        {
            out << (*nit)->NodeId << " ";
            lat += (*nit)->entity->WorldLocation.lat;
            lon += (*nit)->entity->WorldLocation.lon;
            alt += (*nit)->entity->WorldLocation.alt;
            nit++;
        }
        lat = lat / (1.0*n->nodes.size());
        lon = lon / (1.0*n->nodes.size());
        alt = alt / (1.0*n->nodes.size());
        out << "} " << lat << " " << lon << " " << alt << std::endl;
        std::stringstream mask;
        ChannelSet::iterator cit = ns.channelsUsed().begin();
        while( cit != ns.channelsUsed().end() )
        {
            if( *cit == n->channel )
                mask << "1";
            else
                mask << "0";
            cit++;
        }
        out << "[ " << n->address << "] PHY-LISTENABLE-CHANNEL-MASK " << mask.str() << std::endl;
        out << "[ " << n->address << "] PHY-LISTENING-CHANNEL-MASK " << mask.str() << std::endl;
        out << std::endl;
        it++;
    }
}

void writeHostnameSection(std::ostream& out, NodeSet& ns)
{
    NodeSet::iterator it = ns.begin();
    while( it != ns.end() )
    {
        out << "[" << (*it)->NodeId << "] HOSTNAME\t\"" << (*it)->entity->Marking.MarkingData << "\"" << std::endl;
        it++;
    }
    out << std::endl;
}

std::set<std::string> iconNames;

void writeNodeIconSection(std::ostream& out, NodeSet& ns, Config& config)
{
    iconNames.clear();
    NodeSet::iterator it = ns.begin();
    while( it != ns.end() )
    {
        EntityTypeStruct type = (*it)->entity->EntityType;
        const std::string& icon = (*it)->getIconName();
        if( !icon.empty() )
        {
            std::string filename = config.getFileName(icon);
            out << "[" << (*it)->NodeId << "] NODE-ICON";
            out << "\t\ticons" << config.dirSep << filename << std::endl;
            iconNames.insert(icon);
        }
        it++;
    }
    out << std::endl;
}
void writeHLASection(std::ostream& out, Config& config)
{
    out << "# HLA" << std::endl;
    Parameter p = config.getParameter("HLA");
    out << p << std::endl;
    if( p.value == "YES" )
    {
        std::vector<Parameter> parameters = config.getParameterList("HLA-", Config::PREFIX);
        for( size_t i=0; i<parameters.size(); i++ )
            out << parameters[i] << std::endl;
    }
    out << std::endl;
}

void writeDISSection(std::ostream& out, Config& config)
{
    out << "# DIS" << std::endl;
    Parameter p = config.getParameter("DIS");
    out << p << std::endl;
    if( p.value == "YES" )
    {
        std::vector<Parameter> parameters = config.getParameterList("DIS-", Config::PREFIX);
        for( size_t i=0; i<parameters.size(); i++ )
            out << parameters[i] << std::endl;
    }
    out << std::endl;
}
void writeComponentLine(std::ostream& out, Hierarchy* hier)
{
    out << "COMPONENT " << hier->id;
    out << " {";
    std::set<Node*, Node::less>::iterator nit = hier->nodes.begin();
    while( nit != hier->nodes.end() )
    {
        if( (*nit)->NodeId != -1 )
        {
            out << (*nit)->NodeId;
            nit++;
            if( nit != hier->nodes.end() )
                out << " ";
        }
        else
        {
            nit++;
        }
    }
    if( hier->children.size() > 0 && hier->nodes.size() > 0 )
        out << " ";
    std::set<Hierarchy*>::iterator hit = hier->children.begin();
    while( hit != hier->children.end() )
    {
        out << "H" << (*hit)->id;
        hit++;
        if( hit != hier->children.end() )
            out << " ";
    }
    if( hier->nodes.size() > 0 &&  hier->networks.size() > 0 )
        out << " ";
    std::set<Network*>::iterator netIt = hier->networks.begin();
    while( netIt != hier->networks.end() )
    {
        out << (*netIt)->address;
        netIt++;
        if( netIt != hier->networks.end() )
            out << " ";
    }
    out << "} ";
    out << hier->nodes.size() + hier->children.size();

    hier->computeTerrainBounds();
    out << " " << hier->position.x << " " << hier->position.y << " " << hier->position.z;
    out << " " << hier->dimensions.x << " " << hier->dimensions.y << " " << hier->dimensions.z;

    if( !hier->name.empty() )
        out << " HOSTNAME " << hier->name;

    out << std::endl;
}
void writeComponentSection(std::ostream& out, NodeSet& ns)
{
    out << "# COMPONENTS" << std::endl;

    AggregateEntity2HierarchyMap hierarchies = ns.hierarchiesUsed();
    if( hierarchies.size() == 0 )
        return;

    Hierarchy* hier = Hierarchy::rootHierarchy;
    if( hier && !(hier->children.size() == 0 && hier->nodes.size() == 0) )
        writeComponentLine(out, hier);

    AggregateEntity2HierarchyMap::iterator it = hierarchies.begin();
    while( it != hierarchies.end() )
    {
        hier = it->second;
        if( hier->children.size() == 0 && hier->nodes.size() == 0 )
            continue;
        writeComponentLine(out, hier);
        it++;
    }
    out << std::endl;
}

void ConfigFileWriter::writeConfigFile(std::string filename, NodeSet& ns)
{
    std::fstream out;
    out.open(filename.c_str(), std::fstream::out | std::fstream::trunc);

    writeGeneralSection(out, Config::instance());
    writeTerrainSection(out, Config::instance());
    //writeDeviceSection
    writePositionSection(out, Config::instance());
    writeMobilitySection(out, Config::instance());
    writeProgagationSection(out, Config::instance(), ns.channelsUsed());

    writeNetworksSection(out, ns);
    writePhysicalSection(out, Config::instance(), ns.networksUsed());
    /*writeMACSection
    writeNetworkProtocolSection
    writeRoutingSection
    writeTransportSection
    writeApplicationSection
    writeTracingSection
    */
    writeStatisticsSection(out, Config::instance());
    writeHostnameSection(out, ns);
    writeNodeIconSection(out, ns, Config::instance());
    writeSlotsFile(out, Config::instance(), ns);

    if( Config::instance().externalInterfaceType == "HLA" )
        writeHLASection(out, Config::instance());
    else if( Config::instance().externalInterfaceType == "DIS" )
        writeDISSection(out, Config::instance());
    writeComponentSection(out, ns);

    out.close();
}

void ConfigFileWriter::copyFederationFile()
{
    std::string fedFileName;
    std::string orgPath = Config::instance().fedPath;
    const std::string& dirSep = Config::instance().dirSep;
    size_t dir_end = orgPath.rfind(dirSep);
    if( dir_end == orgPath.npos )
        fedFileName = Config::instance().scenarioDir + Config::instance().dirSep + orgPath;
    else
        fedFileName = Config::instance().scenarioDir + Config::instance().dirSep + orgPath.substr(dir_end+1);
    copyFile(Config::instance().fedPath.c_str(), fedFileName.c_str());
}

void ConfigFileWriter::copyIconFiles()
{
    std::string iconDir = Config::instance().scenarioDir + Config::instance().dirSep + "icons" + Config::instance().dirSep;
    _mkdir(iconDir.c_str());
    std::set<std::string>::iterator it = iconNames.begin();
    while( it != iconNames.end() )
    {
        std::string filename = Config::instance().getFileName(*it);
        std::string path = Config::instance().findIconFile(filename);
        if( !filename.empty() && !path.empty() )
        {
            copyFile(path, iconDir+filename);
        }
        it++;
    }
}

void ConfigFileWriter::copyFile(const std::string& src, const std::string& dst)
{
    std::cout << "Copying file \"" << src << "\" to \"" << dst << "\"" << std::endl << std::flush;
    FILE* in = fopen(src.c_str(), "rb");
    if( in == NULL )
        return;

    FILE* out = fopen(dst.c_str(), "wb");
    if( out == NULL )
    {
        fclose(in);
        return;
    }

    fseek( in, 0, SEEK_END);
    size_t sze = ftell(in);
    rewind(in);

    size_t total = 0;
    while( !feof(in) )
    {
        char buf[4096];
        size_t r = fread(buf, 1, sizeof(buf), in);
        if( r < sizeof(buf) )
        {
            char *estr = strerror(errno);
            int err = ferror(in);
            if( err )
                std::cout << "Error reading file \"" << src << "\" : " << estr << std::endl << std::flush;
        }
        total += r;
        fwrite(buf, 1, r, out);
    }
    if( total < sze )
    {
        std::cout << "The file was not completely copied but no error was indicated. Please check the file." << std::endl << std::flush;
    }
    fclose(in);
    fclose(out);
}
