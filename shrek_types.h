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
        bump,
        func,
        jump,
        push_const
    };

    enum class TokenType
    {
        whitespace,
        command,
        label,
        comment
    };

    struct Token
    {
        TokenType token_type = TokenType::whitespace;
        std::string value;
        std::size_t index;
    };

    struct SyntaxTreeNode
    {
        Token token;
        std::vector<SyntaxTreeNode> children;
    };

    struct SyntaxTree
    {
        std::vector<SyntaxTreeNode> syntax;
    };

    struct ByteCode
    {
        std::size_t source_code_index;
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
        RuntimeError(const std::string& what)
            : m_what(what)
        {}

        const std::string& what() const { return m_what; }

    private:
        std::string m_what;
    };
}

#endif // _SHREK_TYPES_H_INCLUDE_GUARD
