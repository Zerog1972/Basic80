10 REM Test des tableaux 2D avec boucles FOR
20 DIM MAT(3, 3)
30 REM Remplir avec boucles
40 FOR I = 0 TO 2
50 FOR J = 0 TO 2
60 LET MAT(I, J) = I * 3 + J + 1
70 NEXT J
80 NEXT I
90 REM Afficher
100 PRINT "Matrice remplie avec boucles:"
110 FOR I = 0 TO 2
120 FOR J = 0 TO 2
130 PRINT MAT(I, J),
140 NEXT J
150 PRINT
160 NEXT I
170 END
