#ifndef _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD
#define _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD

#include <string>
#include <vector>

#include "shrek.h"

namespace shrek
{
    bool read_all_text(const std::string& utf8_filename, std::string& result);

    void discover_modules(ShrekHandle* shrek);
}

#endif // _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD
