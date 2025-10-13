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

public:
    explicit Lexer(const std::string &input);

    [[nodiscard]] std::string getError() const;

    [[nodiscard]] const Token &peek(int n = 0) const;

    Token next();

    void skip();

    void addToken(const Token &token);
};
