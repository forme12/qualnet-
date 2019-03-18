#pragma once

#include "AgStkCommUtil_v9.tlh"
#include "..\agi_interface_signal_impl.h"

class CAgiInterfaceSignalStk9 : public CAgiInterfaceSignalImpl<AgStkCommUtilLib::IAgStkCommUtilSignal>
{
public:
    CAgiInterfaceSignalStk9();
    CAgiInterfaceSignalStk9(const CAgiInterfaceSignalStk9& rhs);
    ~CAgiInterfaceSignalStk9();
};