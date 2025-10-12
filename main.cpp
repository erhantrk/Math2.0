#include <iostream>

#include "Lexer/inc/Lexer.hpp"
#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"

int main() {
    Lexer lexer("y = m*x + c");
    if (!lexer.getError().empty()) {
        std::cerr << lexer.getError() << std::endl;
        return 1;
    }
    Parser parser;
    parser.defineVariable("y");
    parser.defineVariable("m");
    parser.defineVariable("x");
    parser.defineVariable("c");
    auto root = parser.parse(lexer);
    if (root.empty()) {
        std::cerr << parser.getError() << std::endl;
    }
    return 0;
}
