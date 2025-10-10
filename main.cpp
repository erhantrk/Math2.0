#include <iostream>

#include "Lexer.hpp"
#include "Lexer.hpp"
#include "Parser.hpp"

Token::Type fn(Token::Type type) {
    return type;
}
int main() {
    Lexer lexer("!a");
    if (!lexer.getError().empty()) {
        std::cerr << lexer.getError() << std::endl;
        return 1;
    }

    auto root = Parser::parse(lexer);
    if (!root) {
        std::cerr << Parser::getError() << std::endl;
    }
    return 0;
}


