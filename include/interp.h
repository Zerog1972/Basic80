/*
 * interpreter.h - Core interpreter declarations for Basic80
 *
 * Defines the ErrorType enumeration, the program/loop/stack/data structures,
 * the extension hook system, and the Interpreter aggregate struct.
 * Declares all public functions for creating, running, and extending the
 * BASIC interpreter.
 */
#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lexer.h"
#include "vars.h"
#include "expr.h"

#define BASIC80_LINE_BUCKETS 257

/* Error categories */
typedef enum {
    ERR_NONE,
    ERR_SYNTAX,          /* Syntax error                  */
    ERR_RUNTIME,         /* Runtime error                 */
    ERR_TYPE_MISMATCH,   /* Type mismatch                 */
    ERR_OUT_OF_DATA,     /* No more DATA items            */
    ERR_DIVISION_ZERO,   /* Division by zero              */
    ERR_UNDEFINED_VAR,   /* Undefined variable            */
    ERR_ARRAY_BOUNDS,    /* Array index out of bounds     */
    ERR_OUT_OF_MEMORY    /* Out of memory                 */
} ErrorType;

/* A single line of stored BASIC source code */
typedef struct Line {
    int lineNum;
    char *code;
    char **statements;
    int statementCount;
    BasicTokenType *statementTypes;
    struct Line *next;
    struct Line *hashNext;
} Line;

/* Active FOR loop entry on the loop stack */
typedef struct ForLoop {
    char *varName;
    double endValue;
    double stepValue;
    Line *startLine;
    struct ForLoop *next;
} ForLoop;

/* GOSUB return-address entry on the call stack */
typedef struct CallStack {
    Line *returnLine;
    struct CallStack *next;
} CallStack;

/* A single DATA item (stored as a string for type flexibility) */
typedef struct DataItem {
    char *value;  /* Value stored as a string                */
    int lineNum;  /* Source line number, used by RESTORE     */
    struct DataItem *next;
} DataItem;

/* ===== EXTENSION HOOK SYSTEM ===== */

/* Callback type for a custom numeric function */
typedef double (*CustomNumericFunction)(struct Interpreter *interp, Token *tokens, int *pos);

/* Callback type for a custom string function */
typedef char* (*CustomStringFunction)(struct Interpreter *interp, Token *tokens, int *pos);

/* Callback type for a custom BASIC command */
typedef void (*CustomCommandHandler)(struct Interpreter *interp, Token *tokens);

/* Registry node for a custom numeric function */
typedef struct CustomNumFunc {
    char *name;                      /* Function name, e.g. "MYSIN"    */
    CustomNumericFunction handler;   /* Pointer to the callback         */
    struct CustomNumFunc *next;
} CustomNumFunc;

/* Registry node for a custom string function */
typedef struct CustomStrFunc {
    char *name;                      /* Function name, e.g. "REVERSE$" */
    CustomStringFunction handler;    /* Pointer to the callback         */
    struct CustomStrFunc *next;
} CustomStrFunc;

/* Registry node for a custom command */
typedef struct CustomCommand {
    char *name;                      /* Command name, e.g. "BEEP"      */
    CustomCommandHandler handler;    /* Pointer to the handler          */
    struct CustomCommand *next;
} CustomCommand;

/* Central interpreter state */
struct Interpreter {
    Line *program;
    Line *lineBuckets[BASIC80_LINE_BUCKETS];
    Variable *variables;
    Variable *variableBuckets[BASIC80_VAR_BUCKETS];
    Line *currentLine;
    ForLoop *forStack;
    CallStack *callStack;
    DataItem *dataList;      /* Head of the DATA linked list              */
    DataItem *dataListTail;  /* Tail of the DATA linked list (O(1) append) */
    DataItem *dataPointer;   /* Current READ position in the DATA list    */
    int hasError;
    ErrorType lastErrorType; /* Category of the most recent error         */
    int errorColumn;         /* Column offset of the error in the line    */
    char errorContext[256];  /* Code snippet displayed with the error     */

    /* Extension hook registries */
    CustomNumFunc *customNumFuncs;   /* Registered custom numeric functions */
    CustomStrFunc *customStrFuncs;   /* Registered custom string functions  */
    CustomCommand *customCommands;   /* Registered custom commands          */
};

/* ===== INTERPRETER FUNCTIONS ===== */

/**
 * Create and initialise a new BASIC interpreter.
 *
 * Allocates the Interpreter struct and sets all fields to their initial
 * values.  The interpreter must be released with freeInterpreter() when
 * no longer needed.
 *
 * @return Pointer to the new interpreter, or NULL on allocation failure.
 */
Interpreter* createInterpreter(void);

/**
 * Release all memory owned by an interpreter.
 *
 * Frees the program line list, variables, FOR stack, GOSUB call stack,
 * DATA list, and all extension hook registries.
 *
 * @param interp  Pointer to the interpreter to free (NULL is accepted).
 */
void freeInterpreter(Interpreter *interp);

/**
 * Add or replace a numbered line in the program.
 *
 * If a line with the same number already exists it is replaced; otherwise
 * the new line is inserted so that the program remains sorted by line number.
 *
 * @param interp   Pointer to the interpreter
 * @param lineNum  Line number (must be positive)
 * @param code     Source text for the line (without the leading number)
 *
 * Examples:
 *   addLine(interp, 10, "PRINT \"Hello\"");
 *   addLine(interp, 20, "LET A = 42");
 */
void addLine(Interpreter *interp, int lineNum, const char *code);

/**
 * Delete a numbered line from the program.
 *
 * @param interp   Pointer to the interpreter
 * @param lineNum  Line number to delete
 *
 * Note: If the line does not exist, this function is a no-op.
 */
void deleteLine(Interpreter *interp, int lineNum);

/**
 * Run the program currently loaded in the interpreter.
 *
 * Executes from the first line through to END (or the last line), handling
 * FOR/NEXT loops and GOSUB/RETURN calls.  Stops early if an error is raised.
 *
 * @param interp  Pointer to the interpreter
 */
void runProgram(Interpreter *interp);

/**
 * Execute a single statement in immediate (direct) mode.
 *
 * Parses and executes the given line without adding it to the stored
 * program.  Used by the interactive REPL.
 *
 * @param interp  Pointer to the interpreter
 * @param line    Statement text to execute
 *
 * Examples:
 *   executeCommand(interp, "PRINT A");
 *   executeCommand(interp, "LET X = 10");
 */
void executeCommand(Interpreter *interp, const char *line);

/**
 * List all lines of the stored program to stdout.
 *
 * Prints each line in ascending line-number order.
 *
 * @param interp  Pointer to the interpreter
 */
void listProgram(Interpreter *interp);

/**
 * Remove all lines from the stored program.
 *
 * Variables are preserved.  To reset the interpreter completely, call
 * freeInterpreter() followed by createInterpreter().
 *
 * @param interp  Pointer to the interpreter
 */
void clearProgram(Interpreter *interp);

/**
 * Save the current program to a text file.
 *
 * Writes each line (number + code) in ascending order, one per text line.
 *
 * @param interp    Pointer to the interpreter
 * @param filename  Destination file path
 * @return 1 on success, 0 on I/O error.
 */
int saveProgram(Interpreter *interp, const char *filename);

/**
 * Load a program from a text file.
 *
 * Clears the current program first, then reads lines from the file.
 * Each file line must begin with a line number.
 *
 * @param interp    Pointer to the interpreter
 * @param filename  Source file path
 * @return 1 on success, 0 on I/O error.
 */
int loadProgram(Interpreter *interp, const char *filename);

/* ===== ERROR REPORTING ===== */

/**
 * Report a runtime error (simple form).
 *
 * Prints the message and sets the hasError flag.  Prefer reportErrorEx()
 * for new code as it provides richer diagnostics.
 *
 * @param interp   Pointer to the interpreter
 * @param message  Error message to display
 */
void reportError(Interpreter *interp, const char *message);

/**
 * Report an error with full source context.
 *
 * Prints the error category, current line number, column offset, and a
 * visual caret pointing to the offending position in the source.
 *
 * @param interp   Pointer to the interpreter
 * @param type     Error category (ERR_SYNTAX, ERR_RUNTIME, etc.)
 * @param column   Column offset within the current line (0 = start)
 * @param message  Explanatory error message
 *
 * Sample output:
 *   [ERROR Syntax] Line 10, column 5: Variable expected
 *     --> LET = 5
 *          ^
 */
void reportErrorEx(Interpreter *interp, ErrorType type, int column, const char *message);

/**
 * Return a human-readable name for an error category.
 *
 * @param type  Error category value
 * @return Pointer to a static string describing the category.
 *
 * Examples:
 *   getErrorTypeName(ERR_SYNTAX)        -> "Syntax"
 *   getErrorTypeName(ERR_RUNTIME)       -> "Runtime"
 *   getErrorTypeName(ERR_TYPE_MISMATCH) -> "Type"
 */
const char* getErrorTypeName(ErrorType type);

/* ===== EXTENSION HOOK REGISTRY ===== */

/**
 * Register a custom numeric function callable from BASIC.
 *
 * After registration the function can be invoked by name in any numeric
 * expression, e.g. PRINT DOUBLE(21).
 *
 * @param interp    Pointer to the interpreter
 * @param name      Function name (converted to uppercase internally)
 * @param handler   Callback that evaluates the function
 * @return 1 on success, 0 on allocation failure.
 *
 * Example:
 *   double myDouble(Interpreter *interp, Token *tokens, int *pos) {
 *       double arg = evaluateExpression(interp, tokens, pos);
 *       return arg * 2.0;
 *   }
 *   registerCustomNumericFunction(interp, "DOUBLE", myDouble);
 *   // BASIC: PRINT DOUBLE(21)  -> 42
 */
int registerCustomNumericFunction(Interpreter *interp, const char *name, CustomNumericFunction handler);

/**
 * Register a custom string function callable from BASIC.
 *
 * After registration the function can be used in string expressions,
 * e.g. PRINT REVERSE$("Hello").
 *
 * @param interp    Pointer to the interpreter
 * @param name      Function name (must end with '$'; converted to uppercase)
 * @param handler   Callback that returns a heap-allocated string (caller frees)
 * @return 1 on success, 0 on allocation failure.
 *
 * Example:
 *   char* myReverse(Interpreter *interp, Token *tokens, int *pos) {
 *       char *str = evaluateStringExpression(interp, tokens, pos);
 *       // ... reverse str in place ...
 *       return str;
 *   }
 *   registerCustomStringFunction(interp, "REVERSE$", myReverse);
 *   // BASIC: PRINT REVERSE$("Hello")  -> olleH
 */
int registerCustomStringFunction(Interpreter *interp, const char *name, CustomStringFunction handler);

/**
 * Register a custom BASIC command.
 *
 * After registration the command keyword can appear at the start of any
 * BASIC statement, e.g. BEEP.
 *
 * @param interp    Pointer to the interpreter
 * @param name      Command name (converted to uppercase internally)
 * @param handler   Callback that executes the command
 * @return 1 on success, 0 on allocation failure.
 *
 * Example:
 *   void myBeep(Interpreter *interp, Token *tokens) {
 *       printf("\a");
 *       fflush(stdout);
 *   }
 *   registerCustomCommand(interp, "BEEP", myBeep);
 *   // BASIC: BEEP
 */
int registerCustomCommand(Interpreter *interp, const char *name, CustomCommandHandler handler);

/**
 * Look up a registered custom numeric function by name.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Function name to search for
 * @return The handler pointer if found, NULL otherwise.
 */
CustomNumericFunction findCustomNumericFunction(Interpreter *interp, const char *name);

/**
 * Look up a registered custom string function by name.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Function name to search for
 * @return The handler pointer if found, NULL otherwise.
 */
CustomStringFunction findCustomStringFunction(Interpreter *interp, const char *name);

/**
 * Look up a registered custom command by name.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Command name to search for
 * @return The handler pointer if found, NULL otherwise.
 */
CustomCommandHandler findCustomCommand(Interpreter *interp, const char *name);

#endif /* INTERPRETER_H */