#include <iostream>
#include "Tokenizer.hpp"

int main() {
    auto&& [error, tokens] = Tokenizer::tokenize("3 + 5 * (2 - 4)");
    if (tokens.empty()) {
        std::cerr << error << std::endl;
        return 1;
    }
    for (const auto& token : tokens) {
        std::cout << token << ", ";
    }
    return 0;
}