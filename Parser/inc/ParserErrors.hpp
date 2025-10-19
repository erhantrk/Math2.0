//
// Created by Erhan Türker on 10/11/25.
//

#pragma once
#include <string>
#include "../../Lexer/inc/Lexer.hpp"

class ParserError {
public:
    static std::string NoArg(const Lexer &lexer, const Token &token);
    
    static std::string MultilineWoParen(const Token &token);

    static std::string UnexpectedToken(const Lexer &lexer);

    static std::string InvalidStart(const Lexer &lexer);

    static std::string MissingParen(const Lexer &lexer, const Token &open_paren_token);

    static std::string MissingOperandForPrefix(const Token &prefix_token);

    static std::string MissingRhs(const Token &operator_token, bool isImplicit);

    static std::string EmptyParen(const Token &open_paren_token);

    static std::string MissingOperator(const Token &previous_token, const Token &offending_token);

    static std::string InvalidAssignmentTarget(const Token &token);

    static std::string MissingAssignment(const Token &operator_token);

    static std::string UndefinedVariable(const Token &as, const std::string &variableName);

    static std::string NotEnoughArguments(const Token &function, int argCount);

    static std::string TooManyArguments(const Token &function, int argCount);

    static std::string MultiArgumentCalledWoParentheses(const Token &function, int argCount);

    static std::string EmptyArgument(const Token& comma);
};