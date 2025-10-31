//
// Created by Erhan Türker on 10/12/25.
//

#include "../inc/Node.hpp"
#include "../../Parser/inc/Parser.hpp"

std::shared_ptr<Node> Node::createNode(const Token &token, const Parser& parser) {
    switch (token.type) {
        case Token::Type::Word:
            if (parser.isDefinedFunction(token))
                return std::make_shared<Node>(Node{Node::Type::Function, token.value});;
            return std::make_shared<Node>(Node{Node::Type::Variable, token.value});
        case Token::Type::Number: return std::make_unique<Node>(Node{Node::Type::Number, token.value});
        case Token::Type::Symbol:
            if (token.value[0] == '=')
                return std::make_shared<Node>(Node{Node::Type::Assignment, token.value});
            return std::make_shared<Node>(Node{Node::Type::Operand, token.value});
        default: return nullptr;
    }
}

std::shared_ptr<Node> Node::clone() const {
    auto newNode = std::make_shared<Node>(this->type, this->value);

    for (const auto& child : this->children) {
        if (child) {
            newNode->children.push_back(child->clone());
        } else {
            newNode->children.push_back(nullptr);
        }
    }
    return newNode;
}