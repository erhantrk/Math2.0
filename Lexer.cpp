#include "Lexer.hpp"

#include <bsm/audit.h>
using Type = Token::Type;

Lexer::Lexer(const std::string &input) {
    std::vector<std::pair<Type, std::string>> token_specification = {
        {Type::Number,  R"(\d+(\.\d+)?)"},
        {Type::Symbol,  R"([!=\+\-\*/\^\(\)])"},
        {Type::Word,    R"([a-zA-Z_]\w*)"},
        {Type::Skip,    R"([ \t]+)"},
        {Type::Newline, R"(\n)"}
    };

    std::vector<std::pair<Token::Type, std::regex>> regexes;
    regexes.reserve(token_specification.size());
    for (const auto& [name, pattern] : token_specification)
        regexes.emplace_back(name, std::regex(pattern));

    int line_number = 1;
    size_t position = 0;

    while (position < input.size()) {
        bool matched = false;

        for (const auto& [type, re] : regexes) {
            std::smatch match;
            std::string remaining = input.substr(position);

            if (std::regex_search(remaining, match, re, std::regex_constants::match_continuous)) {
                std::string value = match.str();
                if (type == Type::Newline) {
                    ++line_number;
                } else if (type != Type::Skip) {
                    tokens.emplace_back(type, value, line_number);
                }
                position += value.size();
                matched = true;
                break;
            }
        }

        if (!matched) {
            std::string offending(1, input[position]);

            std::string errorPrefix = "Unexpected character \"" + offending + "\" in input: '";
            std::string errorLine = errorPrefix + input + "' at line " + std::to_string(line_number) + "\n";

            std::string caretLine = std::string(errorPrefix.length() + position, ' ') + "^";

            error = errorLine + caretLine;
            tokens = {};
            return;
        }
    }

    error = "";
    tokens = std::vector<Token>{tokens.rbegin(), tokens.rend()};
}

const Token& Lexer::peek() {
    if (tokens.size() == 0) return Eof;
    Token& token = tokens.back();
    return token;
}
const Token& Lexer::next() {
    if (tokens.size() == 0) return Eof;
    Token& token = tokens.back();
    tokens.pop_back();
    return token;
}

void Lexer::skip() {
    tokens.pop_back();
}

void Lexer::addToken(const Token& token) {
    tokens.push_back(token);
}

std::string Lexer::getError() const {
    return error;
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    std::string tokenStr;
    switch (token.type) {
        case Type::Number: tokenStr =  "Number"; break;
        case Type::Symbol: tokenStr =  "Symbol"; break;
        case Type::Word: tokenStr =    "Word"; break;
        case Type::Skip: tokenStr =    "Skip"; break;
        case Type::Newline: tokenStr = "Newline"; break;
        default: ;
    }
    os << "Token(" << tokenStr << ", " << token.value << ", " << token.line << ")";
    return os;
}

Token::Token(const Type& type, const std::string& value, int line)
    : type(type), value(value), line(line) {}
