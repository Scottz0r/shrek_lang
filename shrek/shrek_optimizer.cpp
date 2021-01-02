#include "shrek_optimizer.h"

#include <cassert>
#include <optional>
#include "shrek_builtins.h"

constexpr auto max_optimize_loops = 32;

namespace shrek
{
    std::vector<ExpandedByteCode> optimize_code(const std::vector<ExpandedByteCode>& code)
    {
        auto result = _detail::optimize_easy_constants(code);

        // These optimizations can be done multiple times (ex: multiply + multiply is another const).
        int counter = 0;
        bool is_optimizing = false;

        do
        {
            is_optimizing = false;
            bool was_optimized = false;

            result = _detail::optimize_1_arg_arithmetic_funcs(result, was_optimized);
            is_optimizing |= was_optimized;

            result = _detail::optimize_2_arg_arithmetic_funcs(result, was_optimized);
            is_optimizing |= was_optimized;

            ++counter;
        }
        while (is_optimizing && counter < max_optimize_loops);

        return result;
    }
}

namespace shrek::_detail
{
    inline static bool is_op_push_const(OpCode op)
    {
        return op == OpCode::push_const;
    }

    static bool is_func_2_arg_arithmetic(const ExpandedByteCode& code, int func_num)
    {
        if (code.op_code == OpCode::func && func_num < builtin_max_num)
        {
            switch (func_num)
            {
            case (int)BuiltinFuncs::add:
            case (int)BuiltinFuncs::subtract:
            case (int)BuiltinFuncs::multiply:
            case (int)BuiltinFuncs::divide:
            case (int)BuiltinFuncs::mod:
                return true;
            }
        }

        return false;
    }

    static bool is_func_1_arg_arithmetic(const ExpandedByteCode& code, int func_num)
    {
        if (code.op_code == OpCode::func && func_num < builtin_max_num)
        {
            switch (func_num)
            {
            case (int)BuiltinFuncs::double_val:
            case (int)BuiltinFuncs::negate:
            case (int)BuiltinFuncs::square:
                return true;
            }
        }

        return false;
    }

    /**
        Optimize "easy" constants, where 0 is pushed to the stack and bumped many times.
    */
    std::vector<ExpandedByteCode> optimize_easy_constants(const std::vector<ExpandedByteCode>& code)
    {
        std::vector<ExpandedByteCode> result;
        result.reserve(code.size());

        std::optional<std::size_t> push_index;
        int bump_value = 0;

        for (std::size_t i = 0; i < code.size(); ++i)
        {
            // Check if the chain of pushing has ended before handling the current operation.
            if (push_index.has_value() && code[i].op_code != OpCode::bump)
            {
                // If there was a push and it was bumped, then it can be simplified into a single operation of
                // pushing a constant to the stack. Replace the push0 and bumps with a single operation.
                if (bump_value > 0)
                {
                    ExpandedByteCode replacement_code;
                    replacement_code.op_code = OpCode::push_const;
                    replacement_code.a = bump_value;
                    replacement_code.source_code_index = code[push_index.value()].source_code_index;

                    result.push_back(replacement_code);
                }
                else
                {
                    // There were no bumps, so the push0 op needs to be copied to the output code.
                    result.push_back(code[push_index.value()]);
                }

                push_index = std::nullopt;
                bump_value = 0;
            }

            if (code[i].op_code == OpCode::push0)
            {
                // Start collection state when encountering a push.
                bump_value = 0;
                push_index = i;
            }
            else if (push_index.has_value() && code[i].op_code == OpCode::bump)
            {
                ++bump_value;
            }
            else
            {
                result.push_back(code[i]);
            }
        }

        // A non nullopt push index indicates the last command was a push.
        if (push_index.has_value())
        {
            result.push_back(code.back());
        }

        return result;
    }

    /**
        Optimize code like the following to a single constant. This assumes that "easy constant" optimization has been
        executed.

            Push Constant <= v1 (stack top - 1)
            Push Constant <= v0 (stack top when func executing)
            Push Constant <= If this constant is an arithmetic function.
            Function Call

        This series of commands can be turned into a single constant because arithmetic on constants will always be
        a constant value. This will cover cases where two constants are "mathed" into a single constant.
    */
    std::vector<ExpandedByteCode> optimize_2_arg_arithmetic_funcs(const std::vector<ExpandedByteCode>& code, bool& was_optimized)
    {
        was_optimized = false;

        // If there are not enough operations in the code, do not attempt to optimize.
        if (code.size() < 4)
        {
            return code;
        }

        std::vector<ExpandedByteCode> result;
        result.reserve(code.size());

        std::size_t i = 0;
        while (i < code.size())
        {
            // If there are not enough operations to loop forward, stop trying to optimize. Below logic assumes there
            // will always be at least 4 codes to inspect.
            if (i > code.size() - 4)
            {
                result.push_back(code[i]);
                ++i;
                continue;
            }

            bool has_push_const = true;
            has_push_const &= is_op_push_const(code[i].op_code);
            has_push_const &= is_op_push_const(code[i + 1].op_code);
            has_push_const &= is_op_push_const(code[i + 2].op_code);

            int func_num = code[i + 2].a;

            bool has_arithmetic = is_func_2_arg_arithmetic(code[i + 3], func_num);

            bool was_replaced = false;

            if (has_push_const && has_arithmetic)
            {
                int v0 = code[i + 1].a;
                int v1 = code[i].a;

                int r = 0;
                bool do_replace = true;

                switch (func_num)
                {
                case (int)BuiltinFuncs::add:
                    r = v1 + v0;
                    break;
                case (int)BuiltinFuncs::subtract:
                    r = v1 - v0;
                    break;
                case (int)BuiltinFuncs::multiply:
                    r = v1 * v0;
                    break;
                case (int)BuiltinFuncs::divide:
                    r = v1 / v0;
                    break;
                case (int)BuiltinFuncs::mod:
                    r = v1 % v0;
                    break;
                default:
                    do_replace = false;
                    break;
                }

                if (do_replace)
                {
                    ExpandedByteCode replacement;
                    replacement.source_code_index = code[i].source_code_index;
                    replacement.op_code = OpCode::push_const;
                    replacement.a = r;

                    result.push_back(replacement);
                    was_replaced = true;

                    // At least one optimization was made, so indicate in output argument.
                    was_optimized = true;

                    // Jump i by four operations to the next unoptimized code.
                    i += 4;
                }
            }

            // No optimization was done for this code
            if (!was_replaced)
            {
                result.push_back(code[i]);
                ++i;
            }
        }

        return result;
    }

    /**
        Optimize code like the following to a single constant. This assumes that "easy constant" optimization has been
        executed.

            Push Constant <= v0 (stack top when func executing)
            Push Constant <= If this constant is an arithmetic function.
            Function Call

        This series of commands can be turned into a single constant because arithmetic on constants will always be
        a constant value. This will cover cases where two constants are "mathed" into a single constant.
    */
    std::vector<ExpandedByteCode> optimize_1_arg_arithmetic_funcs(const std::vector<ExpandedByteCode>& code, bool& was_optimized)
    {
        was_optimized = false;

        if (code.size() < 3)
        {
            return code;
        }

        std::vector<ExpandedByteCode> result;
        result.reserve(code.size());

        std::size_t i = 0;
        while (i < code.size())
        {
            // If there are not enough operations to loop forward, stop trying to optimize. Below logic assumes there
            // will always be at least 3 codes to inspect.
            if (i > code.size() - 3)
            {
                result.push_back(code[i]);
                ++i;
                continue;
            }

            bool has_push_const = true;
            has_push_const &= is_op_push_const(code[i].op_code);
            has_push_const &= is_op_push_const(code[i + 1].op_code);

            int func_num = code[i + 1].a;

            bool has_arithmetic = is_func_1_arg_arithmetic(code[i + 2], func_num);

            bool was_replaced = false;

            if (has_push_const && has_arithmetic)
            {
                int v0 = code[i].a;

                int r = 0;
                bool do_replace = true;

                switch (func_num)
                {
                case (int)BuiltinFuncs::double_val:
                    r = v0 * 2;
                    break;
                case (int)BuiltinFuncs::negate:
                    r = -v0;
                    break;
                case (int)BuiltinFuncs::square:
                    r = v0 * v0;
                    break;
                default:
                    do_replace = false;
                    break;
                }

                if (do_replace)
                {
                    ExpandedByteCode replacement;
                    replacement.source_code_index = code[i].source_code_index;
                    replacement.op_code = OpCode::push_const;
                    replacement.a = r;

                    result.push_back(replacement);
                    was_replaced = true;

                    // At least one optimization was made, so indicate in output argument.
                    was_optimized = true;

                    // Jump i by three operations to the next unoptimized code.
                    i += 3;
                }
            }

            // No optimization was done for this code
            if (!was_replaced)
            {
                result.push_back(code[i]);
                ++i;
            }
        }

        return result;
    }
}
