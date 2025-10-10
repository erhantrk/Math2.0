//
// Created by Erhan Türker on 10/7/25.
//

#include "Parser.hpp"

#include <ranges>
#include <unordered_set>
#include <sstream>

static std::unique_ptr<Node> createNode(const Token& token);
static bool isTokenPostFix(const Token& token);
std::pair<int, int> getBindingPower(const Token&  token, bool isPrefix = false);
static bool isTokenPreFix(const Token& token);
static std::string getNoArgError(const Lexer& lexer, const Token& token);
static std::string getUnexpectedTokenError(const Lexer& lexer);
static std::string getInvalidStartError(const Lexer& lexer);
static std::string getMissingParenError(const Lexer& lexer, const Token& open_paren_token);
static std::string getMissingOperandForPrefixError(const Token& prefix_token);
static std::string getMissingRhsError(const Token& operator_token, bool isImplicit);
static std::string getEmptyParenError(const Token& open_paren_token);
static std::string getMissingOperatorError(const Token& previous_token, const Token& offending_token);
static bool isNewline(const Token& token);

std::unique_ptr<Node> Parser::parse(Lexer& lexer) {
    if (lexer.peek().type == Token::Type::Eof) {
        /* This is not an error there is no expression */
        return nullptr;
    }
    auto ast = parseExpression(lexer, 0);
    if (lexer.peek().type != Token::Type::Eof) {
        if (error.empty()) {
            error = getUnexpectedTokenError(lexer);
        }
        return nullptr;
    }

    return ast;
}

std::unique_ptr<Node> Parser::parseExpression(Lexer& lexer, int min_bp){
    const Token& token = lexer.peek();
    const bool isValid = (token.type == Token::Type::Number)
                      || (token.type == Token::Type::Word)
                      || (token.type == Token::Type::Symbol && token.value[0] == '(')
                      || isTokenPreFix(token);
    if (!isValid) {
        return nullptr;
    }
    lexer.skip();
    std::unique_ptr<Node> lhs = nullptr;
    if (token.type == Token::Type::Symbol && token.value[0] == '(') {
        parenthesesLevel++;
        lhs = parseExpression(lexer, 0);
        const Token& pr = lexer.peek();
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
    }
    else if (token.type == Token::Type::Word && isPreDefinedFunction(token)) {
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
    }
    else if (isTokenPreFix(token)) {
        auto [lhs_bp, rhs_bp] = getBindingPower(token, true);
        std::unique_ptr<Node> op = createNode(token);
        std::unique_ptr<Node> arg = parseExpression(lexer, rhs_bp);
        if (arg == nullptr) {
            if (error.empty())
                if (lexer.peek().type == Token::Type::Eof)
                    error = getMissingOperandForPrefixError(token);
                else
                    error = getInvalidStartError(lexer);
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
                Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos-1, token.line_content};
                op = createNode(temp);
                lexer.addToken(temp);
                isImplicit = true;
            }
            else {
                op = createNode(lexer.peek());
            }
        }
        else if (lexer.peek().type == Token::Type::Word && token.type == Token::Type::Number) {
            Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos-1, token.line_content};
            op = createNode(temp);
            lexer.addToken(temp);
            isImplicit = true;
        }
        else {
            if (error.empty())
                error = getMissingOperatorError(token, lexer.peek());
            return nullptr;
        }

        auto&& [lhs_bp, rhs_bp] = getBindingPower(lexer.peek());
        if (lhs_bp < min_bp) break;
        if (isTokenPostFix(lexer.peek())) {
            lexer.skip();
            op->children.push_back(std::move(lhs));
            lhs = std::move(op);
            if (lexer.peek().type == Token::Type::Word || lexer.peek().type == Token::Type::Number) {
                Token temp{Token::Type::Symbol, "*", token.line, lexer.peek().pos-1, token.line_content};
                lexer.addToken(temp);
            }
            continue;
        }
        const auto& optoken = lexer.next();
        auto rhs = parseExpression(lexer, rhs_bp);
        if (rhs  == nullptr) {
            if (error.empty())
                error = getMissingRhsError(optoken, isImplicit);
            return nullptr;
        }
        op->children.push_back(std::move(lhs));
        op->children.push_back(std::move(rhs));
        lhs = std::move(op);
    }
    return lhs;
}

std::pair<int, int> getBindingPower(const Token&  token, bool isPrefix) {
    if (token.value.empty()) return std::make_pair(-1 , -1);
    if (isPrefix) {
        switch (token.value[0]) {
            case '-': case '+': return std::make_pair(-1, 4);
            default: return std::make_pair(-1 , -1);;
        }
    }
    if (token.type == Token::Type::Symbol) {
        switch (token.value[0]) {
            case '-': case '+': return std::make_pair(1, 2);
            case '*': case '/': return std::make_pair(3, 4);
            case '^': return std::make_pair(5, 4);
            case '!': return std::make_pair(5, -1);
            case '=': return std::make_pair(100, -1);
            default: return std::make_pair(-1, -1);
        }
    }
    if (token.type == Token::Type::Word) {
        return std::make_pair(0, 2);
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

static bool isTokenPreFix(const Token& token) {
    if (token.type != Token::Type::Symbol) return false;
    switch (token.value[0]) {
        case '-': case '+': return true;
        default: return false;
    }
}

std::string Parser::getError() {
    return error;
}

static std::string getNoArgError(const Lexer& lexer, const Token& token) {
    const Token& offending_token = lexer.peek();
    std::ostringstream oss;
    oss << "Parse Error: Expected an argument for function '" << token.value << "'";

    if (offending_token.type != Token::Type::Eof) {
        oss << ", but found '" << offending_token.value << "' instead.\n";
    } else {
        oss << " but reached the end of the input.\n";
    }

    const Token& ofToken = offending_token.type != Token::Type::Eof ? offending_token: token;

    oss << "--> at line " << ofToken.line << ":\n";
    oss << "    " << ofToken.line_content << "\n";
    oss << "    " << std::string(ofToken.pos, ' ') << "^-- Here";

    return oss.str();
}

static std::string getUnexpectedTokenError(const Lexer& lexer) {
    const Token& offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Parse Error: Unexpected token '" << offending_token.value << "'\n";
    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- This should not be here";

    return oss.str();
}

static std::string getInvalidStartError(const Lexer& lexer) {
    const Token& offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Parse Error: Invalid start of an expression. Cannot begin with token '"
        << offending_token.value << "'.\n";

    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- An expression cannot start here";

    return oss.str();
}

static std::string getMissingParenError(const Lexer& lexer, const Token& open_paren_token) {
    const Token& offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Parse Error: Missing closing ')' for parenthesis that started on line "
        << open_paren_token.line << ".\n";

    oss << "--> at line " << open_paren_token.line << ":\n";
    oss << "    " << open_paren_token.line_content << "\n";
    oss << "    " << std::string(open_paren_token.pos, ' ') << "^-- This parenthesis was never closed.\n\n";

    if (offending_token.type != Token::Type::Eof) {
        oss << "Instead, found '" << offending_token.value << "' here:\n";
        oss << "--> at line " << offending_token.line << ":\n";
        oss << "    " << offending_token.line_content << "\n";
        oss << "    " << std::string(offending_token.pos, ' ') << "^-- Expected ')'";
    } else {
        oss << "Instead, the input ended before the parenthesis was closed.";
    }

    return oss.str();
}

static std::string getMissingOperandForPrefixError(const Token& prefix_token) {
    std::ostringstream oss;
    oss << "Parse Error: Prefix operator '" << prefix_token.value
        << "' is missing an expression on its right-hand side.\n";

    oss << "--> at line " << prefix_token.line << ":\n";
    oss << "    " << prefix_token.line_content << "\n";
    oss << "    " << std::string(prefix_token.pos, ' ') << "^-- An expression was expected to follow this operator";

    return oss.str();
}

static std::string getMissingRhsError(const Token& operator_token, const bool isImplicit) {
    std::ostringstream oss;
    oss << "Parse Error: Infix operator '" << (isImplicit ? "implicit " : "") << operator_token.value
        << "' is missing a right-hand side expression.\n";

    oss << "--> at line " << operator_token.line << ":\n";
    oss << "    " << operator_token.line_content << "\n";
    oss << "    " << std::string(operator_token.pos, ' ') << "^-- An expression was expected to follow this operator";

    return oss.str();
}

static std::string getEmptyParenError(const Token& open_paren_token) {
    std::ostringstream oss;
    oss << "Parse Error: An expression was expected inside parentheses, but none was found.\n";

    oss << "--> at line " << open_paren_token.line << ":\n";
    oss << "    " << open_paren_token.line_content << "\n";
    oss << "    " << std::string(open_paren_token.pos, ' ') << "^-- Expected an expression after this parenthesis";

    return oss.str();
}

static std::string getMissingOperatorError(const Token& previous_token, const Token& offending_token) {
    std::ostringstream oss;
    oss << "Parse Error: Missing operator between '" << previous_token.value
        << "' and '" << offending_token.value << "'.\n";

    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- An operator was expected here.";

    return oss.str();
}

void Parser::clearError() {
    error.clear();
}

static bool isNewline(const Token& token) {
    return token.type == Token::Type::Symbol && token.value[0] == '\n';
}