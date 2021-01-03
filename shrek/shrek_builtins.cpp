#include "shrek_builtins.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include "fmt/format.h"

namespace shrek
{
    int exec_builtin(ShrekRuntime& shrek, ShrekValue func_num)
    {
        switch ((BuiltinFuncs)func_num)
        {
        case shrek::BuiltinFuncs::input:
            return builtins::input(shrek);
        case shrek::BuiltinFuncs::output:
            return builtins::output(shrek);
        case shrek::BuiltinFuncs::add:
            return builtins::add(shrek);
        case shrek::BuiltinFuncs::subtract:
            return builtins::subtract(shrek);
        case shrek::BuiltinFuncs::multiply:
            return builtins::multiply(shrek);
        case shrek::BuiltinFuncs::divide:
            return builtins::divide(shrek);
        case shrek::BuiltinFuncs::mod:
            return builtins::mod(shrek);
        case shrek::BuiltinFuncs::double_val:
            return builtins::double_val(shrek);
        case shrek::BuiltinFuncs::negate:
            return builtins::negate(shrek);
        case shrek::BuiltinFuncs::square:
            return builtins::square(shrek);
        case shrek::BuiltinFuncs::clone:
            return builtins::clone(shrek);
        case shrek::BuiltinFuncs::stash_add:
            return builtins::stash_add(shrek);
        case shrek::BuiltinFuncs::stash_get:
            return builtins::stash_get(shrek);
        case shrek::BuiltinFuncs::stash_delete:
            return builtins::stash_delete(shrek);
        default:
            shrek.set_func_exception("invalid built-in");
            return SHREK_ERROR;
        }
    }
}

namespace shrek::builtins
{
    int input(ShrekRuntime& shrek)
    {
        try
        {
            std::string line;
            std::getline(std::cin, line);

            if (line.size() > (std::size_t)std::numeric_limits<int>::max())
            {
                shrek.set_func_exception("input too large");
                return SHREK_ERROR;;
            }

            // Place null terminator
            shrek.stack_push(0);

            // Place input characters in reverse order (so popping is in the "correct" order)
            std::size_t i = line.size();
            while (i-- > 0)
            {
                shrek.stack_push(line[i]);
            }

            return SHREK_OK;
        }
        catch (...)
        {
            shrek.set_func_exception("i/o error");
            return SHREK_ERROR;
        }
    }

    int output(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() == 0)
        {
            shrek.set_func_exception("output requires value on the stack");
            return SHREK_ERROR;
        }

        try
        {
            fmt::print("{}\n", shrek.stack_peek());
            std::cout.flush();

            return SHREK_OK;
        }
        catch (...)
        {
            shrek.set_func_exception("output error");
            return SHREK_ERROR;
        }
    }

    int add(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("add requires two values on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_pop();

        auto r = v1 + v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int subtract(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("subtract requires two values on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_pop();

        auto r = v1 - v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int multiply(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("multiply requires two values on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_pop();

        auto r = v1 * v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int divide(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("divide requires two values on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_pop();

        auto r = v1 / v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int mod(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("mod requires two values on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_pop();

        auto r = v1 % v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int double_val(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("double requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();

        auto r = v0 * 2;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int negate(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("negate requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();

        auto r = -v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int square(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("square requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();

        auto r = v0 * v0;
        shrek.stack_push(r);

        return SHREK_OK;
    }

    int clone(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("clone requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_peek();
        shrek.stack_push(v0);

        return SHREK_OK;
    }


    int stash_add(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 2)
        {
            shrek.set_func_exception("stash add requires two values on the stack");
            return SHREK_ERROR;
        }

        // Pop the key, but keep the value on the stack.
        auto v0 = shrek.stack_pop();
        auto v1 = shrek.stack_peek();

        shrek.stash_add(v0, v1);

        return SHREK_OK;
    }

    int stash_get(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("stash get requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();

        ShrekValue val = 0;
        if (!shrek.stash_get(v0, val))
        {
            auto errmsg = fmt::format("stash get: key {} not in stash", v0);
            shrek.set_func_exception(errmsg);
            return SHREK_ERROR;
        }

        shrek.stack_push(val);
        return SHREK_OK;
    }

    int stash_delete(ShrekRuntime& shrek)
    {
        if (shrek.stack_size() < 1)
        {
            shrek.set_func_exception("stash delete requires one value on the stack");
            return SHREK_ERROR;
        }

        auto v0 = shrek.stack_pop();

        if (!shrek.stash_del(v0))
        {
            auto errmsg = fmt::format("stash delete: key {} not in stash", v0);
            shrek.set_func_exception(errmsg);
            return SHREK_ERROR;
        }

        return SHREK_OK;
    }
}
