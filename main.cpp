#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"
#include "Evaluator/inc/Evaluator.hpp"
#include "SymbolicEvaluator/inc/SymbolicEvaluator.hpp"
#include "Util/inc/ASTPrint.hpp"

int main() {
    std::string input = "atan2(x) = x";
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
    SymbolicEvaluator sEvaluator;

    for (const auto& node : ast) {
        if (node->type == Node::Type::FunctionAssignment) {
            evaluator.evaluate(node);
            sEvaluator.registerFunction(node);
            std::cout << "Defined: " << toHumanReadable(node) << std::endl;
            continue;
        }

        double val = evaluator.evaluate(node);
        std::string evalError = evaluator.getError();

        if (evalError.empty()) {
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