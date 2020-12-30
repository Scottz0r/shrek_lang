#include "shrek_runtime.h"

#include <cassert>
#include <stack>
#include "fmt/core.h"
#include "shrek_builtins.h"

#define MAKE_RUNTIME_ERROR(what) (RuntimeError(what, program_counter, curr_code->op_code))

namespace shrek
{
    constexpr auto npos = std::numeric_limits<std::size_t>::max();
    constexpr auto jump_table_default_size = 8;

    static std::size_t program_counter = 0;
    static std::stack<int> stack;
    static std::vector<std::size_t> jump_table;

    const std::vector<ByteCode>* program;
    const ByteCode* curr_code;

    static void build_jump_table();
    static void main_loop();
    static void step_program();
    static void exit_program();
    static std::size_t get_jump_location(int label_num);

    static void op_push0();
    static void op_pop();
    static void op_bump();
    static void op_func();
    static void op_jump();
    static void op_push_const();

    void execute(const std::vector<ByteCode>& code)
    {
        try
        {
            program = &code;
            build_jump_table();
            main_loop();
        }
        catch (const RuntimeError& ex)
        {
            fmt::print("Runtime error: {}", ex.what());
            exit(1);
        }
        catch (...)
        {
            fmt::print("Runtime encountered an unexpected exception");
            exit(1);
        }
    }

    static void build_jump_table()
    {
        jump_table.clear();
        jump_table.resize(jump_table_default_size, npos);

        for (std::size_t i = 0; i < program->size(); ++i)
        {
            const auto& code = (*program)[i];
            if (code.op_code == OpCode::label)
            {
                if (code.a + 1 > jump_table.size())
                {
                    jump_table.resize(code.a + 1, npos);
                }

                jump_table[code.a] = i + 1; // Jump past label to reduce by one operation.
            }
        }
    }

    static void main_loop()
    {
        while (program_counter < program->size())
        {
            if (program_counter >= program->size())
            {
                exit_program();
            }

            curr_code = program->data() + program_counter; //&(*program)[program_counter];
            switch (curr_code->op_code)
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
                throw RuntimeError("Invalid operation", program_counter, OpCode::no_op);
            }
        }
    }

    static void step_program()
    {
        ++program_counter;
        if (program_counter == program->size())
        {
            exit_program();
        }
    }

    static void exit_program()
    {
        int exit_code = 0;
        if (stack.size())
        {
            exit_code = stack.top();
        }

        exit(exit_code);
    }

    static std::size_t get_jump_location(int label_num)
    {
        if (label_num < jump_table.size())
        {
            return jump_table[label_num];
        }

        return npos;
    }

    static void op_push0()
    {
        stack.push(0);
        step_program();
    }

    static void op_pop()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        stack.pop();

        step_program();
    }

    static void op_bump()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto val = stack.top();
        stack.pop();

        ++val;
        stack.push(val);

        step_program();
    }

    static void op_func()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto func_num = stack.top();
        stack.pop();

        std::string errmsg;
        if (!run_func(func_num, stack, errmsg))
        {
            throw MAKE_RUNTIME_ERROR(errmsg);
        }

        step_program();
    }

    static void op_jump()
    {
        assert(curr_code != nullptr);

        constexpr auto jump = 0;
        constexpr auto jump_0 = 1;
        constexpr auto jump_neg = 2;

        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        // Top of stack indicates jump type.
        auto s0 = stack.top();
        stack.pop();

        if (s0 == jump)
        {
            program_counter = get_jump_location(curr_code->a);
        }
        else if(s0 == jump_0)
        {
            if (stack.empty())
            {
                throw MAKE_RUNTIME_ERROR("jump0 requires value on stack after jump type");
            }

            auto s1 = stack.top();
            if (s1 == 0)
            {
                program_counter = get_jump_location(curr_code->a);
            }
            else
            {
                step_program();
            }
        }
        else if (s0 == jump_neg)
        {
            if (stack.empty())
            {
                throw MAKE_RUNTIME_ERROR("jump_neg requires value on stack after jump type");
            }

            auto s1 = stack.top();
            if (s1 < 0)
            {
                program_counter = get_jump_location(curr_code->a);
            }
            else
            {
                step_program();
            }
        }
        else
        {
            throw MAKE_RUNTIME_ERROR("Invalid jump type");
        }
    }

    static void op_push_const()
    {
        assert(curr_code != nullptr);

        stack.push(curr_code->a);
        step_program();
    }
}
