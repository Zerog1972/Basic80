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
        reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après LET.");
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
            reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après READ");
            return;
        }
        
        strcpy(varName, tokens[pos].value);
        
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

void handleHelp(Interpreter *interp, Token *tokens) {
    char cmdName[256];
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
        
        printf("Commandes systeme:\n");
        printf("  LIST     - Afficher le programme\n");
        printf("  RUN      - Executer le programme\n");
        printf("  NEW      - Effacer le programme\n");
        printf("  SAVE     - Sauvegarder le programme\n");
        printf("  LOAD     - Charger un programme\n");
        printf("  EXIT     - Quitter l'interpreteur\n");
        printf("  HELP     - Afficher cette aide\n\n");
        
        printf("Tapez HELP [COMMANDE] pour plus de details.\n");
        printf("Exemple: HELP PRINT\n\n");
        return;
    }
    
    /* Si HELP avec un argument, afficher l'aide detaillee */
    /* Accepter n'importe quel token qui a une valeur (pas seulement TOK_IDENTIFIER) */
    if (tokens[1].type != TOK_EOF && tokens[1].value) {
        strcpy(cmdName, tokens[1].value);
        
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
        else {
            printf("Commande inconnue: %s\n", cmdName);
            printf("Tapez HELP pour voir la liste des commandes disponibles.\n");
        }
    } else {
        printf("Usage: HELP [commande]\n");
    }
}
