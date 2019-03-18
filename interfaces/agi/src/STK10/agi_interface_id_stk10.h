#pragma once

#include "AgStkCommUtil_v10.tlh"
#include "..\agi_interface_id_impl.h"

class CAgiInterfaceIDStk10 : public CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>
{
public:
    CAgiInterfaceIDStk10();
    CAgiInterfaceIDStk10(const CAgiInterfaceIDStk10& rhs);
    ~CAgiInterfaceIDStk10();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilId** ppClone);
};