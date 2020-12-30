#ifndef _SHREK_BUILTINS_H_INCLUDE_GUARD
#define _SHREK_BUILTINS_H_INCLUDE_GUARD

#include "shrek_types.h"
#include <stack>

namespace shrek
{
    bool run_func(int func_num, std::stack<int>& stack, std::string& errmsg);

    namespace builtins
    {
        void input();

        void output();

        void add();

        void subtract();

        void multiply();

        void divide();

        void mod();

        void double_();

        void negate();
    }
}

#endif // !_SHREK_BUILTINS_H_INCLUDE_GUARD
