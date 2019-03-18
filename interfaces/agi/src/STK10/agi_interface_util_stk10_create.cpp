#include "agi_interface_util_stk10_create.h"
#include "agi_interface_util_stk10.h"

IAgiInterfaceUtil* CreateSTK10Util()
{
    return new CAgiInterfaceUtilStk10();
}