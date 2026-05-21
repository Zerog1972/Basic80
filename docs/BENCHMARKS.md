# Benchmarks Basic80

## Objectif

Ce document décrit l'infrastructure de benchmark minimale ajoutée au projet pour mesurer les performances de l'interpréteur sur quelques scénarios représentatifs.

Le point d'entrée est :

- tests/benchmarks.c

---

## Scénarios couverts

Le binaire de benchmark mesure actuellement trois familles de charges :

1. variables
   Charge orientée accès répétés à un grand nombre de variables scalaires.

2. arrays
   Charge intensifiée orientée écritures puis lectures répétées dans un tableau 2D 90x90 sur plusieurs passes.

3. controlflow
   Charge orientée FOR/NEXT, GOSUB/RETURN et sauts conditionnels.

Ces scénarios ont été choisis pour servir de base aux optimisations priorisées dans docs/OPTIMIZATIONS.md.

---

## Compilation

Depuis la racine du projet :

```bash
clang -std=c89 -pedantic -Wall -g -D_CRT_SECURE_NO_WARNINGS -Iinclude tests/benchmarks.c src/interp.c src/lexer.c src/vars.c src/expr.c src/ctrlflow.c src/commands.c -o benchmarks.exe
```

La macro _CRT_SECURE_NO_WARNINGS est recommandée sous Windows pour éviter les warnings de la CRT Microsoft déjà présents dans le reste du projet.

---

## Exécution

Par défaut, le binaire exécute chaque scénario 5 fois.

```bash
.\benchmarks.exe
```

Il est aussi possible de fixer le nombre d'itérations :

```bash
.\benchmarks.exe 10
```

---

## Sortie attendue

Le programme affiche pour chaque scénario :

- le temps de chaque itération,
- la moyenne,
- le minimum,
- le maximum.

Exemple de sortie :

```text
Basic80 benchmarks
Iterations par scenario: 2

=== variables ===
Iteration 1: 318.000 ms
Iteration 2: 324.000 ms
Moyenne : 321.000 ms
Min     : 318.000 ms
Max     : 324.000 ms
```

---

## Interprétation

Ces benchmarks ne remplacent pas les tests fonctionnels. Ils servent à comparer des versions du moteur avant et après optimisation.

Recommandations :

1. Lancer plusieurs fois le même benchmark avant de conclure.
2. Comparer des tendances, pas une mesure isolée.
3. Modifier un seul axe d'optimisation à la fois.
4. Rejouer la suite de tests après chaque optimisation structurelle.

Le scénario arrays a été volontairement renforcé pour limiter le bruit de mesure observé sur l'ancienne version 40x40, qui était trop courte pour exposer clairement les gains algorithmiques sur l'indexation multidimensionnelle.

---

## Extensions recommandées

Les prochains scénarios utiles sont :

1. DATA/READ/RESTORE intensif.
2. Concaténations et fonctions de chaînes.
3. Programmes plus longs avec recherche de lignes fréquente.
4. Benchmarks dédiés aux tableaux 3D.

---

## Validation effectuée

L'infrastructure a été validée par :

1. compilation du binaire benchmarks.exe,
2. exécution avec 2 itérations,
3. vérification que les trois scénarios s'exécutent sans erreur interpréteur.