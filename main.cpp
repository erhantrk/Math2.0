#include <iostream>

#include "Lexer/inc/Lexer.hpp"
#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"

int main() {
    Lexer lexer("(2 + 3");
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
