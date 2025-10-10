#include "Lexer.hpp"
#include <sstream>

using Type = Token::Type;

Token::Token(const Type& type, const std::string& value, int line, int pos, const std::string& line_content)
    : type(type), value(value), line(line), pos(pos), line_content(line_content) {}

Lexer::Lexer(const std::string &input) {
    std::vector<std::pair<Type, std::regex>> token_specification = {
        {Type::Number,  std::regex(R"(\d+(\.\d+)?)")},
        {Type::Symbol,  std::regex(R"([!=\+\-\*/\^\(\)])")},
        {Type::Word,    std::regex(R"([a-zA-Z_]\w*)")},
        {Type::Skip,    std::regex(R"([ \t]+)")},
    };

    std::stringstream ss(input);
    std::string line_content;
    int line_number = 1;

    while (std::getline(ss, line_content)) {
        size_t position = 0;
        while (position < line_content.length()) {
            bool matched = false;
            for (const auto& [type, re] : token_specification) {
                std::smatch match;
                if (std::regex_search(line_content.cbegin() + position, line_content.cend(), match, re, std::regex_constants::match_continuous)) {
                    if (type != Type::Skip) {
                        tokens.emplace_back(type, match.str(), line_number, position, line_content);
                    }
                    position += match.length();
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                std::string offending(1, line_content[position]);
                std::ostringstream err_oss;
                err_oss << "Lexer Error: Unexpected character \"" << offending << "\" at line " << line_number << ", column " << position << ".\n";
                err_oss << "    " << line_content << "\n";
                err_oss << "    " << std::string(position, ' ') << "^";
                error = err_oss.str();
                tokens.clear();
                return;
            }
        }
        line_number++;
    }

    error = "";
    this->tokens = std::vector<Token>{this->tokens.rbegin(), this->tokens.rend()};
}

const Token& Lexer::peek() const{
    if (tokens.empty()) return Eof;
    const Token& token = tokens.back();
    return token;
}
const Token& Lexer::next() {
    if (tokens.empty()) return Eof;
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
        case Type::Number: tokenStr = "Number"; break;
        case Type::Symbol: tokenStr = "Symbol"; break;
        case Type::Word:   tokenStr = "Word"; break;
        case Type::Skip:   tokenStr = "Skip"; break;
        case Type::Eof:    tokenStr = "Eof"; break;
        default: tokenStr = "Unknown";
    }
    os << "Token(" << tokenStr << ", '" << token.value << "', L" << token.line << " P" << token.pos << ")";
    return os;
}