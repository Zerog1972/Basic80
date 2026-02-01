#include "lexer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* Fonction privée pour vérifier si un mot est un mot-clé */
static int isKeyword(const char *word, BasicTokenType *type) {
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
    if (strcmp(word, "ATAN") == 0) { *type = TOK_ATAN; return 1; }
    if (strcmp(word, "ASIN") == 0) { *type = TOK_ASIN; return 1; }
    if (strcmp(word, "ACOS") == 0) { *type = TOK_ACOS; return 1; }
    if (strcmp(word, "SINH") == 0) { *type = TOK_SINH; return 1; }
    if (strcmp(word, "COSH") == 0) { *type = TOK_COSH; return 1; }
    if (strcmp(word, "TANH") == 0) { *type = TOK_TANH; return 1; }
    if (strcmp(word, "SQR") == 0) { *type = TOK_SQR; return 1; }
    if (strcmp(word, "ABS") == 0) { *type = TOK_ABS; return 1; }
    if (strcmp(word, "INT") == 0) { *type = TOK_INT; return 1; }
    if (strcmp(word, "RND") == 0) { *type = TOK_RND; return 1; }
    if (strcmp(word, "LOG") == 0) { *type = TOK_LOG; return 1; }
    if (strcmp(word, "EXP") == 0) { *type = TOK_EXP; return 1; }
    if (strcmp(word, "POW") == 0) { *type = TOK_POW; return 1; }
    if (strcmp(word, "LOG10") == 0) { *type = TOK_LOG10; return 1; }
    if (strcmp(word, "DEG") == 0) { *type = TOK_DEG; return 1; }
    if (strcmp(word, "RAD") == 0) { *type = TOK_RAD; return 1; }
    if (strcmp(word, "LEN") == 0) { *type = TOK_LEN; return 1; }
    if (strcmp(word, "MID") == 0) { *type = TOK_MID; return 1; }
    if (strcmp(word, "LEFT") == 0) { *type = TOK_LEFT; return 1; }
    if (strcmp(word, "RIGHT") == 0) { *type = TOK_RIGHT; return 1; }
    if (strcmp(word, "CHR") == 0) { *type = TOK_CHR; return 1; }
    if (strcmp(word, "ASC") == 0) { *type = TOK_ASC; return 1; }
    return 0;
}

/* Tokenize une ligne de code BASIC */
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
            
            /* Support du suffixe $ pour les variables chaine */
            if (i < len && line[i] == '$') {
                i++;
            }
            
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

/* Libère la mémoire des tokens */
void freeTokens(Token *tokens) {
    int i;
    for (i = 0; tokens[i].type != TOK_EOF; i++) {
        if (tokens[i].value) free(tokens[i].value);
    }
    free(tokens);
}
