#include "../inc/Lexer.hpp"

#include <iostream>
#include <sstream>
#include <utility>

using Type = Token::Type;

Lexer::Lexer(const std::string &input) {
    std::vector<std::pair<Type, std::regex> > token_specification = {
        {Type::Number, std::regex(R"(\d+(\.\d*)?([eE][+-]?\d+)?)")},
        {Type::Symbol, std::regex(R"([!=\+\-\*/\^\(\)])")},
        {Type::Word, std::regex(R"(\b(pi|sin|cos|tan|log|ln|sqrt|abs|atan2)\b)")},
        {Type::Word, std::regex(R"((_\w*)|([a-zA-Z]_\w*)|([a-zA-Z]))")},
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

Token Lexer::peek(int n) const {
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

void Lexer::skip(int n) {
    for (size_t i = 0; i < n; i++) {
    tokens.pop_back();
    }
}

Lexer Lexer::getSubLexer(int pos) {
    return Lexer(std::vector<Token>{tokens.begin() + static_cast<int>(tokens.size()) - pos, tokens.end()});
}

void Lexer::addToken(const Token &token) {
    tokens.push_back(token);
}

std::string Lexer::getError() const {
    return error;
}

Lexer::Lexer(const std::vector<Token>& tokens)
    : tokens(tokens) {
}

int Lexer::getIndexFirstInstance(Token::Type type) {
    for (int i = static_cast<int>(tokens.size()) - 1; i >= 0; --i) {
        if (tokens[i].type == type) {
            return static_cast<int>(tokens.size()) - i - 1;
        }
    }
    return -1;
}

void Lexer::removeToken(int pos) {
    tokens.erase(tokens.begin() + static_cast<int>(tokens.size()) - pos - 1);
}

int Lexer::getClosingParenthesesIndex() const {
    int plCnt = 0;
    for (int i = static_cast<int>(tokens.size()) - 1; i >= 0; --i) {
        if (tokens[i].type == Type::Symbol && tokens[i].value[0] == '(') {
            plCnt++;
        }
        if (tokens[i].type == Type::Symbol && tokens[i].value[0] == ')') {
            if (!plCnt--)
                return static_cast<int>(tokens.size()) - i - 1;
        }
    }
    return -1;
}
