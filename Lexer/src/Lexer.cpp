#include "../inc/Lexer.hpp"
#include <sstream>

using Type = Token::Type;


Lexer::Lexer(const std::string &input) {
    std::vector<std::pair<Type, std::regex> > token_specification = {
        {Type::Number, std::regex(R"(\d+(\.\d+)?)")},
        {Type::Symbol, std::regex(R"([!=\+\-\*/\^\(\)])")},
        {Type::Word, std::regex(R"([a-zA-Z_]\w*)")},
        {Type::Newline, std::regex(R"(\n)")},
        {Type::Skip, std::regex(R"([ \t\r]+)")},
        {Type::Comma, std::regex(R"(,)")},

    };

    size_t position = 0;
    int line_number = 1;

    while (position < input.length()) {
        bool matched = false;
        for (const auto &[type, re]: token_specification) {
            std::smatch match;
            if (std::regex_search(input.cbegin() + position, input.cend(), match, re,
                                  std::regex_constants::match_continuous)) {
                if (type == Type::Newline) {
                    size_t line_start = input.rfind('\n', position - 1);
                    line_start = (line_start == std::string::npos) ? 0 : line_start + 1;

                    std::string line_content = input.substr(line_start, position - line_start);

                    size_t column = position - line_start;

                    tokens.emplace_back(type, match.str(), line_number, column, line_content);
                    line_number++;
                } else if (type != Type::Skip) {
                    size_t line_start = input.rfind('\n', position);
                    line_start = (line_start == std::string::npos) ? 0 : line_start + 1;
                    size_t line_end = input.find('\n', position);
                    line_end = (line_end == std::string::npos) ? input.length() : line_end;
                    std::string line_content = input.substr(line_start, line_end - line_start);

                    tokens.emplace_back(type, match.str(), line_number, position - line_start, line_content);
                }

                position += match.length();
                matched = true;
                break;
            }
        }
        if (!matched) {
            size_t line_start = input.rfind('\n', position);
            line_start = (line_start == std::string::npos) ? 0 : line_start + 1;
            size_t line_end = input.find('\n', position);
            std::string line_content = input.substr(line_start, line_end - line_start);

            std::string offending(1, input[position]);
            std::ostringstream err_oss;
            err_oss << "Unexpected character \"" << offending << "\" at line " << line_number <<
                    ", column " << (position - line_start) << ".\n";
            err_oss << "    " << line_content << "\n";
            err_oss << "    " << std::string(position - line_start, ' ') << "^-- This should not be here.";
            error = err_oss.str();
            tokens.clear();
            return;
        }
    }

    error = "";
    this->tokens = std::vector<Token>{this->tokens.rbegin(), this->tokens.rend()};
}

const Token &Lexer::peek(int n) const {
    if (tokens.size() <= n)
        return Eof;
    return tokens[tokens.size() - 1 - n];
}

Token Lexer::next() {
    if (tokens.empty()) return Eof;
    Token token = tokens.back();
    tokens.pop_back();
    return token;
}

void Lexer::skip() {
    tokens.pop_back();
}

void Lexer::addToken(const Token &token) {
    tokens.push_back(token);
}

std::string Lexer::getError() const {
    return error;
}