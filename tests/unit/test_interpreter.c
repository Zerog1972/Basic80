#include "test_framework.h"
#include "../../include/interpreter.h"
#include "../../include/lexer.h"
#include <stdlib.h>

/* Test: Ajout et suppression de lignes */
void test_interpreter_add_delete_lines(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Ajout et suppression de lignes ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    addLine(interp, 10, "PRINT \"Test\"");
    ASSERT_NOT_NULL(interp->program, "Ligne 10 ajoutée");
    ASSERT_EQUAL(interp->program->lineNum, 10, "Numéro de ligne correct");
    
    addLine(interp, 20, "PRINT \"Test 2\"");
    ASSERT_NOT_NULL(interp->program->next, "Ligne 20 ajoutée");
    
    deleteLine(interp, 10);
    ASSERT_EQUAL(interp->program->lineNum, 20, "Ligne 10 supprimée");
    
    freeInterpreter(interp);
}

/* Test: Ordre des lignes */
void test_interpreter_line_order(TestStats *stats) {
    Interpreter *interp;
    Line *line;
    
    printf("\n--- Test: Ordre des lignes ---\n");
    
    interp = createInterpreter();
    
    addLine(interp, 30, "PRINT \"C\"");
    addLine(interp, 10, "PRINT \"A\"");
    addLine(interp, 20, "PRINT \"B\"");
    
    line = interp->program;
    ASSERT_EQUAL(line->lineNum, 10, "Première ligne: 10");
    line = line->next;
    ASSERT_EQUAL(line->lineNum, 20, "Deuxième ligne: 20");
    line = line->next;
    ASSERT_EQUAL(line->lineNum, 30, "Troisième ligne: 30");
    
    freeInterpreter(interp);
}

/* Test: Modification de ligne */
void test_interpreter_modify_line(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Modification de ligne ---\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "PRINT \"Original\"");
    ASSERT_NOT_NULL(interp->program, "Ligne originale ajoutée");
    
    addLine(interp, 10, "PRINT \"Modifié\"");
    ASSERT_STR_EQUAL(interp->program->code, "PRINT \"Modifié\"", "Ligne modifiée");
    
    freeInterpreter(interp);
}

/* Test: Programme vide */
void test_interpreter_clear(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Effacement du programme ---\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "PRINT \"Test\"");
    addLine(interp, 20, "PRINT \"Test 2\"");
    ASSERT_NOT_NULL(interp->program, "Programme non vide");
    
    clearProgram(interp);
    ASSERT_NULL(interp->program, "Programme vide après clear");
    
    freeInterpreter(interp);
}

/* Test: Gestion des erreurs */
void test_interpreter_error_handling(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Gestion des erreurs ---\n");
    
    interp = createInterpreter();
    ASSERT_FALSE(interp->hasError, "Pas d'erreur initialement");
    
    reportError(interp, "Test d'erreur");
    ASSERT_TRUE(interp->hasError, "Erreur signalée");
    
    freeInterpreter(interp);
}

/* Test: Contexte d'erreur */
void test_interpreter_error_context(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Contexte d'erreur ---\n");
    
    interp = createInterpreter();
    
    reportErrorEx(interp, ERR_SYNTAX, 5, "Erreur de syntaxe");
    ASSERT_TRUE(interp->hasError, "Erreur avec contexte signalée");
    ASSERT_EQUAL(interp->lastErrorType, ERR_SYNTAX, "Type d'erreur correct");
    ASSERT_EQUAL(interp->errorColumn, 5, "Colonne d'erreur correcte");
    
    freeInterpreter(interp);
}

void run_interpreter_tests(void) {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║   TESTS UNITAIRES - INTERPRETER       ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_interpreter_add_delete_lines(&stats);
    test_interpreter_line_order(&stats);
    test_interpreter_modify_line(&stats);
    test_interpreter_clear(&stats);
    test_interpreter_error_handling(&stats);
    test_interpreter_error_context(&stats);
    
    print_test_results("INTERPRETER", &stats);
}
