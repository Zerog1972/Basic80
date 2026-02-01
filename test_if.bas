10 REM Test des conditions IF...THEN...ELSE
20 PRINT "=== Test 1: IF...THEN simple ==="
30 LET X = 10
40 IF X > 5 THEN PRINT "X est superieur a 5"
50 PRINT ""
60 PRINT "=== Test 2: IF...THEN...ELSE ==="
70 LET Y = 3
80 IF Y > 5 THEN PRINT "Y > 5" ELSE PRINT "Y <= 5"
90 PRINT ""
100 PRINT "=== Test 3: Operateurs de comparaison ==="
110 LET A = 10
120 LET B = 10
130 IF A = B THEN PRINT "A egal B"
140 IF A <> 5 THEN PRINT "A different de 5"
150 IF A >= 10 THEN PRINT "A >= 10"
160 PRINT ""
170 PRINT "=== Test 4: IF avec calculs ==="
180 LET SCORE = 75
190 IF SCORE >= 60 THEN PRINT "Admis" ELSE PRINT "Refuse"
200 PRINT ""
210 PRINT "=== Test 5: IF...THEN GOTO ==="
220 LET N = 15
230 IF N > 10 THEN GOTO 260
240 PRINT "N est petit"
250 GOTO 270
260 PRINT "N est grand"
270 PRINT ""
280 PRINT "=== Test 6: Boucle avec IF ==="
290 FOR I = 1 TO 10
300 IF I = 5 THEN PRINT "Milieu!" ELSE PRINT "I =", I
310 NEXT I
320 END
