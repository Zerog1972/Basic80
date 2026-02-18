/*
 * control_flow.h - Control-flow statement declarations for Basic80
 *
 * Declares the handler functions for IF/THEN/ELSE, GOTO, GOSUB/RETURN,
 * FOR/NEXT, and the utility helper findLineByNumber().
 */
#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

#include "interpreter.h"
#include "lexer.h"

/* ===== CONTROL FLOW HANDLERS ===== */

/**
 * Execute an IF/THEN/ELSE statement.
 *
 * Evaluates the condition and executes either the THEN branch or the
 * optional ELSE branch depending on the result.
 *
 * @param interp       Pointer to the interpreter
 * @param tokens       Token array containing the IF statement
 * @param currentLine  Double pointer to the current line (modified on GOTO)
 * @return 1 if a branch jump occurred (GOTO inside THEN/ELSE), 0 otherwise.
 *
 * BASIC syntax:
 *   IF condition THEN statement [ELSE statement]
 *   IF condition THEN lineNum  [ELSE lineNum]
 *
 * Examples:
 *   IF X > 5 THEN PRINT "Large"
 *   IF A = 0 THEN GOTO 100
 *   IF X < 0 THEN Y = -1 ELSE Y = 1
 *   IF A$ = "Y" THEN 100 ELSE 200
 *
 * Note: Only single-line IF is supported.  Use GOTO with line numbers for
 *       multi-statement branches.
 */
int handleIfStatement(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Execute a GOTO statement (unconditional jump).
 *
 * Transfers control to the specified line number.
 *
 * @param interp       Pointer to the interpreter
 * @param tokens       Token array containing the target line number
 * @param currentLine  Double pointer to the current line (modified)
 * @return 1 if the jump succeeded, 0 on error (line not found).
 *
 * BASIC syntax:
 *   GOTO lineNum
 *
 * Examples:
 *   GOTO 100
 *   IF X = 0 THEN GOTO 200
 *
 * Note: An error is reported if the target line does not exist.
 */
int handleGoto(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Execute a GOSUB statement (subroutine call).
 *
 * Pushes the return address onto the call stack and transfers control to
 * the specified subroutine line.  Use RETURN to come back.
 *
 * @param interp       Pointer to the interpreter
 * @param tokens       Token array containing the target line number
 * @param currentLine  Double pointer to the current line (modified)
 * @return 1 if the call succeeded, 0 on error.
 *
 * BASIC syntax:
 *   GOSUB lineNum
 *
 * Examples:
 *   GOSUB 1000
 *   IF FLAG THEN GOSUB 500
 *
 * Note: GOSUB calls can be nested.  The call stack is managed automatically.
 */
int handleGosub(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Execute a RETURN statement (subroutine return).
 *
 * Returns execution to the line following the most recent GOSUB.
 * Pops the call stack.
 *
 * @param interp       Pointer to the interpreter
 * @param currentLine  Double pointer to the current line (modified)
 * @return 1 if the return succeeded, 0 if the call stack was empty.
 *
 * BASIC syntax:
 *   RETURN
 *
 * Note: An error is triggered if RETURN is executed without a prior GOSUB.
 */
int handleReturn(Interpreter *interp, Line **currentLine);

/**
 * Execute a FOR statement (loop initialisation).
 *
 * Sets up the loop control variable, end value, and optional step, then
 * decides whether to enter the loop body or skip to the matching NEXT.
 *
 * @param interp       Pointer to the interpreter
 * @param tokens       Token array containing the FOR parameters
 * @param currentLine  Double pointer to the current line (modified on skip)
 * @return 1 if the loop body was skipped (step direction makes it empty), 0 otherwise.
 *
 * BASIC syntax:
 *   FOR variable = start TO end [STEP increment]
 *
 * Examples:
 *   FOR I = 1 TO 10          -> I from 1 to 10 in steps of 1
 *   FOR X = 0 TO 100 STEP 5  -> X from 0 to 100 in steps of 5
 *   FOR J = 10 TO 1 STEP -1  -> J from 10 down to 1
 *
 * Note: If the initial condition is already false the loop body is never
 *       executed (program counter jumps to the matching NEXT).
 */
int handleFor(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Execute a NEXT statement (loop end).
 *
 * Increments the loop variable and tests whether the loop should continue.
 * Returns to the FOR line if so, or pops the loop stack and falls through.
 *
 * @param interp  Pointer to the interpreter
 * @return 1 if execution should jump back to the FOR line, 0 if the loop ended.
 *
 * BASIC syntax:
 *   NEXT [variable]
 *
 * Examples:
 *   NEXT
 *   NEXT I
 *
 * Note: The variable name after NEXT is optional and currently ignored.
 *       NEXT always applies to the innermost open FOR loop.
 */
int handleNext(Interpreter *interp);

/**
 * Locate a program line by its line number.
 *
 * Utility function used by GOTO, GOSUB, and the FOR/NEXT machinery.
 *
 * @param interp   Pointer to the interpreter
 * @param lineNum  Line number to search for
 * @return Pointer to the Line node if found, NULL otherwise.
 */
Line* findLineByNumber(Interpreter *interp, int lineNum);

#endif /* CONTROL_FLOW_H */
