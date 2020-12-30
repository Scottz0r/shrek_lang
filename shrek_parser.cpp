// TODO: This should really be named "shrek_parser"

#include "shrek_parser.h"

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
    struct ShrekCommands
    {
        static constexpr auto push0 = "S";
        static constexpr auto pop = "H";
        static constexpr auto bump = "R";
        static constexpr auto func = "E";
        static constexpr auto jump = "K";
    };

    static int get_or_add(std::unordered_map<std::string, int>& map, const std::string& search, int add_value);
    static std::size_t next_token(const std::string& code, std::size_t index, Token& result);
    static std::optional<OpCode> get_op_code(const std::string& value);
    static void parse_command(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
    static void parse_label(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
    static void parse_comment(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree);
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
            case TokenType::comment:
                parse_comment(tokens, index, syntax_tree);
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

            if (node.token.token_type == TokenType::label)
            {
                // Get the label number if already found, otherwise add.
                code.op_code = OpCode::label;
                code.a = get_or_add(label_map, node.token.value, (int)label_map.size());

                byte_code.push_back(code);
            }
            else if (node.token.token_type == TokenType::command)
            {
                auto maybe_op_code = get_op_code(node.token.value);
                if (!maybe_op_code)
                {
                    throw SyntaxError("Invalid command", node.token.index, node.token.value);
                }

                code.op_code = *maybe_op_code;

                if (maybe_op_code == OpCode::jump)
                {
                    if (node.children.empty() || node.children[0].token.token_type != TokenType::label)
                    {
                        throw SyntaxError("Jump must be followed by label", node.token.index, node.token.value);
                    }

                    // Get the label number if already found, otherwise add. The label number is not the actual position,
                    // so it can be assigned a number before the label op_code is found.
                    code.a = get_or_add(label_map, node.children[0].token.value, (int)label_map.size());
                }

                byte_code.push_back(code);
            }
        }

        return byte_code;
    }

    static std::size_t next_token(const std::string& code, std::size_t index, Token& result)
    {
        static std::regex label_regex(R"_(![SHREK]+!)_");
        static std::regex cmd_regex(R"_([SHREK])_");
        static std::regex whitespace_regex(R"_(\s+)_");
        static std::regex comment_regex(R"_(#[^\n]*\n?)_");

        std::smatch m;
        if (std::regex_search(code.begin() + index, code.end(), m, label_regex, std::regex_constants::match_continuous))
        {
            result.token_type = TokenType::label;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, cmd_regex, std::regex_constants::match_continuous))
        {
            result.token_type = TokenType::command;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, whitespace_regex, std::regex_constants::match_continuous))
        {
            result.token_type = TokenType::whitespace;
        }
        else if (std::regex_search(code.begin() + index, code.end(), m, comment_regex, std::regex_constants::match_continuous))
        {
            result.token_type = TokenType::comment;
        }
        else
        {
            throw SyntaxError("Invalid token", index, code.substr(index, 1));
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
            case 'S':
                return OpCode::push0;
            case 'H':
                return OpCode::pop;
            case 'R':
                return OpCode::bump;
            case 'E':
                return OpCode::func;
            case 'K':
                return OpCode::jump;
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

        if (token.value == ShrekCommands::jump)
        {
            if (index < tokens.size() && tokens[index].token_type == TokenType::label)
            {
                SyntaxTreeNode child_label;
                child_label.token = tokens[index];
                ++index;

                //node.children.push_back(std::make_unique<SyntaxTreeNode>(std::move(child_label)));
                node.children.push_back(std::move(child_label));
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
        ++index;

        tree.syntax.push_back(std::move(node));
    }

    static void parse_comment(const std::vector<Token>& tokens, std::size_t& index, SyntaxTree& tree)
    {
        assert(index < tokens.size());

        SyntaxTreeNode node;
        node.token = tokens[index];
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
