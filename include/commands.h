#ifndef COMMANDS_H
#define COMMANDS_H

#include "interpreter.h"
#include "lexer.h"

/* ===== GESTION DES COMMANDES BASIC ===== */

/**
 * Exécute la commande PRINT.
 * 
 * Affiche une ou plusieurs expressions (numériques ou chaînes) à l'écran.
 * Les expressions multiples sont séparées par des virgules ou points-virgules.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les paramètres de PRINT
 * 
 * Syntaxe BASIC:
 *   PRINT expr [, expr]* [;]
 * 
 * Exemples:
 *   PRINT "Hello"          -> Affiche: Hello
 *   PRINT A, B             -> Affiche: 42       10
 *   PRINT "X="; X          -> Affiche: X=5 (pas de retour à la ligne)
 *   PRINT                  -> Affiche une ligne vide
 * 
 * Séparateurs:
 *   , (virgule)     -> Tabulation entre expressions
 *   ; (point-virgule) -> Pas d'espace entre expressions
 *   Fin de ligne    -> Retour à la ligne automatique
 */
void handlePrint(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande LET (affectation).
 * 
 * Assigne une valeur à une variable (numérique, chaîne ou tableau).
 * Le mot-clé LET est optionnel en BASIC.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant l'affectation
 * 
 * Syntaxe BASIC:
 *   [LET] variable = expression
 *   [LET] array(indices) = expression
 * 
 * Exemples:
 *   LET A = 42
 *   X = 10 + 5
 *   A$ = "Hello"
 *   M(2,3) = 99
 * 
 * Note: Les variables sont créées automatiquement si elles n'existent pas.
 */
void handleLet(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande DIM (déclaration de tableau).
 * 
 * Alloue un tableau multi-dimensionnel avec les tailles spécifiées.
 * Supporte jusqu'à 10 dimensions. Tous les éléments sont initialisés à 0.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant la déclaration
 * 
 * Syntaxe BASIC:
 *   DIM array(size1 [, size2, ...])
 * 
 * Exemples:
 *   DIM A(10)              -> Tableau 1D de 10 éléments
 *   DIM M(5, 8)            -> Tableau 2D de 5×8 éléments
 *   DIM T(3, 4, 5)         -> Tableau 3D de 3×4×5 éléments
 * 
 * Note: Un tableau ne peut être dimensionné qu'une seule fois.
 *       Tenter de redimensionner génère une erreur.
 */
void handleDim(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande INPUT (saisie utilisateur).
 * 
 * Demande à l'utilisateur d'entrer une valeur pour une variable.
 * Affiche un message optionnel et attend la saisie.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les paramètres de INPUT
 * 
 * Syntaxe BASIC:
 *   INPUT [message;] variable
 * 
 * Exemples:
 *   INPUT A                -> ? _
 *   INPUT "Nom"; N$        -> Nom? _
 *   INPUT "Age: "; AGE     -> Age: _
 * 
 * Note: La saisie est convertie en nombre pour les variables numériques
 *       et en chaîne pour les variables chaînes (terminant par $).
 */
void handleInput(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande DATA (déclaration de données).
 * 
 * Déclare des valeurs constantes qui seront lues par READ.
 * Les données peuvent être numériques ou des chaînes.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les valeurs DATA
 * @param lineNum Numéro de ligne de l'instruction (pour RESTORE)
 * 
 * Syntaxe BASIC:
 *   DATA value1 [, value2, ...]
 * 
 * Exemples:
 *   DATA 10, 20, 30
 *   DATA "Alice", "Bob", "Charlie"
 *   DATA 42, "Hello", 3.14
 * 
 * Note: Les DATA sont stockées globalement et accessibles
 *       séquentiellement via READ. Utiliser RESTORE pour
 *       repositionner le pointeur de lecture.
 */
void handleData(Interpreter *interp, Token *tokens, int lineNum);

/**
 * Exécute la commande READ (lecture de données).
 * 
 * Lit séquentiellement des valeurs depuis les instructions DATA
 * et les assigne aux variables spécifiées.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les variables à lire
 * 
 * Syntaxe BASIC:
 *   READ variable1 [, variable2, ...]
 * 
 * Exemples:
 *   10 DATA 10, 20, 30
 *   20 READ A, B, C        -> A=10, B=20, C=30
 * 
 * Note: Si toutes les données sont lues, une erreur ERR_OUT_OF_DATA
 *       est générée. Utiliser RESTORE pour relire les données.
 */
void handleRead(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande RESTORE (réinitialisation du pointeur DATA).
 * 
 * Repositionne le pointeur de lecture DATA au début ou à une ligne spécifique.
 * Permet de relire les données depuis le début ou un point particulier.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens (peut contenir un numéro de ligne optionnel)
 * 
 * Syntaxe BASIC:
 *   RESTORE [lineNum]
 * 
 * Exemples:
 *   RESTORE               -> Repositionne au début de toutes les DATA
 *   RESTORE 100           -> Repositionne aux DATA de la ligne 100
 * 
 * Note: Si le numéro de ligne spécifié ne contient pas de DATA,
 *       le pointeur est positionné à la première DATA suivante.
 */
void handleRestore(Interpreter *interp, Token *tokens);

/**
 * Exécute la commande HELP (aide sur les commandes).
 * 
 * Affiche la liste des commandes disponibles ou l'aide détaillée
 * pour une commande spécifique.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens (peut contenir un nom de commande optionnel)
 * 
 * Syntaxe BASIC:
 *   HELP              -> Liste toutes les commandes
 *   HELP command      -> Affiche l'aide détaillée pour une commande
 * 
 * Exemples:
 *   HELP              -> Affiche la liste de toutes les commandes
 *   HELP PRINT        -> Affiche l'aide détaillée sur PRINT
 *   HELP FOR          -> Affiche l'aide détaillée sur FOR
 */
void handleHelp(Interpreter *interp, Token *tokens);

#endif /* COMMANDS_H */
