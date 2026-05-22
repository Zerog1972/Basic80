#include "interp.h"
#include <math.h>

/* Variables globales pour les tests */
static int testsTotal = 0;
static int testsReussis = 0;
static int testsEchoues = 0;

/* Macro pour les assertions */
#define ASSERT(condition, message) \
    do { \
        testsTotal++; \
        if (condition) { \
            testsReussis++; \
            printf("  [OK] %s\n", message); \
        } else { \
            testsEchoues++; \
            printf("  [ECHEC] %s\n", message); \
        } \
    } while (0)

#define ASSERT_EQUAL(a, b, message) \
    do { \
        testsTotal++; \
        if ((a) == (b)) { \
            testsReussis++; \
            printf("  [OK] %s\n", message); \
        } else { \
            testsEchoues++; \
            printf("  [ECHEC] %s (attendu: %d, obtenu: %d)\n", message, (int)(b), (int)(a)); \
        } \
    } while (0)

#define ASSERT_DOUBLE_EQUAL(a, b, message) \
    do { \
        testsTotal++; \
        if (fabs((a) - (b)) < 0.001) { \
            testsReussis++; \
            printf("  [OK] %s\n", message); \
        } else { \
            testsEchoues++; \
            printf("  [ECHEC] %s (attendu: %.2f, obtenu: %.2f)\n", message, (b), (a)); \
        } \
    } while (0)

#define ASSERT_STRING_EQUAL(a, b, message) \
    do { \
        testsTotal++; \
        if (strcmp((a), (b)) == 0) { \
            testsReussis++; \
            printf("  [OK] %s\n", message); \
        } else { \
            testsEchoues++; \
            printf("  [ECHEC] %s (attendu: %s, obtenu: %s)\n", message, (b), (a)); \
        } \
    } while (0)

/* Tests du lexer */
void test_lexer_nombres(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Nombres ===\n");
    
    tokens = tokenize("42");
    ASSERT(tokens[0].type == TOK_NUMBER, "Tokenize nombre entier");
    ASSERT_STRING_EQUAL(tokens[0].value, "42", "Valeur du nombre entier");
    ASSERT(tokens[1].type == TOK_EOF, "EOF apres nombre");
    freeTokens(tokens);
    
    tokens = tokenize("3.14");
    ASSERT(tokens[0].type == TOK_NUMBER, "Tokenize nombre decimal");
    ASSERT_STRING_EQUAL(tokens[0].value, "3.14", "Valeur du nombre decimal");
    freeTokens(tokens);
    
    tokens = tokenize("10 20 30");
    ASSERT(tokens[0].type == TOK_NUMBER, "Premier nombre");
    ASSERT(tokens[1].type == TOK_NUMBER, "Deuxieme nombre");
    ASSERT(tokens[2].type == TOK_NUMBER, "Troisieme nombre");
    freeTokens(tokens);
}

void test_lexer_identifiants(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Identifiants ===\n");
    
    tokens = tokenize("X");
    ASSERT(tokens[0].type == TOK_IDENTIFIER, "Tokenize identifiant simple");
    ASSERT_STRING_EQUAL(tokens[0].value, "X", "Valeur de l'identifiant");
    freeTokens(tokens);
    
    tokens = tokenize("variable");
    ASSERT(tokens[0].type == TOK_IDENTIFIER, "Tokenize identifiant long");
    ASSERT_STRING_EQUAL(tokens[0].value, "VARIABLE", "Conversion en majuscules");
    freeTokens(tokens);
    
    tokens = tokenize("var_123");
    ASSERT(tokens[0].type == TOK_IDENTIFIER, "Identifiant avec underscore et chiffres");
    freeTokens(tokens);
    
    tokens = tokenize("NOM$");
    ASSERT(tokens[0].type == TOK_IDENTIFIER, "Identifiant avec suffixe $");
    ASSERT_STRING_EQUAL(tokens[0].value, "NOM$", "Valeur identifiant string");
    freeTokens(tokens);
}

void test_lexer_mots_cles(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Mots-cles ===\n");
    
    tokens = tokenize("PRINT");
    ASSERT(tokens[0].type == TOK_PRINT, "Mot-cle PRINT");
    freeTokens(tokens);
    
    tokens = tokenize("LET");
    ASSERT(tokens[0].type == TOK_LET, "Mot-cle LET");
    freeTokens(tokens);
    
    tokens = tokenize("IF");
    ASSERT(tokens[0].type == TOK_IF, "Mot-cle IF");
    freeTokens(tokens);
    
    tokens = tokenize("GOTO");
    ASSERT(tokens[0].type == TOK_GOTO, "Mot-cle GOTO");
    freeTokens(tokens);
    
    tokens = tokenize("INPUT");
    ASSERT(tokens[0].type == TOK_INPUT, "Mot-cle INPUT");
    freeTokens(tokens);
    
    tokens = tokenize("END");
    ASSERT(tokens[0].type == TOK_END, "Mot-cle END");
    freeTokens(tokens);
}

void test_lexer_operateurs(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Operateurs ===\n");
    
    tokens = tokenize("+");
    ASSERT(tokens[0].type == TOK_PLUS, "Operateur +");
    freeTokens(tokens);
    
    tokens = tokenize("-");
    ASSERT(tokens[0].type == TOK_MINUS, "Operateur -");
    freeTokens(tokens);
    
    tokens = tokenize("*");
    ASSERT(tokens[0].type == TOK_MULTIPLY, "Operateur *");
    freeTokens(tokens);
    
    tokens = tokenize("/");
    ASSERT(tokens[0].type == TOK_DIVIDE, "Operateur /");
    freeTokens(tokens);
    
    tokens = tokenize("=");
    ASSERT(tokens[0].type == TOK_EQUALS, "Operateur =");
    freeTokens(tokens);
    
    tokens = tokenize("<");
    ASSERT(tokens[0].type == TOK_LT, "Operateur <");
    freeTokens(tokens);
    
    tokens = tokenize(">");
    ASSERT(tokens[0].type == TOK_GT, "Operateur >");
    freeTokens(tokens);
    
    tokens = tokenize("<=");
    ASSERT(tokens[0].type == TOK_LE, "Operateur <=");
    freeTokens(tokens);
    
    tokens = tokenize(">=");
    ASSERT(tokens[0].type == TOK_GE, "Operateur >=");
    freeTokens(tokens);
    
    tokens = tokenize("<>");
    ASSERT(tokens[0].type == TOK_NE, "Operateur <>");
    freeTokens(tokens);
}

void test_lexer_chaines(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Chaines ===\n");
    
    tokens = tokenize("\"Hello\"");
    ASSERT(tokens[0].type == TOK_STRING, "Tokenize chaine simple");
    ASSERT_STRING_EQUAL(tokens[0].value, "Hello", "Contenu de la chaine");
    freeTokens(tokens);
    
    tokens = tokenize("\"Hello World\"");
    ASSERT(tokens[0].type == TOK_STRING, "Chaine avec espace");
    ASSERT_STRING_EQUAL(tokens[0].value, "Hello World", "Contenu avec espace");
    freeTokens(tokens);
    
    tokens = tokenize("\"\" ");
    ASSERT(tokens[0].type == TOK_STRING, "Chaine vide");
    ASSERT_STRING_EQUAL(tokens[0].value, "", "Chaine vide correcte");
    freeTokens(tokens);
}

void test_lexer_erreurs(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Gestion d'erreurs ===\n");
    
    /* Test chaine non fermee */
    tokens = tokenize("\"Hello");
    ASSERT(tokens == NULL || tokens[0].type == TOK_EOF, "Chaine non fermee gere");
    if (tokens) freeTokens(tokens);
    
    /* Test allocation normale */
    tokens = tokenize("PRINT \"OK\"");
    ASSERT(tokens != NULL, "Allocation reussie pour ligne normale");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_PRINT, "Token PRINT correct");
        freeTokens(tokens);
    }
    
    /* Test freeTokens avec NULL */
    freeTokens(NULL);
    ASSERT(1, "freeTokens(NULL) ne plante pas");
}

void test_lexer_nouveaux_mots_cles(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Nouveaux mots-cles SAVE/LOAD/AND/OR/NOT ===\n");
    
    tokens = tokenize("SAVE");
    ASSERT(tokens != NULL, "SAVE tokenize");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_SAVE, "Mot-cle SAVE");
        freeTokens(tokens);
    }
    
    tokens = tokenize("LOAD");
    ASSERT(tokens != NULL, "LOAD tokenize");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_LOAD, "Mot-cle LOAD");
        freeTokens(tokens);
    }

    tokens = tokenize("AND");
    ASSERT(tokens != NULL, "AND tokenize");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_AND, "Mot-cle AND");
        freeTokens(tokens);
    }

    tokens = tokenize("OR");
    ASSERT(tokens != NULL, "OR tokenize");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_OR, "Mot-cle OR");
        freeTokens(tokens);
    }

    tokens = tokenize("NOT");
    ASSERT(tokens != NULL, "NOT tokenize");
    if (tokens) {
        ASSERT(tokens[0].type == TOK_NOT, "Mot-cle NOT");
        freeTokens(tokens);
    }
}

void test_lexer_expression_complete(void) {
    Token *tokens;
    
    printf("\n=== Tests du lexer - Expressions completes ===\n");
    
    tokens = tokenize("LET X = 10 + 20");
    ASSERT(tokens[0].type == TOK_LET, "Token LET");
    ASSERT(tokens[1].type == TOK_IDENTIFIER, "Token X");
    ASSERT(tokens[2].type == TOK_EQUALS, "Token =");
    ASSERT(tokens[3].type == TOK_NUMBER, "Token 10");
    ASSERT(tokens[4].type == TOK_PLUS, "Token +");
    ASSERT(tokens[5].type == TOK_NUMBER, "Token 20");
    ASSERT(tokens[6].type == TOK_EOF, "Token EOF");
    freeTokens(tokens);
    
    tokens = tokenize("PRINT \"Result:\", X");
    ASSERT(tokens[0].type == TOK_PRINT, "PRINT dans expression");
    ASSERT(tokens[1].type == TOK_STRING, "STRING dans expression");
    ASSERT(tokens[2].type == TOK_COMMA, "COMMA dans expression");
    ASSERT(tokens[3].type == TOK_IDENTIFIER, "IDENTIFIER dans expression");
    freeTokens(tokens);

    tokens = tokenize("INPUT A, B$, C");
    ASSERT(tokens[0].type == TOK_INPUT, "INPUT dans expression complete");
    ASSERT(tokens[1].type == TOK_IDENTIFIER, "Variable A apres INPUT");
    ASSERT(tokens[2].type == TOK_COMMA, "COMMA apres A");
    ASSERT(tokens[3].type == TOK_IDENTIFIER, "Variable B$ apres COMMA");
    ASSERT(tokens[4].type == TOK_COMMA, "COMMA apres B$");
    ASSERT(tokens[5].type == TOK_IDENTIFIER, "Variable C apres second COMMA");
    ASSERT(tokens[6].type == TOK_EOF, "EOF apres liste INPUT");
    freeTokens(tokens);
}

void test_interpreteur_input_multi(void) {
    Interpreter *interp;
    FILE *f;

    printf("\n=== Tests de l'interpreteur - INPUT multi-variables ===\n");

    f = fopen("tests_input_multi.tmp", "w");
    if (!f) {
        ASSERT(0, "Creation fichier temporaire INPUT");
        return;
    }
    fputs("12.5\nHELLO\n7\n", f);
    fclose(f);

    if (!freopen("tests_input_multi.tmp", "r", stdin)) {
        ASSERT(0, "Redirection stdin pour INPUT multi");
        remove("tests_input_multi.tmp");
        return;
    }

    interp = createInterpreter();
    executeCommand(interp, "INPUT A, B$, C");

    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 12.5, "INPUT A lit la valeur numerique");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "HELLO", "INPUT B$ lit la chaine");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 7.0, "INPUT C lit la deuxieme valeur numerique");

    freeInterpreter(interp);
    remove("tests_input_multi.tmp");
}

void test_interpreteur_input_multi_programme(void) {
    Interpreter *interp;
    FILE *f;

    printf("\n=== Tests de l'interpreteur - INPUT multi dans RUN ===\n");

    f = fopen("tests_input_multi_run.tmp", "w");
    if (!f) {
        ASSERT(0, "Creation fichier temporaire INPUT RUN");
        return;
    }
    fputs("12.5\nHELLO\n7\n", f);
    fclose(f);

    if (!freopen("tests_input_multi_run.tmp", "r", stdin)) {
        ASSERT(0, "Redirection stdin pour INPUT RUN");
        remove("tests_input_multi_run.tmp");
        return;
    }

    interp = createInterpreter();
    addLine(interp, 10, "INPUT A, B$, C");
    addLine(interp, 20, "LET D = A + C");
    addLine(interp, 30, "LET E = LEN(B$)");
    addLine(interp, 40, "END");

    runProgram(interp);

    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 12.5, "RUN INPUT A lit la valeur numerique");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "HELLO", "RUN INPUT B$ lit la chaine");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 7.0, "RUN INPUT C lit la deuxieme valeur numerique");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 19.5, "RUN LET D = A + C apres INPUT multi");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 5.0, "RUN LEN(B$) apres INPUT multi");

    freeInterpreter(interp);
    remove("tests_input_multi_run.tmp");
}

/* Tests de l'interpreteur */
void test_interpreteur_variables(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Variables ===\n");
    
    interp = createInterpreter();
    
    setVariable(interp, "X", 42.0);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 42.0, "Lecture variable X");
    
    setVariable(interp, "Y", 3.14);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 3.14, "Lecture variable Y");
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Z"), 0.0, "Variable inexistante retourne 0");
    
    setVariable(interp, "X", 100.0);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 100.0, "Modification variable X");
    
    freeInterpreter(interp);
}

void test_interpreteur_lignes(void) {
    Interpreter *interp;
    Line *line;
    
    printf("\n=== Tests de l'interpreteur - Gestion des lignes ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "PRINT \"Hello\"");
    ASSERT(interp->program != NULL, "Programme non vide");
    ASSERT_EQUAL(interp->program->lineNum, 10, "Numero de ligne 10");
    
    addLine(interp, 20, "LET X = 5");
    line = interp->program->next;
    ASSERT(line != NULL, "Deuxieme ligne ajoutee");
    ASSERT_EQUAL(line->lineNum, 20, "Numero de ligne 20");
    
    addLine(interp, 5, "REM Commentaire");
    ASSERT_EQUAL(interp->program->lineNum, 5, "Ligne 5 inseree au debut");
    
    addLine(interp, 15, "PRINT X");
    line = interp->program->next;
    ASSERT_EQUAL(line->lineNum, 10, "Ligne 10 en position 2");
    line = line->next;
    ASSERT_EQUAL(line->lineNum, 15, "Ligne 15 inseree entre 10 et 20");
    
    addLine(interp, 10, "PRINT \"Modified\"");
    line = interp->program->next;
    ASSERT_STRING_EQUAL(line->code, "PRINT \"Modified\"", "Ligne 10 modifiee");
    
    freeInterpreter(interp);
}

void test_interpreteur_let(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Commande LET ===\n");
    
    interp = createInterpreter();
    
    executeCommand(interp, "LET X = 42");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 42.0, "LET X = 42");
    
    executeCommand(interp, "LET Y = 10");
    executeCommand(interp, "LET Z = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 10.0, "LET Y = 10");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Z"), 5.0, "LET Z = 5");
    
    executeCommand(interp, "LET A = 3.14");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 3.14, "LET avec decimal");
    
    freeInterpreter(interp);
}

void test_interpreteur_expressions(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Expressions ===\n");
    
    interp = createInterpreter();
    
    executeCommand(interp, "LET X = 10");
    executeCommand(interp, "LET Y = 20");
    
    executeCommand(interp, "LET A = 5 + 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 8.0, "Addition: 5 + 3");
    
    executeCommand(interp, "LET B = 10 - 4");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 6.0, "Soustraction: 10 - 4");
    
    executeCommand(interp, "LET C = 6 * 7");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 42.0, "Multiplication: 6 * 7");
    
    executeCommand(interp, "LET D = 20 / 4");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 5.0, "Division: 20 / 4");
    
    executeCommand(interp, "LET E = X + Y");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 30.0, "Addition de variables: X + Y");
    
    executeCommand(interp, "LET F = X + 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), 15.0, "Variable + constante");
    
    executeCommand(interp, "LET G = 100 - X - Y");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "G"), 70.0, "Expression multiple: 100 - X - Y");
    
    freeInterpreter(interp);
}

void test_interpreteur_programme(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Execution programme ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET X = 5");
    addLine(interp, 20, "LET Y = 10");
    addLine(interp, 30, "LET Z = X + Y");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 5.0, "X apres execution");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 10.0, "Y apres execution");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Z"), 15.0, "Z = X + Y apres execution");
    
    freeInterpreter(interp);
}

void test_interpreteur_goto(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - GOTO ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET X = 1");
    addLine(interp, 20, "LET X = X + 1");
    addLine(interp, 30, "GOTO 50");
    addLine(interp, 40, "LET X = 999");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 2.0, "GOTO saute ligne 40");
    
    freeInterpreter(interp);
}

void test_interpreteur_if_then_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - IF...THEN simple ===\n");
    
    interp = createInterpreter();
    
    /* Test condition vraie */
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "LET Y = 0");
    addLine(interp, 30, "IF X > 5 THEN LET Y = 1");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 1.0, "Condition vraie: Y doit etre 1");
    
    freeInterpreter(interp);
    
    /* Test condition fausse */
    interp = createInterpreter();
    addLine(interp, 10, "LET X = 3");
    addLine(interp, 20, "LET Y = 0");
    addLine(interp, 30, "IF X > 5 THEN LET Y = 1");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 0.0, "Condition fausse: Y doit rester 0");
    
    freeInterpreter(interp);
}

void test_interpreteur_if_then_else(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - IF...THEN...ELSE ===\n");
    
    interp = createInterpreter();
    
    /* Condition vraie */
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "IF X > 5 THEN LET Y = 100 ELSE LET Y = 200");
    addLine(interp, 30, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 100.0, "Condition vraie: THEN execute");
    
    freeInterpreter(interp);
    
    /* Condition fausse */
    interp = createInterpreter();
    addLine(interp, 10, "LET X = 3");
    addLine(interp, 20, "IF X > 5 THEN LET Y = 100 ELSE LET Y = 200");
    addLine(interp, 30, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 200.0, "Condition fausse: ELSE execute");
    
    freeInterpreter(interp);
}

void test_interpreteur_if_operateurs(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - IF avec operateurs ===\n");
    
    interp = createInterpreter();
    
    /* Test < */
    addLine(interp, 10, "LET A = 0");
    addLine(interp, 20, "IF 5 < 10 THEN LET A = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 1.0, "Test operateur <");
    freeInterpreter(interp);
    
    /* Test > */
    interp = createInterpreter();
    addLine(interp, 10, "LET B = 0");
    addLine(interp, 20, "IF 10 > 5 THEN LET B = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 1.0, "Test operateur >");
    freeInterpreter(interp);
    
    /* Test <= */
    interp = createInterpreter();
    addLine(interp, 10, "LET C = 0");
    addLine(interp, 20, "IF 5 <= 5 THEN LET C = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 1.0, "Test operateur <=");
    freeInterpreter(interp);
    
    /* Test >= */
    interp = createInterpreter();
    addLine(interp, 10, "LET D = 0");
    addLine(interp, 20, "IF 10 >= 10 THEN LET D = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 1.0, "Test operateur >=");
    freeInterpreter(interp);
    
    /* Test = */
    interp = createInterpreter();
    addLine(interp, 10, "LET E = 0");
    addLine(interp, 20, "IF 7 = 7 THEN LET E = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 1.0, "Test operateur =");
    freeInterpreter(interp);
    
    /* Test <> */
    interp = createInterpreter();
    addLine(interp, 10, "LET F = 0");
    addLine(interp, 20, "IF 5 <> 10 THEN LET F = 1");
    addLine(interp, 30, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), 1.0, "Test operateur <>");
    freeInterpreter(interp);
}

void test_interpreteur_if_then_goto(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - IF...THEN GOTO ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "IF X > 5 THEN GOTO 50");
    addLine(interp, 30, "LET X = 999");
    addLine(interp, 40, "GOTO 60");
    addLine(interp, 50, "LET X = 42");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 42.0, "IF...THEN GOTO saute correctement");
    
    freeInterpreter(interp);
}

void test_interpreteur_if_else_goto(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - IF...THEN...ELSE avec GOTO ===\n");
    
    interp = createInterpreter();
    
    /* Condition vraie */
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "IF X > 5 THEN GOTO 40 ELSE GOTO 50");
    addLine(interp, 30, "LET X = 999");
    addLine(interp, 40, "LET X = 100");
    addLine(interp, 45, "GOTO 60");
    addLine(interp, 50, "LET X = 200");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 100.0, "Condition vraie: THEN GOTO execute");
    
    freeInterpreter(interp);
    
    /* Condition fausse */
    interp = createInterpreter();
    addLine(interp, 10, "LET X = 3");
    addLine(interp, 20, "IF X > 5 THEN GOTO 40 ELSE GOTO 50");
    addLine(interp, 30, "LET X = 999");
    addLine(interp, 40, "LET X = 100");
    addLine(interp, 45, "GOTO 60");
    addLine(interp, 50, "LET X = 200");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 200.0, "Condition fausse: ELSE GOTO execute");
    
    freeInterpreter(interp);
}

void test_interpreteur_if_logique(void) {
    Interpreter *interp;

    printf("\n=== Tests de l'interpreteur - IF logique AND/OR/NOT ===\n");

    interp = createInterpreter();
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "LET Y = 3");
    addLine(interp, 30, "LET A = 0");
    addLine(interp, 40, "IF X > 5 AND Y < 5 THEN LET A = 1");
    addLine(interp, 50, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 1.0, "AND combine deux comparaisons");
    freeInterpreter(interp);

    interp = createInterpreter();
    addLine(interp, 10, "LET X = 1");
    addLine(interp, 20, "LET B = 0");
    addLine(interp, 30, "IF X = 1 OR X = 2 THEN LET B = 1");
    addLine(interp, 40, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 1.0, "OR valide si une condition est vraie");
    freeInterpreter(interp);

    interp = createInterpreter();
    addLine(interp, 10, "LET X = 0");
    addLine(interp, 20, "LET C = 0");
    addLine(interp, 30, "IF NOT X THEN LET C = 1");
    addLine(interp, 40, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 1.0, "NOT inverse une condition numerique");
    freeInterpreter(interp);

    interp = createInterpreter();
    addLine(interp, 10, "LET X = 10");
    addLine(interp, 20, "LET Y = 5");
    addLine(interp, 30, "LET D = 0");
    addLine(interp, 40, "IF NOT (X < 5 OR Y = 6) THEN LET D = 1");
    addLine(interp, 50, "END");
    runProgram(interp);
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 1.0, "Parentheses et priorite logique");
    freeInterpreter(interp);
}

void test_interpreteur_dim_simple(void) {
    Interpreter *interp;
    int idx[1];
    
    printf("\n=== Tests de l'interpreteur - DIM tableau simple ===\n");
    
    interp = createInterpreter();
    
    /* Créer un tableau et y stocker des valeurs */
    addLine(interp, 10, "DIM A(5)");
    addLine(interp, 20, "LET A(0) = 10");
    addLine(interp, 30, "LET A(1) = 20");
    addLine(interp, 40, "LET A(2) = 30");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    idx[0] = 0;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "A", idx, 1), 10.0, "A(0) = 10");
    idx[0] = 1;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "A", idx, 1), 20.0, "A(1) = 20");
    idx[0] = 2;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "A", idx, 1), 30.0, "A(2) = 30");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_boucle(void) {
    Interpreter *interp;
    int idx[1];
    
    printf("\n=== Tests de l'interpreteur - DIM avec boucle FOR ===\n");
    
    interp = createInterpreter();
    
    /* Remplir un tableau avec une boucle */
    addLine(interp, 10, "DIM B(10)");
    addLine(interp, 20, "FOR I = 0 TO 10");
    addLine(interp, 30, "LET B(I) = I * 2");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    idx[0] = 0;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "B", idx, 1), 0.0, "B(0) = 0");
    idx[0] = 5;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "B", idx, 1), 10.0, "B(5) = 10");
    idx[0] = 10;
    ASSERT_DOUBLE_EQUAL(getArrayElement(interp, "B", idx, 1), 20.0, "B(10) = 20");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_lecture(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Lecture elements tableau ===\n");
    
    interp = createInterpreter();
    
    /* Utiliser des éléments de tableau dans des calculs */
    addLine(interp, 10, "DIM C(3)");
    addLine(interp, 20, "LET C(0) = 5");
    addLine(interp, 30, "LET C(1) = 10");
    addLine(interp, 40, "LET C(2) = 15");
    addLine(interp, 50, "LET SOMME = C(0) + C(1) + C(2)");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "SOMME"), 30.0, "Somme des elements = 30");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_calcul_somme(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - DIM calcul somme ===\n");
    
    interp = createInterpreter();
    
    /* Calculer la somme d'un tableau */
    addLine(interp, 10, "DIM D(5)");
    addLine(interp, 20, "LET D(0) = 10");
    addLine(interp, 30, "LET D(1) = 20");
    addLine(interp, 40, "LET D(2) = 30");
    addLine(interp, 50, "LET D(3) = 40");
    addLine(interp, 60, "LET D(4) = 50");
    addLine(interp, 70, "LET TOTAL = 0");
    addLine(interp, 80, "FOR I = 0 TO 4");
    addLine(interp, 90, "LET TOTAL = TOTAL + D(I)");
    addLine(interp, 100, "NEXT I");
    addLine(interp, 110, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "TOTAL"), 150.0, "Somme totale = 150");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_indice_variable(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - DIM avec indice variable ===\n");
    
    interp = createInterpreter();
    
    /* Utiliser une variable comme indice */
    addLine(interp, 10, "DIM E(5)");
    addLine(interp, 20, "FOR I = 0 TO 5");
    addLine(interp, 30, "LET E(I) = I * I");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "LET IDX = 3");
    addLine(interp, 60, "LET RESULT = E(IDX)");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "RESULT"), 9.0, "E(3) = 3*3 = 9");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_2d_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Tableaux 2D simples ===\n");
    
    interp = createInterpreter();
    
    /* Declaration et affectation de tableau 2D */
    addLine(interp, 10, "DIM M(3, 3)");
    addLine(interp, 20, "LET M(0, 0) = 1");
    addLine(interp, 30, "LET M(0, 1) = 2");
    addLine(interp, 40, "LET M(0, 2) = 3");
    addLine(interp, 50, "LET M(1, 0) = 4");
    addLine(interp, 60, "LET M(1, 1) = 5");
    addLine(interp, 70, "LET M(1, 2) = 6");
    addLine(interp, 80, "LET M(2, 0) = 7");
    addLine(interp, 90, "LET M(2, 1) = 8");
    addLine(interp, 100, "LET M(2, 2) = 9");
    addLine(interp, 110, "LET V1 = M(0, 0)");
    addLine(interp, 120, "LET V2 = M(0, 1)");
    addLine(interp, 130, "LET V3 = M(1, 1)");
    addLine(interp, 140, "LET V4 = M(2, 2)");
    addLine(interp, 150, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "V1"), 1.0, "M(0,0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "V2"), 2.0, "M(0,1) = 2");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "V3"), 5.0, "M(1,1) = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "V4"), 9.0, "M(2,2) = 9");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_2d_boucles(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Tableaux 2D avec boucles ===\n");
    
    interp = createInterpreter();
    
    /* Remplir tableau 2D avec boucles imbriquees */
    addLine(interp, 10, "DIM MAT(3, 3)");
    addLine(interp, 20, "FOR I = 0 TO 2");
    addLine(interp, 30, "FOR J = 0 TO 2");
    addLine(interp, 40, "LET MAT(I, J) = I * 3 + J + 1");
    addLine(interp, 50, "NEXT J");
    addLine(interp, 60, "NEXT I");
    addLine(interp, 70, "LET T1 = MAT(0, 0)");
    addLine(interp, 80, "LET T2 = MAT(0, 2)");
    addLine(interp, 90, "LET T3 = MAT(1, 1)");
    addLine(interp, 100, "LET T4 = MAT(2, 0)");
    addLine(interp, 110, "LET T5 = MAT(2, 2)");
    addLine(interp, 120, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "T1"), 1.0, "MAT(0,0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "T2"), 3.0, "MAT(0,2) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "T3"), 5.0, "MAT(1,1) = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "T4"), 7.0, "MAT(2,0) = 7");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "T5"), 9.0, "MAT(2,2) = 9");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_2d_identite(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Matrice identite 2D ===\n");
    
    interp = createInterpreter();
    
    /* Creer une matrice identite avec IF */
    addLine(interp, 10, "DIM ID(3, 3)");
    addLine(interp, 20, "FOR I = 0 TO 2");
    addLine(interp, 30, "FOR J = 0 TO 2");
    addLine(interp, 40, "IF I = J THEN LET ID(I, J) = 1");
    addLine(interp, 50, "IF I <> J THEN LET ID(I, J) = 0");
    addLine(interp, 60, "NEXT J");
    addLine(interp, 70, "NEXT I");
    addLine(interp, 80, "LET D1 = ID(0, 0)");
    addLine(interp, 90, "LET D2 = ID(1, 1)");
    addLine(interp, 100, "LET D3 = ID(2, 2)");
    addLine(interp, 110, "LET H1 = ID(0, 1)");
    addLine(interp, 120, "LET H2 = ID(1, 0)");
    addLine(interp, 130, "LET H3 = ID(2, 1)");
    addLine(interp, 140, "END");
    
    runProgram(interp);
    
    /* Verifier la diagonale */
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D1"), 1.0, "ID(0,0) = 1 (diagonale)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D2"), 1.0, "ID(1,1) = 1 (diagonale)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D3"), 1.0, "ID(2,2) = 1 (diagonale)");
    
    /* Verifier hors diagonale */
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "H1"), 0.0, "ID(0,1) = 0 (hors diagonale)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "H2"), 0.0, "ID(1,0) = 0 (hors diagonale)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "H3"), 0.0, "ID(2,1) = 0 (hors diagonale)");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_2d_expressions(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Tableaux 2D avec expressions ===\n");
    
    interp = createInterpreter();
    
    /* Utiliser des expressions dans les indices */
    addLine(interp, 10, "DIM A(5, 5)");
    addLine(interp, 20, "LET X = 2");
    addLine(interp, 30, "LET Y = 3");
    addLine(interp, 40, "LET A(X, Y) = 42");
    addLine(interp, 50, "LET A(X + 1, Y - 1) = 100");
    addLine(interp, 60, "LET A(X * 2, Y - 2) = 200");
    addLine(interp, 70, "LET R1 = A(2, 3)");
    addLine(interp, 80, "LET R2 = A(3, 2)");
    addLine(interp, 90, "LET R3 = A(4, 1)");
    addLine(interp, 100, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "R1"), 42.0, "A(2,3) = 42");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "R2"), 100.0, "A(3,2) = 100");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "R3"), 200.0, "A(4,1) = 200");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_3d_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Tableaux 3D simples ===\n");
    
    interp = createInterpreter();
    
    /* Declaration et affectation de tableau 3D */
    addLine(interp, 10, "DIM CUBE(2, 2, 2)");
    addLine(interp, 20, "LET CUBE(0, 0, 0) = 1");
    addLine(interp, 30, "LET CUBE(0, 0, 1) = 2");
    addLine(interp, 40, "LET CUBE(0, 1, 0) = 3");
    addLine(interp, 50, "LET CUBE(0, 1, 1) = 4");
    addLine(interp, 60, "LET CUBE(1, 0, 0) = 5");
    addLine(interp, 70, "LET CUBE(1, 0, 1) = 6");
    addLine(interp, 80, "LET CUBE(1, 1, 0) = 7");
    addLine(interp, 90, "LET CUBE(1, 1, 1) = 8");
    addLine(interp, 100, "LET C1 = CUBE(0, 0, 0)");
    addLine(interp, 110, "LET C2 = CUBE(0, 0, 1)");
    addLine(interp, 120, "LET C3 = CUBE(1, 0, 0)");
    addLine(interp, 130, "LET C4 = CUBE(1, 1, 1)");
    addLine(interp, 140, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C1"), 1.0, "CUBE(0,0,0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C2"), 2.0, "CUBE(0,0,1) = 2");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C3"), 5.0, "CUBE(1,0,0) = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C4"), 8.0, "CUBE(1,1,1) = 8");
    
    freeInterpreter(interp);
}

void test_interpreteur_dim_3d_boucles(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Tableaux 3D avec boucles ===\n");
    
    interp = createInterpreter();
    
    /* Remplir tableau 3D avec boucles triples */
    addLine(interp, 10, "DIM CUBE3D(2, 2, 2)");
    addLine(interp, 20, "FOR I = 0 TO 1");
    addLine(interp, 30, "FOR J = 0 TO 1");
    addLine(interp, 40, "FOR K = 0 TO 1");
    addLine(interp, 50, "LET CUBE3D(I, J, K) = I * 4 + J * 2 + K + 1");
    addLine(interp, 60, "NEXT K");
    addLine(interp, 70, "NEXT J");
    addLine(interp, 80, "NEXT I");
    addLine(interp, 90, "LET S1 = CUBE3D(0, 0, 0)");
    addLine(interp, 100, "LET S2 = CUBE3D(0, 0, 1)");
    addLine(interp, 110, "LET S3 = CUBE3D(0, 1, 0)");
    addLine(interp, 120, "LET S4 = CUBE3D(1, 0, 0)");
    addLine(interp, 130, "LET S5 = CUBE3D(1, 1, 1)");
    addLine(interp, 140, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S1"), 1.0, "CUBE3D(0,0,0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S2"), 2.0, "CUBE3D(0,0,1) = 2");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S3"), 3.0, "CUBE3D(0,1,0) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S4"), 5.0, "CUBE3D(1,0,0) = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S5"), 8.0, "CUBE3D(1,1,1) = 8");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Boucle FOR simple ===\n");
    
    interp = createInterpreter();
    
    /* Test 1: Boucle FOR basique de 1 a 5 */
    addLine(interp, 10, "LET TOTAL = 0");
    addLine(interp, 20, "FOR I = 1 TO 5");
    addLine(interp, 30, "LET TOTAL = TOTAL + I");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "I"), 6.0, "Variable I apres boucle (1+5+1)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "TOTAL"), 15.0, "Somme 1+2+3+4+5 = 15");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_step_positif(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Boucle FOR avec STEP positif ===\n");
    
    interp = createInterpreter();
    
    /* Boucle avec STEP 2 */
    addLine(interp, 10, "LET COUNT = 0");
    addLine(interp, 20, "FOR I = 0 TO 10 STEP 2");
    addLine(interp, 30, "LET COUNT = COUNT + 1");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "I"), 12.0, "Variable I apres boucle (0,2,4,6,8,10)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "COUNT"), 6.0, "6 iterations (0,2,4,6,8,10)");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_step_negatif(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Boucle FOR descendante ===\n");
    
    interp = createInterpreter();
    
    /* Boucle descendante */
    addLine(interp, 10, "LET TOTAL = 0");
    addLine(interp, 20, "FOR I = 10 TO 1 STEP -1");
    addLine(interp, 30, "LET TOTAL = TOTAL + I");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "I"), 0.0, "Variable I apres boucle descendante");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "TOTAL"), 55.0, "Somme 10+9+...+1 = 55");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_limites(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - FOR avec limites speciales ===\n");
    
    interp = createInterpreter();
    
    /* Boucle avec debut = fin */
    addLine(interp, 10, "LET COUNT = 0");
    addLine(interp, 20, "FOR I = 5 TO 5");
    addLine(interp, 30, "LET COUNT = COUNT + 1");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "COUNT"), 1.0, "Boucle executee une fois quand debut = fin");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "I"), 6.0, "Variable I incrementee");
    
    freeInterpreter(interp);
    
    /* Boucle qui ne devrait pas s'executer */
    interp = createInterpreter();
    addLine(interp, 10, "LET COUNT = 0");
    addLine(interp, 20, "FOR I = 10 TO 1");
    addLine(interp, 30, "LET COUNT = COUNT + 1");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "COUNT"), 0.0, "Boucle pas executee si debut > fin avec step positif");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_imbriquees(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Boucles FOR imbriquees ===\n");
    
    interp = createInterpreter();
    
    /* Boucles imbriquees */
    addLine(interp, 10, "LET TOTAL = 0");
    addLine(interp, 20, "FOR I = 1 TO 3");
    addLine(interp, 30, "FOR J = 1 TO 2");
    addLine(interp, 40, "LET TOTAL = TOTAL + 1");
    addLine(interp, 50, "NEXT J");
    addLine(interp, 60, "NEXT I");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "TOTAL"), 6.0, "3 * 2 = 6 iterations");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "I"), 4.0, "Variable I apres boucle externe");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "J"), 3.0, "Variable J apres derniere iteration");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_avec_calculs(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - FOR avec calculs ===\n");
    
    interp = createInterpreter();
    
    /* Calcul de factorielle */
    addLine(interp, 10, "LET FACT = 1");
    addLine(interp, 20, "FOR I = 1 TO 5");
    addLine(interp, 30, "LET FACT = FACT * I");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "FACT"), 120.0, "Factorielle de 5 = 120");
    
    freeInterpreter(interp);
    
    /* Somme des carres */
    interp = createInterpreter();
    addLine(interp, 10, "LET SUM = 0");
    addLine(interp, 20, "FOR I = 1 TO 4");
    addLine(interp, 30, "LET SUM = SUM + I * I");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "SUM"), 30.0, "1^2 + 2^2 + 3^2 + 4^2 = 30");
    
    freeInterpreter(interp);
}

void test_interpreteur_for_variables_limites(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - FOR avec variables pour limites ===\n");
    
    interp = createInterpreter();
    
    /* Utiliser des variables pour les limites */
    addLine(interp, 10, "LET START = 2");
    addLine(interp, 20, "LET FINISH = 6");
    addLine(interp, 30, "LET COUNT = 0");
    addLine(interp, 40, "FOR I = START TO FINISH");
    addLine(interp, 50, "LET COUNT = COUNT + 1");
    addLine(interp, 60, "NEXT I");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "COUNT"), 5.0, "5 iterations (2,3,4,5,6)");
    
    freeInterpreter(interp);
}

void test_interpreteur_save_load(void) {
    Interpreter *interp;
    const char *filename = "test_save_load_temp.bas";
    
    printf("\n=== Tests de l'interpreteur - SAVE/LOAD ===\n");
    
    interp = createInterpreter();
    
    /* Creer un programme simple */
    addLine(interp, 10, "PRINT \"Test\"");
    addLine(interp, 20, "LET X = 42");
    addLine(interp, 30, "END");
    
    /* Sauvegarder */
    ASSERT(saveProgram(interp, filename) == 1, "Sauvegarde du programme");
    
    /* Effacer le programme */
    clearProgram(interp);
    ASSERT(interp->program == NULL, "Programme efface");
    
    /* Charger */
    ASSERT(loadProgram(interp, filename) == 1, "Chargement du programme");
    ASSERT(interp->program != NULL, "Programme charge");
    ASSERT(interp->program->lineNum == 10, "Premiere ligne 10");
    ASSERT(interp->program->next != NULL, "Deuxieme ligne existe");
    ASSERT(interp->program->next->lineNum == 20, "Deuxieme ligne 20");
    
    /* Nettoyer */
    freeInterpreter(interp);
    remove(filename);
}

void test_interpreteur_save_fichier_invalide(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - SAVE/LOAD fichiers invalides ===\n");
    
    interp = createInterpreter();
    
    /* Test chargement fichier inexistant */
    ASSERT(loadProgram(interp, "fichier_inexistant_xyz123.bas") == 0, "Echec chargement fichier inexistant");
    
    /* Test sauvegarde dans chemin invalide (optionnel selon OS) */
    addLine(interp, 10, "PRINT \"Test\"");
    /* Le test suivant peut varier selon l'OS */
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_trigo(void) {
    Interpreter *interp;
    double pi = 3.14159265358979323846;
    
    printf("\n=== Tests de l'interpreteur - Fonctions trigonometriques ===\n");
    
    interp = createInterpreter();
    
    /* Test SIN, COS, TAN */
    addLine(interp, 10, "LET PI = 3.14159");
    addLine(interp, 20, "LET A = SIN(0)");
    addLine(interp, 30, "LET B = SIN(PI)");
    addLine(interp, 40, "LET C = COS(0)");
    addLine(interp, 50, "LET D = COS(PI)");
    addLine(interp, 60, "LET E = TAN(0)");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 0.0, "SIN(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 0.0, "SIN(PI) ≈ 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 1.0, "COS(0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), -1.0, "COS(PI) ≈ -1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 0.0, "TAN(0) = 0");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_hyperboliques(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonctions hyperboliques et trigonometriques inverses ===\n");
    
    interp = createInterpreter();
    
    /* Test ATAN, ASIN, ACOS, SINH, COSH, TANH */
    addLine(interp, 10, "LET A = ATAN(0)");
    addLine(interp, 20, "LET B = ATAN(1)"); /* pi/4 */
    addLine(interp, 30, "LET C = SINH(0)");
    addLine(interp, 40, "LET D = COSH(0)");
    addLine(interp, 50, "LET E = TANH(0)");
    addLine(interp, 60, "LET F = ASIN(0)");
    addLine(interp, 70, "LET G = ACOS(1)");
    addLine(interp, 80, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 0.0, "ATAN(0) = 0");
    /* ATAN(1) is approx 0.785398163 */
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 0.785398163, "ATAN(1) = PI/4");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 0.0, "SINH(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 1.0, "COSH(0) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 0.0, "TANH(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), 0.0, "ASIN(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "G"), 0.0, "ACOS(1) = 0");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_angle_conv(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonctions de conversion d'angles ===\n");
    
    interp = createInterpreter();
    
    /* Test RAD et DEG */
    addLine(interp, 10, "LET D = 180");
    addLine(interp, 20, "LET R = RAD(D)"); /* Should be PI (approx 3.14159) */
    addLine(interp, 30, "LET D2 = DEG(R)"); /* Should be 180 */
    addLine(interp, 40, "LET S = SIN(RAD(90))"); /* SIN(PI/2) = 1 */
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    /* Precision tolerance in ASSERT_DOUBLE_EQUAL is usually small, so we might need approximate check */
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "R"), 3.141592653589793, "RAD(180) = PI");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D2"), 180.0, "DEG(PI) = 180");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "S"), 1.0, "SIN(RAD(90)) = 1");
    
    freeInterpreter(interp);
}


void test_interpreteur_fonctions_math(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonctions mathematiques ===\n");
    
    interp = createInterpreter();
    
    /* Test SQR, ABS, INT */
    addLine(interp, 10, "LET A = SQR(16)");
    addLine(interp, 20, "LET B = SQR(2)");
    addLine(interp, 30, "LET C = ABS(-5)");
    addLine(interp, 40, "LET D = ABS(7)");
    addLine(interp, 50, "LET E = INT(3.7)");
    addLine(interp, 60, "LET F = INT(-2.3)");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 4.0, "SQR(16) = 4");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 1.414, "SQR(2) ≈ 1.414");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 5.0, "ABS(-5) = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 7.0, "ABS(7) = 7");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 3.0, "INT(3.7) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), -3.0, "INT(-2.3) = -3");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_composees(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonctions composees ===\n");
    
    interp = createInterpreter();
    
    /* Test de compositions de fonctions */
    addLine(interp, 10, "LET A = SQR(16)");
    addLine(interp, 20, "LET B = ABS(SQR(4) - 5)");
    addLine(interp, 30, "LET C = INT(SQR(10))");
    addLine(interp, 40, "LET D = 2 * SQR(9) + 1");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 4.0, "SQR(16) = 4");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 3.0, "ABS(SQR(4) - 5) = ABS(-3) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 3.0, "INT(SQR(10)) = INT(3.162..) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 7.0, "2 * SQR(9) + 1 = 2 * 3 + 1 = 7");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_avancees(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonctions avancees (LOG, EXP, POW) ===\n");
    
    interp = createInterpreter();
    
    /* Test LOG, EXP, POW, LOG10 */
    addLine(interp, 10, "LET A = EXP(1)");
    addLine(interp, 20, "LET B = LOG(2.718)");
    addLine(interp, 30, "LET C = LOG10(100)");
    addLine(interp, 40, "LET D = LOG10(1000)");
    addLine(interp, 50, "LET E = POW(2, 3)");
    addLine(interp, 60, "LET F = POW(10, 2)");
    addLine(interp, 70, "LET G = POW(5, 0)");
    addLine(interp, 80, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 2.718, "EXP(1) ≈ e ≈ 2.718");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 1.0, "LOG(e) ≈ 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 2.0, "LOG10(100) = 2");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 3.0, "LOG10(1000) = 3");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 8.0, "POW(2, 3) = 2^3 = 8");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), 100.0, "POW(10, 2) = 10^2 = 100");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "G"), 1.0, "POW(5, 0) = 5^0 = 1");
    
    freeInterpreter(interp);
}

void test_interpreteur_gosub_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - GOSUB simple ===\n");
    
    interp = createInterpreter();
    
    /* Test GOSUB et RETURN basique */
    addLine(interp, 10, "LET X = 0");
    addLine(interp, 20, "GOSUB 100");
    addLine(interp, 30, "LET Y = 2");
    addLine(interp, 40, "END");
    addLine(interp, 100, "LET X = 1");
    addLine(interp, 110, "RETURN");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 1.0, "X modifie dans sous-routine");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 2.0, "Y execute apres RETURN");
    
    freeInterpreter(interp);
}

void test_interpreteur_gosub_calcul(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - GOSUB avec calcul ===\n");
    
    interp = createInterpreter();
    
    /* Sous-routine qui calcule le carre d'un nombre */
    addLine(interp, 10, "LET N = 5");
    addLine(interp, 20, "GOSUB 100");
    addLine(interp, 30, "LET RESULT1 = RES");
    addLine(interp, 40, "LET N = 3");
    addLine(interp, 50, "GOSUB 100");
    addLine(interp, 60, "LET RESULT2 = RES");
    addLine(interp, 70, "END");
    addLine(interp, 100, "REM Sous-routine: calcule le carre de N");
    addLine(interp, 110, "LET RES = N * N");
    addLine(interp, 120, "RETURN");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "RESULT1"), 25.0, "5 * 5 = 25");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "RESULT2"), 9.0, "3 * 3 = 9");
    
    freeInterpreter(interp);
}

void test_interpreteur_gosub_imbrique(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - GOSUB imbrique ===\n");
    
    interp = createInterpreter();
    
    /* GOSUB imbriques */
    addLine(interp, 10, "LET A = 1");
    addLine(interp, 20, "GOSUB 100");
    addLine(interp, 30, "LET D = 4");
    addLine(interp, 40, "END");
    addLine(interp, 100, "LET B = 2");
    addLine(interp, 110, "GOSUB 200");
    addLine(interp, 120, "RETURN");
    addLine(interp, 200, "LET C = 3");
    addLine(interp, 210, "RETURN");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 1.0, "A = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 2.0, "B = 2 (premiere sous-routine)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), 3.0, "C = 3 (sous-routine imbriquee)");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 4.0, "D = 4 (apres retours)");
    
    freeInterpreter(interp);
}

void test_interpreteur_gosub_boucle(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - GOSUB dans boucle ===\n");
    
    interp = createInterpreter();
    
    /* Appel de sous-routine dans une boucle FOR */
    addLine(interp, 10, "LET TOTAL = 0");
    addLine(interp, 20, "FOR I = 1 TO 3");
    addLine(interp, 30, "GOSUB 100");
    addLine(interp, 40, "NEXT I");
    addLine(interp, 50, "END");
    addLine(interp, 100, "LET TOTAL = TOTAL + I");
    addLine(interp, 110, "RETURN");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "TOTAL"), 6.0, "1 + 2 + 3 = 6");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_simple(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Chaines simples ===\n");
    
    interp = createInterpreter();
    
    /* Affectation de chaînes */
    addLine(interp, 10, "LET NOM$ = \"Alice\"");
    addLine(interp, 20, "LET PRENOM$ = \"Bob\"");
    addLine(interp, 30, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "NOM$"), "Alice", "NOM$ = Alice");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "PRENOM$"), "Bob", "PRENOM$ = Bob");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_copie(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Copie de chaines ===\n");
    
    interp = createInterpreter();
    
    /* Copie de variables chaînes */
    addLine(interp, 10, "LET A$ = \"Hello\"");
    addLine(interp, 20, "LET B$ = A$");
    addLine(interp, 30, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "A$"), "Hello", "A$ = Hello");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "Hello", "B$ copie de A$");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_len(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction LEN ===\n");
    
    interp = createInterpreter();
    
    /* Fonction LEN */
    addLine(interp, 10, "LET TEXT$ = \"Bonjour\"");
    addLine(interp, 20, "LET L1 = LEN(\"Hello\")");
    addLine(interp, 30, "LET L2 = LEN(TEXT$)");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "L1"), 5.0, "LEN(\"Hello\") = 5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "L2"), 7.0, "LEN(\"Bonjour\") = 7");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_asc(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction ASC ===\n");
    
    interp = createInterpreter();
    
    /* Fonction ASC - code ASCII */
    addLine(interp, 10, "LET A = ASC(\"A\")");
    addLine(interp, 20, "LET Z = ASC(\"Z\")");
    addLine(interp, 30, "LET TEXT$ = \"Hello\"");
    addLine(interp, 40, "LET H = ASC(TEXT$)");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 65.0, "ASC(\"A\") = 65");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Z"), 90.0, "ASC(\"Z\") = 90");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "H"), 72.0, "ASC(\"Hello\") = 72 (H)");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_chr(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction CHR ===\n");
    
    interp = createInterpreter();
    
    /* Fonction CHR - convertit code ASCII en caractère */
    addLine(interp, 10, "LET A$ = CHR$(65)");
    addLine(interp, 20, "LET Z$ = CHR$(90)");
    addLine(interp, 30, "LET SPC$ = CHR$(32)");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "A$"), "A", "CHR$(65) = A");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "Z$"), "Z", "CHR$(90) = Z");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "SPC$"), " ", "CHR$(32) = espace");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_mid(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction MID ===\n");
    
    interp = createInterpreter();
    
    /* Fonction MID - extraction de sous-chaîne */
    addLine(interp, 10, "LET TEXT$ = \"HelloWorld\"");
    addLine(interp, 20, "LET SUB1$ = MID(TEXT$, 1, 5)");
    addLine(interp, 30, "LET SUB2$ = MID(TEXT$, 6, 5)");
    addLine(interp, 40, "LET SUB3$ = MID(\"BASIC\", 3, 2)");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "SUB1$"), "Hello", "MID(\"HelloWorld\", 1, 5) = Hello");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "SUB2$"), "World", "MID(\"HelloWorld\", 6, 5) = World");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "SUB3$"), "SI", "MID(\"BASIC\", 3, 2) = SI");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_left(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction LEFT ===\n");
    
    interp = createInterpreter();
    
    /* Fonction LEFT - premiers caractères */
    addLine(interp, 10, "LET TEXT$ = \"Programming\"");
    addLine(interp, 20, "LET L1$ = LEFT(TEXT$, 4)");
    addLine(interp, 30, "LET L2$ = LEFT(\"BASIC\", 3)");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "L1$"), "Prog", "LEFT(\"Programming\", 4) = Prog");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "L2$"), "BAS", "LEFT(\"BASIC\", 3) = BAS");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_right(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction RIGHT ===\n");
    
    interp = createInterpreter();
    
    /* Fonction RIGHT - derniers caractères */
    addLine(interp, 10, "LET TEXT$ = \"Programming\"");
    addLine(interp, 20, "LET R1$ = RIGHT(TEXT$, 4)");
    addLine(interp, 30, "LET R2$ = RIGHT(\"BASIC\", 2)");
    addLine(interp, 40, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "R1$"), "ming", "RIGHT(\"Programming\", 4) = ming");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "R2$"), "IC", "RIGHT(\"BASIC\", 2) = IC");
    
    freeInterpreter(interp);
}

void test_interpreteur_chaines_concat(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Concaténation de chaines ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A$ = \"Hello\" + \" \" + \"World\"");
    addLine(interp, 20, "LET B$ = LEFT(\"PROG\", 2) + RIGHT(\"RAM\", 2)");
    addLine(interp, 30, "LET C$ = MID(\"BASIC\", 1, 2) + \"-\" + MID(\"BASIC\", 4, 2)");
    addLine(interp, 40, "LET D$ = A$ + \"!\"");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "A$"), "Hello World", "Concat lit+lit");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "PRAM", "Concat LEFT + RIGHT");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "C$"), "BA-IC", "Concat MID + lit");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "D$"), "Hello World!", "Concat variable + lit");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_atn(void) {
    Interpreter *interp;
    double result;
    
    printf("\n=== Tests de l'interpreteur - Fonction ATN ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A = ATN(1)");
    addLine(interp, 20, "LET B = ATN(0)");
    addLine(interp, 30, "LET C = ATN(-1)");
    addLine(interp, 40, "LET D = ATN(1.732)");
    addLine(interp, 50, "END");
    
    runProgram(interp);
    
    result = getVariable(interp, "A");
    ASSERT_DOUBLE_EQUAL(result, 0.785, "ATN(1) = pi/4 ≈ 0.785");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 0.0, "ATN(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), -0.785, "ATN(-1) = -pi/4");
    result = getVariable(interp, "D");
    ASSERT_DOUBLE_EQUAL(result, 1.047, "ATN(√3) ≈ pi/3");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_sgn(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction SGN ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A = SGN(42)");
    addLine(interp, 20, "LET B = SGN(0)");
    addLine(interp, 30, "LET C = SGN(-17)");
    addLine(interp, 40, "LET D = SGN(0.001)");
    addLine(interp, 50, "LET E = SGN(-0.001)");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 1.0, "SGN(42) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 0.0, "SGN(0) = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), -1.0, "SGN(-17) = -1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 1.0, "SGN(0.001) = 1");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), -1.0, "SGN(-0.001) = -1");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_str(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction STR$ ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A$ = STR$(42)");
    addLine(interp, 20, "LET B$ = STR$(3.14)");
    addLine(interp, 30, "LET C$ = STR$(-17)");
    addLine(interp, 40, "LET D$ = STR$(0)");
    addLine(interp, 50, "LET X = 123");
    addLine(interp, 60, "LET E$ = STR$(X)");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "A$"), "42", "STR$(42) = '42'");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "3.14", "STR$(3.14) = '3.14'");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "C$"), "-17", "STR$(-17) = '-17'");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "D$"), "0", "STR$(0) = '0'");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "E$"), "123", "STR$(variable) = '123'");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_val(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction VAL ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A = VAL(\"42\")");
    addLine(interp, 20, "LET B = VAL(\"3.14\")");
    addLine(interp, 30, "LET C = VAL(\"-17\")");
    addLine(interp, 40, "LET D = VAL(\"0\")");
    addLine(interp, 50, "LET S$ = \"123\"");
    addLine(interp, 60, "LET E = VAL(S$)");
    addLine(interp, 70, "LET F = VAL(\"12.5\") * 2");
    addLine(interp, 80, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "A"), 42.0, "VAL('42') = 42");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "B"), 3.14, "VAL('3.14') = 3.14");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "C"), -17.0, "VAL('-17') = -17");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "D"), 0.0, "VAL('0') = 0");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "E"), 123.0, "VAL(variable) = 123");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "F"), 25.0, "VAL('12.5') * 2 = 25");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_space(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction SPACE$ ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A$ = SPACE$(0)");
    addLine(interp, 20, "LET B$ = SPACE$(1)");
    addLine(interp, 30, "LET C$ = SPACE$(5)");
    addLine(interp, 40, "LET D$ = \"[\" + SPACE$(3) + \"]\"");
    addLine(interp, 50, "LET N = 4");
    addLine(interp, 60, "LET E$ = SPACE$(N)");
    addLine(interp, 70, "END");
    
    runProgram(interp);
    
    ASSERT_EQUAL(strlen(getStringVariable(interp, "A$")), 0, "SPACE$(0) longueur = 0");
    ASSERT_EQUAL(strlen(getStringVariable(interp, "B$")), 1, "SPACE$(1) longueur = 1");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), " ", "SPACE$(1) = ' '");
    ASSERT_EQUAL(strlen(getStringVariable(interp, "C$")), 5, "SPACE$(5) longueur = 5");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "D$"), "[   ]", "Concat avec SPACE$");
    ASSERT_EQUAL(strlen(getStringVariable(interp, "E$")), 4, "SPACE$(variable) longueur = 4");
    
    freeInterpreter(interp);
}

void test_interpreteur_fonctions_string(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - Fonction STRING$ ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET A$ = STRING$(5, 65)");
    addLine(interp, 20, "LET B$ = STRING$(3, \"X\")");
    addLine(interp, 30, "LET C$ = STRING$(0, \"A\")");
    addLine(interp, 40, "LET D$ = STRING$(4, 42)");
    addLine(interp, 50, "LET N = 6");
    addLine(interp, 60, "LET E$ = STRING$(N, \"-\")");
    addLine(interp, 70, "LET F$ = \"[\" + STRING$(10, \"=\") + \"]\"");
    addLine(interp, 80, "END");
    
    runProgram(interp);
    
    ASSERT_STRING_EQUAL(getStringVariable(interp, "A$"), "AAAAA", "STRING$(5, 65) = 'AAAAA'");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "B$"), "XXX", "STRING$(3, 'X') = 'XXX'");
    ASSERT_EQUAL(strlen(getStringVariable(interp, "C$")), 0, "STRING$(0, 'A') longueur = 0");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "D$"), "****", "STRING$(4, 42) = '****'");
    ASSERT_EQUAL(strlen(getStringVariable(interp, "E$")), 6, "STRING$(variable, char) longueur = 6");
    ASSERT_STRING_EQUAL(getStringVariable(interp, "F$"), "[==========]", "Concat avec STRING$");
    
    freeInterpreter(interp);
}

void test_interpreteur_str_val_round_trip(void) {
    Interpreter *interp;
    
    printf("\n=== Tests de l'interpreteur - STR$/VAL round-trip ===\n");
    
    interp = createInterpreter();
    
    addLine(interp, 10, "LET X = 42.5");
    addLine(interp, 20, "LET S$ = STR$(X)");
    addLine(interp, 30, "LET Y = VAL(S$)");
    addLine(interp, 40, "LET EGAL = 0");
    addLine(interp, 50, "IF X = Y THEN LET EGAL = 1");
    addLine(interp, 60, "END");
    
    runProgram(interp);
    
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "X"), 42.5, "X = 42.5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "Y"), 42.5, "Y = VAL(STR(X)) = 42.5");
    ASSERT_DOUBLE_EQUAL(getVariable(interp, "EGAL"), 1.0, "X = Y après round-trip");
    
    freeInterpreter(interp);
}

/* Fonction principale des tests */
int main(void) {
    setvbuf(stdout, NULL, _IONBF, 0);
    printf("========================================\n");
    printf("   TESTS UNITAIRES INTERPRETEUR BASIC\n");
    printf("========================================\n");
    
    /* Tests du lexer */
    test_lexer_nombres();
    test_lexer_identifiants();
    test_lexer_mots_cles();
    test_lexer_operateurs();
    test_lexer_chaines();
    test_lexer_erreurs();
    test_lexer_nouveaux_mots_cles();
    test_lexer_expression_complete();
    
    /* Tests de l'interpreteur */
    test_interpreteur_variables();
    test_interpreteur_lignes();
    test_interpreteur_let();
    test_interpreteur_expressions();
    test_interpreteur_input_multi();
    test_interpreteur_input_multi_programme();
    test_interpreteur_programme();
    test_interpreteur_goto();
    
    /* Tests des conditions IF...THEN...ELSE */
    test_interpreteur_if_then_simple();
    test_interpreteur_if_then_else();
    test_interpreteur_if_operateurs();
    test_interpreteur_if_then_goto();
    test_interpreteur_if_else_goto();
    test_interpreteur_if_logique();
    
    /* Tests des tableaux DIM */
    test_interpreteur_dim_simple();
    test_interpreteur_dim_boucle();
    test_interpreteur_dim_lecture();
    test_interpreteur_dim_calcul_somme();
    test_interpreteur_dim_indice_variable();
    
    /* Tests des tableaux multi-dimensionnels */
    test_interpreteur_dim_2d_simple();
    test_interpreteur_dim_2d_boucles();
    test_interpreteur_dim_2d_identite();
    test_interpreteur_dim_2d_expressions();
    test_interpreteur_dim_3d_simple();
    test_interpreteur_dim_3d_boucles();
    
    /* Tests des fonctions mathématiques */
    test_interpreteur_fonctions_trigo();
    test_interpreteur_fonctions_hyperboliques();
    test_interpreteur_fonctions_angle_conv();
    test_interpreteur_fonctions_math();
    test_interpreteur_fonctions_composees();
    test_interpreteur_fonctions_avancees();
    
    /* Tests de GOSUB/RETURN */
    test_interpreteur_gosub_simple();
    test_interpreteur_gosub_calcul();
    test_interpreteur_gosub_imbrique();
    test_interpreteur_gosub_boucle();
    
    /* Tests des chaînes */
    test_interpreteur_chaines_simple();
    test_interpreteur_chaines_copie();
    test_interpreteur_chaines_len();
    test_interpreteur_chaines_asc();
    test_interpreteur_chaines_chr();
    test_interpreteur_chaines_mid();
    test_interpreteur_chaines_left();
    test_interpreteur_chaines_right();
    test_interpreteur_chaines_concat();
    
    /* Tests des nouvelles fonctions mathématiques */
    test_interpreteur_fonctions_atn();
    test_interpreteur_fonctions_sgn();
    
    /* Tests des nouvelles fonctions de chaînes */
    test_interpreteur_fonctions_str();
    test_interpreteur_fonctions_val();
    test_interpreteur_fonctions_space();
    test_interpreteur_fonctions_string();
    test_interpreteur_str_val_round_trip();
    
    /* Tests des boucles FOR */
    test_interpreteur_for_simple();
    test_interpreteur_for_step_positif();
    test_interpreteur_for_step_negatif();
    test_interpreteur_for_limites();
    test_interpreteur_for_imbriquees();
    test_interpreteur_for_avec_calculs();
    test_interpreteur_for_variables_limites();
    
    /* Tests SAVE/LOAD */
    test_interpreteur_save_load();
    test_interpreteur_save_fichier_invalide();
    
    /* Resume */
    printf("\n========================================\n");
    printf("            RESUME DES TESTS\n");
    printf("========================================\n");
    printf("Total:    %d tests\n", testsTotal);
    printf("Reussis:  %d tests\n", testsReussis);
    printf("Echoues:  %d tests\n", testsEchoues);
    
    if (testsEchoues == 0) {
        printf("\n[SUCCES] Tous les tests sont passes!\n");
        return 0;
    } else {
        printf("\n[ECHEC] %d test(s) ont echoue.\n", testsEchoues);
        return 1;
    }
}
