#ifndef _SHREK_INTERPRETER_H_INCLUDE_GUARD
#define _SHREK_INTERPRETER_H_INCLUDE_GUARD

#include "shrek_types.h"

namespace shrek
{
    std::vector<ByteCode> interpret_code(const std::string& filename);
}

#endif // _SHREK_INTERPRETER_H_INCLUDE_GUARD
