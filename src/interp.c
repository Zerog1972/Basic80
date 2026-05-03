/*
 * interpreter.c - Core interpreter for the Basic80 BASIC interpreter
 *
 * Manages the program line list, the variable table, the FOR/NEXT loop
 * stack, the GOSUB/RETURN call stack, and the DATA/READ/RESTORE pointer.
 * Also provides the extension hook registry (custom numeric functions,
 * string functions, and commands) used by embedding host applications.
 *
 * Main entry points:
 *   createInterpreter()  - allocate and initialise an interpreter instance
 *   runProgram()         - execute the loaded program
 *   executeCommand()     - execute a single BASIC statement directly
 *   freeInterpreter()    - release all resources
 */
#include "interp.h"
#include "ctrlflow.h"
#include "commands.h"

/* ===== INTERPRETER ===== */

Interpreter* createInterpreter(void) {
    Interpreter *interp = malloc(sizeof(Interpreter));
    if (!interp) {
        fprintf(stderr, "Error: Memory allocation failed for interpreter\n");
        return NULL;
    }
    interp->program = NULL;
    interp->variables = NULL;
    interp->currentLine = NULL;
    interp->forStack = NULL;
    interp->callStack = NULL;
    interp->dataList = NULL;
    interp->dataListTail = NULL;
    interp->dataPointer = NULL;
    interp->hasError = 0;
    interp->lastErrorType = ERR_NONE;
    interp->errorColumn = -1;
    interp->errorContext[0] = '\0';
    interp->customNumFuncs = NULL;
    interp->customStrFuncs = NULL;
    interp->customCommands = NULL;
    return interp;
}

const char* getErrorTypeName(ErrorType type) {
    switch (type) {
        case ERR_SYNTAX:         return "Syntax";
        case ERR_RUNTIME:        return "Runtime";
        case ERR_TYPE_MISMATCH:  return "Type";
        case ERR_OUT_OF_DATA:    return "Data";
        case ERR_DIVISION_ZERO:  return "DivisionByZero";
        case ERR_UNDEFINED_VAR:  return "Variable";
        case ERR_ARRAY_BOUNDS:   return "Array";
        case ERR_OUT_OF_MEMORY:  return "Memory";
        default:                 return "Unknown";
    }
}

void reportError(Interpreter *interp, const char *message) {
    reportErrorEx(interp, ERR_RUNTIME, -1, message);
}

void reportErrorEx(Interpreter *interp, ErrorType type, int column, const char *message) {
    interp->hasError = 1;
    interp->lastErrorType = type;
    interp->errorColumn = column;
    
    /* Formatted error output */
    if (interp->currentLine) {
        printf("\n[ERROR %s] Line %d", getErrorTypeName(type), interp->currentLine->lineNum);
        if (column >= 0) {
            printf(", column %d", column);
        }
        printf(": %s\n", message);
        
        /* Print the source line */
        if (interp->currentLine->code) {
            printf("  --> %s\n", interp->currentLine->code);
            
            /* Print a caret cursor if the column is known */
            if (column >= 0) {
                int i;
                printf("      ");
                for (i = 0; i < column; i++) {
                    printf(" ");
                }
                printf("^\n");
            }
        }
    } else {
        printf("\n[ERROR %s]: %s\n", getErrorTypeName(type), message);
    }
}

void freeInterpreter(Interpreter *interp) {
    Line *line;
    Line *next;
    Variable *var;
    Variable *nextVar;
    ForLoop *forLoop;
    ForLoop *nextFor;
    CallStack *callFrame;
    CallStack *nextCall;
    
    line = interp->program;
    while (line) {
        next = line->next;
        free(line->code);
        free(line);
        line = next;
    }
    
    var = interp->variables;
    while (var) {
        nextVar = var->next;
        free(var->name);
        if (var->isArray && var->arrayValues) {
            free(var->arrayValues);
        }
        if (var->isArray && var->dimensions) {
            free(var->dimensions);
        }
        if (var->isString && var->strValue) {
            free(var->strValue);
        }
        free(var);
        var = nextVar;
    }
    
    forLoop = interp->forStack;
    while (forLoop) {
        nextFor = forLoop->next;
        free(forLoop->varName);
        free(forLoop);
        forLoop = nextFor;
    }
    
    callFrame = interp->callStack;
    while (callFrame) {
        nextCall = callFrame->next;
        free(callFrame);
        callFrame = nextCall;
    }
    
    /* Liberate DATA list */
    {
        DataItem *dataItem = interp->dataList;
        DataItem *nextData;
        while (dataItem) {
            nextData = dataItem->next;
            free(dataItem->value);
            free(dataItem);
            dataItem = nextData;
        }
    }
    
    /* Free custom extension hooks */
    {
        CustomNumFunc *numFunc = interp->customNumFuncs;
        CustomNumFunc *nextNum;
        CustomStrFunc *strFunc = interp->customStrFuncs;
        CustomStrFunc *nextStr;
        CustomCommand *cmd = interp->customCommands;
        CustomCommand *nextCmd;
        
        while (numFunc) {
            nextNum = numFunc->next;
            free(numFunc->name);
            free(numFunc);
            numFunc = nextNum;
        }
        
        while (strFunc) {
            nextStr = strFunc->next;
            free(strFunc->name);
            free(strFunc);
            strFunc = nextStr;
        }
        
        while (cmd) {
            nextCmd = cmd->next;
            free(cmd->name);
            free(cmd);
            cmd = nextCmd;
        }
    }
    
    free(interp);
}

void addLine(Interpreter *interp, int lineNum, const char *code) {
    Line *newLine;
    Line *current;
    size_t codeLen = strlen(code);  /* Calculé une seule fois */
    
    newLine = malloc(sizeof(Line));
    if (!newLine) return;
    newLine->lineNum = lineNum;
    newLine->code = malloc(codeLen + 1);
    if (!newLine->code) {
        free(newLine);
        return;
    }
    strcpy(newLine->code, code);
    newLine->next = NULL;
    
    if (!interp->program || interp->program->lineNum > lineNum) {
        newLine->next = interp->program;
        interp->program = newLine;
    } else if (interp->program->lineNum == lineNum) {
        /* Remplacer la première ligne */
        free(interp->program->code);
        interp->program->code = malloc(codeLen + 1);
        strcpy(interp->program->code, code);
        free(newLine->code);
        free(newLine);
    } else {
        current = interp->program;
        while (current->next && current->next->lineNum < lineNum) {
            current = current->next;
        }
        
        if (current->next && current->next->lineNum == lineNum) {
            /* Replace an existing line with the same number */
            free(current->next->code);
            current->next->code = malloc(codeLen + 1);
            strcpy(current->next->code, code);
            free(newLine->code);
            free(newLine);
        } else {
            /* Insert a new line into the sorted list */
            newLine->next = current->next;
            current->next = newLine;
        }
    }
}

/* Delete a line from the program by its line number */
void deleteLine(Interpreter *interp, int lineNum) {
    Line *current;
    Line *toDelete;
    
    if (!interp->program) {
        return; /* Program is empty */
    }
    
    /* Check if the line to delete is the first one */
    if (interp->program->lineNum == lineNum) {
        toDelete = interp->program;
        interp->program = interp->program->next;
        free(toDelete->code);
        free(toDelete);
        return;
    }
    
    /* Search for the line to delete */
    current = interp->program;
    while (current->next && current->next->lineNum != lineNum) {
        current = current->next;
    }
    
    /* If found, unlink and free it */
    if (current->next && current->next->lineNum == lineNum) {
        toDelete = current->next;
        current->next = toDelete->next;
        free(toDelete->code);
        free(toDelete);
    }
}

/* Execute a single statement (no ':' separator processing) */
static void executeSingleStatement(Interpreter *interp, const char *stmt) {
    Token *tokens;
    
    tokens = tokenize(stmt);
    
    if (tokens[0].type == TOK_EOF || tokens[0].type == TOK_REM) {
        /* Empty statement or comment: nothing to do */
        freeTokens(tokens);
        return;
    }
    
    if (tokens[0].type == TOK_PRINT) {
        handlePrint(interp, tokens);
    }
    else if (tokens[0].type == TOK_LET) {
        handleLet(interp, tokens);
    }
    else if (tokens[0].type == TOK_DIM) {
        handleDim(interp, tokens);
    }
    else if (tokens[0].type == TOK_INPUT) {
        handleInput(interp, tokens);
    }
    else if (tokens[0].type == TOK_READ) {
        handleRead(interp, tokens);
    }
    else if (tokens[0].type == TOK_DATA) {
        /* DATA cannot be executed in direct mode; only valid inside a running program */
        printf("Error: DATA can only be used inside a program.\n");
    }
    else if (tokens[0].type == TOK_RESTORE) {
        handleRestore(interp, tokens);
    }
    else if (tokens[0].type == TOK_HELP) {
        handleHelp(interp, tokens);
    }
    else if (tokens[0].type == TOK_FOR) {
        /* FOR loops are only valid inside a running program */
        printf("Error: FOR can only be used inside a program.\n");
    }
    else if (tokens[0].type == TOK_NEXT) {
        /* NEXT is only valid inside a running program */
        printf("Error: NEXT can only be used inside a program.\n");
    }
    else if (tokens[0].type == TOK_IDENTIFIER) {
        /* Implicit assignment without LET: e.g. V = 5 or V(1) = 5 */
        handleLet(interp, tokens);
    }
    else if (tokens[0].type == TOK_IF || tokens[0].type == TOK_GOTO || 
             tokens[0].type == TOK_GOSUB || tokens[0].type == TOK_RETURN ||
             tokens[0].type == TOK_END) {
        printf("Error: %s can only be used inside a program.\n", tokens[0].value);
    }
    else {
        /* Vérifier si c'est une commande personnalisée */
        CustomCommandHandler customCmd = findCustomCommand(interp, tokens[0].value);
        if (customCmd) {
            customCmd(interp, tokens);
        } else {
            /* Unknown command */
            printf("Error: Unknown command '%s'.\n", 
                   tokens[0].value ? tokens[0].value : "");
        }
    }
    
    freeTokens(tokens);
}

/* Free the array of statement strings produced by splitByColon() */
static void freeSplitArray(char **parts, int count) {
    int i;
    for (i = 0; i < count; i++) {
        free(parts[i]);
    }
    free(parts);
}

/* Split a source line by ':' while ignoring colons inside strings or after REM */
static char** splitByColon(const char *line, int *count) {
    char **parts;
    char *lineCopy;
    char *start;
    char *p;
    int capacity;
    int isInRem;
    int isInString;
    int afterThen;  /* Once THEN is found, stop splitting on ':' */
    
    *count = 0;
    capacity = 4;  /* La plupart des lignes BASIC ont <= 4 instructions */
    parts = malloc(sizeof(char*) * capacity);
    if (!parts) return NULL;
    
    lineCopy = malloc(strlen(line) + 1);
    if (!lineCopy) {
        free(parts);
        return NULL;
    }
    strcpy(lineCopy, line);
    
    start = lineCopy;
    p = lineCopy;
    isInRem = 0;
    isInString = 0;
    afterThen = 0;
    
    /* Check if the line starts with a REM statement */
    while (*p && isspace(*p)) p++;
    if (strncmp(p, "REM", 3) == 0 && (p[3] == ' ' || p[3] == '\0')) {
        isInRem = 1;
    }
    
    p = start;
    while (*p) {
        /* Track whether we are inside a quoted string */
        if (*p == '"' && !isInRem) {
            isInString = !isInString;
            p++;
            continue;
        }

        /* Detect THEN keyword: once seen, all remaining text on this line
         * belongs to the IF clause and must NOT be split on ':'. */
        if (!isInString && !isInRem && !afterThen && *p == 'T') {
            if ((p == lineCopy || *(p-1) == ' ' || *(p-1) == '\t') &&
                strncmp(p, "THEN", 4) == 0 &&
                (p[4] == ' ' || p[4] == '\t' || p[4] == '\0')) {
                afterThen = 1;
            }
        }
        
        if (*p == ':' && !isInRem && !isInString && !afterThen) {
            /* Found a ':' outside a string and not after REM/THEN: split here */
            *p = '\0';
            if (*count >= capacity) {
                char **newParts;
                capacity *= 2;
                newParts = realloc(parts, sizeof(char*) * capacity);
                if (!newParts) {
                    free(lineCopy);
                    freeSplitArray(parts, *count);
                    return NULL;
                }
                parts = newParts;
            }
            parts[*count] = malloc(strlen(start) + 1);
            if (!parts[*count]) {
                free(lineCopy);
                freeSplitArray(parts, *count);
                return NULL;
            }
            strcpy(parts[*count], start);
            (*count)++;
            p++;
            start = p;
            afterThen = 0;  /* Reset for the next statement */
            
            /* Vérifier si la prochaine instruction est REM */
            while (*p && isspace(*p)) p++;
            if (strncmp(p, "REM", 3) == 0 && (p[3] == ' ' || p[3] == '\0')) {
                isInRem = 1;
            }
        } else {
            p++;
        }
    }
    
    /* Store the last (or only) segment */
    if (*count >= capacity) {
        char **newParts;
        capacity++;
        newParts = realloc(parts, sizeof(char*) * capacity);
        if (!newParts) {
            free(lineCopy);
            freeSplitArray(parts, *count);
            return NULL;
        }
        parts = newParts;
    }
    parts[*count] = malloc(strlen(start) + 1);
    if (!parts[*count]) {
        free(lineCopy);
        freeSplitArray(parts, *count);
        return NULL;
    }
    strcpy(parts[*count], start);
    (*count)++;
    
    free(lineCopy);
    return parts;
}

/* Execute a BASIC line, handling ':'-separated multiple statements */
void executeCommand(Interpreter *interp, const char *line) {
    char **parts;
    int count;
    int i;
    
    parts = splitByColon(line, &count);
    
    for (i = 0; i < count; i++) {
        /* Skip leading whitespace before each statement */
        char *stmt = parts[i];
        while (*stmt && isspace(*stmt)) stmt++;
        
        if (*stmt) {
            executeSingleStatement(interp, stmt);
        }
    }
    
    freeSplitArray(parts, count);
}

/* Execute one statement inside runProgram(), handling control-flow tokens.
 * Returns: -1 = END encountered, 1 = flow changed (GOTO/IF/FOR/etc.), 0 = normal */
static int executeStatementInProgram(Interpreter *interp, const char *stmt, Line **line) {
    Token *tokens;
    
    tokens = tokenize(stmt);
    
    if (tokens[0].type == TOK_DATA) {
        /* DATA items are collected in the first pass; skip here */
        freeTokens(tokens);
        return 0;
    }
    else if (tokens[0].type == TOK_END) {
        freeTokens(tokens);
        return -1; /* Signal pour terminer le programme */
    }
    else if (tokens[0].type == TOK_IF) {
        if (handleIfStatement(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else if (tokens[0].type == TOK_GOTO) {
        if (handleGoto(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else if (tokens[0].type == TOK_GOSUB) {
        if (handleGosub(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else if (tokens[0].type == TOK_RETURN) {
        if (handleReturn(interp, line)) {
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else if (tokens[0].type == TOK_FOR) {
        if (handleFor(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else if (tokens[0].type == TOK_NEXT) {
        /* Pass the variable name (if any) so handleNext can discard orphaned
         * inner loops left by a GOTO that jumped out of a nested FOR. */
        const char *nextVar = (tokens[1].type == TOK_IDENTIFIER)
                              ? tokens[1].value : NULL;
        if (handleNext(interp, nextVar)) {
            /* Jump back to the line immediately after the matching FOR */
            *line = interp->forStack->startLine->next;
            freeTokens(tokens);
            return 1; /* Control flow handled */
        }
    }
    else {
        executeSingleStatement(interp, stmt);
    }
    
    freeTokens(tokens);
    return 0; /* Pas de contrôle de flux */
}

void runProgram(Interpreter *interp) {
    Line *line;
    char **parts;
    int count;
    int i;
    int result;
    Token *tokens;
    
    interp->hasError = 0;
    
    /* First pass: collect all DATA items before any execution */
    line = interp->program;
    while (line) {
        /* Optimisation : ignorer les lignes sans le mot-clé DATA */
        if (!strstr(line->code, "DATA")) {
            line = line->next;
            continue;
        }
        
        /* Diviser la ligne par ':' en ignorant ceux dans REM */
        parts = splitByColon(line->code, &count);
        
        for (i = 0; i < count; i++) {
            /* Ignorer les espaces au début */
            char *stmt = parts[i];
            while (*stmt && isspace(*stmt)) stmt++;
            
            if (*stmt) {
                tokens = tokenize(stmt);
                if (tokens[0].type == TOK_DATA) {
                    handleData(interp, tokens, line->lineNum);
                }
                freeTokens(tokens);
            }
        }
        
        freeSplitArray(parts, count);
        line = line->next;
    }
    
    /* Second pass: execute the program */
    line = interp->program;
    while (line) {
        interp->currentLine = line;
        
        /* Split each source line by ':' to handle multiple statements per line */
        parts = splitByColon(line->code, &count);
        
        for (i = 0; i < count && !interp->hasError; i++) {
            /* Skip leading whitespace */
            char *stmt = parts[i];
            while (*stmt && isspace(*stmt)) stmt++;
            
            if (*stmt) {
                result = executeStatementInProgram(interp, stmt, &line);
                if (result == -1) {
                    /* END statement encountered: stop execution */
                    freeSplitArray(parts, count);
                    interp->currentLine = NULL;
                    return;
                }
                if (result == 1) {
                    /* Control flow changed (GOTO, IF, FOR, etc.): restart at new line */
                    freeSplitArray(parts, count);
                    if (interp->hasError) {
                        interp->currentLine = NULL;
                        return;
                    }
                    goto next_line;
                }
            }
        }
        
        freeSplitArray(parts, count);
        if (interp->hasError) break;
        line = line->next;
        
        next_line:;
    }
    interp->currentLine = NULL;
}

/* List all lines of the loaded program to stdout */
void listProgram(Interpreter *interp) {
    Line *line = interp->program;
    while (line) {
        printf("%d %s\n", line->lineNum, line->code);
        line = line->next;
    }
}

/* Remove all lines from the program (variables are preserved) */
void clearProgram(Interpreter *interp) {
    Line *line = interp->program;
    Line *next;
    
    while (line) {
        next = line->next;
        free(line->code);
        free(line);
        line = next;
    }
    
    interp->program = NULL;
}

/* Save the current program to a text file (one numbered line per row) */
int saveProgram(Interpreter *interp, const char *filename) {
    FILE *file;
    Line *line;
    
    file = fopen(filename, "w");
    if (!file) {
        printf("Error: Cannot open file '%s' for writing.\n", filename);
        return 0;
    }
    
    line = interp->program;
    while (line) {
        fprintf(file, "%d %s\n", line->lineNum, line->code);
        line = line->next;
    }
    
    fclose(file);
    return 1;
}

/* Load a program from a text file, replacing the current program */
int loadProgram(Interpreter *interp, const char *filename) {
    FILE *file;
    char line[1024];
    int lineNum;
    char *code;
    char *p;
    
    file = fopen(filename, "r");
    if (!file) {
        printf("Error: Cannot open file '%s' for reading.\n", filename);
        return 0;
    }
    
    /* Clear the current program before loading */
    clearProgram(interp);
    
    while (fgets(line, sizeof(line), file)) {
        /* Strip the trailing newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Skip blank lines */
        if (strlen(line) == 0) continue;
        
        /* Parse the leading line number */
        lineNum = atoi(line);
        if (lineNum <= 0) {
            printf("Warning: Invalid line ignored: %s\n", line);
            continue;
        }
        
        /* Skip past the line number and any separating whitespace */
        p = line;
        while (*p && isdigit(*p)) p++;
        while (*p && isspace(*p)) p++;
        
        code = p;
        if (strlen(code) > 0) {
            addLine(interp, lineNum, code);
        }
    }
    
    fclose(file);
    return 1;
}

/* ========================================================================
 * EXTENSION HOOK REGISTRY (custom numeric functions, string functions,
 * and commands callable from BASIC code)
 * ======================================================================== */

int registerCustomNumericFunction(Interpreter *interp, const char *name, CustomNumericFunction handler) {
    CustomNumFunc *newFunc;
    char *nameCopy;
    size_t i;
    size_t nameLen;
    
    if (!interp || !name || !handler) return 0;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Allocate the registry entry */
    newFunc = (CustomNumFunc*)malloc(sizeof(CustomNumFunc));
    if (!newFunc) return 0;
    
    /* Duplicate and uppercase the function name */
    nameCopy = (char*)malloc(nameLen + 1);
    if (!nameCopy) {
        free(newFunc);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < nameLen; i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialise and prepend to the list */
    newFunc->name = nameCopy;
    newFunc->handler = handler;
    newFunc->next = interp->customNumFuncs;
    
    /* Prepend to the list */
    interp->customNumFuncs = newFunc;
    
    return 1;
}

int registerCustomStringFunction(Interpreter *interp, const char *name, CustomStringFunction handler) {
    CustomStrFunc *newFunc;
    char *nameCopy;
    size_t i;
    size_t nameLen;
    
    if (!interp || !name || !handler) return 0;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Allocate the registry entry */
    newFunc = (CustomStrFunc*)malloc(sizeof(CustomStrFunc));
    if (!newFunc) return 0;
    
    /* Duplicate and uppercase the function name */
    nameCopy = (char*)malloc(nameLen + 1);
    if (!nameCopy) {
        free(newFunc);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < nameLen; i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialise and prepend to the list */
    newFunc->name = nameCopy;
    newFunc->handler = handler;
    newFunc->next = interp->customStrFuncs;
    
    /* Prepend to the list */
    interp->customStrFuncs = newFunc;
    
    return 1;
}

int registerCustomCommand(Interpreter *interp, const char *name, CustomCommandHandler handler) {
    CustomCommand *newCmd;
    char *nameCopy;
    size_t i;
    size_t nameLen;
    
    if (!interp || !name || !handler) return 0;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Allocate the registry entry */
    newCmd = (CustomCommand*)malloc(sizeof(CustomCommand));
    if (!newCmd) return 0;
    
    /* Duplicate and uppercase the command name */
    nameCopy = (char*)malloc(nameLen + 1);
    if (!nameCopy) {
        free(newCmd);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < nameLen; i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialise and prepend to the list */
    newCmd->name = nameCopy;
    newCmd->handler = handler;
    newCmd->next = interp->customCommands;
    
    /* Prepend to the list */
    interp->customCommands = newCmd;
    
    return 1;
}

CustomNumericFunction findCustomNumericFunction(Interpreter *interp, const char *name) {
    CustomNumFunc *current;
    char upperName[256];
    size_t i;
    size_t nameLen;
    
    if (!interp || !name) return NULL;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Convert the search name to uppercase for case-insensitive lookup */
    for (i = 0; i < nameLen && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Walk the linked list */
    current = interp->customNumFuncs;
    while (current) {
        if (strcmp(current->name, upperName) == 0) {
            return current->handler;
        }
        current = current->next;
    }
    
    return NULL;
}

CustomStringFunction findCustomStringFunction(Interpreter *interp, const char *name) {
    CustomStrFunc *current;
    char upperName[256];
    size_t i;
    size_t nameLen;
    
    if (!interp || !name) return NULL;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Convert the search name to uppercase for case-insensitive lookup */
    for (i = 0; i < nameLen && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Walk the linked list */
    current = interp->customStrFuncs;
    while (current) {
        if (strcmp(current->name, upperName) == 0) {
            return current->handler;
        }
        current = current->next;
    }
    
    return NULL;
}

CustomCommandHandler findCustomCommand(Interpreter *interp, const char *name) {
    CustomCommand *current;
    char upperName[256];
    size_t i;
    size_t nameLen;
    
    if (!interp || !name) return NULL;
    
    nameLen = strlen(name);  /* Calculé une seule fois */
    
    /* Convert the search name to uppercase for case-insensitive lookup */
    for (i = 0; i < nameLen && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Walk the linked list */
    current = interp->customCommands;
    while (current) {
        if (strcmp(current->name, upperName) == 0) {
            return current->handler;
        }
        current = current->next;
    }
    
    return NULL;
}