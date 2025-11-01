//
// Created by Erhan Türker on 10/31/25.
//

#include <memory>
#include <string>
#include <cmath>
#include <list>

#include "../../Node/inc/Node.hpp"
#include "../inc/ASTPrint.hpp"

static bool isNumber(const std::shared_ptr<Node>& node);
static double getValue(const std::shared_ptr<Node>& node);

struct Term {
    double coefficient;
    std::shared_ptr<Node> variablePart; // nullptr for constants

    Term(double coeff, std::shared_ptr<Node> varPart)
        : coefficient(coeff), variablePart(std::move(varPart)) {}

    std::string getKey() const {
        if (variablePart == nullptr) {
            return "##CONST##";
        }
        return toLisp(variablePart);
    }
};

static std::pair<double, std::shared_ptr<Node>> getTermParts(const std::shared_ptr<Node>& node) {
    if (isNumber(node)) {
        return {getValue(node), nullptr};
    }

    if (node->type == Node::Type::Operand && node->value == "*") {
        auto leftParts = getTermParts(node->children[0]);
        auto rightParts = getTermParts(node->children[1]);

        double newCoeff = leftParts.first * rightParts.first;

        std::shared_ptr<Node> newVarPart = nullptr;
        if (leftParts.second == nullptr) {
            newVarPart = rightParts.second;
        } else if (rightParts.second == nullptr) {
            newVarPart = leftParts.second;
        } else {
            newVarPart = std::make_shared<Node>(Node::Type::Operand, "*");
            newVarPart->children.push_back(leftParts.second);
            newVarPart->children.push_back(rightParts.second);
        }

        return {newCoeff, newVarPart};
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 1) {
        auto parts = getTermParts(node->children[0]);
        parts.first *= -1.0;
        return parts;
    }

    return {1.0, node};
}

static void collectSumTermsImpl(const std::shared_ptr<Node>& node, double currentSign, std::list<Term>& terms) {

    if (node->type == Node::Type::Operand && node->value == "+") {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        collectSumTermsImpl(node->children[1], currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 2) {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        collectSumTermsImpl(node->children[1], -currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 1) {
        collectSumTermsImpl(node->children[0], -currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "+" && node->children.size() == 1) {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        return;
    }

    auto parts = getTermParts(node);

    double finalCoefficient = parts.first * currentSign;
    std::shared_ptr<Node> variablePart = parts.second;

    terms.emplace_back(finalCoefficient, variablePart);
}

static std::list<Term> collectSumTerms(const std::shared_ptr<Node>& node) {
    std::list<Term> terms;
    collectSumTermsImpl(node, 1, terms);
    return terms;
}

static bool isSum(const Term& term) {
    if (term.variablePart == nullptr || term.variablePart->type != Node::Type::Operand) return false;
    return term.variablePart->value == "+" || term.variablePart->value == "-";
}

static std::list<Term> expandTerm(const Term& term) {
    if (!isSum(term)) {
        return { term };
    }
    std::list<Term> innerTerms = collectSumTerms(term.variablePart);

    for (auto& innerTerm : innerTerms) {
        innerTerm.coefficient *= term.coefficient;
    }
    return innerTerms;
}

static std::shared_ptr<Node> simplifySum(std::shared_ptr<Node> node) {
    std::list<Term> collectedTerms = collectSumTerms(node);
    std::list<Term> expandedTerms;
    for (const auto& term : collectedTerms) {
        auto distributedTerms = expandTerm(term);
        expandedTerms.splice(expandedTerms.end(), distributedTerms);
    }
    struct TermData {
        double totalCoefficient = 0.0;
        std::shared_ptr<Node> variablePart = nullptr;
    };

    std::map<std::string, TermData> finalTerms;
    for (const auto& term : expandedTerms) {
        std::string key = term.getKey();
        finalTerms[key].totalCoefficient += term.coefficient;
        if (finalTerms[key].variablePart == nullptr) {
            finalTerms[key].variablePart = term.variablePart;
        }
    }

    std::shared_ptr<Node> root = nullptr;
    for (auto const& [key, data] : finalTerms) {

        if (data.totalCoefficient == 0.0) {
            continue;
        }
        std::shared_ptr<Node> termNode = nullptr;

        if (key == "##CONST##") {
            termNode = Node::createNode(data.totalCoefficient);
        } else {
            if (data.totalCoefficient == 1.0) {
                termNode = data.variablePart;
            } else {
                termNode = std::make_shared<Node>(Node::Type::Operand, "*");
                termNode->children.push_back(Node::createNode(data.totalCoefficient));
                termNode->children.push_back(data.variablePart);
            }
        }

        if (root == nullptr) {
            root = termNode;
        } else {
            auto newRoot = std::make_shared<Node>(Node::Type::Operand, "+");
            newRoot->children.push_back(root);
            newRoot->children.push_back(termNode);
            root = newRoot;
        }
    }

    if (root == nullptr) {
        root = Node::createNode(0.0);
    }

    return root;
}

static std::shared_ptr<Node> simplifyNode(std::shared_ptr<Node> node) { // NOLINT(*-no-recursion)
    if (!node) {
        return nullptr;
    }

    for (auto& child : node->children) {
        child = simplifyNode(child);
    }

    if (node->type != Node::Type::Operand) {
        return node;
    }

    // 2. --- DISPATCHER ---
    if (node->type == Node::Type::Operand && (node->value == "+" || node->value == "-")) {
        node = std::move(simplifySum(node));
    }

    // if (node->type == Node::Type::Operand && (node->value == "*" || node->value == "/")) {
    //     return simplifyProduct(node);
    // }

    // 3. --- Algebraic Simplification ---
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

std::shared_ptr<Node> simplify(const std::shared_ptr<Node>& node) {
    if (!node) {
        return nullptr;
    }
    auto clonedNode = node->clone();
    return simplifyNode(clonedNode);
}

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

double factorial(double n) {
    if (n < 0) return NAN;
    if (n > 170) return INFINITY; // Clamps to infinity
    return std::tgamma(n + 1);
}