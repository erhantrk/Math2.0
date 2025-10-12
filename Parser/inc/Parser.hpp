//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <unordered_set>
#include <vector>

#include "../../Lexer/inc/Lexer.hpp"
#include "../../Node/inc/Node.hpp"

class Parser {
    std::unique_ptr<Node> parseExpression(Lexer &lexer, int min_bp);
    std::unique_ptr<Node> parseLhs(Lexer &lexer, Token& token);
    std::unique_ptr<Node> parseRhs(Lexer &lexer, std::unique_ptr<Node>& lhs, const Token& token, int min_bp);

    std::unique_ptr<Node> parseParentheses(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parseFunction(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parsePrefixToken(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parseOperator(Lexer &lexer, const Token &token, bool& outToken);

    std::unique_ptr<Node> parseStatement(Lexer &lexer);

    std::string error;
    int parenthesesLevel = 0;
    std::unordered_set<std::string> variables = {"e", "pi"};

public:
    static bool isPreDefinedFunction(const Token &token);

    std::vector<std::unique_ptr<Node> > parse(Lexer &lexer);

    std::string getError();

    void clearError();

    void defineVariable(const std::string& name);
};
