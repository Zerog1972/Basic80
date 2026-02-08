# Exemples d'Extensions Basic80

Ce dossier contient des exemples concrets d'utilisation du système d'extensions de Basic80.

## calculator.c - Calculatrice Scientifique

Une calculatrice scientifique complète qui démontre comment créer et utiliser des extensions personnalisées.

### Fonctionnalités

**14 fonctions mathématiques :**
- `SQUARE(x)`, `CUBE(x)`, `POWER(x, y)`, `SQRT(x)`
- `LOG(x)`, `LOG10(x)`
- `CEIL(x)`, `FLOOR(x)`, `ROUND(x)`
- `MIN(a, b)`, `MAX(a, b)`
- `HYPOT(x, y)`, `DEG2RAD(deg)`, `RAD2DEG(rad)`

**2 fonctions de chaînes :**
- `TRIM$(str)` - Supprime les espaces
- `REPLACE$(str, old, new)` - Remplace une sous-chaîne

**2 commandes :**
- `VERSION` - Affiche la version
- `FUNCS` - Liste toutes les fonctions

### Compilation

```bash
clang -std=c89 -pedantic -Wall -g ^
  examples/calculator.c ^
  src/lexer.c src/variables.c src/expression.c ^
  src/interpreter.c src/commands.c src/control_flow.c ^
  -Iinclude -o calculator.exe
```

### Utilisation

```bash
calculator.exe
```

### Exemples d'utilisation

```basic
calc> PRINT SQUARE(5)
25.00

calc> PRINT POWER(2, 10)
1024.00

calc> PRINT SQRT(144)
12.00

calc> PRINT MAX(10, 25)
25.00

calc> PRINT HYPOT(3, 4)
5.00

calc> PRINT DEG2RAD(90)
1.57

calc> LET X = 5
calc> PRINT "Le carre de"; X; "est"; SQUARE(X)
Le carre de 5.00 est 25.00

calc> PRINT TRIM$("  hello  ")
hello

calc> FUNCS
[Liste de toutes les fonctions disponibles]

calc> VERSION
Calculatrice Scientifique Basic80 v1.0
```

## Structure d'une extension

Chaque extension suit cette structure :

```c
#include "interpreter.h"
#include "expression.h"

/* 1. Définir la fonction */
static double my_function(Interpreter *interp, Token *tokens, int *pos) {
    double arg = evaluateExpression(interp, tokens, pos);
    /* Traitement */
    return result;
}

/* 2. Enregistrer la fonction */
int main(void) {
    Interpreter *interp = createInterpreter();
    registerCustomNumericFunction(interp, "MYFUNCTION", my_function);
    /* ... */
}
```

## Créer votre propre extension

1. **Copiez** `calculator.c` comme point de départ
2. **Modifiez** les fonctions selon vos besoins
3. **Enregistrez** vos fonctions dans `main()`
4. **Compilez** avec les sources de Basic80
5. **Testez** votre extension

## Points clés

### Gestion de la mémoire
- Les fonctions chaînes doivent **retourner de la mémoire allouée**
- L'appelant libère automatiquement la mémoire
- Toujours vérifier les allocations (`malloc()` peut échouer)

### Gestion des erreurs
- Utiliser `reportErrorEx()` pour signaler les erreurs
- Valider les arguments (négatifs, zéro, etc.)
- Retourner une valeur par défaut sûre en cas d'erreur

### Arguments multiples
- Lire chaque argument avec `evaluateExpression()`
- Consommer les virgules avec `if (tokens[*pos].type == TOK_COMMA) (*pos)++;`
- Ne pas oublier d'incrémenter `*pos` après chaque élément

### Bonnes pratiques
- Noms clairs et descriptifs (SQUARE, HYPOT, TRIM$)
- Documentation complète des fonctions
- Tests de validation des arguments
- Messages d'erreur explicites

## Ressources

- [docs/EXTENSIONS.md](../docs/EXTENSIONS.md) - Documentation complète du système
- [extensions/extensions.c](../extensions/extensions.c) - Module d'extensions général
- [include/interpreter.h](../include/interpreter.h) - API de l'interpréteur
- [include/expression.h](../include/expression.h) - API d'évaluation

---

**Date :** 8 février 2026  
**Auteur :** Basic80 Team
