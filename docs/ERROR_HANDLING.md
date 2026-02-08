# Système de Gestion d'Erreurs Amélioré

## Vue d'ensemble

Le système de gestion d'erreurs a été amélioré pour fournir des informations de contexte détaillées lors des erreurs, facilitant le debug et améliorant l'expérience utilisateur.

## Types d'Erreurs

Le système reconnaît maintenant plusieurs types d'erreurs catégorisées :

| Type | Description | Exemple |
|------|-------------|---------|
| `ERR_SYNTAX` | Erreur de syntaxe | `LET = 10` (nom de variable manquant) |
| `ERR_RUNTIME` | Erreur d'exécution générale | Erreur pendant l'exécution du programme |
| `ERR_TYPE_MISMATCH` | Incompatibilité de types | `LET A = B$` (chaîne → numérique) |
| `ERR_OUT_OF_DATA` | Épuisement des données DATA | `READ` sans `DATA` correspondant |
| `ERR_DIVISION_ZERO` | Division par zéro | `LET A = 10 / 0` |
| `ERR_UNDEFINED_VAR` | Variable non définie | Utilisation d'une variable non initialisée |
| `ERR_ARRAY_BOUNDS` | Dépassement de tableau | Accès hors des limites du tableau |
| `ERR_OUT_OF_MEMORY` | Mémoire insuffisante | Échec d'allocation mémoire |

## Format des Messages d'Erreur

Les messages d'erreur affichent maintenant :

1. **Type d'erreur** : Catégorie de l'erreur (Syntaxe, Exécution, etc.)
2. **Numéro de ligne** : Ligne où l'erreur s'est produite
3. **Colonne** (si disponible) : Position dans la ligne
4. **Message descriptif** : Explication de l'erreur
5. **Code source** : Ligne de code incriminée
6. **Curseur** : Pointeur visuel vers la position de l'erreur

### Exemple de sortie

```
[ERREUR Syntaxe] Ligne 30, colonne 1: Nom de variable attendu après LET.
  --> LET = 10
      ^
```

```
[ERREUR Division] Ligne 50, colonne 5: Division par zéro.
  --> LET A = 10 / 0
           ^
```

```
[ERREUR Type] Ligne 80, colonne 9: Impossible d'assigner une chaîne à une variable numérique.
  --> LET C = B$
              ^
```

## Utilisation dans le Code

### Pour les Développeurs

#### Fonction `reportErrorEx()`

Utilisez `reportErrorEx()` pour signaler des erreurs avec contexte complet :

```c
void reportErrorEx(Interpreter *interp, ErrorType type, int column, const char *message);
```

**Paramètres :**
- `interp` : Pointeur vers l'interpréteur
- `type` : Type d'erreur (voir énumération `ErrorType`)
- `column` : Position du token dans la ligne (ou -1 si non disponible)
- `message` : Message descriptif de l'erreur

**Exemple :**
```c
if (tokens[pos].type != TOK_IDENTIFIER) {
    reportErrorEx(interp, ERR_SYNTAX, pos, "Nom de variable attendu après LET.");
    return;
}
```

#### Fonction `reportError()` (Legacy)

La fonction originale est toujours disponible pour la rétrocompatibilité :

```c
void reportError(Interpreter *interp, const char *message);
```

Cette fonction appelle maintenant `reportErrorEx()` avec `ERR_RUNTIME` et `column = -1`.

#### Fonction `getErrorTypeName()`

Obtient le nom d'un type d'erreur :

```c
const char* getErrorTypeName(ErrorType type);
```

## Structure Interpreter

La structure `Interpreter` a été enrichie avec les champs suivants :

```c
struct Interpreter {
    /* ... champs existants ... */
    ErrorType lastErrorType;    /* Type de la dernière erreur */
    int errorColumn;            /* Colonne de l'erreur */
    char errorContext[256];     /* Contexte de l'erreur (extrait de code) */
};
```

Ces champs permettent de :
- Tracer l'historique des erreurs
- Implémenter des gestionnaires d'erreurs personnalisés
- Améliorer les outils de débogage

## Avantages

1. **Localisation précise** : Identification rapide de la source de l'erreur
2. **Catégorisation** : Distinction entre types d'erreurs (syntaxe vs exécution)
3. **Feedback visuel** : Pointeur visuel sur la position exacte de l'erreur
4. **Débogage facilité** : Informations détaillées pour résoudre les problèmes
5. **Expérience utilisateur** : Messages clairs et informatifs

## Évolutions Futures

- **Stack trace** : Afficher la pile d'appels (GOSUB) lors des erreurs
- **Suggestions** : Proposer des corrections automatiques
- **Mode debug** : Breakpoints et exécution pas à pas
- **Log des erreurs** : Enregistrement dans un fichier
- **Colorisation** : Mise en évidence syntaxique des messages d'erreur
