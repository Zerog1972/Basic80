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
    interp->dataList = NULL;
    interp->dataPointer = NULL;
    interp->hasError = 0;
    return interp;
}

void reportError(Interpreter *interp, const char *message) {
    interp->hasError = 1;
    if (interp->currentLine) {
        printf("Erreur à la ligne %d: %s\n", interp->currentLine->lineNum, message);
    } else {
        printf("Erreur: %s\n", message);
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
    
    /* Libérer les données DATA */
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
    } else if (interp->program->lineNum == lineNum) {
        /* Remplacer la première ligne */
        free(interp->program->code);
        interp->program->code = malloc(strlen(code) + 1);
        strcpy(interp->program->code, code);
        free(newLine->code);
        free(newLine);
    } else {
        current = interp->program;
        while (current->next && current->next->lineNum < lineNum) {
            current = current->next;
        }
        
        if (current->next && current->next->lineNum == lineNum) {
            /* Remplacer une ligne existante */
            free(current->next->code);
            current->next->code = malloc(strlen(code) + 1);
            strcpy(current->next->code, code);
            free(newLine->code);
            free(newLine);
        } else {
            /* Insérer une nouvelle ligne */
            newLine->next = current->next;
            current->next = newLine;
        }
    }
}

/* Supprimer une ligne du programme */
void deleteLine(Interpreter *interp, int lineNum) {
    Line *current;
    Line *toDelete;
    
    if (!interp->program) {
        return; /* Programme vide */
    }
    
    /* Supprimer la première ligne */
    if (interp->program->lineNum == lineNum) {
        toDelete = interp->program;
        interp->program = interp->program->next;
        free(toDelete->code);
        free(toDelete);
        return;
    }
    
    /* Chercher la ligne à supprimer */
    current = interp->program;
    while (current->next && current->next->lineNum != lineNum) {
        current = current->next;
    }
    
    /* Si trouvée, la supprimer */
    if (current->next && current->next->lineNum == lineNum) {
        toDelete = current->next;
        current->next = toDelete->next;
        free(toDelete->code);
        free(toDelete);
    }
}

/* Exécute une instruction unique (sans ':') */
static void executeSingleStatement(Interpreter *interp, const char *stmt) {
    Token *tokens;
    
    tokens = tokenize(stmt);
    
    /* Ligne vide ou commentaire */
    if (tokens[0].type == TOK_EOF || tokens[0].type == TOK_REM) {
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
        /* DATA ne peut être exécuté en mode direct, seulement dans un programme */
        printf("Erreur: DATA ne peut être utilisé qu'à l'intérieur d'un programme.\n");
    }
    else if (tokens[0].type == TOK_RESTORE) {
        handleRestore(interp, tokens);
    }
    else if (tokens[0].type == TOK_FOR) {
        /* FOR ne s'exécute que dans runProgram */
        printf("Erreur: FOR ne peut être utilisé qu'à l'intérieur d'un programme.\n");
    }
    else if (tokens[0].type == TOK_NEXT) {
        /* NEXT ne s'exécute que dans runProgram */
        printf("Erreur: NEXT ne peut être utilisé qu'à l'intérieur d'un programme.\n");
    }
    else if (tokens[0].type == TOK_IDENTIFIER) {
        /* Affectation implicite sans LET : V = 5 ou V(1) = 5 */
        handleLet(interp, tokens);
    }
    else if (tokens[0].type == TOK_IF || tokens[0].type == TOK_GOTO || 
             tokens[0].type == TOK_GOSUB || tokens[0].type == TOK_RETURN ||
             tokens[0].type == TOK_END) {
        printf("Erreur: %s ne peut être utilisé qu'à l'intérieur d'un programme.\n", tokens[0].value);
    }
    else {
        printf("Erreur: Commande non reconnue '%s'.\n", 
               tokens[0].value ? tokens[0].value : "");
    }
    
    freeTokens(tokens);
}

/* Divise une ligne par ':' mais ignore les ':' après REM et dans les chaînes */
static char** splitByColon(const char *line, int *count) {
    char **parts;
    char *lineCopy;
    char *start;
    char *p;
    int capacity;
    int isInRem;
    int isInString;
    
    *count = 0;
    capacity = 10;
    parts = malloc(sizeof(char*) * capacity);
    
    lineCopy = malloc(strlen(line) + 1);
    strcpy(lineCopy, line);
    
    start = lineCopy;
    p = lineCopy;
    isInRem = 0;
    isInString = 0;
    
    /* Vérifier si la ligne commence par REM */
    while (*p && isspace(*p)) p++;
    if (strncmp(p, "REM", 3) == 0 && (p[3] == ' ' || p[3] == '\0')) {
        isInRem = 1;
    }
    
    p = start;
    while (*p) {
        /* Gérer les guillemets */
        if (*p == '"' && !isInRem) {
            isInString = !isInString;
            p++;
            continue;
        }
        
        if (*p == ':' && !isInRem && !isInString) {
            /* Trouver un ':' hors REM et hors chaîne */
            *p = '\0';
            if (*count >= capacity) {
                capacity *= 2;
                parts = realloc(parts, sizeof(char*) * capacity);
            }
            parts[*count] = malloc(strlen(start) + 1);
            strcpy(parts[*count], start);
            (*count)++;
            p++;
            start = p;
            
            /* Vérifier si la prochaine instruction est REM */
            while (*p && isspace(*p)) p++;
            if (strncmp(p, "REM", 3) == 0 && (p[3] == ' ' || p[3] == '\0')) {
                isInRem = 1;
            }
        } else {
            p++;
        }
    }
    
    /* Ajouter la dernière partie */
    if (*count >= capacity) {
        capacity++;
        parts = realloc(parts, sizeof(char*) * capacity);
    }
    parts[*count] = malloc(strlen(start) + 1);
    strcpy(parts[*count], start);
    (*count)++;
    
    free(lineCopy);
    return parts;
}

/* Libère le tableau de chaînes */
static void freeSplitArray(char **parts, int count) {
    int i;
    for (i = 0; i < count; i++) {
        free(parts[i]);
    }
    free(parts);
}

/* Exécuter une ligne de commande (peut contenir plusieurs instructions séparées par ':') */
void executeCommand(Interpreter *interp, const char *line) {
    char **parts;
    int count;
    int i;
    
    parts = splitByColon(line, &count);
    
    for (i = 0; i < count; i++) {
        /* Ignorer les espaces au début */
        char *stmt = parts[i];
        while (*stmt && isspace(*stmt)) stmt++;
        
        if (*stmt) {
            executeSingleStatement(interp, stmt);
        }
    }
    
    freeSplitArray(parts, count);
}

/* Exécute une instruction dans le contexte de runProgram (gestion des structures de contrôle) */
static int executeStatementInProgram(Interpreter *interp, const char *stmt, Line **line) {
    Token *tokens;
    int controlFlowHandled;
    
    tokens = tokenize(stmt);
    controlFlowHandled = 0;
    
    if (tokens[0].type == TOK_DATA) {
        /* DATA a déjà été traité dans la première passe, ignorer ici */
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
            return 1; /* Contrôle de flux géré */
        }
    }
    else if (tokens[0].type == TOK_GOTO) {
        if (handleGoto(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Contrôle de flux géré */
        }
    }
    else if (tokens[0].type == TOK_GOSUB) {
        if (handleGosub(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Contrôle de flux géré */
        }
    }
    else if (tokens[0].type == TOK_RETURN) {
        if (handleReturn(interp, line)) {
            freeTokens(tokens);
            return 1; /* Contrôle de flux géré */
        }
    }
    else if (tokens[0].type == TOK_FOR) {
        if (handleFor(interp, tokens, line)) {
            freeTokens(tokens);
            return 1; /* Contrôle de flux géré */
        }
    }
    else if (tokens[0].type == TOK_NEXT) {
        if (handleNext(interp)) {
            /* Retourner à la ligne après le FOR */
            *line = interp->forStack->startLine->next;
            freeTokens(tokens);
            return 1; /* Contrôle de flux géré */
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
    
    /* Première passe : extraire toutes les données DATA */
    line = interp->program;
    while (line) {
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
    
    /* Deuxième passe : exécuter le programme */
    line = interp->program;
    while (line) {
        interp->currentLine = line;
        
        /* Diviser la ligne par ':' en ignorant ceux dans REM */
        parts = splitByColon(line->code, &count);
        
        for (i = 0; i < count && !interp->hasError; i++) {
            /* Ignorer les espaces au début */
            char *stmt = parts[i];
            while (*stmt && isspace(*stmt)) stmt++;
            
            if (*stmt) {
                result = executeStatementInProgram(interp, stmt, &line);
                if (result == -1) {
                    /* END rencontré */
                    freeSplitArray(parts, count);
                    interp->currentLine = NULL;
                    return;
                }
                if (result == 1) {
                    /* Contrôle de flux géré (GOTO, IF, FOR, etc.) */
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