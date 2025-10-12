//
// Created by Erhan Türker on 10/12/25.
//

#pragma once
#include <vector>
#include <memory>
#include "../../Lexer/inc/Lexer.hpp"

struct Node {
    enum class Type { Number, Variable, Operand, Function, Assignment };

    Type type;
    std::string value;
    std::vector<std::unique_ptr<Node> > children;

    static std::unique_ptr<Node> createNode(const Token &token);
};
