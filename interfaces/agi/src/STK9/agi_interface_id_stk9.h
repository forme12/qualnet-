#pragma once

#include "AgStkCommUtil_v9.tlh"
#include "..\agi_interface_id_impl.h"

class CAgiInterfaceIDStk9 : public CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>
{
public:
    CAgiInterfaceIDStk9();
    CAgiInterfaceIDStk9(const CAgiInterfaceIDStk9& rhs);
    ~CAgiInterfaceIDStk9();

    STDMETHOD(Clone)(AgStkCommUtilLib::IAgStkCommUtilId** ppClone);
};