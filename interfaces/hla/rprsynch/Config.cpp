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

#include "Config.h"
#include "lic.h"
#include <iostream>
#include <sstream>
#include <algorithm>

#ifdef QT_CORE_LIB
#include <QFileInfo>
#endif

using namespace SNT_HLA;

Parameter Parameter::unknownParameter("", "Unknown Parameter");

Config::Config() :
        dirSep(DIR_SEP),
        scenarioName("ncfscenario"),
        scenarioDir(""),
        externalInterfaceType("HLA"),
        federationName("VR-Link"),
        fedPath("VR-Link.fed"),
        federateName("extractor"),
        rprVersion("1.0"),
        defaultRouterModel("802.11b"),
        firstNodeId(1),
        networkPrefix(60),
        timeout(5),
        merge(false),
        allEntitiesHaveRadios(false),
        debug(false),
        debugSetOnCmdLine(false),
        defaultRadioSystemType("1,1,225,1,1,1")
{
    const char *p = getenv("QUALNET_HOME");
    if (!p)
    {
#if defined(JNE_LIB)
        p = getenv("JNE_HOME");
#elif defined(EXATA)
        p = getenv("EXATA_HOME");
#else
        p = ".";  // We cannot leave the block empty
#endif
    }
    if (!p)
        p = ".";
    qualnetHome = p;

    licenseCheckout();
    setParameter(Parameter("HLA-DEFAULT-ROUTER-MODEL", defaultRouterModel));
}

Config::~Config()
{
    licenseCheckin();
}
int Config::processFlag(const char* flag, const char* values[])
{
    if (*(flag+1))
        return 0;
    switch( *flag )
    {
        case 'h':
        case '?':
            return 0;
            break;
        case 'd':
            debug = true;
            return 1;
            break;
        case 'm':
            merge = true;
            return 1;
            break;
        case 'o':
            allEntitiesHaveRadios = true;
            return 1;
            break;
        case 'r':
            rprVersion = "0.5";
            return 1;
            break;
        case 'f':
            if (values[0])
            {
                federationName = values[0];
                return 2;
            }
            else
            {
                return 0;
            }
            break;
        case 'F':
            if (values[0])
            {
                fedPath = values[0];
                return 2;
            }
            else
            {
                return 0;
            }
            break;
        case 'N':
            if (values[0])
            {
                firstNodeId = atoi(values[0]);
                return 2;
            }
            else
            {
                return 0;
            }
            break;
        case 's':
            if (values[0])
            {
                networkPrefix = atoi(values[0]);
                if (networkPrefix > 0 && networkPrefix < 224)
                    return 2;
                else
                    return 0;
            }
            else
            {
                return 0;
            }
            break;
        case 't':
            if (values[0])
            {
                timeout = atoi(values[0]);
                if (timeout > 0)
                    return 2;
                else
                    return 0;
            }
            else
            {
                return 0;
            }
            break;
        default:
            return 0;
            break;
    }
}

#ifdef FLEXLM
void Config::licenseCheckout()
{
    std::string qualnetLicensePath = qualnetHome + "\\license_dir";
    lm_job = ::licenseCheckout(qualnetLicensePath.c_str());
}
void Config::licenseCheckin()
{
    ::licenseCheckin(lm_job);
}
#else
void Config::licenseCheckout() {}
void Config::licenseCheckin() {}
#endif 


bool Config::parseCommandLine(int argc, const char* argv[])
{
    std::string cmd(argv[0]);
    std::string cmdDir;
    size_t pos = cmd.rfind("\\");
    if (pos == cmd.npos)
        cmdDir = fullpath(".");
    else
        cmdDir = fullpath(cmd.substr(0, pos));
    pos = cmdDir.rfind("\\bin");
    if (pos != cmdDir.npos)
        exeHome = cmdDir.substr(0, pos);
    qualnetHome = exeHome;

    int i = 1;
    while (i < argc)
    {
        const char* arg = argv[i];

        if (arg[0] == '-' || arg[0] == '/')
        {
            int used = processFlag(arg+1, argv+i+1);
            if (used)
                i += used;
            else
                return false;
        }
        else if (i+1 == argc)
        {
            scenarioDir = argv[i];
            size_t dir_end = scenarioDir.rfind(dirSep);
            if (dir_end == scenarioDir.npos)
                scenarioName = scenarioDir;
            else
                scenarioName = scenarioDir.substr(dir_end+1);
            std::string path = fullpath(scenarioDir);
            if (!path.empty())
            {
                scenarioDir = path;
            }
            scenarioPath = scenarioDir + dirSep + scenarioName;
            i++;
        }
        else
        {
            return false;
        }
    }
    if (allEntitiesHaveRadios && merge)
        return false;

    return true;
}

void Config::Usage(const char* program_name)
{
    std::cout << "QualNet HLA RPR-FOM 1.0 Extractor" << std::endl
         << std::endl
         << "Syntax:" << std::endl
         << std::endl
         << program_name
         << "  [options] base-filename" << std::endl
         << std::endl
         << "    (Default values in parentheses)" << std::endl
         << "    -h" << std::endl
         << "\tHelp" << std::endl
         << "    -d" << std::endl
         << "\tDebug mode" << std::endl
         << "    -f federation-name" << std::endl
         << "\tSet federation name (RPR-FOM)" << std::endl
         << "    -F FED-file" << std::endl
         << "\tSet FED file (RPR-FOM.fed)" << std::endl
         << "    -m" << std::endl
         << "\tMerge into existing .config file" << std::endl
         << "    -N nodeId" << std::endl
         << "\tSet initial nodeId (1; 1001 when merging)" << std::endl
         << "    -o" << std::endl
         << "\tIgnore RadioTransmitter objects, create one node/radio per entity"
            << std::endl
         << "    -r" << std::endl
         << "\tUse RPR FOM 0.5" << std::endl
         << "    -s number" << std::endl
         << "\tSet class A network number for new networks (60)" << std::endl
         << "    -t timeout" << std::endl
         << "\tSet number of seconds since last entity discovered before"
            " program exit (5)" << std::endl;
}

void Config::initializeScenarioParameters()
{
    setParameter(Parameter("EXPERIMENT-NAME", scenarioName));
    setParameter(Parameter("HLA-FEDERATION-NAME", federationName));
    setParameter(Parameter("HLA-FED-FILE-PATH", fedPath));
    setParameter(Parameter("HLA-FEDERATE-NAME", federateName));
    setParameter(Parameter("HLA-RPR-FOM-VERSION", rprVersion));

    setParameter(Parameter("CONFIG-FILE-PATH", scenarioName+".config"));
    setParameter(Parameter("NODE-POSITION-FILE", scenarioName+".nodes"));
    setParameter(Parameter("APP-CONFIG-FILE", scenarioName+".app"));
    setParameter(Parameter("ROUTER-MODEL-CONFIG-FILE", scenarioName+".router-models"));
    setParameter(Parameter("LINK-16-SLOT-FILE", scenarioName+".slot"));
    
    setParameter(Parameter("HLA-ENTITIES-FILE-PATH", scenarioName+".hla-entities"));
    setParameter(Parameter("HLA-RADIOS-FILE-PATH", scenarioName+".hla-radios"));
    setParameter(Parameter("HLA-NETWORKS-FILE-PATH", scenarioName+".hla-networks"));
    setParameter(Parameter("DIS-ENTITIES-FILE-PATH", scenarioName+".hla-entities"));
    setParameter(Parameter("DIS-RADIOS-FILE-PATH", scenarioName+".hla-radios"));
    setParameter(Parameter("DIS-NETWORKS-FILE-PATH", scenarioName+".hla-networks"));
    setParameter(Parameter("LINK-16-SLOT-FILE", scenarioName+".slot"));

    if (debugSetOnCmdLine)
        setParameter(Parameter("HLA-DEBUG",    debug?"YES":"NO"));
    else if (getParameter("HLA-DEBUG") == Parameter::unknownParameter)
        setParameter(Parameter("HLA-DEBUG",    debug?"YES":"NO"));

    setParameter(Parameter("NODE-PLACEMENT", "FILE"));
}

void Config::readParameterFiles()
{
#ifdef QT_CORE_LIB
    QFile builtin(":/builtin.hla");
    builtin.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = builtin.readAll();
    std::stringstream input(std::stringstream::in | std::stringstream::out);
    input << data.data();

    std::cout << "Setting default parameters" << std::endl << std::flush;
    readParameterFile(input);
#endif

    const char *p = getenv("HOMEPATH");
    if (!p)
        p = ".";
    std::string userHome(p);

    std::vector<std::string> files;
    findFilesByExtension("hla", qualnetHome+"\\gui\\devices", files);
    findFilesByExtension("hla", qualnetHome+"\\addons", files);
    findFilesByExtension("hla", userHome + "\\.qualnetUserDir", files);
    if (files.size())
        std::cout << "Reading parameter files"  << std::endl << std::flush;
    for (size_t i=0; i<files.size(); i++)
        readParameterFile(files[i]);

    initializeScenarioParameters();
}

void Config::readParameterFile(std::string filename)
{
    std::cout << "Reading parameter file " << filename << std::endl << std::flush;
#ifdef QT_CORE_LIB
    const char* str = filename.c_str();
    QString qstr(str);
    QFileInfo infile(qstr);

    QString apath = infile.absoluteFilePath();
    QString cpath = infile.canonicalFilePath();
#endif
    std::fstream in;
    in.open(filename.c_str(), std::fstream::in);
    readParameterFile(in);
    in.close();
}

void Config::readParameterFile(std::istream& in)
{
    static std::string whiteSpace(" \t");
    while (in.good())
    {
        std::string line;
        getline(in, line);
        if (line.empty())
            continue;

        size_t leadingWS = line.find_first_not_of(whiteSpace);
        if (leadingWS)
            line = line.substr(leadingWS);
        if (line.empty())
            continue;

        size_t startOfComment = line.find_first_of("#");
        if (startOfComment < line.npos)
            line = line.substr(0, startOfComment);
        if (line.empty())
            continue;

        size_t tailingWS = line.find_last_not_of(whiteSpace);
        if (tailingWS == line.npos)
            continue;
        line = line.substr(0, tailingWS+1);
        if (line.empty())
            continue;

        size_t endOfName = line.find_first_of(whiteSpace);
        if (endOfName == line.npos)
            continue;

        size_t startOfValue = line.find_first_not_of(whiteSpace, endOfName);
        if (startOfValue == line.npos)
            continue;

        std::string name = line.substr(0, endOfName);
        std::string value = line.substr(startOfValue, line.npos);
        if (name.find("NODE-ICON") == 0)
        {
            value = findIconFile(value);
        }
        setParameter(Parameter(name, value));
    }
}

void Config::readModelFiles()
{

#ifdef QT_CORE_LIB
    QFile builtin(":/builtin.router-models");
    builtin.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray data = builtin.readAll();
    std::stringstream input(std::stringstream::in | std::stringstream::out);
    input << data.data();
    std::cout << "Setting default Models" << std::endl << std::flush;
    readModelFile(input);
#endif

    const char* p = getenv("HOMEPATH");
    if (!p)
        p = ".";
    std::string userHome(p);

    std::vector<std::string> files;
    findFilesByExtension("device-models", qualnetHome+"\\gui\\devices", files);
    findFilesByExtension("router-models", qualnetHome+"\\gui\\devices", files);
    findFilesByExtension("device-models", qualnetHome+"\\addons", files);
    findFilesByExtension("router-models", qualnetHome+"\\addons", files);
    findFilesByExtension("device-models", userHome + "\\.qualnetUserDir", files);
    findFilesByExtension("router-models", userHome + "\\.qualnetUserDir", files);
    if (files.size())
        std::cout << "Reading Model files" << std::endl << std::flush;
    for (size_t i=0; i<files.size(); i++)
        readModelFile(files[i]);
}

void Config::readModelFile(std::string filename)
{
    std::cout << "Reading Model file " << filename << std::endl << std::flush;
    std::fstream in;
    in.open(filename.c_str(), std::fstream::in);
    readModelFile(in);
    in.close();
}

void Config::readModelFile(std::istream& in)
{
    static std::string whiteSpace(" \t");
    ParameterMap* currentModel = 0;
    while (in.good())
    {
        std::string line;
        getline(in, line);
        if (line.empty())
            continue;

        size_t leadingWS = line.find_first_not_of(whiteSpace);
        if (leadingWS)
            line = line.substr(leadingWS);
        if (line.empty())
            continue;

        size_t startOfComment = line.find_first_of("#");
        if (startOfComment < line.npos)
            line = line.substr(0, startOfComment);
        if (line.empty())
            continue;

        size_t tailingWS = line.find_last_not_of(whiteSpace);
        if (tailingWS == line.npos)
            continue;
        line = line.substr(0, tailingWS+1);
        if (line.empty())
            continue;

        size_t endOfName = line.find_first_of(whiteSpace);
        if (endOfName == line.npos)
            continue;

        size_t startOfValue = line.find_first_not_of(whiteSpace, endOfName);
        if (startOfValue == line.npos)
            continue;

        Parameter param(line.substr(0, endOfName), line.substr(startOfValue, line.npos));
        if (param.name == "ROUTER-MODEL")
        {
            std::map<std::string, ParameterMap*>::iterator it = models.find(param.value);
            if (it != models.end())
            {
                std::cout << "Updating Model " << param.value << std::endl;
                currentModel = it->second;
            }
            else
            {
                std::cout << "Loading Model " << param.value << std::endl;
                currentModel = new ParameterMap;
                std::pair<std::string, ParameterMap*> p(param.value, currentModel);
                models.insert(p);
            }
        }
        else if (param.name == "HLA-RADIO-SYSTEM-TYPE")
        {
            RadioTypeStruct radioType(param.value);
            radioTypeToRouterModel[radioType] = currentModel;
        }
        if (currentModel)
        {
            currentModel->setParameter(param);
        }
    }
}

ParameterMap* Config::getModelParameterList(std::string name)
{
    std::map<std::string, ParameterMap*>::iterator it = models.find(name);
    if (it != models.end())
        return it->second;
    else
        return 0;
}

ParameterMap* Config::getModelParameterList(RadioTypeStruct radioType)
{
    RadioTypeMapWithWildCards<ParameterMap*>::iterator it = radioTypeToRouterModel.matchWC(radioType);
    if (it != radioTypeToRouterModel.end())
        return it->second;
    else
        return getModelParameterList(getParameter("HLA-DEFAULT-ROUTER-MODEL").value);
}

std::vector<Parameter> Config::getParameterList(std::string name, matchType match)
{
    std::vector<Parameter> parameters;
    std::map<std::string, Parameter>::iterator it = scenarioParameters.begin();
    while (it != scenarioParameters.end())
    {
        bool matchFound = false;
        const std::string& key = it->first;
        switch( match )
        {
        case ANY:
            {
                size_t pos = key.find(name);
                if (pos != key.npos)
                    matchFound = true;
            break;
            }
        case PREFIX:
            {
                size_t pos = key.find(name);
                if (pos == 0)
                    matchFound = true;
            break;
            }
        case SUFFIX:
            {
                size_t pos = key.rfind(name);
                if (pos + name.size() == key.size())
                    matchFound = true;
            break;
            }
        default:
            break;
        }
        if (matchFound)
            parameters.push_back(it->second);
        it++;
    }
    return parameters;
}

const std::string& Config::getIconForType(const EntityTypeStruct& type)
{
    std::stringstream id;
    id << "NODE-ICON[";
    id << (unsigned int) type.entityKind << ".";
    id << (unsigned int) type.domain << ".";
    id << (unsigned int) type.countryCode << ".";
    id << (unsigned int) type.category << ".";
    id << (unsigned int) type.subcategory << ".";
    id << (unsigned int) type.specific << ".";
    id << (unsigned int) type.extra;
    id << "]";
    const Parameter& iconPath = getParameter(id.str());
    if (iconPath != Parameter::unknownParameter)
    {
        return iconPath.value;
    }
    else
    {
        return getParameter("NODE-ICON").value;
    }
}


bool fileIsReadable(const std::string& fileName)
{
    FILE* fp = fopen(fileName.c_str(), "r");
    if (!fp)
        return false;
    fclose(fp);
    return true;
}

std::string Config::getFileName(const std::string& path)
{
    size_t dir_end;
    size_t dir_end1 = path.rfind("\\");
    size_t dir_end2 = path.rfind("/");
    if (dir_end1 == path.npos && dir_end2 == path.npos)
    {
        return path;
    }
    else if (dir_end1 == path.npos)
    {
        dir_end = dir_end2;
    }
    else if (dir_end2 == path.npos)
    {
        dir_end = dir_end1;
    }
    else
    {
        dir_end = max(dir_end1, dir_end2);
    }
    return path.substr(dir_end+1);
}

const std::string Config::findIconFile(const std::string& iconName)
{
    // check if the files exists as is
    if (fileIsReadable(iconName))
        return iconName;

    std::string filename = getFileName(iconName);
    std::string path;

    // look in $QUALNET_HOME/gui/icons  and subdirs
    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+"devices"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+"military"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+"people"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+"app_icons"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    path = qualnetHome+dirSep+"gui"+dirSep+"icons"+dirSep+"3Dvisualizer"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    // look in $QUALNET_HOME/addons/hla/rprsynch/icons
    path = qualnetHome+dirSep+"addons"+dirSep+"hla"+dirSep+"rprsynch"+dirSep+"data"+dirSep+"icons"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    // look in $QUALNET_HOME/interfaces/hla/rprsynch/icons
    path = qualnetHome+dirSep+"interfaces"+dirSep+"hla"+dirSep+"rprsynch"+dirSep+"data"+dirSep+"icons"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    // look in ../icons
    path = ".."+dirSep+"icons"+dirSep+filename;
    if (fileIsReadable(path))
        return path;

    return std::string("");
}

std::ostream& SNT_HLA::operator<<(std::ostream& out, const Parameter& para)
{
    out << para.name;
    out << "\t\t";
    out << para.value;

    return out;
}


#ifdef _WIN32
#include <io.h>
#include <set>

void findFilesByExtension( const std::string extension, const std::string rootDir,
                          std::vector<std::string> &filenameList)
{
    std::string filespec = rootDir + "\\*";
    struct _finddata_t fileinfo;
    intptr_t findH = _findfirst(filespec.c_str(), &fileinfo);
    if (findH == -1)
        return;

    std::set<std::string> files;
    std::set<std::string> subDirs;
    int ret = 0;
    do {
        std::string filename(fileinfo.name);
        size_t pos = filename.rfind(std::string(".")+extension);
        if (fileinfo.attrib & _A_SUBDIR && filename != "." && filename != "..")
        {
            subDirs.insert(filename);
        }
        else if (pos != filename.npos && pos == filename.size() - (extension.size()+1))
        {
            files.insert(filename);
        }
        ret = _findnext(findH, &fileinfo);
    }     while (ret != -1);

    _findclose(findH);

    std::set<std::string>::iterator it;
    it = files.begin();
    while (it != files.end())
    {
        filenameList.push_back(rootDir + "\\" + *it);
        it++;
    }
    it = subDirs.begin();
    while (it != subDirs.end())
    {
        findFilesByExtension(extension, rootDir + "\\" + *it, filenameList);
        it++;
    }
}

#endif

