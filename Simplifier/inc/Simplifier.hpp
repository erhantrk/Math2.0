//
// Created by Erhan Türker on 11/2/25.
//

#pragma once
#include "../../Node/inc/Node.hpp"
#include "../../Util/inc/ASTPrint.hpp"
#include <list>

struct Term {
    double coefficient;
    std::shared_ptr<Node> variablePart; // nullptr for constants

    Term(double coeff, std::shared_ptr<Node> varPart)
        : coefficient(coeff), variablePart(std::move(varPart)) {}

    std::string getKey() const {
        if (variablePart == nullptr) {
            return "##CONST##";
        }
        return toLisp(variablePart);
    }
};

struct TermData {
    double totalCoefficient = 0.0;
    std::shared_ptr<Node> variablePart = nullptr;
};


class Simplifier {
    static std::pair<double, std::shared_ptr<Node>> getTermParts(const std::shared_ptr<Node>& node);
    static void collectSumTermsImpl(const std::shared_ptr<Node>& node, double currentSign, std::list<Term>& terms);
    static std::list<Term> collectSumTerms(const std::shared_ptr<Node>& node);
    static bool isSum(const Term& term);
    static std::list<Term> expandTerm(const Term& term);
    static std::shared_ptr<Node> simplifySum(const std::shared_ptr<Node> &node);
    static std::shared_ptr<Node> simplifyNode(std::shared_ptr<Node> node);
public:
    static std::shared_ptr<Node> simplify(const std::shared_ptr<Node>& node);
};

