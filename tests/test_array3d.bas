10 REM Test des tableaux 3D - Cube 2x2x2
20 DIM CUBE(2, 2, 2)
30 REM Remplir le cube
40 LET CUBE(0, 0, 0) = 1
50 LET CUBE(0, 0, 1) = 2
60 LET CUBE(0, 1, 0) = 3
70 LET CUBE(0, 1, 1) = 4
80 LET CUBE(1, 0, 0) = 5
90 LET CUBE(1, 0, 1) = 6
100 LET CUBE(1, 1, 0) = 7
110 LET CUBE(1, 1, 1) = 8
120 REM Afficher le cube
130 PRINT "Cube 2x2x2:"
140 PRINT "Niveau 0:"
150 PRINT CUBE(0, 0, 0), CUBE(0, 0, 1)
160 PRINT CUBE(0, 1, 0), CUBE(0, 1, 1)
170 PRINT "Niveau 1:"
180 PRINT CUBE(1, 0, 0), CUBE(1, 0, 1)
190 PRINT CUBE(1, 1, 0), CUBE(1, 1, 1)
200 END
