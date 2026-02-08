#include "commands.h"
#include "expression.h"
#include "variables.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Constantes de sécurité */
#define MAX_VARNAME_LEN 256
#define MAX_INPUT_BUFFER 1024
#define MAX_ARRAY_DIMENSIONS 10

/**
 * Vérifie si un nom de variable est une variable chaîne.
 * 
 * En BASIC, les variables chaînes se terminent par le suffixe '$'.
 * 
 * @param name Nom de la variable à vérifier
 * @return 1 si c'est une variable chaîne, 0 sinon
 */
static int isStringVariable(const char *name) {
    size_t len = strlen(name);
    return len > 0 && name[len - 1] == '$';
}

/* ===== COMMANDE PRINT ===== */

/**
 * Traite la commande PRINT.
 * 
 * Affiche une ou plusieurs expressions à l'écran. Supporte les expressions
 * numériques et les chaînes de caractères. Les séparateurs virgule et point-virgule
 * contrôlent l'espacement entre les éléments.
 * 
 * Syntaxe: PRINT [expression [,|; expression]...]
 * - virgule: ajoute un espace entre les éléments
 * - point-virgule: pas d'espace entre les éléments
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les expressions à afficher
 */
void handlePrint(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    
    pos = 1;
    while (tokens[pos].type != TOK_EOF) {
        if (isStringExpression(interp, tokens, pos)) {
            char *strResult = evaluateStringExpression(interp, tokens, &pos);
            if (!strResult) {
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour l'évaluation de chaîne.");
                return;
            }
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

/**
 * Traite la commande LET (affectation de variable).
 * 
 * Assigne une valeur à une variable simple ou à un élément de tableau.
 * Le mot-clé LET est optionnel en BASIC. Supporte les variables numériques
 * et les variables chaînes (terminées par $).
 * 
 * Syntaxes:
 * - [LET] variable = expression
 * - [LET] array(indices) = expression
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant l'affectation
 */
void handleLet(Interpreter *interp, Token *tokens) {
    int pos;
    double val;
    char varName[MAX_VARNAME_LEN];
    
    /* Détecter si on a LET ou une affectation directe */
    pos = (tokens[0].type == TOK_LET) ? 1 : 0;
    
    if (tokens[pos].type != TOK_IDENTIFIER) {
        reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après LET.");
        return;
    }
    
    strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
    varName[MAX_VARNAME_LEN - 1] = '\0';
    pos++;
    
    /* Vérifier si c'est une affectation de tableau */
    if (tokens[pos].type == TOK_LPAREN) {
        int indices[MAX_ARRAY_DIMENSIONS];
        int numIndices;
        
        pos++;
        numIndices = 0;
        /* Lire tous les indices séparés par des virgules */
        while (numIndices < MAX_ARRAY_DIMENSIONS) {
            indices[numIndices] = (int)evaluateExpression(interp, tokens, &pos);
            numIndices++;
            if (tokens[pos].type == TOK_COMMA) {
                pos++; /* Passer la virgule */
            } else {
                break; /* Fin des indices */
            }
        }        /* Vérifier si on a dépassé le nombre maximum de dimensions */
        if (numIndices >= MAX_ARRAY_DIMENSIONS && tokens[pos].type == TOK_COMMA) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "Trop de dimensions (maximum 10).");
            return;
        }        if (tokens[pos].type != TOK_RPAREN) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "')' attendu après les indices du tableau.");
            return;
        }
        pos++;
        if (tokens[pos].type != TOK_EQUALS) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "'=' attendu dans l'affectation.");
            return;
        }
        pos++;
        val = evaluateExpression(interp, tokens, &pos);
        setArrayElement(interp, varName, indices, numIndices, val);
    } else if (tokens[pos].type == TOK_EQUALS) {
        pos++;
        
        if (isStringVariable(varName)) {
            char *strResult = evaluateStringExpression(interp, tokens, &pos);
            if (!strResult) {
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour l'évaluation de chaîne.");
                return;
            }
            setStringVariable(interp, varName, strResult);
            free(strResult);
        } else {
            if (isStringExpression(interp, tokens, pos)) {
                reportErrorEx(interp, ERR_TYPE_MISMATCH, pos, "Impossible d'assigner une chaîne à une variable numérique.");
                return;
            }
            val = evaluateExpression(interp, tokens, &pos);
            setVariable(interp, varName, val);
        }
    } else {
        reportErrorEx(interp, ERR_SYNTAX, pos, "'=' ou '(' attendu après le nom de variable.");
    }
}

/* ===== COMMANDE DIM ===== */

/**
 * Traite la commande DIM (déclaration de tableau).
 * 
 * Déclare un tableau multi-dimensionnel avec les tailles spécifiées.
 * Supporte jusqu'à 10 dimensions. Les indices commencent à 0 en BASIC,
 * donc DIM A(10) crée un tableau de 11 éléments (0 à 10).
 * 
 * Syntaxe: DIM array(taille1 [, taille2, ...])
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant la déclaration du tableau
 */
void handleDim(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    int dims[MAX_ARRAY_DIMENSIONS];
    int numDims;
    
    pos = 1;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
        varName[MAX_VARNAME_LEN - 1] = '\0';
        pos++;
        if (tokens[pos].type == TOK_LPAREN) {
            pos++;
            numDims = 0;
            /* Lire toutes les dimensions séparées par des virgules */
            while (numDims < MAX_ARRAY_DIMENSIONS) {
                int dimValue = (int)evaluateExpression(interp, tokens, &pos);
                /* Valider que la dimension est positive */
                if (dimValue < 0) {
                    reportErrorEx(interp, ERR_SYNTAX, pos, "Dimension de tableau négative.");
                    return;
                }
                dims[numDims] = dimValue + 1; /* +1 car BASIC commence à 0 */
                numDims++;
                if (tokens[pos].type == TOK_COMMA) {
                    pos++; /* Passer la virgule */
                } else {
                    break; /* Fin des dimensions */
                }
            }
            /* Vérifier si on a dépassé le nombre maximum de dimensions */
            if (numDims >= MAX_ARRAY_DIMENSIONS && tokens[pos].type == TOK_COMMA) {
                reportErrorEx(interp, ERR_SYNTAX, pos, "Trop de dimensions (maximum 10).");
                return;
            }
            if (tokens[pos].type != TOK_RPAREN) {
                reportErrorEx(interp, ERR_SYNTAX, pos, "')' attendu après les dimensions du tableau.");
                return;
            }
            pos++;
            createArray(interp, varName, dims, numDims);
        }
    }
}

/* ===== COMMANDE INPUT ===== */

/**
 * Traite la commande INPUT.
 * 
 * Demande à l'utilisateur de saisir une valeur au clavier et l'assigne
 * à la variable spécifiée. Pour les variables chaînes (terminées par $),
 * lit une ligne de texte. Pour les variables numériques, lit un nombre.
 * 
 * Syntaxe: INPUT variable
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant le nom de la variable
 */
void handleInput(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    char buffer[MAX_INPUT_BUFFER];
    
    pos = 1;
    if (tokens[pos].type != TOK_IDENTIFIER) {
        reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après INPUT.");
        return;
    }
    
    strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
    varName[MAX_VARNAME_LEN - 1] = '\0';
    
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
        } else {
            /* Erreur de lecture : nettoyer le buffer */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            reportErrorEx(interp, ERR_SYNTAX, pos, "Entrée numérique invalide.");
        }
    }
}

/* ===== COMMANDES DATA/READ/RESTORE ===== */

/**
 * Traite la commande DATA.
 * 
 * Stocke des valeurs constantes dans une liste chaînée pour lecture ultérieure
 * par READ. Les données peuvent être numériques ou des chaînes.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les valeurs DATA
 * @param lineNum Numéro de ligne pour RESTORE
 */
void handleData(Interpreter *interp, Token *tokens, int lineNum) {
    int pos;
    DataItem *newItem;
    DataItem *last;
    char buffer[MAX_INPUT_BUFFER];
    
    pos = 1; /* Sauter le token DATA */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Ignorer les virgules */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        /* Créer un nouvel élément DATA */
        newItem = malloc(sizeof(DataItem));
        if (!newItem) {
            reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour DATA");
            return;
        }
        newItem->lineNum = lineNum;
        newItem->next = NULL;
        
        /* Copier la valeur selon le type */
        if (tokens[pos].type == TOK_STRING) {
            newItem->value = malloc(strlen(tokens[pos].value) + 1);
            if (!newItem->value) {
                free(newItem);
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour DATA");
                return;
            }
            strcpy(newItem->value, tokens[pos].value);
        } else if (tokens[pos].type == TOK_NUMBER) {
            newItem->value = malloc(strlen(tokens[pos].value) + 1);
            if (!newItem->value) {
                free(newItem);
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour DATA");
                return;
            }
            strcpy(newItem->value, tokens[pos].value);
        } else if (tokens[pos].type == TOK_MINUS && tokens[pos + 1].type == TOK_NUMBER) {
            /* Gérer les nombres négatifs */
            snprintf(buffer, sizeof(buffer), "-%s", tokens[pos + 1].value);
            newItem->value = malloc(strlen(buffer) + 1);
            if (!newItem->value) {
                free(newItem);
                reportErrorEx(interp, ERR_OUT_OF_MEMORY, pos, "Mémoire insuffisante pour DATA");
                return;
            }
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

/**
 * Traite la commande READ.
 * 
 * Lit séquentiellement les valeurs depuis les instructions DATA et les
 * assigne aux variables spécifiées.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les noms de variables
 */
void handleRead(Interpreter *interp, Token *tokens) {
    int pos;
    char varName[MAX_VARNAME_LEN];
    
    pos = 1; /* Sauter le token READ */
    
    while (tokens[pos].type != TOK_EOF) {
        /* Ignorer les virgules */
        if (tokens[pos].type == TOK_COMMA) {
            pos++;
            continue;
        }
        
        if (tokens[pos].type != TOK_IDENTIFIER) {
            reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après READ");
            return;
        }
        
        strncpy(varName, tokens[pos].value, MAX_VARNAME_LEN - 1);
        varName[MAX_VARNAME_LEN - 1] = '\0';
        
        /* Vérifier s'il y a des données disponibles */
        if (!interp->dataPointer) {
            reportErrorEx(interp, ERR_OUT_OF_DATA, pos, "Out of DATA");
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

/**
 * Traite la commande RESTORE.
 * 
 * Réinitialise le pointeur de lecture DATA. Sans argument, revient au début
 * de toutes les DATA. Avec un numéro de ligne, positionne au premier DATA
 * de cette ligne.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens (peut contenir un numéro de ligne optionnel)
 */
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

/* ===== COMMANDE HELP ===== */

/**
 * Traite la commande HELP.
 * 
 * Affiche l'aide du système BASIC80. Sans argument, affiche la liste
 * complète des commandes disponibles. Avec un nom de commande en argument,
 * affiche l'aide détaillée de cette commande spécifique.
 * 
 * Syntaxe: HELP [commande]
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens (peut contenir un nom de commande optionnel)
 */
void handleHelp(Interpreter *interp, Token *tokens) {
    char cmdName[MAX_VARNAME_LEN];
    int i;
    
    /* Si HELP sans argument, afficher la liste des commandes */
    if (tokens[1].type == TOK_EOF) {
        printf("=== BASIC80 - Commandes disponibles ===\n\n");
        printf("Commandes de base:\n");
        printf("  PRINT    - Afficher du texte ou des valeurs\n");
        printf("  LET      - Assigner une valeur a une variable\n");
        printf("  INPUT    - Saisir une valeur au clavier\n");
        printf("  REM      - Ajouter un commentaire\n\n");
        
        printf("Structures de controle:\n");
        printf("  IF       - Execution conditionnelle\n");
        printf("  FOR      - Boucle avec compteur\n");
        printf("  NEXT     - Fin de boucle FOR\n");
        printf("  GOTO     - Saut a une ligne\n");
        printf("  GOSUB    - Appeler un sous-programme\n");
        printf("  RETURN   - Retourner d'un sous-programme\n");
        printf("  END      - Terminer le programme\n\n");
        
        printf("Gestion des donnees:\n");
        printf("  DIM      - Declarer un tableau\n");
        printf("  DATA     - Definir des donnees\n");
        printf("  READ     - Lire des donnees\n");
        printf("  RESTORE  - Reinitialiser le pointeur DATA\n\n");
        
        printf("Fonctions mathematiques:\n");
        printf("  ABS      - Valeur absolue\n");
        printf("  SQR      - Racine carree\n");
        printf("  INT      - Partie entiere\n");
        printf("  SGN      - Signe d'un nombre\n");
        printf("  RND      - Nombre aleatoire\n");
        printf("  SIN/COS/TAN/ATN - Fonctions trigonometriques\n");
        printf("  EXP/LOG/LOG10   - Exponentielle et logarithmes\n\n");
        
        printf("Fonctions de chaines:\n");
        printf("  LEN      - Longueur d'une chaine\n");
        printf("  LEFT$/RIGHT$/MID$ - Extraction de sous-chaines\n");
        printf("  CHR$     - Caractere ASCII\n");
        printf("  ASC      - Code ASCII d'un caractere\n");
        printf("  STR$     - Convertir nombre en chaine\n");
        printf("  VAL      - Convertir chaine en nombre\n\n");
        
        printf("Commandes systeme:\n");
        printf("  LIST     - Afficher le programme\n");
        printf("  RUN      - Executer le programme\n");
        printf("  NEW      - Effacer le programme\n");
        printf("  SAVE     - Sauvegarder le programme\n");
        printf("  LOAD     - Charger un programme\n");
        printf("  EXIT     - Quitter l'interpreteur\n");
        printf("  HELP     - Afficher cette aide\n\n");
        
        printf("Tapez HELP [COMMANDE] ou HELP [FONCTION] pour plus de details.\n");
        printf("Exemples: HELP PRINT, HELP SIN, HELP LEN\n\n");
        return;
    }
    
    /* Si HELP avec un argument, afficher l'aide detaillee */
    /* Accepter n'importe quel token qui a une valeur (pas seulement TOK_IDENTIFIER) */
    if (tokens[1].type != TOK_EOF && tokens[1].value) {
        strncpy(cmdName, tokens[1].value, MAX_VARNAME_LEN - 1);
        cmdName[MAX_VARNAME_LEN - 1] = '\0';
        
        /* Convertir en majuscules pour comparaison */
        for (i = 0; cmdName[i]; i++) {
            cmdName[i] = toupper(cmdName[i]);
        }
        
        if (strcmp(cmdName, "PRINT") == 0) {
            printf("=== PRINT ===\n\n");
            printf("Syntaxe: PRINT [expression [,|; expression]...]\n\n");
            printf("Description:\n");
            printf("  Affiche une ou plusieurs expressions a l'ecran.\n");
            printf("  Les expressions peuvent etre numeriques ou des chaines.\n\n");
            printf("Separateurs:\n");
            printf("  , (virgule)      : Tabulation entre expressions\n");
            printf("  ; (point-virgule): Pas d'espace entre expressions\n");
            printf("  Fin de ligne     : Retour a la ligne automatique\n\n");
            printf("Exemples:\n");
            printf("  PRINT \"Hello\"           -> Affiche: Hello\n");
            printf("  PRINT 42                -> Affiche: 42.00\n");
            printf("  PRINT A, B              -> Affiche: 10.00 20.00\n");
            printf("  PRINT \"X=\"; X           -> Affiche: X=5.00\n\n");
        }
        else if (strcmp(cmdName, "LET") == 0) {
            printf("=== LET ===\n\n");
            printf("Syntaxe: [LET] variable = expression\n");
            printf("         [LET] array(indices) = expression\n\n");
            printf("Description:\n");
            printf("  Assigne une valeur a une variable ou un element de tableau.\n");
            printf("  Le mot-cle LET est optionnel.\n\n");
            printf("Exemples:\n");
            printf("  LET A = 42\n");
            printf("  X = 10 + 5\n");
            printf("  A$ = \"Hello\"\n");
            printf("  M(2,3) = 99\n\n");
        }
        else if (strcmp(cmdName, "INPUT") == 0) {
            printf("=== INPUT ===\n\n");
            printf("Syntaxe: INPUT variable\n\n");
            printf("Description:\n");
            printf("  Demande a l'utilisateur de saisir une valeur.\n");
            printf("  Pour les variables chaines (terminees par $), saisir du texte.\n");
            printf("  Pour les variables numeriques, saisir un nombre.\n\n");
            printf("Exemples:\n");
            printf("  INPUT A        -> Attend un nombre\n");
            printf("  INPUT NOM$     -> Attend du texte\n\n");
        }
        else if (strcmp(cmdName, "DIM") == 0) {
            printf("=== DIM ===\n\n");
            printf("Syntaxe: DIM array(taille1 [, taille2, ...])\n\n");
            printf("Description:\n");
            printf("  Declare un tableau multi-dimensionnel.\n");
            printf("  Supporte jusqu'a 10 dimensions.\n");
            printf("  Les indices commencent a 0.\n\n");
            printf("Exemples:\n");
            printf("  DIM A(10)         -> Tableau 1D de 11 elements (0-10)\n");
            printf("  DIM M(5, 8)       -> Tableau 2D de 6x9 elements\n");
            printf("  DIM T(3, 4, 5)    -> Tableau 3D\n\n");
        }
        else if (strcmp(cmdName, "IF") == 0) {
            printf("=== IF ===\n\n");
            printf("Syntaxe: IF condition THEN instruction [ELSE instruction]\n\n");
            printf("Description:\n");
            printf("  Execute une instruction si la condition est vraie.\n");
            printf("  La clause ELSE est optionnelle.\n\n");
            printf("Operateurs de comparaison:\n");
            printf("  =  : Egal\n");
            printf("  <> : Different\n");
            printf("  <  : Inferieur\n");
            printf("  >  : Superieur\n");
            printf("  <= : Inferieur ou egal\n");
            printf("  >= : Superieur ou egal\n\n");
            printf("Exemples:\n");
            printf("  IF X > 10 THEN PRINT \"Grand\"\n");
            printf("  IF A = B THEN X = 1 ELSE X = 2\n");
            printf("  IF A$ = \"OUI\" THEN GOTO 100\n\n");
        }
        else if (strcmp(cmdName, "FOR") == 0) {
            printf("=== FOR / NEXT ===\n\n");
            printf("Syntaxe: FOR variable = debut TO fin [STEP increment]\n");
            printf("         ... instructions ...\n");
            printf("         NEXT [variable]\n\n");
            printf("Description:\n");
            printf("  Boucle avec compteur.\n");
            printf("  STEP est optionnel (par defaut 1).\n\n");
            printf("Exemples:\n");
            printf("  FOR I = 1 TO 10\n");
            printf("    PRINT I\n");
            printf("  NEXT I\n\n");
            printf("  FOR J = 10 TO 1 STEP -1\n");
            printf("    PRINT J\n");
            printf("  NEXT J\n\n");
        }
        else if (strcmp(cmdName, "GOTO") == 0) {
            printf("=== GOTO ===\n\n");
            printf("Syntaxe: GOTO numeroLigne\n\n");
            printf("Description:\n");
            printf("  Saute a une ligne specifique du programme.\n\n");
            printf("Exemple:\n");
            printf("  10 PRINT \"Debut\"\n");
            printf("  20 GOTO 40\n");
            printf("  30 PRINT \"Saute\"\n");
            printf("  40 PRINT \"Fin\"\n\n");
        }
        else if (strcmp(cmdName, "GOSUB") == 0 || strcmp(cmdName, "RETURN") == 0) {
            printf("=== GOSUB / RETURN ===\n\n");
            printf("Syntaxe: GOSUB numeroLigne\n");
            printf("         RETURN\n\n");
            printf("Description:\n");
            printf("  GOSUB appelle un sous-programme a la ligne specifiee.\n");
            printf("  RETURN retourne a l'instruction suivant le GOSUB.\n\n");
            printf("Exemple:\n");
            printf("  10 GOSUB 100\n");
            printf("  20 PRINT \"Retour\"\n");
            printf("  30 END\n");
            printf("  100 PRINT \"Sous-programme\"\n");
            printf("  110 RETURN\n\n");
        }
        else if (strcmp(cmdName, "DATA") == 0 || strcmp(cmdName, "READ") == 0) {
            printf("=== DATA / READ / RESTORE ===\n\n");
            printf("Syntaxe: DATA valeur1, valeur2, ...\n");
            printf("         READ variable1, variable2, ...\n");
            printf("         RESTORE [numeroLigne]\n\n");
            printf("Description:\n");
            printf("  DATA definit des valeurs constantes dans le programme.\n");
            printf("  READ lit sequentiellement ces valeurs.\n");
            printf("  RESTORE reinitialise le pointeur de lecture.\n\n");
            printf("Exemple:\n");
            printf("  10 DATA 10, 20, 30\n");
            printf("  20 READ A, B, C\n");
            printf("  30 PRINT A; B; C    -> Affiche: 10.00 20.00 30.00\n\n");
        }
        else if (strcmp(cmdName, "RESTORE") == 0) {
            printf("=== RESTORE ===\n\n");
            printf("Syntaxe: RESTORE [numeroLigne]\n\n");
            printf("Description:\n");
            printf("  Reinitialise le pointeur de lecture DATA.\n");
            printf("  Sans argument, revient au debut de toutes les DATA.\n");
            printf("  Avec un numero de ligne, positionne aux DATA de cette ligne.\n\n");
            printf("Exemple:\n");
            printf("  10 DATA 10, 20, 30\n");
            printf("  20 READ A, B\n");
            printf("  30 RESTORE\n");
            printf("  40 READ C          -> C = 10 (recommence au debut)\n\n");
        }
        else if (strcmp(cmdName, "REM") == 0) {
            printf("=== REM ===\n\n");
            printf("Syntaxe: REM commentaire\n\n");
            printf("Description:\n");
            printf("  Ajoute un commentaire dans le programme.\n");
            printf("  Tout ce qui suit REM est ignore.\n\n");
            printf("Exemple:\n");
            printf("  10 REM Ceci est un commentaire\n");
            printf("  20 PRINT \"Hello\"  REM Affiche un message\n\n");
        }
        else if (strcmp(cmdName, "END") == 0) {
            printf("=== END ===\n\n");
            printf("Syntaxe: END\n\n");
            printf("Description:\n");
            printf("  Termine l'execution du programme.\n\n");
            printf("Exemple:\n");
            printf("  10 PRINT \"Programme\"\n");
            printf("  20 END\n");
            printf("  30 PRINT \"Jamais execute\"\n\n");
        }
        else if (strcmp(cmdName, "LIST") == 0) {
            printf("=== LIST ===\n\n");
            printf("Syntaxe: LIST\n\n");
            printf("Description:\n");
            printf("  Affiche toutes les lignes du programme en memoire.\n\n");
        }
        else if (strcmp(cmdName, "RUN") == 0) {
            printf("=== RUN ===\n\n");
            printf("Syntaxe: RUN\n\n");
            printf("Description:\n");
            printf("  Execute le programme en memoire depuis le debut.\n\n");
        }
        else if (strcmp(cmdName, "NEW") == 0) {
            printf("=== NEW ===\n\n");
            printf("Syntaxe: NEW\n\n");
            printf("Description:\n");
            printf("  Efface le programme en memoire et les variables.\n\n");
        }
        else if (strcmp(cmdName, "SAVE") == 0) {
            printf("=== SAVE ===\n\n");
            printf("Syntaxe: SAVE \"nomfichier.bas\"\n\n");
            printf("Description:\n");
            printf("  Sauvegarde le programme actuel dans un fichier.\n\n");
            printf("Exemple:\n");
            printf("  SAVE \"monprog.bas\"\n\n");
        }
        else if (strcmp(cmdName, "LOAD") == 0) {
            printf("=== LOAD ===\n\n");
            printf("Syntaxe: LOAD \"nomfichier.bas\"\n\n");
            printf("Description:\n");
            printf("  Charge un programme depuis un fichier.\n\n");
            printf("Exemple:\n");
            printf("  LOAD \"monprog.bas\"\n\n");
        }
        else if (strcmp(cmdName, "HELP") == 0) {
            printf("=== HELP ===\n\n");
            printf("Syntaxe: HELP [commande]\n\n");
            printf("Description:\n");
            printf("  Sans argument: affiche la liste des commandes.\n");
            printf("  Avec argument: affiche l'aide detaillee d'une commande.\n\n");
            printf("Exemples:\n");
            printf("  HELP          -> Liste toutes les commandes\n");
            printf("  HELP PRINT    -> Aide detaillee sur PRINT\n\n");
        }
        else if (strcmp(cmdName, "EXIT") == 0) {
            printf("=== EXIT ===\n\n");
            printf("Syntaxe: EXIT\n\n");
            printf("Description:\n");
            printf("  Quitte l'interpreteur BASIC80.\n");
            printf("  Le programme en memoire est perdu s'il n'est pas sauvegarde.\n\n");
            printf("Note:\n");
            printf("  Utilisez SAVE avant EXIT pour conserver votre programme.\n\n");
        }
        else if (strcmp(cmdName, "NEXT") == 0) {
            printf("=== NEXT ===\n\n");
            printf("Voir HELP FOR pour l'utilisation des boucles FOR/NEXT.\n\n");
        }
        else if (strcmp(cmdName, "THEN") == 0 || strcmp(cmdName, "ELSE") == 0) {
            printf("=== THEN / ELSE ===\n\n");
            printf("Voir HELP IF pour l'utilisation de IF/THEN/ELSE.\n\n");
        }
        else if (strcmp(cmdName, "TO") == 0 || strcmp(cmdName, "STEP") == 0) {
            printf("=== TO / STEP ===\n\n");
            printf("Voir HELP FOR pour l'utilisation de TO et STEP dans les boucles.\n\n");
        }
        /* ===== FONCTIONS MATHEMATIQUES ===== */
        else if (strcmp(cmdName, "ABS") == 0) {
            printf("=== ABS ===\n\n");
            printf("Syntaxe: ABS(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne la valeur absolue d'un nombre.\n\n");
            printf("Exemples:\n");
            printf("  PRINT ABS(-5)       -> Affiche: 5.00\n");
            printf("  PRINT ABS(3.14)     -> Affiche: 3.14\n");
            printf("  X = ABS(A - B)      -> Distance entre A et B\n\n");
        }
        else if (strcmp(cmdName, "SQR") == 0) {
            printf("=== SQR ===\n\n");
            printf("Syntaxe: SQR(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne la racine carree d'un nombre.\n");
            printf("  Le nombre doit etre positif ou nul.\n\n");
            printf("Exemples:\n");
            printf("  PRINT SQR(16)       -> Affiche: 4.00\n");
            printf("  PRINT SQR(2)        -> Affiche: 1.41\n");
            printf("  H = SQR(A*A + B*B)  -> Hypothenuse (Pythagore)\n\n");
        }
        else if (strcmp(cmdName, "INT") == 0) {
            printf("=== INT ===\n\n");
            printf("Syntaxe: INT(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne la partie entiere d'un nombre (arrondi vers le bas).\n\n");
            printf("Exemples:\n");
            printf("  PRINT INT(3.7)      -> Affiche: 3.00\n");
            printf("  PRINT INT(-2.3)     -> Affiche: -3.00\n");
            printf("  N = INT(X / 10)     -> Dizaines de X\n\n");
        }
        else if (strcmp(cmdName, "SGN") == 0) {
            printf("=== SGN ===\n\n");
            printf("Syntaxe: SGN(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le signe d'un nombre:\n");
            printf("    -1 si negatif\n");
            printf("     0 si nul\n");
            printf("    +1 si positif\n\n");
            printf("Exemples:\n");
            printf("  PRINT SGN(-5)       -> Affiche: -1.00\n");
            printf("  PRINT SGN(0)        -> Affiche: 0.00\n");
            printf("  PRINT SGN(42)       -> Affiche: 1.00\n\n");
        }
        else if (strcmp(cmdName, "RND") == 0) {
            printf("=== RND ===\n\n");
            printf("Syntaxe: RND\n\n");
            printf("Description:\n");
            printf("  Retourne un nombre aleatoire entre 0 (inclus) et 1 (exclus).\n\n");
            printf("Exemples:\n");
            printf("  PRINT RND           -> Affiche: 0.73 (exemple)\n");
            printf("  X = INT(RND * 6) + 1 -> Lance un de (1 a 6)\n");
            printf("  Y = RND * 100       -> Nombre entre 0 et 100\n\n");
        }
        else if (strcmp(cmdName, "SIN") == 0) {
            printf("=== SIN ===\n\n");
            printf("Syntaxe: SIN(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le sinus d'un angle en radians.\n\n");
            printf("Exemples:\n");
            printf("  PRINT SIN(0)        -> Affiche: 0.00\n");
            printf("  PRINT SIN(3.14159/2) -> Affiche: 1.00\n");
            printf("  Y = SIN(A * 3.14159 / 180) -> Convertir degres en radians\n\n");
        }
        else if (strcmp(cmdName, "COS") == 0) {
            printf("=== COS ===\n\n");
            printf("Syntaxe: COS(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le cosinus d'un angle en radians.\n\n");
            printf("Exemples:\n");
            printf("  PRINT COS(0)        -> Affiche: 1.00\n");
            printf("  PRINT COS(3.14159)  -> Affiche: -1.00\n");
            printf("  X = COS(A * 3.14159 / 180) -> Convertir degres en radians\n\n");
        }
        else if (strcmp(cmdName, "TAN") == 0) {
            printf("=== TAN ===\n\n");
            printf("Syntaxe: TAN(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne la tangente d'un angle en radians.\n\n");
            printf("Exemples:\n");
            printf("  PRINT TAN(0)        -> Affiche: 0.00\n");
            printf("  PRINT TAN(3.14159/4) -> Affiche: 1.00\n\n");
        }
        else if (strcmp(cmdName, "ATN") == 0) {
            printf("=== ATN ===\n\n");
            printf("Syntaxe: ATN(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne l'arc tangente (en radians) d'un nombre.\n");
            printf("  Resultat entre -PI/2 et PI/2.\n\n");
            printf("Exemples:\n");
            printf("  PRINT ATN(1)        -> Affiche: 0.79 (PI/4)\n");
            printf("  A = ATN(Y/X) * 180 / 3.14159 -> Angle en degres\n\n");
        }
        else if (strcmp(cmdName, "EXP") == 0) {
            printf("=== EXP ===\n\n");
            printf("Syntaxe: EXP(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne e^x (exponentielle de x).\n");
            printf("  e est la base des logarithmes naturels (environ 2.718).\n\n");
            printf("Exemples:\n");
            printf("  PRINT EXP(0)        -> Affiche: 1.00\n");
            printf("  PRINT EXP(1)        -> Affiche: 2.72\n");
            printf("  Y = EXP(X)          -> Croissance exponentielle\n\n");
        }
        else if (strcmp(cmdName, "LOG") == 0) {
            printf("=== LOG ===\n\n");
            printf("Syntaxe: LOG(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le logarithme naturel (base e) d'un nombre.\n");
            printf("  Le nombre doit etre strictement positif.\n\n");
            printf("Exemples:\n");
            printf("  PRINT LOG(2.718)    -> Affiche: 1.00\n");
            printf("  PRINT LOG(1)        -> Affiche: 0.00\n");
            printf("  Y = LOG(X)          -> Fonction inverse de EXP\n\n");
        }
        else if (strcmp(cmdName, "LOG10") == 0) {
            printf("=== LOG10 ===\n\n");
            printf("Syntaxe: LOG10(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le logarithme decimal (base 10) d'un nombre.\n");
            printf("  Le nombre doit etre strictement positif.\n\n");
            printf("Exemples:\n");
            printf("  PRINT LOG10(100)    -> Affiche: 2.00\n");
            printf("  PRINT LOG10(1000)   -> Affiche: 3.00\n");
            printf("  D = LOG10(X)        -> Nombre de chiffres - 1\n\n");
        }
        else if (strcmp(cmdName, "SINH") == 0) {
            printf("=== SINH ===\n\n");
            printf("Syntaxe: SINH(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le sinus hyperbolique d'un nombre.\n\n");
            printf("Exemple:\n");
            printf("  PRINT SINH(0)       -> Affiche: 0.00\n\n");
        }
        else if (strcmp(cmdName, "COSH") == 0) {
            printf("=== COSH ===\n\n");
            printf("Syntaxe: COSH(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne le cosinus hyperbolique d'un nombre.\n\n");
            printf("Exemple:\n");
            printf("  PRINT COSH(0)       -> Affiche: 1.00\n\n");
        }
        else if (strcmp(cmdName, "TANH") == 0) {
            printf("=== TANH ===\n\n");
            printf("Syntaxe: TANH(expression)\n\n");
            printf("Description:\n");
            printf("  Retourne la tangente hyperbolique d'un nombre.\n\n");
            printf("Exemple:\n");
            printf("  PRINT TANH(0)       -> Affiche: 0.00\n\n");
        }
        /* ===== FONCTIONS DE CHAINES ===== */
        else if (strcmp(cmdName, "LEN") == 0) {
            printf("=== LEN ===\n\n");
            printf("Syntaxe: LEN(chaine$)\n\n");
            printf("Description:\n");
            printf("  Retourne la longueur (nombre de caracteres) d'une chaine.\n\n");
            printf("Exemples:\n");
            printf("  PRINT LEN(\"Hello\")  -> Affiche: 5.00\n");
            printf("  A$ = \"Test\"\n");
            printf("  PRINT LEN(A$)       -> Affiche: 4.00\n");
            printf("  IF LEN(X$) > 10 THEN PRINT \"Trop long\"\n\n");
        }
        else if (strcmp(cmdName, "LEFT$") == 0 || strcmp(cmdName, "LEFT") == 0) {
            printf("=== LEFT$ ===\n\n");
            printf("Syntaxe: LEFT$(chaine$, n)\n\n");
            printf("Description:\n");
            printf("  Retourne les n premiers caracteres d'une chaine.\n\n");
            printf("Exemples:\n");
            printf("  PRINT LEFT$(\"Hello\", 3)   -> Affiche: Hel\n");
            printf("  A$ = \"Bonjour\"\n");
            printf("  B$ = LEFT$(A$, 3)         -> B$ = \"Bon\"\n");
            printf("  IF LEFT$(X$, 1) = \"A\" THEN PRINT \"Commence par A\"\n\n");
        }
        else if (strcmp(cmdName, "RIGHT$") == 0 || strcmp(cmdName, "RIGHT") == 0) {
            printf("=== RIGHT$ ===\n\n");
            printf("Syntaxe: RIGHT$(chaine$, n)\n\n");
            printf("Description:\n");
            printf("  Retourne les n derniers caracteres d'une chaine.\n\n");
            printf("Exemples:\n");
            printf("  PRINT RIGHT$(\"Hello\", 2)  -> Affiche: lo\n");
            printf("  A$ = \"Bonjour\"\n");
            printf("  B$ = RIGHT$(A$, 4)        -> B$ = \"jour\"\n");
            printf("  X$ = RIGHT$(\"00\" + STR$(N), 2) -> Format 2 chiffres\n\n");
        }
        else if (strcmp(cmdName, "MID$") == 0 || strcmp(cmdName, "MID") == 0) {
            printf("=== MID$ ===\n\n");
            printf("Syntaxe: MID$(chaine$, debut, longueur)\n\n");
            printf("Description:\n");
            printf("  Extrait une sous-chaine a partir d'une position donnee.\n");
            printf("  Les positions commencent a 1.\n\n");
            printf("Exemples:\n");
            printf("  PRINT MID$(\"Hello\", 2, 3) -> Affiche: ell\n");
            printf("  A$ = \"Bonjour\"\n");
            printf("  B$ = MID$(A$, 4, 2)       -> B$ = \"jo\"\n\n");
        }
        else if (strcmp(cmdName, "CHR$") == 0 || strcmp(cmdName, "CHR") == 0) {
            printf("=== CHR$ ===\n\n");
            printf("Syntaxe: CHR$(code)\n\n");
            printf("Description:\n");
            printf("  Retourne le caractere correspondant au code ASCII.\n\n");
            printf("Exemples:\n");
            printf("  PRINT CHR$(65)      -> Affiche: A\n");
            printf("  PRINT CHR$(72); CHR$(105) -> Affiche: Hi\n");
            printf("  A$ = CHR$(13)       -> Retour chariot\n\n");
        }
        else if (strcmp(cmdName, "ASC") == 0) {
            printf("=== ASC ===\n\n");
            printf("Syntaxe: ASC(chaine$)\n\n");
            printf("Description:\n");
            printf("  Retourne le code ASCII du premier caractere d'une chaine.\n\n");
            printf("Exemples:\n");
            printf("  PRINT ASC(\"A\")      -> Affiche: 65.00\n");
            printf("  PRINT ASC(\"Hello\")  -> Affiche: 72.00 (H)\n");
            printf("  C = ASC(A$)         -> Code du premier caractere\n\n");
        }
        else if (strcmp(cmdName, "STR$") == 0 || strcmp(cmdName, "STR") == 0) {
            printf("=== STR$ ===\n\n");
            printf("Syntaxe: STR$(expression)\n\n");
            printf("Description:\n");
            printf("  Convertit un nombre en chaine de caracteres.\n\n");
            printf("Exemples:\n");
            printf("  A$ = STR$(42)       -> A$ = \"42\"\n");
            printf("  PRINT \"X=\" + STR$(X) -> Affiche: X=10\n");
            printf("  N$ = STR$(123.45)   -> N$ = \"123.45\"\n\n");
        }
        else if (strcmp(cmdName, "VAL") == 0) {
            printf("=== VAL ===\n\n");
            printf("Syntaxe: VAL(chaine$)\n\n");
            printf("Description:\n");
            printf("  Convertit une chaine de caracteres en nombre.\n");
            printf("  S'arrete au premier caractere non numerique.\n\n");
            printf("Exemples:\n");
            printf("  PRINT VAL(\"42\")     -> Affiche: 42.00\n");
            printf("  PRINT VAL(\"3.14\")   -> Affiche: 3.14\n");
            printf("  X = VAL(\"123ABC\")   -> X = 123.00\n");
            printf("  Y = VAL(\"ABC\")      -> Y = 0.00\n\n");
        }
        else {
            printf("Commande ou fonction inconnue: %s\n", cmdName);
            printf("Tapez HELP pour voir la liste des commandes et fonctions disponibles.\n");
        }
    } else {
        printf("Usage: HELP [commande]\n");
    }
}
