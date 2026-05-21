# Audit des optimisations possibles - Basic80

## Date

21 mai 2026

---

## Résumé exécutif

Le projet est déjà dans un état correct sur trois axes importants : modularisation, robustesse mémoire et couverture de tests. Les documents existants montrent que la dette principale n'est plus une dette de structure brute, mais une dette de performance et d'observabilité.

Les optimisations les plus rentables ne sont pas des micro-ajustements. Elles concernent surtout les structures de données et les parcours répétés dans l'interpréteur :

1. Remplacer les recherches linéaires de variables et de lignes par des index dédiés.
2. Éviter les recalculs de stride pour les tableaux multidimensionnels.
3. Réduire la re-tokenization et les allocations temporaires dans les chemins chauds.
4. Ajouter un minimum d'instrumentation pour mesurer les gains avant et après changement.

En pratique, les gains les plus probables sont concentrés dans quatre fichiers : src/vars.c, src/ctrlflow.c, src/interp.c et src/expr.c.

---

## Ce qui est déjà bien traité

Les points suivants ne doivent pas être traités comme priorités d'optimisation immédiates, car ils ont déjà fait l'objet d'un travail sérieux :

- Gestion mémoire sécurisée : voir MEMORY_MANAGEMENT.md.
- Refactorisation modulaire : voir REFACTORING.md.
- Gestion d'erreurs enrichie : voir GLOBAL_IMPROVEMENTS.md et ERROR_HANDLING.md.
- Couverture fonctionnelle large : tests unitaires et tests d'intégration BASIC déjà présents.

Conséquence : la prochaine phase utile est une phase d'optimisation pilotée par les coûts d'exécution et par la mesure.

---

## Matrice de priorisation

| Priorité | Sujet | Impact attendu | Coût | Risque | Décision |
|----------|-------|----------------|------|--------|----------|
| P1 | Index des variables | Très élevé | Moyen | Moyen | À implémenter en premier |
| P1 | Index des lignes de programme | Élevé | Moyen | Faible à moyen | À implémenter juste après |
| P1 | Cache de strides pour tableaux | Élevé sur charges array | Faible | Faible | Quick win structurel |
| P2 | Réduction de la re-tokenization dans FOR et RUN | Moyen à élevé | Moyen | Moyen | Après indexation |
| P2 | Réduction des copies de noms de variables | Moyen | Faible | Faible | Quick win local |
| P2 | Accélération RESTORE par index DATA | Moyen | Faible à moyen | Faible | À faire si usage DATA intensif |
| P3 | Réduction des allocations de chaînes temporaires | Moyen | Moyen | Moyen | À traiter après instrumentation |
| P3 | Optimisation des hooks personnalisés | Faible à moyen | Faible | Faible | Secondaire |
| P3 | Optimisation du chargement/édition des lignes | Faible à moyen | Moyen | Faible | Secondaire |
| P4 | Précompilation légère ou AST | Potentiellement élevé | Élevé | Élevé | Seulement après benchmarks |

---

## Optimisations prioritaires

### 1. Remplacer la recherche linéaire des variables

#### Constat

La fonction findVariable dans src/vars.c parcourt une liste chaînée simple pour chaque accès variable. Elle est utilisée par setVariable, getVariable, setStringVariable, getStringVariable, setArrayElement et getArrayElement.

Points d'ancrage :

- src/vars.c : findVariable
- src/vars.c : setVariable
- src/vars.c : getVariable
- src/vars.c : setArrayElement
- src/vars.c : getArrayElement

#### Problème

Chaque lecture ou écriture de variable est en O(n). Sur des programmes BASIC avec beaucoup de variables ou des boucles contenant de nombreuses expressions, le coût cumulé devient dominant.

Ce coût est amplifié par le fait que l'évaluation d'expressions appelle souvent plusieurs accès variables dans la même instruction.

#### Optimisation proposée

Introduire un index par table de hachage côté interpréteur, tout en conservant éventuellement la liste chaînée pour la libération mémoire et l'itération.

Approche recommandée :

- Ajouter une table de buckets dans la structure Interpreter.
- Faire pointer chaque Variable à la fois dans la liste globale et dans une chaîne de collision de hachage.
- Centraliser l'insertion et la recherche dans un unique module d'accès.

#### Impact attendu

- Passage de O(n) à O(1) amorti pour les accès variables.
- Gain majeur sur LET, PRINT, IF, FOR, évaluations arithmétiques et tableaux.

#### Risques

- Cohérence entre la liste de variables et l'index de hachage.
- Gestion de remplacement lors de redéclaration ou changement de type.

#### Validation recommandée

- Rejouer les tests variables, expressions, FOR et tableaux.
- Ajouter un micro-benchmark avec plusieurs milliers d'accès à 100, 500 puis 1000 variables.

---

### 2. Remplacer la recherche linéaire des lignes de programme

#### Constat

La fonction findLineByNumber dans src/ctrlflow.c parcourt toute la liste des lignes jusqu'à trouver le numéro demandé. addLine dans src/interp.c effectue aussi une insertion triée par parcours linéaire.

Points d'ancrage :

- src/ctrlflow.c : findLineByNumber
- src/ctrlflow.c : handleGoto
- src/ctrlflow.c : handleGosub
- src/interp.c : addLine

#### Problème

GOTO, GOSUB, RESTORE indirectement, ainsi que l'édition du programme en mémoire, restent en O(n). Cela pèse surtout sur les programmes longs ou fortement structurés autour des sauts.

#### Optimisation proposée

Ajouter un index des lignes par numéro.

Deux options réalistes :

1. Table de hachage lineNum -> Line*.
2. Tableau dynamique trié de pointeurs vers Line avec recherche binaire.

La table de hachage est la plus simple si l'objectif principal est l'exécution. Le tableau trié peut être intéressant si l'on veut aussi préparer d'autres parcours ordonnés.

#### Impact attendu

- GOTO et GOSUB en O(1) amorti ou O(log n).
- Accélération visible sur programmes riches en branches et sous-routines.

#### Risques

- Maintien de la cohérence lors de addLine, deleteLine, clearProgram, loadProgram.

#### Validation recommandée

- Rejouer tous les tests GOTO, GOSUB, RETURN, IF...THEN GOTO et chargement de fichiers.

---

### 3. Mettre en cache les strides des tableaux multidimensionnels

#### Constat

computeFlatIndex dans src/vars.c recalcule les multiplicateurs internes à chaque accès à un élément de tableau.

Point d'ancrage :

- src/vars.c : computeFlatIndex

#### Problème

L'algorithme actuel contient une boucle imbriquée sur les dimensions. Le coût devient O(d²) par accès, alors que les strides peuvent être calculés une seule fois lors du DIM.

#### Optimisation proposée

- Ajouter un tableau strides dans la structure Variable.
- Calculer ces strides dans createArray.
- Remplacer le calcul dynamique par une simple somme indices[i] * strides[i].

#### Impact attendu

- Gain direct sur tous les accès A(i), A(i,j), A(i,j,k).
- Très rentable sur les programmes de calcul matriciel, boucles imbriquées et tests 2D/3D.

#### Risques

- Ajustements nécessaires dans freeInterpreter et dans la recréation d'un tableau existant.

#### Validation recommandée

- Rejouer tous les tests de tableaux 1D, 2D et 3D.
- Ajouter un benchmark d'écriture puis de lecture séquentielle sur matrice 2D et cube 3D.

---

### 4. Réduire la re-tokenization pendant l'exécution

#### Constat

Le projet tokenise encore plusieurs fois des chaînes qui pourraient être réutilisées.

Points d'ancrage :

- src/ctrlflow.c : handleFor, quand une boucle est ignorée et que le code re-tokenize chaque ligne jusqu'au NEXT correspondant.
- src/interp.c : runProgram, première passe DATA puis seconde passe exécution.
- src/interp.c : executeSingleStatement et executeStatementInProgram, qui retokenisent les sous-instructions.

#### Problème

Le coût de tokenization se répète sur des lignes déjà vues. Cela peut dégrader les performances lorsque :

- les programmes ont beaucoup de lignes,
- les boucles sont imbriquées,
- les lignes contiennent plusieurs sous-instructions séparées par ':'.

#### Optimisation proposée

Approche progressive recommandée :

1. Quick win : mémoriser le type du premier token par ligne pour éviter de re-tokenizer lors de certains scans FOR/NEXT et de la collecte DATA.
2. Niveau suivant : stocker une version pré-splittée des lignes au chargement.
3. Niveau avancé : stocker les tokens de chaque sous-instruction pendant RUN.

#### Impact attendu

- Gain moyen à élevé sur les programmes structurés avec beaucoup de flux de contrôle.

#### Risques

- Augmentation de la mémoire consommée.
- Besoin de bien invalider le cache lors de addLine, deleteLine, loadProgram et NEW.

#### Validation recommandée

- Rejouer les tests IF/FOR/NEXT, DATA et instructions multiples sur une même ligne.

---

## Optimisations secondaires utiles

### 5. Réduire les copies de noms de variables dans l'évaluateur

#### Constat

expr.c et commands.c copient fréquemment les noms de variables dans des buffers locaux de taille 256 avant lookup ou affectation.

Points d'ancrage :

- src/expr.c : evaluateFactor
- src/expr.c : evaluateStringPrimary
- src/commands.c : handleLet
- src/commands.c : handleDim
- src/commands.c : handleInput
- src/commands.c : handleRead

#### Problème

Le coût unitaire est faible, mais très fréquent. Il ajoute des copies inutiles sur un chemin déjà chaud.

#### Optimisation proposée

- Utiliser directement tokens[pos].value quand la durée de vie du token est suffisante.
- Ne conserver les copies locales que lorsqu'une mutation ou une normalisation est nécessaire.

#### Impact attendu

- Gain faible à moyen, mais presque gratuit à mettre en place.

#### Risques

- Ne pas conserver un pointeur vers une mémoire libérée après freeTokens.

---

### 6. Accélérer RESTORE avec un index DATA par ligne

#### Constat

handleRestore dans src/commands.c repart depuis le début de la liste DATA pour trouver le premier item de la ligne cible.

Points d'ancrage :

- src/commands.c : handleData
- src/commands.c : handleRead
- src/commands.c : handleRestore
- include/interp.h : DataItem

#### Problème

RESTORE ligne est en O(n) sur le nombre total d'items DATA. Ce n'est pas dramatique sur petits programmes, mais devient coûteux si RESTORE est utilisé dans des boucles ou sur un grand catalogue DATA.

#### Optimisation proposée

- Ajouter un index lineNum -> premier DataItem de cette ligne.
- Le construire pendant la première passe DATA de runProgram.

#### Impact attendu

- RESTORE ligne en O(1) amorti.

#### Risques

- Cohérence si le programme est modifié puis rejoué.

---

### 7. Réduire les allocations temporaires de chaînes

#### Constat

evaluateStringPrimary et handlePrint allouent de nombreuses chaînes temporaires pour conversions, sous-chaînes et concaténations.

Points d'ancrage :

- src/expr.c : evaluateStringPrimary
- src/expr.c : evaluateStringExpression
- src/commands.c : handlePrint

#### Problème

La robustesse mémoire est bonne, mais le nombre d'allocations reste élevé. Cela augmente la fragmentation et le coût CPU sur les programmes qui manipulent intensivement des chaînes.

#### Optimisation proposée

Options progressives :

1. Introduire un petit buffer réutilisable pour conversions numériques simples.
2. Introduire un constructeur de chaîne local pour les concaténations.
3. Réserver les allocations dynamiques uniquement aux résultats persistants.

#### Impact attendu

- Gain surtout sur les scénarios riches en LEFT$, RIGHT$, MID$, STR$, STRING$, concaténation et PRINT de chaînes.

#### Risques

- Complexification du cycle de vie mémoire.
- Risque de réutilisation incorrecte si un buffer temporaire est partagé entre appels imbriqués.

---

### 8. Optimiser les hooks personnalisés si leur usage devient important

#### Constat

Les registres de hooks personnalisés sont stockés dans des listes chaînées et les lookups reconstruisent une version uppercase du nom à chaque recherche.

Points d'ancrage :

- src/interp.c : registerCustomNumericFunction
- src/interp.c : registerCustomStringFunction
- src/interp.c : registerCustomCommand
- src/interp.c : findCustomNumericFunction
- src/interp.c : findCustomStringFunction
- src/interp.c : findCustomCommand

#### Problème

Le coût reste faible tant que peu d'extensions sont enregistrées. Si le mécanisme d'extension devient un point central, le lookup finira par devenir un coût visible.

#### Optimisation proposée

- Réutiliser la même infrastructure de table de hachage que pour les variables.
- Conserver le nom normalisé au moment de la tokenization ou de l'enregistrement.

#### Impact attendu

- Secondaire aujourd'hui, potentiellement utile demain.

---

### 9. Réduire le coût de splitByColon et des allocations associées

#### Constat

splitByColon dans src/interp.c alloue un tableau dynamique de segments et copie chaque segment dans une nouvelle chaîne.

Points d'ancrage :

- src/interp.c : splitByColon
- src/interp.c : executeCommand
- src/interp.c : runProgram

#### Problème

Cette logique est propre et robuste, mais elle crée beaucoup d'allocations temporaires pour des lignes qui seront immédiatement exécutées puis libérées.

#### Optimisation proposée

- Conserver les offsets de découpe plutôt que copier toutes les sous-chaînes.
- Ou pré-splitter les lignes au chargement du programme.

#### Impact attendu

- Gain modéré, surtout sur les programmes qui utilisent massivement ':' pour compacter plusieurs instructions.

---

## Améliorations d'architecture recommandées

### 10. Séparer explicitement les structures chaudes des structures froides

#### Constat

Interpreter concentre les lignes, variables, piles de contrôle, DATA, erreurs et hooks dans une seule structure.

Point d'ancrage :

- include/interp.h : struct Interpreter

#### Opportunité

À mesure que des caches et des index seront ajoutés, la structure va grossir. Il devient utile de distinguer :

- données chaudes d'exécution,
- état de diagnostics,
- registres d'extensions,
- caches de programme.

#### Recommandation

Créer à terme des sous-structures internes, par exemple :

- ProgramIndex
- VariableStore
- RuntimeState
- ExtensionRegistry

Ce n'est pas une optimisation de premier ordre en soi, mais cela réduit fortement le coût de maintenance des optimisations futures.

---

### 11. Centraliser les invariants de stockage

#### Constat

Plusieurs règles de cohérence sont aujourd'hui diffuses : ordre des lignes, synchronisation des piles, validité des dimensions, cohérence des pointeurs DATA.

#### Recommandation

Avant de multiplier les caches, centraliser explicitement ces invariants dans un petit nombre de fonctions internes. Cela limitera les régressions quand des index additionnels seront introduits.

---

## Build, tests et observabilité

### 12. Ajouter une vraie couche de benchmark

#### Constat

Le projet dispose de bons tests fonctionnels, mais pas d'une boucle de mesure de performance. Les documents existants mentionnent d'ailleurs l'absence de benchmarks.

#### Recommandation

Ajouter un exécutable ou un mode de test dédié contenant au minimum les scénarios suivants :

1. Accès intensif à 1000 variables.
2. Boucles FOR imbriquées avec tableaux 2D et 3D.
3. Programme riche en GOTO/GOSUB.
4. Manipulation intensive de chaînes.
5. Programme contenant beaucoup de DATA et de RESTORE.

#### Mesures à capturer

- Temps total d'exécution.
- Nombre d'accès variables.
- Nombre de tokenizations.
- Nombre d'allocations temporaires si une instrumentation légère est ajoutée.

---

### 13. Ajouter des tests de non-régression orientés performance structurelle

Les optimisations proposées modifient surtout les structures de données. Il faut donc compléter les tests actuels avec des assertions de comportement sur :

- cohérence entre accès simples et accès indexés,
- invalidation correcte des caches après NEW, LOAD, addLine et deleteLine,
- stabilité du comportement des tableaux multidimensionnels,
- exactitude des boucles FOR ignorées,
- exactitude de RESTORE avec et sans numéro de ligne.

---

## Ordre d'implémentation recommandé

### Phase 1 - Gains immédiats

1. Cache de strides pour tableaux.
2. Réduction des copies de noms de variables.
3. Index DATA optionnel si les scénarios RESTORE sont fréquents.

### Phase 2 - Gains structurants

1. Table de hachage pour variables.
2. Index des lignes de programme.

### Phase 3 - Gains transverses

1. Réduction de la re-tokenization.
2. Optimisation de splitByColon.
3. Réduction des allocations de chaînes temporaires.

### Phase 4 - Travail avancé uniquement si mesuré utile

1. Précompilation légère des lignes.
2. Stockage de tokens réutilisables.
3. AST ou représentation intermédiaire.

---

## Quick wins concrets

Voici les changements à faible risque et à bon ratio effort/gain :

1. Ajouter des strides précalculés aux tableaux.
2. Éviter les strcpy inutiles dans expr.c et commands.c.
3. Ajouter un cache minimal lineNum -> Line*.
4. Ajouter un compteur de tokenization et un compteur d'accès variable en mode debug.

---

## Proposition de feuille de route technique

### Itération 1

- Ajouter instrumentation simple.
- Ajouter strides de tableaux.
- Mesurer avant/après.

### Itération 2

- Ajouter index des variables.
- Rejouer toute la suite de tests.
- Ajouter benchmark variables.

### Itération 3

- Ajouter index des lignes.
- Rejouer les tests de contrôle de flux.
- Mesurer les scénarios GOTO/GOSUB.

### Itération 4

- Réduire la re-tokenization et les allocations temporaires.
- Ajuster uniquement ce qui produit un gain démontré.

---

## Conclusion

Le projet n'a pas besoin d'une nouvelle grande refonte générale. Il a besoin d'un cycle court d'optimisations ciblées, mesurées et protégées par les tests existants.

La meilleure séquence est la suivante :

1. Mesurer.
2. Corriger les structures de données dominantes.
3. Réduire les recalculs et allocations répétées.
4. N'introduire des optimisations plus profondes que si les benchmarks justifient leur coût.

En l'état, les deux sujets les plus rentables sont clairement l'indexation des variables et l'indexation des lignes, suivis immédiatement par le cache de strides des tableaux.