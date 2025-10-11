# Math2.0

Math2.0 is an advanced C++ project that parses mathematical expressions given as text, understands them, and converts them into a structural tree (AST — Abstract Syntax Tree). The project supports many fundamental features a modern parser should have, such as operator precedence, parentheses, functions, variable assignments, and detailed error handling.

## Key Features

- **Operator Precedence and Associativity:** Processes standard operators like `+`, `-`, `*`, `/`, `^` (exponentiation) according to mathematical rules.
- **Comprehensive Operator Support:**
    - **Prefix Operators:** Unary operators such as `+5`, `-x`.
    - **Postfix Operators:** Suffix operators such as `x!` (factorial).
- **Variable Support:** You can make assignments like `x = 10` and use these variables in expressions such as `2 * x`. Predefined constants like `e` and `pi` are available.
- **Functions:** Supports built‑in mathematical functions such as `sin(x)`, `cos(y)`, `sqrt(9)`.
- **Implicit Multiplication:** Automatically interprets forms like `2x`, `2(x+y)`, or `(x+1)(y+2)` as multiplication.
- **Parentheses Support:** Fully supports nested parentheses to override default precedence.
- **Multi‑line Expressions:** Expressions inside parentheses can span multiple lines.
- **Detailed Error Reporting:** Produces clear, descriptive error messages that show where and why the error occurred.

## Usage Examples

Below are examples of how Math2.0 converts various mathematical expressions into a Lisp‑like AST format.

| Mathematical Expression   | Generated AST (Lisp Format)     |
|---------------------------|---------------------------------|
| `2 + 3 * 4`               | `(+ 2 (* 3 4))`                 |
| `(2 + 3) * 4`             | `(* (+ 2 3) 4)`                 |
| `2x^3`                    | `(* 2 (^ x 3))`                 |
| `-sin(x+1)!`              | `(- (! (sin (+ x 1))))`         |
| `x = 10 + 5` <br> `2 * x` | `(= x (+ 10 5))` <br> `(* 2 x)` |
| `(a ^ b) ^ c`             | `(^ (^ a b) c)`                 |
| `sin (x + y)`             | `(sin (+ x y))`                 |

## Error Handling

When the parser encounters an invalid expression, it does not terminate the program; instead, it provides an explanatory message pinpointing the source of the problem.

**Example Invalid Input:** `sin()`

**Produced Error Message:**
```
An expression was expected inside parentheses, but none was found.
--> at line 1:
    sin()
       ^-- Expected an expression after this parenthesis
```

The project can be built using a standard CMake setup.

```bash
# 1. Clone the repository
git clone https://github.com/erhantrk/Math2.0
cd math2.0

# 2. Create a build directory and enter it
mkdir build
cd build

# 3. Generate build files with CMake
cmake ..

# 4. Build the project
make

# 5. Run the main program
./Math2.0
```

You can modify the sample expression in `main.cpp` to run your own tests.

## Roadmap & Future Improvements

- **Evaluation:** Add an `Evaluator` module that executes the AST to compute the result of the mathematical expression.
- **More Functions and Constants:** Expand the set of trigonometric and logarithmic functions.
- **Interactive Console (REPL):** Provide a command‑line interface where users can enter expressions interactively.
- **User‑Defined Functions:** Allow users to define their own functions such as `f(x) = x^2`.
