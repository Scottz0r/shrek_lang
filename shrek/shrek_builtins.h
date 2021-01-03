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
        clone,
        stash_add,
        stash_get,
        stash_delete
    };

    constexpr auto builtin_max_num = (int)BuiltinFuncs::clone;

    constexpr auto builtin_reserved_num = 255;

    int exec_builtin(ShrekRuntime& shrek, ShrekValue func_num);

    namespace builtins
    {
        int input(ShrekRuntime& shrek);

        int output(ShrekRuntime& shrek);

        int add(ShrekRuntime& shrek);

        int subtract(ShrekRuntime& shrek);

        int multiply(ShrekRuntime& shrek);

        int divide(ShrekRuntime& shrek);

        int mod(ShrekRuntime& shrek);

        int double_val(ShrekRuntime& shrek);

        int negate(ShrekRuntime& shrek);

        int square(ShrekRuntime& shrek);

        int clone(ShrekRuntime& shrek);

        int stash_add(ShrekRuntime& shrek);

        int stash_get(ShrekRuntime& shrek);

        int stash_delete(ShrekRuntime& shrek);
    }
}

#endif // !_SHREK_BUILTINS_H_INCLUDE_GUARD
