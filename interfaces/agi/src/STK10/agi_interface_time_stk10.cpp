#include <sstream>
#include "agi_interface_time_stk10.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceTimeStk10::CAgiInterfaceTimeStk10()
{
}

CAgiInterfaceTimeStk10::CAgiInterfaceTimeStk10(clocktype time) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(time)
{
}

CAgiInterfaceTimeStk10::CAgiInterfaceTimeStk10(const CAgiInterfaceTimeStk10& rhs) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(rhs)
{
}

CAgiInterfaceTimeStk10::~CAgiInterfaceTimeStk10()
{
}

STDMETHODIMP CAgiInterfaceTimeStk10::Clone(IAgStkCommUtilTime** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilTime> newTime = new CAgiInterfaceTimeStk10(*this);
    return newTime.CopyTo(ppClone);
}

AgStkCommUtilLib::IAgStkCommUtilTime* CAgiInterfaceTimeStk10::ConstructNew(clocktype time)
{
    return new CAgiInterfaceTimeStk10(time);
}