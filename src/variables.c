#include "variables.h"
#include "interpreter.h"
#include <stdlib.h>
#include <string.h>

/* Trouver une variable par son nom */
Variable* findVariable(Interpreter *interp, const char *name) {
    Variable *var = interp->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) return var;
        var = var->next;
    }
    return NULL;
}

/* Définir une variable numérique */
void setVariable(Interpreter *interp, const char *name, double value) {
    Variable *var;
    Variable *newVar;
    
    var = findVariable(interp, name);
    if (var) {
        if (!var->isArray) {
            var->value = value;
            var->isString = 0;
            if (var->strValue) {
                free(var->strValue);
                var->strValue = NULL;
            }
        }
    } else {
        newVar = malloc(sizeof(Variable));
        newVar->name = malloc(strlen(name) + 1);
        strcpy(newVar->name, name);
        newVar->value = value;
        newVar->isString = 0;
        newVar->strValue = NULL;
        newVar->isArray = 0;
        newVar->arrayValues = NULL;
        newVar->arraySize = 0;
        newVar->numDimensions = 0;
        newVar->dimensions = NULL;
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Obtenir la valeur d'une variable numérique */
double getVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && !var->isArray && !var->isString) {
        return var->value;
    }
    return 0.0;
}

/* Définir une variable chaîne */
void setStringVariable(Interpreter *interp, const char *name, const char *value) {
    Variable *var;
    Variable *newVar;
    
    var = findVariable(interp, name);
    if (var) {
        if (!var->isArray) {
            if (var->strValue) {
                free(var->strValue);
            }
            var->strValue = malloc(strlen(value) + 1);
            if (!var->strValue) {
                var->strValue = NULL;
                return;
            }
            strcpy(var->strValue, value);
            var->isString = 1;
        }
    } else {
        newVar = malloc(sizeof(Variable));
        if (!newVar) return;
        newVar->name = malloc(strlen(name) + 1);
        if (!newVar->name) {
            free(newVar);
            return;
        }
        strcpy(newVar->name, name);
        newVar->value = 0.0;
        newVar->isString = 1;
        newVar->strValue = malloc(strlen(value) + 1);
        if (!newVar->strValue) {
            free(newVar->name);
            free(newVar);
            return;
        }
        strcpy(newVar->strValue, value);
        newVar->isArray = 0;
        newVar->arrayValues = NULL;
        newVar->arraySize = 0;
        newVar->numDimensions = 0;
        newVar->dimensions = NULL;
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Obtenir une variable chaîne */
char* getStringVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && var->isString && var->strValue) {
        return var->strValue;
    }
    return "";
}

/* Créer un tableau multi-dimensionnel */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims) {
    Variable *var;
    Variable *newVar;
    int i;
    int totalSize;
    
    /* Calculer la taille totale */
    totalSize = 1;
    for (i = 0; i < numDims; i++) {
        totalSize *= dims[i];
    }
    
    var = findVariable(interp, name);
    if (var) {
        /* Si la variable existe déjà, la transformer en tableau */
        if (var->arrayValues) {
            free(var->arrayValues);
        }
        if (var->dimensions) {
            free(var->dimensions);
        }
        var->isArray = 1;
        var->arraySize = totalSize;
        var->numDimensions = numDims;
        var->dimensions = malloc(sizeof(int) * numDims);
        if (!var->dimensions) {
            var->isArray = 0;
            return;
        }
        for (i = 0; i < numDims; i++) {
            var->dimensions[i] = dims[i];
        }
        var->arrayValues = malloc(sizeof(double) * totalSize);
        if (!var->arrayValues) {
            free(var->dimensions);
            var->dimensions = NULL;
            var->isArray = 0;
            return;
        }
        for (i = 0; i < totalSize; i++) {
            var->arrayValues[i] = 0.0;
        }
    } else {
        /* Créer une nouvelle variable tableau */
        newVar = malloc(sizeof(Variable));
        if (!newVar) return;
        newVar->name = malloc(strlen(name) + 1);
        if (!newVar->name) {
            free(newVar);
            return;
        }
        strcpy(newVar->name, name);
        newVar->value = 0.0;
        newVar->isString = 0;
        newVar->strValue = NULL;
        newVar->isArray = 1;
        newVar->arraySize = totalSize;
        newVar->numDimensions = numDims;
        newVar->dimensions = malloc(sizeof(int) * numDims);
        if (!newVar->dimensions) {
            free(newVar->name);
            free(newVar);
            return;
        }
        for (i = 0; i < numDims; i++) {
            newVar->dimensions[i] = dims[i];
        }
        newVar->arrayValues = malloc(sizeof(double) * totalSize);
        if (!newVar->arrayValues) {
            free(newVar->dimensions);
            free(newVar->name);
            free(newVar);
            return;
        }
        for (i = 0; i < totalSize; i++) {
            newVar->arrayValues[i] = 0.0;
        }
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Définir la valeur d'un élément de tableau */
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value) {
    Variable *var;
    int flatIndex;
    int i;
    int j;
    int multiplier;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return;
    }
    
    /* Convertir les indices multi-dimensionnels en index plat (row-major order) */
    flatIndex = 0;
    for (i = 0; i < numIndices; i++) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return; /* Index hors limites */
        }
        /* Calculer le multiplicateur pour cette dimension */
        multiplier = 1;
        for (j = i + 1; j < numIndices; j++) {
            multiplier *= var->dimensions[j];
        }
        flatIndex += indices[i] * multiplier;
    }
    
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        var->arrayValues[flatIndex] = value;
    }
}

/* Obtenir la valeur d'un élément de tableau */
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices) {
    Variable *var;
    int flatIndex;
    int i;
    int j;
    int multiplier;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return 0.0;
    }
    
    /* Convertir les indices multi-dimensionnels en index plat (row-major order) */
    flatIndex = 0;
    for (i = 0; i < numIndices; i++) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return 0.0; /* Index hors limites */
        }
        /* Calculer le multiplicateur pour cette dimension */
        multiplier = 1;
        for (j = i + 1; j < numIndices; j++) {
            multiplier *= var->dimensions[j];
        }
        flatIndex += indices[i] * multiplier;
    }
    
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        return var->arrayValues[flatIndex];
    }
    return 0.0;
}
