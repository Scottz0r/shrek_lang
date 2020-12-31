#ifndef _SHREK_RUNTIME_H_INCLUDE_GUARD
#define _SHREK_RUNTIME_H_INCLUDE_GUARD

#include "shrek_types.h"
#include "shrek_ext.h"

#include <functional>
#include <stack>
#include <vector>
#include <optional>

// TODO: Put in a shrek_ext.h header?


namespace shrek
{
    // using ShrekFunc = std::function<void(ShrekRuntime&)>;

    struct RuntimeHooks
    {
        virtual ~RuntimeHooks() = default;

        virtual void on_step() = 0;

        virtual void on_runtime_error() = 0;
    };

    class ShrekRuntime
    {
        std::vector<ByteCode> m_code;
        std::vector<std::size_t> m_jump_table;
        std::size_t m_program_counter = 0;
        RuntimeHooks* m_hooks = nullptr;
        std::stack<int> m_stack;
        std::unordered_map<int, ShrekFunc> m_func_table;

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
        inline std::stack<int>& stack() { return m_stack; }

        bool load(std::vector<ByteCode>&& code);

        int execute();

        void set_hooks(RuntimeHooks* hooks);

        const ByteCode& curr_code() const;

        bool register_function(int func_number, ShrekFunc func);
    };
}

#endif // !_SHREK_RUNTIME_H_INCLUDE_GUARD
