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
        }
    }
    printf("\n");
}

/* ===== COMMANDE LET ===== */

void handleLet(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    char varName[256];
    
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
            
            if (isStringVariable(varName)) {
                char *strResult = evaluateStringExpression(interp, tokens, &pos);
                setStringVariable(interp, varName, strResult);
                free(strResult);
            } else {
                if (isStringExpression(interp, tokens, pos)) {
                    printf("Erreur de type: Impossible d'assigner une chaine a une variable numerique '%s'\n", varName);
                    return;
                }
                val = evaluateExpression(interp, tokens, &pos);
                setVariable(interp, varName, val);
            }
        }
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
