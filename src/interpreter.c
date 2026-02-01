#include "interpreter.h"
#include "control_flow.h"
#include "commands.h"

/* ===== INTERPRÉTEUR ===== */

Interpreter* createInterpreter(void) {
    Interpreter *interp = malloc(sizeof(Interpreter));
    interp->program = NULL;
    interp->variables = NULL;
    interp->currentLine = NULL;
    interp->forStack = NULL;
    interp->callStack = NULL;
    return interp;
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
    
    free(interp);
}

void addLine(Interpreter *interp, int lineNum, const char *code) {
    Line *newLine;
    Line *current;
    
    newLine = malloc(sizeof(Line));
    newLine->lineNum = lineNum;
    newLine->code = malloc(strlen(code) + 1);
    strcpy(newLine->code, code);
    newLine->next = NULL;
    
    if (!interp->program || interp->program->lineNum > lineNum) {
        newLine->next = interp->program;
        interp->program = newLine;
    } else {
        current = interp->program;
        while (current->next && current->next->lineNum < lineNum) {
            current = current->next;
        }
        
        if (current->lineNum == lineNum) {
            free(current->code);
            current->code = malloc(strlen(code) + 1);
            strcpy(current->code, code);
            free(newLine);
        } else {
            newLine->next = current->next;
            current->next = newLine;
        }
    }
}

/* Exécuter une ligne de commande */
void executeCommand(Interpreter *interp, const char *line) {
    Token *tokens;
    
    tokens = tokenize(line);
    
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
    else if (tokens[0].type == TOK_FOR) {
        /* FOR ne s'exécute que dans runProgram */
    }
    else if (tokens[0].type == TOK_NEXT) {
        /* NEXT ne s'exécute que dans runProgram */
    }
    
    freeTokens(tokens);
}

void runProgram(Interpreter *interp) {
    Line *line;
    Token *tokens;
    
    line = interp->program;
    while (line) {
        interp->currentLine = line;
        
        /* Trouver le premier mot de la ligne */
        tokens = tokenize(line->code);
        
        if (tokens[0].type == TOK_END) {
            freeTokens(tokens);
            break;
        }
        else if (tokens[0].type == TOK_IF) {
            if (handleIfStatement(interp, tokens, &line)) {
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_GOTO) {
            if (handleGoto(interp, tokens, &line)) {
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_GOSUB) {
            if (handleGosub(interp, tokens, &line)) {
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_RETURN) {
            if (handleReturn(interp, &line)) {
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_FOR) {
            if (handleFor(interp, tokens, &line)) {
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_NEXT) {
            if (handleNext(interp)) {
                /* Retourner à la ligne après le FOR */
                line = interp->forStack->startLine->next;
                freeTokens(tokens);
                continue;
            }
        }
        else {
            executeCommand(interp, line->code);
        }
        
        freeTokens(tokens);
        line = line->next;
    }
}