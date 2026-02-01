#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/* Types de tokens */
typedef enum {
    TOK_PRINT,
    TOK_LET,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_GOTO,
    TOK_GOSUB,
    TOK_RETURN,
    TOK_INPUT,
    TOK_DIM,
    TOK_FOR,
    TOK_TO,
    TOK_STEP,
    TOK_NEXT,
    TOK_END,
    TOK_REM,
    TOK_SIN,
    TOK_COS,
    TOK_TAN,
    TOK_SQR,
    TOK_ABS,
    TOK_INT,
    TOK_RND,
    TOK_LEN,
    TOK_MID,
    TOK_LEFT,
    TOK_RIGHT,
    TOK_CHR,
    TOK_ASC,
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_EQUALS,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_NE,
    TOK_COMMA,
    TOK_NEWLINE,
    TOK_EOF
} TokenType;

/* Structure pour un token */
typedef struct {
    TokenType type;
    char *value;
    int lineNum;
} Token;

/* Structure pour une variable */
typedef struct Variable {
    char *name;
    int isString;        /* 1 si chaîne, 0 si nombre */
    double value;        /* Valeur numérique */
    char *strValue;      /* Valeur chaîne */
    int isArray;
    double *arrayValues;
    int arraySize;       /* Taille totale (produit de toutes les dimensions) */
    int numDimensions;   /* Nombre de dimensions (1, 2, 3, etc.) */
    int *dimensions;     /* Taille de chaque dimension */
    struct Variable *next;
} Variable;

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
typedef struct {
    Line *program;
    Variable *variables;
    Line *currentLine;
    ForLoop *forStack;
    CallStack *callStack;
} Interpreter;

/* Fonctions du lexer */
Token* tokenize(const char *line);
void freeTokens(Token *tokens);

/* Fonctions de l'interpréteur */
Interpreter* createInterpreter(void);
void freeInterpreter(Interpreter *interp);
void addLine(Interpreter *interp, int lineNum, const char *code);
void runProgram(Interpreter *interp);
void executeCommand(Interpreter *interp, const char *line);

/* Fonctions utilitaires */
Variable* findVariable(Interpreter *interp, const char *name);
void setVariable(Interpreter *interp, const char *name, double value);
double getVariable(Interpreter *interp, const char *name);
void setStringVariable(Interpreter *interp, const char *name, const char *value);
char* getStringVariable(Interpreter *interp, const char *name);

/* Fonctions pour les tableaux */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims);
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value);
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);

#endif