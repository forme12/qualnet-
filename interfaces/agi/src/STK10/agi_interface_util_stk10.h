#pragma once
#include "..\agi_interface_util_impl.h"
#include "agi_interface_time_stk10.h"
#include "agi_interface_id_stk10.h"
#include "agi_interface_signal_stk10.h"
#include "agi_interface_mapping_stk10.h"

class CAgiInterfaceUtilStk10 : public CAgiInterfaceUtilImpl<CAgiInterfaceIDStk10, CAgiInterfaceTimeStk10, 
                                                            CAgiInterfaceSignalStk10, CAgiInterfaceMappingStk10>
{
public:
    CAgiInterfaceUtilStk10();
    ~CAgiInterfaceUtilStk10();
};