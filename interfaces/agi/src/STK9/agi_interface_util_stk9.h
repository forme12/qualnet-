#pragma once
#include "..\agi_interface_util_impl.h"
#include "agi_interface_time_stk9.h"
#include "agi_interface_id_stk9.h"
#include "agi_interface_signal_stk9.h"
#include "agi_interface_mapping_stk9.h"

class CAgiInterfaceUtilStk9 : public CAgiInterfaceUtilImpl<CAgiInterfaceIDStk9, CAgiInterfaceTimeStk9, 
                                                           CAgiInterfaceSignalStk9, CAgiInterfaceMappingStk9>
{
public:
    CAgiInterfaceUtilStk9();
    ~CAgiInterfaceUtilStk9();
};