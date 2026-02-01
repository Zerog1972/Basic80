#include "utils.h"

/* ===== LEXER ===== */

static int isKeyword(const char *word, TokenType *type) {
    if (strcmp(word, "PRINT") == 0) { *type = TOK_PRINT; return 1; }
    if (strcmp(word, "LET") == 0) { *type = TOK_LET; return 1; }
    if (strcmp(word, "IF") == 0) { *type = TOK_IF; return 1; }
    if (strcmp(word, "THEN") == 0) { *type = TOK_THEN; return 1; }
    if (strcmp(word, "ELSE") == 0) { *type = TOK_ELSE; return 1; }
    if (strcmp(word, "GOTO") == 0) { *type = TOK_GOTO; return 1; }
    if (strcmp(word, "GOSUB") == 0) { *type = TOK_GOSUB; return 1; }
    if (strcmp(word, "RETURN") == 0) { *type = TOK_RETURN; return 1; }
    if (strcmp(word, "INPUT") == 0) { *type = TOK_INPUT; return 1; }
    if (strcmp(word, "DIM") == 0) { *type = TOK_DIM; return 1; }
    if (strcmp(word, "FOR") == 0) { *type = TOK_FOR; return 1; }
    if (strcmp(word, "TO") == 0) { *type = TOK_TO; return 1; }
    if (strcmp(word, "STEP") == 0) { *type = TOK_STEP; return 1; }
    if (strcmp(word, "NEXT") == 0) { *type = TOK_NEXT; return 1; }
    if (strcmp(word, "END") == 0) { *type = TOK_END; return 1; }
    if (strcmp(word, "REM") == 0) { *type = TOK_REM; return 1; }
    if (strcmp(word, "SIN") == 0) { *type = TOK_SIN; return 1; }
    if (strcmp(word, "COS") == 0) { *type = TOK_COS; return 1; }
    if (strcmp(word, "TAN") == 0) { *type = TOK_TAN; return 1; }
    if (strcmp(word, "SQR") == 0) { *type = TOK_SQR; return 1; }
    if (strcmp(word, "ABS") == 0) { *type = TOK_ABS; return 1; }
    if (strcmp(word, "INT") == 0) { *type = TOK_INT; return 1; }
    if (strcmp(word, "RND") == 0) { *type = TOK_RND; return 1; }
    if (strcmp(word, "LEN") == 0) { *type = TOK_LEN; return 1; }
    if (strcmp(word, "MID") == 0) { *type = TOK_MID; return 1; }
    if (strcmp(word, "LEFT") == 0) { *type = TOK_LEFT; return 1; }
    if (strcmp(word, "RIGHT") == 0) { *type = TOK_RIGHT; return 1; }
    if (strcmp(word, "CHR") == 0) { *type = TOK_CHR; return 1; }
    if (strcmp(word, "ASC") == 0) { *type = TOK_ASC; return 1; }
    return 0;
}

Token* tokenize(const char *line) {
    Token *tokens;
    int tokenCount;
    int i;
    int len;
    Token *tok;
    int start;
    char word[256];
    int wordLen;
    int j;
    
    tokens = malloc(sizeof(Token) * 256);
    tokenCount = 0;
    i = 0;
    len = strlen(line);
    
    while (i < len) {
        /* Ignorer espaces */
        while (i < len && isspace(line[i])) i++;
        if (i >= len) break;
        
        tok = &tokens[tokenCount++];
        tok->lineNum = 0;
        
        /* Chaînes de caractères */
        if (line[i] == '"') {
            i++;
            start = i;
            while (i < len && line[i] != '"') i++;
            tok->type = TOK_STRING;
            tok->value = malloc(i - start + 1);
            strncpy(tok->value, &line[start], i - start);
            tok->value[i - start] = '\0';
            i++;
        }
        /* Nombres */
        else if (isdigit(line[i])) {
            start = i;
            while (i < len && (isdigit(line[i]) || line[i] == '.')) i++;
            tok->type = TOK_NUMBER;
            tok->value = malloc(i - start + 1);
            strncpy(tok->value, &line[start], i - start);
            tok->value[i - start] = '\0';
        }
        /* Identifiants et mots-clés */
        else if (isalpha(line[i])) {
            start = i;
            while (i < len && (isalnum(line[i]) || line[i] == '_')) i++;
            wordLen = i - start;
            strncpy(word, &line[start], wordLen);
            word[wordLen] = '\0';
            
            /* Convertir en majuscules */
            for (j = 0; j < wordLen; j++) word[j] = toupper(word[j]);
            
            if (!isKeyword(word, &tok->type)) {
                tok->type = TOK_IDENTIFIER;
            }
            tok->value = malloc(strlen(word) + 1);
            strcpy(tok->value, word);
        }
        /* Opérateurs */
        else {
            tok->value = malloc(2);
            tok->value[0] = line[i];
            tok->value[1] = '\0';
            
            switch (line[i]) {
                case '=': tok->type = TOK_EQUALS; break;
                case '+': tok->type = TOK_PLUS; break;
                case '-': tok->type = TOK_MINUS; break;
                case '*': tok->type = TOK_MULTIPLY; break;
                case '/': tok->type = TOK_DIVIDE; break;
                case '(': tok->type = TOK_LPAREN; break;
                case ')': tok->type = TOK_RPAREN; break;
                case '<': 
                    if (i + 1 < len && line[i + 1] == '=') {
                        tok->type = TOK_LE;
                        tok->value[0] = '<'; tok->value[1] = '=';
                        i++;
                    } else if (i + 1 < len && line[i + 1] == '>') {
                        tok->type = TOK_NE;
                        tok->value[0] = '<'; tok->value[1] = '>';
                        i++;
                    } else {
                        tok->type = TOK_LT;
                    }
                    break;
                case '>':
                    if (i + 1 < len && line[i + 1] == '=') {
                        tok->type = TOK_GE;
                        tok->value[0] = '>'; tok->value[1] = '=';
                        i++;
                    } else {
                        tok->type = TOK_GT;
                    }
                    break;
                case ',': tok->type = TOK_COMMA; break;
                default: 
                    free(tok->value);
                    tokenCount--;
                    break;
            }
            i++;
        }
    }
    
    tokens[tokenCount].type = TOK_EOF;
    tokens[tokenCount].value = NULL;
    return tokens;
}

void freeTokens(Token *tokens) {
    int i;
    for (i = 0; tokens[i].type != TOK_EOF; i++) {
        if (tokens[i].value) free(tokens[i].value);
    }
    free(tokens);
}

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

Variable* findVariable(Interpreter *interp, const char *name) {
    Variable *var = interp->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) return var;
        var = var->next;
    }
    return NULL;
}

void setVariable(Interpreter *interp, const char *name, double value) {
    Variable *var;
    Variable *newVar;
    
    var = findVariable(interp, name);
    if (var) {
        if (!var->isArray) {
            var->value = value;
            var->isString = 0;
            if (var->strValue) {
                free(var->strValue);
                var->strValue = NULL;
            }
        }
    } else {
        newVar = malloc(sizeof(Variable));
        newVar->name = malloc(strlen(name) + 1);
        strcpy(newVar->name, name);
        newVar->value = value;
        newVar->isString = 0;
        newVar->strValue = NULL;
        newVar->isArray = 0;
        newVar->arrayValues = NULL;
        newVar->arraySize = 0;
        newVar->numDimensions = 0;
        newVar->dimensions = NULL;
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

double getVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && !var->isArray && !var->isString) {
        return var->value;
    }
    return 0.0;
}

/* Définir une variable chaîne */
void setStringVariable(Interpreter *interp, const char *name, const char *value) {
    Variable *var;
    Variable *newVar;
    
    var = findVariable(interp, name);
    if (var) {
        if (!var->isArray) {
            if (var->strValue) {
                free(var->strValue);
            }
            var->strValue = malloc(strlen(value) + 1);
            strcpy(var->strValue, value);
            var->isString = 1;
        }
    } else {
        newVar = malloc(sizeof(Variable));
        newVar->name = malloc(strlen(name) + 1);
        strcpy(newVar->name, name);
        newVar->value = 0.0;
        newVar->isString = 1;
        newVar->strValue = malloc(strlen(value) + 1);
        strcpy(newVar->strValue, value);
        newVar->isArray = 0;
        newVar->arrayValues = NULL;
        newVar->arraySize = 0;
        newVar->numDimensions = 0;
        newVar->dimensions = NULL;
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Obtenir une variable chaîne */
char* getStringVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && var->isString && var->strValue) {
        return var->strValue;
    }
    return "";
}

/* Créer un tableau */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims) {
    Variable *var;
    Variable *newVar;
    int i;
    int totalSize;
    
    /* Calculer la taille totale */
    totalSize = 1;
    for (i = 0; i < numDims; i++) {
        totalSize *= dims[i];
    }
    
    var = findVariable(interp, name);
    if (var) {
        /* Si la variable existe déjà, la transformer en tableau */
        if (var->arrayValues) {
            free(var->arrayValues);
        }
        if (var->dimensions) {
            free(var->dimensions);
        }
        var->isArray = 1;
        var->arraySize = totalSize;
        var->numDimensions = numDims;
        var->dimensions = malloc(sizeof(int) * numDims);
        for (i = 0; i < numDims; i++) {
            var->dimensions[i] = dims[i];
        }
        var->arrayValues = malloc(sizeof(double) * totalSize);
        for (i = 0; i < totalSize; i++) {
            var->arrayValues[i] = 0.0;
        }
    } else {
        /* Créer une nouvelle variable tableau */
        newVar = malloc(sizeof(Variable));
        newVar->name = malloc(strlen(name) + 1);
        strcpy(newVar->name, name);
        newVar->value = 0.0;
        newVar->isString = 0;
        newVar->strValue = NULL;
        newVar->isArray = 1;
        newVar->arraySize = totalSize;
        newVar->numDimensions = numDims;
        newVar->dimensions = malloc(sizeof(int) * numDims);
        for (i = 0; i < numDims; i++) {
            newVar->dimensions[i] = dims[i];
        }
        newVar->arrayValues = malloc(sizeof(double) * totalSize);
        for (i = 0; i < totalSize; i++) {
            newVar->arrayValues[i] = 0.0;
        }
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Définir la valeur d'un élément de tableau */
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value) {
    Variable *var;
    int flatIndex;
    int i;
    int multiplier;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return;
    }
    
    /* Convertir les indices multi-dimensionnels en index plat */
    flatIndex = 0;
    multiplier = 1;
    for (i = numIndices - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return; /* Index hors limites */
        }
        flatIndex += indices[i] * multiplier;
        multiplier *= var->dimensions[i];
    }
    
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        var->arrayValues[flatIndex] = value;
    }
}

/* Obtenir la valeur d'un élément de tableau */
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices) {
    Variable *var;
    int flatIndex;
    int i;
    int multiplier;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return 0.0;
    }
    
    /* Convertir les indices multi-dimensionnels en index plat */
    flatIndex = 0;
    multiplier = 1;
    for (i = numIndices - 1; i >= 0; i--) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return 0.0; /* Index hors limites */
        }
        flatIndex += indices[i] * multiplier;
        multiplier *= var->dimensions[i];
    }
    
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        return var->arrayValues[flatIndex];
    }
    return 0.0;
}

/* Déclaration forward */
static double evaluateExpression(Interpreter *interp, Token *tokens, int *pos);
static double evaluateTerm(Interpreter *interp, Token *tokens, int *pos);
static double evaluateFactor(Interpreter *interp, Token *tokens, int *pos);
static int evaluateCondition(Interpreter *interp, Token *tokens, int *pos);
static int isStringExpression(Interpreter *interp, Token *tokens, int pos);
static char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos);
static char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos);

/* Évaluer une condition (pour IF...THEN) */
static int evaluateCondition(Interpreter *interp, Token *tokens, int *pos) {
    double left, right;
    TokenType op;
    
    left = evaluateExpression(interp, tokens, pos);
    
    if (tokens[*pos].type == TOK_LT || tokens[*pos].type == TOK_GT ||
        tokens[*pos].type == TOK_LE || tokens[*pos].type == TOK_GE ||
        tokens[*pos].type == TOK_EQUALS || tokens[*pos].type == TOK_NE) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateExpression(interp, tokens, pos);
        
        switch (op) {
            case TOK_LT: return left < right;
            case TOK_GT: return left > right;
            case TOK_LE: return left <= right;
            case TOK_GE: return left >= right;
            case TOK_EQUALS: return left == right;
            case TOK_NE: return left != right;
            default: return 0;
        }
    }
    
    /* Si pas d'opérateur de comparaison, considérer comme vrai si non-zéro */
    return left != 0;
}

/* Évaluer un facteur (nombre, variable ou parenthèses) */
static double evaluateFactor(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    char varName[256];
    int indices[10]; /* Maximum 10 dimensions */
    int numIndices;
    double arg;
    
    result = 0;
    
    if (tokens[*pos].type == TOK_NUMBER) {
        result = atof(tokens[*pos].value);
        (*pos)++;
    } else if (tokens[*pos].type == TOK_IDENTIFIER) {
        strcpy(varName, tokens[*pos].value);
        (*pos)++;
        
        /* Vérifier si c'est un accès à un tableau */
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            numIndices = 0;
            /* Lire tous les indices séparés par des virgules */
            while (numIndices < 10) {
                indices[numIndices] = (int)evaluateExpression(interp, tokens, pos);
                numIndices++;
                if (tokens[*pos].type == TOK_COMMA) {
                    (*pos)++; /* Passer la virgule */
                } else {
                    break; /* Fin des indices */
                }
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = getArrayElement(interp, varName, indices, numIndices);
        } else {
            result = getVariable(interp, varName);
        }
    } else if (tokens[*pos].type == TOK_SIN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = sin(arg);
        }
    } else if (tokens[*pos].type == TOK_COS) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = cos(arg);
        }
    } else if (tokens[*pos].type == TOK_TAN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = tan(arg);
        }
    } else if (tokens[*pos].type == TOK_SQR) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = sqrt(arg);
        }
    } else if (tokens[*pos].type == TOK_ABS) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = fabs(arg);
        }
    } else if (tokens[*pos].type == TOK_INT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = floor(arg);
        }
    } else if (tokens[*pos].type == TOK_RND) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            /* RND(n) retourne un nombre aléatoire entre 0 et n-1 */
            result = (double)(rand() % (int)arg);
        } else {
            /* RND sans parenthèses retourne un nombre entre 0.0 et 1.0 */
            result = (double)rand() / (double)RAND_MAX;
        }
    } else if (tokens[*pos].type == TOK_LEN) {
        /* LEN(string) - retourne la longueur d'une chaîne */
        char varName[256];
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            if (tokens[*pos].type == TOK_STRING) {
                result = (double)strlen(tokens[*pos].value);
                (*pos)++;
            } else if (tokens[*pos].type == TOK_IDENTIFIER) {
                strcpy(varName, tokens[*pos].value);
                (*pos)++;
                result = (double)strlen(getStringVariable(interp, varName));
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
        }
    } else if (tokens[*pos].type == TOK_ASC) {
        /* ASC(string) - retourne le code ASCII du premier caractère */
        char varName[256];
        char *str;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            if (tokens[*pos].type == TOK_STRING) {
                str = tokens[*pos].value;
                if (strlen(str) > 0) {
                    result = (double)((unsigned char)str[0]);
                }
                (*pos)++;
            } else if (tokens[*pos].type == TOK_IDENTIFIER) {
                strcpy(varName, tokens[*pos].value);
                (*pos)++;
                str = getStringVariable(interp, varName);
                if (strlen(str) > 0) {
                    result = (double)((unsigned char)str[0]);
                }
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
        }
    } else if (tokens[*pos].type == TOK_LPAREN) {
        (*pos)++;
        result = evaluateExpression(interp, tokens, pos);
        if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
    }
    
    return result;
}

/* Évaluer un terme (multiplication et division) */
static double evaluateTerm(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    TokenType op;
    double right;
    
    result = evaluateFactor(interp, tokens, pos);
    
    while (tokens[*pos].type == TOK_MULTIPLY || tokens[*pos].type == TOK_DIVIDE) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateFactor(interp, tokens, pos);
        
        switch (op) {
            case TOK_MULTIPLY: result *= right; break;
            case TOK_DIVIDE: if (right != 0) result /= right; break;
            default: break;
        }
    }
    
    return result;
}

/* Évaluer une primitive de chaîne (littéral, variable ou fonction) */
static char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos) {
    char *result = NULL;
    char varName[256];
    char *str;
    int start, length, n;
    int strLen;

    if (tokens[*pos].type == TOK_STRING) {
        result = (char*)malloc(strlen(tokens[*pos].value) + 1);
        if (result) {
            strcpy(result, tokens[*pos].value);
        }
        (*pos)++;
    } else if (tokens[*pos].type == TOK_IDENTIFIER) {
        strcpy(varName, tokens[*pos].value);
        (*pos)++;
        str = getStringVariable(interp, varName);
        result = (char*)malloc(strlen(str) + 1);
        if (result) {
            strcpy(result, str);
        }
    } else if (tokens[*pos].type == TOK_CHR) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            int code;
            (*pos)++;
            code = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = (char*)malloc(2);
            if (result) {
                result[0] = (char)code;
                result[1] = '\0';
            }
        }
    } else if (tokens[*pos].type == TOK_MID) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            start = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            length = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (start < 1) start = 1;
            if (start > strLen) {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            } else {
                int actualLen = length;
                if (start - 1 + actualLen > strLen) {
                    actualLen = strLen - (start - 1);
                }
                result = (char*)malloc(actualLen + 1);
                if (result) {
                    strncpy(result, str + (start - 1), actualLen);
                    result[actualLen] = '\0';
                }
            }
            free(str);
        }
    } else if (tokens[*pos].type == TOK_LEFT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (n > strLen) n = strLen;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                strncpy(result, str, n);
                result[n] = '\0';
            }
            free(str);
        }
    } else if (tokens[*pos].type == TOK_RIGHT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (n > strLen) n = strLen;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                strcpy(result, str + strLen - n);
            }
            free(str);
        }
    }

    if (!result) {
        result = (char*)malloc(1);
        if (result) result[0] = '\0';
    }

    return result;
}

/* Évaluer une expression de chaîne avec concaténation (+) */
static char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos) {
    char *result;

    result = evaluateStringPrimary(interp, tokens, pos);
    while (tokens[*pos].type == TOK_PLUS) {
        char *rhs;
        char *concat;
        size_t len;
        (*pos)++;
        rhs = evaluateStringPrimary(interp, tokens, pos);
        len = strlen(result) + strlen(rhs) + 1;
        concat = (char*)malloc(len);
        if (concat) {
            strcpy(concat, result);
            strcat(concat, rhs);
        }
        free(result);
        free(rhs);
        result = concat ? concat : (char*)malloc(1);
        if (!result) {
            /* Allocation échouée, retourner chaîne vide */
            result = (char*)malloc(1);
            if (result) result[0] = '\0';
        }
    }

    return result;
}

/* Vérifier si un token commence une expression de chaîne */
static int isStringExpression(Interpreter *interp, Token *tokens, int pos) {
    Variable *var;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        var = findVariable(interp, tokens[pos].value);
        if (var && var->isString) {
            return 1;
        }
    }
    return tokens[pos].type == TOK_STRING ||
           tokens[pos].type == TOK_CHR ||
           tokens[pos].type == TOK_MID ||
           tokens[pos].type == TOK_LEFT ||
           tokens[pos].type == TOK_RIGHT;
}

/* Évaluer une expression (addition et soustraction) */
static double evaluateExpression(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    TokenType op;
    double right;
    
    result = evaluateTerm(interp, tokens, pos);
    
    while (tokens[*pos].type == TOK_PLUS || tokens[*pos].type == TOK_MINUS) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateTerm(interp, tokens, pos);
        
        switch (op) {
            case TOK_PLUS: result += right; break;
            case TOK_MINUS: result -= right; break;
            default: break;
        }
    }
    
    return result;
}

/* Exécuter une ligne de commande */
void executeCommand(Interpreter *interp, const char *line) {
    Token *tokens;
    int pos;
    double val;
    char varName[256];
    
    tokens = tokenize(line);
    pos = 0;
    
    if (tokens[0].type == TOK_PRINT) {
        pos = 1;
        while (tokens[pos].type != TOK_EOF) {
            if (isStringExpression(interp, tokens, pos)) {
                char *strResult = evaluateStringExpression(interp, tokens, &pos);
                printf("%s", strResult);
                free(strResult);
            } else {
                val = evaluateExpression(interp, tokens, &pos);
                printf("%.2f", val);
            }
            if (tokens[pos].type == TOK_COMMA) {
                printf(" ");
                pos++;
            }
        }
        printf("\n");
    }
    else if (tokens[0].type == TOK_LET) {
        pos = 1;
        if (tokens[pos].type == TOK_IDENTIFIER) {
            strcpy(varName, tokens[pos].value);
            pos++;
            
            /* Vérifier si c'est une affectation de tableau */
            if (tokens[pos].type == TOK_LPAREN) {
                int indices[10]; /* Maximum 10 dimensions */
                int numIndices;
                
                pos++;
                numIndices = 0;
                /* Lire tous les indices séparés par des virgules */
                while (numIndices < 10) {
                    indices[numIndices] = (int)evaluateExpression(interp, tokens, &pos);
                    numIndices++;
                    if (tokens[pos].type == TOK_COMMA) {
                        pos++; /* Passer la virgule */
                    } else {
                        break; /* Fin des indices */
                    }
                }
                if (tokens[pos].type == TOK_RPAREN) pos++;
                if (tokens[pos].type == TOK_EQUALS) {
                    pos++;
                    val = evaluateExpression(interp, tokens, &pos);
                    setArrayElement(interp, varName, indices, numIndices, val);
                }
            } else if (tokens[pos].type == TOK_EQUALS) {
                pos++;
                if (isStringExpression(interp, tokens, pos)) {
                    char *strResult = evaluateStringExpression(interp, tokens, &pos);
                    setStringVariable(interp, varName, strResult);
                    free(strResult);
                } else if (tokens[pos].type == TOK_IDENTIFIER) {
                    Variable *srcVar = findVariable(interp, tokens[pos].value);
                    if (srcVar && srcVar->isString) {
                        setStringVariable(interp, varName, srcVar->strValue);
                        pos++;
                    } else {
                        val = evaluateExpression(interp, tokens, &pos);
                        setVariable(interp, varName, val);
                    }
                } else {
                    val = evaluateExpression(interp, tokens, &pos);
                    setVariable(interp, varName, val);
                }
            }
        }
    }
    else if (tokens[0].type == TOK_DIM) {
        /* DIM arrayName(size1) ou DIM arrayName(size1, size2, ...) */
        int dims[10]; /* Maximum 10 dimensions */
        int numDims;
        int i;
        
        pos = 1;
        if (tokens[pos].type == TOK_IDENTIFIER) {
            strcpy(varName, tokens[pos].value);
            pos++;
            if (tokens[pos].type == TOK_LPAREN) {
                pos++;
                numDims = 0;
                /* Lire toutes les dimensions séparées par des virgules */
                while (numDims < 10) {
                    dims[numDims] = (int)evaluateExpression(interp, tokens, &pos) + 1; /* +1 car BASIC commence à 0 */
                    numDims++;
                    if (tokens[pos].type == TOK_COMMA) {
                        pos++; /* Passer la virgule */
                    } else {
                        break; /* Fin des dimensions */
                    }
                }
                if (tokens[pos].type == TOK_RPAREN) pos++;
                createArray(interp, varName, dims, numDims);
            }
        }
    }
    else if (tokens[0].type == TOK_INPUT) {
        pos = 1;
        if (tokens[pos].type == TOK_IDENTIFIER) {
            strcpy(varName, tokens[pos].value);
            if (scanf("%lf", &val) == 1) {
                setVariable(interp, varName, val);
            }
        }
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
    int targetLine;
    Line *target;
    int pos;
    char varName[256];
    double startVal, endVal, stepVal;
    ForLoop *forLoop;
    double currentVal;
    
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
            /* IF condition THEN action [ELSE action] */
            int condition;
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
                    /* Exécuter la partie THEN */
                    thenPart[0] = '\0';
                    if (elsePos > 0) {
                        /* Construire la commande jusqu'au ELSE */
                        for (i = thenPos; i < elsePos - 1 && tokens[i].type != TOK_EOF; i++) {
                            if (i > thenPos) strcat(thenPart, " ");
                            if (tokens[i].type == TOK_STRING) {
                                strcat(thenPart, "\"");
                                strcat(thenPart, tokens[i].value);
                                strcat(thenPart, "\"");
                            } else {
                                strcat(thenPart, tokens[i].value);
                            }
                        }
                    } else {
                        /* Construire toute la commande après THEN */
                        for (i = thenPos; tokens[i].type != TOK_EOF; i++) {
                            if (i > thenPos) strcat(thenPart, " ");
                            if (tokens[i].type == TOK_STRING) {
                                strcat(thenPart, "\"");
                                strcat(thenPart, tokens[i].value);
                                strcat(thenPart, "\"");
                            } else {
                                strcat(thenPart, tokens[i].value);
                            }
                        }
                    }
                    
                    if (strlen(thenPart) > 0) {
                        if (strncmp(thenPart, "GOTO ", 5) == 0) {
                            /* GOTO dans IF...THEN */
                            targetLine = atoi(&thenPart[5]);
                            target = interp->program;
                            while (target && target->lineNum != targetLine) {
                                target = target->next;
                            }
                            if (target) {
                                line = target;
                                freeTokens(tokens);
                                continue;
                            }
                        } else {
                            executeCommand(interp, thenPart);
                        }
                    }
                } else if (elsePos > 0) {
                    /* Exécuter la partie ELSE */
                    elsePart[0] = '\0';
                    for (i = elsePos; tokens[i].type != TOK_EOF; i++) {
                        if (i > elsePos) strcat(elsePart, " ");
                        if (tokens[i].type == TOK_STRING) {
                            strcat(elsePart, "\"");
                            strcat(elsePart, tokens[i].value);
                            strcat(elsePart, "\"");
                        } else {
                            strcat(elsePart, tokens[i].value);
                        }
                    }
                    
                    if (strlen(elsePart) > 0) {
                        if (strncmp(elsePart, "GOTO ", 5) == 0) {
                            /* GOTO dans ELSE */
                            targetLine = atoi(&elsePart[5]);
                            target = interp->program;
                            while (target && target->lineNum != targetLine) {
                                target = target->next;
                            }
                            if (target) {
                                line = target;
                                freeTokens(tokens);
                                continue;
                            }
                        } else {
                            executeCommand(interp, elsePart);
                        }
                    }
                }
            }
        }
        else if (tokens[0].type == TOK_GOTO) {
            if (tokens[1].type == TOK_NUMBER) {
                targetLine = atoi(tokens[1].value);
                target = interp->program;
                while (target && target->lineNum != targetLine) {
                    target = target->next;
                }
                if (target) {
                    line = target;
                    freeTokens(tokens);
                    continue;
                }
            }
        }
        else if (tokens[0].type == TOK_GOSUB) {
            /* GOSUB ligne_cible - Empile la ligne de retour et saute */
            if (tokens[1].type == TOK_NUMBER) {
                CallStack *newCall;
                
                targetLine = atoi(tokens[1].value);
                target = interp->program;
                while (target && target->lineNum != targetLine) {
                    target = target->next;
                }
                if (target) {
                    /* Empiler la ligne suivante pour le RETURN */
                    newCall = malloc(sizeof(CallStack));
                    newCall->returnLine = line->next;
                    newCall->next = interp->callStack;
                    interp->callStack = newCall;
                    
                    /* Sauter à la sous-routine */
                    line = target;
                    freeTokens(tokens);
                    continue;
                }
            }
        }
        else if (tokens[0].type == TOK_RETURN) {
            /* RETURN - Dépile et retourne à la ligne après GOSUB */
            if (interp->callStack) {
                CallStack *topCall = interp->callStack;
                line = topCall->returnLine;
                interp->callStack = topCall->next;
                free(topCall);
                freeTokens(tokens);
                continue;
            }
        }
        else if (tokens[0].type == TOK_FOR) {
            /* FOR variable = start TO end [STEP step] */
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
                            Line *searchLine;
                            Token *searchTokens;
                            int nestLevel;
                            
                            setVariable(interp, varName, startVal);
                            nestLevel = 1;
                            searchLine = line->next;
                            
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
                                line = searchLine;
                                freeTokens(tokens);
                                continue;
                            }
                        } else {
                            /* Créer une nouvelle boucle FOR */
                            forLoop = malloc(sizeof(ForLoop));
                            forLoop->varName = malloc(strlen(varName) + 1);
                            strcpy(forLoop->varName, varName);
                            forLoop->endValue = endVal;
                            forLoop->stepValue = stepVal;
                            forLoop->startLine = line;
                            forLoop->next = interp->forStack;
                            interp->forStack = forLoop;
                            /* Initialiser la variable */
                            setVariable(interp, varName, startVal);
                        }
                    }
                }
            }
        }
        else if (tokens[0].type == TOK_NEXT) {
            /* NEXT variable */
            if (interp->forStack) {
                forLoop = interp->forStack;
                currentVal = getVariable(interp, forLoop->varName);
                currentVal += forLoop->stepValue;
                setVariable(interp, forLoop->varName, currentVal);
                
                /* Vérifier si la boucle doit continuer */
                if ((forLoop->stepValue > 0 && currentVal <= forLoop->endValue) ||
                    (forLoop->stepValue < 0 && currentVal >= forLoop->endValue)) {
                    /* Retourner à la ligne après le FOR */
                    line = forLoop->startLine->next;
                    freeTokens(tokens);
                    continue;
                } else {
                    /* Sortir de la boucle */
                    interp->forStack = forLoop->next;
                    free(forLoop->varName);
                    free(forLoop);
                }
            }
        }
        else {
            executeCommand(interp, line->code);
        }
        
        freeTokens(tokens);
        line = line->next;
    }
}