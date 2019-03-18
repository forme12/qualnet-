#include "agi_interface_id_stk10.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceIDStk10::CAgiInterfaceIDStk10()
{
}

CAgiInterfaceIDStk10::CAgiInterfaceIDStk10(const CAgiInterfaceIDStk10& rhs) :
CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>(rhs)
{
}

CAgiInterfaceIDStk10::~CAgiInterfaceIDStk10()
{
}

STDMETHODIMP CAgiInterfaceIDStk10::Clone(IAgStkCommUtilId** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilId> newID = new CAgiInterfaceIDStk10(*this);
    return newID.CopyTo(ppClone);
}