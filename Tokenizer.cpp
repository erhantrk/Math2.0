#include "Tokenizer.hpp"
using Type = Token::Type;
std::pair<std::string, std::vector<Token>> Tokenizer::tokenize(const std::string& input) {
    std::vector<Token> tokens;

    std::vector<std::pair<Type, std::string>> token_specification = {
        {Type::Number,  R"(\d+(\.\d+)?)"},
        {Type::Symbol,  R"([\+\-\*/\^\(\)])"},
        {Type::Assign,  R"(=)"},
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

            return std::make_pair(
                errorLine + caretLine,
                std::vector<Token>{}
            );
        }
    }

    return std::make_pair("", tokens);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    std::string tokenStr;
    switch (token.type) {
        case Type::Number: tokenStr = "Number"; break;
        case Type::Symbol: tokenStr = "Symbol"; break;
        case Type::Assign: tokenStr = "Assign"; break;
        case Type::Word: tokenStr = "Word"; break;
        case Type::Skip: tokenStr = "Skip"; break;
        case Type::Newline: tokenStr = "Newline"; break;
    }
    os << "Token(" << tokenStr << ", " << token.value << ", " << token.line << ")";
    return os;
}

Token::Token(const Type& type, const std::string& value, int line)
    : type(type), value(value), line(line) {}
