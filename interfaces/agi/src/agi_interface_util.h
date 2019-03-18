#pragma once
#include "clock.h"
#include <utility>
#include <windows.h>

#define DEBUGBREAK 0

#define NodeInterfacePair std::pair<long, int>

struct PropPathProfile;
class IAgiInterfaceUtil;

class CAgiInterfaceUtil
{
public:
    static CAgiInterfaceUtil& GetInstance();

    enum StkVersion
    {
        eStk9 = 0,
        eStk10,
    };

    struct ComputeRequest
    {
        clocktype time_nanoseconds;
        NodeInterfacePair xmtrId;
        NodeInterfacePair rcvrId;
        int channelIndex;
        double frequency_hertz;
        double power_dBm;
        double dataRate_bps;
    };

    struct AzEl
    {
        double azimuth_degrees;
        double elevation_degrees;
    };

    struct LLA
    {
        double latitude_degrees;
        double longitude_degrees;
        double altitude_meters;
    };

    ~CAgiInterfaceUtil();

    void Initialize(StkVersion stkVersion, const bool& agiVerParamFound);
    bool ComputeLink(const ComputeRequest& req, PropPathProfile* pPathProfile);
    void ComputeReceiveGain(const ComputeRequest& req, double& gain_dB);
    bool ComputeNodePosition(long nodeId, clocktype time, LLA& pos);
    bool ComputeNodeOrientation(long nodeId, clocktype time, AzEl& orient);

private:
    CAgiInterfaceUtil();
    IAgiInterfaceUtil* m_pUtilImpl;
};