#include <iostream>

#include "Lexer.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

Token::Type fn(Token::Type type) {
    return type;
}
int main() {
    Lexer lexer("x = (\n 4 + 5 / \n (3 + 2))");
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


