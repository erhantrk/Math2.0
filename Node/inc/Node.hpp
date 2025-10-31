//
// Created by Erhan Türker on 10/12/25.
//

#pragma once
#include <vector>
#include <memory>
#include "../../Lexer/inc/Lexer.hpp"

class Parser;

struct Node {
    enum class Type { Number, Variable, Operand, Function, Assignment, FunctionAssignment, Parameter, FunctionExpression};

    Type type;
    std::string value;
    std::vector<std::shared_ptr<Node>> children;

    Node(Type t, std::string v) : type(t), value(std::move(v)) {}

    static std::shared_ptr<Node> createNode(const Token &token, const Parser& parser);

    template<typename Func>
    void apply(Func fun) {fun(this); for (const auto& child : children) if (child) child->apply(fun);}
    [[nodiscard]] std::shared_ptr<Node> clone() const;
};
