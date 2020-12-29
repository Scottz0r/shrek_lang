#include "shrek_runtime.h"

#include <stack>
#include "fmt/core.h"

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

    static void push0();
    static void pop();
    static void add();
    static void subtract();
    static void jump0();
    static void jump_neg();
    static void bump_plus();
    static void bump_neg();
    static void input();
    static void output();

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
                push0();
                break;
            case OpCode::pop:
                pop();
                break;
            case OpCode::add:
                add();
                break;
            case OpCode::subtract:
                subtract();
                break;
            case OpCode::jump0:
                jump0();
                break;
            case OpCode::jump_neg:
                jump_neg();
                break;
            case OpCode::bump_plus:
                bump_plus();
                break;
            case OpCode::bump_neg:
                bump_neg();
                break;
            case OpCode::input:
                input();
                break;
            case OpCode::output:
                output();
                break;
            default:
                // TODO: Different error type? This seems like an error in development of the runtime.
                throw RuntimeError("Unknown operation", program_counter, OpCode::no_op);
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

    static void push0()
    {
        stack.push(0);
        step_program();
    }

    static void pop()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        stack.pop();

        step_program();
    }

    static void add()
    {
        if (stack.size() < 2)
        {
            throw MAKE_RUNTIME_ERROR("Not enough stack for add");
        }

        auto a = stack.top();
        stack.pop();
        auto b = stack.top();
        stack.pop();

        stack.push(a + b);

        step_program();
    }

    static void subtract()
    {
        if (stack.size() < 2)
        {
            throw MAKE_RUNTIME_ERROR("Not enough stack for subtract");
        }

        auto a = stack.top();
        stack.pop();
        auto b = stack.top();
        stack.pop();

        stack.push(b - a);

        step_program();
    }

    static void jump0()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto val = stack.top();

        if (val == 0)
        {
            const auto& code = (*program)[program_counter];
            program_counter = get_jump_location(code.a);
        }
        else
        {
            step_program();
        }
    }

    static void jump_neg()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto val = stack.top();

        if (val < 0)
        {
            const auto& code = (*program)[program_counter];
            program_counter = get_jump_location(code.a);
        }
        else
        {
            step_program();
        }
    }

    static void bump_plus()
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

    static void bump_neg()
    {
        if (stack.empty())
        {
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto val = stack.top();
        stack.pop();

        --val;
        stack.push(val);

        step_program();
    }

    static void input()
    {
        std::string input;
        //std::cin >> input;


        // TODO: Better parsing?
        auto val = std::stoi(input);
        stack.push(val);

        step_program();
    }

    static void output()
    {
        if (stack.empty())
        {
            // TODO: Macro to help make this?
            throw MAKE_RUNTIME_ERROR("Stack is empty");
        }

        auto val = stack.top();
        stack.pop();

        fmt::print("{:#x}\n", val);
        fflush(stdout);

        step_program();
    }
}
