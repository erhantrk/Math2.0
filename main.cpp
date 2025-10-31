#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"
#include "Evaluator/inc/Evaluator.hpp"
#include "SymbolicEvaluator/inc/SymbolicEvaluator.hpp"   // <-- Include new expander
#include "Util/ASTPrint.hpp" // <-- Include new printer

int main() {
    std::string input = "g(x)=atan2(sin x, cos x^2)\n"
                        "f(x, y) = x*y*e^(x+y)\n"
                        "a = -1\n"
                        "g(f(x, a)) \n";

    Lexer lexer(input);
    if (!lexer.getError().empty()) {
        std::cerr << "Lexer Error: " << lexer.getError() << std::endl;
        return 1;
    }

    Parser parser;
    auto ast = parser.parse(lexer);

    if (!parser.getError().empty()) {
        std::cerr << "Parser Error: " << parser.getError() << std::endl;
        return 1;
    }

    Evaluator evaluator;
    SymbolicEvaluator sEvaluator; // <-- Create an expander instance

    for (const auto& node : ast) {
        // 1. Register function definitions with BOTH
        if (node->type == Node::Type::FunctionAssignment) {
            evaluator.evaluate(node); // This registers the function in the evaluator
            sEvaluator.registerFunction(node); // Register function in the expander
            std::cout << "Defined: " << toHumanReadable(node) << std::endl;
            continue;
        }

        // 2. Try to evaluate numerically
        double val = evaluator.evaluate(node);
        std::string evalError = evaluator.getError();

        if (evalError.empty()) {
            // Success! Print the number
            std::cout << toHumanReadable(node) << " = " << val << std::endl;
            if (node->type == Node::Type::Assignment) {
                sEvaluator.registerVariable(std::make_pair(node->value, val));
            }
        } else {
            std::cout << "Expanding: " << toHumanReadable(node) << std::endl;
            auto expandedNode = sEvaluator.expand(node);
            std::cout << "--> " << toLisp(expandedNode) << std::endl;
            std::cout << "--> " << toHumanReadable(expandedNode) << std::endl;
        }
    }
    return 0;
}