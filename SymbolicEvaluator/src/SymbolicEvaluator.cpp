//
// Created by Erhan Türker on 10/29/25.
//

#include "../inc/SymbolicEvaluator.hpp"

#include <stdexcept>
#include "../../Util/inc/ASTUtil.hpp"
#include "../../Simplifier/inc/Simplifier.hpp"

void SymbolicEvaluator::registerFunction(const std::shared_ptr<Node>& funcDefNode) {
    if (funcDefNode->type == Node::Type::FunctionAssignment) {
        functions[funcDefNode->value] = funcDefNode;
    }
}

void SymbolicEvaluator::registerVariable(std::pair<std::string, double> var) {
    variables.emplace(var);
}

std::shared_ptr<Node> SymbolicEvaluator::expand(const std::shared_ptr<Node>& node) {
    if (!node) {
        return nullptr;
    }
    return expandNode(node);
}

std::shared_ptr<Node> SymbolicEvaluator::expandNode(const std::shared_ptr<Node>& node) { // NOLINT(*-no-recursion)
    if (!node) {
        return nullptr;
    }

    auto expandedNode = node->clone();
    for (auto& child : expandedNode->children) {
        child = expandNode(child);
    }

    if (expandedNode->type == Node::Type::Function) {
        auto it = functions.find(expandedNode->value);

        if (it != functions.end()) {
            auto funcDefNode = it->second; // This is the FunctionAssignment node
            auto funcBody = funcDefNode->children[0]; // This is the body AST
            const auto& arguments = expandedNode->children;
            auto clonedBody = funcBody->clone();
            auto substitutedBody = substituteParameters(clonedBody, arguments);
            return expandNode(substitutedBody);
        }
    }
    else if (expandedNode->type == Node::Type::Variable) {
        auto it = variables.find(expandedNode->value);
        if (it != variables.end()) {
            auto variable = it->second;
            expandedNode->value = std::to_string(variable);
            expandedNode->type = Node::Type::Number;
        }
        if (node->value == "pi") {
            expandedNode->value = std::to_string(M_PI);
            expandedNode->type = Node::Type::Number;
        }
        if (node->value == "e") {
            expandedNode->value = std::to_string(M_E);
            expandedNode->type = Node::Type::Number;
        }
    }

    return Simplifier::simplify(expandedNode);
}


std::shared_ptr<Node> SymbolicEvaluator::substituteParameters(const std::shared_ptr<Node>& body, const std::vector<std::shared_ptr<Node>>& arguments) { // NOLINT(*-no-recursion)
    if (!body) {
        return nullptr;
    }

    if (body->type == Node::Type::Parameter) {
        size_t separator_pos = body->value.find('-');
        if (separator_pos == std::string::npos) {
            throw std::runtime_error("Invalid parameter format: " + body->value);
        }
        int index = std::stoi(body->value.substr(0, separator_pos)); // "0-x" -> 0

        if (index >= arguments.size()) {
            throw std::runtime_error("Function argument index out of bounds.");
        }

        return arguments[index]->clone();
    }

    for (auto& child : body->children) {
        child = substituteParameters(child, arguments);
    }

    return body;
}
