10 REM Exemple de tableaux avec DIM
20 PRINT "=== Test des tableaux ==="
30 DIM CARRES(5)
40 PRINT "Remplissage du tableau..."
50 FOR I = 0 TO 5
60 LET CARRES(I) = I * I
70 NEXT I
80 PRINT "Affichage des carres:"
90 FOR I = 0 TO 5
100 PRINT I, "au carre =", CARRES(I)
110 NEXT I
120 PRINT "=== Calcul de la somme ==="
130 LET SOMME = 0
140 FOR I = 0 TO 5
150 LET SOMME = SOMME + CARRES(I)
160 NEXT I
170 PRINT "Somme des carres de 0 a 5 =", SOMME
180 END
