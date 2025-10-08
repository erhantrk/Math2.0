//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <vector>

#include "Lexer.hpp"

struct Node {
    enum class Type {Number, Variable, Operand, Function};
    Type type;
    std::string value;
    std::vector<std::unique_ptr<Node>> children;
};

class Parser {
public:
    static bool isPreDefinedFunction(const Token& token);
    static std::unique_ptr<Node> parseExpression(Lexer& lexer, int min_bp);
};
