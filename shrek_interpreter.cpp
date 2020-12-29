#include "shrek_interpreter.h"

#include <cassert>
#include <limits.h>
#include <unordered_map>
#include <stack>
#include <regex>
#include <memory>
#include <vector>

// TODO: for quick printing.
#include <iostream>

namespace shrek
{
    enum class OpCode
    {
        no_op = 0,
        push0 = 1,
        pop = 2,
        add = 3,
        subtract = 4,
        jump0 = 5,
        jump_neg = 6,
        bump_plus = 7,
        bump_neg = 8
    };

    enum class TokenType
    {
        Whitespace,
        Command,
        Label
    };

    struct Token
    {
        TokenType token_type = TokenType::Whitespace;
        std::string value;
        std::size_t index;
    };

    struct LangTree
    {
        OpCode op_code = OpCode::no_op;
        Token token;
        std::unique_ptr<LangTree> next;
    };

    static std::size_t next_token(const std::string& code, std::size_t index, Token& result);
    static OpCode get_op_code(const std::string& value);

    void interpret_code(const std::string& code)
    {
        // Tokenize code.
        std::size_t index = 0;
        std::vector<Token> tokens;

        while (index < code.size())
        {
            Token t;
            index = next_token(code, index, t);
            if (t.token_type != TokenType::Whitespace)
            {
                tokens.push_back(std::move(t));
            }
        }

        // Build tree
        std::vector<LangTree> tree;
        for (std::size_t i = 0; i < tokens.size(); ++i)
        {
            const auto& t = tokens[i];

            if(t.token_type == TokenType::Command)
            {
                LangTree node;
                node.token = t;
                node.op_code = get_op_code(node.token.value);

                if (node.op_code == OpCode::jump0 || node.op_code == OpCode::jump_neg)
                {
                    if (i + 1 < tokens.size() && tokens[i + 1].token_type == TokenType::Label)
                    {
                        ++i;
                        node.next = std::make_unique<LangTree>();
                        node.next->token = tokens[i];
                    }
                    else
                    {
                        // TODO: Syntax error - Require label call after command.
                    }
                }

                tree.push_back(std::move(node));
            }
            else if(t.token_type == TokenType::Label)
            {
                LangTree node;
                node.token = t;
                if (i + 1 < tokens.size())
                {
                    ++i;
                    node.next = std::make_unique<LangTree>();
                    node.next->token = tokens[i];
                }

                tree.push_back(std::move(node));
            }
        }

        // Build Jump Map
        std::unordered_map<std::string, std::size_t> jump_map;
        for (std::size_t i = 0; i < tree.size(); ++i)
        {
            const auto& node = tree[i];

            if (node.op_code == OpCode::no_op && node.token.token_type == TokenType::Label)
            {
                if (jump_map.count(node.token.value))
                {
                    // TODO: Error - label redefinition.
                }

                jump_map.insert(std::make_pair(node.token.value, i));
            }
        }
    }

    static std::size_t next_token(const std::string& code, std::size_t index, Token& result)
    {
        static std::regex label_regex(R"_(^![shrekSHREK]+!)_");
        static std::regex cmd_regex(R"_(^[shrekSHREK])_");
        static std::regex whitespace_regex(R"_(\s*)_");

        std::smatch m;
        if (std::regex_search(code.begin() + index, code.end(), m, label_regex))
        {
            result.token_type = TokenType::Label;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, cmd_regex))
        {
            result.token_type = TokenType::Command;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, whitespace_regex))
        {
            result.token_type = TokenType::Whitespace;
        }
        else
        {
            // TODO: Syntax error - Invalid token.
        }

        result.value = m.str();
        result.index = index;

        return index + result.value.size();
    }

    static OpCode get_op_code(const std::string& value)
    {
        if (value.size() > 0)
        {
            switch (value[0])
            {
            case 's':
                return OpCode::push0;
            case 'h':
                return OpCode::pop;
            case 'r':
                return OpCode::add;
            case 'e':
                return OpCode::subtract;
            case 'k':
                return OpCode::jump0;
            case 'S':
                return OpCode::jump_neg;
            case 'H':
                return OpCode::bump_plus;
            case 'R':
                return OpCode::bump_neg;
            case 'E':
                break;
            case 'K':
                break;
            }
        }

        // TODO: syntax error - invalid operation.
        return OpCode::no_op;
    }

    //static constexpr auto ERR_STACK_EMPTY = "Stack empty";
    //static constexpr auto ERR_STACK_SIZE = "Stack does not contain enough elements for the operation";

    //static char* _program_code;
    //static std::size_t _program_size;
    //static std::size_t _program_counter;
    //static std::unordered_map<std::string, std::size_t> _label_map;
    //static std::stack<int> _stack;

    //static void _map_labels();
    //static void _raise_error(const std::string& what);
    //static void _step_program();

    //static void _push0();
    //static void _pop();
    //static void _add();
    //static void _subtract();
    //static void _jump0();
    //static void _jump_neg();
    //static void _bump_plus();
    //static void _bump_neg();

    //void load_program(std::ifstream& fp)
    //{
    //    if (!fp.is_open())
    //    {
    //        throw std::exception("Cannot open program file");
    //    }

    //    fp.seekg(0, std::ios::end);
    //    auto file_size = fp.tellg();

    //    if (file_size > std::numeric_limits<std::size_t>::max())
    //    {
    //        throw std::exception("Program too large");
    //    }

    //    _program_size = file_size;
    //    fp.seekg(0, std::ios::beg);
    //    fp.read(_program_code, _program_size);
    //}

    //void execute()
    //{
    //    _program_counter = 0;

    //    while (_program_counter < _program_size)
    //    {
    //        char c = _program_code[_program_counter];

    //        switch (c)
    //        {
    //        case 's':
    //            break;
    //        case 'h':
    //            break;
    //        case 'r':
    //            break;
    //        case 'e':
    //            break;
    //        case 'k':
    //            break;
    //        case 'S':
    //            break;
    //        case 'H':
    //            break;
    //        case 'R':
    //            break;
    //        case 'K':
    //            break;
    //        case '!':
    //            break;
    //        }
    //    }
    //}

    //static void _map_labels()
    //{
    //    // TODO:
    //}

    //static void _raise_error(const std::string& what)
    //{

    //}

    //static void _step_program()
    //{
    //    ++_program_counter;
    //}

    //static void _push0()
    //{
    //    _stack.push(0);
    //    _step_program();
    //}

    //static void _pop()
    //{
    //    if (_stack.empty())
    //    {
    //        _raise_error(ERR_STACK_EMPTY);
    //    }

    //    auto value = _stack.top();
    //    _stack.pop();

    //    std::cout << value; // TODO

    //    _step_program();
    //}

    //static void _add()
    //{
    //    if (_stack.size() < 2)
    //    {
    //        _raise_error(ERR_STACK_SIZE);
    //    }

    //    auto v0 = _stack.top();
    //    _stack.pop();

    //    auto v1 = _stack.top();
    //    _stack.pop();

    //    _stack.push(v0 + v1);
    //    _step_program();
    //}

    //static void _subtract()
    //{
    //    if (_stack.size() < 2)
    //    {
    //        _raise_error(ERR_STACK_SIZE);
    //    }

    //    auto v0 = _stack.top();
    //    _stack.pop();

    //    auto v1 = _stack.top();
    //    _stack.pop();

    //    _stack.push(v1 - v0);
    //    _step_program();
    //}

    //static void _jump0()
    //{

    //}

    //static void _jump_neg()
    //{

    //}

    //static void _bump_plus()
    //{

    //}

    //static void _bump_neg()
    //{

    //}
}
