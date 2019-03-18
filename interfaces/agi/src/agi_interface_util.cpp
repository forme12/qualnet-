#include <direct.h>
#include <math.h>
#include <windows.h>
#include "clock.h"
#include "qualnet_error.h"
#include "agi_interface_smart_ptr.h"
#include "propagation.h"
#include "agi_interface_util.h"
#include ".\STK9\agi_interface_util_stk9_create.h"
#include ".\STK10\agi_interface_util_stk10_create.h"

CAgiInterfaceUtil& CAgiInterfaceUtil::GetInstance()
{
    static CAgiInterfaceUtil theSingleton;
    return theSingleton;
}

CAgiInterfaceUtil::CAgiInterfaceUtil() :
m_pUtilImpl(0)
{
}

CAgiInterfaceUtil::~CAgiInterfaceUtil()
{
    if (m_pUtilImpl)
        delete m_pUtilImpl;
}

void CAgiInterfaceUtil::Initialize(StkVersion stkVersion, const bool& agiVerParamFound)
{
    if (stkVersion == eStk10)
    {
        m_pUtilImpl = CreateSTK10Util();
        if (!m_pUtilImpl->ConstructFactory())
        {
            ERROR_ReportError("QualNet Interface Initialization Error:  Configuration file parameter AGI-STK-VERSION has a value of 10 but the STK10 installation can not be found.");
        }
    }
    else
    {
        m_pUtilImpl = CreateSTK9Util();
        if (!m_pUtilImpl->ConstructFactory())
        {
            IAgiInterfaceUtil* pUtilImpl10 = CreateSTK10Util();
            bool factoryConstructed = pUtilImpl10->ConstructFactory();
            delete pUtilImpl10;
            if (factoryConstructed)
            {
                if (agiVerParamFound)
                {
                    ERROR_ReportError("QualNet Interface Initialization Error:  Configuration file parameter AGI-STK-VERSION has a value of 9 but the STK9 installation was not found.  However the STK10 installation was found.  In order to run against STK10, change the AGI-STK-VERSION value to 10.");
                }
                else
                {
                    ERROR_ReportError("QualNet Interface Initialization Error:  The STK9 installation was not found.  However the STK10 installation was found.  In order to run against STK10, add the line \"AGI-STK-VERSION 10\" to the configuration file.");
                }
            }
            else
            {
                if (agiVerParamFound)
                {
                    ERROR_ReportError("QualNet Interface Initialization Error:  Configuration file parameter AGI-STK-VERSION has a value of 9 but no compatible STK installations have been found.");
                }
                else
                {
                    ERROR_ReportError("QualNet Interface Initialization Error:  No compatible STK installations have been found.");
                }
            }
        }
    }

    m_pUtilImpl->Initialize();
}

bool CAgiInterfaceUtil::ComputeLink(const ComputeRequest& req, PropPathProfile* pPathProfile)
{
    return m_pUtilImpl->ComputeLink(req, pPathProfile);
}

void CAgiInterfaceUtil::ComputeReceiveGain(const ComputeRequest& req, double& gain_dB)
{
    m_pUtilImpl->ComputeReceiveGain(req, gain_dB);
}

bool CAgiInterfaceUtil::ComputeNodePosition(long nodeId, clocktype time, LLA& pos)
{
    return m_pUtilImpl->ComputeNodePosition(nodeId, time, pos);
}

bool CAgiInterfaceUtil::ComputeNodeOrientation(long nodeId, clocktype time, AzEl& orient)
{
    return m_pUtilImpl->ComputeNodeOrientation(nodeId, time, orient);
}
