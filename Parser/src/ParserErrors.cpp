//
// Created by Erhan Türker on 10/11/25.
//

#include "../inc/ParserErrors.hpp"
#include <sstream>

std::string getNoArgError(const Lexer &lexer, const Token &token) {
    const Token &offending_token = lexer.peek();
    std::ostringstream oss;
    oss << "Expected an argument for function '" << token.value << "'";

    if (offending_token.type != Token::Type::Eof && offending_token.type != Token::Type::Newline) {
        oss << ", but found '" << offending_token.value << "' instead.\n";
    } else {
        oss << " but reached the end of the input.\n";
    }

    const Token &ofToken = offending_token.type != Token::Type::Eof ? offending_token : token;

    oss << "--> at line " << ofToken.line << ":\n";
    oss << "    " << ofToken.line_content << "\n";
    oss << "    " << std::string(ofToken.pos, ' ') << "^-- Here";

    return oss.str();
}

std::string getUnexpectedTokenError(const Lexer &lexer) {
    const Token &offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Unexpected token '" << offending_token.value << "'\n";
    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- This should not be here";

    return oss.str();
}

std::string getInvalidStartError(const Lexer &lexer) {
    const Token &offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Invalid start of an expression. Cannot begin with token '"
            << offending_token.value << "'.\n";

    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- An expression cannot start here";

    return oss.str();
}

std::string getMissingParenError(const Lexer &lexer, const Token &open_paren_token) {
    const Token &offending_token = lexer.peek();

    std::ostringstream oss;
    oss << "Missing closing ')' for parenthesis that started on line "
            << open_paren_token.line << ".\n";

    oss << "--> at line " << open_paren_token.line << ":\n";
    oss << "    " << open_paren_token.line_content << "\n";
    oss << "    " << std::string(open_paren_token.pos, ' ') << "^-- This parenthesis was never closed.\n\n";

    if (offending_token.type != Token::Type::Eof && offending_token.type != Token::Type::Newline) {
        oss << "Instead, found '" << offending_token.value << "' here:\n";
        oss << "--> at line " << offending_token.line << ":\n";
        oss << "    " << offending_token.line_content << "\n";
        oss << "    " << std::string(offending_token.pos, ' ') << "^-- Expected ')'";
    } else {
        oss << "Instead, the input ended before the parenthesis was closed.";
    }

    return oss.str();
}

std::string getMissingOperandForPrefixError(const Token &prefix_token) {
    std::ostringstream oss;
    oss << "Prefix operator '" << prefix_token.value
            << "' is missing an expression on its right-hand side.\n";

    oss << "--> at line " << prefix_token.line << ":\n";
    oss << "    " << prefix_token.line_content << "\n";
    oss << "    " << std::string(prefix_token.pos, ' ') << "^-- An expression was expected to follow this operator";

    return oss.str();
}

std::string getMissingRhsError(const Token &operator_token, const bool isImplicit) {
    std::ostringstream oss;
    oss << "Infix operator '" << (isImplicit ? "implicit " : "") << operator_token.value
            << "' is missing a right-hand side expression.\n";

    oss << "--> at line " << operator_token.line << ":\n";
    oss << "    " << operator_token.line_content << "\n";
    oss << "    " << std::string(operator_token.pos, ' ') << "^-- An expression was expected to follow this operator";

    return oss.str();
}

std::string getMissingAssignmentError(const Token &operator_token) {
    std::ostringstream oss;
    oss << "Assignment operator '=' is missing a right-hand side expression.\n";
    oss << "--> at line " << operator_token.line << ":\n";
    oss << "    " << operator_token.line_content << "\n";
    oss << "    " << std::string(operator_token.pos, ' ') << "^-- An expression was expected to follow the assignment.";

    return oss.str();
}

std::string getEmptyParenError(const Token &open_paren_token) {
    std::ostringstream oss;
    oss << "An expression was expected inside parentheses, but none was found.\n";

    oss << "--> at line " << open_paren_token.line << ":\n";
    oss << "    " << open_paren_token.line_content << "\n";
    oss << "    " << std::string(open_paren_token.pos, ' ') << "^-- Expected an expression after this parenthesis";

    return oss.str();
}

std::string getMissingOperatorError(const Token &previous_token, const Token &offending_token) {
    std::ostringstream oss;
    oss << "Missing operator between '" << previous_token.value
            << "' and '" << offending_token.value << "'.\n";

    oss << "--> at line " << offending_token.line << ":\n";
    oss << "    " << offending_token.line_content << "\n";
    oss << "    " << std::string(offending_token.pos, ' ') << "^-- An operator was expected here.";

    return oss.str();
}

std::string getMultilineWoParenError(const Token &token) {
    std::ostringstream oss;
    oss << "Multiline expressions must be enclosed in parentheses.\n";
    oss << "--> at line " << token.line << ":\n";
    oss << "    " << token.line_content << "\n";
    oss << "    " << std::string(token.pos, ' ') << "^-- An expression cannot be split across lines here.\n";
    oss << "    " << std::string(token.pos, ' ') << "   Consider wrapping the entire expression in parentheses `()`.";

    return oss.str();
}

std::string getInvalidAssignmentTargetError(const Token &token) {
    std::ostringstream oss;
    oss << "Invalid target for assignment.\n";
    oss << "--> at line " << token.line << ":\n";
    oss << "    " << token.line_content << "\n";
    oss << "    " << std::string(token.pos, ' ') << "^-- Cannot assign to this expression.";
    return oss.str();
}

std::string getUndefinedVariableError(const Token &as, const std::string &variableName) {
    size_t var_col = as.line_content.find(variableName, as.pos);

    if (var_col == std::string::npos) {
        var_col = as.pos;
    }

    std::ostringstream out;
    out << "Use of undefined variable '" << variableName << "'.\n"
            << "--> at line " << as.line << ":\n"
            << "    " << as.line_content << "\n"
            << "    " << std::string(var_col, ' ')
            << "^-- This variable has not been defined";

    return out.str();
}
