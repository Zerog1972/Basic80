/*
 * variables.c - Variable storage and array management for Basic80
 *
 * Implements a singly-linked list of Variable nodes that holds both
 * scalar values (numeric or string) and multi-dimensional numeric
 * arrays.  All functions operate through an Interpreter pointer so
 * that the variable table stays encapsulated inside the interpreter
 * state.
 */
#include "vars.h"
#include "interp.h"
#include <stdlib.h>
#include <string.h>

static unsigned int hashVariableName(const char *name) {
    unsigned int hash;
    unsigned char ch;

    hash = 5381U;
    while (*name) {
        ch = (unsigned char)*name;
        hash = ((hash << 5) + hash) + (unsigned int)ch;
        name++;
    }
    return hash % BASIC80_VAR_BUCKETS;
}

static void linkVariable(Interpreter *interp, Variable *var) {
    unsigned int bucket;

    bucket = hashVariableName(var->name);
    var->next = interp->variables;
    interp->variables = var;
    var->hashNext = interp->variableBuckets[bucket];
    interp->variableBuckets[bucket] = var;
}

static Variable* createVariableNode(const char *name) {
    Variable *newVar;
    size_t nameLen;

    nameLen = strlen(name);
    newVar = malloc(sizeof(Variable));
    if (!newVar) return NULL;

    newVar->name = malloc(nameLen + 1);
    if (!newVar->name) {
        free(newVar);
        return NULL;
    }

    memcpy(newVar->name, name, nameLen + 1);
    newVar->isString = 0;
    newVar->value = 0.0;
    newVar->strValue = NULL;
    newVar->isArray = 0;
    newVar->arrayValues = NULL;
    newVar->arraySize = 0;
    newVar->numDimensions = 0;
    newVar->dimensions = NULL;
    newVar->strides = NULL;
    newVar->next = NULL;
    newVar->hashNext = NULL;
    return newVar;
}

static int allocateArrayMetadata(Variable *var, int *dims, int numDims) {
    int index;

    var->dimensions = malloc(sizeof(int) * numDims);
    if (!var->dimensions) {
        return 0;
    }

    var->strides = malloc(sizeof(int) * numDims);
    if (!var->strides) {
        free(var->dimensions);
        var->dimensions = NULL;
        return 0;
    }

    memcpy(var->dimensions, dims, sizeof(int) * numDims);

    if (numDims > 0) {
        var->strides[numDims - 1] = 1;
        for (index = numDims - 2; index >= 0; index--) {
            var->strides[index] = var->strides[index + 1] * var->dimensions[index + 1];
        }
    }

    return 1;
}

/* Compute the flat (row-major) index from multi-dimensional indices.
 * Returns -1 if any index is out of bounds. */
static int computeFlatIndex(Variable *var, int *indices, int numIndices) {
    int flatIndex = 0;
    int i;
    for (i = 0; i < numIndices; i++) {
        if (indices[i] < 0 || indices[i] >= var->dimensions[i]) {
            return -1; /* Out of bounds */
        }
        flatIndex += indices[i] * var->strides[i];
    }
    return flatIndex;
}

/* Find a variable by name; returns NULL if not found */
Variable* findVariable(Interpreter *interp, const char *name) {
    Variable *var;
    unsigned int bucket;

    bucket = hashVariableName(name);
    var = interp->variableBuckets[bucket];
    while (var) {
        if (strcmp(var->name, name) == 0) return var;
        var = var->hashNext;
    }
    return NULL;
}

/* Set a numeric variable, creating it if it does not exist yet */
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
        newVar = createVariableNode(name);
        if (!newVar) return;
        newVar->value = value;
        linkVariable(interp, newVar);
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
        valueLen = strlen(value);  /* Calculé une seule fois */
        newVar = createVariableNode(name);
        if (!newVar) return;
        newVar->isString = 1;
        newVar->strValue = malloc(valueLen + 1);
        if (!newVar->strValue) {
            free(newVar->name);
            free(newVar);
            return;
        }
        memcpy(newVar->strValue, value, valueLen + 1);
        linkVariable(interp, newVar);
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
        if (var->strides) {
            free(var->strides);
        }
        var->isArray = 1;
        var->arraySize = totalSize;
        var->numDimensions = numDims;
        var->dimensions = NULL;
        var->strides = NULL;
        if (!allocateArrayMetadata(var, dims, numDims)) {
            var->isArray = 0;
            return;
        }
        /* calloc initialises to zero bytes (0.0 for IEEE 754 doubles) */
        var->arrayValues = calloc(totalSize, sizeof(double));
        if (!var->arrayValues) {
            free(var->dimensions);
            free(var->strides);
            var->dimensions = NULL;
            var->strides = NULL;
            var->isArray = 0;
            return;
        }
    } else {
        /* Variable does not exist: allocate a brand-new array variable */
        newVar = createVariableNode(name);
        if (!newVar) return;
        newVar->isArray = 1;
        newVar->arraySize = totalSize;
        newVar->numDimensions = numDims;
        if (!allocateArrayMetadata(newVar, dims, numDims)) {
            free(newVar->name);
            free(newVar);
            return;
        }
        /* calloc initialises to zero bytes (0.0 for IEEE 754 doubles) */
        newVar->arrayValues = calloc(totalSize, sizeof(double));
        if (!newVar->arrayValues) {
            free(newVar->dimensions);
            free(newVar->strides);
            free(newVar->name);
            free(newVar);
            return;
        }
        linkVariable(interp, newVar);
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
