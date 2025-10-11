//
// Created by Erhan Türker on 10/7/25.
//

#include "Parser.hpp"
#include <ranges>
#include <unordered_set>
#include "ParserErrors.hpp"

static std::unique_ptr<Node> createNode(const Token &token);

static bool isTokenPostFix(const Token &token);

std::pair<int, int> getBindingPower(const Token &token, bool isPrefix = false);

static bool isTokenPreFix(const Token &token);

static bool isNewline(const Token &token);

std::vector<std::unique_ptr<Node> > Parser::parse(Lexer &lexer) {
    std::vector<std::unique_ptr<Node> > statements;
    clearError();
    parenthesesLevel = 0;

    while (lexer.peek().type != Token::Type::Eof) {
        while (isNewline(lexer.peek())) {
            lexer.skip();
        }
        if (lexer.peek().type == Token::Type::Eof) break;

        auto statement_node = parseStatement(lexer);
        if (!statement_node) {
            statements.clear();
            return statements;
        }
        statements.push_back(std::move(statement_node));

        const Token &next = lexer.peek();
        if (next.type != Token::Type::Eof && !isNewline(next)) {
            error = getUnexpectedTokenError(lexer);
            statements.clear();
            return statements;
        }
    }
    return statements;
}

static bool areAllVariablesDefined(const std::unique_ptr<Node> &node,
                                   const std::unordered_set<std::string> &definedVariables,
                                   std::string &undefinedVariable) {
    if (!node) {
        return true;
    }

    if (node->type == Node::Type::Variable) {
        if (definedVariables.find(node->value) == definedVariables.end()) {
            undefinedVariable = node->value;
            return false;
        }
    }

    for (const auto &child: node->children) {
        if (!areAllVariablesDefined(child, definedVariables, undefinedVariable)) {
            return false;
        }
    }

    return true;
}

std::unique_ptr<Node> Parser::parseStatement(Lexer &lexer) {
    auto expr = parseExpression(lexer, 0);
    if (!expr) {
        if (lexer.peek().type != Token::Type::Eof && error.empty()) error = getUnexpectedTokenError(lexer);
        return nullptr;
    }

    if (lexer.peek().value == "=") {
        if (expr->type != Node::Type::Variable) {
            error = getInvalidAssignmentTargetError(lexer.peek());
            return nullptr;
        }
        const Token &as = lexer.next();
        auto rhs = parseExpression(lexer, 0);
        if (!rhs) {
            if (error.empty())
                error = getMissingAssignmentError(as);
            return nullptr;
        }
        std::string undefinedVar;
        bool isDefined = areAllVariablesDefined(rhs, variables, undefinedVar);
        if (!isDefined) {
            if (error.empty())
                error = getUndefinedVariableError(as, undefinedVar);
            return nullptr;
        }

        auto assignment_node = std::make_unique<Node>(Node::Type::Assignment, expr->value);
        assignment_node->children.push_back(std::move(rhs));
        variables.emplace(expr->value);
        return assignment_node;
    }
    return expr;
}

std::unique_ptr<Node> Parser::parseExpression(Lexer &lexer, int min_bp) {
    const Token &token = lexer.peek();
    const bool isValid = (token.type == Token::Type::Number)
                         || (token.type == Token::Type::Word)
                         || (token.type == Token::Type::Symbol && token.value[0] == '(')
                         || isTokenPreFix(token)
                         || isNewline(token);
    if (!isValid) {
        return nullptr;
    }
    lexer.skip();
    if (isNewline(token)) *const_cast<Token *>(&token) = lexer.next(); /* Special case we need to skip the newline */
    std::unique_ptr<Node> lhs = nullptr;
    if (token.type == Token::Type::Symbol && token.value[0] == '(') {
        parenthesesLevel++;
        lhs = parseExpression(lexer, 0);
        const Token &pr = lexer.peek();
        if (pr.type != Token::Type::Symbol || pr.value[0] != ')') {
            if (!lhs)
                error = getInvalidStartError(lexer);
            if (error.empty())
                error = getMissingParenError(lexer, token);
            return nullptr;
        }
        lexer.skip();
        parenthesesLevel--;
        if (!lhs) {
            if (error.empty())
                error = getEmptyParenError(token);
            return nullptr;
        }
        if (lexer.peek().type == Token::Type::Number || lexer.peek().type == Token::Type::Word) {
            Token temp{Token::Type::Symbol, "*", token.line, token.pos, token.line_content};
            lexer.addToken(temp);
        }
    } else if (token.type == Token::Type::Word && isPreDefinedFunction(token)) {
        auto [lhs_bp, rhs_bp] = getBindingPower(token);
        std::unique_ptr<Node> op = createNode(token);
        if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == '(') {
            parenthesesLevel++;
            rhs_bp = 100;
        }
        std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
        if (arg == nullptr) {
            if (error.empty())
                error = getNoArgError(lexer, token);
            return nullptr;
        }
        op->children.push_back(std::move(arg));
        lhs = std::move(op);
    } else if (isTokenPreFix(token)) {
        auto [lhs_bp, rhs_bp] = getBindingPower(token, true);
        std::unique_ptr<Node> op = createNode(token);
        std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
        if (arg == nullptr) {
            if (error.empty())
                if (lexer.peek().type == Token::Type::Eof) {
                    error = getMissingOperandForPrefixError(token);
                } else {
                    error = getInvalidStartError(lexer);
                }
            return nullptr;
        }
        op->children.push_back(std::move(arg));
        lhs = std::move(op);
    } else {
        lhs = createNode(token);
    }

    while (true) {
        if (lexer.peek().type == Token::Type::Eof) break;
        if (parenthesesLevel == 0 && isNewline(lexer.peek())) break;
        if (parenthesesLevel > 0 && isNewline(lexer.peek())) {
            lexer.skip();
            continue;
        }
        if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == ')') break;

        std::unique_ptr<Node> op = nullptr;
        bool isImplicit = false;
        if (lexer.peek().type == Token::Type::Symbol) {
            if (lexer.peek().value[0] == '(') {
                Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
                op = createNode(temp);
                lexer.addToken(temp);
                isImplicit = true;
            } else {
                op = createNode(lexer.peek());
            }
        } else if (lexer.peek().type == Token::Type::Word && token.type == Token::Type::Number) {
            Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
            op = createNode(temp);
            lexer.addToken(temp);
            isImplicit = true;
        } else {
            if (error.empty()) {
                if (isNewline(token)) {
                    error = getMultilineWoParenError(token);
                } else
                    error = getMissingOperatorError(token, lexer.peek());
                return nullptr;
            }
        }

        auto &&[lhs_bp, rhs_bp] = getBindingPower(lexer.peek());
        if (lhs_bp < min_bp) break;
        if (isTokenPostFix(lexer.peek())) {
            lexer.skip();
            op->children.push_back(std::move(lhs));
            lhs = std::move(op);
            if (lexer.peek().type == Token::Type::Word || lexer.peek().type == Token::Type::Number) {
                Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
                lexer.addToken(temp);
            }
            continue;
        }
        const auto &opToken = lexer.next();
        auto rhs = parseExpression(lexer, rhs_bp);
        if (rhs == nullptr) {
            if (error.empty())
                error = getMissingRhsError(opToken, isImplicit);
            return nullptr;
        }
        op->children.push_back(std::move(lhs));
        op->children.push_back(std::move(rhs));
        lhs = std::move(op);
    }
    return lhs;
}

std::pair<int, int> getBindingPower(const Token &token, bool isPrefix) {
    if (token.value.empty()) return std::make_pair(-1, -1);
    if (isPrefix) {
        switch (token.value[0]) {
            case '-':
            case '+': return std::make_pair(-1, 4);
            default: return std::make_pair(-1, -1);;
        }
    }
    if (token.type == Token::Type::Symbol) {
        switch (token.value[0]) {
            case '-':
            case '+': return std::make_pair(1, 2);
            case '*':
            case '/': return std::make_pair(3, 4);
            case '^': return std::make_pair(5, 4);
            case '!': return std::make_pair(5, -1);
            default: return std::make_pair(-1, -1);
        }
    }
    if (token.type == Token::Type::Word) {
        return std::make_pair(0, 2);
    }
    return std::make_pair(-1, -1);
}

bool Parser::isPreDefinedFunction(const Token &token) {
    static const std::unordered_set<std::string> preDefinedFunctions = {
        "sin", "cos", "tan", "log", "ln", "sqrt", "abs"
    };
    return preDefinedFunctions.contains(token.value);
}

std::string Parser::getError() {
    return error;
}

void Parser::clearError() {
    error.clear();
}

static bool isNewline(const Token &token) {
    return token.type == Token::Type::Newline;
}

static std::unique_ptr<Node> createNode(const Token &token) {
    switch (token.type) {
        case Token::Type::Word:
            if (Parser::isPreDefinedFunction(token))
                return std::make_unique<Node>(Node{Node::Type::Function, token.value});;
            return std::make_unique<Node>(Node{Node::Type::Variable, token.value});
        case Token::Type::Number: return std::make_unique<Node>(Node{Node::Type::Number, token.value});
        case Token::Type::Symbol:
            if (token.value[0] == '=')
                return std::make_unique<Node>(Node{Node::Type::Assignment, token.value});
            return std::make_unique<Node>(Node{Node::Type::Operand, token.value});
        default: return nullptr;
    }
}

static bool isTokenPostFix(const Token &token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '!': return true;
        default: return false;
    }
}

static bool isTokenPreFix(const Token &token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '-':
        case '+': return true;
        default: return false;
    }
}
