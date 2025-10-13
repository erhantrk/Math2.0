//
// Created by Erhan Türker on 10/12/25.
//

#pragma once
#include <string>

struct Token {
    enum class Type { Number, Symbol, Word, Skip, Newline, Comma, Eof };

    Type type;
    std::string value;
    int line;
    int pos;
    std::string line_content;

    Token(const Type &type, const std::string &value, int line, int pos, const std::string &line_content);
    static bool isTokenPostFix(const Token &token);
    static bool isTokenPreFix(const Token &token);
    static bool isNewline(const Token &token);
};

std::ostream &operator<<(std::ostream &os, const Token &token);
