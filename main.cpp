#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <fstream>
#include <sstream>

#include "crow_all.h" // Ensure this is in your folder

#include "Lexer/inc/Lexer.hpp"
#include "Parser/inc/Parser.hpp"
#include "Evaluator/inc/Evaluator.hpp"
#include "SymbolicEvaluator/inc/SymbolicEvaluator.hpp"
#include "Util/inc/ASTPrint.hpp"

// We modify processInput to write to a stringstream instead of cout
void processInput(const std::string& input, Parser& parser, Evaluator& evaluator, SymbolicEvaluator& sEvaluator, std::stringstream& out) {
    if (input.empty()) return;

    // --- NEW: Handle "clear x" ---
    if (input.substr(0, 6) == "clear ") {
        std::string varName = input.substr(6);
        // Trim whitespace
        varName.erase(0, varName.find_first_not_of(" \n\r\t"));
        varName.erase(varName.find_last_not_of(" \n\r\t") + 1);

        evaluator.clearVariable(varName);
        sEvaluator.clearVariable(varName);
        out << "Variable '" << varName << "' cleared.\n";
        return;
    }

    Lexer lexer(input);
    if (!lexer.getError().empty()) {
        out << "Error: " << lexer.getError() << "\n";
        return;
    }

    auto ast = parser.parse(lexer);
    if (!parser.getError().empty()) {
        out << "Error: " << parser.getError() << "\n";
        parser.clearError();
        return;
    }

    for (const auto& node : ast) {
        // --- CASE 1: DEFINITIONS (f(x) = ...) ---
        if (node->type == Node::Type::FunctionAssignment) {
            // 1. Expand symbolicly (resolve d/dx, simplify)
            auto expandedNode = sEvaluator.expand(node);

            // 2. Register Symbolic
            sEvaluator.registerFunction(expandedNode);

            // 3. Register Numeric (So we can plot it)
            evaluator.evaluate(expandedNode);

            out << "Defined: " << toHumanReadable(expandedNode) << "\n";
            continue;
        }

        // --- CASE 2: EXPRESSIONS (d/dx(x), 5+5, etc.) ---

        // STEP 1: Always Expand First!
        // This converts "d/dx(x)" -> "1" BEFORE we try to calculate it.
        auto expandedNode = sEvaluator.expand(node);

        // STEP 2: Try Numeric Evaluation on the expanded result
        double val = evaluator.evaluate(expandedNode);
        std::string evalError = evaluator.getError();

        if (evalError.empty()) {
            // Success: It's a number (e.g., 1, 10, 0.5)
            if (node->type != Node::Type::Assignment) {
                 out << val << "\n";
            } else {
                 // Variable Assignment: x = 5
                 out << toHumanReadable(expandedNode) << " = " << val << "\n";
            }
        } else {
            // Failure: It's likely purely symbolic (e.g., "x + y" where y is unknown)
            // We just print the expanded symbolic form.
            out << "--> " << toHumanReadable(expandedNode) << "\n";
        }
    }
}
int main(int argc, char* argv[]) {
    // Persistent State
    Parser parser;
    Evaluator evaluator;
    SymbolicEvaluator sEvaluator;

    crow::SimpleApp app;

    // 1. Serve the HTML Interface
    CROW_ROUTE(app, "/")([](){
        std::ifstream file("index.html");
        if(file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            return crow::response(buffer.str());
        }
        return crow::response(500, "Could not load index.html");
    });

    // 2. The Execution Endpoint
    // It accepts raw text, runs it, and returns the text result.
    CROW_ROUTE(app, "/api/calculate").methods("POST"_method)
    ([&](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400, "Invalid JSON");

        std::string code = x["code"].s();
        std::stringstream outputBuffer;

        processInput(code, parser, evaluator, sEvaluator, outputBuffer);

        crow::json::wvalue resp;
        resp["result"] = outputBuffer.str();
        return crow::response(resp);
    });

    // 3. Reset Memory
    CROW_ROUTE(app, "/api/reset").methods("POST"_method)
    ([&](){
        evaluator = Evaluator();
        sEvaluator = SymbolicEvaluator();
        return crow::response("Memory Cleared");
    });

    std::cout << "Math Engine running on port 8080..." << std::endl;
    app.port(8080).multithreaded().run();

    return 0;
}