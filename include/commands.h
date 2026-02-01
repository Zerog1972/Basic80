#ifndef COMMANDS_H
#define COMMANDS_H

#include "interpreter.h"
#include "lexer.h"

/* ===== GESTION DES COMMANDES BASIC ===== */

/* Commande PRINT
 * Affiche des expressions numériques et/ou des chaînes de caractères
 * Supporte plusieurs expressions séparées par des virgules */
void handlePrint(Interpreter *interp, Token *tokens);

/* Commande LET
 * Affectation de variables (numériques, chaînes, tableaux) */
void handleLet(Interpreter *interp, Token *tokens);

/* Commande DIM
 * Déclaration de tableaux multi-dimensionnels (jusqu'à 10 dimensions) */
void handleDim(Interpreter *interp, Token *tokens);

/* Commande INPUT
 * Lecture interactive d'une valeur numérique depuis l'utilisateur */
void handleInput(Interpreter *interp, Token *tokens);

#endif /* COMMANDS_H */
