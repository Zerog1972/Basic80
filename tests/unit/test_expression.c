#include "test_framework.h"
#include "../../include/expression.h"
#include "../../include/interpreter.h"
#include "../../include/lexer.h"
#include <stdlib.h>
#include <math.h>

/* Test: Expressions arithmétiques simples */
void test_expression_arithmetic(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    double result;
    
    printf("\n--- Test: Expressions arithmétiques ---\n");
    
    interp = createInterpreter();
    
    tokens = tokenize("10 + 5");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 15.0, 0.001, "10 + 5 = 15");
    freeTokens(tokens);
    
    tokens = tokenize("20 - 8");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 12.0, 0.001, "20 - 8 = 12");
    freeTokens(tokens);
    
    tokens = tokenize("6 * 7");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 42.0, 0.001, "6 * 7 = 42");
    freeTokens(tokens);
    
    tokens = tokenize("100 / 4");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 25.0, 0.001, "100 / 4 = 25");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Expressions avec parenthèses */
void test_expression_parentheses(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    double result;
    
    printf("\n--- Test: Expressions avec parenthèses ---\n");
    
    interp = createInterpreter();
    
    tokens = tokenize("(10 + 5) * 2");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 30.0, 0.001, "(10 + 5) * 2 = 30");
    freeTokens(tokens);
    
    tokens = tokenize("10 + (5 * 2)");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 20.0, 0.001, "10 + (5 * 2) = 20");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Priorité des opérateurs */
void test_expression_precedence(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    double result;
    
    printf("\n--- Test: Priorité des opérateurs ---\n");
    
    interp = createInterpreter();
    
    tokens = tokenize("10 + 5 * 2");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 20.0, 0.001, "10 + 5 * 2 = 20 (priorité *)");
    freeTokens(tokens);
    
    tokens = tokenize("100 - 10 / 2");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 95.0, 0.001, "100 - 10 / 2 = 95 (priorité /)");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Expressions avec variables */
void test_expression_variables(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    double result;
    
    printf("\n--- Test: Expressions avec variables ---\n");
    
    interp = createInterpreter();
    setVariable(interp, "A", 10.0);
    setVariable(interp, "B", 5.0);
    
    tokens = tokenize("A + B");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 15.0, 0.001, "A + B = 15");
    freeTokens(tokens);
    
    tokens = tokenize("A * B");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 50.0, 0.001, "A * B = 50");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Fonctions mathématiques */
void test_expression_math_functions(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    double result;
    
    printf("\n--- Test: Fonctions mathématiques ---\n");
    
    interp = createInterpreter();
    
    tokens = tokenize("SIN(0)");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 0.0, 0.001, "SIN(0) = 0");
    freeTokens(tokens);
    
    tokens = tokenize("ABS(-5)");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 5.0, 0.001, "ABS(-5) = 5");
    freeTokens(tokens);
    
    tokens = tokenize("SQR(16)");
    pos = 0;
    result = evaluateExpression(interp, tokens, &pos);
    ASSERT_DOUBLE_EQUAL(result, 4.0, 0.001, "SQR(16) = 4");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Expressions de chaînes */
void test_expression_string(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    char *result;
    
    printf("\n--- Test: Expressions de chaînes ---\n");
    
    interp = createInterpreter();
    setStringVariable(interp, "A$", "Hello");
    setStringVariable(interp, "B$", "World");
    
    tokens = tokenize("A$ + B$");
    pos = 0;
    result = evaluateStringExpression(interp, tokens, &pos);
    ASSERT_STR_EQUAL(result, "HelloWorld", "Concaténation A$ + B$");
    free(result);
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

/* Test: Comparaisons */
void test_expression_comparison(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos;
    int result;
    
    printf("\n--- Test: Comparaisons ---\n");
    
    interp = createInterpreter();
    
    tokens = tokenize("10 > 5");
    pos = 0;
    result = evaluateCondition(interp, tokens, &pos);
    ASSERT_EQUAL(result, 1, "10 > 5 = vrai");
    freeTokens(tokens);
    
    tokens = tokenize("10 < 5");
    pos = 0;
    result = evaluateCondition(interp, tokens, &pos);
    ASSERT_EQUAL(result, 0, "10 < 5 = faux");
    freeTokens(tokens);
    
    tokens = tokenize("10 = 10");
    pos = 0;
    result = evaluateCondition(interp, tokens, &pos);
    ASSERT_EQUAL(result, 1, "10 = 10 = vrai");
    freeTokens(tokens);
    
    freeInterpreter(interp);
}

void run_expression_tests(void) {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║   TESTS UNITAIRES - EXPRESSIONS       ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_expression_arithmetic(&stats);
    test_expression_parentheses(&stats);
    test_expression_precedence(&stats);
    test_expression_variables(&stats);
    test_expression_math_functions(&stats);
    test_expression_string(&stats);
    test_expression_comparison(&stats);
    
    print_test_results("EXPRESSIONS", &stats);
}
