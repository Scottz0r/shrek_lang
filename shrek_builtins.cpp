#include "shrek_builtins.h"

#include <iostream>

#include <unordered_map>
#include "fmt/format.h"

namespace shrek
{
    typedef void(*func_type)(void);

    static std::unordered_map<int, func_type> func_table =
    {
        {0, builtins::input},
        {1, builtins::output},
        {2, builtins::add},
        {3, builtins::subtract},
        {4, builtins::multiply},
        {5, builtins::divide},
        {6, builtins::mod},
        {7, builtins::double_},
        {8, builtins::negate}
    };

    static std::stack<int>* p_stack;
    std::string except;

    // TODO: Would be nice if the runtime was an object that could be passed to this.
    bool run_func(int func_num, std::stack<int>& stack, std::string& errmsg)
    {
        p_stack = &stack;

        auto it = func_table.find(func_num);
        if (it == func_table.end())
        {
            errmsg = "Function not found";
            return false;
        }

        except.clear();
        it->second();
        if (!except.empty())
        {
            errmsg = except;
            return false;
        }

        return true;
    }

    namespace builtins
    {
        void input()
        {
            std::string line;
            std::getline(std::cin, line);

            if (line.size() > std::numeric_limits<int>::max())
            {
                except = "input too large";
                return;
            }

            std::size_t i = line.size();
            while (i-- > 0)
            {
                p_stack->push(line[i]);
            }

            p_stack->push((int)line.size());
        }

        void output()
        {
            if (p_stack->empty())
            {
                except = "output requires value on the stack";
                return;
            }

            auto value = p_stack->top();
            fmt::print("{:#x}\n", value);
            std::cout.flush();
        }

        void add()
        {
            if (p_stack->size() < 2)
            {
                except = "add requires two values on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto v1 = p_stack->top();
            p_stack->pop();

            auto r = v1 + v0;
            p_stack->push(r);
        }

        void subtract()
        {
            if (p_stack->size() < 2)
            {
                except = "subtract requires two values on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto v1 = p_stack->top();
            p_stack->pop();

            auto r = v1 - v0;
            p_stack->push(r);
        }

        void multiply()
        {
            if (p_stack->size() < 2)
            {
                except = "multiply requires two values on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto v1 = p_stack->top();
            p_stack->pop();

            auto r = v1 * v0;
            p_stack->push(r);
        }

        void divide()
        {
            if (p_stack->size() < 2)
            {
                except = "divide requires two values on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto v1 = p_stack->top();
            p_stack->pop();

            auto r = v1 / v0;
            p_stack->push(r);
        }

        void mod()
        {
            if (p_stack->size() < 2)
            {
                except = "mod requires two values on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto v1 = p_stack->top();
            p_stack->pop();

            auto r = v1 % v0;
            p_stack->push(r);
        }

        void double_()
        {
            if (p_stack->empty())
            {
                except = "double requires one value on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto r = v0 * 2;
            p_stack->push(r);
        }

        void negate()
        {
            if (p_stack->empty())
            {
                except = "negate requires one value on the stack";
                return;
            }

            auto v0 = p_stack->top();
            p_stack->pop();

            auto r = -v0;
            p_stack->push(r);
        }
    }
}
