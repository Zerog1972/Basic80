#include "../include/expr.h"
#include "../include/interp.h"
#include "../include/lexer.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    char *result;
    
    interp = createInterpreter();
    
    printf("Test MID$ avec \"Hello\", 2, 3:\n");
    tokens = tokenize("MID$(\"Hello\", 2, 3)");
    
    /* Afficher les tokens */
    printf("Tokens: ");
    pos = 0;
    while (tokens[pos].type != TOK_EOF) {
        printf("[%d:%s] ", tokens[pos].type, tokens[pos].value);
        pos++;
    }
    printf("\n");
    
    pos = 0;
    result = evaluateStringExpression(interp, tokens, &pos);
    
    if (result) {
        printf("Resultat: '%s'\n", result);
        printf("Longueur: %d\n", (int)strlen(result));
        printf("Attendu: 'ell'\n");
        
        /* Afficher chaque caractere en hexa */
        printf("Caracteres: ");
        for (int i = 0; result[i]; i++) {
            printf("[%02X]", (unsigned char)result[i]);
        }
        printf("\n");
        
        free(result);
    } else {
        printf("Erreur: result est NULL\n");
    }
    
    freeTokens(tokens);
    freeInterpreter(interp);
    
    return 0;
}
