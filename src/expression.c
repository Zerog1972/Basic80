/*
 * expression.c - Expression evaluator for the Basic80 interpreter
 *
 * Implements a recursive-descent parser that evaluates numeric and
 * string expressions found in BASIC source lines.  The hierarchy is:
 *   evaluateExpression  (+/-)
 *     evaluateTerm      (mul/div)  
 *       evaluateFactor  (literals, variables, built-in functions)
 *
 * String expressions are handled separately through
 * evaluateStringExpression / evaluateStringPrimary.
 */
#include "expression.h"
#include "interpreter.h"
#include "variables.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* Pi constant shared by DEG and RAD handlers */
#define BASIC_PI 3.14159265358979323846

/* Returns 1 if the variable name ends with '$' (string variable convention) */
static int isStringVariable(const char *name) {
    size_t len = strlen(name);
    return len > 0 && name[len - 1] == '$';
}

/* Evaluate a boolean condition expression (used by the IF statement) */
int evaluateCondition(Interpreter *interp, Token *tokens, int *pos) {
    double left, right;
    BasicTokenType op;
    
    left = evaluateExpression(interp, tokens, pos);
    
    if (tokens[*pos].type == TOK_LT || tokens[*pos].type == TOK_GT ||
        tokens[*pos].type == TOK_LE || tokens[*pos].type == TOK_GE ||
        tokens[*pos].type == TOK_EQUALS || tokens[*pos].type == TOK_NE) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateExpression(interp, tokens, pos);
        
        switch (op) {
            case TOK_LT: return left < right;
            case TOK_GT: return left > right;
            case TOK_LE: return left <= right;
            case TOK_GE: return left >= right;
            case TOK_EQUALS: return left == right;
            case TOK_NE: return left != right;
            default: return 0;
        }
    }
    
    /* No relational operator found: treat as true if non-zero */
    return left != 0;
}

/* Evaluate a factor: number literal, variable, array access, or built-in function */
double evaluateFactor(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    char varName[256];
    int indices[10]; /* Maximum 10 dimensions */
    int numIndices;
    double arg;
    
    result = 0;
    
    if (tokens[*pos].type == TOK_NUMBER) {
        result = atof(tokens[*pos].value);
        (*pos)++;
    } else if (tokens[*pos].type == TOK_IDENTIFIER) {
        CustomNumericFunction customFunc;
        
        strcpy(varName, tokens[*pos].value);
        
        /* Check whether this identifier names a custom numeric function */
        customFunc = findCustomNumericFunction(interp, varName);
        if (customFunc) {
            (*pos)++; /* Passer le nom de la fonction */
            if (tokens[*pos].type == TOK_LPAREN) {
                (*pos)++; /* Passer la parenthèse ouvrante */
                result = customFunc(interp, tokens, pos);
                if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            }
        } else {
            /* Regular variable or array element */
            if (isStringVariable(varName)) {
                reportErrorEx(interp, ERR_TYPE_MISMATCH, *pos, "String variable used in a numeric expression.");
                (*pos)++; /* Skip variable to avoid infinite loop if caller retries? Or just return 0 */
                return 0.0;
            }

            (*pos)++;
            
            /* Check if this is an indexed array access */
            if (tokens[*pos].type == TOK_LPAREN) {
                (*pos)++;
                numIndices = 0;
                /* Read all comma-separated indices */
                while (numIndices < 10) {
                    indices[numIndices] = (int)evaluateExpression(interp, tokens, pos);
                    numIndices++;
                    if (tokens[*pos].type == TOK_COMMA) {
                        (*pos)++; /* Advance past the comma */
                    } else {
                        break; /* No more indices */
                    }
                }
                if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
                result = getArrayElement(interp, varName, indices, numIndices);
            } else {
                result = getVariable(interp, varName);
            }
        }
    } else if (tokens[*pos].type == TOK_SIN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = sin(arg);
        }
    } else if (tokens[*pos].type == TOK_COS) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = cos(arg);
        }
    } else if (tokens[*pos].type == TOK_TAN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = tan(arg);
        }
    } else if (tokens[*pos].type == TOK_ATAN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = atan(arg);
        }
    } else if (tokens[*pos].type == TOK_ATN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = atan(arg);
        }
    } else if (tokens[*pos].type == TOK_ASIN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = asin(arg);
        }
    } else if (tokens[*pos].type == TOK_ACOS) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = acos(arg);
        }
    } else if (tokens[*pos].type == TOK_SINH) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = sinh(arg);
        }
    } else if (tokens[*pos].type == TOK_COSH) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = cosh(arg);
        }
    } else if (tokens[*pos].type == TOK_TANH) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = tanh(arg);
        }
    } else if (tokens[*pos].type == TOK_SQR) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = sqrt(arg);
        }
    } else if (tokens[*pos].type == TOK_ABS) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = fabs(arg);
        }
    } else if (tokens[*pos].type == TOK_INT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = floor(arg);
        }
    } else if (tokens[*pos].type == TOK_SGN) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            /* SGN returns -1, 0, or +1 depending on the sign of the argument */
            if (arg < 0) result = -1.0;
            else if (arg > 0) result = 1.0;
            else result = 0.0;
        }
    } else if (tokens[*pos].type == TOK_RND) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            /* RND(n) returns a random integer in [0, n-1] */
            result = (double)(rand() % (int)arg);
        } else {
            /* RND without parentheses returns a float in [0.0, 1.0) */
            result = (double)rand() / (double)RAND_MAX;
        }
    } else if (tokens[*pos].type == TOK_LOG) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = log(arg);
        }
    } else if (tokens[*pos].type == TOK_EXP) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = exp(arg);
        }
    } else if (tokens[*pos].type == TOK_LOG10) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = log10(arg);
        }
    } else if (tokens[*pos].type == TOK_POW) {
        double base, exponent;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            base = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            exponent = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = pow(base, exponent);
        }
    } else if (tokens[*pos].type == TOK_DEG) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            /* Convert radians to degrees */
            result = arg * 180.0 / BASIC_PI;
        }
    } else if (tokens[*pos].type == TOK_RAD) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            arg = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            /* Convert degrees to radians */
            result = arg * BASIC_PI / 180.0;
        }
    } else if (tokens[*pos].type == TOK_LEN) {
        /* LEN(string) - returns the number of characters in a string */
        char varName[256];
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            if (tokens[*pos].type == TOK_STRING) {
                result = (double)strlen(tokens[*pos].value);
                (*pos)++;
            } else if (tokens[*pos].type == TOK_IDENTIFIER) {
                strcpy(varName, tokens[*pos].value);
                (*pos)++;
                result = (double)strlen(getStringVariable(interp, varName));
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
        }
    } else if (tokens[*pos].type == TOK_ASC) {
        /* ASC(string) - returns the ASCII code of the first character */
        char varName[256];
        char *str;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            if (tokens[*pos].type == TOK_STRING) {
                str = tokens[*pos].value;
                if (str[0] != '\0') {
                    result = (double)((unsigned char)str[0]);
                }
                (*pos)++;
            } else if (tokens[*pos].type == TOK_IDENTIFIER) {
                strcpy(varName, tokens[*pos].value);
                (*pos)++;
                str = getStringVariable(interp, varName);
                if (str[0] != '\0') {
                    result = (double)((unsigned char)str[0]);
                }
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
        }
    } else if (tokens[*pos].type == TOK_VAL) {
        /* VAL(string) - converts a string to its numeric value */
        char varName[256];
        char *str;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            if (tokens[*pos].type == TOK_STRING) {
                str = tokens[*pos].value;
                result = atof(str);
                (*pos)++;
            } else if (tokens[*pos].type == TOK_IDENTIFIER) {
                strcpy(varName, tokens[*pos].value);
                (*pos)++;
                str = getStringVariable(interp, varName);
                result = atof(str);
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
        }
    } else if (tokens[*pos].type == TOK_LPAREN) {
        (*pos)++;
        result = evaluateExpression(interp, tokens, pos);
        if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
    }
    
    return result;
}

/* Evaluate a term: handles multiplication and division */
double evaluateTerm(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    BasicTokenType op;
    double right;
    
    result = evaluateFactor(interp, tokens, pos);
    
    while (tokens[*pos].type == TOK_MULTIPLY || tokens[*pos].type == TOK_DIVIDE) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateFactor(interp, tokens, pos);
        
        switch (op) {
            case TOK_MULTIPLY: result *= right; break;
            case TOK_DIVIDE: 
                if (right != 0) {
                    result /= right;
                } else {
                    reportErrorEx(interp, ERR_DIVISION_ZERO, *pos - 1, "Division par zéro.");
                }
                break;
            default: break;
        }
    }
    
    return result;
}

/* Evaluate a string primary: string literal, variable, or string function call */
char* evaluateStringPrimary(Interpreter *interp, Token *tokens, int *pos) {
    char *result = NULL;
    char varName[256];
    char *str;
    int start, length, n;
    int strLen;

    if (tokens[*pos].type == TOK_NUMBER) {
        /* Convert a numeric literal to its string representation */
        double numValue = atof(tokens[*pos].value);
        result = (char*)malloc(50);
        if (result) {
            sprintf(result, "%.2f", numValue);
        } else {
            result = (char*)malloc(1);
            if (result) result[0] = '\0';
        }
        (*pos)++;
    } else if (tokens[*pos].type == TOK_LPAREN) {
        /* Parenthesized expression - evaluate numerically and convert to string */
        double numValue = evaluateExpression(interp, tokens, pos);
        result = (char*)malloc(50);
        if (result) {
            sprintf(result, "%.2f", numValue);
        } else {
            result = (char*)malloc(1);
            if (result) result[0] = '\0';
        }
    } else if (tokens[*pos].type == TOK_STRING) {
        size_t vlen = strlen(tokens[*pos].value);
        result = (char*)malloc(vlen + 1);
        if (result) {
            memcpy(result, tokens[*pos].value, vlen + 1);
        } else {
            result = (char*)malloc(1);
            if (result) result[0] = '\0';
        }
        (*pos)++;;
    } else if (tokens[*pos].type == TOK_IDENTIFIER) {
        CustomStringFunction customFunc;
        
        strcpy(varName, tokens[*pos].value);
        
        /* Check whether this identifier names a custom string function */
        customFunc = findCustomStringFunction(interp, varName);
        if (customFunc) {
            (*pos)++; /* Passer le nom de la fonction */
            if (tokens[*pos].type == TOK_LPAREN) {
                (*pos)++; /* Passer la parenthèse ouvrante */
                result = customFunc(interp, tokens, pos);
                if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            }
        } else {
            /* Regular string variable or numeric variable coerced to string */
            if (!isStringVariable(varName)) {
                /* Numeric variable: evaluate and convert to string */
                double numValue;
                numValue = evaluateExpression(interp, tokens, pos);
                result = (char*)malloc(50);
                if (result) {
                    sprintf(result, "%.2f", numValue);
                } else {
                    result = (char*)malloc(1);
                    if (result) result[0] = '\0';
                }
                return result;
            }

            (*pos)++;
            str = getStringVariable(interp, varName);
            result = (char*)malloc(strlen(str) + 1);
            if (result) {
                strcpy(result, str);
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
        }
    } else if (tokens[*pos].type == TOK_CHR) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            int code;
            (*pos)++;
            code = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = (char*)malloc(2);
            if (result) {
                result[0] = (char)code;
                result[1] = '\0';
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
        }
    } else if (tokens[*pos].type == TOK_MID) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            start = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            length = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (start < 1) start = 1;
            if (start > strLen) {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            } else {
                int actualLen = length;
                if (start - 1 + actualLen > strLen) {
                    actualLen = strLen - (start - 1);
                }
                result = (char*)malloc(actualLen + 1);
                if (result) {
                    strncpy(result, str + (start - 1), actualLen);
                    result[actualLen] = '\0';
                } else {
                    result = (char*)malloc(1);
                    if (result) result[0] = '\0';
                }
            }
            free(str);
        }
    } else if (tokens[*pos].type == TOK_LEFT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (n > strLen) n = strLen;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                strncpy(result, str, n);
                result[n] = '\0';
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
            free(str);
        }
    } else if (tokens[*pos].type == TOK_RIGHT) {
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            str = evaluateStringExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;

            strLen = (int)strlen(str);
            if (n > strLen) n = strLen;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                strcpy(result, str + strLen - n);
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
            free(str);
        }
    } else if (tokens[*pos].type == TOK_STR) {
        /* STR$(x) - converts a number to its string representation */
        double numValue;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            numValue = evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            result = (char*)malloc(50);
            if (result) {
                sprintf(result, "%.10g", numValue);
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
        }
    } else if (tokens[*pos].type == TOK_SPACE) {
        /* SPACE$(n) - returns a string of n space characters */
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                memset(result, ' ', n);
                result[n] = '\0';
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
        }
    } else if (tokens[*pos].type == TOK_STRING_FUNC) {
        /* STRING$(n,c) - returns a string of n repetitions of character c */
        char c;
        (*pos)++;
        if (tokens[*pos].type == TOK_LPAREN) {
            (*pos)++;
            n = (int)evaluateExpression(interp, tokens, pos);
            if (tokens[*pos].type == TOK_COMMA) (*pos)++;
            /* The character argument may be an ASCII code or a string literal */
            if (tokens[*pos].type == TOK_NUMBER) {
                c = (char)((int)evaluateExpression(interp, tokens, pos));
            } else if (tokens[*pos].type == TOK_STRING) {
                c = tokens[*pos].value[0];
                (*pos)++;
            } else {
                c = ' ';
            }
            if (tokens[*pos].type == TOK_RPAREN) (*pos)++;
            if (n < 0) n = 0;
            result = (char*)malloc(n + 1);
            if (result) {
                memset(result, (unsigned char)c, n);
                result[n] = '\0';
            } else {
                result = (char*)malloc(1);
                if (result) result[0] = '\0';
            }
        }
    }

    if (!result) {
        result = (char*)malloc(1);
        if (result) result[0] = '\0';
    }

    return result;
}

/* Evaluate a string expression, handling + concatenation */
char* evaluateStringExpression(Interpreter *interp, Token *tokens, int *pos) {
    char *result;

    result = evaluateStringPrimary(interp, tokens, pos);
    while (tokens[*pos].type == TOK_PLUS) {
        char *rhs;
        char *concat;
        size_t llen, rlen;
        (*pos)++;
        rhs = evaluateStringPrimary(interp, tokens, pos);
        llen = strlen(result);
        rlen = strlen(rhs);
        concat = (char*)malloc(llen + rlen + 1);
        if (concat) {
            memcpy(concat, result, llen);
            memcpy(concat + llen, rhs, rlen + 1); /* +1 copies the '\0' */
        }
        free(result);
        free(rhs);
        result = concat ? concat : (char*)malloc(1);
        if (!result) {
            result = (char*)malloc(1);
            if (result) result[0] = '\0';
        }
    }

    return result;
}

/* Return 1 if the token at pos starts a string-valued expression */
int isStringExpression(Interpreter *interp, Token *tokens, int pos) {
    Variable *var;
    if (tokens[pos].type == TOK_IDENTIFIER) {
        var = findVariable(interp, tokens[pos].value);
        if (var && var->isString) {
            return 1;
        }
    }
    return tokens[pos].type == TOK_STRING ||
           tokens[pos].type == TOK_CHR ||
           tokens[pos].type == TOK_MID ||
           tokens[pos].type == TOK_LEFT ||
           tokens[pos].type == TOK_RIGHT ||
           tokens[pos].type == TOK_STR ||
           tokens[pos].type == TOK_SPACE ||
           tokens[pos].type == TOK_STRING_FUNC;
}

/* Evaluate a full numeric expression: handles addition and subtraction */
double evaluateExpression(Interpreter *interp, Token *tokens, int *pos) {
    double result;
    BasicTokenType op;
    double right;
    
    result = evaluateTerm(interp, tokens, pos);
    
    while (tokens[*pos].type == TOK_PLUS || tokens[*pos].type == TOK_MINUS) {
        op = tokens[*pos].type;
        (*pos)++;
        right = evaluateTerm(interp, tokens, pos);
        
        switch (op) {
            case TOK_PLUS: result += right; break;
            case TOK_MINUS: result -= right; break;
            default: break;
        }
    }
    
    return result;
}
