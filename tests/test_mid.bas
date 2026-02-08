10 REM Test de la fonction MID$
20 A$ = "BONJOUR"
30 PRINT "Chaine complète: "; A$
40 PRINT "MID$(A$, 1, 3) = "; MID$(A$, 1, 3)
50 PRINT "MID$(A$, 4, 2) = "; MID$(A$, 4, 2)
60 PRINT "MID$(A$, 2, 5) = "; MID$(A$, 2, 5)
70 PRINT "MID$(\"Hello World\", 7, 5) = "; MID$("Hello World", 7, 5)
80 END
