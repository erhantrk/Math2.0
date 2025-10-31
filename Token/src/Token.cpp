//
// Created by Erhan Türker on 10/12/25.
//

#include "../inc/Token.hpp"
#include <sstream>

using Type = Token::Type;

Token::Token(const Type &type, const std::string &value, int line, int pos, const std::string &line_content)
    : type(type), value(value), line(line), pos(pos), line_content(line_content) {
}

Token::Token(const Type &type, const std::string &value)
    : type(type), value(value), line(0), pos(0){
}

std::ostream &operator<<(std::ostream &os, const Token &token) {
    std::string tokenStr;
    switch (token.type) {
        case Type::Number: tokenStr = "Number";
            break;
        case Type::Symbol: tokenStr = "Symbol";
            break;
        case Type::Word: tokenStr = "Word";
            break;
        case Type::Skip: tokenStr = "Skip";
            break;
        case Type::Eof: tokenStr = "Eof";
            break;
        default: tokenStr = "Unknown";
    }
    os << "Token(" << tokenStr << ", '" << token.value << "', L" << token.line << " P" << token.pos << ")";
    return os;
}


bool Token::isNewline(const Token &token) {
    return token.type == Token::Type::Newline;
}

bool Token::isTokenPostFix(const Token &token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '!': return true;
        default: return false;
    }
}

bool Token::isTokenPreFix(const Token &token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '-':
        case '+': return true;
        default: return false;
    }
}