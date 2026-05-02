/*
 * expression.h - Expression evaluator interface for Basic80
 *
 * Declares the public functions used to evaluate numeric expressions,
 * string expressions, and boolean conditions found in BASIC source lines.
 */
#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "lexer.h"

/* Forward declaration (full definition is in interpreter.h) */
typedef struct Interpreter Interpreter;

/* ===== CONDITION EVALUATION ===== */

/**
 * Evaluate a boolean condition expression.
 *
 * Evaluates a comparison expression and returns its boolean result.
 * Supported relational operators: =  <  >  <=  >=  <>
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the condition
 * @param pos     Position pointer (advanced past the condition on return)
 * @return 1 if the condition is true, 0 if it is false.
 *
 * Examples:
 *   "10 > 5"    -> 1
 *   "X = 42"    -> 1 if X equals 42, 0 otherwise
 *   "A$ <> B$"  -> 1 if A$ differs from B$
 */
int evaluateCondition(Interpreter *interp, Token *tokens, int *pos);

/* ===== NUMERIC EXPRESSION EVALUATION ===== */

/**
 * Evaluate a complete arithmetic expression (addition and subtraction).
 *
 * Main entry point for numeric expression evaluation.  Respects standard
 * operator precedence.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the expression
 * @param pos     Position pointer (advanced past the expression on return)
 * @return Numeric result of the expression.
 *
 * Examples:
 *   "10 + 5 * 2"      -> 20.0
 *   "(10 + 5) * 2"    -> 30.0
 *   "A + B - C"       -> Computed value from the current variable state
 */
double evaluateExpression(Interpreter *interp, Token *tokens, int *pos);

/**
 * Evaluate a term (multiplication and division).
 *
 * Handles medium-precedence operators (* and /).  Called by
 * evaluateExpression() in the recursive-descent hierarchy.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array
 * @param pos     Position pointer
 * @return Numeric result of the term.
 */
double evaluateTerm(Interpreter *interp, Token *tokens, int *pos);

/**
 * Evaluate a factor (numbers, variables, functions, parenthesized expressions).
 *
 * Handles atomic expression elements:
 *   - Numeric literals  (42, 3.14)
 *   - Variables         (A, X, COUNT)
 *   - Array elements    (A(5), M(2,3))
 *   - Built-in math functions (SIN, COS, SQR, ABS, etc.)
 *   - Custom numeric functions registered via registerCustomNumericFunction()
 *   - Parenthesized sub-expressions
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array
 * @param pos     Position pointer
 * @return Numeric result of the factor.
 */
double evaluateFactor(Interpreter *interp, Token *tokens, int *pos);

/* ===== STRING EXPRESSION EVALUATION ===== */

/**
 * Test whether the token at pos starts a string-valued expression.
 *
 * Inspects the token type and (for identifiers) the variable table to
 * determine whether the expression will produce a string.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array
 * @param pos     Starting position (not modified)
 * @return 1 if the expression is string-typed, 0 otherwise.
 */
int isStringExpression(Interpreter *interp, Token *tokens, int pos);

/**
 * Evaluate a string expression (handles + concatenation).
 *
 * Evaluates an expression that produces a string value.  Supports
 * concatenation with the + operator.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the expression
 * @param pos     Position pointer (advanced on return)
 * @return Heap-allocated result string (caller must free it).
 *
 * Examples:
 *   "A$ + B$"           -> Concatenation of two string variables
 *   "\"Hello\" + \" World\""  -> "Hello World"
 *   "LEFT$(A$, 3)"      -> First 3 characters of A$
 */
char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos);

/**
 * Evaluate a string primary element.
 *
 * Handles the atomic building blocks of a string expression:
 *   - String literals  ("Hello")
 *   - String variables (A$, NAME$)
 *   - Built-in string functions (LEFT$, MID$, RIGHT$, CHR$, STR$, etc.)
 *   - Custom string functions registered via registerCustomStringFunction()
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array
 * @param pos     Position pointer (advanced on return)
 * @return Heap-allocated result string (caller must free it).
 */
char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos);

#endif
