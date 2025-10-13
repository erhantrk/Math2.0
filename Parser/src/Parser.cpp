//
// Created by Erhan Türker on 10/7/25.
//

#include "../inc/Parser.hpp"
#include <ranges>
#include <unordered_set>

#include "../inc/ParserErrors.hpp"

static bool areAllVariablesDefined(const std::unique_ptr<Node>&, const std::unordered_set<std::string>&, std::string&, const std::vector<std::string>& = {});

std::pair<int, int> getBindingPower(const Token &token, bool isPrefix = false) {
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

std::vector<std::unique_ptr<Node> > Parser::parse(Lexer &lexer) {
    std::vector<std::unique_ptr<Node> > statements;
    clearError();
    parenthesesLevel = 0;

    while (lexer.peek().type != Token::Type::Eof) {
        while (Token::isNewline(lexer.peek())) {
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
        if (next.type != Token::Type::Eof && !Token::isNewline(next)) {
            error = getUnexpectedTokenError(lexer);
            statements.clear();
            return statements;
        }
    }
    return statements;
}

std::unique_ptr<Node> Parser::parseStatement(Lexer &lexer) {
    const Token &tmp = lexer.peek(); /* To give correct error if token is undefined variable */
    bool isFunctionDef = isFunctionDefinition(lexer);
    std::pair<std::string, std::vector<std::string>> function = {};
    if (isFunctionDef) {
        function =  parseFunctionDefinition(lexer);
    }

    auto lhs = parseExpression(lexer, 0);
    if (!lhs) {
        if (lexer.peek().type != Token::Type::Eof && error.empty()) error = getUnexpectedTokenError(lexer);
        return nullptr;
    }

    if (lexer.peek().value == "=" && !isFunctionDef) {
        if (lhs->type != Node::Type::Variable) {
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
        if (!areAllVariablesDefined(rhs, variables, undefinedVar)) {
            if (error.empty())
                error = getUndefinedVariableError(as, undefinedVar);
            return nullptr;
        }

        auto assignment_node = std::make_unique<Node>(Node::Type::Assignment, lhs->value);
        assignment_node->children.push_back(std::move(rhs));
        variables.emplace(lhs->value);
        return assignment_node;
    }
    std::string undefinedVar;

    if (!areAllVariablesDefined(lhs, variables, undefinedVar, function.second)) {
        if (error.empty())
            error = getUndefinedVariableError(tmp, undefinedVar);
        return nullptr;
    }
    if (isFunctionDef) {
        auto definition_node = std::make_unique<Node>(Node::Type::FunctionAssignment, function.first);
        definition_node->children.push_back(std::move(lhs));
        functions.emplace(function);
        definition_node->apply([&](Node* node) {
            if (node->type == Node::Type::Variable && std::ranges::find(function.second, node->value) != function.second.end()) {
                node->type = Node::Type::Parameter;
            }
        });
        return definition_node;
    }
    return lhs;
}

std::unique_ptr<Node> Parser::parseExpression(Lexer &lexer, int min_bp) { // NOLINT(*-no-recursion)
    Token token(Token::Type::Eof, "", 0, 0, "");
    std::unique_ptr<Node> lhs = parseLhs(lexer, token);
    if (!lhs) {
        return nullptr;
    }
    return parseRhs(lexer, lhs, token, min_bp);
}

std::unique_ptr<Node> Parser::parseLhs(Lexer &lexer, Token &outToken) { // NOLINT(*-no-recursion)
    const Token &token = lexer.peek();
    const bool isValid = (token.type == Token::Type::Number)
                         || (token.type == Token::Type::Word)
                         || (token.type == Token::Type::Symbol && token.value[0] == '(')
                         || Token::isTokenPreFix(token)
                         || (Token::isNewline(token) && parenthesesLevel);
    if (!isValid) {
        return nullptr;
    }

    lexer.skip();

    /* Special case we need to skip the newline */
    if (Token::isNewline(token)) *const_cast<Token *>(&token) = lexer.next();

    std::unique_ptr<Node> lhs = nullptr;

    if (token.type == Token::Type::Symbol && token.value[0] == '(') {
        lhs = parseParentheses(lexer, token);
    } else if (token.type == Token::Type::Word && isPreDefinedFunction(token)) {
        lhs = parseFunction(lexer, token);
    } else if (Token::isTokenPreFix(token)) {
        lhs = parsePrefixToken(lexer, token);
    } else {
        lhs = Node::createNode(token);
    }

    outToken = token;
    return std::move(lhs);
}

std::unique_ptr<Node> Parser::parseRhs(Lexer &lexer, std::unique_ptr<Node>& lhs, const Token& token, int min_bp) { // NOLINT(*-no-recursion)
    while (true) {
        if (lexer.peek().type == Token::Type::Eof) break;
        if (parenthesesLevel == 0 && Token::isNewline(lexer.peek())) break;
        if (parenthesesLevel > 0 && Token::isNewline(lexer.peek())) {
            lexer.skip();
            continue;
        }
        if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == ')') break;

        bool isImplicit = false;
        std::unique_ptr<Node> op = parseOperator(lexer, token, isImplicit);
        if (!op) return nullptr;

        auto &&[lhs_bp, rhs_bp] = getBindingPower(lexer.peek());
        if (lhs_bp < min_bp) break;

        if (Token::isTokenPostFix(lexer.peek())) {
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
    return std::move(lhs);
}

std::unique_ptr<Node> Parser::parseParentheses(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    parenthesesLevel++;
    std::unique_ptr<Node> lhs = parseExpression(lexer, 0);
    const Token &pr = lexer.peek();
    if (pr.type != Token::Type::Symbol || pr.value[0] != ')') {
        if (!error.empty())
            return nullptr;
        if (!lhs)
            error = getInvalidStartError(lexer);
        else
            error = getMissingParenError(lexer, token);
        return nullptr;
    }
    lexer.skip();
    parenthesesLevel--;
    if (!lhs) {
        if (error.empty())
            error = getEmptyParenError(token);
        return std::move(lhs);
    }
    if (lexer.peek().type == Token::Type::Number || lexer.peek().type == Token::Type::Word) {
        Token temp{Token::Type::Symbol, "*", token.line, token.pos, token.line_content};
        lexer.addToken(temp);
    }
    return std::move(lhs);
}

std::unique_ptr<Node> Parser::parseFunction(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    auto [lhs_bp, rhs_bp] = getBindingPower(token);
    std::unique_ptr<Node> op = Node::createNode(token);
    if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == '(') {
        rhs_bp = 100;
    }
    std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
    if (arg == nullptr) {
        if (error.empty())
            error = getNoArgError(lexer, token);
        return nullptr;
    }
    op->children.push_back(std::move(arg));
    return std::move(op);
}

std::unique_ptr<Node> Parser::parsePrefixToken(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    auto [lhs_bp, rhs_bp] = getBindingPower(token, true);
    std::unique_ptr<Node> op = Node::createNode(token);
    std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
    if (arg == nullptr) {
        if (!error.empty())
            return nullptr;
        if (lexer.peek().type == Token::Type::Eof) {
            error = getMissingOperandForPrefixError(token);
        } else {
            error = getInvalidStartError(lexer);
        }
        return nullptr;
    }
    op->children.push_back(std::move(arg));
    return std::move(op);
}

std::unique_ptr<Node> Parser::parseOperator(Lexer &lexer, const Token &token, bool& isImplicit) {
    std::unique_ptr<Node> op = nullptr;
    if (lexer.peek().type == Token::Type::Symbol) {
        if (lexer.peek().value[0] == '(') {
            Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
            op = Node::createNode(temp);
            lexer.addToken(temp);
            isImplicit = true;
        } else {
            op = Node::createNode(lexer.peek());
        }
    } else if (lexer.peek().type == Token::Type::Word && token.type == Token::Type::Number) {
        Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
        op = Node::createNode(temp);
        lexer.addToken(temp);
        isImplicit = true;
    } else {
        if (!error.empty()) {
            return nullptr;
        }
        if (Token::isNewline(token)) {
            error = getMultilineWoParenError(Token(token));
        } else
            error = getMissingOperatorError(Token(token), lexer.peek());
        return nullptr;
    }
    return std::move(op);
}

bool Parser::isFunctionDefinition(const Lexer &lexer) const {
    if (lexer.peek().type != Token::Type::Word || variables.contains(lexer.peek().value)) return false;
    if (lexer.peek(1).type != Token::Type::Symbol || lexer.peek(1).value[0] != '(') return false;
    int i = 2;
    while (true) {
        if (lexer.peek(i++).type != Token::Type::Word) return false;
        if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i++).value[0] == ')') break;
        if (lexer.peek(i++).type != Token::Type::Comma) return false;
    }
    if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i).value[0] == '=') return true;
    return false;
}

bool Parser::isPreDefinedFunction(const Token &token) {
    static const std::unordered_set<std::string> preDefinedFunctions = {
        "sin", "cos", "tan", "log", "ln", "sqrt", "abs"
    };
    return preDefinedFunctions.contains(token.value);
}

static bool areAllVariablesDefined(const std::unique_ptr<Node> &node, const std::unordered_set<std::string> &definedVariables, // NOLINT(*-no-recursion)
    std::string &undefinedVariable, const std::vector<std::string>& parameters) {
    if (!node) {
        return true;
    }

    if (node->type == Node::Type::Variable) {
        if (!(definedVariables.contains(node->value) || std::ranges::find(parameters, node->value) != parameters.end())) {
            undefinedVariable = node->value;
            return false;
        }
    }

    for (const auto &child: node->children) {
        if (!areAllVariablesDefined(child, definedVariables, undefinedVariable, parameters)) {
            return false;
        }
    }

    return true;
}

std::pair<std::string, std::vector<std::string>> Parser::parseFunctionDefinition(Lexer &lexer) { // NOLINT(*-no-recursion)
    std::string functionName = lexer.next().value;
    lexer.skip();
    std::vector<std::string> tempVariables;
    while (true) {
        const auto& token = lexer.next();
        if (token.type == Token::Type::Word)
            tempVariables.push_back(token.value);
        else if (token.type == Token::Type::Symbol && token.value[0] == '=') break;
    }
    return std::make_pair(functionName, tempVariables);
}

std::string Parser::getError() {
    return error;
}

void Parser::clearError() {
    error.clear();
}

void Parser::defineVariable(const std::string& name) {
    variables.emplace(name);
}
