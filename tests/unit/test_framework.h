#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>
#include <string.h>

/* Statistiques des tests */
typedef struct {
    int total;
    int passed;
    int failed;
} TestStats;

/* Macros pour les tests */
#define ASSERT_TRUE(expr, msg) \
    do { \
        stats->total++; \
        if (expr) { \
            stats->passed++; \
            printf("  ✓ %s\n", msg); \
        } else { \
            stats->failed++; \
            printf("  ✗ ÉCHEC: %s\n", msg); \
        } \
    } while(0)

#define ASSERT_FALSE(expr, msg) \
    ASSERT_TRUE(!(expr), msg)

#define ASSERT_EQUAL(a, b, msg) \
    ASSERT_TRUE((a) == (b), msg)

#define ASSERT_NOT_EQUAL(a, b, msg) \
    ASSERT_TRUE((a) != (b), msg)

#define ASSERT_STR_EQUAL(a, b, msg) \
    ASSERT_TRUE(strcmp(a, b) == 0, msg)

#define ASSERT_NULL(ptr, msg) \
    ASSERT_TRUE((ptr) == NULL, msg)

#define ASSERT_NOT_NULL(ptr, msg) \
    ASSERT_TRUE((ptr) != NULL, msg)

#define ASSERT_DOUBLE_EQUAL(a, b, epsilon, msg) \
    ASSERT_TRUE(fabs((a) - (b)) < epsilon, msg)

/* Fonction pour afficher les résultats */
void print_test_results(const char *module_name, TestStats *stats);

/* Fonction pour initialiser les stats */
void init_test_stats(TestStats *stats);

#endif /* TEST_FRAMEWORK_H */
