//
// Created by Erhan Türker on 10/29/25.
//
#pragma once

#include <string>
#include <memory>
#include <sstream>
#include <map>
#include "../../Node/inc/Node.hpp"

static void toLispImpl(const Node* n, std::ostringstream& out) {
    if (!n) {
        out << "<null>";
        return;
    }

    switch (n->type) {
        case Node::Type::Number: {
            std::string s = n->value;
            size_t dot_pos = s.find('.');
            if (dot_pos != std::string::npos) {
                s.erase(n->value.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') {
                    s.pop_back();
                }
            }
            out << s;
        }
            break;
        case Node::Type::Variable:
            out << n->value;
            break;
        case Node::Type::Parameter: {
            size_t separator_pos = n->value.find('-');
            if (separator_pos != std::string::npos) {
                out << n->value.substr(separator_pos + 1);
            } else {
                out << n->value;
            }
            break;
        }
        case Node::Type::Assignment:
            out << "(= " << n->value << " ";
            toLispImpl(n->children[0].get(), out);
            out << ")";
            break;
        case Node::Type::Operand:
            if (n->value == "!" && !n->children.empty()) {
                out << "(! ";
                toLispImpl(n->children[0].get(), out);
                out << ")";
            } else {
                out << "(" << n->value;
                for (size_t i = 0; i < n->children.size(); ++i) {
                    out << " ";
                    toLispImpl(n->children[i].get(), out);
                }
                out << ")";
            }
            break;
        case Node::Type::Function:
        case Node::Type::FunctionAssignment:
            out << "(" << n->value;
            if (!n->children.empty()) {
                out << " ";
                for (size_t i = 0; i < n->children.size(); ++i) {
                    if (i) out << " ";
                    toLispImpl(n->children[i].get(), out);
                }
            }
            out << ")";
            break;
        default: break;
    }
}

static std::string toLisp(const std::shared_ptr<Node>& n) {
    std::ostringstream out;
    toLispImpl(n.get(), out);
    return out.str();
}


enum Precedence {
    PREC_NONE = 0,
    PREC_SUM = 1,        // + -
    PREC_PRODUCT = 2,    // * /
    PREC_POWER = 3,      // ^
    PREC_UNARY = 4,      // ! - (unary)
    PREC_CALL = 5,       // f(x)
    PREC_ATOM = 6        // Numbers, variables
};

static int getOperatorPrecedence(const std::string& op) {
    static const std::map<std::string, int> precMap = {
        {"+", PREC_SUM},         {"-", PREC_SUM},
        {"*", PREC_PRODUCT},     {"/", PREC_PRODUCT},
        {"^", PREC_POWER}
    };
    auto it = precMap.find(op);
    if (it != precMap.end()) {
        return it->second;
    }
    return PREC_NONE;
}

static bool isUnary(const Node* n) {
    if (n->type != Node::Type::Operand) return false;
    return (n->value == "!" || n->value == "-") && n->children.size() == 1;
}

static int getNodePrecedence(const Node* n) {
    if (!n) return PREC_NONE;
    switch (n->type) {
        case Node::Type::Number:
        case Node::Type::Variable:
        case Node::Type::Parameter:
        case Node::Type::Function:
        case Node::Type::FunctionExpression:
        case Node::Type::Derivative:
            return PREC_ATOM;
        case Node::Type::Assignment:
        case Node::Type::FunctionAssignment:
            return PREC_NONE;
        case Node::Type::Operand:
            if (isUnary(n)) return PREC_UNARY;
            return getOperatorPrecedence(n->value);
        default:
            return PREC_NONE;
    }
}

// Checks if the node n (in the context of parentPrec) starts with a character
// that is safe for implicit multiplication (like a letter or '(').
// Returns false if it starts with a digit or operator sign.
static bool isSafeForImplicit(const Node* n, int parentPrec) {
    if (!n) return true;

    // If precedence requires parentheses, it will start with '(', which is safe.
    int nPrec = getNodePrecedence(n);
    if (nPrec < parentPrec) return true;

    // If not parenthesized, check what it starts with.
    if (n->type == Node::Type::Number) return false; // Starts with digit

    if (n->type == Node::Type::Operand) {
        // Unary +, - start with sign.
        if (n->value == "+" || n->value == "-") {
            // Unary or Binary start with sign or number
            if (n->children.size() == 1) return false;
            // Binary: Check left child
            return isSafeForImplicit(n->children[0].get(), nPrec + 1);
        }
        if (n->value == "!") {
            // Postfix ! starts with child
            return isSafeForImplicit(n->children[0].get(), nPrec);
        }
        // Binary *, /, ^ start with left child
        int opPrec = getOperatorPrecedence(n->value);
        return isSafeForImplicit(n->children[0].get(), opPrec + 1);
    }

    return true; // Variables, Functions, etc. are safe
}

static void toHumanReadableImpl(const Node* n, std::ostringstream& out, int parentPrecedence) {
    if (!n) {
        out << "<null>";
        return;
    }

    switch (n->type) {
        case Node::Type::Number: {
            std::string s = n->value;
            size_t dot_pos = s.find('.');
            if (dot_pos != std::string::npos) {
                s.erase(n->value.find_last_not_of('0') + 1, std::string::npos);
                if (s.back() == '.') {
                    s.pop_back();
                }
            }
            out << s;
        }
            break;
        case Node::Type::Variable:
            out << n->value;
            break;
        case Node::Type::Parameter: {
            size_t separator_pos = n->value.find('-');
            if (separator_pos != std::string::npos) {
                out << n->value.substr(separator_pos + 1);
            } else {
                out << n->value;
            }
            break;
        }
        case Node::Type::Assignment: {
            int currentPrec = 0;
            if (currentPrec < parentPrecedence) out << "(";

            out << n->value << " = ";
            toHumanReadableImpl(n->children[0].get(), out, currentPrec);

            if (currentPrec < parentPrecedence) out << ")";
            break;
        }
        case Node::Type::Operand: {
            const std::string& op = n->value;

            if (isUnary(n)) {
                int currentPrec = PREC_UNARY;
                if (currentPrec < parentPrecedence) out << "(";
                if (op == "!") {
                    toHumanReadableImpl(n->children[0].get(), out, currentPrec);
                    out << op;
                }else {
                    out << op;
                    toHumanReadableImpl(n->children[0].get(), out, currentPrec);
                }
                if (currentPrec < parentPrecedence) {
                    out << ")";
                }
            }
            else {
                int currentPrec = getOperatorPrecedence(op);
                if (currentPrec < parentPrecedence) out << "(";
                toHumanReadableImpl(n->children[0].get(), out, currentPrec+1);

                for (size_t i = 1; i < n->children.size(); ++i) {
                    if (op == "*") {
                        // Check if we can use implicit multiplication.
                        // We need explicit * if the right-hand side looks like a number or operator.
                        if (!isSafeForImplicit(n->children[i].get(), currentPrec + 1)) {
                            out << "*";
                        }
                        // Else print nothing (implicit)
                    } else if (op == "+" || op == "-") {
                        out << " " << op << " ";
                    } else {
                        out << op;
                    }
                    toHumanReadableImpl(n->children[i].get(), out, currentPrec+1);
                }
                if (currentPrec < parentPrecedence) out << ")";
            }
            break;
        }
        case Node::Type::Function: {
            out << n->value << "(";
            for (size_t i = 0; i < n->children.size(); ++i) {
                if (i > 0) out << ", ";
                toHumanReadableImpl(n->children[i].get(), out, PREC_NONE);
            }
            out << ")";
            break;
        }
        case Node::Type::FunctionAssignment: {
            int currentPrec = 0;
            if (currentPrec < parentPrecedence) out << "(";

            out << n->value;
            for (size_t i = 0; i < n->children.size() - 1; ++i) {
                if (i > 0) out << ", ";
                toHumanReadableImpl(n->children[i].get(), out, PREC_NONE);
            }
            out << " = ";

            if (!n->children.empty()) {
                toHumanReadableImpl(n->children.back().get(), out, currentPrec);
            }

            if (currentPrec < parentPrecedence) out << ")";
            break;
        }
        default:
            break;
    }
}

static std::string toHumanReadable(const std::shared_ptr<Node>& n) {
    std::ostringstream out;
    toHumanReadableImpl(n.get(), out, PREC_NONE);
    return out.str();
}