#include <iostream>

#include "Lexer/inc/Lexer.hpp"
#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"

int main() {
    Lexer lexer("x = 2 \n y = 3 \n t = x + y");
    if (!lexer.getError().empty()) {
        std::cerr << lexer.getError() << std::endl;
        return 1;
    }

    auto root = Parser::parse(lexer);
    if (root.empty()) {
        std::cerr << Parser::getError() << std::endl;
    }
    return 0;
}
