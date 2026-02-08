#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

#include "interpreter.h"
#include "lexer.h"

/* ===== GESTION DU FLUX DE CONTRÔLE ===== */

/**
 * Exécute une instruction IF/THEN/ELSE.
 * 
 * Évalue une condition et exécute les instructions appropriées
 * selon le résultat (branche THEN ou ELSE).
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant l'instruction IF
 * @param currentLine Pointeur double vers la ligne courante (modifié si GOTO)
 * @return 1 si un saut a été effectué (GOTO dans THEN/ELSE), 0 sinon
 * 
 * Syntaxe BASIC:
 *   IF condition THEN instruction [ELSE instruction]
 *   IF condition THEN lineNum [ELSE lineNum]
 * 
 * Exemples:
 *   IF X > 5 THEN PRINT "Grand"
 *   IF A = 0 THEN GOTO 100
 *   IF X < 0 THEN Y = -1 ELSE Y = 1
 *   IF A$ = "Y" THEN 100 ELSE 200
 * 
 * Note: Seule la forme simple (une ligne) est supportée.
 *       Pour plusieurs instructions, utiliser des numéros de ligne.
 */
int handleIfStatement(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Exécute une instruction GOTO (saut inconditionnel).
 * 
 * Transfère le contrôle à la ligne spécifiée.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant le numéro de ligne cible
 * @param currentLine Pointeur double vers la ligne courante (modifié)
 * @return 1 si le saut a réussi, 0 en cas d'erreur
 * 
 * Syntaxe BASIC:
 *   GOTO lineNum
 * 
 * Exemples:
 *   GOTO 100
 *   IF X = 0 THEN GOTO 200
 * 
 * Note: Si la ligne cible n'existe pas, une erreur est générée.
 */
int handleGoto(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Exécute une instruction GOSUB (appel de sous-routine).
 * 
 * Sauvegarde la ligne de retour et transfère le contrôle
 * à la sous-routine spécifiée. Utiliser RETURN pour revenir.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant le numéro de ligne cible
 * @param currentLine Pointeur double vers la ligne courante (modifié)
 * @return 1 si le saut a réussi, 0 en cas d'erreur
 * 
 * Syntaxe BASIC:
 *   GOSUB lineNum
 * 
 * Exemples:
 *   GOSUB 1000
 *   IF FLAG THEN GOSUB 500
 * 
 * Note: Les appels GOSUB peuvent être imbriqués.
 *       La pile d'appels est gérée automatiquement.
 */
int handleGosub(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Exécute une instruction RETURN (retour de sous-routine).
 * 
 * Retourne à la ligne suivant le GOSUB correspondant.
 * Dépile la pile d'appels.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param currentLine Pointeur double vers la ligne courante (modifié)
 * @return 1 si le retour a réussi, 0 en cas d'erreur
 * 
 * Syntaxe BASIC:
 *   RETURN
 * 
 * Note: Une erreur est générée si RETURN est appelé sans GOSUB préalable.
 */
int handleReturn(Interpreter *interp, Line **currentLine);

/**
 * Exécute une instruction FOR (début de boucle).
 * 
 * Initialise une boucle FOR avec une variable de contrôle,
 * une valeur de fin et un pas optionnel.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param tokens Tableau de tokens contenant les paramètres de la boucle
 * @param currentLine Pointeur double vers la ligne courante (modifié si saut)
 * @return 1 si un saut vers NEXT a été effectué (pas de boucle), 0 sinon
 * 
 * Syntaxe BASIC:
 *   FOR variable = start TO end [STEP increment]
 * 
 * Exemples:
 *   FOR I = 1 TO 10          -> I va de 1 à 10 par pas de 1
 *   FOR X = 0 TO 100 STEP 5  -> X va de 0 à 100 par pas de 5
 *   FOR J = 10 TO 1 STEP -1  -> J va de 10 à 1 (décroissant)
 * 
 * Note: Si la condition de fin est déjà fausse au départ,
 *       la boucle n'est pas exécutée (saut au NEXT correspondant).
 */
int handleFor(Interpreter *interp, Token *tokens, Line **currentLine);

/**
 * Exécute une instruction NEXT (fin de boucle).
 * 
 * Incrémente la variable de contrôle et teste si la boucle continue.
 * Si la condition est remplie, retourne au FOR, sinon sort de la boucle.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @return 1 si un retour au FOR a été effectué, 0 sinon (fin de boucle)
 * 
 * Syntaxe BASIC:
 *   NEXT [variable]
 * 
 * Exemples:
 *   NEXT
 *   NEXT I
 * 
 * Note: Le nom de variable après NEXT est optionnel et ignoré.
 *       NEXT s'applique toujours au FOR le plus récent non terminé.
 */
int handleNext(Interpreter *interp);

/**
 * Recherche une ligne par son numéro dans le programme.
 * 
 * Fonction utilitaire pour localiser une ligne spécifique.
 * Utilisée par GOTO, GOSUB, et la gestion des boucles.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param lineNum Numéro de ligne à rechercher
 * @return Pointeur vers la ligne si trouvée, NULL sinon
 */
Line* findLineByNumber(Interpreter *interp, int lineNum);

#endif /* CONTROL_FLOW_H */
