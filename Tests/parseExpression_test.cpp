//
// Created by Erhan Türker on 10/8/25.
//

#include "catch_amalgamated.hpp"
#include <string>
#include <vector>

#include "../Parser/inc/Parser.hpp"
#include "../Lexer/inc/Lexer.hpp"
#include "../Util/inc/ASTPrint.hpp"



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
    REQUIRE(toLisp(ast[0]) == "4");
}

TEST_CASE("Simple multiplication and division") {
    Lexer lx("10 / 5 * 2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "4");
}

TEST_CASE("Exponent with multiplication") {
    Lexer lx("2 * 3^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "18");
}

TEST_CASE("Exponent with parentheses") {
    Lexer lx("(2 * 3)^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "36");
}

TEST_CASE("Unary minus with addition") {
    Lexer lx("-5 + -3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "-8");
}

TEST_CASE("Multiplication with unary minus") {
    Lexer lx("5 * -3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "-15");
}

TEST_CASE("Double negative") {
    Lexer lx("5--3");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "8");
}

TEST_CASE("Chained unary minus") {
    Lexer lx("---x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- x)");
}

TEST_CASE("Unary minus on a function call without parentheses") {
    Lexer lx("-sin x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(- (sin x))");
}

TEST_CASE("Unary plus with factorial") {
    Lexer lx("+x!");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(! x)");
}

TEST_CASE("Double factorial") {
    Lexer lx("3!!");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "720");
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

TEST_CASE("Implicit multiplication with a unary minus on a parenthesized expression") {
    Lexer lx("-(x+y)(z-1)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (- (- x) y) (- z 1))");
}

TEST_CASE("Nested implicit multiplication") {
    Lexer lx("2(x+1)3(y+2)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (* 6 (+ 1 x)) (+ 2 y))");
}

TEST_CASE("Implicit multiplication of parenthesized expressions across newlines") {
    Lexer lx("(x+1)\n(y+2)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast[0]) == "(+ 1 x)");
    REQUIRE(toLisp(ast[1]) == "(+ 2 y)");
}

TEST_CASE("Implicit multiplication after function call across newline") {
    Lexer lx("sin(x)\n(y+1)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast[0]) == "(sin x)");
    REQUIRE(toLisp(ast[1]) == "(+ 1 y)");
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
    REQUIRE(toLisp(ast[0]) == "(* (+ 1 x) (+ 2 y))");
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

TEST_CASE("Assignment to a variable that is then used in another assignment") {
    Lexer lx("x = 5\ny = x^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast[0]) == "(= x 5)");
    REQUIRE(toLisp(ast[1]) == "(= y (^ x 2))");
}

TEST_CASE("Complex polynomial") {
    Lexer lx("3x^2 + 2y - 1");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(+ (+ -1 (* 3 (^ x 2))) (* 2 y))");
}

TEST_CASE("Complex expression with unary minus and factorial") {
    Lexer lx("-(x+y)*z!");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (! z) (- (- x) y))"); /* For now this is good, after adding factorization this will be better */
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

TEST_CASE("Number notations") {
    Lexer lx("123 + 45.67 - 6.022E+23 * 9.1e-31");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "168.669999");
}

TEST_CASE("Predefined constants in expressions") {
    Lexer lx("2 * pi * r");
    Parser parser;
    parser.defineVariable("r");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (* 2 pi) r)");
}

TEST_CASE("Chained division") {
    Lexer lx("a / b / c");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(/ a (* b c))");
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
    REQUIRE(toLisp(ast[0]) == "2");
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
    REQUIRE(toLisp(ast[0]) == "(+ (+ (* b c) a) d)");
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
    REQUIRE(toLisp(ast[0]) == "(* (* (+ b c) a) d)");
}

TEST_CASE("Implicit multiplication with nested parentheses") {
    Lexer lx("3(4(5))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "60");
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
    REQUIRE(toLisp(ast[0]) == "-4");
}

TEST_CASE("Parentheses with unary minus and exponent") {
    Lexer lx("(-2)^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "4");
}

TEST_CASE("Complex mixed arithmetic") {
    Lexer lx("1 + 2 * 3 / 4 - 5");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "-2.5");
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
    REQUIRE(toLisp(ast[0]) == "(* (cos y) (sin x))");
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
    REQUIRE(toLisp(ast[0]) == "(- x)");
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
    REQUIRE(toLisp(ast[0]) == "(/ 1 (+ 1 x))");
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

TEST_CASE("Parenthesized expression vs right-associative exponentiation") {
    Lexer lx("(x^y)^z");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(^ x (* y z))");
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
    REQUIRE(toLisp(ast[0]) == "36");
}

TEST_CASE("Deeply nested parentheses with mixed operators and newlines") {
    Lexer lx("a * (b + \n (c - d) / \n (e + f))");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    parser.defineVariable("d");
    parser.defineVariable("e");
    parser.defineVariable("f");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (+ (/ (- c d) (+ e f)) b) a)");
}

TEST_CASE("Multiline implicit multiplication with factorial") {
    Lexer lx("(2(x +\n y)\n (3x)!)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(* (* 2 (! (* 3 x))) (+ x y))");
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
    REQUIRE(toLisp(ast[0]) == "(= a 3)");
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
    REQUIRE(toLisp(ast[0]) == "(* (! (+ 1 x)) (+ 2 y))");
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
    REQUIRE(toLisp(ast[0]) == "(* 2 (! (sin x)))");
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
    REQUIRE(toLisp(ast[0]) == "(sqrt 24)");
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
    REQUIRE(toLisp(ast.back()) == "(= y (* (! (+ 1 x)) x))");
}

TEST_CASE("Simple function definition") {
    Lexer lx("f(x)=x^2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 1);
    REQUIRE(toLisp(ast.back()) == "(f (^ x 2))");
}

TEST_CASE("Multi parameter function definition") {
    Lexer lx("f(x,y,z)=x*y*z");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 1);
    REQUIRE(toLisp(ast.back()) == "(f (* (* x y) z))");
}

TEST_CASE("Simple function call") {
    Lexer lx("f(x)=x\nf(2)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 2)");
}

TEST_CASE("Simple function call wo parentheses") {
    Lexer lx("f(x)=x\nf 2");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 2)");
}

TEST_CASE("Multi parameter function call") {
    Lexer lx("f(x,y,z)=x*y*z\nf(1,2,3)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 1 2 3)");
}

TEST_CASE("Multi parameter function call with newline") {
    Lexer lx("f(x,y,z)=x*y*z\nf(1,\n"
             "2,\n"
             "3)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 1 2 3)");
}

TEST_CASE("Multi parameter function call with newline complex") {
    Lexer lx("f(x,y,z)=x*y*z\nf(1\n"
             ",2,\n"
             "3)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 1 2 3)");
}

TEST_CASE("Function call with parentheses") {
    Lexer lx("f(x, y) = x + y\n f((3+5), (2^4))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f 8 16)");
}

TEST_CASE("Function call with function parameter") {
    Lexer lx("f(x, y) = x + y\n g(x, y) = x * y\n f(g(1,2), g(2,3))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 3);
    REQUIRE(toLisp(ast.back()) == "(f (g 1 2) (g 2 3))");
}

TEST_CASE("Function calling on itself multiple times") {
    Lexer lx("f(x, y) = x + y\n f(f(f(1, 2), 3), f(4, 5))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f (f (f 1 2) 3) (f 4 5))");
}

TEST_CASE("Function calling inside function with newlines") {
    Lexer lx("f(x, y) = x + y\n"
             "f(f(1,\n"
             "2),\n"
             "f(3,\n"
             "f(4,\n"
             "5)))");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f (f 1 2) (f 3 (f 4 5)))");
}

TEST_CASE("Function with multiple arguments and expressions") {
    Lexer lx("atan2(y*2, x/3)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    auto ast = parser.parse(lx);
    REQUIRE(!ast.empty());
    REQUIRE(toLisp(ast[0]) == "(atan2 (* 2 y) (/ x 3))");
}

TEST_CASE("Nested function calls without parentheses") {
    Lexer lx( "f(a)=a\n"
                      "g(b)=b\n"
                      "h(c)=c\n"
                      "f g h x");
    Parser parser;
    parser.defineVariable("x");
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 4);
    REQUIRE(toLisp(ast.back()) == "(f (g (h x)))");
}

TEST_CASE("Mixed style nested function calls") {
    Lexer lx("f(a,b)=a+b\n"
                      "g(c)=c\n"
                      "h(d)=d\n"
                      "f(g x, h(y))");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 4);
    REQUIRE(toLisp(ast.back()) == "(f (g x) (h y))");
}

TEST_CASE("Deeply nested mixed-style calls") {
    Lexer lx("f(a,b)=a\n"
             "g(a)=a\n"
             "h(a)=a\n"
             "k(a)=a\n"
             "m(a)=a\n"
             "n(a)=a\n"
             "f(g(h k l), m n o)");
    Parser parser;
    parser.defineVariable("l");
    parser.defineVariable("o");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 7);
    REQUIRE(toLisp(ast.back()) == "(f (g (h (k l))) (m (n o)))");
}

TEST_CASE("Function argument is an implicit multiplication") {
    Lexer lx("f(a,b)=a+b\n f(x^y, 2z)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");
    parser.defineVariable("z");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f (^ x y) (* 2 z))");
}

TEST_CASE("Multi-argument function with complex expressions as arguments") {
    Lexer lx("dist(x1, y1, x2, y2) = sqrt((x2-x1)^2 + (y2-y1)^2)\n"
             "dist(a+1, b*2, c/3, d-4)");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");
    parser.defineVariable("c");
    parser.defineVariable("d");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(dist (+ 1 a) (* 2 b) (/ c 3) (- d 4))");
}

TEST_CASE("Multi-argument function with nested function calls as arguments") {
    Lexer lx("f(a,b)=a+b\n g(c,d)=c*d\n h(x)=x^2\n f(g(1,2), h(3))");
    Parser parser;

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 4);
    REQUIRE(toLisp(ast.back()) == "(f (g 1 2) (h 3))");
}

TEST_CASE("Implicit multiplication with multi-argument function call") {
    Lexer lx("f(x,y)=x+y\n (2f(a, b)! * 3!)!");
    Parser parser;
    parser.defineVariable("a");
    parser.defineVariable("b");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(! (* 12 (! (f a b))))");
}

TEST_CASE("Function with many arguments spread across multiple lines") {
    Lexer lx("avg(a,b,c,d,e) = (a+b+c+d+e)/5\n"
             " avg(10, \n"
             " 20, \n"
             " 30, \n\n"
             " 40, \n\n"
             " 50)");
    Parser parser;

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(avg 10 20 30 40 50)");
}

TEST_CASE("Parenthesized implicit multiplication as a function argument") {
    Lexer lx("f(a,b)=a+b\n f((x+1)(x-1), y)");
    Parser parser;
    parser.defineVariable("x");
    parser.defineVariable("y");

    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 2);
    REQUIRE(toLisp(ast.back()) == "(f (* (+ 1 x) (- x 1)) y)");
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

TEST_CASE("invalid: Chained function calls without parentheses over newlines") {
    std::string expected_error = "Expected an argument for function 'sin' but reached the end of the input.\n"
            "--> at line 1:\n"
            "    sin \n"
            "        ^-- Here";
    REQUIRE_PARSER_ERROR("sin \n cos \n tan 1", expected_error);
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

TEST_CASE("invalid: mismatched parentheses in nested expression") {
    std::string expected_error = "Missing closing ')' for parenthesis that started on line 1.\n"
                                 "--> at line 1:\n"
                                 "    (2 * (3 + 4)\n"
                                 "    ^-- This parenthesis was never closed.\n\n"
                                 "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(2 * (3 + 4)", expected_error);
}

TEST_CASE("invalid: function call with too many arguments") {
    Lexer lx("f(x, y) = x+y\nf(1, 2, 3)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(parser.getError() ==   "Function call with too many arguments.\n"
                                    "--> at line 2:\n"
                                    "    f(1, 2, 3)\n"
                                    "    ^-- 'f' expects 2 arguments.");
    REQUIRE(ast.empty());
}

TEST_CASE("invalid: function call without sufficient arguments") {
    Lexer lx("f(x, y) = x+y\nf(1)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(parser.getError() ==   "Function call without sufficient arguments.\n"
                                    "--> at line 2:\n"
                                    "    f(1)\n"
                                    "    ^-- 'f' expects 2 arguments.");
    REQUIRE(ast.empty());
}

TEST_CASE("invalid: comma outside of function call arguments") {
    std::string expected_error = "Unexpected token ','\n"
                                 "--> at line 1:\n"
                                 "    1, 2\n"
                                 "     ^-- This should not be here";
    REQUIRE_PARSER_ERROR("1, 2", expected_error);
}

TEST_CASE("invalid: Function definition over multiple lines") {
    Lexer lx("f(x,\n"
                        "  y) = (x \n"
                        " + y)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(ast.size() == 1);
    REQUIRE(toLisp(ast[0]) == "(f (+ x y))");
}

TEST_CASE("invalid: comma in function definition parameters") {
    std::string expected_error = "Unexpected token ','\n"
                                 "--> at line 1:\n"
                                 "    f(x,) = x\n"
                                 "       ^-- This should not be here";
    REQUIRE_PARSER_ERROR("f(x,) = x", expected_error);
}

TEST_CASE("invalid: multiline missing closing paren") {
    std::string expected_error = "Missing closing ')' for parenthesis that started on line 1.\n"
                                 "--> at line 1:\n"
                                 "    (1 + \n"
                                 "    ^-- This parenthesis was never closed.\n\n"
                                 "Instead, the input ended before the parenthesis was closed.";
    REQUIRE_PARSER_ERROR("(1 + \n 2", expected_error);
}

TEST_CASE("invalid: function call with an empty argument") {
    Lexer lx("f(x,y,z)=x+y+z\nf(1,,3)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(parser.getError() ==   "An expression was expected for an argument, but none was found.\n"
                                    "--> at line 2:\n"
                                    "    f(1,,3)\n"
                                    "        ^-- Expected an argument here");
    REQUIRE(ast.empty());
}

TEST_CASE("invalid: function call with a trailing comma") {
    Lexer lx("f(x,y)=x+y\nf(1, 2,)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(parser.getError() ==   "Function call with too many arguments.\n"
                                    "--> at line 2:\n"
                                    "    f(1, 2,)\n"
                                    "    ^-- 'f' expects 2 arguments.");
    REQUIRE(ast.empty());
}

TEST_CASE("invalid: missing comma between arguments") { /* TODO can do better error message but not a bug */
    Lexer lx("f(x,y)=x+y\nf(1 2)");
    Parser parser;
    auto ast = parser.parse(lx);
    REQUIRE(parser.getError() ==   "Function call without sufficient arguments.\n"
                                    "--> at line 2:\n"
                                    "    f(1 2)\n"
                                    "    ^-- 'f' expects 2 arguments.");
    REQUIRE(ast.empty());
}

TEST_CASE("invalid: infix operator followed by prefix without rhs") {
    std::string expected_error = "Prefix operator '-' is missing an expression on its right-hand side.\n"
                                 "--> at line 1:\n"
                                 "    5 + -\n"
                                 "        ^-- An expression was expected to follow this operator";
    REQUIRE_PARSER_ERROR("5 + -", expected_error);
}

TEST_CASE("invalid: assignment to a unary expression") {
    std::string expected_error = "Invalid target for assignment.\n"
                                 "--> at line 1:\n"
                                 "    -x = 5\n"
                                 "       ^-- Cannot assign to this expression.";
    REQUIRE_PARSER_ERROR("-x = 5", expected_error);
}

TEST_CASE("invalid: Multi arg function without paren") {
    std::string expected_error = "Multi argument function called without parentheses.\n"
                                 "--> at line 1:\n"
                                 "    atan2 1, 2\n"
                                 "    ^-- 'atan2' expects 2 arguments. Cannot call without parentheses.";
    REQUIRE_PARSER_ERROR("atan2 1, 2", expected_error);
}