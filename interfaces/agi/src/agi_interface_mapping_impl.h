#pragma once

#include <fstream>
#include <map>
#include <list>
#include "agi_interface_smart_ptr.h"
#include "agi_interface_id_impl.h"
#include <algorithm>

#define STR_TOUPPER( s )(std::transform(s.begin(), s.end(), s.begin(), (int(*)(int))toupper))

template<class T>
class CAgiInterfaceMappingImpl : public T
{
public:
    CAgiInterfaceMappingImpl();
    virtual ~CAgiInterfaceMappingImpl();

    bool LoadMappingFile(const std::string& mappingFilePath);
    void SetStkScenarioFilePath(const std::string& stkScenarioPath);
    STDMETHOD(GetStkScenarioFilePath)(BSTR * pPath);
    STDMETHOD(GetTransmitAntennaPath)(AgStkCommUtilLib::IAgStkCommUtilId* pID, BSTR * pPath );
    STDMETHOD(GetReceiveAntennaPath)(AgStkCommUtilLib::IAgStkCommUtilId* pID, double * pRainOutagePercent, BSTR * pPath );

    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void __RPC_FAR *__RPC_FAR *ppvObject);

    bool IsSatellite(const long& nodeId) const;
    bool IsSatellite(const std::pair<long, int>& interfaceId) const;

private:
    void LoadInterface(long entityId, std::ifstream& mappingFile);
    void LoadSatelliteInterface(long entityId, std::ifstream& mappingFile);
    void LoadEntities(std::ifstream& mappingFile);
    void LoadEntity(std::ifstream& mappingFile);

    typedef std::pair<std::string, std::string> StkInterface;

    class StkQualNetInterfaceInfo
    {
    public:
        StkQualNetInterfaceInfo(StkInterface stkInterface, double rainOutagePercent) :
          m_rainOutagePercent(rainOutagePercent),
            m_stkInterface(stkInterface),
            m_isSatellite(false)
        {
        }

        StkQualNetInterfaceInfo(int channelIndex, StkInterface stkInterface, double rainOutagePercent) :
            m_rainOutagePercent(rainOutagePercent),
            m_isSatellite(true)
            {
                m_channelMap[channelIndex] = stkInterface;
            }

        StkInterface m_stkInterface;
        double m_rainOutagePercent;
        bool m_isSatellite;
        std::map<int, StkInterface> m_channelMap;
    };

    typedef std::pair<int, StkQualNetInterfaceInfo> StkQualnetInterface;
    typedef std::map<long, std::list<StkQualnetInterface> > EntityIdMap;
    typedef std::map<long, bool> EntitySatMap;
    typedef std::map<std::pair<long, int>, bool> InterfaceSatMap;

    EntityIdMap m_entityIdMap;
    EntitySatMap m_entitySatMap;
    InterfaceSatMap m_interfaceSatMap;

    std::string m_stkScenarioFilePath;

    unsigned int m_cRefs;
};

template<class T>
CAgiInterfaceMappingImpl<T>::CAgiInterfaceMappingImpl() :
m_cRefs(0)
{
}

template<class T>
CAgiInterfaceMappingImpl<T>::~CAgiInterfaceMappingImpl()
{
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceMappingImpl<T>::AddRef()
{
    m_cRefs++;
    return m_cRefs;
}

template<class T>
STDMETHODIMP_(ULONG) CAgiInterfaceMappingImpl<T>::Release()
{
    ULONG ulCount = --m_cRefs;
    if (!ulCount)
    {
        m_cRefs = -(LONG_MAX/2);
        delete this;
    }
    return ulCount;
}

template<class T>
HRESULT STDMETHODCALLTYPE CAgiInterfaceMappingImpl<T>::QueryInterface(
    /* [in] */ REFIID riid,
    /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
{
    if (IsEqualIID(riid, IID_IUnknown))
    {
        *ppvObject = static_cast<IUnknown*>(this);
        AddRef();
        return S_OK;
    }

    if (IsEqualIID(riid, __uuidof(AgStkCommUtilLib::IAgStkCommUtilEntityMapping)))
    {
        *ppvObject = static_cast<AgStkCommUtilLib::IAgStkCommUtilEntityMapping*>(this);
        AddRef();
        return S_OK;
    }

    *ppvObject = NULL;
    return E_NOINTERFACE;
}

template<class T>
STDMETHODIMP CAgiInterfaceMappingImpl<T>::GetStkScenarioFilePath(BSTR * pPath)
{
    _bstr_t stkScenarioPath = m_stkScenarioFilePath.c_str();
    *pPath = stkScenarioPath.Detach();
    return S_OK;
}

template<class T>
STDMETHODIMP CAgiInterfaceMappingImpl<T>::GetTransmitAntennaPath(AgStkCommUtilLib::IAgStkCommUtilId* pID, BSTR * pPath )
{
    HRESULT hr = E_FAIL;

    CAgiInterfaceSmartPtr<IAgiInterfaceID> pInterfaceID;
    if (SUCCEEDED(pID->QueryInterface(__uuidof(IAgiInterfaceID), (void**)&pInterfaceID)))
    {
        long nodeID = pInterfaceID->GetNodeID();
        int interfaceID = pInterfaceID->GetInterfaceID();
        int channelIndex = pInterfaceID->GetChannelIndex();

        EntityIdMap::iterator nodeIt = m_entityIdMap.find(nodeID);
        if (nodeIt != m_entityIdMap.end())
        {
            for (std::list<StkQualnetInterface>::iterator interfaceItr = nodeIt->second.begin();
                interfaceItr != nodeIt->second.end(); ++interfaceItr)
            {
                if (interfaceItr->first == interfaceID)
                {
                    if (interfaceItr->second.m_isSatellite)
                    {
                        std::map<int, StkInterface>::iterator itr = interfaceItr->second.m_channelMap.find(channelIndex);
                        if (itr != interfaceItr->second.m_channelMap.end())
                        {
                            _bstr_t path = itr->second.first.c_str();
                            *pPath = path.Detach();
                            hr = S_OK;
                        }
                        else if (interfaceItr->second.m_channelMap.size() > 0)
                        {
                            itr = interfaceItr->second.m_channelMap.begin();
                            _bstr_t path = itr->second.first.c_str();
                            *pPath = path.Detach();
                            hr = S_OK;
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                       _bstr_t path = interfaceItr->second.m_stkInterface.first.c_str();
                       *pPath = path.Detach();
                       hr = S_OK;
                    }
                    break;
                }
            }
        }
    }

    return hr;
}

template<class T>
STDMETHODIMP CAgiInterfaceMappingImpl<T>::GetReceiveAntennaPath(AgStkCommUtilLib::IAgStkCommUtilId* pID, 
                                                                double* pRainOutagePercent, BSTR* pPath )
{
    HRESULT hr = E_FAIL;

    CAgiInterfaceSmartPtr<IAgiInterfaceID> pInterfaceID;
    if (SUCCEEDED(pID->QueryInterface(__uuidof(IAgiInterfaceID), (void**)&pInterfaceID)))
    {
        long nodeID = pInterfaceID->GetNodeID();
        int interfaceID = pInterfaceID->GetInterfaceID();
        int channelIndex = pInterfaceID->GetChannelIndex();

        EntityIdMap::iterator nodeIt = m_entityIdMap.find(nodeID);
        if (nodeIt != m_entityIdMap.end())
        {
            for (std::list<StkQualnetInterface>::iterator interfaceItr = nodeIt->second.begin(); interfaceItr != nodeIt->second.end(); ++interfaceItr)
            {
                if (interfaceItr->first == interfaceID)
                {
                    if (interfaceItr->second.m_isSatellite)
                    {
                        *pRainOutagePercent = interfaceItr->second.m_rainOutagePercent;

                        std::map<int, StkInterface>::iterator itr = interfaceItr->second.m_channelMap.find(channelIndex);
                        if (itr != interfaceItr->second.m_channelMap.end())
                        {
                            _bstr_t path = itr->second.second.c_str();
                            *pPath = path.Detach();
                            hr = S_OK;
                        }
                        else if (interfaceItr->second.m_channelMap.size() > 0)
                        {
                            itr = interfaceItr->second.m_channelMap.begin();
                            _bstr_t path = itr->second.second.c_str();
                            *pPath = path.Detach();
                            hr = S_OK;
                        }
                        else
                        {
                            hr = E_FAIL;
                        }
                    }
                    else
                    {
                       *pRainOutagePercent = interfaceItr->second.m_rainOutagePercent;

                       _bstr_t path = interfaceItr->second.m_stkInterface.second.c_str();
                       *pPath = path.Detach();
                       hr = S_OK;
                    }
                    break;
                }
            }
        }
    }

    return hr;
}

template<class T>
void CAgiInterfaceMappingImpl<T>::SetStkScenarioFilePath(const std::string& stkScenarioPath)
{
    m_stkScenarioFilePath = stkScenarioPath;
}

template<class T>
bool CAgiInterfaceMappingImpl<T>::LoadMappingFile(const std::string& mappingFilePath)
{
    bool retVal = true;
    std::ifstream mappingFile(mappingFilePath.c_str());
    if (mappingFile)
    {
        std::string line;
        std::string token;

        while (!mappingFile.eof())
        {
            std::getline(mappingFile, line);
            if (!line.empty())
            {
                std::istringstream lineStream(line);
                lineStream.setf(std::ios::skipws);

                lineStream >> token;

                STR_TOUPPER(token);

                if (token == "BEGIN")
                {
                    lineStream >> token;

                    STR_TOUPPER(token);

                    if (token == "ENTITIES")
                    {
                        LoadEntities(mappingFile);
                    }
                }
            }
        }

        retVal = m_entityIdMap.size() > 0 ? true : false;
    }
    else
    {
        retVal = false;
    }

    return retVal;
}

template<class T>
void CAgiInterfaceMappingImpl<T>::LoadInterface(long entityId, std::ifstream& mappingFile)
{
    std::string token;
    std::string line;

    std::string xmtrPath;
    std::string rcvrPath;
    int qualnetInterfaceId;
    double rainOutagePercent = 0.0;

    std::getline(mappingFile, line);
    std::istringstream lineStream1(line);
    lineStream1.setf(std::ios::skipws);

    lineStream1 >> token;
    lineStream1 >> qualnetInterfaceId;

    std::getline(mappingFile, line);
    std::istringstream lineStream2(line);
    lineStream2.setf(std::ios::skipws);

    lineStream2 >> token;
    lineStream2 >> xmtrPath;

    std::getline(mappingFile, line);
    std::istringstream lineStream3(line);
    lineStream3.setf(std::ios::skipws);

    lineStream3 >> token;
    lineStream3 >> rcvrPath;

    std::getline(mappingFile, line);
    std::istringstream lineStream4(line);
    lineStream4.setf(std::ios::skipws);

    lineStream4 >> token;
    lineStream4 >> rainOutagePercent;

    StkInterface stkInterface = std::make_pair<std::string, std::string>(xmtrPath, rcvrPath);
    StkQualNetInterfaceInfo stkQualNetInterfaceInfo(stkInterface, rainOutagePercent);

    StkQualnetInterface stkQualnetInterface = std::make_pair<int, StkQualNetInterfaceInfo>(qualnetInterfaceId, stkQualNetInterfaceInfo);

    EntityIdMap::iterator it = m_entityIdMap.find(entityId);
    it->second.push_back(stkQualnetInterface);

    std::getline(mappingFile, line);
}

template<class T>
void CAgiInterfaceMappingImpl<T>::LoadSatelliteInterface(long entityId, std::ifstream& mappingFile)
{
    std::string token;
    std::string line;

    std::string xmtrPath;
    std::string rcvrPath;
    int qualnetInterfaceId;
    int channelIndex;
    double rainOutagePercent = 0.0;

    std::getline(mappingFile, line);
    std::istringstream lineStream0(line);
    lineStream0.setf(std::ios::skipws);

    lineStream0 >> token;
    lineStream0 >> channelIndex;

    std::getline(mappingFile, line);
    std::istringstream lineStream1(line);
    lineStream1.setf(std::ios::skipws);

    lineStream1 >> token;
    lineStream1 >> qualnetInterfaceId;

    std::getline(mappingFile, line);
    std::istringstream lineStream2(line);
    lineStream2.setf(std::ios::skipws);

    lineStream2 >> token;
    lineStream2 >> xmtrPath;

    std::getline(mappingFile, line);
    std::istringstream lineStream3(line);
    lineStream3.setf(std::ios::skipws);

    lineStream3 >> token;
    lineStream3 >> rcvrPath;

    std::getline(mappingFile, line);
    std::istringstream lineStream4(line);
    lineStream4.setf(std::ios::skipws);

    lineStream4 >> token;
    lineStream4 >> rainOutagePercent;

    EntityIdMap::iterator entityIt = m_entityIdMap.find(entityId);
    if (entityIt != m_entityIdMap.end())
    {
        StkInterface stkInterface = std::make_pair<std::string, std::string>(xmtrPath, rcvrPath);

        StkQualNetInterfaceInfo* pFoundInterfaceInfo = 0;
        for (std::list<StkQualnetInterface>::iterator interfaceItr = entityIt->second.begin();
            interfaceItr != entityIt->second.end(); ++interfaceItr)
        {
            if (interfaceItr->first == qualnetInterfaceId)
            {
                pFoundInterfaceInfo = &interfaceItr->second;
                break;
            }
        }

        if (pFoundInterfaceInfo)
        {
            pFoundInterfaceInfo->m_channelMap[channelIndex] = stkInterface;
        }
        else
        {
            StkQualNetInterfaceInfo stkQualNetInterfaceInfo(channelIndex, stkInterface, rainOutagePercent);
            StkQualnetInterface stkQualnetInterface =
                std::make_pair<int, StkQualNetInterfaceInfo>(qualnetInterfaceId, stkQualNetInterfaceInfo);
            entityIt->second.push_back(stkQualnetInterface);
        }

        m_interfaceSatMap[std::make_pair<long, int>(entityId, qualnetInterfaceId)] = true;
    }
    std::getline(mappingFile, line);
}

template<class T>
void CAgiInterfaceMappingImpl<T>::LoadEntities(std::ifstream& mappingFile)
{
    int entityId = 0;
    bool notDone = true;
    std::string line;
    std::string token;
    while (notDone)
    {
        std::getline(mappingFile, line);
        std::istringstream lineStream(line);
        lineStream.setf(std::ios::skipws);

        lineStream >> token;

        STR_TOUPPER(token);

        if (token == "END")
        {
            lineStream >> token;

            STR_TOUPPER(token);

            if (token == "ENTITIES")
                notDone = false;
        }
        else if (token == "BEGIN")
        {
            lineStream >> token;

            STR_TOUPPER(token);

            if (token == "ENTITY")
                LoadEntity(mappingFile);
        }
    }
}

template<class T>
void CAgiInterfaceMappingImpl<T>::LoadEntity(std::ifstream& mappingFile)
{
    long entityId = 0;
    std::string isSatellite;
    bool notDone = true;
    std::string line;
    std::string token;
    while (notDone)
    {
        std::getline(mappingFile, line);
        std::istringstream lineStream(line);
        lineStream.setf(std::ios::skipws);

        lineStream >> token;

        STR_TOUPPER(token);

        if (token == "END")
        {
            lineStream >> token;

            STR_TOUPPER(token);

            if (token == "ENTITY")
                notDone = false;
            else if (token == "INTERFACE")
                continue;
        }
        else if (token == "ENTITYID")
        {
            lineStream >> entityId;

            std::list<StkQualnetInterface> newList;
            m_entityIdMap.insert(std::make_pair<long, std::list<StkQualnetInterface> >(entityId, newList));
            m_entitySatMap.insert(std::make_pair<long, bool>(entityId, false));
        }
        else if (token == "ISSATELLITE")
        {
            lineStream >> isSatellite;

            STR_TOUPPER(isSatellite);

            if (isSatellite == "TRUE")
                m_entitySatMap[entityId] = true;
        }
        else if (token == "BEGIN")
        {
            lineStream >> token;

            STR_TOUPPER(token);

            if (token == "INTERFACE")
            {
                LoadInterface(entityId, mappingFile);
            }
            else if (token == "SATELLITEINTERFACE")
            {
                LoadSatelliteInterface(entityId, mappingFile);
            }
        }
    }
}

template<class T>
bool CAgiInterfaceMappingImpl<T>::IsSatellite(const long& nodeId) const
{
    bool retVal = false;

    EntitySatMap::const_iterator it = m_entitySatMap.find(nodeId);
    if (it != m_entitySatMap.end())
    {
        retVal = it->second;
    }

    return retVal;
}

template<class T>
bool CAgiInterfaceMappingImpl<T>::IsSatellite(const std::pair<long, int>& interfaceId) const
{
    bool retVal = false;

    InterfaceSatMap::const_iterator it = m_interfaceSatMap.find(interfaceId);
    if (it != m_interfaceSatMap.end())
    {
        retVal = it->second;
    }

    return retVal;
}