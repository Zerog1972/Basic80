/*
 * commands.h - BASIC command handler declarations for Basic80
 *
 * Declares the public handler functions for each BASIC statement that
 * is not a control-flow instruction.  Each function receives the
 * interpreter state and the token array produced by tokenize().
 */
#ifndef COMMANDS_H
#define COMMANDS_H

#include "interp.h"
#include "lexer.h"

/* ===== BASIC COMMAND HANDLERS ===== */

/**
 * Execute the PRINT statement.
 *
 * Outputs one or more numeric or string expressions to stdout.  Multiple
 * expressions can be separated by commas (tab stop) or semicolons (no gap).
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the PRINT arguments
 *
 * BASIC syntax:
 *   PRINT expr [, expr]* [;]
 *
 * Examples:
 *   PRINT "Hello"          -> Outputs: Hello
 *   PRINT A, B             -> Outputs: 42.00       10.00
 *   PRINT "X="; X          -> Outputs: X=5.00  (no newline gap)
 *   PRINT                  -> Outputs a blank line
 *
 * Separators:
 *   , (comma)      -> space between expressions
 *   ; (semicolon)  -> no space between expressions
 *   End of tokens  -> automatic newline
 */
void handlePrint(Interpreter *interp, Token *tokens);

/**
 * Execute the LET statement (variable assignment).
 *
 * Assigns a value to a simple variable, a string variable, or an array
 * element.  The LET keyword is optional in BASIC.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the assignment
 *
 * BASIC syntax:
 *   [LET] variable = expression
 *   [LET] array(indices) = expression
 *
 * Examples:
 *   LET A = 42
 *   X = 10 + 5
 *   A$ = "Hello"
 *   M(2,3) = 99
 *
 * Note: Variables are created automatically the first time they are assigned.
 */
void handleLet(Interpreter *interp, Token *tokens);

/**
 * Execute the DIM statement (array declaration).
 *
 * Allocates a multi-dimensional array with the given sizes.  Supports up
 * to 10 dimensions.  All elements are initialised to 0.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the declaration
 *
 * BASIC syntax:
 *   DIM array(size1 [, size2, ...])
 *
 * Examples:
 *   DIM A(10)          -> 1-D array of 10 elements
 *   DIM M(5, 8)        -> 2-D array of 5x8 elements
 *   DIM T(3, 4, 5)     -> 3-D array of 3x4x5 elements
 *
 * Note: Re-dimensioning an existing array generates an error.
 */
void handleDim(Interpreter *interp, Token *tokens);

/**
 * Execute the INPUT statement (user input).
 *
 * Prompts the user for a value and stores it in the given variable.
 * Displays an optional prompt and waits for keyboard input.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the INPUT arguments
 *
 * BASIC syntax:
 *   INPUT [message;] variable
 *
 * Examples:
 *   INPUT A                -> ? _
 *   INPUT "Name"; N$       -> Name? _
 *   INPUT "Age: "; AGE     -> Age: _
 *
 * Note: Input is converted to a number for numeric variables and stored
 *       as-is for string variables (names ending with '$').
 */
void handleInput(Interpreter *interp, Token *tokens);

/**
 * Execute the DATA statement (constant data declaration).
 *
 * Stores constant values in the interpreter's DATA list for sequential
 * retrieval by READ.  Values may be numeric or string literals.
 *
 * @param interp   Pointer to the interpreter
 * @param tokens   Token array containing the DATA values
 * @param lineNum  Source line number (used by the RESTORE command)
 *
 * BASIC syntax:
 *   DATA value1 [, value2, ...]
 *
 * Examples:
 *   DATA 10, 20, 30
 *   DATA "Alice", "Bob", "Charlie"
 *   DATA 42, "Hello", 3.14
 *
 * Note: DATA items are collected in a first pass before execution begins.
 *       They are read sequentially by READ and repositioned by RESTORE.
 */
void handleData(Interpreter *interp, Token *tokens, int lineNum);

/**
 * Execute the READ statement.
 *
 * Reads values sequentially from the DATA list and assigns them to the
 * specified variables.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the variable list
 *
 * BASIC syntax:
 *   READ variable1 [, variable2, ...]
 *
 * Examples:
 *   10 DATA 10, 20, 30
 *   20 READ A, B, C        -> A=10, B=20, C=30
 *
 * Note: Attempting to READ past the last DATA item raises ERR_OUT_OF_DATA.
 *       Use RESTORE to rewind the DATA pointer.
 */
void handleRead(Interpreter *interp, Token *tokens);

/**
 * Execute the RESTORE statement.
 *
 * Resets the DATA read pointer to the beginning of all DATA items, or to
 * the first DATA item at or after a specified line number.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array (may contain an optional line number)
 *
 * BASIC syntax:
 *   RESTORE [lineNum]
 *
 * Examples:
 *   RESTORE           -> Rewind to the first DATA item
 *   RESTORE 100       -> Rewind to the first DATA item at line 100
 *
 * Note: If the specified line has no DATA, the pointer advances to
 *       the next line that does.
 */
void handleRestore(Interpreter *interp, Token *tokens);

/**
 * Execute the HELP statement.
 *
 * Displays the list of available commands, or detailed help for a single
 * command when one is given as an argument.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array (may contain an optional command name)
 *
 * BASIC syntax:
 *   HELP              -> List all commands
 *   HELP command      -> Show detailed help for a specific command
 *
 * Examples:
 *   HELP              -> Displays the complete command list
 *   HELP PRINT        -> Detailed help for PRINT
 *   HELP FOR          -> Detailed help for FOR
 */
void handleHelp(Interpreter *interp, Token *tokens);

/**
 * Execute the CLS statement (clear screen).
 *
 * Clears the terminal screen and moves the cursor to the top-left corner.
 *
 * @param interp  Pointer to the interpreter (unused)
 * @param tokens  Token array (unused)
 *
 * BASIC syntax:
 *   CLS
 */
void handleCls(Interpreter *interp, Token *tokens);

#endif /* COMMANDS_H */
