#include <stdio.h>
#include "utils.h"

void test_interpreteur_dim_simple(void);
void test_interpreteur_dim_boucle(void);
void test_interpreteur_dim_lecture(void);
void test_interpreteur_dim_calcul_somme(void);
void test_interpreteur_dim_indice_variable(void);

int main(void) {
    /* Disable stdout buffering to capture output before a crash. */
    setvbuf(stdout, NULL, _IONBF, 0);

    test_interpreteur_dim_simple();
    test_interpreteur_dim_boucle();
    test_interpreteur_dim_lecture();
    test_interpreteur_dim_calcul_somme();
    test_interpreteur_dim_indice_variable();

    printf("\n--- Repro directe ---\n");
    Interpreter *interp = createInterpreter();
    addLine(interp, 10, "DIM C(3)");
    addLine(interp, 20, "LET C(0) = 5");
    addLine(interp, 30, "LET C(1) = 10");
    addLine(interp, 40, "LET C(2) = 15");
    addLine(interp, 50, "LET SOMME = C(0) + C(1) + C(2)");
    addLine(interp, 60, "END");
    runProgram(interp);
    printf("C(0)=%g C(1)=%g C(2)=%g SOMME=%g\n",
           getArrayElement(interp, "C", 0),
           getArrayElement(interp, "C", 1),
           getArrayElement(interp, "C", 2),
           getVariable(interp, "SOMME"));
    freeInterpreter(interp);

    return 0;
}
