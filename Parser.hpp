//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <unordered_set>
#include <vector>

#include "Lexer.hpp"

struct Node {
    enum class Type { Number, Variable, Operand, Function, Assignment };

    Type type;
    std::string value;
    std::vector<std::unique_ptr<Node> > children;
};

class Parser {
    static std::unique_ptr<Node> parseExpression(Lexer &lexer, int min_bp);

    static std::unique_ptr<Node> parseStatement(Lexer &lexer);

    static inline std::string error;
    static inline int parenthesesLevel = 0;
    static inline std::unordered_set<std::string> variables = {};

public:
    static bool isPreDefinedFunction(const Token &token);

    static std::vector<std::unique_ptr<Node> > parse(Lexer &lexer);

    static std::string getError();

    static void clearError();
};
