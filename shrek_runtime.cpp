#include "shrek_runtime.h"

#include <stack>

// TODO - for output, probably need better.
#include <iostream>

namespace shrek
{
    static std::size_t program_counter = 0;
    static std::stack<int> stack;

    const std::vector<ByteCode>* program;

    static void main_loop();
    static void step_program();
    static void exit_program();

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
            main_loop();
        }
        catch (const RuntimeError& ex)
        {

        }
        catch (...)
        {

        }
    }

    static void main_loop()
    {
        while (program_counter < program->size())
        {
            if (program_counter >= program->size())
            {
                // TODO: More detailed error.
                throw RuntimeError("Program read invalid index");
            }

            const auto& code = (*program)[program_counter];
            switch (code.op_code)
            {
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
                throw RuntimeError("Unknown operation");
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

    static void push0()
    {
        stack.push(0);
        step_program();
    }

    static void pop()
    {
        if (stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        stack.pop();

        step_program();
    }

    static void add()
    {
        if (stack.size() < 2)
        {
            throw RuntimeError("Not enough stack for add");
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
            throw RuntimeError("Not enough stack for subtract");
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
            throw RuntimeError("Stack is empty");
        }

        auto val = stack.top();

        if (val == 0)
        {
            const auto& code = (*program)[program_counter];
            program_counter = static_cast<std::size_t>(code.a);
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
            throw RuntimeError("Stack is empty");
        }

        auto val = stack.top();

        if (val < 0)
        {
            const auto& code = (*program)[program_counter];
            program_counter = static_cast<std::size_t>(code.a);
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
            throw RuntimeError("Stack is empty");
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
            throw RuntimeError("Stack is empty");
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
        std::cin >> input;

        // TODO: Better parsing?
        auto val = std::stoi(input);
        stack.push(val);

        step_program();
    }

    static void output()
    {
        if (stack.empty())
        {
            throw RuntimeError("Stack is empty");
        }

        auto val = stack.top();
        stack.pop();

        std::cout << val;
        std::cout.flush();

        step_program();
    }
}
