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

/* Fonctions de gestion des variables */
Variable* findVariable(Interpreter *interp, const char *name);
void setVariable(Interpreter *interp, const char *name, double value);
double getVariable(Interpreter *interp, const char *name);
void setStringVariable(Interpreter *interp, const char *name, const char *value);
char* getStringVariable(Interpreter *interp, const char *name);

/* Fonctions pour les tableaux */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims);
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value);
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);

#endif /* VARIABLES_H */
