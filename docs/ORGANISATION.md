# Organisation du code - Interpréteur BASIC

## Structure modulaire actuelle

### 📁 Fichiers principaux

#### **main.c**
- **Rôle** : Point d'entrée REPL (Read-Eval-Print Loop)
- **Dépendances** : interpreter.h
- **Compilation** : `clang -std=c89 -pedantic -Wall -g main.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o basic.exe`

#### **tests.c** 
- **Rôle** : 184 tests unitaires couvrant toutes les fonctionnalités
- **Dépendances** : interpreter.h
- **Compilation** : `clang -std=c89 -pedantic -Wall -g tests.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o tests.exe`
- **Exécution** : `.\tests.exe`

#### **debug_runner.c**
- **Rôle** : Programme de test pour debug des tableaux
- **Dépendances** : interpreter.h  
- **Compilation** : `clang -std=c89 -pedantic -Wall -g debug_runner.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o debug_runner.exe`

---

### 📦 Modules extraits

#### **Module lexer** (lexer.h + lexer.c)
**Lignes** : 63 + 156 = 219 lignes

**Responsabilités** :
- Analyse lexicale du code BASIC
- Conversion du texte source en tokens
- Reconnaissance des mots-clés (37 keywords)

**Exports** :
- `typedef enum TokenType` - 52 types de tokens
- `typedef struct Token` - Structure de token
- `Token* tokenize(const char *line, int lineNum)` - Tokenisation
- `void freeTokens(Token *tokens)` - Libération mémoire

**Dépendances** : Aucune (standalone)

---

#### **Module variables** (variables.h + variables.c)
**Lignes** : 32 + 227 = 259 lignes

**Responsabilités** :
- Gestion des variables numériques et chaînes
- Gestion des tableaux multi-dimensionnels (jusqu'à 10 dimensions)
- Stockage en liste chaînée

**Exports** :
- `typedef struct Variable` - Structure de variable
- `Variable* findVariable()` - Recherche de variable
- `void setVariable()` / `double getVariable()` - Variables numériques
- `void setStringVariable()` / `char* getStringVariable()` - Variables chaînes
- `void createArray()` - Création de tableaux
- `void setArrayElement()` / `double getArrayElement()` - Accès aux éléments (indexation row-major)

**Dépendances** : interpreter.h (forward declaration de Interpreter)

**Note importante** : `getArrayElement()` prend maintenant 4 paramètres :
```c
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);
```

---

#### **Module expression** (expression.h + expression.c)
**Lignes** : 22 + 379 = 401 lignes

**Responsabilités** :
- Évaluation des expressions arithmétiques (priorité des opérateurs)
- Évaluation des conditions (comparaisons)
- Évaluation des expressions de chaînes
- Gestion des fonctions mathématiques intégrées

**Exports** :
- `int evaluateCondition()` - Conditions pour IF...THEN
- `double evaluateExpression()` - Expressions (+ -)
- `double evaluateTerm()` - Termes (* /)
- `double evaluateFactor()` - Facteurs (nombres, variables, fonctions)
- `int isStringExpression()` - Détection d'expressions de chaînes
- `char* evaluateStringExpression()` - Concaténation (+)
- `char* evaluateStringPrimary()` - Primitives de chaînes

**Fonctions intégrées gérées** :
- **Math** : SIN, COS, TAN, SQR, ABS, INT, RND
- **Strings** : LEN, ASC, CHR, MID, LEFT, RIGHT

**Dépendances** : interpreter.h, lexer.h, variables.h

---

#### **Module control_flow** (control_flow.h + control_flow.c)
**Lignes** : 39 + 256 = 295 lignes

**Responsabilités** :
- Gestion du flux de contrôle (boucles, conditions, sauts)
- Gestion des instructions IF/THEN/ELSE avec support GOTO
- Gestion des boucles FOR/NEXT avec détection de conditions impossibles
- Gestion des sous-routines GOSUB/RETURN
- Gestion des sauts GOTO

**Exports** :
- `int handleIfStatement()` - IF/THEN/ELSE avec support GOTO intégré
- `int handleGoto()` - Saut vers une ligne
- `int handleGosub()` - Appel de sous-routine
- `int handleReturn()` - Retour de sous-routine
- `int handleFor()` - Initialisation de boucle FOR
- `int handleNext()` - Itération de boucle NEXT
- `Line* findLineByNumber()` - Recherche de ligne par numéro

**Dépendances** : interpreter.h, lexer.h, expression.h, variables.h

---

#### **Module commands** (commands.h + commands.c)
**Lignes** : 27 + 134 = 161 lignes

**Responsabilités** :
- Gestion des commandes BASIC principales
- Affichage (PRINT) avec support multiples expressions
- Affectation (LET) de variables, tableaux et chaînes
- Déclaration de tableaux (DIM) multi-dimensionnels
- Lecture interactive (INPUT)

**Exports** :
- `void handlePrint()` - Affichage avec détection automatique du type
- `void handleLet()` - Affectation universelle (variables/tableaux/chaînes)
- `void handleDim()` - Déclaration de tableaux (jusqu'à 10 dimensions)
- `void handleInput()` - Lecture de valeurs numériques

**Dépendances** : interpreter.h, lexer.h, expression.h, variables.h

---

### 🎯 Noyau central

#### **interpreter.h + interpreter.c**
**Lignes** : 55 + 189 = 244 lignes (réduit de 85% depuis le début)

**Responsabilités restantes** :
- Structures principales : `Line`, `ForLoop`, `CallStack`, `Interpreter`
- Gestion du programme : `addLine()`, `createInterpreter()`, `freeInterpreter()`
- Orchestration minimale : `executeCommand()` (24 lignes), `runProgram()`
- Délégation aux modules spécialisés

**Includes** :
```c
#include "interpreter.h"
#include "control_flow.h"
#include "commands.h"
```

**Évolution** :
- ✅ Version initiale : 1242 lignes (monolithique)
- ✅ Après Étape 1 (lexer) : 1057 lignes (-15%)
- ✅ Après Étape 2 (variables) : 861 lignes (-31%)
- ✅ Après Étape 3 (expression) : 492 lignes (-60%)
- ✅ Après Étape 4 (control_flow) : 286 lignes (-77%)
- ✅ Après Étape 5 (commands) : **189 lignes (-85%)**

---

## 🔧 Compilation

### Commandes par cible

**Interpréteur REPL** :
```bash
clang -std=c89 -pedantic -Wall -g main.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o basic.exe
```

**Tests unitaires** :
```bash
clang -std=c89 -pedantic -Wall -g tests.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o tests.exe
```

**Debug runner** :
```bash
clang -std=c89 -pedantic -Wall -g debug_runner.c interpreter.c lexer.c variables.c expression.c control_flow.c commands.c -o debug_runner.exe
```

**⚠️ NE PAS compiler avec `*.c`** car cela inclurait plusieurs fichiers avec `main()` (main.c, tests.c, debug_runner.c).

---

## 📊 Statistiques

| Composant | Lignes | % du total | Statut |
|-----------|--------|------------|--------|
| **lexer.h/c** | 219 | 16% | ✅ Module extrait |
| **variables.h/c** | 259 | 20% | ✅ Module extrait |
| **expression.h/c** | 401 | 30% | ✅ Module extrait |
| **control_flow.h/c** | 295 | 22% | ✅ Module extrait |
| **commands.h/c** | 161 | 12% | ✅ Module extrait |
| **interpreter.h/c** | 244 | 18% | 🏆 Orchestration pure |
| **Total** | 1579 | 100% | |

**Réduction de interpreter.c** : 1242 → 189 lignes (-85%)

---

## 🧪 Validation

**Tests** : 184/184 passent ✅

**Conformité** : C89/ANSI C strict (`-std=c89 -pedantic`)

---

## 🏆 Refactorisation complète

### Objectifs atteints :
✅ **5 modules extraits** avec responsabilités clairement définies  
✅ **interpreter.c réduit à 189 lignes** (orchestration minimale)  
✅ **Tous les modules < 400 lignes** (maintenabilité optimale)  
✅ **Architecture modulaire exemplaire** en C89 strict  
✅ **Aucun test régressé** (184/184 passent)  

### Architecture finale :
```
interpreter.c (189 lignes) - Orchestration
    └─ commands.c (134 lignes) - PRINT, LET, DIM, INPUT
    └─ control_flow.c (256 lignes) - IF, FOR, GOTO, GOSUB
    └─ expression.c (382 lignes) - Évaluation expressions
    └─ variables.c (211 lignes) - Variables et tableaux
    └─ lexer.c (161 lignes) - Tokenisation
```

---

## 📝 Notes importantes pour le développement

### Signature des fonctions modifiées

**getArrayElement** et **setArrayElement** :
```c
// ❌ Ancienne signature (ne plus utiliser)
double getArrayElement(Interpreter *interp, const char *name, int index);

// ✅ Nouvelle signature (depuis refactorisation)
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);
```

**Exemple d'utilisation** :
```c
int idx[1];
idx[0] = 5;
double value = getArrayElement(interp, "A", idx, 1);  // A(5)

int idx2[2];
idx2[0] = 2;
idx2[1] = 3;
double value2 = getArrayElement(interp, "M", idx2, 2);  // M(2,3)
```

### Ordre des includes

Toujours respecter cet ordre dans les fichiers sources :
```c
#include "lexer.h"         // Pas de dépendances
#include "variables.h"     // Dépend de Interpreter (forward decl)
#include "expression.h"    // Dépend de lexer et variables
#include "interpreter.h"   // Inclut les 3 précédents
```

---

**Dernière mise à jour** : Après extraction du module expression (Étape 3)
