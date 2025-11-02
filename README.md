# Math2.0: A Symbolic Math Engine

Math2.0 is a robust, modular C++ mathematical expression engine. It parses, evaluates, and symbolically manipulates complex mathematical expressions, correctly handling operator precedence, variables, and user-defined functions.

The project is built around a clean AST (Abstract Syntax Tree) and features a **hybrid evaluation system**: it first attempts numerical evaluation, and if it fails (e.g., due to undefined variables), it falls back to symbolic manipulation.

---

## Core Components

The engine is divided into five main modules:

* **Lexer:** A token generator that converts raw string input into a stream of tokens (Numbers, Symbols, Words, etc.).
* **Parser:** A recursive-descent parser that consumes tokens and builds an Abstract Syntax Tree (AST), correctly handling operator precedence, associativity, prefix/postfix operators, and implicit multiplication.
* **Evaluator:** A tree-walking evaluator that recursively calculates the numerical result of an AST.
* **Simplifier:** An algebraic simplifier that applies rules to an AST to reduce complexity (e.g., constant folding, `x + x -> 2*x`, `x^0 -> 1`).
* **SymbolicEvaluator:** Expands user-defined functions and variables within the AST, creating a new, expanded tree for further simplification or evaluation.

---

## Key Features

* **Full Operator Support:** Correctly handles precedence and associativity for `+`, `-`, `*`, `/`, `^` (power), and `!` (factorial).
* **Prefix/Postfix:** Handles unary operators like `-x` and postfix operators like `x!`.
* **Implicit Multiplication:** Automatically detects and inserts multiplication (e.g., `2x`, `2(x+y)`, `(x+1)(y+2)`).
* **Variables:** Supports variable assignment (`x = 10`) and built-in constants (`pi`, `e`).
* **User-Defined Functions:** Define and call your own functions (e.g., `f(x) = x^2`, `g(x, y) = f(x) + f(y)`).
* **Hybrid Evaluation:** The engine intelligently switches between numerical and symbolic processing.
* **Detailed Error Reporting:** Provides clear, user-friendly error messages that pinpoint the exact location and nature of any syntax error.

---

## Examples

The system can process a wide variety of inputs.

| Mathematical Expression | Generated AST (Lisp Format) |
|---|---|
| `2 + 3 * 4` | `14` |
| `(2 + 3) * 4` | `20` |
| `2x^3` | `(* 2 (^ x 3))` |
| `-sin(x+1)!` | `(- (! (sin (+ x 1))))` |
| `f(x) = x^2` | `(f (^ x 2))` |
| `g(y) = f(y) + 1` | `(g (+ (f y) 1))` |

### Error Handling

When the parser encounters an invalid expression, it provides a detailed message:

**Input:** `sin()`

**Error:**
```
An expression was expected inside parentheses, but none was found.
--> at line 1:
    sin()
       ^-- Expected an expression after this parenthesis
```
---

## Build Instructions

The project uses a standard CMake setup.

```bash
# 1. Clone the repository
git clone [https://github.com/erhantrk/Math2.0](https://github.com/erhantrk/Math2.0)
cd math2.0

# 2. Create a build directory and enter it
mkdir build
cd build

# 3. Generate build files with CMake
cmake ..

# 4. Build the project
make

# 5. Run the main program or tests
./Math2.0
```

You can modify the sample expression in `main.cpp` to run your own tests.

## Roadmap & Future Improvements
- **Interactive Console (REPL):** Provide a command‑line interface where users can enter expressions interactively.
