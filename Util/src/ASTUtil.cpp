//
// Created by Erhan Türker on 10/31/25.
//

#include <memory>
#include <string>
#include <cmath>
#include <list>

#include "../../Node/inc/Node.hpp"
#include "../inc/ASTPrint.hpp"
#include "../inc/ASTUtil.hpp"


bool isNumber(const std::shared_ptr<Node>& node) {
    return node && node->type == Node::Type::Number;
}

double getValue(const std::shared_ptr<Node>& node) {
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

std::string takeNegative(const std::string& value) {
    return value[0] == '-' ? value.substr(1) : '-' + value;
}