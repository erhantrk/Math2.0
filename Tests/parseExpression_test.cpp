//
// Created by Erhan Türker on 10/8/25.
//

#include "catch_amalgamated.hpp"
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "../Parser/inc/Parser.hpp"
#include "../Lexer/inc/Lexer.hpp"

static void toLispImpl(const Node *n, std::ostringstream &out) {
    if (!n) {
        out << "<null>";
        return;
    }

    switch (n->type) {
        case Node::Type::Number:
            out << n->value;
            break;
        case Node::Type::Variable:
            out << n->value;
            break;
        case Node::Type::Assignment:
            out << "(= " << n->value << " ";
            toLispImpl(n->children[0].get(), out);
            out << ")";
            break;
        case Node::Type::Operand:
            if (n->value == "!" && !n->children.empty()) {
                out << "(! ";
                toLispImpl(n->children[0].get(), out);
                out << ")";
            } else {
                out << "(" << n->value;
                for (size_t i = 0; i < n->children.size(); ++i) {
                    out << " ";
                    toLispImpl(n->children[i].get(), out);
                }
                out << ")";
            }
            break;
        case Node::Type::Function:
            out << "(" << n->value;
            if (!n->children.empty()) {
                out << " ";
                for (size_t i = 0; i < n->children.size(); ++i) {
                    if (i) out << " ";
                    toLispImpl(n->children[i].get(), out);
                }
            }
            out << ")";
            break;
    }
}

static std::string toLisp(const std::unique_ptr<Node> &n) {
    std::ostringstream out;
    toLispImpl(n.get(), out);
    return out.str();
}



TEST_CASE("simple number") {
    Lexer lx("1");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "1");
}

TEST_CASE("Simple addition and subtraction") {
    Lexer lx("5 - 3 + 2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(+ (- 5 3) 2)");
}

TEST_CASE("Simple multiplication and division") {
    Lexer lx("10 / 5 * 2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (/ 10 5) 2)");
}

TEST_CASE("Exponent with multiplication") {
    Lexer lx("2 * 3^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (^ 3 2))");
}

TEST_CASE("Exponent with parentheses") {
    Lexer lx("(2 * 3)^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(^ (* 2 3) 2)");
}

TEST_CASE("Unary minus with addition") {
    Lexer lx("-5 + -3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(+ (- 5) (- 3))");
}

TEST_CASE("Multiplication with unary minus") {
    Lexer lx("5 * -3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 5 (- 3))");
}

TEST_CASE("Double negative") {
    Lexer lx("5--3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- 5 (- 3))");
}

TEST_CASE("Double factorial") {
    Lexer lx("3!!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (! 3))");
}

TEST_CASE("Factorial with parentheses") {
    Lexer lx("(x+y)!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (+ x y))");
}

TEST_CASE("Unary minus with factorial") {
    Lexer lx("-x!");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (! x))");
}

TEST_CASE("Simple function call tan") {
    Lexer lx("tan(x)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(tan x)");
}

TEST_CASE("Simple function call log") {
    Lexer lx("log(10)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(log 10)");
}

TEST_CASE("Function call with expression") {
    Lexer lx("sqrt(x^2 + y^2)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sqrt (+ (^ x 2) (^ y 2)))");
}

TEST_CASE("Implicit multiplication with function") {
    Lexer lx("2sin(x)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (sin x))");
}

TEST_CASE("Chained function calls") {
    Lexer lx("sin cos tan x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sin (cos (tan x)))");
}

TEST_CASE("Implicit multiplication with parentheses") {
    Lexer lx("(x+1)(y+2)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (+ x 1) (+ y 2))");
}

TEST_CASE("Simple assignment") {
    Lexer lx("y = m*x + c");
    Parser parser;
    parser.defineVariable("y");
    parser.defineVariable("m");
    parser.defineVariable("x");
    parser.defineVariable("c");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(= y (+ (* m x) c))");
}

TEST_CASE("Assignment with complex expression") {
    Lexer lx("x = -b / (2a)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("b");
    parser.defineVariable("a");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(= x (/ (- b) (* 2 a)))");
}

TEST_CASE("Complex polynomial") {
    Lexer lx("3x^2 + 2y - 1");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (+ (* 3 (^ x 2)) (* 2 y)) 1)");
}

TEST_CASE("Complex expression with unary minus and factorial") {
    Lexer lx("-(x+y)*z!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (- (+ x y)) (! z))");
}

TEST_CASE("Complex expression with functions and factorials") {
    Lexer lx("sin(x!) ^ cos(y!)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(^ (sin (! x)) (cos (! y)))");
}

TEST_CASE("Scientific notation with exponent") {
    Lexer lx("2.5 * 10^3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2.5 (^ 10 3))");
}

TEST_CASE("Chained division") {
    Lexer lx("a / b / c");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(/ (/ a b) c)");
}

TEST_CASE("Chained subtraction") {
    Lexer lx("a - b - c");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (- a b) c)");
}

TEST_CASE("Function with unary minus argument") {
    Lexer lx("sin -x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sin (- x))");
}

TEST_CASE("Factorial of factorial") {
    Lexer lx("(2!)!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (! 2))");
}

TEST_CASE("Mixed precedence addition and multiplication") {
    Lexer lx("a + b * c + d");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    parser.defineVariable("d");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(+ (+ a (* b c)) d)");
}

TEST_CASE("Mixed precedence with parentheses") {
    Lexer lx("a * (b + c) * d");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    parser.defineVariable("d");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (* a (+ b c)) d)");
}

TEST_CASE("Implicit multiplication with nested parentheses") {
    Lexer lx("3(4(5))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 3 (* 4 5))");
}

TEST_CASE("Implicit multiplication of variables in parentheses") {
    Lexer lx("(x)(y)(z)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (* x y) z)");
}

TEST_CASE("Unary minus vs exponent precedence") {
    Lexer lx("-2^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (^ 2 2))");
}

TEST_CASE("Parentheses with unary minus and exponent") {
    Lexer lx("(-2)^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(^ (- 2) 2)");
}

TEST_CASE("Complex mixed arithmetic") {
    Lexer lx("1 + 2 * 3 / 4 - 5");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (+ 1 (/ (* 2 3) 4)) 5)");
}

TEST_CASE("Implicit multiplication of factorial and variable") {
    Lexer lx("x!y");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! x) y)");
}

TEST_CASE("Implicit multiplication of functions") {
    Lexer lx("sin(x)cos(y)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (sin x) (cos y))");
}

TEST_CASE("Implicit multiplication of number and factorial") {
    Lexer lx("3x!");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 3 (! x))");
}

TEST_CASE("Function call without parentheses") {
    Lexer lx("sqrt 9");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sqrt 9)");
}

TEST_CASE("Multiplication with unary minus variable") {
    Lexer lx("2 * -x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (- x))");
}

TEST_CASE("Chained unary operators") {
    Lexer lx("+-x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(+ (- x))");
}

TEST_CASE("Implicit multiplication with number and parenthesized factorial") {
    Lexer lx("2(x!)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (! x))");
}

TEST_CASE("Division with parenthesized expression") {
    Lexer lx("1 / (x+1)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(/ 1 (+ x 1))");
}

TEST_CASE("Right-associative exponentiation with factorial") {
    Lexer lx("x^y^z!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(^ x (^ y (! z)))");
}

TEST_CASE("Factorial of a complex function call") {
    Lexer lx("sin( (x+y) / (a-b) )!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("a");
    parser.defineVariable("b");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (sin (/ (+ x y) (- a b))))");
}


TEST_CASE("Multiline parentheses") {
    Lexer lx("(10 +\n 2) * 3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (+ 10 2) 3)");
}

TEST_CASE("Multiline implicit multiplication with factorial") {
    Lexer lx("2(x +\n y)!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (! (+ x y)))");
}

TEST_CASE("Multiline function call with factorial") {
    Lexer lx("sin(\n x * y\n)!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (sin (* x y)))");
}

TEST_CASE("Multiline assignment") {
    Lexer lx("a = (\n 1 + (2 * 3)\n - 4\n)");
    Parser parser;
    parser.defineVariable("a");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(= a (- (+ 1 (* 2 3)) 4))");
}

TEST_CASE("Implicit multiplication after factorial") {
    Lexer lx("x! y");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! x) y)");
}

TEST_CASE("Implicit multiplication after parenthesized factorial") {
    Lexer lx("(x)!y");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! x) y)");
}

TEST_CASE("Implicit multiplication with factorial and parentheses") {
    Lexer lx("(x+1)!(y+2)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! (+ x 1)) (+ y 2))");
}

TEST_CASE("Implicit multiplication number and variable factorial") {
    Lexer lx("2x!");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 2 (! x))");
}

TEST_CASE("Factorial of function call with multiplication") {
    Lexer lx("sin(x)! * 2");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! (sin x)) 2)");
}

TEST_CASE("Implicit multiplication with parenthesized factorial") {
    Lexer lx("3(x!)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* 3 (! x))");
}

TEST_CASE("Function call without parentheses on factorial") {
    Lexer lx("sin x!");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sin (! x))");
}

TEST_CASE("Factorial of function call") {
    Lexer lx("log(10)!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (log 10))");
}

TEST_CASE("Function call with factorial argument") {
    Lexer lx("sin(x!)");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sin (! x))");
}

TEST_CASE("Function call without parentheses on factorial number") {
    Lexer lx("sqrt 4!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(sqrt (! 4))");
}

TEST_CASE("Factorial of function call without parentheses") {
    Lexer lx("(sqrt 4)!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! (sqrt 4))");
}

TEST_CASE("Implicit multiplication of function and variable") {
    Lexer lx("sin(x)y");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (sin x) y)");
}

TEST_CASE("Implicit multiplication of parenthesized function and variable") {
    Lexer lx("(sin x)y");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (sin x) y)");
}

TEST_CASE("Complex expression with unary minus and factorials") {
    Lexer lx("-sin(x+y)! ^ -cos(z)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (^ (! (sin (+ x y))) (- (cos z))))");
}

TEST_CASE("Multiple statements") {
    Lexer lx("x = 2\ny = x(x+1)!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() >= 2);
    REQUIRE(toLisp(ast.back()) == "(= y (* x (! (+ x 1))))");
}

#define REQUIRE_PARSER_ERROR(input_string, expected_error_string)   \
    do {                                                            \
        Lexer lx(input_string);                                     \
        if (!lx.getError().empty()){                                \
            REQUIRE(lx.getError() == (expected_error_string));      \
            break;                                                  \
        }                                                           \
        Parser parser;                                              \
        auto ast = parser.parse(lx);                                \
        REQUIRE_FALSE(!ast.empty());                                \
        REQUIRE(parser.getError() == (expected_error_string));      \
} while (0)



TEST_CASE("invalid: empty parentheses") {
    std::string expected_error = "An expression was expected inside parentheses, but none was found.\n"
            "--> at line 1:\n"
            "    ()\n"
            "    ^-- Expected an expression after this parenthesis";
    REQUIRE_PARSER_ERROR("()", expected_error);
}

TEST_CASE("invalid: empty function call") {
    std::string expected_error = "An expression was expected inside parentheses, but none was found.\n"
            "--> at line 1:\n"
            "    sin()\n"
            "       ^-- Expected an expression after this parenthesis";
    REQUIRE_PARSER_ERROR("sin()", expected_error);
}

TEST_CASE("invalid: missing rhs for infix") {
    std::string expected_error = "Infix operator '+' is missing a right-hand side expression.\n"
            "--> at line 1:\n"
            "    5 +\n"
            "      ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("5 +", expected_error);
}

TEST_CASE("invalid: missing rhs for infix in parens") {
    std::string expected_error = "Infix operator '+' is missing a right-hand side expression.\n"
            "--> at line 1:\n"
            "    (5 + )\n"
            "       ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("(5 + )", expected_error);
}

TEST_CASE("invalid: expression starts with infix") {
    std::string expected_error = "Unexpected token '*'\n"
            "--> at line 1:\n"
            "    * 5\n"
            "    ^-- This should not be here";
    REQUIRE_PARSER_ERROR("* 5", expected_error);
}

TEST_CASE("invalid: prefix factorial") {
    std::string expected_error = "Unexpected token '!'\n"
            "--> at line 1:\n"
            "    !5\n"
            "    ^-- This should not be here";
    REQUIRE_PARSER_ERROR("!5", expected_error);
}

TEST_CASE("invalid: missing closing paren") {
    std::string expected_error = "Missing closing ')' for parenthesis that started on line 1.\n"
            "--> at line 1:\n"
            "    (5 + 2\n"
            "    ^-- This parenthesis was never closed.\n\n"
            "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(5 + 2", expected_error);
}

TEST_CASE("invalid: unexpected closing paren") {
    std::string expected_error = "Unexpected token ')'\n"
            "--> at line 1:\n"
            "    5 + 2)\n"
            "         ^-- This should not be here";
    REQUIRE_PARSER_ERROR("5 + 2)", expected_error);
}

TEST_CASE("invalid: function with no arg at eof") {
    std::string expected_error = "Expected an argument for function 'sin' but reached the end of the input.\n"
            "--> at line 1:\n"
            "    sin\n"
            "    ^-- Here";
    REQUIRE_PARSER_ERROR("sin", expected_error);
}

TEST_CASE("invalid: function after number with no arg at eof") {
    std::string expected_error = "Expected an argument for function 'sin' but reached the end of the input.\n"
            "--> at line 1:\n"
            "    5 sin\n"
            "      ^-- Here";
    REQUIRE_PARSER_ERROR("5 sin", expected_error);
}

TEST_CASE("invalid: missing operator") {
    std::string expected_error = "Missing operator between '5' and '2'.\n"
            "--> at line 1:\n"
            "    5 2\n"
            "      ^-- An operator was expected here.";
    REQUIRE_PARSER_ERROR("5 2", expected_error);
}

TEST_CASE("invalid: missing rhs for assignment") {
    std::string expected_error = "Assignment operator '=' is missing a right-hand side expression.\n"
            "--> at line 1:\n"
            "    x =\n"
            "      ^-- An expression was expected to follow the assignment.";
    REQUIRE_PARSER_ERROR("x =", expected_error);
}

TEST_CASE("invalid: assignment to non-lvalue number") {
    std::string expected_error = "Invalid target for assignment.\n"
            "--> at line 1:\n"
            "    5 = x\n"
            "      ^-- Cannot assign to this expression.";
    REQUIRE_PARSER_ERROR("5 = x", expected_error);
}

TEST_CASE("invalid: assignment to non-lvalue expression") {
    std::string expected_error = "Invalid target for assignment.\n"
            "--> at line 1:\n"
            "    (x+1) = y\n"
            "          ^-- Cannot assign to this expression.";
    REQUIRE_PARSER_ERROR("(x+1) = y", expected_error);
}

TEST_CASE("invalid: infix followed by infix") {
    std::string expected_error = "Infix operator '+' is missing a right-hand side expression.\n"
            "--> at line 1:\n"
            "    5 + * 3\n"
            "      ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("5 + * 3", expected_error);
}

TEST_CASE("invalid: multiline without parens") {
    std::string expected_error = "Infix operator '+' is missing a right-hand side expression.\n"
            "--> at line 1:\n"
            "    5 +\n"
            "      ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("5 +\n 2", expected_error);
}

TEST_CASE("invalid: undefined variable in assignment") {
    std::string expected_error = "Use of undefined variable 'x'.\n"
            "--> at line 1:\n"
            "    y = x + z\n"
            "        ^-- This variable has not been defined";
    REQUIRE_PARSER_ERROR("y = x + z", expected_error);
}

TEST_CASE("invalid: undefined variable in function call") {
    std::string expected_error = "Use of undefined variable 'y'.\n"
            "--> at line 1:\n"
            "    sin y\n"
            "        ^-- This variable has not been defined";
    REQUIRE_PARSER_ERROR("sin y", expected_error);
}

TEST_CASE("invalid: prefix operator missing rhs") {
    std::string expected_error = "Prefix operator '+' is missing an expression on its right-hand side.\n"
            "--> at line 1:\n"
            "    +\n"
            "    ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("+", expected_error);
}

TEST_CASE("invalid: missing closing paren in implicit multiplication") {
    std::string expected_error = "Missing closing ')' for parenthesis that started on line 1.\n"
            "--> at line 1:\n"
            "    5(2+3\n"
            "     ^-- This parenthesis was never closed.\n\n"
            "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("5(2+3", expected_error);
}