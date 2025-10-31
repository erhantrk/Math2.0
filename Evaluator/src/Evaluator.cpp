//
// Created by Erhan Türker on 10/17/25.
//
#include "../inc/Evaluator.hpp"
#include <cmath>
#include <stdexcept>
#include <string>
#include "../../Util/ASTUtil.hpp"

double Evaluator::evaluate(const std::shared_ptr<Node>& node) {
    error.clear();
    callStack.clear();
    try {
        if (!node) {
            throw std::runtime_error("Cannot evaluate a null AST node.");
        }
        return evaluateNode(node);
    } catch (const std::runtime_error& e) {
        error = e.what();
        return NAN;
    }
}

std::string Evaluator::getError() const {
    return error;
}

double Evaluator::evaluateNode(const std::shared_ptr<Node>& node) { // NOLINT(*-no-recursion)
    if (!node) {
        throw std::runtime_error("Encountered a null node during evaluation.");
    }

    switch (node->type) {
        case Node::Type::Number:
            return std::stod(node->value);

        case Node::Type::Variable: {
            if (node->value == "pi") return M_PI;
            if (node->value == "e") return M_E;
            if (!variables.contains(node->value)) {
                throw std::runtime_error("Undefined variable: '" + node->value + "'");
            }
            return variables.at(node->value);
        }

        case Node::Type::Assignment: {
            double value = evaluateNode(node->children[0]);
            variables[node->value] = value;
            return value;
        }

        case Node::Type::FunctionAssignment:
            functions[node->value] = node;
            return NAN;

        case Node::Type::Operand: {
            if (node->value == "+") {
                if (node->children.size() == 1) return evaluateNode(node->children[0]);
                return evaluateNode(node->children[0]) + evaluateNode(node->children[1]);
            }
            if (node->value == "-") {
                if (node->children.size() == 1) return -evaluateNode(node->children[0]);
                return evaluateNode(node->children[0]) - evaluateNode(node->children[1]);
            }
            if (node->value == "*") return evaluateNode(node->children[0]) * evaluateNode(node->children[1]);
            if (node->value == "/") {
                double divisor = evaluateNode(node->children[1]);
                if (divisor == 0) throw std::runtime_error("Division by zero.");
                return evaluateNode(node->children[0]) / divisor;
            }
            if (node->value == "^") return std::pow(evaluateNode(node->children[0]), evaluateNode(node->children[1]));
            if (node->value == "!") return factorial(evaluateNode(node->children[0]));
            throw std::runtime_error("Unknown operand: " + node->value);
        }

        case Node::Type::Function: {
            if (node->value == "sin") return std::sin(evaluateNode(node->children[0]));
            if (node->value == "cos") return std::cos(evaluateNode(node->children[0]));
            if (node->value == "tan") return std::tan(evaluateNode(node->children[0]));
            if (node->value == "sqrt") return std::sqrt(evaluateNode(node->children[0]));
            if (node->value == "log") return std::log10(evaluateNode(node->children[0]));
            if (node->value == "ln") return std::log(evaluateNode(node->children[0]));
            if (node->value == "abs") return std::abs(evaluateNode(node->children[0]));
            if (node->value == "atan2") {
                if (node->children.size() < 2) throw std::runtime_error("atan2 requires two arguments.");
                return std::atan2(evaluateNode(node->children[0]), evaluateNode(node->children[1]));
            }

            auto it = functions.find(node->value);
            if (it != functions.end()) {
                if (auto defNode = it->second.lock()) {
                    std::vector<double> evaluatedArgs;
                    for (const auto& arg_node : node->children) {
                        evaluatedArgs.push_back(evaluateNode(arg_node));
                    }

                    callStack.push_back({evaluatedArgs});

                    double result = evaluateNode(defNode->children[0]);

                    callStack.pop_back();
                    return result;

                } else {
                    functions.erase(it);
                    throw std::runtime_error("Attempted to call a function that no longer exists.");
                }
            }

            throw std::runtime_error("Unknown function: '" + node->value + "'");
        }

        case Node::Type::Parameter: {
            if (callStack.empty()) {
                throw std::runtime_error("Found a parameter node outside of a function call context.");
            }
            size_t separator_pos = node->value.find('-');
            if (separator_pos == std::string::npos) {
                throw std::runtime_error("Invalid parameter format: " + node->value);
            }
            int index = std::stoi(node->value.substr(0, separator_pos));

            const auto& frame = callStack.back();
            if (index >= frame.evaluatedArguments.size()) {
                 throw std::runtime_error("Function argument index out of bounds.");
            }
            return frame.evaluatedArguments[index];
        }

        default:
            throw std::runtime_error("Cannot evaluate this node type.");
    }
}

