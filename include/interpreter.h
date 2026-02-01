#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lexer.h"
#include "variables.h"
#include "expression.h"

/* Structure pour une ligne de code */
typedef struct Line {
    int lineNum;
    char *code;
    struct Line *next;
} Line;

/* Structure pour une boucle FOR */
typedef struct ForLoop {
    char *varName;
    double endValue;
    double stepValue;
    Line *startLine;
    struct ForLoop *next;
} ForLoop;

/* Structure pour la pile d'appels GOSUB */
typedef struct CallStack {
    Line *returnLine;
    struct CallStack *next;
} CallStack;

/* Structure pour l'interpréteur */
struct Interpreter {
    Line *program;
    Variable *variables;
    Line *currentLine;
    ForLoop *forStack;
    CallStack *callStack;
};

/* Fonctions de l'interpréteur */
Interpreter* createInterpreter(void);
void freeInterpreter(Interpreter *interp);
void addLine(Interpreter *interp, int lineNum, const char *code);
void runProgram(Interpreter *interp);
void executeCommand(Interpreter *interp, const char *line);
void listProgram(Interpreter *interp);
void clearProgram(Interpreter *interp);
int saveProgram(Interpreter *interp, const char *filename);
int loadProgram(Interpreter *interp, const char *filename);

#endif /* UTILS_H */