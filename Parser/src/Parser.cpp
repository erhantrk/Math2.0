//
// Created by Erhan Türker on 10/7/25.
//

#include "../inc/Parser.hpp"

#include <algorithm>
#include <ranges>
#include <unordered_set>

#include "../../Simplifier/inc/Simplifier.hpp"
#include "../../Util/inc/ASTUtil.hpp"
#include "../inc/ParserErrors.hpp"

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

std::vector<std::shared_ptr<Node> > Parser::parse(Lexer &lexer) {
    std::vector<std::shared_ptr<Node> > statements;
    clearError();
    parenthesesLevel = 0;

    while (lexer.peek().type != Token::Type::Eof) {
        while (Token::isNewline(lexer.peek())) {
            lexer.skip();
        }
        if (lexer.peek().type == Token::Type::Eof) break;

        auto statement_node = Simplifier::simplify(parseStatement(lexer));
        if (!statement_node) {
            statements.clear();
            return statements;
        }
        statements.push_back(std::move(statement_node));

        const Token &next = lexer.peek();
        if (next.type != Token::Type::Eof && !Token::isNewline(next)) {
            error = ParserError::UnexpectedToken(lexer);
            statements.clear();
            return statements;
        }
    }
    return statements;
}

std::shared_ptr<Node> Parser::parseStatement(Lexer &lexer) {
    const Token &tmp = lexer.peek(); /* To give correct error if token is undefined variable */
    bool isFunctionDef = isFunctionDefinition(lexer);
    std::pair<std::string, std::vector<std::string>> function = {};
    if (isFunctionDef) {
        if (preDefinedFunctions.contains(lexer.peek().value)){
            error = ParserError::AssignmentToPredefinedFunction(lexer.peek().value, lexer.peek());
            return nullptr;
        }
        function =  parseFunctionDefinition(lexer);
    }

    auto lhs = parseExpression(lexer, 0);
    if (!lhs) {
        if (lexer.peek().type != Token::Type::Eof && error.empty()) error = ParserError::UnexpectedToken(lexer);
        return nullptr;
    }

    if (lexer.peek().value == "=" && !isFunctionDef) {
        if (lhs->type != Node::Type::Variable) {
            error = ParserError::InvalidAssignmentTarget(lexer.peek());
            return nullptr;
        }
        if (preDefinedVariables.contains(lhs->value)) {
            error = ParserError::AssignmentToLiteralValue(lhs->value, lexer.peek());
            return nullptr;
        }
        const Token &as = lexer.next();
        auto rhs = parseExpression(lexer, 0);
        if (!rhs) {
            if (error.empty())
                error = ParserError::MissingAssignment(as);
            return nullptr;
        }
        std::string undefinedVar;
        if (!areAllVariablesDefined(rhs,  undefinedVar)) {
            if (error.empty())
                error = ParserError::UndefinedVariable(as, undefinedVar);
            return nullptr;
        }

        auto assignment_node = std::make_unique<Node>(Node::Type::Assignment, lhs->value);
        assignment_node->children.push_back(std::move(rhs));
        variables.emplace(lhs->value);
        return assignment_node;
    }
    std::string undefinedVar;

    if (isFunctionDef && !areAllVariablesDefined(lhs, undefinedVar, function.second)) {
        if (error.empty())
            error = ParserError::UndefinedVariable(tmp, undefinedVar);
        return nullptr;
    }
    if (isFunctionDef) {
        auto definitionNode = std::make_unique<Node>(Node::Type::FunctionAssignment, function.first);
        definitionNode->children.push_back(std::move(lhs));
        functions.emplace(function);
        definitionNode->apply([&](Node* node) {
            if (node->type == Node::Type::Variable && std::ranges::find(function.second, node->value) != function.second.end()) {
                node->type = Node::Type::Parameter;
                auto it = std::ranges::find(function.second, node->value);
                if (it != function.second.end()) {
                    auto index = std::distance(function.second.begin(), it);
                    node->value = std::to_string(index) + "-" + node->value;
                }
            }
        });
        return definitionNode;
    }
    return lhs;
}

std::shared_ptr<Node> Parser::parseExpression(Lexer &lexer, int min_bp) { // NOLINT(*-no-recursion)
    Token token(Token::Type::Eof, "", 0, 0, "");
    std::shared_ptr<Node> lhs = parseLhs(lexer, token);
    if (!lhs) {
        return nullptr;
    }
    return parseRhs(lexer, lhs, token, min_bp);
}

std::shared_ptr<Node> Parser::parseLhs(Lexer &lexer, Token &outToken) { // NOLINT(*-no-recursion)
    const Token &token = lexer.peek();
    const bool isValid = (token.type == Token::Type::Number)
                         || (token.type == Token::Type::Word)
                         || (token.type == Token::Type::Symbol && token.value[0] == '(')
                         || Token::isTokenPreFix(token)
                         || (Token::isNewline(token) && parenthesesLevel)
                         || (token.type == Token::Type::Derivative);
    if (!isValid) {
        return nullptr;
    }

    lexer.skip();

    /* Special case we need to skip the newline */
    while (Token::isNewline(token)) *const_cast<Token *>(&token) = lexer.next();

    std::shared_ptr<Node> lhs = nullptr;

    if (token.type == Token::Type::Symbol && token.value[0] == '(') {
        lhs = parseParentheses(lexer, token);
    } else if (token.type == Token::Type::Word && isDefinedFunction(token)) {
        lhs = parseFunction(lexer, token);
    } else if (Token::isTokenPreFix(token)) {
        lhs = parsePrefixToken(lexer, token);
    } else if (token.type == Token::Type::Derivative) {
        lhs = parseDerivative(lexer, token);
    } else {
        lhs = Node::createNode(token, *this);
    }

    outToken = token;
    return std::move(lhs);
}

std::shared_ptr<Node> Parser::parseRhs(Lexer &lexer, std::shared_ptr<Node>& lhs, const Token& token, int min_bp) { // NOLINT(*-no-recursion)
    while (true) {
        if (lexer.peek().type == Token::Type::Eof) break;
        if (parenthesesLevel == 0 && Token::isNewline(lexer.peek())) break;
        if (parenthesesLevel > 0 && Token::isNewline(lexer.peek())) {
            lexer.skip();
            continue;
        }
        if (lexer.peek().type == Token::Type::Symbol && lexer.peek().value[0] == ')') break;

        bool isImplicit = false;
        std::shared_ptr<Node> op = parseOperator(lexer, token, isImplicit);
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
                error = ParserError::MissingRhs(opToken, isImplicit);
            return nullptr;
        }
        op->children.push_back(std::move(lhs));
        op->children.push_back(std::move(rhs));
        lhs = std::move(op);
    }
    return std::move(lhs);
}

std::shared_ptr<Node> Parser::parseParentheses(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    parenthesesLevel++;
    std::shared_ptr<Node> lhs = parseExpression(lexer, 0);
    const Token &pr = lexer.peek();
    if (pr.type != Token::Type::Symbol || pr.value[0] != ')') {
        if (!error.empty())
            return nullptr;
        if (!lhs)
            error = ParserError::InvalidStart(lexer);
        else
            error = ParserError::MissingParen(lexer, token);
        return nullptr;
    }
    lexer.skip();
    parenthesesLevel--;
    if (!lhs) {
        if (error.empty())
            error = ParserError::EmptyParen(token);
        return std::move(lhs);
    }
    if (lexer.peek().type == Token::Type::Number || lexer.peek().type == Token::Type::Word) {
        Token temp{Token::Type::Symbol, "*", token.line, token.pos, token.line_content};
        lexer.addToken(temp);
    }
    return std::move(lhs);
}

std::shared_ptr<Node> Parser::parseFunction(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    std::shared_ptr<Node> func = Node::createNode(token, *this);

    int argCount = 0;
    if (preDefinedFunctions.contains(token.value)) {
        argCount = static_cast<int>(preDefinedFunctions.at(token.value).size());
    } else if (functions.contains(token.value)) {
        argCount = static_cast<int>(functions.at(token.value).size());
    } else {
        error = ParserError::UnkownFunction(token);
        return nullptr;
    }

    const Token &pl = lexer.peek();
    if (pl.type == Token::Type::Symbol && pl.value[0] == '(') {  /* Standard form */
        parenthesesLevel++;
        lexer.skip();
        while (lexer.peek().type == Token::Type::Newline) lexer.skip();
        int prIndex = lexer.getClosingParenthesesIndex();
        if (prIndex < 0) {
            return nullptr;
        }
        Lexer argLexer = lexer.getSubLexer(prIndex);
        if (!prIndex) {
            if (error.empty())
                error = ParserError::EmptyParen(pl);
            return nullptr;
        }
        lexer.removeToken(prIndex);
        if (argCount > 1) {
            for (int i = 1; i < argCount; ++i) {
                int commaIndex = getNextComma(argLexer);
                if (commaIndex < 0) {
                    if (error.empty())
                        error = ParserError::NotEnoughArguments(token, argCount);
                    return nullptr;
                }
                Lexer argPartLexer = argLexer.getSubLexer(commaIndex);
                if (argPartLexer.peek().type == Token::Type::Eof) {
                    if (error.empty())
                        error = ParserError::EmptyArgument(argLexer.peek());
                }
                argLexer.skip(commaIndex + 1);
                std::shared_ptr<Node> argPart = parseExpression(argPartLexer, 0);
                if (argPart == nullptr) {
                    return nullptr; /* Arg is already invalid use previous error */
                }
                func->children.push_back(std::move(argPart));
            }
        }
        std::shared_ptr<Node> arg = parseExpression(argLexer, 0);
        if (arg == nullptr) {
            error = ParserError::TooManyArguments(token, argCount); /*Overwrite the unexpected token error */
            return nullptr; /* Arg is already invalid use previous error */
        }
        func->children.push_back(std::move(arg));
        lexer.skip(prIndex);
        if (lexer.peek().type == Token::Type::Number || lexer.peek().type == Token::Type::Word) {
            Token temp{Token::Type::Symbol, "*", token.line, token.pos, token.line_content};
            lexer.addToken(temp);
        }
        parenthesesLevel--;
    }
    else { /* Prefix like form */
        if (argCount>1) {
            if (error.empty())
                error = ParserError::MultiArgumentCalledWoParentheses(token, argCount);
            return nullptr;
        }
        auto&& [lhs_bp, rhs_bp] = getBindingPower(token);
        std::shared_ptr<Node> expr = parseExpression(lexer, rhs_bp);
        if (expr == nullptr) {
            if (error.empty())
                error = ParserError::NoArg(lexer, token);
            return nullptr;
        }
        func->children.push_back(std::move(expr));
    }

    return std::move(func);
}

std::shared_ptr<Node> Parser::parsePrefixToken(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    auto&& [lhs_bp, rhs_bp] = getBindingPower(token, true);
    std::shared_ptr<Node> op = Node::createNode(token, *this);
    std::shared_ptr<Node> arg = parseExpression(lexer, rhs_bp);
    if (arg == nullptr) {
        if (!error.empty())
            return nullptr;
        if (lexer.peek().type == Token::Type::Eof) {
            error = ParserError::MissingOperandForPrefix(token);
        } else {
            error = ParserError::InvalidStart(lexer);
        }
        return nullptr;
    }
    op->children.push_back(std::move(arg));
    return std::move(op);
}

std::shared_ptr<Node> Parser::parseOperator(Lexer &lexer, const Token &token, bool& isImplicit) {
    std::shared_ptr<Node> op = nullptr;
    if (lexer.peek().type == Token::Type::Symbol) {
        if (lexer.peek().value[0] == '(') {
            Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
            op = Node::createNode(temp, *this);
            lexer.addToken(temp);
            isImplicit = true;
        } else {
            op = Node::createNode(lexer.peek(), *this);
        }
    } else if (
            (token.type == Token::Type::Number && lexer.peek().type == Token::Type::Word)
        ||  (token.type == Token::Type::Word && lexer.peek().type == Token::Type::Word)
        ||  (token.type == Token::Type::Word && lexer.peek().type == Token::Type::Number)) {
        Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos - 1, token.line_content};
        op = Node::createNode(temp, *this);
        lexer.addToken(temp);
        isImplicit = true;
    } else {
        if (!error.empty()) {
            return nullptr;
        }
        if (Token::isNewline(token)) {
            error = ParserError::MultilineWoParen(Token(token));
        } else if (lexer.peek().type == Token::Type::Comma) {
            error = ParserError::UnexpectedToken(lexer);
        } else
            error = ParserError::MissingOperator(Token(token), lexer.peek());
        return nullptr;
    }
    return std::move(op);
}

std::shared_ptr<Node> Parser::parseDerivative(Lexer &lexer, const Token &token) { // NOLINT(*-no-recursion)
    std::string var = token.value.substr(3);
    auto derivNode = std::make_shared<Node>(Node::Type::Derivative, var);

    const Token &pl = lexer.peek();
    if (pl.type != Token::Type::Symbol || pl.value[0] != '(') {
        if (error.empty())
            error = ParserError::NoArg(lexer, token);
        return nullptr;
    }

    parenthesesLevel++;
    lexer.skip();

    std::shared_ptr<Node> expression = parseExpression(lexer, 0);

    const Token &pr = lexer.peek();
    if (pr.type != Token::Type::Symbol || pr.value[0] != ')') {
        if (!error.empty())
            return nullptr;
        if (!expression)
            error = ParserError::EmptyParen(pl);
        else
            error = ParserError::MissingParen(lexer, pl);
        return nullptr;
    }

    lexer.skip();
    parenthesesLevel--;

    if (!expression) {
        if (error.empty())
            error = ParserError::EmptyParen(pl);
        return nullptr;
    }

    derivNode->children.push_back(std::move(expression));

    if (lexer.peek().type == Token::Type::Number || lexer.peek().type == Token::Type::Word) {
        Token temp{Token::Type::Symbol, "*", token.line, token.pos, token.line_content};
        lexer.addToken(temp);
    }

    return derivNode;
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

bool Parser::isFunctionDefinition(const Lexer &lexer) const {
    if (lexer.peek().type != Token::Type::Word
        || variables.contains(lexer.peek().value)
        || preDefinedVariables.contains(lexer.peek().value)) return false;
    if (lexer.peek(1).type != Token::Type::Symbol || lexer.peek(1).value[0] != '(') return false;
    int i = 2;
    while (true) {
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i++).type != Token::Type::Word) return false;
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i++).value[0] == ')') break;
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i++).type != Token::Type::Comma) return false;
    }
    if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i).value[0] == '=') return true;
    return false;
}

bool Parser::isFunctionExpression(const Lexer &lexer) const {
    if (lexer.peek().type != Token::Type::Word
        || variables.contains(lexer.peek().value)
        || preDefinedVariables.contains(lexer.peek().value)) return false;
    if (lexer.peek(1).type != Token::Type::Symbol || lexer.peek(1).value[0] != '(') return false;
    int i = 2;
    while (true) {
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i++).type != Token::Type::Word) return false;
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i++).value[0] == ')') break;
        while (Token::isNewline(lexer.peek(i))) {i++;}
        if (lexer.peek(i++).type != Token::Type::Comma) return false;
    }
    if (lexer.peek(i).type == Token::Type::Symbol && lexer.peek(i).value[0] == '=') return false;
    return true;
}

bool Parser::isDefinedFunction(const Token &token) const {
    return preDefinedFunctions.contains(token.value) || functions.contains(token.value);
}

int Parser::getNextComma(const Lexer& lexer) const {
    int skip = 0;
    for (int i = 0; ;) {
        if (lexer.peek(i).type == Token::Type::Eof) return -1;
        if (lexer.peek(i).type == Token::Type::Word) {
            if (functions.contains(lexer.peek(i).value)) {
                skip += static_cast<int>(functions.at(lexer.peek(i).value).size()) - 1;
            } else if (preDefinedFunctions.contains(lexer.peek(i).value)) {
                skip += static_cast<int>(preDefinedFunctions.at(lexer.peek(i).value).size()) - 1;
            }
        }
        if (lexer.peek(i).type == Token::Type::Comma) {
            if (skip == 0) return i;
            skip--;
        }
        i++;
    }
}

bool Parser::areAllVariablesDefined(const std::shared_ptr<Node> &node, std::string &undefinedVariable, const std::vector<std::string>& parameters) { // NOLINT(*-no-recursion)
    if (!node) {
        return true;
    }

    if (node->type == Node::Type::Variable) {
        bool foundInParams = std::ranges::find(parameters, node->value) != parameters.end();
        bool foundInSets = variables.contains(node->value) || preDefinedVariables.contains(node->value);

        if (!(foundInSets || foundInParams)) {
            undefinedVariable = node->value;
            return false;
        }
    }

    for (const auto &child: node->children) {
        if (!areAllVariablesDefined(child, undefinedVariable, parameters)) {
            return false;
        }
    }

    return true;
}

std::string Parser::getError() {
    return error;
}

void Parser::clearError() {
    error.clear();
}
