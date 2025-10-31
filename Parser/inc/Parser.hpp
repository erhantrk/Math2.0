//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <unordered_set>
#include <unordered_map>
#include <vector>

#include "../../Lexer/inc/Lexer.hpp"
#include "../../Node/inc/Node.hpp"

class Parser {
    std::shared_ptr<Node> parseExpression(Lexer &lexer, int min_bp);
    std::shared_ptr<Node> parseLhs(Lexer &lexer, Token& token);
    std::shared_ptr<Node> parseRhs(Lexer &lexer, std::shared_ptr<Node>& lhs, const Token& token, int min_bp);

    std::shared_ptr<Node> parseParentheses(Lexer &lexer, const Token &token);
    std::shared_ptr<Node> parseFunction(Lexer &lexer, const Token &token);
    std::shared_ptr<Node> parsePrefixToken(Lexer &lexer, const Token &token);
    std::shared_ptr<Node> parseOperator(Lexer &lexer, const Token &token, bool& outToken);

    std::shared_ptr<Node> parseStatement(Lexer &lexer);
    [[nodiscard]] bool isFunctionDefinition(const Lexer &lexer) const;
    [[nodiscard]] bool isFunctionExpression(const Lexer &lexer) const;
    static std::pair<std::string, std::vector<std::string>> parseFunctionDefinition(Lexer &lexer);
    int getNextComma(const Lexer& lexer) const;

    std::string error;
    int parenthesesLevel = 0;
    bool isParsingFunctionCall = false;
    std::unordered_set<std::string> variables = {"e", "pi"};
    std::unordered_map<std::string, std::vector<std::string>> functions;

    static inline const std::unordered_map<std::string, std::vector<std::string>> preDefinedFunctions = {
        {"sin", {"0-x"}}, {"cos", {"0-x"}}, {"tan", {"0-x"}},
        {"log", {"0-x"}}, {"ln", {"0-x"}}, {"sqrt", {"0-x"}},
        {"abs", {"0-x"}}, {"atan2", {"0-x", "1-y"}}
    };

public:
    [[nodiscard]] bool isDefinedFunction(const Token &token) const;

    std::vector<std::shared_ptr<Node> > parse(Lexer &lexer);

    std::string getError();

    void clearError();

    /* For tests */
    void defineVariable(const std::string& name);
};
