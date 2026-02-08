10 REM Test du nouveau système de gestion d'erreurs avec contexte
20 PRINT "Test 1: Erreur de syntaxe"
30 LET = 10
40 PRINT "Test 2: Division par zéro"
50 LET A = 10 / 0
60 PRINT "Test 3: Type mismatch"
70 LET B$ = "Bonjour"
80 LET C = B$
90 PRINT "Test 4: Out of DATA"
100 READ X
110 END
