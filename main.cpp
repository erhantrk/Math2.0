#include <iostream>

#include "Lexer.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

Token::Type fn(Token::Type type) {
    return type;
}
int main() {
    Lexer lexer("sin 1*5 - 5");
    if (!lexer.getError().empty()) {
        std::cerr << lexer.getError() << std::endl;
        return 1;
    }
    auto root = Parser::parseExpression(lexer, 0.0f);
    return 0;
}

