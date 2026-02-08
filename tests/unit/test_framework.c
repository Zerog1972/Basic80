#include "test_framework.h"
#include <math.h>

void init_test_stats(TestStats *stats) {
    stats->total = 0;
    stats->passed = 0;
    stats->failed = 0;
}

void print_test_results(const char *module_name, TestStats *stats) {
    printf("\n========================================\n");
    printf("Module: %s\n", module_name);
    printf("========================================\n");
    printf("Tests: %d | Réussis: %d | Échecs: %d\n", 
           stats->total, stats->passed, stats->failed);
    
    if (stats->failed == 0) {
        printf("✓ TOUS LES TESTS RÉUSSIS\n");
    } else {
        printf("✗ %d TEST(S) ÉCHOUÉ(S)\n", stats->failed);
    }
    printf("========================================\n\n");
}
