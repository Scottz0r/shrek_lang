#include "shrek_runtime.h"

#include <cassert>
#include <stack>
#include "fmt/core.h"

#include "shrek.h"
#include "shrek_optimizer.h"
#include "shrek_parser.h"
#include "shrek_platform_specific.h"
#include "shrek_builtins.h"

namespace shrek
{
    constexpr auto npos = std::numeric_limits<std::size_t>::max();
    constexpr auto max_stack_size = std::numeric_limits<ShrekValue>::max();
    constexpr auto jump_table_default_size = 8;

    ShrekRuntime::ShrekRuntime(ShrekHandle* owning_handle)
        : m_owning_handle(owning_handle)
    {

    }

    int ShrekRuntime::run(int argc, const char** argv)
    {
        // TODO: Actual argument support.
        if (argc < 2)
        {
            fmt::print("Invalid arguments. Missing code file.");
            return 1;
        }

        try
        {
            m_code = shrek::parse_code(argv[1]);
            m_code = shrek::optimize_code(m_code);

            // Try to discover extension modules before execution.
            discover_modules(m_owning_handle);

            build_jump_table();
            return main_loop();
        }
        catch (const SyntaxError& ex)
        {
            fmt::print("Syntax Error: {} at index {}, token \"{}\"", ex.what(), ex.index(), ex.token());
            return 1;
        }
        catch (const RuntimeError& ex)
        {
            // TODO: can get current runtime state from instance.
            fmt::print("Runtime error: {}", ex.what());

            if (m_hooks)
            {
                m_hooks->on_runtime_error();
            }

            return 1;
        }
        catch (...)
        {
            fmt::print("Runtime encountered an unexpected exception");
            return 256;
        }
    }

    void ShrekRuntime::set_hooks(RuntimeHooks* hooks)
    {
        m_hooks = hooks;
    }

    void ShrekRuntime::build_jump_table()
    {
        m_jump_table.clear();
        m_jump_table.resize(jump_table_default_size, npos);

        for (std::size_t i = 0; i < m_code.size(); ++i)
        {
            const auto& code = m_code[i];
            if (code.op_code == OpCode::label)
            {
                if ((std::size_t)code.a + 1 > m_jump_table.size())
                {
                    m_jump_table.resize(code.a + 1, npos);
                }

                m_jump_table[code.a] = i + 1; // Jump past label to reduce by one operation.
            }
        }
    }

    const ExpandedByteCode& ShrekRuntime::curr_code() const
    {
        if (m_program_counter < m_code.size())
        {
            return m_code[m_program_counter];
        }

        throw RuntimeError("Program counter at invalid position");
    }

    bool ShrekRuntime::register_function(ShrekValue func_number, ShrekFunc func)
    {
        // Fail registration if a function was already registered with the given number.
        if (m_func_table.count(func_number) > 0)
        {
            return false;
        }

        m_func_table[func_number] = func;
        return true;
    }

    void ShrekRuntime::set_func_exception(const std::string& value)
    {
        m_func_exception = value;
    }

    void ShrekRuntime::stack_push(ShrekValue value)
    {
        if (m_stack.size() >= max_stack_size)
        {
            throw RuntimeError("Maximum stack size exceeded");
        }

        m_stack.push(value);
    }

    ShrekValue ShrekRuntime::stack_pop()
    {
        if (m_stack.empty())
        {
            throw RuntimeError("Cannot pop: stack is empty");
        }

        auto value = m_stack.top();
        m_stack.pop();
        return value;
    }

    ShrekValue ShrekRuntime::stack_peek()
    {
        if (m_stack.empty())
        {
            throw RuntimeError("Cannot peek: stack is empty");
        }

        return m_stack.top();
    }

    ShrekValue ShrekRuntime::stack_size()
    {
        return (ShrekValue)m_stack.size();
    }

    void ShrekRuntime::stash_add(ShrekValue key, ShrekValue value)
    {
        m_stash_table[key] = value;
    }

    bool ShrekRuntime::stash_get(ShrekValue key, ShrekValue& out_value)
    {
        auto it = m_stash_table.find(key);
        if (it == m_stash_table.end())
        {
            return false;
        }

        out_value = it->second;
        return true;
    }

    bool ShrekRuntime::stash_del(ShrekValue key)
    {
        if (m_stash_table.count(key) > 0)
        {
            m_stash_table.erase(key);
            return true;
        }

        return false;
    }

    int ShrekRuntime::main_loop()
    {
        while (m_program_counter < m_code.size())
        {
            if (m_hooks)
            {
                m_hooks->on_step();
            }

            switch (curr_code().op_code)
            {
            case OpCode::label:
            case OpCode::no_op:
                step_program();
                break;
            case OpCode::push0:
                op_push0();
                break;
            case OpCode::pop:
                op_pop();
                break;
            case OpCode::bump:
                op_bump();
                break;
            case OpCode::func:
                op_func();
                break;
            case OpCode::jump:
                op_jump();
                break;
            case OpCode::push_const:
                op_push_const();
                break;
            default:
                throw RuntimeError("Invalid operation");
            }
        }

        int exit_code = 0;
        if (!m_stack.empty())
        {
            exit_code = (int)m_stack.top();
        }

        return exit_code;
    }

    void ShrekRuntime::step_program()
    {
        ++m_program_counter;
    }

    std::size_t ShrekRuntime::get_jump_location(int label_num)
    {
        if ((std::size_t)label_num < m_jump_table.size())
        {
            // If label not defined, npos will be returned, which will terminate the program.
            return m_jump_table[label_num];
        }

        return npos;
    }

    void ShrekRuntime::op_push0()
    {
        m_stack.push(0);
        step_program();
    }

    void ShrekRuntime::op_pop()
    {
        if (m_stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        m_stack.pop();

        step_program();
    }

    void ShrekRuntime::op_bump()
    {
        if (m_stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        auto val = m_stack.top();
        m_stack.pop();

        ++val;
        m_stack.push(val);

        step_program();
    }

    void ShrekRuntime::op_func()
    {
        if (m_stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        auto func_num = m_stack.top();
        m_stack.pop();

        if (func_num < builtin_reserved_num)
        {
            exec_builtin(*this, func_num);
        }
        else
        {
            // Use an extension function registered in the function table.
            auto it = m_func_table.find(func_num);
            if (it != m_func_table.end())
            {
                m_func_exception.clear();

                int rc = it->second(m_owning_handle);
                if (rc != SHREK_OK)
                {
                    if (m_func_exception.empty())
                    {
                        m_func_exception = "registered function did not set exception text";
                    }

                    throw RuntimeError(fmt::format("Error running function {}: {}", func_num, m_func_exception));
                }
            }
            else
            {
                throw RuntimeError(fmt::format("Function number {} not registered", func_num));
            }
        }


        step_program();
    }

    void ShrekRuntime::op_jump()
    {
        constexpr auto jump = 0;
        constexpr auto jump_0 = 1;
        constexpr auto jump_neg = 2;

        if (m_stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        // Top of m_stack indicates jump type.
        auto s0 = m_stack.top();
        m_stack.pop();

        if (s0 == jump)
        {
            m_program_counter = get_jump_location(curr_code().a);
        }
        else if(s0 == jump_0)
        {
            if (m_stack.empty())
            {
                throw RuntimeError("jump0 requires value on m_stack after jump type");
            }

            auto s1 = m_stack.top();
            if (s1 == 0)
            {
                m_program_counter = get_jump_location(curr_code().a);
            }
            else
            {
                step_program();
            }
        }
        else if (s0 == jump_neg)
        {
            if (m_stack.empty())
            {
                throw RuntimeError("jump_neg requires value on m_stack after jump type");
            }

            auto s1 = m_stack.top();
            if (s1 < 0)
            {
                m_program_counter = get_jump_location(curr_code().a);
            }
            else
            {
                step_program();
            }
        }
        else
        {
            throw RuntimeError("Invalid jump type");
        }
    }

    void ShrekRuntime::op_push_const()
    {
        m_stack.push(curr_code().a);
        step_program();
    }
}
