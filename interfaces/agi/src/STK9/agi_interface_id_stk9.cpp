#include "agi_interface_id_stk9.h"

using namespace AgStkCommUtilLib;

CAgiInterfaceIDStk9::CAgiInterfaceIDStk9()
{
}

CAgiInterfaceIDStk9::CAgiInterfaceIDStk9(const CAgiInterfaceIDStk9& rhs) :
CAgiInterfaceIDImpl<AgStkCommUtilLib::IAgStkCommUtilId>(rhs)
{
}

CAgiInterfaceIDStk9::~CAgiInterfaceIDStk9()
{
}

STDMETHODIMP CAgiInterfaceIDStk9::Clone(IAgStkCommUtilId** ppClone)
{
    CAgiInterfaceSmartPtr<IAgStkCommUtilId> newID = new CAgiInterfaceIDStk9(*this);
    return newID.CopyTo(ppClone);
}
