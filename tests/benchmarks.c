#include "interp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BENCH_ITERATIONS 5

typedef void (*BenchmarkBuilder)(Interpreter *interp);

typedef struct {
    const char *name;
    const char *description;
    BenchmarkBuilder builder;
} BenchmarkCase;

static double elapsedMilliseconds(clock_t start, clock_t end) {
    return ((double)(end - start) * 1000.0) / (double)CLOCKS_PER_SEC;
}

static void addVariableProgram(Interpreter *interp) {
    int line = 10;
    int index;

    addLine(interp, line, "LET TOTAL = 0");
    line += 10;

    for (index = 0; index < 120; index++) {
        char buffer[128];
        sprintf(buffer, "LET V%d = %d", index, index + 1);
        addLine(interp, line, buffer);
        line += 10;
    }

    addLine(interp, line, "FOR I = 1 TO 400");
    line += 10;

    for (index = 0; index < 120; index++) {
        char buffer[128];
        sprintf(buffer, "LET TOTAL = TOTAL + V%d", index);
        addLine(interp, line, buffer);
        line += 10;
    }

    addLine(interp, line, "NEXT I");
    line += 10;
    addLine(interp, line, "END");
}

static void addArrayProgram(Interpreter *interp) {
    addLine(interp, 10, "DIM GRID(90, 90)");
    addLine(interp, 20, "LET TOTAL = 0");
    addLine(interp, 30, "FOR PASS = 1 TO 12");
    addLine(interp, 40, "FOR I = 0 TO 89");
    addLine(interp, 50, "FOR J = 0 TO 89");
    addLine(interp, 60, "LET GRID(I, J) = I * J + PASS + I + J");
    addLine(interp, 70, "NEXT J");
    addLine(interp, 80, "NEXT I");
    addLine(interp, 90, "FOR I = 0 TO 89");
    addLine(interp, 100, "FOR J = 0 TO 89");
    addLine(interp, 110, "LET TOTAL = TOTAL + GRID(I, J)");
    addLine(interp, 120, "NEXT J");
    addLine(interp, 130, "NEXT I");
    addLine(interp, 140, "NEXT PASS");
    addLine(interp, 150, "END");
}

static void addControlFlowProgram(Interpreter *interp) {
    addLine(interp, 10, "LET X = 0");
    addLine(interp, 20, "LET Y = 0");
    addLine(interp, 30, "FOR I = 1 TO 800");
    addLine(interp, 40, "GOSUB 200");
    addLine(interp, 50, "IF X < 400 THEN GOTO 90");
    addLine(interp, 60, "LET Y = Y + X");
    addLine(interp, 70, "GOTO 100");
    addLine(interp, 90, "LET Y = Y + 1");
    addLine(interp, 100, "NEXT I");
    addLine(interp, 110, "END");
    addLine(interp, 200, "LET X = X + 1");
    addLine(interp, 210, "RETURN");
}

static int runBenchmarkCase(const BenchmarkCase *bench, int iterations) {
    int iteration;
    double totalMs;
    double minMs;
    double maxMs;

    totalMs = 0.0;
    minMs = 0.0;
    maxMs = 0.0;

    printf("\n=== %s ===\n", bench->name);
    printf("%s\n", bench->description);

    for (iteration = 0; iteration < iterations; iteration++) {
        Interpreter *interp;
        clock_t start;
        clock_t end;
        double currentMs;

        interp = createInterpreter();
        if (!interp) {
            fprintf(stderr, "Erreur: creation interpreteur impossible pour %s\n", bench->name);
            return 0;
        }

        bench->builder(interp);
        start = clock();
        runProgram(interp);
        end = clock();

        if (interp->hasError) {
            fprintf(stderr, "Erreur pendant le benchmark %s (iteration %d)\n", bench->name, iteration + 1);
            freeInterpreter(interp);
            return 0;
        }

        currentMs = elapsedMilliseconds(start, end);
        if (iteration == 0 || currentMs < minMs) minMs = currentMs;
        if (iteration == 0 || currentMs > maxMs) maxMs = currentMs;
        totalMs += currentMs;

        printf("Iteration %d: %.3f ms\n", iteration + 1, currentMs);
        freeInterpreter(interp);
    }

    printf("Moyenne : %.3f ms\n", totalMs / (double)iterations);
    printf("Min     : %.3f ms\n", minMs);
    printf("Max     : %.3f ms\n", maxMs);
    return 1;
}

int main(int argc, char **argv) {
    BenchmarkCase benchmarks[] = {
        {
            "variables",
            "Mesure les acces repetes a de nombreuses variables scalaires.",
            addVariableProgram
        },
        {
            "arrays",
            "Mesure intensivement les acces en lecture/ecriture sur un tableau 2D.",
            addArrayProgram
        },
        {
            "controlflow",
            "Mesure FOR/NEXT, GOSUB/RETURN et les sauts conditionnels.",
            addControlFlowProgram
        }
    };
    int benchmarkCount;
    int index;
    int iterations;

    benchmarkCount = (int)(sizeof(benchmarks) / sizeof(benchmarks[0]));
    iterations = BENCH_ITERATIONS;

    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) {
            fprintf(stderr, "Usage: %s [iterations]\n", argv[0]);
            return 1;
        }
    }

    printf("Basic80 benchmarks\n");
    printf("Iterations par scenario: %d\n", iterations);

    for (index = 0; index < benchmarkCount; index++) {
        if (!runBenchmarkCase(&benchmarks[index], iterations)) {
            return 1;
        }
    }

    return 0;
}