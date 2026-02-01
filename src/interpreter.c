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

/* Afficher le programme */
void listProgram(Interpreter *interp) {
    Line *line = interp->program;
    while (line) {
        printf("%d %s\n", line->lineNum, line->code);
        line = line->next;
    }
}

/* Effacer le programme */
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

/* Sauvegarder le programme dans un fichier */
int saveProgram(Interpreter *interp, const char *filename) {
    FILE *file;
    Line *line;
    
    file = fopen(filename, "w");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier '%s' en écriture.\n", filename);
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

/* Charger un programme depuis un fichier */
int loadProgram(Interpreter *interp, const char *filename) {
    FILE *file;
    char line[1024];
    int lineNum;
    char *code;
    char *p;
    
    file = fopen(filename, "r");
    if (!file) {
        printf("Erreur: Impossible d'ouvrir le fichier '%s' en lecture.\n", filename);
        return 0;
    }
    
    /* Effacer le programme actuel */
    clearProgram(interp);
    
    while (fgets(line, sizeof(line), file)) {
        /* Retirer le newline */
        line[strcspn(line, "\n")] = 0;
        
        /* Ignorer les lignes vides */
        if (strlen(line) == 0) continue;
        
        /* Parser le numéro de ligne */
        lineNum = atoi(line);
        if (lineNum <= 0) {
            printf("Attention: Ligne invalide ignorée: %s\n", line);
            continue;
        }
        
        /* Trouver le début du code après le numéro */
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