//
// Created by Erhan Türker on 10/6/25.
//

#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <vector>

struct Token {
    enum class Type {Number, Symbol, Word, Skip, Newline, Eof};
    Type type;
    std::string value;
    int line;
    int pos;
    std::string line_content;

    Token(const Type& type, const std::string& value, int line, int pos, const std::string& line_content);
};

std::ostream& operator<<(std::ostream& os, const Token& token);

class Lexer {
    std::string error;
    std::vector<Token> tokens;
    static inline Token Eof{Token::Type::Eof, "", 0, 0, ""};
public:
    explicit Lexer(const std::string& input);
    std::string getError() const;
    const Token&  peek() const;
    const Token&  next();
    void skip();
    void addToken(const Token& token);
};