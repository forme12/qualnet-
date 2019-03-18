#pragma once
#include "propagation.h"
#include "agi_interface_util.h"

class IAgiInterfaceUtil
{
public:
    virtual ~IAgiInterfaceUtil(){}
    virtual bool ConstructFactory() = 0;
    virtual void Initialize() = 0;
    virtual bool ComputeLink(const CAgiInterfaceUtil::ComputeRequest& req, PropPathProfile* pPathProfile) = 0;
    virtual void ComputeReceiveGain(const CAgiInterfaceUtil::ComputeRequest& req, double& gain_dB) = 0;
    virtual bool ComputeNodePosition(long nodeId, clocktype time, CAgiInterfaceUtil::LLA& pos) = 0;
    virtual bool ComputeNodeOrientation(long nodeId, clocktype time, CAgiInterfaceUtil::AzEl& orient) = 0;
};