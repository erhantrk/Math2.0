//
// Created by Erhan Türker on 10/31/25.
//

#pragma once

#include <memory>
#include <string>
#include <cmath>
#include "../Node/inc/Node.hpp"

static bool isNumber(const std::shared_ptr<Node>& node) {
    return node && node->type == Node::Type::Number;
}

static double getValue(const std::shared_ptr<Node>& node) {
    if (!isNumber(node)) return NAN;
    try {
        return std::stod(node->value);
    } catch (const std::exception&) {
        return NAN;
    }
}

static double factorial(double n) {
    if (n < 0) return NAN;
    if (n > 170) return INFINITY; // Clamps to infinity
    return std::tgamma(n + 1);
}

static std::shared_ptr<Node> simplifyNode(std::shared_ptr<Node> node) { // NOLINT(*-no-recursion)
    if (!node) {
        return nullptr;
    }

    // 1. Simplify all children first (post-order traversal)
    for (auto& child : node->children) {
        child = simplifyNode(child);
    }

    if (node->type != Node::Type::Operand) {
        return node; // Not an operation, can't simplify further
    }

    // 2. --- Constant Folding ---
    // (This is needed *after* expansion, e.g., f(1,2) -> 1+2 -> 3)
    bool allChildrenAreNumbers = !node->children.empty();
    for (const auto& child : node->children) {
        if (!isNumber(child)) {
            allChildrenAreNumbers = false;
            break;
        }
    }

    if (allChildrenAreNumbers) {
        try {
            const std::string& op = node->value;
            if (op == "+" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) + getValue(node->children[1]));
            }
            if (op == "-" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) - getValue(node->children[1]));
            }
            if (op == "*" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) * getValue(node->children[1]));
            }
            if (op == "/" && node->children.size() == 2) {
                double divisor = getValue(node->children[1]);
                if (divisor == 0.0) return node;
                return Node::createNode(getValue(node->children[0]) / divisor);
            }
            if (op == "^" && node->children.size() == 2) {
                return Node::createNode(std::pow(getValue(node->children[0]), getValue(node->children[1])));
            }
            if (op == "-" && node->children.size() == 1) {
                return Node::createNode(-getValue(node->children[0]));
            }
            if (op == "!" && node->children.size() == 1) {
                return Node::createNode(factorial(getValue(node->children[0])));
            }
        } catch (const std::exception& _) {
            return node; // Failed to fold
        }
    }

    // 3. --- Algebraic Simplification (Identities) ---
    if (node->children.size() == 2) {
        auto& lhs = node->children[0];
        auto& rhs = node->children[1];
        const std::string& op = node->value;

        if (op == "*") {
            // x * 1 -> x
            if (isNumber(rhs) && getValue(rhs) == 1.0) return lhs;
            // 1 * x -> x
            if (isNumber(lhs) && getValue(lhs) == 1.0) return rhs;
            // x * 0 -> 0 or 0 * x -> 0
            if ((isNumber(rhs) && getValue(rhs) == 0.0) || (isNumber(lhs) && getValue(lhs) == 0.0))
                return Node::createNode(0.0);
        }
        else if (op == "+") {
            // x + 0 -> x
            if (isNumber(rhs) && getValue(rhs) == 0.0) return lhs;
            // 0 + x -> x
            if (isNumber(lhs) && getValue(lhs) == 0.0) return rhs;
        }
        else if (op == "-") {
            // x - 0 -> x
            if (isNumber(rhs) && getValue(rhs) == 0.0) return lhs;
            // 0 - x -> -x
            if (isNumber(lhs) && getValue(lhs) == 0.0) {
                auto minus = Node::createNode(Token(Token::Type::Symbol,"-"));
                minus->children.push_back(rhs);
                rhs = std::move(minus);
                return rhs;
            }

        }
        else if (op == "^") {
            // x ^ 1 -> x
            if (isNumber(rhs) && getValue(rhs) == 1.0) return lhs;
            // x ^ 0 -> 1
            if (isNumber(rhs) && getValue(rhs) == 0.0) return Node::createNode(1.0);
            // 1 ^ x -> 1
            if (isNumber(lhs) && getValue(lhs) == 1.0) return Node::createNode(1.0);
        }
    }

    return node;
}

static std::shared_ptr<Node> simplify(const std::shared_ptr<Node>& node) {
    if (!node) {
        return nullptr;
    }
    auto clonedNode = node->clone();
    return simplifyNode(clonedNode);
}