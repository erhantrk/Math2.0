#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <memory>

#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"
#include "Evaluator/inc/Evaluator.hpp"

int main() {
    std::string input = "f(x)=x\n"
                        "f(2)";

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

    for (const auto& node : ast) {
        double val = evaluator.evaluate(node);
        if (val != NAN) std::cout << val << std::endl;
    }
    return 0;
}