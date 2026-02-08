#include "interpreter.h"
#include "control_flow.h"
#include "commands.h"

/* ===== INTERPRÉTEUR ===== */

Interpreter* createInterpreter(void) {
    Interpreter *interp = malloc(sizeof(Interpreter));
    if (!interp) {
        fprintf(stderr, "Erreur: Allocation mémoire échouée pour l'interpréteur\n");
        return NULL;
    }
    interp->program = NULL;
    interp->variables = NULL;
    interp->currentLine = NULL;
    interp->forStack = NULL;
    interp->callStack = NULL;
    interp->dataList = NULL;
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
        case ERR_SYNTAX:         return "Syntaxe";
        case ERR_RUNTIME:        return "Exécution";
        case ERR_TYPE_MISMATCH:  return "Type";
        case ERR_OUT_OF_DATA:    return "Données";
        case ERR_DIVISION_ZERO:  return "Division";
        case ERR_UNDEFINED_VAR:  return "Variable";
        case ERR_ARRAY_BOUNDS:   return "Tableau";
        case ERR_OUT_OF_MEMORY:  return "Mémoire";
        default:                 return "Inconnue";
    }
}

void reportError(Interpreter *interp, const char *message) {
    reportErrorEx(interp, ERR_RUNTIME, -1, message);
}

void reportErrorEx(Interpreter *interp, ErrorType type, int column, const char *message) {
    interp->hasError = 1;
    interp->lastErrorType = type;
    interp->errorColumn = column;
    
    /* Affichage formaté de l'erreur */
    if (interp->currentLine) {
        printf("\n[ERREUR %s] Ligne %d", getErrorTypeName(type), interp->currentLine->lineNum);
        if (column >= 0) {
            printf(", colonne %d", column);
        }
        printf(": %s\n", message);
        
        /* Afficher le code de la ligne */
        if (interp->currentLine->code) {
            printf("  --> %s\n", interp->currentLine->code);
            
            /* Afficher un curseur si la colonne est connue */
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
        printf("\n[ERREUR %s]: %s\n", getErrorTypeName(type), message);
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
    
    /* Libérer les hooks personnalisés */
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
    
    newLine = malloc(sizeof(Line));
    if (!newLine) return;
    newLine->lineNum = lineNum;
    newLine->code = malloc(strlen(code) + 1);
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
    else if (tokens[0].type == TOK_HELP) {
        handleHelp(interp, tokens);
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
        /* Vérifier si c'est une commande personnalisée */
        CustomCommandHandler customCmd = findCustomCommand(interp, tokens[0].value);
        if (customCmd) {
            customCmd(interp, tokens);
        } else {
            printf("Erreur: Commande non reconnue '%s'.\n", 
                   tokens[0].value ? tokens[0].value : "");
        }
    }
    
    freeTokens(tokens);
}

/* Libère le tableau de chaînes */
static void freeSplitArray(char **parts, int count) {
    int i;
    for (i = 0; i < count; i++) {
        free(parts[i]);
    }
    free(parts);
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
    
    tokens = tokenize(stmt);
    
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

/* ========================================================================
 * FONCTIONS POUR LE SYSTEME DE HOOKS/CALLBACKS
 * ======================================================================== */

int registerCustomNumericFunction(Interpreter *interp, const char *name, CustomNumericFunction handler) {
    CustomNumFunc *newFunc;
    char *nameCopy;
    size_t i;
    
    if (!interp || !name || !handler) return 0;
    
    /* Allouer la structure */
    newFunc = (CustomNumFunc*)malloc(sizeof(CustomNumFunc));
    if (!newFunc) return 0;
    
    /* Copier le nom en majuscules */
    nameCopy = (char*)malloc(strlen(name) + 1);
    if (!nameCopy) {
        free(newFunc);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < strlen(nameCopy); i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialiser la structure */
    newFunc->name = nameCopy;
    newFunc->handler = handler;
    newFunc->next = interp->customNumFuncs;
    
    /* Ajouter en tete de liste */
    interp->customNumFuncs = newFunc;
    
    return 1;
}

int registerCustomStringFunction(Interpreter *interp, const char *name, CustomStringFunction handler) {
    CustomStrFunc *newFunc;
    char *nameCopy;
    size_t i;
    
    if (!interp || !name || !handler) return 0;
    
    /* Allouer la structure */
    newFunc = (CustomStrFunc*)malloc(sizeof(CustomStrFunc));
    if (!newFunc) return 0;
    
    /* Copier le nom en majuscules */
    nameCopy = (char*)malloc(strlen(name) + 1);
    if (!nameCopy) {
        free(newFunc);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < strlen(nameCopy); i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialiser la structure */
    newFunc->name = nameCopy;
    newFunc->handler = handler;
    newFunc->next = interp->customStrFuncs;
    
    /* Ajouter en tete de liste */
    interp->customStrFuncs = newFunc;
    
    return 1;
}

int registerCustomCommand(Interpreter *interp, const char *name, CustomCommandHandler handler) {
    CustomCommand *newCmd;
    char *nameCopy;
    size_t i;
    
    if (!interp || !name || !handler) return 0;
    
    /* Allouer la structure */
    newCmd = (CustomCommand*)malloc(sizeof(CustomCommand));
    if (!newCmd) return 0;
    
    /* Copier le nom en majuscules */
    nameCopy = (char*)malloc(strlen(name) + 1);
    if (!nameCopy) {
        free(newCmd);
        return 0;
    }
    
    strcpy(nameCopy, name);
    for (i = 0; i < strlen(nameCopy); i++) {
        nameCopy[i] = (char)toupper((unsigned char)nameCopy[i]);
    }
    
    /* Initialiser la structure */
    newCmd->name = nameCopy;
    newCmd->handler = handler;
    newCmd->next = interp->customCommands;
    
    /* Ajouter en tete de liste */
    interp->customCommands = newCmd;
    
    return 1;
}

CustomNumericFunction findCustomNumericFunction(Interpreter *interp, const char *name) {
    CustomNumFunc *current;
    char upperName[256];
    size_t i;
    
    if (!interp || !name) return NULL;
    
    /* Convertir le nom en majuscules pour la comparaison */
    for (i = 0; i < strlen(name) && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Parcourir la liste */
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
    
    if (!interp || !name) return NULL;
    
    /* Convertir le nom en majuscules pour la comparaison */
    for (i = 0; i < strlen(name) && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Parcourir la liste */
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
    
    if (!interp || !name) return NULL;
    
    /* Convertir le nom en majuscules pour la comparaison */
    for (i = 0; i < strlen(name) && i < sizeof(upperName) - 1; i++) {
        upperName[i] = (char)toupper((unsigned char)name[i]);
    }
    upperName[i] = '\0';
    
    /* Parcourir la liste */
    current = interp->customCommands;
    while (current) {
        if (strcmp(current->name, upperName) == 0) {
            return current->handler;
        }
        current = current->next;
    }
    
    return NULL;
}