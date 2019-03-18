#pragma once

#include "AgStkCommUtil_v9.tlh"
#include "..\agi_interface_mapping_impl.h"

class CAgiInterfaceMappingStk9 : public CAgiInterfaceMappingImpl<AgStkCommUtilLib::IAgStkCommUtilEntityMapping>
{
public:
    CAgiInterfaceMappingStk9();
    ~CAgiInterfaceMappingStk9();
};