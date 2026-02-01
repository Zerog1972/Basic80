#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "lexer.h"

/* Forward declaration */
typedef struct Interpreter Interpreter;

/* Évaluation des conditions */
int evaluateCondition(Interpreter *interp, Token *tokens, int *pos);

/* Évaluation des expressions numériques */
double evaluateExpression(Interpreter *interp, Token *tokens, int *pos);
double evaluateTerm(Interpreter *interp, Token *tokens, int *pos);
double evaluateFactor(Interpreter *interp, Token *tokens, int *pos);

/* Évaluation des expressions de chaînes */
int isStringExpression(Interpreter *interp, Token *tokens, int pos);
char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos);
char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos);

#endif
