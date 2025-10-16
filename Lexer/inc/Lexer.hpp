//
// Created by Erhan Türker on 10/6/25.
//

#pragma once

#include <regex>
#include <string>
#include <vector>

#include "../../Token/inc/Token.hpp"

class Lexer {
    std::string error;
    std::vector<Token> tokens;
    static inline Token Eof{Token::Type::Eof, "", 0, 0, ""};
    explicit Lexer(const std::vector<Token>& tokens);

public:
    explicit Lexer(const std::string &input);

    Lexer getSubLexer(int pos);

    [[nodiscard]] std::string getError() const;

    [[nodiscard]] Token peek(int n = 0) const;

    [[nodiscard]] Token next();

    void skip(int n = 1);

    void addToken(const Token &token);

    int getIndexFirstInstance(Token::Type type);

    int getClosingParenthesesIndex() const;

    void removeToken(int pos);
};
