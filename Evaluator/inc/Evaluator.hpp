//
// Created by Erhan Türker on 10/17/25.
//
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../Node/inc/Node.hpp"

class Evaluator {
public:
    double evaluate(const std::shared_ptr<Node>& node);
    void clearVariable(const std::string& name);
    [[nodiscard]] std::string getError() const;

private:
    struct CallFrame {
        std::vector<double> evaluatedArguments;
    };

    double evaluateNode(const std::shared_ptr<Node>& node);

    std::unordered_map<std::string, double> variables;

    std::unordered_map<std::string, std::weak_ptr<const Node>> functions;

    std::vector<CallFrame> callStack;
    std::string error;
};

