#ifndef _SHREK_OPTIMIZER_H_INCLUDE_GUARD
#define _SHREK_OPTIMIZER_H_INCLUDE_GUARD

#include "shrek_types.h"

namespace shrek
{
    std::vector<ExpandedByteCode> optimize_code(const std::vector<ExpandedByteCode>& code);

    namespace _detail
    {
        std::vector<ExpandedByteCode> optimize_easy_constants(const std::vector<ExpandedByteCode>& code);

        std::vector<ExpandedByteCode> optimize_2_arg_arithmetic_funcs(const std::vector<ExpandedByteCode>& code, bool& was_optimized);

        std::vector<ExpandedByteCode> optimize_1_arg_arithmetic_funcs(const std::vector<ExpandedByteCode>& code, bool& was_optimized);
    }
}

#endif // _SHREK_OPTIMIZER_H_INCLUDE_GUARD
