#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"
#include "Evaluator/inc/Evaluator.hpp"
#include "SymbolicEvaluator/inc/SymbolicEvaluator.hpp"
#include "Util/inc/ASTPrint.hpp"

const std::string RESET = "\033[0m";
const std::string RED = "\033[31m";

int getParenBalance(const std::string& s) {
    int balance = 0;
    for (char c : s) {
        if (c == '(') balance++;
        else if (c == ')') balance--;
    }
    return balance;
}

void processInput(const std::string& input, Parser& parser, Evaluator& evaluator, SymbolicEvaluator& sEvaluator) {
    if (input.empty()) {
        return;
    }

    Lexer lexer(input);
    if (!lexer.getError().empty()) {
        std::cout << RED << "Lexer Error: " << lexer.getError() << RESET << '\n';
        return;
    }

    auto ast = parser.parse(lexer);
    if (!parser.getError().empty()) {
        std::cout << RED << "Parser Error: " << parser.getError() << RESET << '\n';
        parser.clearError();
        return;
    }

    for (const auto& node : ast) {
        if (node->type == Node::Type::FunctionAssignment) {
            evaluator.evaluate(node);
            sEvaluator.registerFunction(node);
            std::cout << "Defined: " << toHumanReadable(node) << '\n';
            continue;
        }

        double val = evaluator.evaluate(node);
        std::string evalError = evaluator.getError();

        if (evalError.empty()) {
            std::cout << toHumanReadable(node) << " = " << val << '\n';
            if (node->type == Node::Type::Assignment) {
                sEvaluator.registerVariable(std::make_pair(node->value, val));
            }
        } else {
            std::cout << "Expanding: " << toHumanReadable(node) << '\n';
            auto expandedNode = sEvaluator.expand(node);
            std::cout << "--> " << toLisp(expandedNode) << '\n';
            std::cout << "--> " << toHumanReadable(expandedNode) << '\n';
        }
    }
}

void processFile(const std::string& filePath, Parser& parser, Evaluator& evaluator, SymbolicEvaluator& sEvaluator) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cout << RED << "Error: Could not open file '" << filePath << "'" << RESET << '\n';
        return;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    processInput(buffer.str(), parser, evaluator, sEvaluator);
    std::cout.flush();
}

void runRepl(Parser& parser, Evaluator& evaluator, SymbolicEvaluator& sEvaluator) {
    std::cout << "Math2.0 REPL Mode. Type 'exit' to quit." << std::endl;
    std::cout << "Input will auto-continue if parentheses are open." << std::endl;

    std::string line;
    std::string multiLineInput;
    int parenBalance = 0;

    while (true) {
        if (parenBalance == 0) {
            std::cout << ">> " << std::flush;
            multiLineInput.clear();
        } else {
            std::cout << ".. " << std::flush;
        }

        if (!std::getline(std::cin, line)) {
            break;
        }

        if (line == "exit") {
            if (parenBalance != 0) {
                std::cout << RED << "Input canceled." << RESET << std::endl;
                parenBalance = 0;
            } else {
                break;
            }
            continue;
        }

        multiLineInput += line + "\n";
        parenBalance = getParenBalance(multiLineInput);

        if (parenBalance <= 0) {
            processInput(multiLineInput, parser, evaluator, sEvaluator);
            std::cout.flush();
            parenBalance = 0;
        }
    }
}


int main(int argc, char* argv[]) {
    Parser parser;
    Evaluator evaluator;
    SymbolicEvaluator sEvaluator;

    if (argc == 1) {
        runRepl(parser, evaluator, sEvaluator);
    } else {
        std::string arg = argv[1];
        processFile(arg, parser, evaluator, sEvaluator);
    }

    return 0;
}