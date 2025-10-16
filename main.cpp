#include <iostream>

#include "Lexer/inc/Lexer.hpp"
#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"

int main() {
    Lexer lexer("(1 + \n \n 2)");
    if (!lexer.getError().empty()) {
        std::cerr << lexer.getError() << std::endl;
        return 1;
    }
    Parser parser;
    auto root = parser.parse(lexer);
    if (root.empty()) {
        std::cerr << parser.getError() << std::endl;
    }
    return 0;
}
