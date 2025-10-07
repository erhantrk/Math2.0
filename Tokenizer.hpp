//
// Created by Erhan Türker on 10/6/25.
//

#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <vector>

struct Token {
    enum class Type {Number, Symbol, Assign, Word, Skip, Newline};
    Type type;
    std::string value;
    int line;

    Token(const Type& type, const std::string& value, int line);
};

std::ostream& operator<<(std::ostream& os, const Token& token);

class Tokenizer {
public:
    static std::pair<std::string, std::vector<Token>> tokenize(const std::string& input);
};
