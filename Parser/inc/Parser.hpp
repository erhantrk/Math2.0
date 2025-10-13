//
// Created by Erhan Türker on 10/7/25.
//

#pragma once
#include <unordered_set>
#include <vector>

#include "../../Lexer/inc/Lexer.hpp"
#include "../../Node/inc/Node.hpp"

struct PairHasher {
    std::size_t operator()(const std::pair<std::string, std::vector<std::string>>& p) const {
        const std::size_t hash1 = std::hash<std::string>{}(p.first);
        std::size_t set_hash = 0;
        for (const auto& s : p.second) {
            set_hash ^= std::hash<std::string>{}(s) + 0x9e3779b9 + (set_hash << 6) + (set_hash >> 2);
        }
        return hash1 ^ (set_hash << 1);
    }
};


class Parser {
    std::unique_ptr<Node> parseExpression(Lexer &lexer, int min_bp);
    std::unique_ptr<Node> parseLhs(Lexer &lexer, Token& token);
    std::unique_ptr<Node> parseRhs(Lexer &lexer, std::unique_ptr<Node>& lhs, const Token& token, int min_bp);

    std::unique_ptr<Node> parseParentheses(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parseFunction(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parsePrefixToken(Lexer &lexer, const Token &token);
    std::unique_ptr<Node> parseOperator(Lexer &lexer, const Token &token, bool& outToken);

    std::unique_ptr<Node> parseStatement(Lexer &lexer);
    [[nodiscard]] bool isFunctionDefinition(const Lexer &lexer) const;
    std::pair<std::string, std::vector<std::string>> parseFunctionDefinition(Lexer &lexer);

    std::string error;
    int parenthesesLevel = 0;
    std::unordered_set<std::string> variables = {"e", "pi"};
    std::unordered_set<std::pair<std::string, std::vector<std::string>>, PairHasher> functions = {};

public:
    static bool isPreDefinedFunction(const Token &token);

    std::vector<std::unique_ptr<Node> > parse(Lexer &lexer);

    std::string getError();

    void clearError();

    /* For tests */
    void defineVariable(const std::string& name);
};
