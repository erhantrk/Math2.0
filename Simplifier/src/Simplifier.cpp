//
// Created by Erhan Türker on 11/2/25.
//

#include "../inc/Simplifier.hpp"
#include "../../Util/inc/ASTUtil.hpp"

TermData Simplifier::getTermParts(const std::shared_ptr<Node>& node) { // NOLINT(*-no-recursion)
    if (isNumber(node)) {
        return {getValue(node), nullptr};
    }

    if (node->type == Node::Type::Operand && node->value == "*") {
        auto leftParts = getTermParts(node->children[0]);
        auto rightParts = getTermParts(node->children[1]);

        double newCoefficient = leftParts.coefficient * rightParts.coefficient;

        std::shared_ptr<Node> newVarPart = nullptr;
        if (leftParts.variablePart == nullptr) {
            newVarPart = rightParts.variablePart;
        } else if (rightParts.variablePart == nullptr) {
            newVarPart = leftParts.variablePart;
        } else {
            newVarPart = std::make_shared<Node>(Node::Type::Operand, "*");
            newVarPart->children.push_back(leftParts.variablePart);
            newVarPart->children.push_back(rightParts.variablePart);
        }

        return {newCoefficient, newVarPart};
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 1) {
        auto parts = getTermParts(node->children[0]);
        parts.coefficient *= -1.0;
        return parts;
    }

    return {1.0, node};
}

static Factor getFactorParts(const std::shared_ptr<Node>& node) {
    if (node->type == Node::Type::Operand && node->value == "^") {
        if (isNumber(node->children[1])) {
            return {node->children[0], getValue(node->children[1])};
        }
    }
    return {node, 1.0};
}

void Simplifier::collectSumTermsImpl(const std::shared_ptr<Node>& node, double currentSign, std::list<Term>& terms) { // NOLINT(*-no-recursion)

    if (node->type == Node::Type::Operand && node->value == "+" && node->children.size() == 2) {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        collectSumTermsImpl(node->children[1], currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 2) {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        collectSumTermsImpl(node->children[1], -currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "-" && node->children.size() == 1) {
        collectSumTermsImpl(node->children[0], -currentSign, terms);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "+" && node->children.size() == 1) {
        collectSumTermsImpl(node->children[0], currentSign, terms);
        return;
    }

    auto parts = getTermParts(node);

    double finalCoefficient = parts.coefficient * currentSign;
    std::shared_ptr<Node> variablePart = parts.variablePart;

    terms.emplace_back(finalCoefficient, variablePart);
}

std::list<Term> Simplifier::collectSumTerms(const std::shared_ptr<Node>& node) {
    std::list<Term> terms;
    collectSumTermsImpl(node, 1, terms);
    return terms;
}

bool Simplifier::isSum(const Term& term) {
    if (term.variablePart == nullptr || term.variablePart->type != Node::Type::Operand) return false;
    return term.variablePart->value == "+" || term.variablePart->value == "-";
}

std::list<Term> Simplifier::expandTerm(const Term& term) {
    if (!isSum(term)) {
        return { term };
    }
    std::list<Term> innerTerms = collectSumTerms(term.variablePart);

    for (auto& innerTerm : innerTerms) {
        innerTerm.coefficient *= term.coefficient;
    }
    return innerTerms;
}

std::shared_ptr<Node> Simplifier::simplifySum(const std::shared_ptr<Node>& node) {
    std::list<Term> collectedTerms = collectSumTerms(node);
    std::list<Term> expandedTerms;
    for (const auto& term : collectedTerms) {
        auto distributedTerms = expandTerm(term);
        expandedTerms.splice(expandedTerms.end(), distributedTerms);
    }

    std::map<std::string, TermData> finalTerms;
    for (const auto& term : expandedTerms) {
        std::string key = term.getKey();
        finalTerms[key].coefficient += term.coefficient;
        if (finalTerms[key].variablePart == nullptr) {
            finalTerms[key].variablePart = term.variablePart;
        }
    }

    std::shared_ptr<Node> root = nullptr;
    for (auto const& [key, data] : finalTerms) {

        if (data.coefficient == 0.0) {
            continue;
        }
        std::shared_ptr<Node> termNode = nullptr;

        if (key == "##CONST##") {
            termNode = Node::createNode(data.coefficient);
        } else {
            if (data.coefficient == 1.0) {
                termNode = data.variablePart;
            }else if (data.coefficient == -1.0) {
                termNode = std::make_shared<Node>(Node::Type::Operand, "-");
                termNode->children.push_back(data.variablePart);
            }else {
                termNode = std::make_shared<Node>(Node::Type::Operand, "*");
                termNode->children.push_back(Node::createNode(data.coefficient));
                termNode->children.push_back(data.variablePart);
            }
        }

        if (root == nullptr) {
            root = termNode;
        } else if (data.coefficient >= 0){
            auto newRoot = std::make_shared<Node>(Node::Type::Operand, "+");
            newRoot->children.push_back(root);
            newRoot->children.push_back(termNode);
            root = newRoot;
        }
        else {
            auto newRoot = std::make_shared<Node>(Node::Type::Operand, "-");
            newRoot->children.push_back(root);
            termNode = std::move(termNode->children.back());
            newRoot->children.push_back(termNode);
            root = newRoot;
        }
    }

    if (root == nullptr) {
        root = Node::createNode(0.0);
    }

    return root;
}

void Simplifier::collectProductFactorsImpl(const std::shared_ptr<Node>& node, double currentPower, std::list<Factor>& factors) { // NOLINT(*-no-recursion)
    if (node->type == Node::Type::Operand && node->value == "*") {
        collectProductFactorsImpl(node->children[0], currentPower, factors);
        collectProductFactorsImpl(node->children[1], currentPower, factors);
        return;
    }

    if (node->type == Node::Type::Operand && node->value == "/") {
        collectProductFactorsImpl(node->children[0], currentPower, factors);
        collectProductFactorsImpl(node->children[1], -currentPower, factors);
        return;
    }

    auto parts = getFactorParts(node);

    parts.power *= currentPower;

    factors.emplace_back(parts.base, parts.power);
}

std::list<Factor> Simplifier::collectProductFactors(const std::shared_ptr<Node>& node) {
    std::list<Factor> factors;
    collectProductFactorsImpl(node, 1.0, factors);
    return factors;
}

bool Simplifier::isProduct(const Factor& factor) {
    if (factor.base == nullptr || factor.base->type != Node::Type::Operand) return false;
    return factor.base->value == "*" || factor.base->value == "/";
}

std::list<Factor> Simplifier::expandFactor(const Factor& factor) {
    if (!isProduct(factor)) {
        return { factor };
    }
    std::list<Factor> innerFactors = collectProductFactors(factor.base);
    for (auto& innerFactor : innerFactors) {
        innerFactor.power *= factor.power;
    }
    return innerFactors;
}

std::shared_ptr<Node> Simplifier::simplifyProduct(const std::shared_ptr<Node> &node) { // NOLINT(*-no-recursion)
    std::list<Factor> collectedFactors = collectProductFactors(node);
    std::list<Factor> expandedFactors;
    for (const auto& factor : collectedFactors) {
        auto distributedFactors = expandFactor(factor);
        expandedFactors.splice(expandedFactors.end(), distributedFactors);
    }

    double totalCoefficient = 1.0;
    std::map<std::string, FactorData> finalFactors;

    for (auto& factor : expandedFactors) {
        if (isNumber(factor.base)) {
            totalCoefficient *= std::pow(getValue(factor.base), factor.power);
        } else {
            std::string key = factor.getKey();
            finalFactors[key].totalPower += factor.power;
            if (finalFactors[key].base == nullptr) {
                finalFactors[key].base = factor.base;
            }
        }
    }

    std::list<std::shared_ptr<Node>> numeratorFactors;
    std::list<std::shared_ptr<Node>> denominatorFactors;

    double invCoefficient = 1.0 / totalCoefficient;
    if (invCoefficient == std::floor(invCoefficient) && invCoefficient != 1.0) {
        denominatorFactors.push_back(Node::createNode(invCoefficient));
    } else if (totalCoefficient != 1.0) {
        numeratorFactors.push_back(Node::createNode(totalCoefficient));
    }

    for (auto const& [key, data] : finalFactors) {
        if (data.totalPower == 0.0) {
            continue;
        }

        if (data.totalPower > 0) {
            if (data.totalPower == 1.0) {
                numeratorFactors.push_back(data.base);
            } else {
                const auto pwrOp = Node::createNode(Token(Token::Type::Symbol,"^"));
                const auto pwr = Node::createNode(data.totalPower);
                pwrOp->children.push_back(data.base);
                pwrOp->children.push_back(pwr);
                numeratorFactors.push_back(pwrOp);
            }
        } else if (data.totalPower < 0) {
            if (data.totalPower == -1.0) {
                denominatorFactors.push_back(data.base);
            } else {
                const auto pwrOp = Node::createNode(Token(Token::Type::Symbol,"^"));
                const auto pwr = Node::createNode(-data.totalPower);
                pwrOp->children.push_back(data.base);
                pwrOp->children.push_back(pwr);
                denominatorFactors.push_back(pwrOp);
            }
        }
    }

    std::shared_ptr<Node> numTree = nullptr;
    if (numeratorFactors.empty()) {
        numTree = Node::createNode(1.0);
    } else {
        numTree = numeratorFactors.front();
        numeratorFactors.pop_front();
        while(!numeratorFactors.empty()) {
            auto newRoot = std::make_shared<Node>(Node::Type::Operand, "*");
            newRoot->children.push_back(numTree);
            newRoot->children.push_back(numeratorFactors.front());
            numeratorFactors.pop_front();
            numTree = newRoot;
        }
    }

    if (denominatorFactors.empty()) {
        return numTree;
    }

    std::shared_ptr<Node> denTree = denominatorFactors.front();
    denominatorFactors.pop_front();
    while(!denominatorFactors.empty()) {
        auto newRoot = std::make_shared<Node>(Node::Type::Operand, "*");
        newRoot->children.push_back(denTree);
        newRoot->children.push_back(denominatorFactors.front());
        denominatorFactors.pop_front();
        denTree = newRoot;
    }

    auto finalRoot = std::make_shared<Node>(Node::Type::Operand, "/");
    finalRoot->children.push_back(simplifyNode(numTree));
    finalRoot->children.push_back(simplifyNode(denTree));

    return finalRoot;
}

std::shared_ptr<Node> Simplifier::constantFoldNode(std::shared_ptr<Node> node) {
    bool allChildrenAreNumbers = !node->children.empty();
    for (const auto& child : node->children) {
        if (!isNumber(child)) {
            allChildrenAreNumbers = false;
            break;
        }
    }

    if (allChildrenAreNumbers) {
        try {
            const std::string& op = node->value;
            // Operators
            if (op == "+" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) + getValue(node->children[1]));
            }
            if (op == "-" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) - getValue(node->children[1]));
            }
            if (op == "*" && node->children.size() == 2) {
                return Node::createNode(getValue(node->children[0]) * getValue(node->children[1]));
            }
            if (op == "/" && node->children.size() == 2) {
                double divisor = getValue(node->children[1]);
                if (divisor == 0.0) return node;
                return Node::createNode(getValue(node->children[0]) / divisor);
            }
            if (op == "^" && node->children.size() == 2) {
                return Node::createNode(std::pow(getValue(node->children[0]), getValue(node->children[1])));
            }
            if (op == "-" && node->children.size() == 1) {
                return Node::createNode(-getValue(node->children[0]));
            }
            if (op == "!" && node->children.size() == 1) {
                return Node::createNode(factorial(getValue(node->children[0])));
            }

            // Functions
            if (op == "sin" && node->children.size() == 1) return Node::createNode(std::sin(getValue(node->children[0])));
            if (op == "cos" && node->children.size() == 1) return Node::createNode(std::cos(getValue(node->children[0])));
            if (op == "tan" && node->children.size() == 1) return Node::createNode(std::tan(getValue(node->children[0])));
            if (op == "sqrt" && node->children.size() == 1) return Node::createNode(std::sqrt(getValue(node->children[0])));
            if (op == "log" && node->children.size() == 1) return Node::createNode(std::log10(getValue(node->children[0])));
            if (op == "ln" && node->children.size() == 1) return Node::createNode(std::log(getValue(node->children[0])));
            if (op == "abs" && node->children.size() == 1) return Node::createNode(std::abs(getValue(node->children[0])));
            if (op == "atan2" && node->children.size() == 2) return Node::createNode(std::atan2(getValue(node->children[0]), getValue(node->children[1])));

        } catch (const std::exception& _) {
            return node;
        }
    }
    return node;
}

std::shared_ptr<Node> Simplifier::simplifyPower(const std::shared_ptr<Node>& node) { // NOLINT(*-no-recursion)
    auto& base = node->children[0];
    auto& exponent = node->children[1];

    //(a * b)^n -> (a^n) * (b^n)
    if (base->type == Node::Type::Operand && base->value == "*") {
        auto& a = base->children[0];
        auto& b = base->children[1];

        auto newLeft = std::make_shared<Node>(Node::Type::Operand, "^");
        newLeft->children.push_back(a);
        newLeft->children.push_back(exponent);

        auto newRight = std::make_shared<Node>(Node::Type::Operand, "^");
        newRight->children.push_back(b);
        newRight->children.push_back(exponent);

        auto newProduct = std::make_shared<Node>(Node::Type::Operand, "*");
        newProduct->children.push_back(newLeft);
        newProduct->children.push_back(newRight);

        return simplifyNode(newProduct);
    }

    // (a / b)^n  ->  (a^n) / (b^n)
    if (base->type == Node::Type::Operand && base->value == "/") {
        auto& a = base->children[0];
        auto& b = base->children[1];

        auto newNum = std::make_shared<Node>(Node::Type::Operand, "^");
        newNum->children.push_back(a);
        newNum->children.push_back(exponent);

        auto newDen = std::make_shared<Node>(Node::Type::Operand, "^");
        newDen->children.push_back(b);
        newDen->children.push_back(exponent);

        auto newDivision = std::make_shared<Node>(Node::Type::Operand, "/");
        newDivision->children.push_back(newNum);
        newDivision->children.push_back(newDen);

        return simplifyNode(newDivision);
    }

    // (a^m)^n -> a^(m*n)
    if (base->type == Node::Type::Operand && base->value == "^") {
        auto& a = base->children[0];
        auto& m = base->children[1];

        auto newExponent = std::make_shared<Node>(Node::Type::Operand, "*");
        newExponent->children.push_back(m);
        newExponent->children.push_back(exponent);

        auto newPower = std::make_shared<Node>(Node::Type::Operand, "^");
        newPower->children.push_back(a);
        newPower->children.push_back(newExponent);

        return simplifyNode(newPower);
    }

    return node;
}
std::shared_ptr<Node> Simplifier::simplifyNode(std::shared_ptr<Node> node) { // NOLINT(*-no-recursion)
    if (!node) {
        return nullptr;
    }

    for (auto& child : node->children) {
        child = simplifyNode(child);
    }

    if (node->type != Node::Type::Operand && node->type != Node::Type::Function) {
        return node;
    }

    // 2. --- Constant Folding ---
    node = std::move(constantFoldNode(node));

    // We need pre simplification of 0 / x and x / inf cases
    if (node->children.size() == 2) {
        auto& lhs = node->children[0];
        auto& rhs = node->children[1];
        const std::string& op = node->value;
        if (op == "/") {
            if (rhs->value == "inf") return Node::createNode(0.0f);
            if (isNumber(lhs) && getValue(lhs) == 0.0) return Node::createNode(0.0f);
        }
    }

    // 3. --- DISPATCHER ---
    if (node->type == Node::Type::Operand && (node->value == "+" || node->value == "-")) {
        node = std::move(simplifySum(node));
    }

    if (node->type == Node::Type::Operand && (node->value == "*" || node->value == "/")) {
        node = std::move(simplifyProduct(node));
    }

    if (node->type == Node::Type::Operand && node->value == "^") {
        node = std::move(simplifyPower(node));
    }

    // 4. --- Algebraic Simplification ---
    if (node->children.size() == 2) {
        auto& lhs = node->children[0];
        auto& rhs = node->children[1];
        const std::string& op = node->value;

        if (op == "*") {
            // x * 1 -> x
            if (isNumber(rhs) && getValue(rhs) == 1.0) return lhs;
            // 1 * x -> x
            if (isNumber(lhs) && getValue(lhs) == 1.0) return rhs;
            // x * 0 -> 0 or 0 * x -> 0
            if ((isNumber(rhs) && getValue(rhs) == 0.0) || (isNumber(lhs) && getValue(lhs) == 0.0))
                return Node::createNode(0.0);
        }
        else if (op == "/") {
            if (rhs->value == "inf") return Node::createNode(0.0f);
            if (isNumber(lhs) && getValue(lhs) == 0.0) return Node::createNode(0.0f);
        }
        else if (op == "+") {
            // x + 0 -> x
            if (isNumber(rhs) && getValue(rhs) == 0.0) return lhs;
            // 0 + x -> x
            if (isNumber(lhs) && getValue(lhs) == 0.0) return rhs;

            // -1 + x -> x - 1
            if (isNumber(lhs) && getValue(lhs) < 0.0) {
                node->value = '-';
                auto const ch = node->children[0];
                ch->value = takeNegative(ch->value);
                node->children[0] = std::move(node->children[1]);
                node->children[1] = ch;
            }
        }
        else if (op == "-") {
            // x - 0 -> x
            if (isNumber(rhs) && getValue(rhs) == 0.0) return lhs;
            // 0 - x -> -x
            if (isNumber(lhs) && getValue(lhs) == 0.0) {
                auto minus = Node::createNode(Token(Token::Type::Symbol,"-"));
                minus->children.push_back(rhs);
                rhs = std::move(minus);
                return rhs;
            }

        }
        else if (op == "^") {
            // x ^ 1 -> x
            if (isNumber(rhs) && getValue(rhs) == 1.0) return lhs;
            // x ^ 0 -> 1
            if (isNumber(rhs) && getValue(rhs) == 0.0) return Node::createNode(1.0);
            // 1 ^ x -> 1
            if (isNumber(lhs) && getValue(lhs) == 1.0) return Node::createNode(1.0);
        }
    }

    return node;
}

std::shared_ptr<Node> Simplifier::simplify(const std::shared_ptr<Node>& node) {
    if (!node) {
        return nullptr;
    }
    auto clonedNode = node->clone();
    return simplifyNode(clonedNode);
}
