10 REM Calcul de la suite de Fibonacci
20 PRINT "Suite de Fibonacci jusqu'a N"
30 PRINT "Entrez N:"
40 INPUT N
50 LET A = 0
60 LET B = 1
70 PRINT "F(0) =", A
80 PRINT "F(1) =", B
90 FOR I = 2 TO N
100 LET C = A + B
110 PRINT "F(", I, ") =", C
120 LET A = B
130 LET B = C
140 NEXT I
150 END
