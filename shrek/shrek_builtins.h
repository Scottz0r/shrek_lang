#ifndef _SHREK_BUILTINS_H_INCLUDE_GUARD
#define _SHREK_BUILTINS_H_INCLUDE_GUARD

#include "shrek.h"
#include "shrek_runtime.h"

namespace shrek
{
    enum class BuiltinFuncs
    {
        input,
        output,
        add,
        subtract,
        multiply,
        divide,
        mod,
        double_val,
        negate,
        square,
        clone
    };

    constexpr auto builtin_max_num = (int)BuiltinFuncs::clone;

    // void register_builtins(ShrekRuntime& shrek);
}

// TODO: Probably doesn't need to be a C-API.
extern "C"
{
    shrek_API_FUNC(int) shrek_builtins_register(ShrekHandle* handle);
}

#endif // !_SHREK_BUILTINS_H_INCLUDE_GUARD
