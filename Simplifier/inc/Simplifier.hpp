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

    Term(double coefficient, std::shared_ptr<Node> varPart)
        : coefficient(coefficient), variablePart(std::move(varPart)) {}

    [[nodiscard]] std::string getKey() const {
        if (variablePart == nullptr) {
            return "##CONST##";
        }
        return toLisp(variablePart);
    }
};

struct TermData {
    double coefficient = 0.0;
    std::shared_ptr<Node> variablePart = nullptr;
    double power = 0.0;
};

struct Factor {
    std::shared_ptr<Node> base;
    double power;

    Factor(std::shared_ptr<Node> b, double p) : base(std::move(b)), power(p) {}

    [[nodiscard]] std::string getKey() const { return toLisp(base); }
};

struct FactorData {
    double totalPower = 0.0;
    std::shared_ptr<Node> base = nullptr;
};


class Simplifier {
    static TermData getTermParts(const std::shared_ptr<Node>& node);
    static void collectSumTermsImpl(const std::shared_ptr<Node>& node, double currentSign, std::list<Term>& terms);
    static std::list<Term> collectSumTerms(const std::shared_ptr<Node>& node);
    static bool isSum(const Term& term);
    static std::list<Term> expandTerm(const Term& term);
    static std::shared_ptr<Node> simplifySum(const std::shared_ptr<Node> &node);

    static void collectProductFactorsImpl(const std::shared_ptr<Node>& node, double currentPower, std::list<Factor>& factors);
    static std::list<Factor> collectProductFactors(const std::shared_ptr<Node>& node);
    static bool isProduct(const Factor& factor);
    static std::list<Factor> expandFactor(const Factor& factor);
    static std::shared_ptr<Node> simplifyProduct(const std::shared_ptr<Node> &node);

    static std::shared_ptr<Node> simplifyPower(const std::shared_ptr<Node> &node);
    static std::shared_ptr<Node> constantFoldNode(std::shared_ptr<Node> node);

    static std::shared_ptr<Node> simplifyNode(std::shared_ptr<Node> node);
public:
    static std::shared_ptr<Node> simplify(const std::shared_ptr<Node>& node);
};

