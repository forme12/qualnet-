#pragma once

#include "AgStkCommUtil_v10.tlh"
#include "..\agi_interface_time_impl.h"

class CAgiInterfaceTimeStk10 : public CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>
{
public:
    CAgiInterfaceTimeStk10();
    CAgiInterfaceTimeStk10(clocktype time);
    CAgiInterfaceTimeStk10(const CAgiInterfaceTimeStk10& rhs);
    ~CAgiInterfaceTimeStk10();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilTime** ppClone);

protected:
    virtual AgStkCommUtilLib::IAgStkCommUtilTime* ConstructNew(clocktype time);
};