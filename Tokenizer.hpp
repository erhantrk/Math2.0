//
// Created by Erhan Türker on 10/6/25.
//

#pragma once

#include <iostream>
#include <regex>
#include <string>
#include <vector>

struct Token {
    std::string type;
    std::string value;
    int line;

    Token(const std::string& type, const std::string& value, int line);
};

std::ostream& operator<<(std::ostream& os, const Token& token);

class Tokenizer {
public:
    static std::pair<std::string, std::vector<Token>> tokenize(const std::string& input);
};
