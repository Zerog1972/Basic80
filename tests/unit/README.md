# Tests Unitaires - Basic80

## Vue d'ensemble

Ce répertoire contient la suite complète de tests unitaires pour le projet Basic80. Les tests sont organisés par module et vérifient le bon fonctionnement de chaque composant individuellement.

## Structure

```
tests/unit/
├── test_framework.h          # Framework de tests (macros ASSERT)
├── test_framework.c          # Implémentation du framework
├── test_lexer.c              # Tests du lexeur
├── test_variables.c          # Tests des variables et tableaux
├── test_expression.c         # Tests des expressions
├── test_interpreter.c        # Tests de l'interpréteur
├── run_unit_tests.c          # Point d'entrée principal
├── build_and_test.ps1        # Script de compilation et exécution
├── Makefile                  # Makefile pour compilation
└── README.md                 # Ce fichier
```

## Compilation et Exécution

### Méthode 1 : Script PowerShell (Recommandé sous Windows)

```powershell
cd tests\unit
.\build_and_test.ps1
```

### Méthode 2 : Make

```bash
cd tests/unit
make test
```

### Méthode 3 : Compilation manuelle

```bash
cd tests/unit
clang -std=c89 -pedantic -Wall -g -I../../include \
  ../../src/*.c \
  test_framework.c test_lexer.c test_variables.c \
  test_expression.c test_interpreter.c run_unit_tests.c \
  -o run_unit_tests.exe -lm

./run_unit_tests.exe
```

## Modules Testés

### 1. Lexer (test_lexer.c)

Tests de l'analyse lexicale (tokenization) :
- ✅ Nombres entiers et décimaux
- ✅ Chaînes de caractères
- ✅ Mots-clés (IF, THEN, ELSE, GOTO, etc.)
- ✅ Identifiants de variables
- ✅ Opérateurs arithmétiques (+, -, *, /)
- ✅ Opérateurs de comparaison (<, >, <=, >=, =, <>)
- ✅ Parenthèses et virgules
- ✅ Lignes vides

**Tests : 8 | Assertions : ~30**

### 2. Variables (test_variables.c)

Tests de la gestion des variables :
- ✅ Variables numériques (création, lecture, modification)
- ✅ Variables chaînes (création, lecture, modification)
- ✅ Tableaux 1D, 2D, 3D
- ✅ Accès aux éléments de tableaux
- ✅ Variables non définies (valeurs par défaut)

**Tests : 7 | Assertions : ~20**

### 3. Expressions (test_expression.c)

Tests de l'évaluation d'expressions :
- ✅ Opérations arithmétiques de base
- ✅ Parenthèses et priorité des opérateurs
- ✅ Expressions avec variables
- ✅ Fonctions mathématiques (SIN, COS, ABS, SQR, etc.)
- ✅ Concaténation de chaînes
- ✅ Opérateurs de comparaison

**Tests : 7 | Assertions : ~25**

### 4. Interpreter (test_interpreter.c)

Tests de l'interpréteur :
- ✅ Ajout et suppression de lignes
- ✅ Ordre automatique des lignes
- ✅ Modification de lignes existantes
- ✅ Effacement du programme (NEW)
- ✅ Gestion des erreurs
- ✅ Contexte d'erreur (type, ligne, colonne)

**Tests : 6 | Assertions : ~15**

## Framework de Tests

Le framework de tests fournit des macros pour faciliter l'écriture de tests :

### Macros Disponibles

```c
ASSERT_TRUE(expr, msg)              // Vérifie que expr est vrai
ASSERT_FALSE(expr, msg)             // Vérifie que expr est faux
ASSERT_EQUAL(a, b, msg)             // Vérifie que a == b
ASSERT_NOT_EQUAL(a, b, msg)         // Vérifie que a != b
ASSERT_STR_EQUAL(a, b, msg)         // Vérifie que les chaînes sont égales
ASSERT_NULL(ptr, msg)               // Vérifie que ptr est NULL
ASSERT_NOT_NULL(ptr, msg)           // Vérifie que ptr n'est pas NULL
ASSERT_DOUBLE_EQUAL(a, b, eps, msg) // Vérifie que |a-b| < epsilon
```

### Exemple d'Utilisation

```c
void test_addition(TestStats *stats) {
    Interpreter *interp;
    Token *tokens;
    int pos = 0;
    double result;
    
    interp = createInterpreter();
    tokens = tokenize("10 + 5");
    result = evaluateExpression(interp, tokens, &pos);
    
    ASSERT_DOUBLE_EQUAL(result, 15.0, 0.001, "10 + 5 = 15");
    
    freeTokens(tokens);
    freeInterpreter(interp);
}
```

## Format de Sortie

Les tests affichent un rapport détaillé :

```
╔════════════════════════════════════════╗
║      TESTS UNITAIRES - LEXER          ║
╚════════════════════════════════════════╝

--- Test: Tokenization de nombres ---
  ✓ Tokenize retourne un résultat
  ✓ Premier token est un nombre
  ✓ Valeur du premier nombre

========================================
Module: LEXER
========================================
Tests: 30 | Réussis: 30 | Échecs: 0
✓ TOUS LES TESTS RÉUSSIS
========================================
```

## Statistiques Globales

| Module | Tests | Assertions | Couverture Estimée |
|--------|-------|------------|-------------------|
| Lexer | 8 | ~30 | 85% |
| Variables | 7 | ~20 | 90% |
| Expressions | 7 | ~25 | 80% |
| Interpreter | 6 | ~15 | 75% |
| **TOTAL** | **28** | **~90** | **~80%** |

## Ajout de Nouveaux Tests

### Créer un nouveau fichier de test

1. Créer `test_module.c` :

```c
#include "test_framework.h"
#include "../../include/module.h"

void test_feature(TestStats *stats) {
    printf("\n--- Test: Description ---\n");
    
    /* Code de test */
    ASSERT_TRUE(condition, "Message");
}

void run_module_tests(void) {
    TestStats stats;
    init_test_stats(&stats);
    
    printf("\n╔════════════════════════════════════════╗\n");
    printf("║    TESTS UNITAIRES - MODULE           ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    test_feature(&stats);
    
    print_test_results("MODULE", &stats);
}
```

2. Ajouter la déclaration dans `run_unit_tests.c` :

```c
void run_module_tests(void);

int main(void) {
    /* ... */
    run_module_tests();
    /* ... */
}
```

3. Ajouter le fichier à `build_and_test.ps1` et au `Makefile`.

## Tests d'Intégration vs Tests Unitaires

### Tests Unitaires (Ce répertoire)

- Testent chaque module **isolément**
- Exécution **rapide** (< 1 seconde)
- Détection **précise** des bugs
- Pas de dépendances externes

### Tests d'Intégration (../tests/)

- Testent le système **complet**
- Exécution via fichiers `.bas` et `.txt`
- Validation du **comportement global**
- Tests end-to-end

## Intégration Continue (CI)

Pour intégrer les tests dans un pipeline CI/CD :

```yaml
# Exemple GitHub Actions
- name: Run Unit Tests
  run: |
    cd tests/unit
    powershell -File build_and_test.ps1
```

## Bonnes Pratiques

1. **Un test = une fonctionnalité** : Chaque test doit vérifier un seul aspect
2. **Nettoyage** : Toujours libérer la mémoire (`freeInterpreter`, `freeTokens`)
3. **Messages clairs** : Décrire précisément ce qui est testé
4. **Tests indépendants** : Aucun test ne doit dépendre d'un autre
5. **Couverture** : Viser 80%+ de couverture de code

## Debugging

Pour déboguer un test qui échoue :

```bash
# Compiler avec symboles de debug
clang -g -O0 -I../../include ...

# Exécuter avec debugger
gdb ./run_unit_tests.exe
(gdb) break test_function_name
(gdb) run
```

## Améliorations Futures

- [ ] Tests pour `commands.c` (PRINT, INPUT, DATA/READ)
- [ ] Tests pour `control_flow.c` (IF/THEN, FOR/NEXT, GOSUB/RETURN)
- [ ] Tests de performance (benchmarks)
- [ ] Mesure de couverture de code (gcov/lcov)
- [ ] Tests de stress (programmes volumineux)
- [ ] Tests de mémoire (Valgrind, AddressSanitizer)

## Support

Pour toute question ou problème :
1. Vérifier que clang est installé et dans le PATH
2. Vérifier que tous les fichiers sources sont présents
3. Consulter les messages d'erreur de compilation

---

**Dernière mise à jour** : 8 février 2026
**Mainteneur** : Équipe Basic80
