#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "lexer.h"
#include "variables.h"
#include "expression.h"

/* Types d'erreurs */
typedef enum {
    ERR_NONE,
    ERR_SYNTAX,          /* Erreur de syntaxe */
    ERR_RUNTIME,         /* Erreur d'exécution */
    ERR_TYPE_MISMATCH,   /* Incompatibilité de types */
    ERR_OUT_OF_DATA,     /* Plus de données DATA */
    ERR_DIVISION_ZERO,   /* Division par zéro */
    ERR_UNDEFINED_VAR,   /* Variable non définie */
    ERR_ARRAY_BOUNDS,    /* Dépassement de tableau */
    ERR_OUT_OF_MEMORY    /* Mémoire insuffisante */
} ErrorType;

/* Structure pour une ligne de code */
typedef struct Line {
    int lineNum;
    char *code;
    struct Line *next;
} Line;

/* Structure pour une boucle FOR */
typedef struct ForLoop {
    char *varName;
    double endValue;
    double stepValue;
    Line *startLine;
    struct ForLoop *next;
} ForLoop;

/* Structure pour la pile d'appels GOSUB */
typedef struct CallStack {
    Line *returnLine;
    struct CallStack *next;
} CallStack;

/* Structure pour stocker les données DATA */
typedef struct DataItem {
    char *value;  /* Stocke la valeur comme chaîne */
    int lineNum;  /* Numéro de ligne pour RESTORE */
    struct DataItem *next;
} DataItem;

/* ===== SYSTÈME DE HOOKS/CALLBACKS POUR L'EXTENSION ===== */

/* Type de callback pour une fonction numérique personnalisée */
typedef double (*CustomNumericFunction)(struct Interpreter *interp, Token *tokens, int *pos);

/* Type de callback pour une fonction chaîne personnalisée */
typedef char* (*CustomStringFunction)(struct Interpreter *interp, Token *tokens, int *pos);

/* Type de callback pour une commande personnalisée */
typedef void (*CustomCommandHandler)(struct Interpreter *interp, Token *tokens);

/* Structure pour stocker une fonction numérique personnalisée */
typedef struct CustomNumFunc {
    char *name;                      /* Nom de la fonction (ex: "MYSIN") */
    CustomNumericFunction handler;   /* Pointeur vers la fonction */
    struct CustomNumFunc *next;
} CustomNumFunc;

/* Structure pour stocker une fonction chaîne personnalisée */
typedef struct CustomStrFunc {
    char *name;                      /* Nom de la fonction (ex: "REVERSE$") */
    CustomStringFunction handler;    /* Pointeur vers la fonction */
    struct CustomStrFunc *next;
} CustomStrFunc;

/* Structure pour stocker une commande personnalisée */
typedef struct CustomCommand {
    char *name;                      /* Nom de la commande (ex: "BEEP") */
    CustomCommandHandler handler;    /* Pointeur vers le gestionnaire */
    struct CustomCommand *next;
} CustomCommand;

/* Structure pour l'interpréteur */
struct Interpreter {
    Line *program;
    Variable *variables;
    Line *currentLine;
    ForLoop *forStack;
    CallStack *callStack;
    DataItem *dataList;      /* Liste chaînée des DATA */
    DataItem *dataPointer;   /* Pointeur de lecture courant */
    int hasError;
    ErrorType lastErrorType; /* Type de la dernière erreur */
    int errorColumn;         /* Colonne de l'erreur (position dans la ligne) */
    char errorContext[256];  /* Contexte de l'erreur (extrait de code) */
    
    /* Hooks pour l'extension */
    CustomNumFunc *customNumFuncs;   /* Liste des fonctions numériques personnalisées */
    CustomStrFunc *customStrFuncs;   /* Liste des fonctions chaînes personnalisées */
    CustomCommand *customCommands;   /* Liste des commandes personnalisées */
};

/* ===== FONCTIONS DE L'INTERPRÉTEUR ===== */

/**
 * Crée et initialise un nouvel interpréteur BASIC.
 * 
 * Alloue la mémoire nécessaire et initialise tous les champs.
 * L'interpréteur doit être libéré avec freeInterpreter() après utilisation.
 * 
 * @return Pointeur vers le nouvel interpréteur, NULL en cas d'échec d'allocation
 */
Interpreter* createInterpreter(void);

/**
 * Libère toute la mémoire associée à un interpréteur.
 * 
 * Désalloue le programme, les variables, les boucles FOR,
 * la pile d'appels GOSUB, et les données DATA.
 * 
 * @param interp Pointeur vers l'interpréteur à libérer (peut être NULL)
 */
void freeInterpreter(Interpreter *interp);

/**
 * Ajoute ou remplace une ligne dans le programme.
 * 
 * Si une ligne avec le même numéro existe déjà, elle est remplacée.
 * Les lignes sont automatiquement triées par numéro de ligne.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param lineNum Numéro de ligne (doit être positif)
 * @param code Code source de la ligne (sans le numéro de ligne)
 * 
 * Exemples:
 *   addLine(interp, 10, "PRINT \"Hello\"");
 *   addLine(interp, 20, "LET A = 42");
 */
void addLine(Interpreter *interp, int lineNum, const char *code);

/**
 * Supprime une ligne du programme.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param lineNum Numéro de ligne à supprimer
 * 
 * Note: Si la ligne n'existe pas, aucune action n'est effectuée.
 */
void deleteLine(Interpreter *interp, int lineNum);

/**
 * Exécute le programme chargé en mémoire.
 * 
 * Lance l'exécution depuis la première ligne jusqu'à la dernière
 * ou jusqu'à rencontrer END ou une erreur.
 * Gère les boucles FOR/NEXT et les appels GOSUB/RETURN.
 * 
 * @param interp Pointeur vers l'interpréteur
 */
void runProgram(Interpreter *interp);

/**
 * Exécute une commande en mode direct (sans numéro de ligne).
 * 
 * Permet d'exécuter une instruction BASIC immédiatement sans
 * l'ajouter au programme. Utilisé en mode interactif.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param line Ligne de commande à exécuter
 * 
 * Exemples:
 *   executeCommand(interp, "PRINT A");
 *   executeCommand(interp, "LET X = 10");
 */
void executeCommand(Interpreter *interp, const char *line);

/**
 * Affiche le programme chargé en mémoire.
 * 
 * Liste toutes les lignes du programme dans l'ordre,
 * avec leurs numéros de ligne.
 * 
 * @param interp Pointeur vers l'interpréteur
 */
void listProgram(Interpreter *interp);

/**
 * Efface tout le programme de la mémoire.
 * 
 * Supprime toutes les lignes de code, mais conserve les variables.
 * Pour réinitialiser complètement, utiliser freeInterpreter()
 * suivi de createInterpreter().
 * 
 * @param interp Pointeur vers l'interpréteur
 */
void clearProgram(Interpreter *interp);

/**
 * Sauvegarde le programme dans un fichier.
 * 
 * Écrit toutes les lignes du programme dans le fichier spécifié
 * au format texte (une ligne par instruction).
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param filename Nom du fichier de destination
 * @return 1 en cas de succès, 0 en cas d'erreur
 */
int saveProgram(Interpreter *interp, const char *filename);

/**
 * Charge un programme depuis un fichier.
 * 
 * Efface le programme actuel et charge le contenu du fichier.
 * Chaque ligne doit commencer par un numéro de ligne.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param filename Nom du fichier source
 * @return 1 en cas de succès, 0 en cas d'erreur
 */
int loadProgram(Interpreter *interp, const char *filename);

/* ===== GESTION DES ERREURS ===== */

/**
 * Signale une erreur d'exécution (version simple).
 * 
 * Affiche un message d'erreur et positionne le drapeau hasError.
 * Utiliser reportErrorEx() pour un rapport d'erreur plus détaillé.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param message Message d'erreur à afficher
 * 
 * Note: Cette fonction est conservée pour compatibilité.
 *       Préférer reportErrorEx() pour les nouvelles implémentations.
 */
void reportError(Interpreter *interp, const char *message);

/**
 * Signale une erreur avec contexte détaillé.
 * 
 * Affiche un message d'erreur avec:
 * - Type d'erreur catégorisé
 * - Numéro de ligne
 * - Position dans la ligne (colonne)
 * - Code source avec curseur visuel
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param type Type d'erreur (ERR_SYNTAX, ERR_RUNTIME, etc.)
 * @param column Position du curseur dans la ligne (0 = début)
 * @param message Message explicatif de l'erreur
 * 
 * Exemple d'affichage:
 *   [ERREUR Syntaxe] Ligne 10, colonne 5: Variable attendue
 *     --> LET = 5
 *          ^
 */
void reportErrorEx(Interpreter *interp, ErrorType type, int column, const char *message);

/**
 * Retourne le nom lisible d'un type d'erreur.
 * 
 * @param type Type d'erreur
 * @return Chaîne de caractères décrivant le type d'erreur
 * 
 * Exemples:
 *   getErrorTypeName(ERR_SYNTAX)     -> "Syntaxe"
 *   getErrorTypeName(ERR_RUNTIME)    -> "Exécution"
 *   getErrorTypeName(ERR_TYPE_MISMATCH) -> "Type"
 */
const char* getErrorTypeName(ErrorType type);

/* ===== SYSTÈME DE HOOKS/CALLBACKS ===== */

/**
 * Enregistre une fonction numérique personnalisée.
 * 
 * Permet d'ajouter de nouvelles fonctions mathématiques ou numériques
 * qui peuvent être appelées depuis le code BASIC.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la fonction (sera converti en majuscules)
 * @param handler Pointeur vers la fonction de callback
 * @return 1 en cas de succès, 0 en cas d'erreur (mémoire insuffisante)
 * 
 * Exemple:
 *   double myDouble(Interpreter *interp, Token *tokens, int *pos) {
 *       double arg = evaluateExpression(interp, tokens, pos);
 *       return arg * 2.0;
 *   }
 *   registerCustomNumericFunction(interp, "DOUBLE", myDouble);
 *   // BASIC: PRINT DOUBLE(21)  ' Affiche: 42
 */
int registerCustomNumericFunction(Interpreter *interp, const char *name, CustomNumericFunction handler);

/**
 * Enregistre une fonction chaîne personnalisée.
 * 
 * Permet d'ajouter de nouvelles fonctions de manipulation de chaînes
 * qui peuvent être appelées depuis le code BASIC.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la fonction (sera converti en majuscules, doit se terminer par $)
 * @param handler Pointeur vers la fonction de callback
 * @return 1 en cas de succès, 0 en cas d'erreur (mémoire insuffisante)
 * 
 * Exemple:
 *   char* myReverse(Interpreter *interp, Token *tokens, int *pos) {
 *       char *str = evaluateStringExpression(interp, tokens, pos);
 *       // ... inverser str ...
 *       return str;  // L'appelant libérera la mémoire
 *   }
 *   registerCustomStringFunction(interp, "REVERSE$", myReverse);
 *   // BASIC: PRINT REVERSE$("Hello")  ' Affiche: olleH
 */
int registerCustomStringFunction(Interpreter *interp, const char *name, CustomStringFunction handler);

/**
 * Enregistre une commande personnalisée.
 * 
 * Permet d'ajouter de nouvelles commandes qui peuvent être exécutées
 * depuis le code BASIC.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la commande (sera converti en majuscules)
 * @param handler Pointeur vers le gestionnaire de commande
 * @return 1 en cas de succès, 0 en cas d'erreur (mémoire insuffisante)
 * 
 * Exemple:
 *   void myBeep(Interpreter *interp, Token *tokens) {
 *       printf("\a");  // Émet un bip
 *       fflush(stdout);
 *   }
 *   registerCustomCommand(interp, "BEEP", myBeep);
 *   // BASIC: BEEP
 */
int registerCustomCommand(Interpreter *interp, const char *name, CustomCommandHandler handler);

/**
 * Recherche une fonction numérique personnalisée par son nom.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la fonction à rechercher
 * @return Pointeur vers le handler si trouvé, NULL sinon
 */
CustomNumericFunction findCustomNumericFunction(Interpreter *interp, const char *name);

/**
 * Recherche une fonction chaîne personnalisée par son nom.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la fonction à rechercher
 * @return Pointeur vers le handler si trouvé, NULL sinon
 */
CustomStringFunction findCustomStringFunction(Interpreter *interp, const char *name);

/**
 * Recherche une commande personnalisée par son nom.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la commande à rechercher
 * @return Pointeur vers le handler si trouvé, NULL sinon
 */
CustomCommandHandler findCustomCommand(Interpreter *interp, const char *name);

#endif /* UTILS_H */