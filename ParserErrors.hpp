//
// Created by Erhan Türker on 10/11/25.
//

#pragma once
#include <string>
#include "Lexer.hpp"


std::string getNoArgError(const Lexer &lexer, const Token &token);

std::string getMultilineWoParenError(const Token &token);

std::string getUnexpectedTokenError(const Lexer &lexer);

std::string getInvalidStartError(const Lexer &lexer);

std::string getMissingParenError(const Lexer &lexer, const Token &open_paren_token);

std::string getMissingOperandForPrefixError(const Token &prefix_token);

std::string getMissingRhsError(const Token &operator_token, bool isImplicit);

std::string getEmptyParenError(const Token &open_paren_token);

std::string getMissingOperatorError(const Token &previous_token, const Token &offending_token);

std::string getInvalidAssignmentTargetError(const Token &token);

std::string getMissingAssignmentError(const Token &operator_token);

std::string getUndefinedVariableError(const Token &as, const std::string &variableName);
