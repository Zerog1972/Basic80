#include "tstfrmwk.h"

/* Déclarations des fonctions de test */
void run_lexer_tests(void);
void run_variables_tests(void);
void run_expression_tests(void);
void run_interpreter_tests(void);

int main(void) {
    int total_tests = 0;
    int total_passed = 0;
    int total_failed = 0;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║                                                        ║\n");
    printf("║         SUITE DE TESTS UNITAIRES - BASIC80            ║\n");
    printf("║                                                        ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    
    /* Exécuter tous les tests */
    run_lexer_tests();
    run_variables_tests();
    run_expression_tests();
    run_interpreter_tests();
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════╗\n");
    printf("║              RÉSUMÉ GLOBAL DES TESTS                  ║\n");
    printf("╚════════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("Tous les tests unitaires ont été exécutés.\n");
    printf("Consultez les résultats ci-dessus pour plus de détails.\n\n");
    
    return 0;
}
