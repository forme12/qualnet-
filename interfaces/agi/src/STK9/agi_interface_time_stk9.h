#pragma once

#include "AgStkCommUtil_v9.tlh"
#include "..\agi_interface_time_impl.h"

class CAgiInterfaceTimeStk9 : public CAgiInterfaceTimeImpl<AgStkCommUtilLib::IAgStkCommUtilTime>
{
public:
    CAgiInterfaceTimeStk9();
    CAgiInterfaceTimeStk9(clocktype time);
    CAgiInterfaceTimeStk9(const CAgiInterfaceTimeStk9& rhs);
    ~CAgiInterfaceTimeStk9();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilTime** ppClone);

protected:
    virtual AgStkCommUtilLib::IAgStkCommUtilTime* ConstructNew(clocktype time);
};