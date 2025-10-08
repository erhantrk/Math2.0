//
// Created by Erhan Türker on 10/7/25.
//

#include "Parser.hpp"
#include <unordered_set>

static std::unique_ptr<Node> createNode(const Token& token);
static bool isTokenPostFix(const Token& token);
std::pair<int, int> getBindingPower(const Token&  token);


std::unique_ptr<Node> Parser::parseExpression(Lexer& lexer, int min_bp){
    const Token& token = lexer.next();
    if (token.type != Token::Type::Number && token.type != Token::Type::Word && (token.type != Token::Type::Symbol && token.value[0] == '(')) {
        /* Add error here */
        return nullptr;
    }
    std::unique_ptr<Node> lhs = nullptr;
    if (token.type == Token::Type::Symbol) {
        lhs = parseExpression(lexer, 0);
        const Token& pr = lexer.next();
        if (pr.type != Token::Type::Symbol || pr.value[0] != ')') {
            /* Add error here */
            return nullptr;
        }
    }
    else if (token.type == Token::Type::Word && isPreDefinedFunction(token)) {
        auto [lhs_bp, rhs_bp] = getBindingPower(token);
        std::unique_ptr<Node> op = createNode(token);
        std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
        if (arg == nullptr) {
            /* Add error here */
            return nullptr;
        }
        op->children.push_back(std::move(arg));
        lhs = std::move(op);
    }
    else {
        lhs = createNode(token);
    }

    while (true) {
        if (lexer.peek().type == Token::Type::Eof) break;
        if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == ')') break;

        std::unique_ptr<Node> op = nullptr;
        if (lexer.peek().type == Token::Type::Symbol) op = createNode(lexer.peek());
        else if (lexer.peek().type == Token::Type::Word && token.type == Token::Type::Number) {
            Token temp{Token::Type::Symbol, "*", token.line};
            op = createNode(temp);
            lexer.addToken(temp);
        }
        else {
            /* Add error here */
            return nullptr;
        }

        auto&& [lhs_bp, rhs_bp] = getBindingPower(lexer.peek());
        if (lhs_bp < min_bp) break;
        if (isTokenPostFix(lexer.next())) {
            op->children.push_back(std::move(lhs));
            lhs = std::move(op);
            continue;
        }
        auto rhs = parseExpression(lexer, rhs_bp);
        if (rhs  == nullptr) {
            /* Add error here */
            return nullptr;
        }
        op->children.push_back(std::move(lhs));
        op->children.push_back(std::move(rhs));
        lhs = std::move(op);
    }
    return lhs;
}

std::pair<int, int> getBindingPower(const Token&  token) {
    if (token.value.empty()) return std::make_pair(-1 , -1);
    if (token.type == Token::Type::Symbol) {
        switch (token.value[0]) {
            case '-': case '+': return std::make_pair(1, 2);
            case '*': case '/': return std::make_pair(3, 4);
            case '^': return std::make_pair(7, 6);
            case '!': return std::make_pair(8, 0);
            default: return std::make_pair(-1, -1);
        }
    }
    if (token.type == Token::Type::Word) {
        return std::make_pair(0, 5);
    }
    return std::make_pair(-1, -1);
}


bool Parser::isPreDefinedFunction(const Token& token) {
    static const std::unordered_set<std::string> preDefinedFunctions = {
        "sin","cos","tan","log","ln","sqrt","abs"};
    return preDefinedFunctions.contains(token.value);

}

static std::unique_ptr<Node> createNode(const Token& token) {
    switch (token.type) {
        /* TODO check before word if its a function */
        case Token::Type::Word:
            if (Parser::isPreDefinedFunction(token))
                return std::make_unique<Node>(Node{Node::Type::Function, token.value});;
            return std::make_unique<Node>(Node{Node::Type::Variable, token.value});
        case Token::Type::Number: return std::make_unique<Node>(Node{Node::Type::Number, token.value});
        case Token::Type::Symbol: return std::make_unique<Node>(Node{Node::Type::Operand, token.value});
        default: return nullptr;
    }
}

static bool isTokenPostFix(const Token& token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '!': return true;
        default: return false;
    }
}
