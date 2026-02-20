/*
 * variables.c - Variable storage and array management for Basic80
 *
 * Implements a singly-linked list of Variable nodes that holds both
 * scalar values (numeric or string) and multi-dimensional numeric
 * arrays.  All functions operate through an Interpreter pointer so
 * that the variable table stays encapsulated inside the interpreter
 * state.
 */
#include "variables.h"
#include "interpreter.h"
#include <stdlib.h>
#include <string.h>

/* Compute the flat (row-major) index from multi-dimensional indices.
 * Returns -1 if any index is out of bounds. */
static int computeFlatIndex(Variable *var, int *indices, int numIndices) {
    int flatIndex = 0;
    int i, j, multiplier;
    for (i = 0; i < numIndices; i++) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return -1; /* Out of bounds */
        }
        multiplier = 1;
        for (j = i + 1; j < numIndices; j++) {
            multiplier *= var->dimensions[j];
        }
        flatIndex += indices[i] * multiplier;
    }
    return flatIndex;
}

/* Find a variable by name; returns NULL if not found */
Variable* findVariable(Interpreter *interp, const char *name) {
    Variable *var = interp->variables;
    while (var) {
        if (strcmp(var->name, name) == 0) return var;
        var = var->next;
    }
    return NULL;
}

/* Set a numeric variable, creating it if it does not exist yet */
void setVariable(Interpreter *interp, const char *name, double value) {
    Variable *var;
    Variable *newVar;
    size_t nameLen;
    
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
        nameLen = strlen(name);  /* Calculé une seule fois */
        newVar = malloc(sizeof(Variable));
        newVar->name = malloc(nameLen + 1);
        memcpy(newVar->name, name, nameLen + 1);
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

/* Get the numeric value of a variable; returns 0.0 if not defined */
double getVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && !var->isArray && !var->isString) {
        return var->value;
    }
    return 0.0;
}

/* Set a string variable, creating it if it does not exist yet */
void setStringVariable(Interpreter *interp, const char *name, const char *value) {
    Variable *var;
    Variable *newVar;
    size_t nameLen;
    size_t valueLen;
    
    var = findVariable(interp, name);
    if (var) {
        if (!var->isArray) {
            valueLen = strlen(value);  /* Calculé une seule fois */
            if (var->strValue) {
                free(var->strValue);
            }
            var->strValue = malloc(valueLen + 1);
            if (!var->strValue) {
                var->strValue = NULL;
                return;
            }
            memcpy(var->strValue, value, valueLen + 1);
            var->isString = 1;
        }
    } else {
        nameLen  = strlen(name);   /* Calculé une seule fois */
        valueLen = strlen(value);  /* Calculé une seule fois */
        newVar = malloc(sizeof(Variable));
        if (!newVar) return;
        newVar->name = malloc(nameLen + 1);
        if (!newVar->name) {
            free(newVar);
            return;
        }
        memcpy(newVar->name, name, nameLen + 1);
        newVar->value = 0.0;
        newVar->isString = 1;
        newVar->strValue = malloc(valueLen + 1);
        if (!newVar->strValue) {
            free(newVar->name);
            free(newVar);
            return;
        }
        memcpy(newVar->strValue, value, valueLen + 1);
        newVar->isArray = 0;
        newVar->arrayValues = NULL;
        newVar->arraySize = 0;
        newVar->numDimensions = 0;
        newVar->dimensions = NULL;
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Get the string value of a variable; returns "" if not defined */
char* getStringVariable(Interpreter *interp, const char *name) {
    Variable *var = findVariable(interp, name);
    if (var && var->isString && var->strValue) {
        return var->strValue;
    }
    return "";
}

/* Create a multi-dimensional numeric array (all elements initialised to 0.0) */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims) {
    Variable *var;
    Variable *newVar;
    int i;
    int totalSize;
    size_t nameLen;
    
    /* Compute the total number of elements (product of all dimension sizes) */
    totalSize = 1;
    for (i = 0; i < numDims; i++) {
        totalSize *= dims[i];
    }
    
    var = findVariable(interp, name);
    if (var) {
        /* Variable already exists: replace its storage with a new array */
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
        memcpy(var->dimensions, dims, sizeof(int) * numDims);
        /* calloc initialises to zero bytes (0.0 for IEEE 754 doubles) */
        var->arrayValues = calloc(totalSize, sizeof(double));
        if (!var->arrayValues) {
            free(var->dimensions);
            var->dimensions = NULL;
            var->isArray = 0;
            return;
        }
    } else {
        /* Variable does not exist: allocate a brand-new array variable */
        nameLen = strlen(name);  /* Calculé une seule fois */
        newVar = malloc(sizeof(Variable));
        if (!newVar) return;
        newVar->name = malloc(nameLen + 1);
        if (!newVar->name) {
            free(newVar);
            return;
        }
        memcpy(newVar->name, name, nameLen + 1);
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
        memcpy(newVar->dimensions, dims, sizeof(int) * numDims);
        /* calloc initialises to zero bytes (0.0 for IEEE 754 doubles) */
        newVar->arrayValues = calloc(totalSize, sizeof(double));
        if (!newVar->arrayValues) {
            free(newVar->dimensions);
            free(newVar->name);
            free(newVar);
            return;
        }
        newVar->next = interp->variables;
        interp->variables = newVar;
    }
}

/* Set the value of a single array element */
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value) {
    Variable *var;
    int flatIndex;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return;
    }
    
    flatIndex = computeFlatIndex(var, indices, numIndices);
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        var->arrayValues[flatIndex] = value;
    }
}

/* Get the value of a single array element; returns 0.0 on error */
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices) {
    Variable *var;
    int flatIndex;
    
    var = findVariable(interp, name);
    if (!var || !var->isArray || numIndices != var->numDimensions) {
        return 0.0;
    }
    
    flatIndex = computeFlatIndex(var, indices, numIndices);
    if (flatIndex >= 0 && flatIndex < var->arraySize) {
        return var->arrayValues[flatIndex];
    }
    return 0.0;
}
