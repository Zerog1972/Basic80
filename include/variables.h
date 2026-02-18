/*
 * variables.h - Variable and array management interface for Basic80
 *
 * Declares the Variable structure and all functions that create, read,
 * and write scalar variables and multi-dimensional numeric arrays.
 */
#ifndef VARIABLES_H
#define VARIABLES_H

/* Structure used to store any BASIC variable (scalar or array) */
typedef struct Variable {
    char *name;
    int isString;        /* 1 = string variable, 0 = numeric variable */
    double value;        /* Numeric value (used when isString == 0) */
    char *strValue;      /* String value  (used when isString == 1) */
    int isArray;
    double *arrayValues;
    int arraySize;       /* Total element count (product of all dimension sizes) */
    int numDimensions;   /* Number of dimensions (1, 2, 3, ...) */
    int *dimensions;     /* Size of each individual dimension */
    struct Variable *next;
} Variable;

/* Forward declaration for Interpreter (defined in interpreter.h) */
typedef struct Interpreter Interpreter;

/* ===== SCALAR VARIABLE FUNCTIONS ===== */

/**
 * Search for a variable by name.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Variable name to look up (case-sensitive)
 * @return Pointer to the Variable if found, NULL otherwise.
 *
 * Note: This function does NOT create a new variable when the name is absent.
 */
Variable* findVariable(Interpreter *interp, const char *name);

/**
 * Set a numeric variable, creating it if it does not already exist.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Variable name (e.g. "A", "X", "COUNT")
 * @param value   Numeric value to assign
 *
 * Examples:
 *   setVariable(interp, "A", 42.0);
 *   setVariable(interp, "PI", 3.14159);
 */
void setVariable(Interpreter *interp, const char *name, double value);

/**
 * Get the numeric value of a variable.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Variable name to read
 * @return Current value, or 0.0 if the variable does not exist.
 *
 * Note: Undefined variables return 0.0 (standard BASIC behaviour).
 */
double getVariable(Interpreter *interp, const char *name);

/**
 * Set a string variable, creating it if it does not already exist.
 *
 * The string is duplicated in heap memory.
 *
 * @param interp  Pointer to the interpreter
 * @param name    Variable name (must end with '$', e.g. "A$", "NAME$")
 * @param value   String value to assign
 *
 * Examples:
 *   setStringVariable(interp, "A$", "Hello");
 *   setStringVariable(interp, "NAME$", "John Doe");
 */
void setStringVariable(Interpreter *interp, const char *name, const char *value);

/**
 * Get the string value of a variable.
 *
 * @param interp  Pointer to the interpreter
 * @param name    String variable name (must end with '$')
 * @return Current string value, or "" if the variable does not exist.
 *
 * Note: The returned pointer must NOT be freed by the caller.
 *       Undefined string variables return "" (standard BASIC behaviour).
 */
char* getStringVariable(Interpreter *interp, const char *name);

/* ===== ARRAY FUNCTIONS ===== */

/**
 * Create a multi-dimensional numeric array (all elements initialised to 0.0).
 *
 * Supports up to 10 dimensions.
 *
 * @param interp    Pointer to the interpreter
 * @param name      Array name (e.g. "A", "M", "T")
 * @param dims      Array containing the size of each dimension
 * @param numDims   Number of dimensions (1..10)
 *
 * Examples:
 *   int dims1[] = {10};        createArray(interp, "A", dims1, 1);  // A(10)
 *   int dims2[] = {5, 8};      createArray(interp, "M", dims2, 2);  // M(5,8)
 *   int dims3[] = {3, 4, 5};   createArray(interp, "T", dims3, 3);  // T(3,4,5)
 */
void createArray(Interpreter *interp, const char *name, int *dims, int numDims);

/**
 * Set the value of an array element.
 *
 * @param interp      Pointer to the interpreter
 * @param name        Array name
 * @param indices     Index for each dimension (0-based)
 * @param numIndices  Number of indices (must match the number of dimensions)
 * @param value       Value to assign
 *
 * Examples:
 *   int idx1[] = {5};          setArrayElement(interp, "A", idx1, 1, 42.0);
 *   int idx2[] = {2, 3};       setArrayElement(interp, "M", idx2, 2, 99.0);
 *   int idx3[] = {1, 2, 1};    setArrayElement(interp, "T", idx3, 3, 123.0);
 */
void setArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices, double value);

/**
 * Get the value of an array element.
 *
 * @param interp      Pointer to the interpreter
 * @param name        Array name
 * @param indices     Index for each dimension (0-based)
 * @param numIndices  Number of indices (must match the number of dimensions)
 * @return Value of the element, or 0.0 on error.
 *
 * Note: An out-of-bounds index triggers an error message and returns 0.0.
 */
double getArrayElement(Interpreter *interp, const char *name, int *indices, int numIndices);

#endif /* VARIABLES_H */
