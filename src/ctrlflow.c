/*
 * control_flow.c - Control-flow statement handlers for Basic80
 *
 * Implements the runtime execution of all BASIC control-flow statements:
 *   IF/THEN/ELSE, GOTO, GOSUB/RETURN, FOR/NEXT
 *
 * Each handler may modify *currentLine to redirect execution.  Returning
 * 1 signals to runProgram() that the program counter has changed and the
 * current line must be restarted from the new position.
 */
#include "ctrlflow.h"
#include "expr.h"
#include "vars.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ===== UTILITY FUNCTIONS ===== */

/* Find a program line by its line number; returns NULL if not found */
Line* findLineByNumber(Interpreter *interp, int lineNum) {
    Line *line = interp->program;
    while (line && line->lineNum != lineNum) {
        line = line->next;
    }
    return line;
}

/* Helper: reconstruct a BASIC statement string from a range of tokens.
 * Uses a running pointer to avoid the O(n²) cost of strcat in a loop. */
static void buildCommandFromTokens(Token *tokens, int startPos, int endPos, char *output) {
    int i;
    char *p = output;
    size_t vlen;
    *p = '\0';
    for (i = startPos; i < endPos && tokens[i].type != TOK_EOF; i++) {
        if (i > startPos) *p++ = ' ';
        if (tokens[i].type == TOK_STRING) {
            *p++ = '"';
            vlen = strlen(tokens[i].value);
            memcpy(p, tokens[i].value, vlen);
            p += vlen;
            *p++ = '"';
        } else {
            vlen = strlen(tokens[i].value);
            memcpy(p, tokens[i].value, vlen);
            p += vlen;
        }
    }
    *p = '\0';
}

/* Execute a THEN/ELSE clause: split on ':', execute each statement.
 * Handles GOTO anywhere in the clause (not just as the first keyword).
 * Returns 1 and updates *currentLine if a GOTO jump was made, 0 otherwise. */
static int executeIfClause(Interpreter *interp, const char *clause, Line **currentLine) {
    char buf[512];
    char *start;
    char *p;
    int inStr;
    int targetLine;
    Line *target;

    strncpy(buf, clause, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    start = buf;
    inStr = 0;
    for (p = buf; ; p++) {
        if (*p == '"') inStr = !inStr;
        if (!inStr && (*p == ':' || *p == '\0')) {
            char saved = *p;
            *p = '\0';
            while (*start == ' ' || *start == '\t') start++;
            if (*start != '\0') {
                if (strncmp(start, "GOTO ", 5) == 0) {
                    targetLine = atoi(start + 5);
                    target = findLineByNumber(interp, targetLine);
                    if (target) {
                        *currentLine = target;
                        return 1;
                    }
                } else {
                    executeCommand(interp, start);
                }
            }
            if (saved == '\0') break;
            start = p + 1;
        }
    }
    return 0;
}

/* ===== IF/THEN/ELSE HANDLING ===== */

int handleIfStatement(Interpreter *interp, Token *tokens, Line **currentLine) {
    int condition;
    int pos;
    int thenPos;
    int elsePos;
    int i;
    char thenPart[512];
    char elsePart[512];
    
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
            /* Execute the THEN branch */
            if (elsePos > 0) {
                buildCommandFromTokens(tokens, thenPos, elsePos - 1, thenPart);
            } else {
                buildCommandFromTokens(tokens, thenPos, 1000, thenPart);
            }
            if (thenPart[0] != '\0') {
                if (executeIfClause(interp, thenPart, currentLine)) return 1;
            }
        } else if (elsePos > 0) {
            /* Execute the ELSE branch */
            buildCommandFromTokens(tokens, elsePos, 1000, elsePart);
            if (elsePart[0] != '\0') {
                if (executeIfClause(interp, elsePart, currentLine)) return 1;
            }
        }
    }
    
    return 0;
}

/* ===== GOTO HANDLING ===== */

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

/* ===== GOSUB/RETURN HANDLING ===== */

int handleGosub(Interpreter *interp, Token *tokens, Line **currentLine) {
    int targetLine;
    Line *target;
    CallStack *newCall;
    
    if (tokens[1].type == TOK_NUMBER) {
        targetLine = atoi(tokens[1].value);
        target = findLineByNumber(interp, targetLine);
        if (target) {
            /* Push the return address (line after the GOSUB) onto the call stack */
            newCall = malloc(sizeof(CallStack));
            if (!newCall) {
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, 1, "Out of memory for GOSUB");
                return 0;
            }
            newCall->returnLine = (*currentLine)->next;
            newCall->next = interp->callStack;
            interp->callStack = newCall;
            
            /* Jump to the subroutine */
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

/* ===== FOR/NEXT HANDLING ===== */

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
                    /* Loop body would never execute: skip forward to the matching NEXT */
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
                    /* Initial condition passes: push a new FOR loop entry */
                    forLoop = malloc(sizeof(ForLoop));
                    if (!forLoop) {
                        reportErrorEx(interp, ERR_OUT_OF_MEMORY, 1, "Out of memory for FOR");
                        return 0;
                    }
                    forLoop->varName = malloc(strlen(varName) + 1);
                    if (!forLoop->varName) {
                        free(forLoop);
                        reportErrorEx(interp, ERR_OUT_OF_MEMORY, 1, "Out of memory for FOR");
                        return 0;
                    }
                    strcpy(forLoop->varName, varName);
                    forLoop->endValue = endVal;
                    forLoop->stepValue = stepVal;
                    forLoop->startLine = *currentLine;
                    forLoop->next = interp->forStack;
                    interp->forStack = forLoop;
                    /* Initialise the loop control variable */
                    setVariable(interp, varName, startVal);
                }
            }
        }
    }
    return 0;
}

int handleNext(Interpreter *interp, const char *varName) {
    ForLoop *forLoop;
    double currentVal;

    if (!interp->forStack) return 0;

    /* If a variable name is given, discard any inner loops that don't match.
     * This handles the case where GOTO jumped out of an inner FOR loop,
     * leaving its stack entry orphaned.
     * IMPORTANT: only discard inner loops if varName is actually on the stack.
     * If it is NOT found, the corresponding FOR was skipped entirely and this
     * NEXT is a no-op (do not touch any outer loop). */
    if (varName && varName[0] != '\0') {
        ForLoop *search = interp->forStack;
        int found = 0;
        while (search) {
            if (strcmp(search->varName, varName) == 0) { found = 1; break; }
            search = search->next;
        }
        if (!found) return 0; /* FOR was skipped: treat NEXT as no-op */

        while (interp->forStack &&
               strcmp(interp->forStack->varName, varName) != 0) {
            forLoop = interp->forStack;
            interp->forStack = forLoop->next;
            free(forLoop->varName);
            free(forLoop);
        }
    }

    if (!interp->forStack) return 0;

    forLoop = interp->forStack;
    currentVal = getVariable(interp, forLoop->varName);
    currentVal += forLoop->stepValue;
    setVariable(interp, forLoop->varName, currentVal);

    /* Check whether the loop should continue */
    if ((forLoop->stepValue > 0 && currentVal <= forLoop->endValue) ||
        (forLoop->stepValue < 0 && currentVal >= forLoop->endValue)) {
        /* Loop continues: return 1 to signal FOR line re-entry */
        return 1;
    } else {
        /* Loop finished: pop the FOR entry */
        interp->forStack = forLoop->next;
        free(forLoop->varName);
        free(forLoop);
    }
    return 0;
}
