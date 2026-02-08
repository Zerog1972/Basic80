/**
 * @file calculator.c
 * @brief Exemple concret d'utilisation des extensions Basic80
 * 
 * Ce programme demonstre comment creer et utiliser des extensions
 * personnalisees dans Basic80. Il implemente une calculatrice scientifique
 * avec des fonctions mathematiques avancees.
 * 
 * COMPILATION:
 *   clang -std=c89 -pedantic -Wall -g 
 *        examples/calculator.c 
 *        src/lexer.c src/variables.c src/expression.c 
 *        src/interpreter.c src/commands.c src/control_flow.c
 *        -Iinclude -o calculator.exe
 * 
 * UTILISATION:
 *   calculator.exe
 * 
 * @author Basic80 Team
 * @date 8 fevrier 2026
 */

#include "interpreter.h"
#include "expression.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ====================================================================
 * EXTENSIONS MATHEMATIQUES PERSONNALISEES
 * ==================================================================== */

/**
 * @brief SQUARE(x) - Retourne x au carre (x^2)
 */
static double func_square(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    return x * x;
}

/**
 * @brief CUBE(x) - Retourne x au cube (x^3)
 */
static double func_cube(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    return x * x * x;
}

/**
 * @brief POWER(base, exp) - Retourne base^exp
 */
static double func_power(Interpreter *interp, Token *tokens, int *pos) {
    double base = evaluateExpression(interp, tokens, pos);
    double exponent;
    
    if (tokens[*pos].type == TOK_COMMA) {
        (*pos)++;
    }
    
    exponent = evaluateExpression(interp, tokens, pos);
    return pow(base, exponent);
}

/**
 * @brief SQRT(x) - Racine carree
 */
static double func_sqrt(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    
    if (x < 0.0) {
        reportErrorEx(interp, ERR_RUNTIME, *pos, 
                      "SQRT: argument negatif");
        return 0.0;
    }
    
    return sqrt(x);
}

/**
 * @brief LOG(x) - Logarithme naturel (base e)
 */
static double func_log(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    
    if (x <= 0.0) {
        reportErrorEx(interp, ERR_RUNTIME, *pos, 
                      "LOG: argument doit etre > 0");
        return 0.0;
    }
    
    return log(x);
}

/**
 * @brief LOG10(x) - Logarithme base 10
 */
static double func_log10(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    
    if (x <= 0.0) {
        reportErrorEx(interp, ERR_RUNTIME, *pos, 
                      "LOG10: argument doit etre > 0");
        return 0.0;
    }
    
    return log10(x);
}

/**
 * @brief CEIL(x) - Arrondi vers le haut
 */
static double func_ceil(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    return ceil(x);
}

/**
 * @brief FLOOR(x) - Arrondi vers le bas
 */
static double func_floor(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    return floor(x);
}

/**
 * @brief ROUND(x) - Arrondi au plus proche
 */
static double func_round(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    return floor(x + 0.5);
}

/**
 * @brief MIN(a, b) - Retourne le minimum de deux nombres
 */
static double func_min(Interpreter *interp, Token *tokens, int *pos) {
    double a = evaluateExpression(interp, tokens, pos);
    double b;
    
    if (tokens[*pos].type == TOK_COMMA) {
        (*pos)++;
    }
    
    b = evaluateExpression(interp, tokens, pos);
    return (a < b) ? a : b;
}

/**
 * @brief MAX(a, b) - Retourne le maximum de deux nombres
 */
static double func_max(Interpreter *interp, Token *tokens, int *pos) {
    double a = evaluateExpression(interp, tokens, pos);
    double b;
    
    if (tokens[*pos].type == TOK_COMMA) {
        (*pos)++;
    }
    
    b = evaluateExpression(interp, tokens, pos);
    return (a > b) ? a : b;
}

/**
 * @brief HYPOT(x, y) - Retourne sqrt(x^2 + y^2)
 */
static double func_hypot(Interpreter *interp, Token *tokens, int *pos) {
    double x = evaluateExpression(interp, tokens, pos);
    double y;
    
    if (tokens[*pos].type == TOK_COMMA) {
        (*pos)++;
    }
    
    y = evaluateExpression(interp, tokens, pos);
    return sqrt(x * x + y * y);
}

/**
 * @brief DEG2RAD(deg) - Convertit degres en radians
 */
static double func_deg2rad(Interpreter *interp, Token *tokens, int *pos) {
    double deg = evaluateExpression(interp, tokens, pos);
    return deg * 3.14159265358979323846 / 180.0;
}

/**
 * @brief RAD2DEG(rad) - Convertit radians en degres
 */
static double func_rad2deg(Interpreter *interp, Token *tokens, int *pos) {
    double rad = evaluateExpression(interp, tokens, pos);
    return rad * 180.0 / 3.14159265358979323846;
}

/* ====================================================================
 * EXTENSIONS DE CHAINES
 * ==================================================================== */

/**
 * @brief TRIM$(str) - Supprime les espaces au debut et a la fin
 */
static char* func_trim(Interpreter *interp, Token *tokens, int *pos) {
    char *str = evaluateStringExpression(interp, tokens, pos);
    char *result;
    int start, end, len, i;
    
    /* Trouver le debut (sans espaces) */
    start = 0;
    while (str[start] == ' ' || str[start] == '\t') {
        start++;
    }
    
    /* Trouver la fin (sans espaces) */
    end = (int)strlen(str) - 1;
    while (end >= start && (str[end] == ' ' || str[end] == '\t')) {
        end--;
    }
    
    /* Copier la sous-chaine */
    len = end - start + 1;
    result = (char*)malloc(len + 1);
    if (!result) {
        free(str);
        result = (char*)malloc(1);
        if (result) result[0] = '\0';
        return result;
    }
    
    for (i = 0; i < len; i++) {
        result[i] = str[start + i];
    }
    result[len] = '\0';
    
    free(str);
    return result;
}

/**
 * @brief REPLACE$(str, old, new) - Remplace premiere occurrence
 */
static char* func_replace(Interpreter *interp, Token *tokens, int *pos) {
    char *str = evaluateStringExpression(interp, tokens, pos);
    char *oldStr;
    char *newStr;
    char *result;
    char *found;
    int oldLen, newLen, beforeLen, afterLen, totalLen;
    
    if (tokens[*pos].type == TOK_COMMA) (*pos)++;
    oldStr = evaluateStringExpression(interp, tokens, pos);
    
    if (tokens[*pos].type == TOK_COMMA) (*pos)++;
    newStr = evaluateStringExpression(interp, tokens, pos);
    
    /* Chercher oldStr dans str */
    found = strstr(str, oldStr);
    
    if (!found) {
        /* Pas trouve, retourner l'original */
        free(oldStr);
        free(newStr);
        return str;
    }
    
    /* Calculer les tailles */
    oldLen = (int)strlen(oldStr);
    newLen = (int)strlen(newStr);
    beforeLen = (int)(found - str);
    afterLen = (int)strlen(found + oldLen);
    totalLen = beforeLen + newLen + afterLen;
    
    /* Allouer le resultat */
    result = (char*)malloc(totalLen + 1);
    if (!result) {
        free(str);
        free(oldStr);
        free(newStr);
        result = (char*)malloc(1);
        if (result) result[0] = '\0';
        return result;
    }
    
    /* Copier: avant + nouveau + apres */
    memcpy(result, str, beforeLen);
    memcpy(result + beforeLen, newStr, newLen);
    memcpy(result + beforeLen + newLen, found + oldLen, afterLen);
    result[totalLen] = '\0';
    
    free(str);
    free(oldStr);
    free(newStr);
    return result;
}

/* ====================================================================
 * COMMANDES UTILITAIRES
 * ==================================================================== */

/**
 * @brief VERSION - Affiche la version de la calculatrice
 */
static void cmd_version(Interpreter *interp, Token *tokens) {
    (void)interp;
    (void)tokens;
    printf("Calculatrice Scientifique Basic80 v1.0\n");
    printf("Extensions chargees: 14 fonctions mathematiques + 2 fonctions chaines\n");
}

/**
 * @brief FUNCS - Affiche la liste des fonctions disponibles
 */
static void cmd_funcs(Interpreter *interp, Token *tokens) {
    (void)interp;
    (void)tokens;
    printf("\n=== FONCTIONS MATHEMATIQUES DISPONIBLES ===\n\n");
    printf("Puissances et racines:\n");
    printf("  SQUARE(x)      - x au carre\n");
    printf("  CUBE(x)        - x au cube\n");
    printf("  POWER(x, y)    - x puissance y\n");
    printf("  SQRT(x)        - Racine carree\n");
    printf("\n");
    printf("Logarithmes:\n");
    printf("  LOG(x)         - Logarithme naturel (base e)\n");
    printf("  LOG10(x)       - Logarithme base 10\n");
    printf("\n");
    printf("Arrondissements:\n");
    printf("  CEIL(x)        - Arrondi vers le haut\n");
    printf("  FLOOR(x)       - Arrondi vers le bas\n");
    printf("  ROUND(x)       - Arrondi au plus proche\n");
    printf("\n");
    printf("Comparaisons:\n");
    printf("  MIN(a, b)      - Minimum\n");
    printf("  MAX(a, b)      - Maximum\n");
    printf("\n");
    printf("Geometrie:\n");
    printf("  HYPOT(x, y)    - Hypotenuse (distance)\n");
    printf("\n");
    printf("Conversions:\n");
    printf("  DEG2RAD(deg)   - Degres vers radians\n");
    printf("  RAD2DEG(rad)   - Radians vers degres\n");
    printf("\n");
    printf("Chaines:\n");
    printf("  TRIM$(str)         - Supprime espaces debut/fin\n");
    printf("  REPLACE$(s,o,n)    - Remplace premiere occurrence\n");
    printf("\n");
}

/* ====================================================================
 * ENREGISTREMENT ET PROGRAMME PRINCIPAL
 * ==================================================================== */

/**
 * @brief Enregistre toutes les extensions dans l'interpreteur
 */
static void registerCalculatorExtensions(Interpreter *interp) {
    /* Fonctions mathematiques */
    registerCustomNumericFunction(interp, "SQUARE", func_square);
    registerCustomNumericFunction(interp, "CUBE", func_cube);
    registerCustomNumericFunction(interp, "POWER", func_power);
    registerCustomNumericFunction(interp, "SQRT", func_sqrt);
    registerCustomNumericFunction(interp, "LOG", func_log);
    registerCustomNumericFunction(interp, "LOG10", func_log10);
    registerCustomNumericFunction(interp, "CEIL", func_ceil);
    registerCustomNumericFunction(interp, "FLOOR", func_floor);
    registerCustomNumericFunction(interp, "ROUND", func_round);
    registerCustomNumericFunction(interp, "MIN", func_min);
    registerCustomNumericFunction(interp, "MAX", func_max);
    registerCustomNumericFunction(interp, "HYPOT", func_hypot);
    registerCustomNumericFunction(interp, "DEG2RAD", func_deg2rad);
    registerCustomNumericFunction(interp, "RAD2DEG", func_rad2deg);
    
    /* Fonctions chaines */
    registerCustomStringFunction(interp, "TRIM$", func_trim);
    registerCustomStringFunction(interp, "REPLACE$", func_replace);
    
    /* Commandes */
    registerCustomCommand(interp, "VERSION", cmd_version);
    registerCustomCommand(interp, "FUNCS", cmd_funcs);
}

/**
 * @brief Point d'entree du programme
 */
int main(void) {
    Interpreter *interp;
    char input[1024];
    
    printf("========================================\n");
    printf("  CALCULATRICE SCIENTIFIQUE BASIC80\n");
    printf("========================================\n\n");
    
    /* Creer l'interpreteur */
    interp = createInterpreter();
    
    /* Charger les extensions */
    registerCalculatorExtensions(interp);
    
    printf("Extensions chargees avec succes!\n");
    printf("Tapez FUNCS pour voir la liste des fonctions.\n");
    printf("Tapez VERSION pour voir les infos.\n");
    printf("Tapez EXIT pour quitter.\n\n");
    
    /* Quelques exemples */
    printf("Exemples d'utilisation:\n");
    printf("  PRINT SQUARE(5)           ' Affiche: 25\n");
    printf("  PRINT POWER(2, 10)        ' Affiche: 1024\n");
    printf("  PRINT SQRT(144)           ' Affiche: 12\n");
    printf("  PRINT MAX(10, 25)         ' Affiche: 25\n");
    printf("  PRINT HYPOT(3, 4)         ' Affiche: 5\n");
    printf("  PRINT DEG2RAD(90)         ' Affiche: 1.57...\n");
    printf("\n");
    
    /* Boucle interactive */
    while (1) {
        printf("calc> ");
        fflush(stdout);
        
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }
        
        /* Retirer le newline */
        {
            size_t len = strlen(input);
            if (len > 0 && input[len-1] == '\n') {
                input[len-1] = '\0';
            }
        }
        
        /* Commandes speciales */
        if (strcmp(input, "EXIT") == 0 || strcmp(input, "QUIT") == 0) {
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
        
        /* Executer la commande */
        executeCommand(interp, input);
        
        /* Afficher les erreurs */
        if (interp->hasError) {
            interp->hasError = 0;
        }
        
        printf("\n");
    }
    
    /* Nettoyer */
    freeInterpreter(interp);
    printf("\nAu revoir!\n");
    
    return 0;
}
