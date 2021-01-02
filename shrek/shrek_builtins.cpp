#include "shrek_builtins.h"

#include <cassert>
#include <iostream>
#include <unordered_map>
#include "fmt/format.h"

namespace shrek
{
    namespace builtins
    {
        int input(ShrekHandle* shrek) noexcept
        {
            assert(shrek);

            try
            {
                std::string line;
                std::getline(std::cin, line);

                if (line.size() > (std::size_t)std::numeric_limits<int>::max())
                {
                    shrek_set_except(shrek, "input too large");
                    return 1;
                }

                // Place null terminator
                shrek_push(shrek, 0);

                // Place input characters in reverse order (so popping is in the "correct" order)
                std::size_t i = line.size();
                while (i-- > 0)
                {
                    shrek_push(shrek, line[i]);
                }

                return SHREK_OK;
            }
            catch (...)
            {
                shrek_set_except(shrek, "i/o error");
                return SHREK_ERROR;
            }
        }

        int output(ShrekHandle* shrek) noexcept
        {
            assert(shrek);

            int value;
            if (shrek_peek(shrek, &value) != SHREK_OK)
            {
                shrek_set_except(shrek, "output requires value on the stack");
                return SHREK_ERROR;
            }

            try
            {
                fmt::print("{}\n", value);
                std::cout.flush();

                return SHREK_OK;
            }
            catch (...)
            {
                shrek_set_except(shrek, "output error");
                return SHREK_ERROR;
            }
        }

        int add(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 2)
            {
                shrek_set_except(shrek, "add requires two values on the stack");
                return SHREK_ERROR;
            }

            int v0, v1;
            shrek_pop(shrek, &v0);
            shrek_pop(shrek, &v1);

            auto r = v1 + v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int subtract(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 2)
            {
                shrek_set_except(shrek, "subtract requires two values on the stack");
                return SHREK_ERROR;
            }

            int v0, v1;
            shrek_pop(shrek, &v0);
            shrek_pop(shrek, &v1);

            auto r = v1 - v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int multiply(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 2)
            {
                shrek_set_except(shrek, "multiply requires two values on the stack");
                return SHREK_ERROR;
            }

            int v0, v1;
            shrek_pop(shrek, &v0);
            shrek_pop(shrek, &v1);

            auto r = v1 * v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int divide(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 2)
            {
                shrek_set_except(shrek, "divide requires two values on the stack");
                return SHREK_ERROR;
            }

            int v0, v1;
            shrek_pop(shrek, &v0);
            shrek_pop(shrek, &v1);

            auto r = v1 / v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int mod(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 2)
            {
                shrek_set_except(shrek, "mod requires two values on the stack");
                return SHREK_ERROR;
            }

            int v0, v1;
            shrek_pop(shrek, &v0);
            shrek_pop(shrek, &v1);

            auto r = v1 % v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int double_val(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 1)
            {
                shrek_set_except(shrek, "double requires one value on the stack");
                return SHREK_ERROR;
            }

            int v0;
            shrek_pop(shrek, &v0);

            auto r = v0 * 2;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int negate(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 1)
            {
                shrek_set_except(shrek, "negate requires one value on the stack");
                return SHREK_ERROR;
            }

            int v0;
            shrek_pop(shrek, &v0);

            auto r = -v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int square(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 1)
            {
                shrek_set_except(shrek, "square requires one value on the stack");
                return SHREK_ERROR;
            }

            int v0;
            shrek_pop(shrek, &v0);

            auto r = v0 * v0;
            shrek_push(shrek, r);

            return SHREK_OK;
        }

        int clone(ShrekHandle* shrek) noexcept
        {
            if (shrek_stack_size(shrek) < 1)
            {
                shrek_set_except(shrek, "clone requires one value on the stack");
                return SHREK_ERROR;
            }

            int v0;
            shrek_peek(shrek, &v0);
            shrek_push(shrek, v0);

            return SHREK_OK;
        }
    }
}

extern "C"
{
    int shrek_builtins_register(ShrekHandle* shrek)
    {
        std::vector<ShrekFunc> builtin_funcs =
        {
            shrek::builtins::input,
            shrek::builtins::output,
            shrek::builtins::add,
            shrek::builtins::subtract,
            shrek::builtins::multiply,
            shrek::builtins::divide,
            shrek::builtins::mod,
            shrek::builtins::double_val,
            shrek::builtins::negate,
            shrek::builtins::square,
            shrek::builtins::clone
        };

        for (std::size_t i = 0; i < builtin_funcs.size(); ++i)
        {
            int rc = shrek_register_func(shrek, (int)i, builtin_funcs[i]);
            if (rc != SHREK_OK)
            {
                return SHREK_ERROR;
            }
        }

        return SHREK_OK;
    }
}
