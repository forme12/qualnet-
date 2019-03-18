#pragma once

#include "AgStkCommUtil_v10.tlh"
#include "..\agi_interface_mapping_impl.h"

class CAgiInterfaceMappingStk10 : public CAgiInterfaceMappingImpl<AgStkCommUtilLib::IAgStkCommUtilEntityMapping>
{
public:
    CAgiInterfaceMappingStk10();
    ~CAgiInterfaceMappingStk10();
};