#include "agi_interface_util_stk9_create.h"
#include "agi_interface_util_stk9.h"

IAgiInterfaceUtil* CreateSTK9Util()
{
    return new CAgiInterfaceUtilStk9();
}