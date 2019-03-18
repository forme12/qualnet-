#include <sstream>
#include "agi_interface_time_stk9.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceTimeStk9::CAgiInterfaceTimeStk9()
{
}

CAgiInterfaceTimeStk9::CAgiInterfaceTimeStk9(clocktype time) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(time)
{
}

CAgiInterfaceTimeStk9::CAgiInterfaceTimeStk9(const CAgiInterfaceTimeStk9& rhs) :
CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>(rhs)
{
}

CAgiInterfaceTimeStk9::~CAgiInterfaceTimeStk9()
{
}

STDMETHODIMP CAgiInterfaceTimeStk9::Clone(IAgStkCommUtilTime** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilTime> newTime = new CAgiInterfaceTimeStk9(*this);
    return newTime.CopyTo(ppClone);
}

AgStkCommUtilLib::IAgStkCommUtilTime* CAgiInterfaceTimeStk9::ConstructNew(clocktype time)
{
    return new CAgiInterfaceTimeStk9(time);
}