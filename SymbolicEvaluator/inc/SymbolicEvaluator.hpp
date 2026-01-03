//
// Created by Erhan Türker on 10/29/25.
//
#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../Node/inc/Node.hpp"

class SymbolicEvaluator {
public:
    std::shared_ptr<Node> expand(const std::shared_ptr<Node>& node);
    void clearVariable(const std::string& name);
    void registerFunction(const std::shared_ptr<Node>& funcDefNode);
    void registerVariable(std::pair<std::string, double> var);

private:
    std::shared_ptr<Node> expandNode(const std::shared_ptr<Node>& node);

    static std::shared_ptr<Node> substituteParameters(const std::shared_ptr<Node>& body, const std::vector<std::shared_ptr<Node>>& arguments);

    std::unordered_map<std::string, std::shared_ptr<const Node>> functions;
    std::unordered_map<std::string, double> variables;
};