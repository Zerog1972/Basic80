#include "tstfrmwk.h"
#include "../../include/vars.h"
#include "../../include/interp.h"
#include <stdlib.h>
#include <math.h>

/* Test: Création et récupération de variable numérique */
void test_variable_numeric(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Variables numériques ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    setVariable(interp, "A", 42.0);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 42.0, 0.001, "Récupération de A=42");
    
    setVariable(interp, "B", -10.5);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), -10.5, 0.001, "Récupération de B=-10.5");
    
    freeInterpreter(interp);
}

/* Test: Variables chaînes */
void test_variable_string(TestStats *stats) {
    Interpreter *interp;
    char *result;
    
    printf("\n--- Test: Variables chaînes ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    setStringVariable(interp, "A$", "Hello");
    result = getStringVariable(interp, "A$");
    ASSERT_STR_EQUAL(result, "Hello", "Récupération de A$=\"Hello\"");
    
    setStringVariable(interp, "B$", "World");
    result = getStringVariable(interp, "B$");
    ASSERT_STR_EQUAL(result, "World", "Récupération de B$=\"World\"");
    
    freeInterpreter(interp);
}

/* Test: Modification de variable existante */
void test_variable_modification(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Modification de variables ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    setVariable(interp, "X", 10.0);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 10.0, 0.001, "X initialisé à 10");
    
    setVariable(interp, "X", 20.0);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 20.0, 0.001, "X modifié à 20");
    
    freeInterpreter(interp);
}

/* Test: Tableau 1D */
void test_array_1d(TestStats *stats) {
    Interpreter *interp;
    int dims[1];
    int indices[1];
    
    printf("\n--- Test: Tableaux 1D ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    dims[0] = 10;
    createArray(interp, "A", dims, 1);
    
    indices[0] = 5;
    setArrayElement(interp, "A", indices, 1, 42.0);
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "A", indices, 1), 42.0, 0.001, "A(5) = 42");
    
    indices[0] = 3;
    setArrayElement(interp, "A", indices, 1, -7.5);
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "A", indices, 1), -7.5, 0.001, "A(3) = -7.5");
    
    freeInterpreter(interp);
}

/* Test: Tableau 2D */
void test_array_2d(TestStats *stats) {
    Interpreter *interp;
    int dims[2];
    int indices[2];
    
    printf("\n--- Test: Tableaux 2D ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    dims[0] = 5;
    dims[1] = 5;
    createArray(interp, "M", dims, 2);
    
    indices[0] = 2;
    indices[1] = 3;
    setArrayElement(interp, "M", indices, 2, 99.0);
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "M", indices, 2), 99.0, 0.001, "M(2,3) = 99");
    
    freeInterpreter(interp);
}

/* Test: Tableau 3D */
void test_array_3d(TestStats *stats) {
    Interpreter *interp;
    int dims[3];
    int indices[3];
    
    printf("\n--- Test: Tableaux 3D ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    dims[0] = 3;
    dims[1] = 3;
    dims[2] = 3;
    createArray(interp, "T", dims, 3);
    
    indices[0] = 1;
    indices[1] = 2;
    indices[2] = 1;
    setArrayElement(interp, "T", indices, 3, 123.0);
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "T", indices, 3), 123.0, 0.001, "T(1,2,1) = 123");
    
    freeInterpreter(interp);
}

/* Test: Variable inexistante */
void test_variable_undefined(TestStats *stats) {
    Interpreter *interp;
    
    printf("\n--- Test: Variable non définie ---\n");
    
    interp = createInterpreter();
    ASSERT_NOT_NULL(interp, "Création de l'interpréteur");
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "UNDEFINED"), 0.0, 0.001, "Variable non définie retourne 0");
    ASSERT_STR_EQUAL(getStringVariable(interp, "UNDEFINED$"), "", "Variable chaîne non définie retourne \"\"");
    
    freeInterpreter(interp);
}

void run_variables_tests(void) {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║    TESTS UNITAIRES - VARIABLES        ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_variable_numeric(&stats);
    test_variable_string(&stats);
    test_variable_modification(&stats);
    test_array_1d(&stats);
    test_array_2d(&stats);
    test_array_3d(&stats);
    test_variable_undefined(&stats);
    
    print_test_results("VARIABLES", &stats);
}
