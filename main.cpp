#include <iostream>

#include "Lexer.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

int main() {
    Lexer lexer("x = 2 / 3 * yi + 2 -3 +f");
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
