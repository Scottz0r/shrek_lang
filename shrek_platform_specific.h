#ifndef _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD
#define _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD

#include <string>
#include <vector>

namespace shrek
{
    bool read_all_text(const std::string& utf8_filename, std::string& result);
}

#endif // _SHREK_PLATFORM_SPECIFIC_H_INCLUDE_GUARD
