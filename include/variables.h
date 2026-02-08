#ifndef VARIABLES_H
#define VARIABLES_H

/* Structure pour une variable */
typedef struct Variable {
    char *name;
    int isString;        /* 1 si chaîne, 0 si nombre */
    double value;        /* Valeur numérique */
    char *strValue;      /* Valeur chaîne */
    int isArray;
    double *arrayValues;
    int arraySize;       /* Taille totale (produit de toutes les dimensions) */
    int numDimensions;   /* Nombre de dimensions (1, 2, 3, etc.) */
    int *dimensions;     /* Taille de chaque dimension */
    struct Variable *next;
} Variable;

/* Forward declaration pour Interpreter */
typedef struct Interpreter Interpreter;

/* ===== FONCTIONS DE GESTION DES VARIABLES ===== */

/**
 * Recherche une variable par son nom dans l'interpréteur.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la variable à rechercher (sensible à la casse)
 * @return Pointeur vers la variable si trouvée, NULL sinon
 * 
 * Note: Cette fonction ne crée pas de nouvelle variable si elle n'existe pas.
 */
Variable* findVariable(Interpreter *interp, const char *name);

/**
 * Définit ou modifie la valeur d'une variable numérique.
 * 
 * Si la variable n'existe pas, elle est créée automatiquement.
 * Si elle existe déjà, sa valeur est mise à jour.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la variable (ex: "A", "X", "COUNT")
 * @param value Valeur numérique à assigner
 * 
 * Exemples:
 *   setVariable(interp, "A", 42.0);
 *   setVariable(interp, "PI", 3.14159);
 */
void setVariable(Interpreter *interp, const char *name, double value);

/**
 * Récupère la valeur d'une variable numérique.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la variable à lire
 * @return Valeur de la variable, ou 0.0 si la variable n'existe pas
 * 
 * Note: Les variables non définies retournent 0.0 par défaut (comportement BASIC).
 */
double getVariable(Interpreter *interp, const char *name);

/**
 * Définit ou modifie la valeur d'une variable chaîne.
 * 
 * Si la variable n'existe pas, elle est créée automatiquement.
 * La chaîne est dupliquée en mémoire.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la variable (doit se terminer par $, ex: "A$", "NAME$")
 * @param value Chaîne de caractères à assigner
 * 
 * Exemples:
 *   setStringVariable(interp, "A$", "Hello");
 *   setStringVariable(interp, "NAME$", "John Doe");
 */
void setStringVariable(Interpreter *interp, const char *name, const char *value);

/**
 * Récupère la valeur d'une variable chaîne.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom de la variable chaîne (doit se terminer par $)
 * @return Valeur de la variable, ou "" si la variable n'existe pas
 * 
 * Note: La chaîne retournée ne doit PAS être libérée par l'appelant.
 *       Les variables chaînes non définies retournent "" par défaut.
 */
char* getStringVariable(Interpreter *interp, const char *name);

/* ===== FONCTIONS POUR LES TABLEAUX ===== */

/**
 * Crée un tableau multi-dimensionnel.
 * 
 * Alloue la mémoire nécessaire pour un tableau avec les dimensions spécifiées.
 * Supporte jusqu'à 10 dimensions. Tous les éléments sont initialisés à 0.0.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom du tableau (ex: "A", "M", "T")
 * @param dims Tableau contenant la taille de chaque dimension
 * @param numDims Nombre de dimensions (1 à 10)
 * 
 * Exemples:
 *   int dims1[] = {10};        createArray(interp, "A", dims1, 1);  // A(10)
 *   int dims2[] = {5, 8};      createArray(interp, "M", dims2, 2);  // M(5,8)
 *   int dims3[] = {3, 4, 5};   createArray(interp, "T", dims3, 3);  // T(3,4,5)
 */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims);

/**
 * Définit la valeur d'un élément d'un tableau.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom du tableau
 * @param indices Tableau des indices pour chaque dimension (base 0)
 * @param numIndices Nombre d'indices (doit correspondre au nombre de dimensions)
 * @param value Valeur à assigner
 * 
 * Exemples:
 *   int idx1[] = {5};          setArrayElement(interp, "A", idx1, 1, 42.0);
 *   int idx2[] = {2, 3};       setArrayElement(interp, "M", idx2, 2, 99.0);
 *   int idx3[] = {1, 2, 1};    setArrayElement(interp, "T", idx3, 3, 123.0);
 */
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value);

/**
 * Récupère la valeur d'un élément d'un tableau.
 * 
 * @param interp Pointeur vers l'interpréteur
 * @param name Nom du tableau
 * @param indices Tableau des indices pour chaque dimension (base 0)
 * @param numIndices Nombre d'indices (doit correspondre au nombre de dimensions)
 * @return Valeur de l'élément, ou 0.0 en cas d'erreur
 * 
 * Note: En cas d'indice hors bornes, un message d'erreur est affiché
 *       et la fonction retourne 0.0.
 */
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);

#endif /* VARIABLES_H */
