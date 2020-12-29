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
        std::unique_ptr<SyntaxTreeNode> next;
    };

    struct SyntaxTree
    {
        std::vector<SyntaxTreeNode> syntax;
        std::unordered_map<std::string, std::size_t> jump_map;
    };

    struct ByteCode
    {
        OpCode op_code = OpCode::no_op;
        int a = 0;
        std::size_t originator_token_index = 0;
    };

    // TODO: Better errors.
    struct SyntaxError : public std::exception
    {
        SyntaxError(const char* what)
            : std::exception(what)
        {}
    };

    // TODO: Better errors.
    struct RuntimeError : public std::exception
    {
        RuntimeError(const char* what)
            : std::exception(what)
        {}
    };
}

#endif // _SHREK_TYPES_H_INCLUDE_GUARD
