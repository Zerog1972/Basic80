#include "tstfrmwk.h"
#include "../../include/lexer.h"
#include <stdlib.h>

/* Test: Tokenization de nombres */
void test_lexer_numbers(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization de nombres ---\n");
    
    tokens = tokenize("10 20.5 -30");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_NUMBER, "Premier token est un nombre");
    ASSERT_STR_EQUAL(tokens[0].value, "10", "Valeur du premier nombre");
    ASSERT_EQUAL(tokens[1].type, TOK_NUMBER, "Deuxième token est un nombre");
    ASSERT_STR_EQUAL(tokens[1].value, "20.5", "Valeur du nombre décimal");
    freeTokens(tokens);
}

/* Test: Tokenization de chaînes */
void test_lexer_strings(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization de chaînes ---\n");
    
    tokens = tokenize("PRINT \"Hello World\"");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_PRINT, "Premier token est PRINT");
    ASSERT_EQUAL(tokens[1].type, TOK_STRING, "Deuxième token est une chaîne");
    ASSERT_STR_EQUAL(tokens[1].value, "Hello World", "Valeur de la chaîne");
    freeTokens(tokens);
}

/* Test: Tokenization de mots-clés */
void test_lexer_keywords(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization de mots-clés ---\n");
    
    tokens = tokenize("IF THEN ELSE GOTO");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_IF, "IF reconnu");
    ASSERT_EQUAL(tokens[1].type, TOK_THEN, "THEN reconnu");
    ASSERT_EQUAL(tokens[2].type, TOK_ELSE, "ELSE reconnu");
    ASSERT_EQUAL(tokens[3].type, TOK_GOTO, "GOTO reconnu");
    freeTokens(tokens);
}

/* Test: Tokenization d'identifiants */
void test_lexer_identifiers(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization d'identifiants ---\n");
    
    tokens = tokenize("A B$ X1 VAR_NAME");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_IDENTIFIER, "A est un identifiant");
    ASSERT_STR_EQUAL(tokens[0].value, "A", "Valeur de A");
    ASSERT_EQUAL(tokens[1].type, TOK_IDENTIFIER, "B$ est un identifiant");
    ASSERT_STR_EQUAL(tokens[1].value, "B$", "Valeur de B$");
    freeTokens(tokens);
}

/* Test: Tokenization d'opérateurs */
void test_lexer_operators(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization d'opérateurs ---\n");
    
    tokens = tokenize("+ - * / = < > <= >= <>");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_PLUS, "+ reconnu");
    ASSERT_EQUAL(tokens[1].type, TOK_MINUS, "- reconnu");
    ASSERT_EQUAL(tokens[2].type, TOK_MULTIPLY, "* reconnu");
    ASSERT_EQUAL(tokens[3].type, TOK_DIVIDE, "/ reconnu");
    ASSERT_EQUAL(tokens[4].type, TOK_EQUALS, "= reconnu");
    ASSERT_EQUAL(tokens[5].type, TOK_LT, "< reconnu");
    ASSERT_EQUAL(tokens[6].type, TOK_GT, "> reconnu");
    ASSERT_EQUAL(tokens[7].type, TOK_LE, "<= reconnu");
    ASSERT_EQUAL(tokens[8].type, TOK_GE, ">= reconnu");
    ASSERT_EQUAL(tokens[9].type, TOK_NE, "<> reconnu");
    freeTokens(tokens);
}

/* Test: Tokenization avec parenthèses */
void test_lexer_parentheses(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization avec parenthèses ---\n");
    
    tokens = tokenize("(A + B) * C");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_LPAREN, "( reconnu");
    ASSERT_EQUAL(tokens[4].type, TOK_RPAREN, ") reconnu");
    freeTokens(tokens);
}

/* Test: Tokenization de virgules */
void test_lexer_commas(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Tokenization de virgules ---\n");
    
    tokens = tokenize("A, B, C");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_IDENTIFIER, "Premier identifiant");
    ASSERT_EQUAL(tokens[1].type, TOK_COMMA, "Virgule reconnue");
    ASSERT_EQUAL(tokens[2].type, TOK_IDENTIFIER, "Deuxième identifiant");
    freeTokens(tokens);
}

/* Test: Ligne vide */
void test_lexer_empty_line(TestStats *stats) {
    Token *tokens;
    
    printf("\n--- Test: Ligne vide ---\n");
    
    tokens = tokenize("");
    ASSERT_NOT_NULL(tokens, "Tokenize retourne un résultat");
    ASSERT_EQUAL(tokens[0].type, TOK_EOF, "Token EOF pour ligne vide");
    freeTokens(tokens);
}

void run_lexer_tests(void) {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║      TESTS UNITAIRES - LEXER          ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_lexer_numbers(&stats);
    test_lexer_strings(&stats);
    test_lexer_keywords(&stats);
    test_lexer_identifiers(&stats);
    test_lexer_operators(&stats);
    test_lexer_parentheses(&stats);
    test_lexer_commas(&stats);
    test_lexer_empty_line(&stats);
    
    print_test_results("LEXER", &stats);
}
