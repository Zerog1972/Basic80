10 REM Test des boucles FOR
20 PRINT "=== Test 1: Boucle FOR simple ==="
30 FOR I = 1 TO 5
40 PRINT "I =", I
50 NEXT I
60 PRINT ""
70 PRINT "=== Test 2: Boucle FOR avec STEP ==="
80 FOR J = 0 TO 10 STEP 2
90 PRINT "J =", J
100 NEXT J
110 PRINT ""
120 PRINT "=== Test 3: Boucle FOR descendante ==="
130 FOR K = 10 TO 1 STEP -1
140 PRINT "K =", K
150 NEXT K
160 PRINT ""
170 PRINT "=== Test 4: Calcul dans la boucle ==="
180 LET TOTAL = 0
190 FOR N = 1 TO 10
200 LET TOTAL = TOTAL + N
210 NEXT N
220 PRINT "Somme de 1 a 10 =", TOTAL
230 PRINT ""
240 PRINT "=== Test 5: Boucles imbriquees ==="
250 FOR X = 1 TO 3
260 FOR Y = 1 TO 3
270 PRINT "X =", X, "Y =", Y
280 NEXT Y
290 NEXT X
300 END
