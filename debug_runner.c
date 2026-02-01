#include <stdio.h>
#include "interpreter.h"

int main(void) {
    /* Disable stdout buffering to capture output before a crash. */
    setvbuf(stdout, NULL, _IONBF, 0);

    printf("\n--- Test DIM et accès tableau ---\n");
    Interpreter *interp = createInterpreter();
    int idx[1];
    addLine(interp, 10, "DIM C(3)");
    addLine(interp, 20, "LET C(0) = 5");
    addLine(interp, 30, "LET C(1) = 10");
    addLine(interp, 40, "LET C(2) = 15");
    addLine(interp, 50, "LET SOMME = C(0) + C(1) + C(2)");
    addLine(interp, 60, "END");
    runProgram(interp);
    idx[0] = 0;
    printf("C(0)=%g ", getArrayElement(interp, "C", idx, 1));
    idx[0] = 1;
    printf("C(1)=%g ", getArrayElement(interp, "C", idx, 1));
    idx[0] = 2;
    printf("C(2)=%g SOMME=%g\n", getArrayElement(interp, "C", idx, 1),
           getVariable(interp, "SOMME"));
    freeInterpreter(interp);

    return 0;
}
