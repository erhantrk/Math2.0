//
// Created by Erhan Türker on 10/12/25.
//

#include "../inc/Node.hpp"
#include "../../Parser/inc/Parser.hpp"

std::unique_ptr<Node> Node::createNode(const Token &token) {
    switch (token.type) {
        case Token::Type::Word:
            if (Parser::isPreDefinedFunction(token))
                return std::make_unique<Node>(Node{Node::Type::Function, token.value});;
            return std::make_unique<Node>(Node{Node::Type::Variable, token.value});
        case Token::Type::Number: return std::make_unique<Node>(Node{Node::Type::Number, token.value});
        case Token::Type::Symbol:
            if (token.value[0] == '=')
                return std::make_unique<Node>(Node{Node::Type::Assignment, token.value});
            return std::make_unique<Node>(Node{Node::Type::Operand, token.value});
        default: return nullptr;
    }
}