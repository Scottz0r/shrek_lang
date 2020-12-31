#include "shrek_runtime.h"

#include <cassert>
#include <stack>
#include "fmt/core.h"

#include "shrek.h"
#include "shrek_builtins.h"

namespace shrek
{
    constexpr auto npos = std::numeric_limits<std::size_t>::max();
    constexpr auto jump_table_default_size = 8;

    bool ShrekRuntime::load(std::vector<ByteCode>&& code)
    {
        m_code = std::move(code);

        ShrekHandle handle;
        handle.runtime = this;
        if (shrek_builtins_register(&handle) != SHREK_OK)
        {
            fmt::print("Failed to load built-ins");
            return false;
        }

        return true;
    }

    int ShrekRuntime::execute()
    {
        int exit_code = 0;

        try
        {
            build_jump_table();
            exit_code = main_loop();
        }
        catch (const RuntimeError& ex)
        {
            // TODO: can get current runtime state from instance.
            fmt::print("Runtime error: {}", ex.what());

            if (m_hooks)
            {
                m_hooks->on_runtime_error();
            }

            exit_code = 1;
        }
        catch (...)
        {
            fmt::print("Runtime encountered an unexpected exception");
            exit_code = 256;
        }

        return exit_code;
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
                if (code.a + 1 > m_jump_table.size())
                {
                    m_jump_table.resize(code.a + 1, npos);
                }

                m_jump_table[code.a] = i + 1; // Jump past label to reduce by one operation.
            }
        }
    }

    const ByteCode& ShrekRuntime::curr_code() const
    {
        if (m_program_counter < m_code.size())
        {
            return m_code[m_program_counter];
        }

        throw RuntimeError("Program counter at invalid position");
    }

    bool ShrekRuntime::register_function(int func_number, ShrekFunc func)
    {
        if (m_func_table.count(func_number) > 0)
        {
            return false;
        }

        m_func_table[func_number] = func;
        return true;
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
            exit_code = m_stack.top();
        }

        return exit_code;
    }

    void ShrekRuntime::step_program()
    {
        ++m_program_counter;
    }

    std::size_t ShrekRuntime::get_jump_location(int label_num)
    {
        if (label_num < m_jump_table.size())
        {
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

        auto it = m_func_table.find(func_num);
        if (it != m_func_table.end())
        {
            ShrekHandle handle;
            handle.runtime = this;
            int rc = it->second(&handle);
            if (rc != SHREK_OK)
            {
                // TODO: Error.
                throw RuntimeError("Error running function. TODO: Actual error reported.");
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
