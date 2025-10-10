//
// Created by Erhan Türker on 10/8/25.
//

#include "catch_amalgamated.hpp"
#include <memory>
#include <string>
#include <vector>
#include <sstream>

#include "../Parser.hpp"
#include "../Lexer.hpp"

static void toLispImpl(const Node* n, std::ostringstream& out) {
    if (!n) { out << "<null>"; return; }

    switch (n->type) {
        case Node::Type::Number:
            out << n->value;
            break;
        case Node::Type::Variable:
            out << n->value;
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

static std::string toLisp(const std::unique_ptr<Node>& n) {
    std::ostringstream out;
    toLispImpl(n.get(), out);
    return out.str();
}

TEST_CASE("number literal only") {
    Lexer lx("42");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "42");
}

TEST_CASE("variable only") {
    Lexer lx("x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "x");
}

TEST_CASE("simple addition") {
    Lexer lx("2 + 3");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ 2 3)");
}

TEST_CASE("multiplication binds tighter than addition") {
    Lexer lx("2 + 3 * 4");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ 2 (* 3 4))");
}

TEST_CASE("right-associative power") {
    Lexer lx("a ^ b ^ c");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ a (^ b c))");
}

TEST_CASE("postfix factorial") {
    Lexer lx("x!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! x)");
}

TEST_CASE("chained postfix factorial") {
    Lexer lx("x!!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! (! x))");
}

TEST_CASE("predefined function as prefix: sin x") {
    Lexer lx("sin x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin x)");
}

TEST_CASE("nested predefined functions: cos sin x") {
    Lexer lx("cos sin x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(cos (sin x))");
}

TEST_CASE("function + binary op: sin x + y") {
    Lexer lx("sin x + y");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ (sin x) y)");
}

TEST_CASE("Prefix function and postfix") {
    Lexer lx("sin x!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (! x))");
}

TEST_CASE("starts with symbol +") {
    Lexer lx("+ 2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ 2)");
}

TEST_CASE("Number and var") {
    Lexer lx("2x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 x)");
}


TEST_CASE("Number and prefix expr") {
    Lexer lx("2sin x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 (sin x))");
}

TEST_CASE("Decimal number only") {
    Lexer lx("2.3123");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "2.3123");
}

TEST_CASE("Postfix with number") {
    Lexer lx("5!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! 5)");
}

TEST_CASE("Empty") {
    Lexer lx("");
    auto ast = Parser::parse(lx);
    REQUIRE_FALSE(ast);
}

TEST_CASE("Auto mult with power") {
    Lexer lx("2x^3");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 (^ x 3))");
}

TEST_CASE("Power and postfix") {
    Lexer lx("2x^3! ^ sin 2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 (^ x (^ (! 3) (sin 2))))");
}

TEST_CASE("parenthesized addition") {
    Lexer lx("(2 + 3)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ 2 3)");
}

TEST_CASE("parentheses override precedence: (2 + 3) * 4") {
    Lexer lx("(2 + 3) * 4");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (+ 2 3) 4)");
}

TEST_CASE("implicit multiplication with parentheses: 2(x + y)") {
    Lexer lx("2(x + y)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 (+ x y))");
}

TEST_CASE("implicit multiplication with parentheses: (z ^ t)(x + y)") {
    Lexer lx("(z ^ t)(x + y)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (^ z t) (+ x y))");
}

TEST_CASE("function with parenthesized arg: sin (x + y)") {
    Lexer lx("sin (x + y)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (+ x y))");
}

TEST_CASE("redundant parentheses collapse: (((x)))") {
    Lexer lx("(((x)))");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "x");
}

TEST_CASE("power with parenthesized base: (2x)^3") {
    Lexer lx("(2x)^3");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ (* 2 x) 3)");
}

TEST_CASE("power with parenthesized exponent: x^(y + 1)") {
    Lexer lx("x^(y + 1)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ x (+ y 1))");
}

TEST_CASE("parentheses and right-assoc power: (a ^ b) ^ c") {
    Lexer lx("(a ^ b) ^ c");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ (^ a b) c)");
}

TEST_CASE("factorial on parenthesized group: (x + 1)!") {
    Lexer lx("(x + 1)!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! (+ x 1))");
}

TEST_CASE("power and postfix with parentheses: (x!)^2") {
    Lexer lx("(x!)^2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ (! x) 2)");
}

TEST_CASE("power and prefix function: x^sin 2") {
    Lexer lx("x^sin 2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ x (sin 2))");
}

TEST_CASE("nested with funcs, postfix, and parens: cos (sin (x + 1)!)") {
    Lexer lx("cos (sin (x + 1)!)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(cos (! (sin (+ x 1))))");
}

TEST_CASE("simple unary minus") {
    Lexer lx("-5");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- 5)");
}

TEST_CASE("unary minus binds tighter than addition") {
    Lexer lx("-a + b");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(+ (- a) b)");
}

TEST_CASE("unary minus binds tighter than multiplication") {
    Lexer lx("-a * b");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (- a) b)");
}

TEST_CASE("chained unary operators") {
    Lexer lx("-+-x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- (+ (- x)))");
}

TEST_CASE("power binds tighter than unary minus") {
    Lexer lx("-x^2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- (^ x 2))");
}

TEST_CASE("parentheses override unary minus precedence") {
    Lexer lx("(-x)^2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ (- x) 2)");
}

TEST_CASE("postfix binds tighter than unary minus") {
    Lexer lx("-x!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- (! x))");
}

TEST_CASE("unary minus with prefix function") {
    Lexer lx("-sin(x)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- (sin x))");
}

TEST_CASE("prefix function and operators") {
    Lexer lx("sin 3^3*2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (* (^ 3 3) 2))");
}

TEST_CASE("prefix function and operators with parentheses") {
    Lexer lx("sin (3^3)*2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (sin (^ 3 3)) 2)");
}

TEST_CASE("implicit multiplication between two variables") {
    Lexer lx("x y");
    auto ast = Parser::parse(lx);
    REQUIRE_FALSE(ast);
}

TEST_CASE("implicit multiplication with postfix on parenthesis") {
    Lexer lx("2(x+y)!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 2 (! (+ x y)))");
}

TEST_CASE("Prefix fun with prefix symbol") {
    Lexer lx("sin - 2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (- 2))");
}

TEST_CASE("implicit multiplication (x)2") {
    Lexer lx("(x)2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* x 2)");
}

TEST_CASE("implicit multiplication (x^2)sin x") {
    Lexer lx("(x^2)sin x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (^ x 2) (sin x))");
}

TEST_CASE("implicit multiplication (x^2)!!x") {
    Lexer lx("(x^2)!!x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (! (! (^ x 2))) x)");
}


TEST_CASE("ultimate stress test") {
    Lexer lx("-sin(x+1)! ^ -2(y!)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (- (^ (! (sin (+ x 1))) (- 2))) (! y))");
}

TEST_CASE("implicit multiplication: variable and parenthesis") {
    Lexer lx("x(y+z)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* x (+ y z))");
}

TEST_CASE("implicit multiplication: parenthesis and variable") {
    Lexer lx("(x+y)z");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (+ x y) z)");
}

TEST_CASE("implicit multiplication: postfixed group and parenthesis") {
    Lexer lx("(x+y)!(a-b)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* (! (+ x y)) (- a b))");
}

TEST_CASE("unary vs binary minus: subtraction of a negative") {
    Lexer lx("a - -b");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(- a (- b))");
}

TEST_CASE("unary vs binary minus: multiplication by a negative") {
    Lexer lx("a * -b");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* a (- b))");
}

TEST_CASE("function argument is an implicit multiplication") {
    Lexer lx("sin 2x");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (* 2 x))");
}

TEST_CASE("postfix on a parenthesized function argument") {
    Lexer lx("sin (2x)!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! (sin (* 2 x)))");
}

TEST_CASE("postfix on a function call itself") {
    Lexer lx("sin(x)!");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(! (sin x))");
}

TEST_CASE("deeply nested mixed operators") {
    Lexer lx("((a*2)! + -b)^-c");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(^ (+ (! (* a 2)) (- b)) (- c))");
}

TEST_CASE("invalid: infix operator as prefix") {
    Lexer lx("* 5");
    auto ast = Parser::parse(lx);
    REQUIRE_FALSE(ast);
}

TEST_CASE("invalid: postfix operator as prefix") {
    Lexer lx("!5");
    auto ast = Parser::parse(lx);
    REQUIRE_FALSE(ast);
}

TEST_CASE("invalid: number format with multiple decimals") {
    Lexer lx("3.14.15");
    auto ast = Parser::parse(lx);
    REQUIRE_FALSE(ast);
}

TEST_CASE("ambiguity: explicitly post-fixing a function argument") {
    Lexer lx("sin((2x)!)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (! (* 2 x)))");
}

TEST_CASE("multiplication and unary operator") {
    Lexer lx("3 * - 4");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(* 3 (- 4))");
}

#define REQUIRE_PARSER_ERROR(input_string, expected_error_string)   \
    do {                                                            \
        Parser::clearError();                                       \
        Lexer lx(input_string);                                     \
        auto ast = Parser::parse(lx);                               \
        REQUIRE_FALSE(ast);                                         \
        REQUIRE(Parser::getError() == (expected_error_string));     \
    } while (0)


TEST_CASE("invalid: empty parentheses") {
    std::string expected_error = "Parse Error: An expression was expected inside parentheses, but none was found.\n"
                                 "--> at line 1:\n"
                                 "    ()\n"
                                 "    ^-- Expected an expression after this parenthesis";
    REQUIRE_PARSER_ERROR("()", expected_error);
}

TEST_CASE("invalid: mismatched parentheses") {
    std::string expected_error = "Parse Error: Missing closing ')' for parenthesis that started on line 1.\n"
                                 "--> at line 1:\n"
                                 "    (2 + 3\n"
                                 "    ^-- This parenthesis was never closed.\n\n"
                                 "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(2 + 3", expected_error);
}

TEST_CASE("invalid: operator missing rhs inside parens") {
    std::string expected_error = "Parse Error: Infix operator '+' is missing a right-hand side expression.\n"
                                 "--> at line 1:\n"
                                 "    (2 + )\n"
                                 "       ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("(2 + )", expected_error);
}

TEST_CASE("Complex false example") {
    std::string expected_error = "Parse Error: Expected an argument for function 'cos' but reached the end of the input.\n"
                                 "--> at line 1:\n"
                                 "    2sin x * 5cos\n"
                                 "              ^-- Here";
    REQUIRE_PARSER_ERROR("2sin x * 5cos", expected_error);
}

TEST_CASE("Complex false example 2") {
    std::string expected_error = "Parse Error: Expected an argument for function 'sin', but found '!' instead.\n"
                                 "--> at line 1:\n"
                                 "    2x^3! ^ sin !\n"
                                 "                ^-- Here";
    REQUIRE_PARSER_ERROR("2x^3! ^ sin !", expected_error);
}

TEST_CASE("invalid: double infix operator") {
    std::string expected_error = "Parse Error: Infix operator '+' is missing a right-hand side expression.\n"
                                 "--> at line 1:\n"
                                 "    1 + * 2\n"
                                 "      ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("1 + * 2", expected_error);
}

TEST_CASE("invalid: trailing binary operator") {
    std::string expected_error = "Parse Error: Infix operator '*' is missing a right-hand side expression.\n"
                                 "--> at line 1:\n"
                                 "    1 + 2 *\n"
                                 "          ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("1 + 2 *", expected_error);
}

TEST_CASE("invalid: closing parenthesis without opening") {
    std::string expected_error = "Parse Error: Unexpected token ')'\n"
                                 "--> at line 1:\n"
                                 "    1 + 2)\n"
                                 "         ^-- This should not be here";
    REQUIRE_PARSER_ERROR("1 + 2)", expected_error);
}

TEST_CASE("implicit false multiplication") {
    std::string expected_error = "Parse Error: Missing operator between 'x' and 'sin'.\n"
                                 "--> at line 1:\n"
                                 "    x sin(y)\n"
                                 "      ^-- An operator was expected here.";
    REQUIRE_PARSER_ERROR("x sin(y)", expected_error);
}

TEST_CASE("invalid: function with empty parens") {
    std::string expected_error = "Parse Error: An expression was expected inside parentheses, but none was found.\n"
                                 "--> at line 1:\n"
                                 "    sin()\n"
                                 "       ^-- Expected an expression after this parenthesis";
    REQUIRE_PARSER_ERROR("sin()", expected_error);
}

TEST_CASE("invalid: number next to a number") {
    std::string expected_error = "Parse Error: Missing operator between '3' and '4'.\n"
                                 "--> at line 1:\n"
                                 "    3 4\n"
                                 "      ^-- An operator was expected here.";
    REQUIRE_PARSER_ERROR("3 4", expected_error);
}

TEST_CASE("Error: dangling open parenthesis deep nest") {
    std::string expected_error = "Parse Error: Missing closing ')' for parenthesis that started on line 1.\n"
                                 "--> at line 1:\n"
                                 "    (a + (b * c)\n"
                                 "    ^-- This parenthesis was never closed.\n\n"
                                 "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(a + (b * c)", expected_error);
}

TEST_CASE("Error: operator abuse plus minus") {
    std::string expected_error = "Parse Error: Invalid start of an expression. Cannot begin with token '*'.\n"
                                 "--> at line 1:\n"
                                 "    a + - * b\n"
                                 "          ^-- An expression cannot start here";
    REQUIRE_PARSER_ERROR("a + - * b", expected_error);
}

TEST_CASE("Error: postfix on nothing at start") {
    std::string expected_error = "Parse Error: Unexpected token '!'\n"
                                 "--> at line 1:\n"
                                 "    !a\n"
                                 "    ^-- This should not be here";
    REQUIRE_PARSER_ERROR("!a", expected_error);
}

TEST_CASE("Error: invalid implicit multiplication with function") {
    std::string expected_error = "Parse Error: Missing operator between 'x' and 'sin'.\n"
                                 "--> at line 1:\n"
                                 "    x sin(y)\n"
                                 "      ^-- An operator was expected here.";
    REQUIRE_PARSER_ERROR("x sin(y)", expected_error);
}

TEST_CASE("Error: function call missing argument at end") {
    std::string expected_error = "Parse Error: Expected an argument for function 'cos' but reached the end of the input.\n"
                                 "--> at line 1:\n"
                                 "    a + cos\n"
                                 "        ^-- Here";
    REQUIRE_PARSER_ERROR("a + cos", expected_error);
}

TEST_CASE("Error: unexpected closing parenthesis deep") {
     std::string expected_error = "Parse Error: Unexpected token ')'\n"
                                  "--> at line 1:\n"
                                  "    (a + (b * c)))\n"
                                  "                 ^-- This should not be here";
    REQUIRE_PARSER_ERROR("(a + (b * c)))", expected_error);
}

TEST_CASE("Error: trailing unary minus") {
    std::string expected_error = "Parse Error: Infix operator '-' is missing a right-hand side expression.\n"
                                 "--> at line 1:\n"
                                 "    a - \n"
                                 "      ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("a - ", expected_error);
}

TEST_CASE("Error: operator after open paren") {
    std::string expected_error = "Parse Error: Invalid start of an expression. Cannot begin with token '*'.\n"
                                 "--> at line 1:\n"
                                 "    (* a)\n"
                                 "     ^-- An expression cannot start here";
    REQUIRE_PARSER_ERROR("(* a)", expected_error);
}

TEST_CASE("multiline expression inside parentheses") {
    Lexer lx("a = (100 +\n"
             "     200 +\n"
             "     300)");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (+ (+ 100 200) 300))");
}

TEST_CASE("multiline with nested parentheses") {
    Lexer lx("a = (1 +\n"
             "     (2 * 3)\n"
             "    )");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (+ 1 (* 2 3)))");
}

TEST_CASE("multiline function call") {
    Lexer lx("sin(\n"
             "  x + y\n"
             ")");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(sin (+ x y))");
}

TEST_CASE("multiline expression with dangling operator") {
    Lexer lx("a = 10 +\n"
             "    20");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (+ 10 20))");
}
TEST_CASE("Error: unclosed parenthesis with newline") {
    std::string expected_error = "Parse Error: Missing closing ')' for parenthesis that started on line 1.\n"
                                 "--> at line 1:\n"
                                 "    (a +\n"
                                 "    ^-- This parenthesis was never closed.\n\n"
                                 "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(a +\n"
                         " b", expected_error);
}

TEST_CASE("multiline with dangling multiplication operator") {
    Lexer lx("a = 10 *\n"
             "    2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (* 10 2))");
}

TEST_CASE("multiline with chained operators and precedence") {
    Lexer lx("a = 10 +\n"
             "    20 *\n"
             "    30");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (+ 10 (* 20 30)))");
}

TEST_CASE("multiline with unary operator on new line") {
    Lexer lx("a = 10 * \n"
             "    -2");
    auto ast = Parser::parse(lx);
    REQUIRE(ast);
    REQUIRE(toLisp(ast) == "(= a (* 10 (- 2)))");
}
