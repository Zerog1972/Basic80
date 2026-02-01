# Basic80

Un interpréteur simple pour le langage BASIC classique.

## Fonctionnalités

### Commandes BASIC supportées:
- **PRINT** - Affiche du texte et des variables
- **LET** - Assigne une valeur à une variable
- **DIM** - Déclare un tableau
- **INPUT** - Lit une valeur depuis l'utilisateur
- **GOTO** - Saute à une ligne numérotée
- **GOSUB** - Appelle une sous-routine
- **RETURN** - Retourne de la sous-routine
- **IF...THEN...ELSE** - Exécution conditionnelle
- **FOR...TO...STEP...NEXT** - Boucles avec compteur
- **REM** - Commentaires
- **END** - Termine le programme

### Opérateurs mathématiques:
- `+` Addition
- `-` Soustraction
- `*` Multiplication
- `/` Division
- `()` Parenthèses pour la priorité

### Fonctions mathématiques:
- **SIN(x)** - Sinus (x en radians)
- **COS(x)** - Cosinus (x en radians)
- **TAN(x)** - Tangente (x en radians)
- **ATAN(x)** - Arc Tangente (retourne radians)
- **ASIN(x)** - Arc Sinus (retourne radians)
- **ACOS(x)** - Arc Cosinus (retourne radians)
- **SINH(x)** - Sinus Hyperbolique
- **COSH(x)** - Cosinus Hyperbolique
- **TANH(x)** - Tangente Hyperbolique
- **RAD(x)** - Convertit degres en radians
- **DEG(x)** - Convertit radians en degres
- **SQR(x)** - Racine carrée
- **ABS(x)** - Valeur absolue
- **INT(x)** - Partie entière (arrondi vers le bas)
- **RND(x)** - Nombre aléatoire entre 0 et x-1
- **RND** - Nombre aléatoire entre 0.0 et 1.0
- **LOG(x)** - Logarithme naturel (ln)
- **EXP(x)** - Exponentielle (e^x)
- **POW(x, y)** - Puissance (x^y)
- **LOG10(x)** - Logarithme base 10

### Fonctions de chaînes:
- **LEN(s)** - Longueur d'une chaîne
- **ASC(s)** - Code ASCII du premier caractère
- **MID(s, start, len)** - Sous-chaîne depuis `start` (1-indexé) sur `len` caractères
- **LEFT(s, n)** - `n` premiers caractères
- **RIGHT(s, n)** - `n` derniers caractères
- **CHR(n)** - Caractère correspondant au code ASCII `n`
- Concaténation avec `+` entre expressions de chaînes

### Opérateurs de comparaison (déclarés):
- `<` Inférieur à
- `>` Supérieur à
- `<=` Inférieur ou égal
- `>=` Supérieur ou égal
- `<>` Différent

## Utilisation

### Compilation
```bash
clang -std=c89 -pedantic -Wall -g -Iinclude src/*.c -o basic80.exe
```

Ou utilisez la tâche VS Code: **C/C++: clang.exe build all files**

### Tests unitaires
Compiler et exécuter les tests :
```bash
clang -std=c89 -pedantic -Wall -g -Iinclude tests/tests.c src/interpreter.c src/lexer.c src/variables.c src/expression.c src/control_flow.c src/commands.c -o tests.exe
.\tests.exe
```

Les tests unitaires incluent :
- **Tests du lexer** : nombres, identifiants, mots-clés, opérateurs, chaînes
- **Tests de l'interpréteur** : variables, lignes, commandes LET, expressions
- **Tests des conditions IF...THEN...ELSE** :
  - IF...THEN simple (condition vraie/fausse)
  - IF...THEN...ELSE complet
  - Tests de tous les opérateurs de comparaison (<, >, <=, >=, =, <>)
  - IF...THEN GOTO
  - IF...THEN...ELSE avec GOTO
- **Tests des tableaux DIM** :
  - Déclaration et affectation d'éléments simples (1D)
  - Remplissage de tableaux avec boucle FOR
  - Lecture d'éléments de tableau dans expressions
  - Calculs avec tableaux (somme des éléments)
  - Utilisation de variables comme indices
- **Tests des tableaux multi-dimensionnels** :
  - Tableaux 2D simples (matrices)
  - Tableaux 2D avec boucles FOR imbriquées
  - Matrices identité (avec conditions IF)
  - Expressions dans les indices de tableaux 2D
  - Tableaux 3D simples (cubes)
  - Tableaux 3D avec boucles FOR triples
- **Tests des fonctions mathématiques** :
  - Fonctions trigonométriques (SIN, COS, TAN)
  - Fonctions mathématiques (SQR, ABS, INT)
  - Fonctions composées et expressions complexes
- **Tests de GOSUB/RETURN** :
  - GOSUB simple avec sous-routine
  - GOSUB avec calculs (réutilisation de sous-routine)
  - GOSUB imbriqués (sous-routines appelées depuis sous-routines)
  - GOSUB dans boucle FOR
- **Tests des chaînes de caractères** :
  - Affectation de chaînes littérales
  - Copie de chaînes entre variables
  - Fonctions LEN(), ASC(), CHR(), MID(), LEFT(), RIGHT()
  - Concaténation de chaînes avec l'opérateur +
- **Tests des boucles FOR** :
  - Boucle FOR simple (1 TO 5)
  - Boucle FOR avec STEP positif (0 TO 10 STEP 2)
  - Boucle FOR descendante avec STEP négatif (10 TO 1 STEP -1)
  - Boucles FOR avec limites spéciales (debut = fin, debut > fin)
  - Boucles FOR imbriquées
  - Boucles FOR avec calculs (factorielle, somme des carrés)
  - Boucles FOR avec variables pour limites

Total : **191 tests unitaires**

Le code est strictement conforme au standard **C89/ANSI C**.

### Exécution
```bash
./basic80.exe
```

### Mode interactif

**Commandes du shell:**
- `LIST` - Affiche le programme en mémoire
- `RUN` - Exécute le programme
- `CLEAR` - Efface le programme
- `EXIT` - Quitte l'interpréteur

**Lignes numérotées:**
Ajoutent des instructions au programme (ex: `10 PRINT "Hello"`)

**Commandes directes:**
S'exécutent immédiatement (ex: `PRINT "Bonjour"`)

## Exemples

### Exemple 1: Programme simple
```basic
10 PRINT "Hello, World!"
20 LET X = 42
30 PRINT "La réponse est:", X
40 END
```

### Exemple 2: Calculs
```basic
10 LET A = 10
20 LET B = 5
30 LET C = A + B * 2
40 PRINT "Résultat:", C
50 END
```

### Exemple 3: Avec INPUT
```basic
10 PRINT "Entrez votre age:"
20 INPUT AGE
30 LET ANNEES = 100 - AGE
40 PRINT "Il vous reste environ", ANNEES, "annees"
50 END
```

### Exemple 4: Boucle FOR simple
```basic
10 FOR I = 1 TO 5
20 PRINT "I =", I
30 NEXT I
40 END
```

### Exemple 5: Boucle FOR avec STEP
```basic
10 FOR I = 0 TO 10 STEP 2
20 PRINT "I =", I
30 NEXT I
40 END
```

### Exemple 6: Boucle FOR descendante
```basic
10 FOR I = 10 TO 1 STEP -1
20 PRINT "Compte a rebours:", I
30 NEXT I
40 PRINT "Decollage!"
50 END
```

### Exemple 7: Calcul avec boucle FOR
```basic
10 LET TOTAL = 0
20 FOR I = 1 TO 10
30 LET TOTAL = TOTAL + I
40 NEXT I
50 PRINT "Somme de 1 a 10 =", TOTAL
60 END
```

### Exemple 8: Condition IF...THEN
```basic
10 INPUT X
20 IF X > 0 THEN PRINT "Nombre positif"
30 END
```

### Exemple 9: Condition IF...THEN...ELSE
```basic
10 PRINT "Entrez votre age:"
20 INPUT AGE
30 IF AGE >= 18 THEN PRINT "Majeur" ELSE PRINT "Mineur"
40 END
```

### Exemple 10: IF avec GOTO
```basic
10 LET SCORE = 75
20 IF SCORE >= 60 THEN GOTO 50
30 PRINT "Echec"
40 GOTO 60
50 PRINT "Reussite"
60 END
```

### Exemple 11: Tableaux simples
```basic
10 DIM A(5)
20 LET A(0) = 10
30 LET A(1) = 20
40 LET A(2) = 30
50 PRINT "Premier element:", A(0)
60 PRINT "Deuxieme element:", A(1)
70 END
```

### Exemple 12: Tableaux avec boucle
```basic
10 DIM NOMBRES(10)
20 FOR I = 0 TO 10
30 LET NOMBRES(I) = I * I
40 NEXT I
50 PRINT "Carres de 0 a 10:"
60 FOR I = 0 TO 10
70 PRINT I, "au carre =", NOMBRES(I)
80 NEXT I
90 END
```

### Exemple 13: Somme des éléments d'un tableau
```basic
10 DIM VALEURS(5)
20 LET VALEURS(0) = 10
30 LET VALEURS(1) = 20
40 LET VALEURS(2) = 30
50 LET VALEURS(3) = 40
60 LET VALEURS(4) = 50
70 LET TOTAL = 0
80 FOR I = 0 TO 4
90 LET TOTAL = TOTAL + VALEURS(I)
100 NEXT I
110 PRINT "Somme totale:", TOTAL
120 END
```

### Exemple 14: Tableaux multi-dimensionnels - Matrice 3x3
```basic
10 REM Matrice 3x3
20 DIM M(3, 3)
30 REM Remplir la matrice avec boucles
40 FOR I = 0 TO 2
50 FOR J = 0 TO 2
60 LET M(I, J) = I * 3 + J + 1
70 NEXT J
80 NEXT I
90 REM Afficher quelques elements
100 PRINT "M(0,0) =", M(0, 0)
110 PRINT "M(1,1) =", M(1, 1)
120 PRINT "M(2,2) =", M(2, 2)
130 END
```

### Exemple 15: Matrice identité
```basic
10 REM Creer une matrice identite 3x3
20 DIM ID(3, 3)
30 FOR I = 0 TO 2
40 FOR J = 0 TO 2
50 IF I = J THEN LET ID(I, J) = 1
60 IF I <> J THEN LET ID(I, J) = 0
70 NEXT J
80 NEXT I
90 PRINT "Diagonale:"
100 PRINT ID(0, 0), ID(1, 1), ID(2, 2)
110 END
```

### Exemple 16: Tableau 3D (cube)
```basic
10 REM Tableau 3D - Cube 2x2x2
20 DIM CUBE(2, 2, 2)
30 FOR I = 0 TO 1
40 FOR J = 0 TO 1
50 FOR K = 0 TO 1
60 LET CUBE(I, J, K) = I * 4 + J * 2 + K + 1
70 NEXT K
80 NEXT J
90 NEXT I
100 PRINT "CUBE(0,0,0) =", CUBE(0, 0, 0)
110 PRINT "CUBE(1,1,1) =", CUBE(1, 1, 1)
120 END
```

### Exemple 17: Fonctions mathématiques
```basic
10 LET PI = 3.14159
20 LET A = SIN(PI)
30 LET B = COS(0)
40 LET C = SQR(16)
50 LET D = ABS(-5)
60 LET E = INT(3.7)
70 PRINT "SIN(PI) =", A
80 PRINT "COS(0) =", B
90 PRINT "SQR(16) =", C
100 PRINT "ABS(-5) =", D
110 PRINT "INT(3.7) =", E
120 END
```

### Exemple 18: Cercle trigonométrique
```basic
10 LET PI = 3.14159
20 FOR ANGLE = 0 TO 360 STEP 45
30 LET RAD = ANGLE * PI / 180
40 LET X = COS(RAD)
50 LET Y = SIN(RAD)
60 PRINT "Angle", ANGLE, "-> X=", X, "Y=", Y
70 NEXT ANGLE
80 END
```

### Exemple 19: Sous-routine simple
```basic
10 PRINT "Programme principal"
20 GOSUB 100
30 PRINT "Retour au principal"
40 END
100 REM Sous-routine
110 PRINT "Dans la sous-routine"
120 RETURN
```

### Exemple 20: Sous-routine de calcul
```basic
10 LET N = 5
20 GOSUB 100
30 PRINT "Factorielle de 5 =", FACT
40 END
100 REM Calcul de factorielle
110 LET FACT = 1
120 FOR I = 1 TO N
130 LET FACT = FACT * I
140 NEXT I
150 RETURN
```

### Exemple 21: Chaînes de caractères
```basic
10 LET NOM = "Alice"
20 LET PRENOM = "Bob"
30 PRINT "Nom:", NOM
40 PRINT "Prenom:", PRENOM
50 LET L = LEN(NOM)
60 PRINT "Longueur du nom:", L
70 LET C = ASC("A")
80 PRINT "Code ASCII de A:", C
90 END
```

### Exemple 22: Copie de chaînes
```basic
10 LET MESSAGE = "Hello World"
20 LET COPIE = MESSAGE
30 PRINT "Original:", MESSAGE
40 PRINT "Copie:", COPIE
50 PRINT "Longueur:", LEN(COPIE)
60 END
```

### Exemple 23: Avec GOTO
```basic
10 LET X = 1
20 PRINT "Compte:", X
30 LET X = X + 1
40 IF X < 5 THEN GOTO 20
50 PRINT "Fin!"
60 END
```

### Exemple 24: Concaténation de chaînes
```basic
10 LET A = "Hello" + " " + "World"
20 LET B = LEFT("PROG", 2) + RIGHT("RAM", 2)
30 PRINT "Phrase:", A
40 PRINT "Fusion:", B
50 END
```

### Exemple 25: Fonctions mathématiques avancées
```basic
10 LET E = EXP(1)
20 LET L = LOG(E)
30 LET P = POW(2, 8)
40 LET L10 = LOG10(1000)
50 PRINT "e =", E
60 PRINT "ln(e) =", L
70 PRINT "2^8 =", P
80 PRINT "log10(1000) =", L10
90 END
```

### Exemple 26: Fonctions hyperboliques
```basic
10 LET S = SINH(0)
20 LET C = COSH(0)
30 LET T = TANH(0)
40 LET A = ATAN(1)
50 PRINT "SINH(0) =", S
60 PRINT "COSH(0) =", C
70 PRINT "TANH(0) =", T
80 PRINT "ATAN(1) (approx PI/4) =", A
90 END
```

### Exemple 27: Utilisation des degres et radians
```basic
10 LET ANGLE = 90
20 REM Convertir degres en radians pour SIN
30 LET S = SIN(RAD(ANGLE))
40 PRINT "SIN(90 degres) =", S
50 REM Convertir resultat inverse en degres
60 LET A = ASIN(1)
70 LET D = DEG(A)
80 PRINT "ASIN(1) en degres =", D
90 END
```

## Architecture

Le projet est organisé comme suit :
- **src/** : Code source (`.c`)
- **include/** : En-têtes (`.h`)
- **tests/** : Tests unitaires et fichiers de données de test
- **docs/** : Documentation supplémentaire

### Fichiers sources (src/):
- **main.c** - Programme principal et boucle interactive
- **interpreter.c** - Noyau de l'interpréteur et orchestration
- **commands.c** - Commandes BASIC (PRINT, LET, DIM, INPUT)
- **control_flow.c** - Gestion du flux de contrôle (IF/FOR/GOTO/GOSUB)
- **expression.c** - Évaluation des expressions arithmétiques et chaînes
- **variables.c** - Gestion des variables et tableaux multi-dimensionnels
- **lexer.c** - Analyse lexicale et tokenisation

### Composants:
1. **Lexer** - Tokenise le code source (41 mots-clés, 56 types de tokens)
2. **Variables** - Gestion variables numériques, chaînes, tableaux jusqu'à 10 dimensions
3. **Expression** - Évaluation avec priorité des opérateurs, fonctions mathématiques et chaînes
4. **Control Flow** - Gestion IF/THEN/ELSE, FOR/NEXT, GOTO, GOSUB/RETURN
5. **Commands** - Exécution des commandes PRINT, LET, DIM, INPUT
6. **Interpreter** - Orchestration minimale du programme BASIC

## Limitations actuelles

- Pas de gestion d'erreurs avancée
- Les angles des fonctions trigonométriques sont en radians (utilisez RAD/DEG pour la conversion)

## Extensions possibles

- Sauvegarder/charger des programmes depuis des fichiers
- Améliorer les diagnostics d'erreurs (ligne, colonne, message)
