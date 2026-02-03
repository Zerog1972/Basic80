#include "commands.h"
#include "expression.h"
#include "variables.h"
#include <stdio.h>
#include <string.h>

/* Helper pour vérifier le suffixe $ */
static int isStringVariable(const char *name) {
    size_t len = strlen(name);
    return len > 0 && name[len - 1] == '$';
}

/* ===== COMMANDE PRINT ===== */

void handlePrint(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    
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
        } else if (tokens[pos].type == TOK_SEMICOLON) {
            /* Point-virgule : pas d'espace entre les éléments */
            pos++;
        }
    }
    printf("\n");
}

/* ===== COMMANDE LET ===== */

void handleLet(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    char varName[256];
    
    /* Détecter si on a LET ou une affectation directe */
    pos = (tokens[0].type == TOK_LET) ? 1 : 0;
    
    if (tokens[pos].type != TOK_IDENTIFIER) {
        reportError(interp, "Nom de variable attendu après LET.");
        return;
    }
    
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
        if (tokens[pos].type != TOK_RPAREN) {
            reportError(interp, "')' attendu après les indices du tableau.");
            return;
        }
        pos++;
        if (tokens[pos].type != TOK_EQUALS) {
            reportError(interp, "'=' attendu dans l'affectation.");
            return;
        }
        pos++;
        val = evaluateExpression(interp, tokens, &pos);
        setArrayElement(interp, varName, indices, numIndices, val);
    } else if (tokens[pos].type == TOK_EQUALS) {
        pos++;
        
        if (isStringVariable(varName)) {
            char *strResult = evaluateStringExpression(interp, tokens, &pos);
            setStringVariable(interp, varName, strResult);
            free(strResult);
        } else {
            if (isStringExpression(interp, tokens, pos)) {
                reportError(interp, "Impossible d'assigner une chaîne à une variable numérique.");
                return;
            }
            val = evaluateExpression(interp, tokens, &pos);
            setVariable(interp, varName, val);
        }
    } else {
        reportError(interp, "'=' ou '(' attendu après le nom de variable.");
    }
}

/* ===== COMMANDE DIM ===== */

void handleDim(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[256];
    int dims[10]; /* Maximum 10 dimensions */
    int numDims;
    
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

/* ===== COMMANDE INPUT ===== */

void handleInput(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[256];
    char buffer[1024];
    
    pos = 1;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        strcpy(varName, tokens[pos].value);
        
        printf("? ");
        if (isStringVariable(varName)) {
            if (fgets(buffer, sizeof(buffer), stdin)) {
                buffer[strcspn(buffer, "\n")] = 0;
                setStringVariable(interp, varName, buffer);
            }
        } else {
            double val;
            if (scanf("%lf", &val) == 1) {
                /* Consommer le reste de la ligne */
                int c;
                while ((c = getchar()) != '\n' && c != EOF);
                setVariable(interp, varName, val);
            }
        }
    }
}
/* ===== COMMANDES DATA/READ/RESTORE ===== */

void handleData(Interpreter *interp, Token *tokens, int lineNum) {
    int pos;
    DataItem *newItem;
    DataItem *last;
    char buffer[1024];
    
    pos = 1; /* Sauter le token DATA */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Ignorer les virgules */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        /* Créer un nouvel élément DATA */
        newItem = malloc(sizeof(DataItem));
        newItem->lineNum = lineNum;
        newItem->next = NULL;
        
        /* Copier la valeur selon le type */
        if (tokens[pos].type == TOK_STRING) {
            newItem->value = malloc(strlen(tokens[pos].value) + 1);
            strcpy(newItem->value, tokens[pos].value);
        } else if (tokens[pos].type == TOK_NUMBER) {
            newItem->value = malloc(strlen(tokens[pos].value) + 1);
            strcpy(newItem->value, tokens[pos].value);
        } else if (tokens[pos].type == TOK_MINUS && tokens[pos + 1].type == TOK_NUMBER) {
            /* Gérer les nombres négatifs */
            snprintf(buffer, sizeof(buffer), "-%s", tokens[pos + 1].value);
            newItem->value = malloc(strlen(buffer) + 1);
            strcpy(newItem->value, buffer);
            pos++; /* Sauter le nombre après le moins */
        } else {
            /* Token non reconnu, ignorer */
            free(newItem);
            pos++;
            continue;
        }
        
        /* Ajouter à la fin de la liste */
        if (!interp->dataList) {
            interp->dataList = newItem;
            interp->dataPointer = newItem;
        } else {
            last = interp->dataList;
            while (last->next) {
                last = last->next;
            }
            last->next = newItem;
        }
        
        pos++;
    }
}

void handleRead(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[256];
    
    pos = 1; /* Sauter le token READ */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Ignorer les virgules */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        if (tokens[pos].type != TOK_IDENTIFIER) {
            reportError(interp, "Nom de variable attendu après READ");
            return;
        }
        
        strcpy(varName, tokens[pos].value);
        
        /* Vérifier s'il y a des données disponibles */
        if (!interp->dataPointer) {
            reportError(interp, "Out of DATA");
            return;
        }
        
        /* Lire la valeur selon le type de variable */
        if (isStringVariable(varName)) {
            /* Variable chaîne */
            setStringVariable(interp, varName, interp->dataPointer->value);
        } else {
            /* Variable numérique */
            double val = atof(interp->dataPointer->value);
            setVariable(interp, varName, val);
        }
        
        /* Avancer le pointeur DATA */
        interp->dataPointer = interp->dataPointer->next;
        
        pos++;
    }
}

void handleRestore(Interpreter *interp, Token *tokens) {
    int targetLine;
    DataItem *item;
    
    /* Si RESTORE sans argument, revenir au début */
    if (tokens[1].type == TOK_EOF) {
        interp->dataPointer = interp->dataList;
        return;
    }
    
    /* Si RESTORE avec numéro de ligne */
    if (tokens[1].type == TOK_NUMBER) {
        targetLine = (int)atof(tokens[1].value);
        
        /* Chercher le premier DATA à cette ligne */
        item = interp->dataList;
        while (item && item->lineNum < targetLine) {
            item = item->next;
        }
        
        if (item && item->lineNum == targetLine) {
            interp->dataPointer = item;
        } else {
            /* Si la ligne n'existe pas, remettre au début */
            interp->dataPointer = interp->dataList;
        }
    } else {
        interp->dataPointer = interp->dataList;
    }
}
