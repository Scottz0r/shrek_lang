#ifndef _SHREK_RUNTIME_H_INCLUDE_GUARD
#define _SHREK_RUNTIME_H_INCLUDE_GUARD

#include "shrek.h"
#include "shrek_types.h"

#include <functional>
#include <stack>
#include <vector>
#include <optional>

namespace shrek
{
    struct RuntimeHooks
    {
        virtual ~RuntimeHooks() = default;

        virtual void on_step() = 0;

        virtual void on_runtime_error() = 0;
    };

    class ShrekRuntime
    {
        std::vector<ExpandedByteCode> m_code;
        std::vector<std::size_t> m_jump_table;
        std::size_t m_program_counter = 0;
        RuntimeHooks* m_hooks = nullptr;
        std::stack<int> m_stack;
        std::unordered_map<int, ShrekFunc> m_func_table;
        std::string m_func_exception;

        // Handle for C API calls.
        ShrekHandle* m_owning_handle;

        void build_jump_table();
        int main_loop();
        void step_program();
        std::size_t get_jump_location(int label_num);

        void op_push0();
        void op_pop();
        void op_bump();
        void op_func();
        void op_jump();
        void op_push_const();

    public:
        ShrekRuntime(ShrekHandle* owning_handle);

        inline std::stack<int>& stack() { return m_stack; }

        int run(int argc, const char** argv);

        void set_hooks(RuntimeHooks* hooks);

        const ExpandedByteCode& curr_code() const;

        bool register_function(int func_number, ShrekFunc func);

        void set_func_exception(const std::string& value);
    };
}

#endif // !_SHREK_RUNTIME_H_INCLUDE_GUARD
