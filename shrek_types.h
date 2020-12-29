#ifndef _SHREK_TYPES_H_INCLUDE_GUARD
#define _SHREK_TYPES_H_INCLUDE_GUARD

#include <exception>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace shrek
{
    enum class OpCode
    {
        no_op,
        label,
        push0,
        pop,
        add,
        subtract,
        jump0,
        jump_neg,
        bump_plus,
        bump_neg,
        input,
        output
    };

    enum class TokenType
    {
        whitespace,
        command,
        label
    };

    struct Token
    {
        TokenType token_type = TokenType::whitespace;
        std::string value;
        std::size_t index;
    };

    struct SyntaxTreeNode
    {
        OpCode op_code = OpCode::no_op;
        Token token;
        std::vector<std::unique_ptr<SyntaxTreeNode>> children;
    };

    struct SyntaxTree
    {
        std::vector<SyntaxTreeNode> syntax;
    };

    struct ByteCode
    {
        OpCode op_code = OpCode::no_op;
        int a = 0;
    };

    class SyntaxError
    {
    public:
        SyntaxError(const std::string& what, std::size_t index, const std::string& token)
            : m_what(what)
            , m_index(index)
            , m_token(token)
        {}

        const std::string& what() const { return m_what; }

        std::size_t index() const { return m_index; }

        const std::string& token() const { return m_token; }

    private:
        std::string m_what;
        std::size_t m_index;
        std::string m_token;
    };

    class RuntimeError
    {
    public:
        RuntimeError(const std::string& what, std::size_t program_counter, OpCode op_code)
            : m_what(what)
            , m_program_counter(program_counter)
            , m_op_code( op_code )
        {}

        const std::string& what() const { return m_what; }

        std::size_t program_counter() const { return m_program_counter; }

        OpCode op_code() const { return m_op_code; }

    private:
        std::string m_what;
        std::size_t m_program_counter;
        OpCode m_op_code;
    };
}

#endif // _SHREK_TYPES_H_INCLUDE_GUARD
