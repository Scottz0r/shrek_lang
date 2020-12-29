// TODO: This should really be named "shrek_parser"

#include "shrek_interpreter.h"

#include <cassert>
#include <limits.h>
#include <unordered_map>
#include <stack>
#include <regex>

#include "shrek_types.h"

namespace shrek
{
    static std::size_t next_token(const std::string& code, std::size_t index, Token& result);
    static OpCode get_op_code(const std::string& value);

    static SyntaxTreeNode parse_command(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
    static SyntaxTreeNode parse_label(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);

    std::vector<ByteCode> interpret_code(const std::string& code)
    {
        // Tokenize code.
        std::size_t index = 0;
        std::vector<Token> tokens;

        while (index < code.size())
        {
            Token t;
            index = next_token(code, index, t);
            tokens.push_back(std::move(t));
        }

        // Build syntax tree
        SyntaxTree syntax_tree;
        index = 0;
        while (index < tokens.size())
        {
            SyntaxTreeNode node;

            switch (tokens[index].token_type)
            {
            case TokenType::command:
                node = parse_command(tokens, index, syntax_tree);
                syntax_tree.syntax.push_back(std::move(node));
                break;
            case TokenType::label:
                node = parse_label(tokens, index, syntax_tree);
                syntax_tree.syntax.push_back(std::move(node));
                break;
            default:
                // Skip other items in tree
                ++index;
                break;
            }
        }

        // Build Byte Code
        std::vector<ByteCode> byte_code;
        byte_code.reserve(syntax_tree.syntax.size());
        for (const auto& node : syntax_tree.syntax)
        {
            bool code_added = false;

            const auto* cnode = &node;
            while (cnode)
            {
                if (cnode->op_code != OpCode::no_op)
                {
                    ByteCode code;
                    code.op_code = cnode->op_code;
                    code.originator_token_index = cnode->token.index;

                    if (cnode->op_code == OpCode::jump0 || cnode->op_code == OpCode::jump_neg)
                    {
                        assert(cnode->next);
                        code.a = (int)syntax_tree.jump_map.at(cnode->next->token.value);
                    }

                    byte_code.push_back(code);
                    code_added = true;
                }

                cnode = cnode->next.get();
            }

            // TODO: Should be needed - There should always be an operation added with valid syntax. This must be done
            // so jump map is not messed up.
            if (!code_added)
            {
                ByteCode code;
                code.op_code = OpCode::no_op;
                code.originator_token_index = node.token.index;
                byte_code.push_back(code);
            }
        }

        return byte_code;
    }

    static std::size_t next_token(const std::string& code, std::size_t index, Token& result)
    {
        static std::regex label_regex(R"_(^![shrekSHREK]+!)_");
        static std::regex cmd_regex(R"_(^[shrekSHREK])_");
        static std::regex whitespace_regex(R"_(\s*)_");

        std::smatch m;
        if (std::regex_search(code.begin() + index, code.end(), m, label_regex))
        {
            result.token_type = TokenType::label;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, cmd_regex))
        {
            result.token_type = TokenType::command;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, whitespace_regex))
        {
            result.token_type = TokenType::whitespace;
        }
        else
        {
            throw SyntaxError("Invalid token."); // TODO: More detailed error.
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
                return OpCode::input;
            case 'K':
                return OpCode::output;
            }
        }

        // TODO: More detailed error
        throw SyntaxError("Invalid command.");
    }

    static SyntaxTreeNode parse_command(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree)
    {
        assert(index < tokens.size());

        const auto& token = tokens[index];
        ++index;

        SyntaxTreeNode node;
        node.token = token;
        node.op_code = get_op_code(node.token.value);

        if (node.op_code == OpCode::jump0 || node.op_code == OpCode::jump_neg)
        {
            if (index < tokens.size() && tokens[index].token_type == TokenType::label)
            {
                node.next = std::make_unique<SyntaxTreeNode>();
                node.next->op_code = OpCode::no_op;
                node.next->token = tokens[index];

                ++index;
            }
            else
            {
                // TODO: More detailed error
                throw SyntaxError("Missing label after jump command");
            }
        }

        return node;
    }

    static SyntaxTreeNode parse_label(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree)
    {
        assert(index < tokens.size());

        SyntaxTreeNode node;
        node.token = tokens[index];
        node.op_code = OpCode::no_op;

        ++index;

        if (tree.jump_map.count(node.token.value) > 0)
        {
            // TODO: More detailed error.
            throw SyntaxError("Label redefinition.");
        }

        node.next = std::make_unique<SyntaxTreeNode>();
        *node.next = parse_command(tokens, index, tree);

        // Point to the record that is about to be added to the syntax tree.
        tree.jump_map.insert(std::make_pair(node.token.value, tree.syntax.size()));

        return node;
    }
}
