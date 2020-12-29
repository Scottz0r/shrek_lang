// TODO: This should really be named "shrek_parser"

#include "shrek_interpreter.h"

#include <cassert>
#include <limits.h>
#include <optional>
#include <stack>
#include <regex>
#include <unordered_map>
#include "fmt/format.h"

#include "shrek_types.h"
#include "shrek_platform_specific.h"

namespace shrek
{
    static int get_or_add(std::unordered_map<std::string, int>& map, const std::string& search, int add_value);

    static std::size_t next_token(const std::string& code, std::size_t index, Token& result);
    static std::optional<OpCode> get_op_code(const std::string& value);
    static void parse_command(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
    static void parse_label(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
    std::vector<ByteCode> interpret_code_impl(const std::string& code);

    std::vector<ByteCode> interpret_code(const std::vector<std::string>& args)
    {
        // TODO: Parse arguments.
        if (args.size() < 2)
        {
            fmt::print("Invalid arguments");
            exit(1);
        }

        std::string code;
        if (!read_all_text(args.at(1), code))
        {
            fmt::print("Failed to read source file");
            exit(1);
        }

        try
        {
            return interpret_code_impl(code);
        }
        catch (const SyntaxError& ex)
        {
            fmt::print("Syntax Error: {} at index {}, token \"{}\"", ex.what(), ex.index(), ex.token());
            exit(1);
        }
        catch (...)
        {
            fmt::print("Interpreter encountered an unexpected exception.");
            exit(1);
        }
    }

    std::vector<ByteCode> interpret_code_impl(const std::string& code)
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
                parse_command(tokens, index, syntax_tree);
                break;
            case TokenType::label:
                parse_label(tokens, index, syntax_tree);
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

        std::unordered_map<std::string, int> label_map;

        for (const auto& node : syntax_tree.syntax)
        {
            ByteCode code;
            code.op_code = node.op_code;

            if (node.op_code == OpCode::label)
            {
                // Get the label number if already found, otherwise add.
                code.a = get_or_add(label_map, node.token.value, (int)label_map.size());
            }
            else if (node.op_code == OpCode::jump0 || node.op_code == OpCode::jump_neg)
            {
                if (node.children.empty() || node.children[0]->op_code != OpCode::label)
                {
                    throw SyntaxError("Jump must be followed by label", node.token.index, node.token.value);
                }

                // Get the label number if already found, otherwise add. The label number is not the actual position,
                // so it can be assigned a number before the label op_code is found.
                code.a = get_or_add(label_map, node.children[0]->token.value, (int)label_map.size());
            }

            // TODO: Debugging symbols?

            byte_code.push_back(code);
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
            throw SyntaxError("Invalid token", index, "");
        }

        result.value = m.str();
        result.index = index;

        return index + result.value.size();
    }

    static std::optional<OpCode> get_op_code(const std::string& value)
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

        return std::nullopt;
    }

    static void parse_command(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree)
    {
        assert(index < tokens.size());

        const auto& token = tokens[index];
        ++index;

        SyntaxTreeNode node;
        node.token = token;

        auto op_code_translated = get_op_code(node.token.value);
        if (!op_code_translated)
        {
            throw SyntaxError("Invalid command", node.token.index, node.token.value);
        }

        node.op_code = *op_code_translated;

        if (node.op_code == OpCode::jump0 || node.op_code == OpCode::jump_neg)
        {
            if (index < tokens.size() && tokens[index].token_type == TokenType::label)
            {
                SyntaxTreeNode child_label;
                child_label.token = tokens[index];
                child_label.op_code = OpCode::label;
                ++index;

                node.children.push_back(std::make_unique<SyntaxTreeNode>(std::move(child_label)));
            }
            else
            {
                throw SyntaxError("Missing label after jump command", node.token.index, node.token.value);
            }
        }

        tree.syntax.push_back(std::move(node));
    }

    static void parse_label(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree)
    {
        assert(index < tokens.size());

        SyntaxTreeNode node;
        node.token = tokens[index];
        node.op_code = OpCode::label;
        ++index;

        tree.syntax.push_back(std::move(node));
    }

    static int get_or_add(std::unordered_map<std::string, int>& map, const std::string& search, int add_value)
    {
        auto it = map.find(search);
        if (it != map.end())
        {
            return it->second;
        }

        map.insert(std::make_pair(search, add_value));
        return add_value;
    }
}
