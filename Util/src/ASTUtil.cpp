//
// Created by Erhan Türker on 10/31/25.
//

#include <memory>
#include <string>
#include <cmath>

#include "../../Node/inc/Node.hpp"
#include "../inc/ASTUtil.hpp"
#include "../../Simplifier/inc/Simplifier.hpp"


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
    if (n > 170) return INFINITY;
    return std::tgamma(n + 1);
}

std::string takeNegative(const std::string& value) {
    return value[0] == '-' ? value.substr(1) : '-' + value;
}

static std::shared_ptr<Node> createNum(double val) {
    return Node::createNode(val);
}

static std::shared_ptr<Node> createOp(const std::string& op) {
    return std::make_shared<Node>(Node::Type::Operand, op);
}

static std::shared_ptr<Node> createFunc(const std::string& func) {
    return std::make_shared<Node>(Node::Type::Function, func);
}

std::shared_ptr<Node> differentiate(const std::shared_ptr<Node>& node, const std::string& var) {
    if (!node) {
        return createNum(0);
    }

    switch (node->type) {
        case Node::Type::Number:
            return createNum(0);

        case Node::Type::Variable: {
            if (node->value == var) {
                return createNum(1);
            } else {
                return createNum(0);
            }
        }

        case Node::Type::Parameter: {
            size_t separator_pos = node->value.find('-');
            std::string paramName = node->value.substr(separator_pos + 1);
            if (paramName == var) {
                return createNum(1);
            } else {
                return createNum(0);
            }
        }

        case Node::Type::Operand: {
            const std::string& op = node->value;
            auto f = node->children[0];

            if (node->children.size() == 1) {
                auto f_prime = differentiate(f, var);
                if (op == "-") {
                    auto minus = createOp("-");
                    minus->children.push_back(f_prime);
                    return Simplifier::simplify(minus);
                }
                return f_prime;
            }

            auto g = node->children[1];
            auto f_prime = differentiate(f, var);
            auto g_prime = differentiate(g, var);

            if (op == "+" || op == "-") {
                auto result = createOp(op);
                result->children.push_back(f_prime);
                result->children.push_back(g_prime);
                return Simplifier::simplify(result);
            }

            if (op == "*") {
                auto mult1 = createOp("*");
                mult1->children.push_back(f_prime);
                mult1->children.push_back(g->clone());

                auto mult2 = createOp("*");
                mult2->children.push_back(f->clone());
                mult2->children.push_back(g_prime);

                auto plus = createOp("+");
                plus->children.push_back(mult1);
                plus->children.push_back(mult2);
                return Simplifier::simplify(plus);
            }

            if (op == "/") {
                auto mult1 = createOp("*");
                mult1->children.push_back(f_prime);
                mult1->children.push_back(g->clone());

                auto mult2 = createOp("*");
                mult2->children.push_back(f->clone());
                mult2->children.push_back(g_prime);

                auto sub = createOp("-");
                sub->children.push_back(mult1);
                sub->children.push_back(mult2);

                auto g_squared = createOp("^");
                g_squared->children.push_back(g->clone());
                g_squared->children.push_back(createNum(2));

                auto div = createOp("/");
                div->children.push_back(sub);
                div->children.push_back(g_squared);
                return Simplifier::simplify(div);
            }

            if (op == "^") {
                if (isNumber(g)) {
                    double n = getValue(g);

                    auto n_node = g->clone();
                    auto n_minus_1 = createNum(n - 1.0);
                    auto f_pow = createOp("^");
                    f_pow->children.push_back(f->clone());
                    f_pow->children.push_back(n_minus_1);

                    auto mult1 = createOp("*");
                    mult1->children.push_back(n_node);
                    mult1->children.push_back(f_pow);

                    auto mult2 = createOp("*");
                    mult2->children.push_back(mult1);
                    mult2->children.push_back(f_prime);
                    return Simplifier::simplify(mult2);
                }

                auto term1_ln = createFunc("ln");
                term1_ln->children.push_back(f->clone());
                auto term1 = createOp("*");
                term1->children.push_back(g_prime);
                term1->children.push_back(term1_ln);

                auto term2_div = createOp("/");
                term2_div->children.push_back(f_prime);
                term2_div->children.push_back(f->clone());
                auto term2 = createOp("*");
                term2->children.push_back(g->clone());
                term2->children.push_back(term2_div);

                auto sum = createOp("+");
                sum->children.push_back(term1);
                sum->children.push_back(term2);

                auto result = createOp("*");
                result->children.push_back(node->clone());
                result->children.push_back(sum);
                return Simplifier::simplify(result);
            }

            return createNum(0);
        }

        case Node::Type::Function: {
            auto g = node->children[0];
            auto g_prime = differentiate(g, var);

            std::shared_ptr<Node> outer_deriv = nullptr;
            const std::string& func = node->value;

            if (func == "sin") {
                outer_deriv = createFunc("cos");
                outer_deriv->children.push_back(g->clone());
            } else if (func == "cos") {
                auto sin_g = createFunc("sin");
                sin_g->children.push_back(g->clone());
                outer_deriv = createOp("-");
                outer_deriv->children.push_back(sin_g);
            } else if (func == "tan") {
                auto cos_g = createFunc("cos");
                cos_g->children.push_back(g->clone());
                auto cos_g_sq = createOp("^");
                cos_g_sq->children.push_back(cos_g);
                cos_g_sq->children.push_back(createNum(2));
                outer_deriv = createOp("/");
                outer_deriv->children.push_back(createNum(1));
                outer_deriv->children.push_back(cos_g_sq);
            } else if (func == "ln") {
                outer_deriv = createOp("/");
                outer_deriv->children.push_back(createNum(1));
                outer_deriv->children.push_back(g->clone());
            } else if (func == "log") {
                auto ln_10 = createFunc("ln");
                ln_10->children.push_back(createNum(10));
                auto den = createOp("*");
                den->children.push_back(g->clone());
                den->children.push_back(ln_10);
                outer_deriv = createOp("/");
                outer_deriv->children.push_back(createNum(1));
                outer_deriv->children.push_back(den);
            } else if (func == "sqrt") {
                auto two_sqrt_g = createOp("*");
                two_sqrt_g->children.push_back(createNum(2));
                two_sqrt_g->children.push_back(node->clone());
                outer_deriv = createOp("/");
                outer_deriv->children.push_back(createNum(1));
                outer_deriv->children.push_back(two_sqrt_g);
            } else if (func == "abs") {
                outer_deriv = createOp("/");
                outer_deriv->children.push_back(g->clone());
                outer_deriv->children.push_back(node->clone());
            } else {
                return createNum(0);
            }

            auto result = createOp("*");
            result->children.push_back(outer_deriv);
            result->children.push_back(g_prime);
            return Simplifier::simplify(result);
        }

        default:
            return createNum(0);
    }
}