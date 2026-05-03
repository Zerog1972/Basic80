/*
 * commands.c - BASIC command handlers for Basic80
 *
 * Implements the runtime execution of all BASIC commands:
 *   PRINT, LET, DIM, INPUT, DATA, READ, RESTORE, HELP
 *
 * Each handler receives the interpreter state and an array of tokens
 * produced by tokenize().  Handlers call the expression evaluator and
 * variable management functions as needed, and report errors through
 * reportErrorEx().
 */
#include "commands.h"
#include "expr.h"
#include "vars.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Safety constants */
#define MAX_VARNAME_LEN 256
#define MAX_INPUT_BUFFER 1024
#define MAX_ARRAY_DIMENSIONS 10

/**
 * Returns 1 if the variable name ends with '$' (string variable convention).
 */
static int isStringVariable(const char *name) {
    size_t len = strlen(name);
    return len > 0 && name[len - 1] == '$';
}

/* ===== PRINT COMMAND ===== */

/**
 * Handle the PRINT command.
 *
 * Outputs one or more expressions to stdout.  Supports both numeric and
 * string expressions.  The comma separator inserts a space between items;
 * the semicolon separator produces no gap; a trailing newline is added
 * unless the last token is a semicolon.
 *
 * Syntax: PRINT [expression [,|; expression]...]
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array containing the expressions to print
 */
void handlePrint(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    int trailingSemicolon;
    
    pos = 1;
    trailingSemicolon = 0;
    while (tokens[pos].type != TOK_EOF) {
        trailingSemicolon = 0;
        if (isStringExpression(interp, tokens, pos)) {
            char *strResult = evaluateStringExpression(interp, tokens, &pos);
            if (!strResult) {
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Out of memory evaluating string expression.");
                return;
            }
            printf("%s", strResult);
            free(strResult);
        } else {
            val = evaluateExpression(interp, tokens, &pos);
            printf("%.2f", val);
        }
        if (tokens[pos].type == TOK_COMMA) {
            printf(" ");
            pos++;
        } else if (tokens[pos].type == TOK_SEMICOLON) {
            /* Semicolon: no space between items */
            trailingSemicolon = 1;
            pos++;
        }
    }
    if (!trailingSemicolon) printf("\n");
}

/* ===== LET COMMAND ===== */

/**
 * Handle the LET command (variable assignment).
 *
 * Assigns a value to a simple variable or to an array element.  The keyword
 * LET is optional.  Supports numeric variables and string variables (names
 * ending with '$').
 *
 * Syntax:
 *   [LET] variable = expression
 *   [LET] array(indices) = expression
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array containing the assignment
 */
void handleLet(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    char varName[MAX_VARNAME_LEN];
    
    /* Detect whether the statement begins with LET or is a direct assignment */
    pos = (tokens[0].type == TOK_LET) ? 1 : 0;
    
    if (tokens[pos].type != TOK_IDENTIFIER) {
        reportErrorEx(interp, ERR_SYNTAX, pos, "Variable name expected after LET.");
        return;
    }
    
    strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
    varName[MAX_VARNAME_LEN - 1] = '\0';
    pos++;
    
    /* Check if this is an array element assignment */
    if (tokens[pos].type == TOK_LPAREN) {
        int indices[MAX_ARRAY_DIMENSIONS];
        int numIndices;
        
        pos++;
        numIndices = 0;
        /* Read all comma-separated indices */
        while (numIndices < MAX_ARRAY_DIMENSIONS) {
            indices[numIndices] = (int)evaluateExpression(interp, tokens, &pos);
            numIndices++;
            if (tokens[pos].type == TOK_COMMA) {
                pos++; /* Advance past the comma */
            } else {
                break; /* No more indices */
            }
        }        /* Check that we have not exceeded the maximum number of dimensions */
        if (numIndices >= MAX_ARRAY_DIMENSIONS && tokens[pos].type == TOK_COMMA) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "Too many dimensions (maximum 10).");
            return;
        }        if (tokens[pos].type != TOK_RPAREN) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "')' expected after array indices.");
            return;
        }
        pos++;
        if (tokens[pos].type != TOK_EQUALS) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "'=' expected in assignment.");
            return;
        }
        pos++;
        val = evaluateExpression(interp, tokens, &pos);
        setArrayElement(interp, varName, indices, numIndices, val);
    } else if (tokens[pos].type == TOK_EQUALS) {
        pos++;
        
        if (isStringVariable(varName)) {
            char *strResult = evaluateStringExpression(interp, tokens, &pos);
            if (!strResult) {
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour l'évaluation de chaîne.");
                return;
            }
            setStringVariable(interp, varName, strResult);
            free(strResult);
        } else {
            if (isStringExpression(interp, tokens, pos)) {
                reportErrorEx(interp, ERR_TYPE_MISMATCH, pos, "Cannot assign a string to a numeric variable.");
                return;
            }
            val = evaluateExpression(interp, tokens, &pos);
            setVariable(interp, varName, val);
        }
    } else {
        reportErrorEx(interp, ERR_SYNTAX, pos, "'=' or '(' expected after variable name.");
    }
}

/* ===== DIM COMMAND ===== */

/**
 * Handle the DIM command (array declaration).
 *
 * Declares a multi-dimensional numeric array with the given sizes.  Supports
 * up to 10 dimensions.  BASIC arrays are 0-based, so DIM A(10) creates an
 * array with 11 elements (indices 0..10).
 *
 * Syntax: DIM array(size1 [, size2, ...])
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array containing the DIM declaration
 */
void handleDim(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    int dims[MAX_ARRAY_DIMENSIONS];
    int numDims;
    
    pos = 1;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
        varName[MAX_VARNAME_LEN - 1] = '\0';
        pos++;
        if (tokens[pos].type == TOK_LPAREN) {
            pos++;
            numDims = 0;
            /* Read all comma-separated dimension sizes */
            while (numDims < MAX_ARRAY_DIMENSIONS) {
                int dimValue = (int)evaluateExpression(interp, tokens, &pos);
                /* Validate that dimension size is non-negative */
                if (dimValue < 0) {
                    reportErrorEx(interp, ERR_SYNTAX, pos, "Negative array dimension.");
                    return;
                }
                dims[numDims] = dimValue + 1; /* +1 because BASIC indices start at 0 */
                numDims++;
                if (tokens[pos].type == TOK_COMMA) {
                    pos++; /* Advance past the comma */
                } else {
                    break; /* No more dimensions */
                }
            }
            /* Check that the maximum dimension count is not exceeded */
            if (numDims >= MAX_ARRAY_DIMENSIONS && tokens[pos].type == TOK_COMMA) {
                reportErrorEx(interp, ERR_SYNTAX, pos, "Too many dimensions (maximum 10).");
                return;
            }
            if (tokens[pos].type != TOK_RPAREN) {
                reportErrorEx(interp, ERR_SYNTAX, pos, "')' expected after array dimensions.");
                return;
            }
            pos++;
            createArray(interp, varName, dims, numDims);
        }
    }
}

/* ===== INPUT COMMAND ===== */

/**
 * Handle the INPUT command.
 *
 * Prompts the user to enter a value and stores it in the specified variable.
 * For string variables (ending with '$') reads a full text line.  For
 * numeric variables reads a floating-point number.
 *
 * Syntax: INPUT variable
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array containing the variable name
 */
void handleInput(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    char buffer[MAX_INPUT_BUFFER];
    
    pos = 1;
    if (tokens[pos].type != TOK_IDENTIFIER) {
        reportErrorEx(interp, ERR_SYNTAX, pos, "Variable name expected after INPUT.");
        return;
    }
    
    strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
    varName[MAX_VARNAME_LEN - 1] = '\0';
    
    printf("? ");
    if (isStringVariable(varName)) {
        if (fgets(buffer, sizeof(buffer), stdin)) {
            buffer[strcspn(buffer, "\n")] = 0;
            setStringVariable(interp, varName, buffer);
        }
    } else {
        double val;
        if (scanf("%lf", &val) == 1) {
            /* Consume the rest of the input line */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            setVariable(interp, varName, val);
        } else {
            /* Invalid input: flush the buffer and report an error */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            reportErrorEx(interp, ERR_SYNTAX, pos, "Invalid numeric input.");
        }
    }
}

/* ===== DATA/READ/RESTORE COMMANDS ===== */

/**
 * Handle the DATA command.
 *
 * Stores constant values in a linked list for later retrieval by READ.
 * Values may be numeric or string literals.
 *
 * @param interp  Pointer to the interpreter
 * @param tokens  Token array containing the DATA values
 * @param lineNum Source line number (used by RESTORE)
 */
void handleData(Interpreter *interp, Token *tokens, int lineNum) {
    int pos;
    DataItem *newItem;
    char buffer[MAX_INPUT_BUFFER];
    
    pos = 1; /* Skip the DATA token */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Skip comma separators */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        /* Allocate a new DATA item */
        newItem = malloc(sizeof(DataItem));
        if (!newItem) {
            reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Out of memory for DATA");
            return;
        }
        newItem->lineNum = lineNum;
        newItem->next = NULL;
        
        /* Copy the value according to its token type */
        if (tokens[pos].type == TOK_STRING || tokens[pos].type == TOK_NUMBER) {
            /* String and number literals share the same copy logic */
            newItem->value = malloc(strlen(tokens[pos].value) + 1);
            if (!newItem->value) {
                free(newItem);
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Out of memory for DATA");
                return;
            }
            strcpy(newItem->value, tokens[pos].value);
        } else if (tokens[pos].type == TOK_MINUS && tokens[pos + 1].type == TOK_NUMBER) {
            /* Handle negative number literals */
            sprintf(buffer, "-%s", tokens[pos + 1].value);
            newItem->value = malloc(strlen(buffer) + 1);
            if (!newItem->value) {
                free(newItem);
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Out of memory for DATA");
                return;
            }
            strcpy(newItem->value, buffer);
            pos++; /* Advance past the number following the minus sign */
        } else {
            /* Unrecognised token: skip it */
            free(newItem);
            pos++;
            continue;
        }
        
        /* Append to the DATA linked list in O(1) via the tail pointer */
        if (!interp->dataList) {
            interp->dataList = newItem;
            interp->dataPointer = newItem;
        } else {
            interp->dataListTail->next = newItem;
        }
        interp->dataListTail = newItem;
        
        pos++;
    }
}

/**
 * Handle the READ command.
 *
 * Reads values sequentially from the DATA list and assigns them to the
 * specified variables.
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array containing the variable names
 */
void handleRead(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    
    pos = 1; /* Skip the READ token */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Skip comma separators */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        if (tokens[pos].type != TOK_IDENTIFIER) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "Variable name expected after READ");
            return;
        }
        
        strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
        varName[MAX_VARNAME_LEN - 1] = '\0';
        
        /* Check that more DATA is available */
        if (!interp->dataPointer) {
            reportErrorEx(interp, ERR_OUT_OF_DATA, pos, "Out of DATA");
            return;
        }
        
        /* Assign the value according to the variable type */
        if (isStringVariable(varName)) {
            /* String variable */
            setStringVariable(interp, varName, interp->dataPointer->value);
        } else {
            /* Numeric variable */
            double val = atof(interp->dataPointer->value);
            setVariable(interp, varName, val);
        }
        
        /* Advance the DATA pointer */
        interp->dataPointer = interp->dataPointer->next;
        
        pos++;
    }
}

/**
 * Handle the RESTORE command.
 *
 * Resets the DATA read pointer.  Without an argument, rewinds to the very
 * first DATA item.  With a line number, positions to the first DATA item
 * at or after that line.
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array (may contain an optional line number)
 */
void handleRestore(Interpreter *interp, Token *tokens) {
    int targetLine;
    DataItem *item;
    
    /* RESTORE without argument: rewind to the beginning */
    if (tokens[1].type == TOK_EOF) {
        interp->dataPointer = interp->dataList;
        return;
    }
    
    /* RESTORE with a line number argument */
    if (tokens[1].type == TOK_NUMBER) {
        targetLine = atoi(tokens[1].value);  /* atoi suffit pour un numéro de ligne */
        
        /* Find the first DATA item at or after the given line */
        item = interp->dataList;
        while (item && item->lineNum < targetLine) {
            item = item->next;
        }
        
        if (item && item->lineNum == targetLine) {
            interp->dataPointer = item;
        } else {
            /* Line not found: rewind to the beginning */
            interp->dataPointer = interp->dataList;
        }
    } else {
        interp->dataPointer = interp->dataList;
    }
}

/* ===== HELP COMMAND ===== */

/**
 * Handle the HELP command.
 *
 * Without an argument, displays the full list of available commands.
 * With a command or function name, shows detailed help for that item.
 *
 * Syntax: HELP [command]
 *
 * @param interp Pointer to the interpreter
 * @param tokens Token array (may contain an optional command name)
 */
void handleHelp(Interpreter *interp, Token *tokens) {
    char cmdName[MAX_VARNAME_LEN];
    int i;
    
    /* HELP without argument: display the command list */
    if (tokens[1].type == TOK_EOF) {
        printf("=== BASIC80 - Available commands ===\n\n");
        printf("Basic commands:\n");
        printf("  PRINT    - Display text or values\n");
        printf("  LET      - Assign a value to a variable\n");
        printf("  INPUT    - Read a value from the keyboard\n");
        printf("  REM      - Add a comment\n\n");

        printf("Control structures:\n");
        printf("  IF       - Conditional execution\n");
        printf("  FOR      - Counter loop\n");
        printf("  NEXT     - End of FOR loop\n");
        printf("  GOTO     - Jump to a line\n");
        printf("  GOSUB    - Call a subroutine\n");
        printf("  RETURN   - Return from a subroutine\n");
        printf("  END      - Terminate the program\n\n");

        printf("Data handling:\n");
        printf("  DIM      - Declare an array\n");
        printf("  DATA     - Define constant data\n");
        printf("  READ     - Read data values\n");
        printf("  RESTORE  - Reset the DATA pointer\n\n");

        printf("Math functions:\n");
        printf("  ABS      - Absolute value\n");
        printf("  SQR      - Square root\n");
        printf("  INT      - Integer part\n");
        printf("  SGN      - Sign of a number\n");
        printf("  RND      - Random number\n");
        printf("  SIN/COS/TAN      - Trigonometric functions\n");
        printf("  ATN/ATAN/ASIN/ACOS - Inverse trigonometric functions\n");
        printf("  EXP/LOG/LOG10    - Exponential and logarithms\n");
        printf("  POW              - Power (x^y)\n");
        printf("  DEG/RAD          - Angle unit conversion\n");
        printf("  SINH/COSH/TANH   - Hyperbolic functions\n\n");

        printf("String functions:\n");
        printf("  LEN      - Length of a string\n");
        printf("  LEFT$/RIGHT$/MID$ - Substring extraction\n");
        printf("  CHR$     - ASCII character\n");
        printf("  ASC      - ASCII code of a character\n");
        printf("  STR$     - Convert number to string\n");
        printf("  VAL      - Convert string to number\n");
        printf("  SPACE$   - String of spaces\n");
        printf("  STRING$  - Repeat a character\n\n");

        printf("System commands:\n");
        printf("  LIST     - Display the program\n");
        printf("  RUN      - Run the program\n");
        printf("  NEW      - Clear the program\n");
        printf("  SAVE     - Save the program\n");
        printf("  LOAD     - Load a program\n");
        printf("  CLS      - Clear the screen\n");
        printf("  EXIT     - Quit the interpreter\n");
        printf("  HELP     - Show this help\n\n");

        printf("Type HELP [COMMAND] or HELP [FUNCTION] for more details.\n");
        printf("Examples: HELP PRINT, HELP SIN, HELP LEN\n\n");
        return;
    }
    
    /* HELP with an argument: display detailed help for a specific command */
    /* Accept any token that carries a value (not just TOK_IDENTIFIER) */
    if (tokens[1].type != TOK_EOF && tokens[1].value) {
        strncpy(cmdName, tokens[1].value, MAX_VARNAME_LEN - 1);
        cmdName[MAX_VARNAME_LEN - 1] = '\0';
        
        /* Normalize to uppercase for case-insensitive matching */
        for (i = 0; cmdName[i]; i++) {
            cmdName[i] = toupper(cmdName[i]);
        }
        
        if (strcmp(cmdName, "PRINT") == 0) {
            printf("=== PRINT ===\n\n");
            printf("Syntax: PRINT [expression [,|; expression]...]\n\n");
            printf("Description:\n");
            printf("  Outputs one or more expressions to the screen.\n");
            printf("  Expressions can be numeric or string.\n\n");
            printf("Separators:\n");
            printf("  , (comma)        : Space between expressions\n");
            printf("  ; (semicolon)    : No space between expressions\n");
            printf("  End of line      : Automatic newline\n\n");
            printf("Examples:\n");
            printf("  PRINT \"Hello\"           -> Outputs: Hello\n");
            printf("  PRINT 42                -> Outputs: 42.00\n");
            printf("  PRINT A, B              -> Outputs: 10.00 20.00\n");
            printf("  PRINT \"X=\"; X           -> Outputs: X=5.00\n\n");
        }
        else if (strcmp(cmdName, "LET") == 0) {
            printf("=== LET ===\n\n");
            printf("Syntax: [LET] variable = expression\n");
            printf("        [LET] array(indices) = expression\n\n");
            printf("Description:\n");
            printf("  Assigns a value to a variable or an array element.\n");
            printf("  The LET keyword is optional.\n\n");
            printf("Examples:\n");
            printf("  LET A = 42\n");
            printf("  X = 10 + 5\n");
            printf("  A$ = \"Hello\"\n");
            printf("  M(2,3) = 99\n\n");
        }
        else if (strcmp(cmdName, "INPUT") == 0) {
            printf("=== INPUT ===\n\n");
            printf("Syntax: INPUT variable\n\n");
            printf("Description:\n");
            printf("  Asks the user to enter a value.\n");
            printf("  For string variables (ending with $), reads text.\n");
            printf("  For numeric variables, reads a number.\n\n");
            printf("Examples:\n");
            printf("  INPUT A        -> Waits for a number\n");
            printf("  INPUT NAME$    -> Waits for text\n\n");
        }
        else if (strcmp(cmdName, "DIM") == 0) {
            printf("=== DIM ===\n\n");
            printf("Syntax: DIM array(size1 [, size2, ...])\n\n");
            printf("Description:\n");
            printf("  Declares a multi-dimensional array.\n");
            printf("  Supports up to 10 dimensions.\n");
            printf("  Indices start at 0.\n\n");
            printf("Examples:\n");
            printf("  DIM A(10)         -> 1D array of 11 elements (0-10)\n");
            printf("  DIM M(5, 8)       -> 2D array of 6x9 elements\n");
            printf("  DIM T(3, 4, 5)    -> 3D array\n\n");
        }
        else if (strcmp(cmdName, "IF") == 0) {
            printf("=== IF ===\n\n");
            printf("Syntax: IF condition THEN statement [ELSE statement]\n\n");
            printf("Description:\n");
            printf("  Executes a statement if the condition is true.\n");
            printf("  The ELSE clause is optional.\n\n");
            printf("Comparison operators:\n");
            printf("  =  : Equal\n");
            printf("  <> : Not equal\n");
            printf("  <  : Less than\n");
            printf("  >  : Greater than\n");
            printf("  <= : Less than or equal\n");
            printf("  >= : Greater than or equal\n\n");
            printf("Examples:\n");
            printf("  IF X > 10 THEN PRINT \"Large\"\n");
            printf("  IF A = B THEN X = 1 ELSE X = 2\n");
            printf("  IF A$ = \"YES\" THEN GOTO 100\n\n");
        }
        else if (strcmp(cmdName, "FOR") == 0) {
            printf("=== FOR / NEXT ===\n\n");
            printf("Syntax: FOR variable = start TO end [STEP increment]\n");
            printf("        ... statements ...\n");
            printf("        NEXT [variable]\n\n");
            printf("Description:\n");
            printf("  Counter loop.\n");
            printf("  STEP is optional (default 1).\n\n");
            printf("Examples:\n");
            printf("  FOR I = 1 TO 10\n");
            printf("    PRINT I\n");
            printf("  NEXT I\n\n");
            printf("  FOR J = 10 TO 1 STEP -1\n");
            printf("    PRINT J\n");
            printf("  NEXT J\n\n");
        }
        else if (strcmp(cmdName, "GOTO") == 0) {
            printf("=== GOTO ===\n\n");
            printf("Syntax: GOTO lineNumber\n\n");
            printf("Description:\n");
            printf("  Jumps to a specific line in the program.\n\n");
            printf("Example:\n");
            printf("  10 PRINT \"Start\"\n");
            printf("  20 GOTO 40\n");
            printf("  30 PRINT \"Skipped\"\n");
            printf("  40 PRINT \"End\"\n\n");
        }
        else if (strcmp(cmdName, "GOSUB") == 0 || strcmp(cmdName, "RETURN") == 0) {
            printf("=== GOSUB / RETURN ===\n\n");
            printf("Syntax: GOSUB lineNumber\n");
            printf("        RETURN\n\n");
            printf("Description:\n");
            printf("  GOSUB calls a subroutine at the specified line.\n");
            printf("  RETURN goes back to the statement after GOSUB.\n\n");
            printf("Example:\n");
            printf("  10 GOSUB 100\n");
            printf("  20 PRINT \"Back\"\n");
            printf("  30 END\n");
            printf("  100 PRINT \"Subroutine\"\n");
            printf("  110 RETURN\n\n");
        }
        else if (strcmp(cmdName, "DATA") == 0 || strcmp(cmdName, "READ") == 0) {
            printf("=== DATA / READ / RESTORE ===\n\n");
            printf("Syntax: DATA value1, value2, ...\n");
            printf("        READ variable1, variable2, ...\n");
            printf("        RESTORE [lineNumber]\n\n");
            printf("Description:\n");
            printf("  DATA defines constant values in the program.\n");
            printf("  READ reads those values sequentially.\n");
            printf("  RESTORE resets the read pointer.\n\n");
            printf("Example:\n");
            printf("  10 DATA 10, 20, 30\n");
            printf("  20 READ A, B, C\n");
            printf("  30 PRINT A; B; C    -> Outputs: 10.00 20.00 30.00\n\n");
        }
        else if (strcmp(cmdName, "RESTORE") == 0) {
            printf("=== RESTORE ===\n\n");
            printf("Syntax: RESTORE [lineNumber]\n\n");
            printf("Description:\n");
            printf("  Resets the DATA read pointer.\n");
            printf("  Without argument, rewinds to the first DATA item.\n");
            printf("  With a line number, positions at the DATA on that line.\n\n");
            printf("Example:\n");
            printf("  10 DATA 10, 20, 30\n");
            printf("  20 READ A, B\n");
            printf("  30 RESTORE\n");
            printf("  40 READ C          -> C = 10 (restarts from beginning)\n\n");
        }
        else if (strcmp(cmdName, "REM") == 0) {
            printf("=== REM ===\n\n");
            printf("Syntax: REM comment\n\n");
            printf("Description:\n");
            printf("  Adds a comment to the program.\n");
            printf("  Everything after REM is ignored.\n\n");
            printf("Example:\n");
            printf("  10 REM This is a comment\n");
            printf("  20 PRINT \"Hello\"  REM Display a message\n\n");
        }
        else if (strcmp(cmdName, "END") == 0) {
            printf("=== END ===\n\n");
            printf("Syntax: END\n\n");
            printf("Description:\n");
            printf("  Terminates program execution.\n\n");
            printf("Example:\n");
            printf("  10 PRINT \"Program\"\n");
            printf("  20 END\n");
            printf("  30 PRINT \"Never executed\"\n\n");
        }
        else if (strcmp(cmdName, "LIST") == 0) {
            printf("=== LIST ===\n\n");
            printf("Syntax: LIST\n\n");
            printf("Description:\n");
            printf("  Displays all lines of the program in memory.\n\n");
        }
        else if (strcmp(cmdName, "RUN") == 0) {
            printf("=== RUN ===\n\n");
            printf("Syntax: RUN\n\n");
            printf("Description:\n");
            printf("  Runs the program in memory from the beginning.\n\n");
        }
        else if (strcmp(cmdName, "NEW") == 0) {
            printf("=== NEW ===\n\n");
            printf("Syntax: NEW\n\n");
            printf("Description:\n");
            printf("  Clears the program in memory and all variables.\n\n");
        }
        else if (strcmp(cmdName, "SAVE") == 0) {
            printf("=== SAVE ===\n\n");
            printf("Syntax: SAVE \"filename.bas\"\n\n");
            printf("Description:\n");
            printf("  Saves the current program to a file.\n\n");
            printf("Example:\n");
            printf("  SAVE \"myprog.bas\"\n\n");
        }
        else if (strcmp(cmdName, "LOAD") == 0) {
            printf("=== LOAD ===\n\n");
            printf("Syntax: LOAD \"filename.bas\"\n\n");
            printf("Description:\n");
            printf("  Loads a program from a file.\n\n");
            printf("Example:\n");
            printf("  LOAD \"myprog.bas\"\n\n");
        }
        else if (strcmp(cmdName, "HELP") == 0) {
            printf("=== HELP ===\n\n");
            printf("Syntax: HELP [command]\n\n");
            printf("Description:\n");
            printf("  Without argument: displays the command list.\n");
            printf("  With argument: shows detailed help for a command.\n\n");
            printf("Examples:\n");
            printf("  HELP          -> Lists all commands\n");
            printf("  HELP PRINT    -> Detailed help on PRINT\n\n");
        }
        else if (strcmp(cmdName, "EXIT") == 0) {
            printf("=== EXIT ===\n\n");
            printf("Syntax: EXIT\n\n");
            printf("Description:\n");
            printf("  Quits the BASIC80 interpreter.\n");
            printf("  The program in memory is lost if not saved.\n\n");
            printf("Note:\n");
            printf("  Use SAVE before EXIT to keep your program.\n\n");
        }
        else if (strcmp(cmdName, "NEXT") == 0) {
            printf("=== NEXT ===\n\n");
            printf("See HELP FOR for information on FOR/NEXT loops.\n\n");
        }
        else if (strcmp(cmdName, "THEN") == 0 || strcmp(cmdName, "ELSE") == 0) {
            printf("=== THEN / ELSE ===\n\n");
            printf("See HELP IF for information on IF/THEN/ELSE.\n\n");
        }
        else if (strcmp(cmdName, "TO") == 0 || strcmp(cmdName, "STEP") == 0) {
            printf("=== TO / STEP ===\n\n");
            printf("See HELP FOR for information on TO and STEP in loops.\n\n");
        }
        /* ===== MATH FUNCTIONS ===== */
        else if (strcmp(cmdName, "ABS") == 0) {
            printf("=== ABS ===\n\n");
            printf("Syntax: ABS(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the absolute value of a number.\n\n");
            printf("Examples:\n");
            printf("  PRINT ABS(-5)       -> Outputs: 5.00\n");
            printf("  PRINT ABS(3.14)     -> Outputs: 3.14\n");
            printf("  X = ABS(A - B)      -> Distance between A and B\n\n");
        }
        else if (strcmp(cmdName, "SQR") == 0) {
            printf("=== SQR ===\n\n");
            printf("Syntax: SQR(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the square root of a number.\n");
            printf("  The number must be non-negative.\n\n");
            printf("Examples:\n");
            printf("  PRINT SQR(16)       -> Outputs: 4.00\n");
            printf("  PRINT SQR(2)        -> Outputs: 1.41\n");
            printf("  H = SQR(A*A + B*B)  -> Hypotenuse (Pythagoras)\n\n");
        }
        else if (strcmp(cmdName, "INT") == 0) {
            printf("=== INT ===\n\n");
            printf("Syntax: INT(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the integer part of a number (rounds down).\n\n");
            printf("Examples:\n");
            printf("  PRINT INT(3.7)      -> Outputs: 3.00\n");
            printf("  PRINT INT(-2.3)     -> Outputs: -3.00\n");
            printf("  N = INT(X / 10)     -> Tens digit of X\n\n");
        }
        else if (strcmp(cmdName, "SGN") == 0) {
            printf("=== SGN ===\n\n");
            printf("Syntax: SGN(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the sign of a number:\n");
            printf("    -1 if negative\n");
            printf("     0 if zero\n");
            printf("    +1 if positive\n\n");
            printf("Examples:\n");
            printf("  PRINT SGN(-5)       -> Outputs: -1.00\n");
            printf("  PRINT SGN(0)        -> Outputs: 0.00\n");
            printf("  PRINT SGN(42)       -> Outputs: 1.00\n\n");
        }
        else if (strcmp(cmdName, "RND") == 0) {
            printf("=== RND ===\n\n");
            printf("Syntax: RND\n\n");
            printf("Description:\n");
            printf("  Returns a random number between 0 (inclusive) and 1 (exclusive).\n\n");
            printf("Examples:\n");
            printf("  PRINT RND           -> Outputs: 0.73 (example)\n");
            printf("  X = INT(RND * 6) + 1 -> Roll a die (1 to 6)\n");
            printf("  Y = RND * 100       -> Number between 0 and 100\n\n");
        }
        else if (strcmp(cmdName, "SIN") == 0) {
            printf("=== SIN ===\n\n");
            printf("Syntax: SIN(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the sine of an angle in radians.\n\n");
            printf("Examples:\n");
            printf("  PRINT SIN(0)        -> Outputs: 0.00\n");
            printf("  PRINT SIN(3.14159/2) -> Outputs: 1.00\n");
            printf("  Y = SIN(A * 3.14159 / 180) -> Convert degrees to radians\n\n");
        }
        else if (strcmp(cmdName, "COS") == 0) {
            printf("=== COS ===\n\n");
            printf("Syntax: COS(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the cosine of an angle in radians.\n\n");
            printf("Examples:\n");
            printf("  PRINT COS(0)        -> Outputs: 1.00\n");
            printf("  PRINT COS(3.14159)  -> Outputs: -1.00\n");
            printf("  X = COS(A * 3.14159 / 180) -> Convert degrees to radians\n\n");
        }
        else if (strcmp(cmdName, "TAN") == 0) {
            printf("=== TAN ===\n\n");
            printf("Syntax: TAN(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the tangent of an angle in radians.\n\n");
            printf("Examples:\n");
            printf("  PRINT TAN(0)        -> Outputs: 0.00\n");
            printf("  PRINT TAN(3.14159/4) -> Outputs: 1.00\n\n");
        }
        else if (strcmp(cmdName, "ATN") == 0 || strcmp(cmdName, "ATAN") == 0) {
            printf("=== ATN / ATAN ===\n\n");
            printf("Syntax: ATN(expression)\n");
            printf("        ATAN(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the arctangent (in radians) of a number.\n");
            printf("  Result is between -PI/2 and PI/2.\n");
            printf("  ATN and ATAN are identical.\n\n");
            printf("Examples:\n");
            printf("  PRINT ATN(1)        -> Outputs: 0.79 (PI/4)\n");
            printf("  PRINT ATAN(1)       -> Outputs: 0.79 (PI/4)\n");
            printf("  A = ATN(Y/X) * 180 / 3.14159 -> Angle in degrees\n\n");
        }
        else if (strcmp(cmdName, "ASIN") == 0) {
            printf("=== ASIN ===\n\n");
            printf("Syntax: ASIN(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the arcsine (in radians) of a number.\n");
            printf("  The argument must be between -1 and 1.\n");
            printf("  Result is between -PI/2 and PI/2.\n\n");
            printf("Examples:\n");
            printf("  PRINT ASIN(0)       -> Outputs: 0.00\n");
            printf("  PRINT ASIN(1)       -> Outputs: 1.57 (PI/2)\n");
            printf("  A = ASIN(0.5) * 180 / 3.14159 -> 30 degrees\n\n");
        }
        else if (strcmp(cmdName, "ACOS") == 0) {
            printf("=== ACOS ===\n\n");
            printf("Syntax: ACOS(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the arccosine (in radians) of a number.\n");
            printf("  The argument must be between -1 and 1.\n");
            printf("  Result is between 0 and PI.\n\n");
            printf("Examples:\n");
            printf("  PRINT ACOS(1)       -> Outputs: 0.00\n");
            printf("  PRINT ACOS(0)       -> Outputs: 1.57 (PI/2)\n");
            printf("  A = ACOS(-1) * 180 / 3.14159 -> 180 degrees\n\n");
        }
        else if (strcmp(cmdName, "EXP") == 0) {
            printf("=== EXP ===\n\n");
            printf("Syntax: EXP(expression)\n\n");
            printf("Description:\n");
            printf("  Returns e^x (exponential of x).\n");
            printf("  e is the base of natural logarithms (approximately 2.718).\n\n");
            printf("Examples:\n");
            printf("  PRINT EXP(0)        -> Outputs: 1.00\n");
            printf("  PRINT EXP(1)        -> Outputs: 2.72\n");
            printf("  Y = EXP(X)          -> Exponential growth\n\n");
        }
        else if (strcmp(cmdName, "LOG") == 0) {
            printf("=== LOG ===\n\n");
            printf("Syntax: LOG(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the natural logarithm (base e) of a number.\n");
            printf("  The number must be strictly positive.\n\n");
            printf("Examples:\n");
            printf("  PRINT LOG(2.718)    -> Outputs: 1.00\n");
            printf("  PRINT LOG(1)        -> Outputs: 0.00\n");
            printf("  Y = LOG(X)          -> Inverse function of EXP\n\n");
        }
        else if (strcmp(cmdName, "LOG10") == 0) {
            printf("=== LOG10 ===\n\n");
            printf("Syntax: LOG10(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the base-10 logarithm of a number.\n");
            printf("  The number must be strictly positive.\n\n");
            printf("Examples:\n");
            printf("  PRINT LOG10(100)    -> Outputs: 2.00\n");
            printf("  PRINT LOG10(1000)   -> Outputs: 3.00\n");
            printf("  D = LOG10(X)        -> Number of digits - 1\n\n");
        }
        else if (strcmp(cmdName, "POW") == 0) {
            printf("=== POW ===\n\n");
            printf("Syntax: POW(base, exponent)\n\n");
            printf("Description:\n");
            printf("  Returns base raised to the power exponent (base^exponent).\n\n");
            printf("Examples:\n");
            printf("  PRINT POW(2, 10)    -> Outputs: 1024.00\n");
            printf("  PRINT POW(3, 3)     -> Outputs: 27.00\n");
            printf("  Y = POW(X, 0.5)     -> Square root of X\n\n");
        }
        else if (strcmp(cmdName, "DEG") == 0) {
            printf("=== DEG ===\n\n");
            printf("Syntax: DEG(expression)\n\n");
            printf("Description:\n");
            printf("  Converts an angle from radians to degrees.\n\n");
            printf("Examples:\n");
            printf("  PRINT DEG(3.14159)  -> Outputs: 180.00\n");
            printf("  PRINT DEG(1.5708)   -> Outputs: 90.00\n");
            printf("  A = DEG(ATN(1))     -> Outputs: 45.00\n\n");
        }
        else if (strcmp(cmdName, "RAD") == 0) {
            printf("=== RAD ===\n\n");
            printf("Syntax: RAD(expression)\n\n");
            printf("Description:\n");
            printf("  Converts an angle from degrees to radians.\n\n");
            printf("Examples:\n");
            printf("  PRINT RAD(180)      -> Outputs: 3.14\n");
            printf("  PRINT RAD(90)       -> Outputs: 1.57\n");
            printf("  X = SIN(RAD(30))    -> Sine of 30 degrees\n\n");
        }
        else if (strcmp(cmdName, "SINH") == 0) {
            printf("=== SINH ===\n\n");
            printf("Syntax: SINH(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the hyperbolic sine of a number.\n\n");
            printf("Example:\n");
            printf("  PRINT SINH(0)       -> Outputs: 0.00\n\n");
        }
        else if (strcmp(cmdName, "COSH") == 0) {
            printf("=== COSH ===\n\n");
            printf("Syntax: COSH(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the hyperbolic cosine of a number.\n\n");
            printf("Example:\n");
            printf("  PRINT COSH(0)       -> Outputs: 1.00\n\n");
        }
        else if (strcmp(cmdName, "TANH") == 0) {
            printf("=== TANH ===\n\n");
            printf("Syntax: TANH(expression)\n\n");
            printf("Description:\n");
            printf("  Returns the hyperbolic tangent of a number.\n\n");
            printf("Example:\n");
            printf("  PRINT TANH(0)       -> Outputs: 0.00\n\n");
        }
        /* ===== STRING FUNCTIONS ===== */
        else if (strcmp(cmdName, "LEN") == 0) {
            printf("=== LEN ===\n\n");
            printf("Syntax: LEN(string$)\n\n");
            printf("Description:\n");
            printf("  Returns the length (number of characters) of a string.\n\n");
            printf("Examples:\n");
            printf("  PRINT LEN(\"Hello\")  -> Outputs: 5.00\n");
            printf("  A$ = \"Test\"\n");
            printf("  PRINT LEN(A$)       -> Outputs: 4.00\n");
            printf("  IF LEN(X$) > 10 THEN PRINT \"Too long\"\n\n");
        }
        else if (strcmp(cmdName, "LEFT$") == 0 || strcmp(cmdName, "LEFT") == 0) {
            printf("=== LEFT$ ===\n\n");
            printf("Syntax: LEFT$(string$, n)\n\n");
            printf("Description:\n");
            printf("  Returns the first n characters of a string.\n\n");
            printf("Examples:\n");
            printf("  PRINT LEFT$(\"Hello\", 3)   -> Outputs: Hel\n");
            printf("  A$ = \"World\"\n");
            printf("  B$ = LEFT$(A$, 3)         -> B$ = \"Wor\"\n");
            printf("  IF LEFT$(X$, 1) = \"A\" THEN PRINT \"Starts with A\"\n\n");
        }
        else if (strcmp(cmdName, "RIGHT$") == 0 || strcmp(cmdName, "RIGHT") == 0) {
            printf("=== RIGHT$ ===\n\n");
            printf("Syntax: RIGHT$(string$, n)\n\n");
            printf("Description:\n");
            printf("  Returns the last n characters of a string.\n\n");
            printf("Examples:\n");
            printf("  PRINT RIGHT$(\"Hello\", 2)  -> Outputs: lo\n");
            printf("  A$ = \"World\"\n");
            printf("  B$ = RIGHT$(A$, 4)        -> B$ = \"orld\"\n");
            printf("  X$ = RIGHT$(\"00\" + STR$(N), 2) -> Format 2 digits\n\n");
        }
        else if (strcmp(cmdName, "MID$") == 0 || strcmp(cmdName, "MID") == 0) {
            printf("=== MID$ ===\n\n");
            printf("Syntax: MID$(string$, start, length)\n\n");
            printf("Description:\n");
            printf("  Extracts a substring starting at a given position.\n");
            printf("  Positions start at 1.\n\n");
            printf("Examples:\n");
            printf("  PRINT MID$(\"Hello\", 2, 3) -> Outputs: ell\n");
            printf("  A$ = \"World\"\n");
            printf("  B$ = MID$(A$, 2, 3)       -> B$ = \"orl\"\n\n");
        }
        else if (strcmp(cmdName, "CHR$") == 0 || strcmp(cmdName, "CHR") == 0) {
            printf("=== CHR$ ===\n\n");
            printf("Syntax: CHR$(code)\n\n");
            printf("Description:\n");
            printf("  Returns the character corresponding to an ASCII code.\n\n");
            printf("Examples:\n");
            printf("  PRINT CHR$(65)      -> Outputs: A\n");
            printf("  PRINT CHR$(72); CHR$(105) -> Outputs: Hi\n");
            printf("  A$ = CHR$(13)       -> Carriage return\n\n");
        }
        else if (strcmp(cmdName, "ASC") == 0) {
            printf("=== ASC ===\n\n");
            printf("Syntax: ASC(string$)\n\n");
            printf("Description:\n");
            printf("  Returns the ASCII code of the first character of a string.\n\n");
            printf("Examples:\n");
            printf("  PRINT ASC(\"A\")      -> Outputs: 65.00\n");
            printf("  PRINT ASC(\"Hello\")  -> Outputs: 72.00 (H)\n");
            printf("  C = ASC(A$)         -> Code of the first character\n\n");
        }
        else if (strcmp(cmdName, "STR$") == 0 || strcmp(cmdName, "STR") == 0) {
            printf("=== STR$ ===\n\n");
            printf("Syntax: STR$(expression)\n\n");
            printf("Description:\n");
            printf("  Converts a number to a string.\n\n");
            printf("Examples:\n");
            printf("  A$ = STR$(42)       -> A$ = \"42\"\n");
            printf("  PRINT \"X=\" + STR$(X) -> Outputs: X=10\n");
            printf("  N$ = STR$(123.45)   -> N$ = \"123.45\"\n\n");
        }
        else if (strcmp(cmdName, "VAL") == 0) {
            printf("=== VAL ===\n\n");
            printf("Syntax: VAL(string$)\n\n");
            printf("Description:\n");
            printf("  Converts a string to a number.\n");
            printf("  Stops at the first non-numeric character.\n\n");
            printf("Examples:\n");
            printf("  PRINT VAL(\"42\")     -> Outputs: 42.00\n");
            printf("  PRINT VAL(\"3.14\")   -> Outputs: 3.14\n");
            printf("  X = VAL(\"123ABC\")   -> X = 123.00\n");
            printf("  Y = VAL(\"ABC\")      -> Y = 0.00\n\n");
        }
        else if (strcmp(cmdName, "SPACE$") == 0 || strcmp(cmdName, "SPACE") == 0) {
            printf("=== SPACE$ ===\n\n");
            printf("Syntax: SPACE$(n)\n\n");
            printf("Description:\n");
            printf("  Returns a string of n space characters.\n\n");
            printf("Examples:\n");
            printf("  PRINT SPACE$(5)     -> Outputs: \"     \" (5 spaces)\n");
            printf("  PRINT \"A\" + SPACE$(3) + \"B\" -> Outputs: A   B\n");
            printf("  A$ = SPACE$(10)     -> A$ = 10 spaces\n\n");
        }
        else if (strcmp(cmdName, "STRING$") == 0 || strcmp(cmdName, "STRING") == 0) {
            printf("=== STRING$ ===\n\n");
            printf("Syntax: STRING$(n, code)\n");
            printf("        STRING$(n, string$)\n\n");
            printf("Description:\n");
            printf("  Returns a string of n copies of a character.\n");
            printf("  The character is specified by its ASCII code or the first\n");
            printf("  character of a string.\n\n");
            printf("Examples:\n");
            printf("  PRINT STRING$(5, 42)    -> Outputs: *****\n");
            printf("  PRINT STRING$(3, \"AB\") -> Outputs: AAA\n");
            printf("  SEP$ = STRING$(40, 45)  -> 40 hyphens\n\n");
        }
        else if (strcmp(cmdName, "CLS") == 0) {
            printf("=== CLS ===\n\n");
            printf("Syntax: CLS\n\n");
            printf("Description:\n");
            printf("  Clears the screen and moves the cursor to the top-left corner.\n\n");
            printf("Examples:\n");
            printf("  CLS                 -> Clears the screen\n");
            printf("  CLS : PRINT \"Menu\" -> Clear then display\n\n");
        }
        else {
            printf("Unknown command or function: %s\n", cmdName);
            printf("Type HELP to see the list of available commands and functions.\n");
        }
    } else {
        printf("Usage: HELP [command]\n");
    }
}

/* ===== CLS COMMAND ===== */

void handleCls(Interpreter *interp, Token *tokens) {
    (void)interp;
    (void)tokens;
#ifdef _WIN32
    system("cls");
#else
    printf("\033[2J\033[H");
    fflush(stdout);
#endif
}
