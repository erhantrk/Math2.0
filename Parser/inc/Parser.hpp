//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <unordered_set>
#include <vector>

#include "../../Lexer/inc/Lexer.hpp"
#include "../../Node/inc/Node.hpp"

class Parser {
    static std::unique_ptr<Node> parseExpression(Lexer &lexer, int min_bp);
    static std::unique_ptr<Node> parseLhs(Lexer &lexer, Token& token);
    static std::unique_ptr<Node> parseRhs(Lexer &lexer, std::unique_ptr<Node>& lhs, const Token& token, int min_bp);

    static std::unique_ptr<Node> parseParentheses(Lexer &lexer, const Token &token);
    static std::unique_ptr<Node> parseFunction(Lexer &lexer, const Token &token);
    static std::unique_ptr<Node> parsePrefixToken(Lexer &lexer, const Token &token);
    static std::unique_ptr<Node> parseOperator(Lexer &lexer, const Token &token, bool& outToken);

    static std::unique_ptr<Node> parseStatement(Lexer &lexer);

    static inline std::string error;
    static inline int parenthesesLevel = 0;
    static inline std::unordered_set<std::string> variables = {"e", "pi"};

public:
    static bool isPreDefinedFunction(const Token &token);

    static std::vector<std::unique_ptr<Node> > parse(Lexer &lexer);

    static std::string getError();

    static void clearError();
};
