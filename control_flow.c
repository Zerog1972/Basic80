#include "control_flow.h"
#include "expression.h"
#include "variables.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ===== FONCTIONS UTILITAIRES ===== */

Line* findLineByNumber(Interpreter *interp, int lineNum) {
    Line *line = interp->program;
    while (line && line->lineNum != lineNum) {
        line = line->next;
    }
    return line;
}

/* Fonction pour reconstruire une commande à partir des tokens */
static void buildCommandFromTokens(Token *tokens, int startPos, int endPos, char *output) {
    int i;
    output[0] = '\0';
    for (i = startPos; i < endPos && tokens[i].type != TOK_EOF; i++) {
        if (i > startPos) strcat(output, " ");
        if (tokens[i].type == TOK_STRING) {
            strcat(output, "\"");
            strcat(output, tokens[i].value);
            strcat(output, "\"");
        } else {
            strcat(output, tokens[i].value);
        }
    }
}

/* ===== GESTION IF/THEN/ELSE ===== */

int handleIfStatement(Interpreter *interp, Token *tokens, Line **currentLine) {
    int condition;
    int pos;
    int thenPos;
    int elsePos;
    int i;
    char thenPart[512];
    char elsePart[512];
    int targetLine;
    Line *target;
    
    pos = 1;
    condition = evaluateCondition(interp, tokens, &pos);
    
    /* Trouver THEN */
    if (tokens[pos].type == TOK_THEN) {
        pos++;
        thenPos = pos;
        
        /* Chercher ELSE (optionnel) */
        elsePos = -1;
        for (i = pos; tokens[i].type != TOK_EOF; i++) {
            if (tokens[i].type == TOK_ELSE) {
                elsePos = i + 1;
                break;
            }
        }
        
        if (condition) {
            /* Exécuter la partie THEN */
            if (elsePos > 0) {
                buildCommandFromTokens(tokens, thenPos, elsePos - 1, thenPart);
            } else {
                buildCommandFromTokens(tokens, thenPos, 1000, thenPart);
            }
            
            if (strlen(thenPart) > 0) {
                if (strncmp(thenPart, "GOTO ", 5) == 0) {
                    targetLine = atoi(&thenPart[5]);
                    target = findLineByNumber(interp, targetLine);
                    if (target) {
                        *currentLine = target;
                        return 1;
                    }
                } else {
                    executeCommand(interp, thenPart);
                }
            }
        } else if (elsePos > 0) {
            /* Exécuter la partie ELSE */
            buildCommandFromTokens(tokens, elsePos, 1000, elsePart);
            
            if (strlen(elsePart) > 0) {
                if (strncmp(elsePart, "GOTO ", 5) == 0) {
                    targetLine = atoi(&elsePart[5]);
                    target = findLineByNumber(interp, targetLine);
                    if (target) {
                        *currentLine = target;
                        return 1;
                    }
                } else {
                    executeCommand(interp, elsePart);
                }
            }
        }
    }
    
    return 0;
}

/* ===== GESTION GOTO ===== */

int handleGoto(Interpreter *interp, Token *tokens, Line **currentLine) {
    int targetLine;
    Line *target;
    
    if (tokens[1].type == TOK_NUMBER) {
        targetLine = atoi(tokens[1].value);
        target = findLineByNumber(interp, targetLine);
        if (target) {
            *currentLine = target;
            return 1;
        }
    }
    return 0;
}

/* ===== GESTION GOSUB/RETURN ===== */

int handleGosub(Interpreter *interp, Token *tokens, Line **currentLine) {
    int targetLine;
    Line *target;
    CallStack *newCall;
    
    if (tokens[1].type == TOK_NUMBER) {
        targetLine = atoi(tokens[1].value);
        target = findLineByNumber(interp, targetLine);
        if (target) {
            /* Empiler la ligne suivante pour le RETURN */
            newCall = malloc(sizeof(CallStack));
            newCall->returnLine = (*currentLine)->next;
            newCall->next = interp->callStack;
            interp->callStack = newCall;
            
            /* Sauter à la sous-routine */
            *currentLine = target;
            return 1;
        }
    }
    return 0;
}

int handleReturn(Interpreter *interp, Line **currentLine) {
    CallStack *topCall;
    
    if (interp->callStack) {
        topCall = interp->callStack;
        *currentLine = topCall->returnLine;
        interp->callStack = topCall->next;
        free(topCall);
        return 1;
    }
    return 0;
}

/* ===== GESTION FOR/NEXT ===== */

int handleFor(Interpreter *interp, Token *tokens, Line **currentLine) {
    int pos;
    char varName[256];
    double startVal, endVal, stepVal;
    ForLoop *forLoop;
    Line *searchLine;
    Token *searchTokens;
    int nestLevel;
    
    pos = 1;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        strcpy(varName, tokens[pos].value);
        pos++;
        if (tokens[pos].type == TOK_EQUALS) {
            pos++;
            startVal = evaluateExpression(interp, tokens, &pos);
            if (tokens[pos].type == TOK_TO) {
                pos++;
                endVal = evaluateExpression(interp, tokens, &pos);
                stepVal = 1.0;
                if (tokens[pos].type == TOK_STEP) {
                    pos++;
                    stepVal = evaluateExpression(interp, tokens, &pos);
                }
                
                /* Vérifier si la boucle peut s'exécuter au moins une fois */
                if ((stepVal > 0 && startVal > endVal) ||
                    (stepVal < 0 && startVal < endVal)) {
                    /* Ne pas exécuter la boucle, sauter jusqu'au NEXT correspondant */
                    setVariable(interp, varName, startVal);
                    nestLevel = 1;
                    searchLine = (*currentLine)->next;
                    
                    while (searchLine && nestLevel > 0) {
                        searchTokens = tokenize(searchLine->code);
                        if (searchTokens[0].type == TOK_FOR) {
                            nestLevel++;
                        } else if (searchTokens[0].type == TOK_NEXT) {
                            nestLevel--;
                        }
                        freeTokens(searchTokens);
                        
                        if (nestLevel > 0) {
                            searchLine = searchLine->next;
                        }
                    }
                    
                    if (searchLine) {
                        *currentLine = searchLine;
                        return 1;
                    }
                } else {
                    /* Créer une nouvelle boucle FOR */
                    forLoop = malloc(sizeof(ForLoop));
                    forLoop->varName = malloc(strlen(varName) + 1);
                    strcpy(forLoop->varName, varName);
                    forLoop->endValue = endVal;
                    forLoop->stepValue = stepVal;
                    forLoop->startLine = *currentLine;
                    forLoop->next = interp->forStack;
                    interp->forStack = forLoop;
                    /* Initialiser la variable */
                    setVariable(interp, varName, startVal);
                }
            }
        }
    }
    return 0;
}

int handleNext(Interpreter *interp) {
    ForLoop *forLoop;
    double currentVal;
    
    if (interp->forStack) {
        forLoop = interp->forStack;
        currentVal = getVariable(interp, forLoop->varName);
        currentVal += forLoop->stepValue;
        setVariable(interp, forLoop->varName, currentVal);
        
        /* Vérifier si la boucle doit continuer */
        if ((forLoop->stepValue > 0 && currentVal <= forLoop->endValue) ||
            (forLoop->stepValue < 0 && currentVal >= forLoop->endValue)) {
            /* Retourner à la ligne après le FOR - retourne 1 pour indiquer qu'on doit continuer */
            return 1;
        } else {
            /* Sortir de la boucle */
            interp->forStack = forLoop->next;
            free(forLoop->varName);
            free(forLoop);
        }
    }
    return 0;
}
