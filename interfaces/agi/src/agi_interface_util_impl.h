#pragma once
#include <comutil.h>
#include <string>
#include <direct.h>
#include "agi_interface_smart_ptr.h"
#include "agi_common_interfaces.h"

#define RAD2DEG 57.2957795130823208767
#define MCONVTODB(x) ((x)>0.0?10.0*log10(x): -500.0)
#define MCONVFROMDB(x) (pow(10.0, (x)/10.0))

namespace AgStkCommUtilLib
{
    class IAgStkCommUtilFactory;
    class AgStkCommUtilFactory;
};
struct PropPathProfile;

template<class ID, class TIME, class SIGNAL, class MAPPING>
class CAgiInterfaceUtilImpl : public IAgiInterfaceUtil
{
public:
    CAgiInterfaceUtilImpl();
    virtual ~CAgiInterfaceUtilImpl();

    virtual bool ConstructFactory();
    virtual void Initialize();
    virtual bool ComputeLink(const CAgiInterfaceUtil::ComputeRequest& req, PropPathProfile* pPathProfile);
    virtual void ComputeReceiveGain(const CAgiInterfaceUtil::ComputeRequest& req, double& gain_dB);
    virtual bool ComputeNodePosition(long nodeId, clocktype time, CAgiInterfaceUtil::LLA& pos);
    virtual bool ComputeNodeOrientation(long nodeId, clocktype time, CAgiInterfaceUtil::AzEl& orient);

private:
    ID* m_xmtrID;
    ID* m_rcvrID;
    TIME* m_currentTime;
    SIGNAL* m_signal;
    void ReportError(const std::string& errorMsg);

    bool m_initialized;
    AgStkCommUtilLib::IAgStkCommUtilFactory* m_pStkCommUtilFactory;
    MAPPING* m_pMappingObject;
};

static bool lTryGetErrorMsg(IUnknown* pUnk, const IID& riid, std::string& errorMsg)
{
    CAgiInterfaceSmartPtr<ISupportErrorInfo> pISER;
    if (SUCCEEDED(pUnk->QueryInterface(__uuidof(ISupportErrorInfo), (void**)&pISER)))
    {
        if (SUCCEEDED(pISER->InterfaceSupportsErrorInfo(riid)))
        {
            CAgiInterfaceSmartPtr<IErrorInfo> pEI;
            if (SUCCEEDED(GetErrorInfo(NULL, &pEI)))
            {
                _bstr_t msg;
                pEI->GetDescription(msg.GetAddress());

                errorMsg = std::string((char*)msg);
                return true;
            }
        }
    }

    return false;
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::CAgiInterfaceUtilImpl() :
m_initialized(false),
m_pStkCommUtilFactory(0),
m_pMappingObject(0)
{
    m_currentTime = new TIME();
    m_currentTime->AddRef();

    m_xmtrID = new ID();
    m_xmtrID->AddRef();

    m_rcvrID = new ID();
    m_rcvrID->AddRef();

    m_signal = new SIGNAL();
    m_signal->AddRef();
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::~CAgiInterfaceUtilImpl()
{
    m_currentTime->Release();
    m_xmtrID->Release();
    m_rcvrID->Release();
    m_signal->Release();

    if (m_pStkCommUtilFactory)
       m_pStkCommUtilFactory->Release();

    if (m_pMappingObject)
       m_pMappingObject->Release();
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
void CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ReportError(const std::string& errorMsg)
{
    ERROR_ReportError(errorMsg.c_str());
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
bool CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ConstructFactory()
{
    bool result = true;

    CoInitialize(NULL);

    if (FAILED(CoCreateInstance(__uuidof(AgStkCommUtilLib::AgStkCommUtilFactory), NULL, CLSCTX_ALL,
                               __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), (void**)&m_pStkCommUtilFactory)))
    {
        result = false;
    }

    return result;
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
void CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::Initialize()
{
    if (!m_initialized && m_pStkCommUtilFactory)
    {
        std::string errorMsg;
        char curDir[_MAX_PATH];
        char mapFileSearchPath[_MAX_PATH];
        char mapFilePath[_MAX_PATH];

        /* Get the current working directory: */
        if (_getcwd(curDir, _MAX_PATH) == NULL)
        {
            ReportError("QualNet Interface Initialization Error:  Unable to determine the current working directory.");
        }
        else
        {
            strcpy(mapFileSearchPath, curDir);
            strcat(mapFileSearchPath, "\\*.qimap");

            //Look in the current directory for the qulanet interface mapping file.
            WIN32_FIND_DATA info;
            HANDLE h = FindFirstFile(mapFileSearchPath, &info);
            if (h == INVALID_HANDLE_VALUE)
            {
                ReportError("QualNet Interface Initialization Error:  Unable to find the qualNet interface mapping file in the current directory (*.qimap).");
            }

            sprintf(mapFilePath, "%s\\%s", curDir, info.cFileName);

            m_pMappingObject = new MAPPING();
            m_pMappingObject->AddRef();

            if (!m_pMappingObject->LoadMappingFile(mapFilePath))
            {
                errorMsg = "Mapping file \"";
                errorMsg += std::string(mapFilePath);
                errorMsg += "\" load failed.";
                ReportError("QualNet Interface Initialization Error:  " + errorMsg);
            }

            char vdfFileSearchPath[_MAX_PATH];
            char vdfFilePath[_MAX_PATH];

            strcpy(vdfFileSearchPath, curDir);
            strcat(vdfFileSearchPath, "\\*.vdf");

            //Look in the current directory for the qulanet interface mapping file.
            h = FindFirstFile(vdfFileSearchPath, &info);
            if (h == INVALID_HANDLE_VALUE)
            {
                ReportError("QualNet Interface Initialization Error:  Unable to find the stk scenario file in the current directory (*.vdf).");
            }

            sprintf(vdfFilePath, "%s\\%s", curDir, info.cFileName);
            m_pMappingObject->SetStkScenarioFilePath(vdfFilePath);

            if (FAILED(m_pStkCommUtilFactory->Initialize(m_pMappingObject)))
            {
                if (lTryGetErrorMsg(m_pStkCommUtilFactory, __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), errorMsg))
                {
                    ReportError("QualNet Interface Initialization Error:  " + errorMsg);
                }
                else
                {
                    ReportError("QualNet Interface Initialization Error:  Unable to initialize the STK Comm Utility Factory.");
                }
            }

            char curDirAfter[_MAX_PATH];
            if (_getcwd(curDirAfter, _MAX_PATH) != NULL)
            {
                std::string sCurDirBefore = curDir;
                std::string sCurDirAfter = curDirAfter;
                if (sCurDirBefore != sCurDirAfter)
                    ReportError("QualNet Interface Initialization Error:  The current working directory was changed during initialization.");
            }
        }

        m_initialized = true;
    }
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
bool CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ComputeLink(const CAgiInterfaceUtil::ComputeRequest& req,
                                                                                                PropPathProfile* pPathProfile)
{
    bool retVal = false;

    if (pPathProfile)
    {
        int xmtInterfaceIndex = req.xmtrId.second;

        //The QualNet advanced satellite model has a single interface
        //so we will determine whether or not the xmtr nodeId is a satellite
        //and then use interface index 1 instead which is mapped to
        //to the stk transmit antenna
        if (m_pMappingObject->IsSatellite(req.xmtrId.first))
        {
            xmtInterfaceIndex = 1;
        }

        m_xmtrID->SetInfo(req.xmtrId.first, xmtInterfaceIndex, req.channelIndex,
                          m_pMappingObject->IsSatellite(std::make_pair<long, int>(req.xmtrId.first, xmtInterfaceIndex)));
        m_rcvrID->SetInfo(req.rcvrId.first, req.rcvrId.second, req.channelIndex,
                          m_pMappingObject->IsSatellite(std::make_pair<long, int>(req.rcvrId.first, req.rcvrId.second)));

        CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilLink> pLink;
        if (SUCCEEDED(m_pStkCommUtilFactory->GetLink(m_xmtrID, m_rcvrID, &pLink)))
        {
            CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilLinkResult> pResult;
            double dPowerMilliWatts = MCONVFROMDB(req.power_dBm);
            m_currentTime->SetTime(req.time_nanoseconds);

            m_signal->put_FrequencyHertz(req.frequency_hertz);
            m_signal->put_PowerWatts(dPowerMilliWatts/1000.0);

            if (FAILED(pLink->ComputeToReceive(m_currentTime, m_signal, &pResult)))
            {
                char errorMsg[1024];
                sprintf(errorMsg, "QualNet Interface Error:  Unknown error while computing the receive power from (node,interface) (%d, %d) to (%d, %d)",
                        req.xmtrId.first, req.xmtrId.second, req.rcvrId.first, req.rcvrId.second);
                ReportError(errorMsg);
            }

            VARIANT_BOOL hasAccess = VARIANT_FALSE;
            pResult->get_AccessSatisfied(&hasAccess);
            if (hasAccess == VARIANT_TRUE)
            {
                double rip = 0.0;
                pResult->get_ReceivedPowerIsotropicWatts(&rip);
                pPathProfile->rxPower_dBm = MCONVTODB(rip * 1000.0);

                double delay = 0;
                pResult->get_DelaySeconds(&delay);
                pPathProfile->propDelay = (clocktype)(delay * 1.0e9);

                pResult->get_ReceivedFrequencyHertz(&pPathProfile->rxFrequency);
                pResult->get_RangeMeters(&pPathProfile->distance);
                pResult->get_PathLossUnits(&pPathProfile->pathloss_dB);
                pPathProfile->pathloss_dB = MCONVTODB(pPathProfile->pathloss_dB);

                CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilOrientAzEl> pOrientResult;

                double azimuth_radians, elevation_radians;
                pResult->get_TransmitLinkDirection(&pOrientResult);
                pOrientResult->get_AzimuthRadians(&azimuth_radians);
                pPathProfile->txDOA.azimuth = (OrientationType)(azimuth_radians * RAD2DEG);
                pOrientResult->get_ElevationRadians(&elevation_radians);
                pPathProfile->txDOA.elevation = (OrientationType)(elevation_radians * RAD2DEG);

                pResult->get_ReceiveLinkDirection(&pOrientResult);
                pOrientResult->get_AzimuthRadians(&azimuth_radians);
                pPathProfile->rxDOA.azimuth = (OrientationType)(azimuth_radians * RAD2DEG);
                pOrientResult->get_ElevationRadians(&elevation_radians);
                pPathProfile->rxDOA.elevation = (OrientationType)(elevation_radians * RAD2DEG);

                CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilPosition> pPosResult;
                CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilLatLonAlt> pLLA;

                double lat_radians, lon_radians;
                pResult->get_TransmitPos(&pPosResult);
                pPosResult->get_LatLonAlt(&pLLA);
                pLLA->get_LatitudeRadians(&lat_radians);
                pPathProfile->fromPosition.latlonalt.latitude = (CoordinateType)(lat_radians * RAD2DEG);
                pLLA->get_LongitudeRadians(&lon_radians);
                pPathProfile->fromPosition.latlonalt.longitude = (CoordinateType)(lon_radians * RAD2DEG);
                pLLA->get_AltitudeMeters(&pPathProfile->fromPosition.latlonalt.altitude);
                pPathProfile->fromPosition.type = GEODETIC;

                pResult->get_ReceivePos(&pPosResult);
                pPosResult->get_LatLonAlt(&pLLA);
                pLLA->get_LatitudeRadians(&lat_radians);
                pPathProfile->toPosition.latlonalt.latitude = (CoordinateType)(lat_radians * RAD2DEG);
                pLLA->get_LongitudeRadians(&lon_radians);
                pPathProfile->toPosition.latlonalt.longitude = (CoordinateType)(lon_radians * RAD2DEG);
                pLLA->get_AltitudeMeters(&pPathProfile->toPosition.latlonalt.altitude);
                pPathProfile->toPosition.type = GEODETIC;

                pPathProfile->fading_dB = 0.0;
                pPathProfile->channelReal = 0.0;
                pPathProfile->channelImag = 0.0;
                pPathProfile->weatherPathloss_dB = 0.0;

                retVal = true;
            }
        }
        else
        {
            std::string sErrorMsg;
            if (lTryGetErrorMsg(m_pStkCommUtilFactory, __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), sErrorMsg))
            {
                std::string erroMsg = "QualNet Interface Error:  " + sErrorMsg;
                ReportError(sErrorMsg);
            }
            else
            {
                char errorMsg[1024];
                sprintf(errorMsg, "QualNet Interface Error:  Unable to construct a communications link between (node,interface) (%d, %d) and (%d, %d)",
                        req.xmtrId.first, req.xmtrId.second, req.rcvrId.first, req.rcvrId.second);
                ReportError(errorMsg);
            }
        }
    }
    return retVal;
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
void CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ComputeReceiveGain(const CAgiInterfaceUtil::ComputeRequest& req, double& gain_dB)
{
    gain_dB = 0.0;

    m_xmtrID->SetInfo(req.xmtrId.first, req.xmtrId.second, req.channelIndex,
                      m_pMappingObject->IsSatellite(std::make_pair<long, int>(req.xmtrId.first, req.xmtrId.second)));
    m_rcvrID->SetInfo(req.rcvrId.first, req.rcvrId.second, req.channelIndex,
                      m_pMappingObject->IsSatellite(std::make_pair<long, int>(req.rcvrId.first, req.rcvrId.second)));

    CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilLink> pLink;
    if (SUCCEEDED(m_pStkCommUtilFactory->GetLink(m_xmtrID, m_rcvrID, &pLink)))
    {
        m_currentTime->SetTime(req.time_nanoseconds);
        m_signal->put_FrequencyHertz(req.frequency_hertz);

        double gainLinear = 0.0;
        if (FAILED(pLink->ComputeReceiveGain(m_currentTime, m_signal, &gainLinear)))
        {
            char errorMsg[1024];
            sprintf(errorMsg, "QualNet Interface Error:  Unknown error while computing the receive gain from (node,interface) (%d, %d) to (%d, %d)",
                    req.xmtrId.first, req.xmtrId.second, req.rcvrId.first, req.rcvrId.second);
            ReportError(errorMsg);
       }

        gain_dB = MCONVTODB(gainLinear);
    }
    else
    {
        std::string sErrorMsg;
        if (lTryGetErrorMsg(m_pStkCommUtilFactory, __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), sErrorMsg))
        {
            std::string erroMsg = "QualNet Interface Error:  " + sErrorMsg;
            ReportError(sErrorMsg);
        }
        else
        {
            char errorMsg[1024];
            sprintf(errorMsg, "QualNet Interface Error:  Unable to construct a communications link between (node,interface) (%d, %d) and (%d, %d)",
                    req.xmtrId.first, req.xmtrId.second, req.rcvrId.first, req.rcvrId.second);
            ReportError(errorMsg);
        }
    }
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
bool CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ComputeNodePosition(long nodeId, clocktype time,
                                                                                                        CAgiInterfaceUtil::LLA& pos)
{
    bool retVal = false;

    pos.latitude_degrees = pos.longitude_degrees = pos.altitude_meters = 0.0;

    m_xmtrID->SetInfo(nodeId, 0, 0, false);

    CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilObject> pObj;
    if (SUCCEEDED(m_pStkCommUtilFactory->GetObject(m_xmtrID, &pObj)))
    {
        m_currentTime->SetTime(time);

        CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilPosition> pPosResult;
        if (SUCCEEDED(pObj->ComputePosition(m_currentTime, &pPosResult)))
        {
            CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilLatLonAlt> pLLA;
            pPosResult->get_LatLonAlt(&pLLA);
            pLLA->get_LatitudeRadians(&pos.latitude_degrees);
            pos.latitude_degrees *= RAD2DEG;
            pLLA->get_LongitudeRadians(&pos.longitude_degrees);
            pos.longitude_degrees *= RAD2DEG;
            pLLA->get_AltitudeMeters(&pos.altitude_meters);
            retVal = true;
        }
    }
    else
    {
        std::string sErrorMsg;
        if (lTryGetErrorMsg(m_pStkCommUtilFactory, __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), sErrorMsg))
        {
            std::string erroMsg = "QualNet Interface Error:  " + sErrorMsg;
            ReportError(sErrorMsg);
        }
        else
        {
            char errorMsg[1024];
            sprintf(errorMsg, "QualNet Interface Error:  Unable to construct an object for Node ID = %d", nodeId);
            ReportError(errorMsg);
        }
    }
    return retVal;
}

template<class ID, class TIME, class SIGNAL, class MAPPING>
bool CAgiInterfaceUtilImpl<ID, TIME, SIGNAL, MAPPING>::ComputeNodeOrientation(long nodeId, clocktype time,
                                                                                                           CAgiInterfaceUtil::AzEl& orient)
{
    bool retVal = false;

    orient.azimuth_degrees = orient.elevation_degrees = 0.0;

    m_xmtrID->SetInfo(nodeId, 0, 0, false);

    CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilObject> pObj;
    if (SUCCEEDED(m_pStkCommUtilFactory->GetObject(m_xmtrID, &pObj)))
    {
        m_currentTime->SetTime(time);

        CAgiInterfaceSmartPtr<AgStkCommUtilLib::IAgStkCommUtilOrientAzEl> pOrientResult;
        if (SUCCEEDED(pObj->ComputeOrientation(m_currentTime, &pOrientResult)))
        {
            pOrientResult->get_AzimuthRadians(&orient.azimuth_degrees);
            orient.azimuth_degrees *= RAD2DEG;
            pOrientResult->get_ElevationRadians(&orient.elevation_degrees);
            orient.elevation_degrees *= RAD2DEG;
            retVal = true;
        }
    }
    else
    {
        std::string sErrorMsg;
        if (lTryGetErrorMsg(m_pStkCommUtilFactory, __uuidof(AgStkCommUtilLib::IAgStkCommUtilFactory), sErrorMsg))
        {
            std::string erroMsg = "QualNet Interface Error:  " + sErrorMsg;
            ReportError(sErrorMsg);
        }
        else
        {
            char errorMsg[1024];
            sprintf(errorMsg, "QualNet Interface Error:  Unable to construct an object for Node ID = %d", nodeId);
            ReportError(errorMsg);
        }
    }
    return retVal;
}
