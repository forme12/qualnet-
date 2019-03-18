#pragma once

#include "AgStkCommUtil_v10.tlh"
#include "..\agi_interface_signal_impl.h"

class CAgiInterfaceSignalStk10 : public CAgiInterfaceSignalImpl<AgStkCommUtilLib::IAgStkCommUtilSignal>
{
public:
    CAgiInterfaceSignalStk10();
    CAgiInterfaceSignalStk10(const CAgiInterfaceSignalStk10& rhs);
    ~CAgiInterfaceSignalStk10();
};