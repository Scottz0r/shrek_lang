#ifndef _SHREK_RUNTIME_H_INCLUDE_GUARD
#define _SHREK_RUNTIME_H_INCLUDE_GUARD

#include "shrek_types.h"

namespace shrek
{
    void execute(const std::vector<ByteCode>& code);
}

#endif // !_SHREK_RUNTIME_H_INCLUDE_GUARD
