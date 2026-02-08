#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "lexer.h"

/* Forward declaration */
typedef struct Interpreter Interpreter;

/* ===== ÉVALUATION DES CONDITIONS ===== */

/**
 * Évalue une condition booléenne (comparaison).
 * 
 * Évalue une expression de comparaison et retourne le résultat booléen.
 * Supporte les opérateurs: =, <, >, <=, >=, <>
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant la condition
 * @param pos Pointeur vers la position courante (modifié après évaluation)
 * @return 1 si la condition est vraie, 0 si elle est fausse
 * 
 * Exemples:
 *   "10 > 5"    -> 1
 *   "X = 42"    -> 1 si X vaut 42, 0 sinon
 *   "A$ <> B$"  -> 1 si A$ différent de B$
 */
int evaluateCondition(Interpreter *interp, Token *tokens, int *pos);

/* ===== ÉVALUATION DES EXPRESSIONS NUMÉRIQUES ===== */

/**
 * Évalue une expression arithmétique complète (addition, soustraction).
 * 
 * Point d'entrée principal pour l'évaluation d'expressions numériques.
 * Gère la priorité des opérateurs selon les règles mathématiques standard.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant l'expression
 * @param pos Pointeur vers la position courante (modifié après évaluation)
 * @return Résultat numérique de l'expression
 * 
 * Exemples:
 *   "10 + 5 * 2"      -> 20.0
 *   "(10 + 5) * 2"    -> 30.0
 *   "A + B - C"       -> Valeur calculée selon les variables
 */
double evaluateExpression(Interpreter *interp, Token *tokens, int *pos);

/**
 * Évalue un terme (multiplication, division).
 * 
 * Gère les opérations de priorité moyenne (* et /).
 * Appelé par evaluateExpression() dans l'analyse descendante.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens
 * @param pos Pointeur vers la position courante
 * @return Résultat numérique du terme
 */
double evaluateTerm(Interpreter *interp, Token *tokens, int *pos);

/**
 * Évalue un facteur (nombres, variables, fonctions, parenthèses).
 * 
 * Gère les éléments atomiques d'une expression:
 * - Nombres littéraux (42, 3.14)
 * - Variables (A, X, COUNT)
 * - Tableaux (A(5), M(2,3))
 * - Fonctions mathématiques (SIN, COS, SQR, ABS, etc.)
 * - Expressions entre parenthèses
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens
 * @param pos Pointeur vers la position courante
 * @return Résultat numérique du facteur
 */
double evaluateFactor(Interpreter *interp, Token *tokens, int *pos);

/* ===== ÉVALUATION DES EXPRESSIONS DE CHAÎNES ===== */

/**
 * Détermine si une expression est de type chaîne.
 * 
 * Analyse les tokens à partir de la position donnée pour déterminer
 * si l'expression évalue vers une chaîne (variable$, "littéral", fonctions).
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens
 * @param pos Position de départ (non modifiée)
 * @return 1 si l'expression est de type chaîne, 0 sinon
 */
int isStringExpression(Interpreter *interp, Token *tokens, int pos);

/**
 * Évalue une expression de chaîne (concaténation).
 * 
 * Évalue une expression produisant une chaîne de caractères.
 * Supporte la concaténation avec l'opérateur +.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant l'expression
 * @param pos Pointeur vers la position courante (modifié après évaluation)
 * @return Chaîne de caractères résultante (doit être libérée par l'appelant)
 * 
 * Exemples:
 *   "A$ + B$"           -> Concaténation de deux variables
 *   "Hello" + " World"  -> "Hello World"
 *   LEFT$(A$, 3)        -> Premiers 3 caractères de A$
 */
char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos);

/**
 * Évalue un élément primaire de chaîne.
 * 
 * Gère les éléments atomiques d'une expression de chaîne:
 * - Chaînes littérales ("Hello")
 * - Variables chaînes (A$, NAME$)
 * - Fonctions de chaînes (LEFT$, MID$, RIGHT$, CHR$, STR$, etc.)
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens
 * @param pos Pointeur vers la position courante
 * @return Chaîne de caractères résultante (doit être libérée par l'appelant)
 */
char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos);

#endif
