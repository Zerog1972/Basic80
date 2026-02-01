#ifndef CONTROL_FLOW_H
#define CONTROL_FLOW_H

#include "interpreter.h"
#include "lexer.h"

/* ===== GESTION DU FLUX DE CONTRÔLE ===== */

/* Gestion de IF/THEN/ELSE
 * Retourne 1 si un saut de ligne a été effectué (GOTO dans THEN/ELSE), 0 sinon */
int handleIfStatement(Interpreter *interp, Token *tokens, Line **currentLine);

/* Gestion de GOTO
 * Retourne 1 si le saut a réussi, 0 sinon */
int handleGoto(Interpreter *interp, Token *tokens, Line **currentLine);

/* Gestion de GOSUB
 * Retourne 1 si le saut a réussi, 0 sinon */
int handleGosub(Interpreter *interp, Token *tokens, Line **currentLine);

/* Gestion de RETURN
 * Retourne 1 si le retour a réussi, 0 sinon */
int handleReturn(Interpreter *interp, Line **currentLine);

/* Gestion de FOR
 * Retourne 1 si un saut vers NEXT a été effectué (boucle non exécutée), 0 sinon */
int handleFor(Interpreter *interp, Token *tokens, Line **currentLine);

/* Gestion de NEXT
 * Retourne 1 si un retour au FOR a été effectué, 0 sinon */
int handleNext(Interpreter *interp);

/* Fonction utilitaire pour trouver une ligne par numéro */
Line* findLineByNumber(Interpreter *interp, int lineNum);

#endif /* CONTROL_FLOW_H */
