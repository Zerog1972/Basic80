/*
 * lexer.h - Public interface for the Basic80 lexical analyser
 *
 * Declares the token type enumeration, the Token structure, and the two
 * public functions  tokenize() / freeTokens().
 */
#ifndef LEXER_H
#define LEXER_H

/* Token types recognised by the Basic80 lexer */
typedef enum {
    TOK_PRINT,
    TOK_LET,
    TOK_IF,
    TOK_THEN,
    TOK_ELSE,
    TOK_GOTO,
    TOK_GOSUB,
    TOK_RETURN,
    TOK_INPUT,
    TOK_DIM,
    TOK_FOR,
    TOK_TO,
    TOK_STEP,
    TOK_NEXT,
    TOK_END,
    TOK_REM,
    TOK_READ,
    TOK_DATA,
    TOK_RESTORE,
    TOK_SAVE,
    TOK_LOAD,
    TOK_HELP,
    TOK_CLS,
    TOK_SIN,
    TOK_COS,
    TOK_TAN,
    TOK_ATAN,
    TOK_ASIN,
    TOK_ACOS,
    TOK_SINH,
    TOK_COSH,
    TOK_TANH,
    TOK_SQR,
    TOK_ABS,
    TOK_INT,
    TOK_RND,
    TOK_LOG,
    TOK_EXP,
    TOK_POW,
    TOK_LOG10,
    TOK_DEG,
    TOK_RAD,
    TOK_SGN,
    TOK_ATN,
    TOK_LEN,
    TOK_MID,
    TOK_LEFT,
    TOK_RIGHT,
    TOK_CHR,
    TOK_ASC,
    TOK_STR,
    TOK_VAL,
    TOK_SPACE,
    TOK_STRING_FUNC,
    TOK_NUMBER,
    TOK_IDENTIFIER,
    TOK_STRING,
    TOK_EQUALS,
    TOK_PLUS,
    TOK_MINUS,
    TOK_MULTIPLY,
    TOK_DIVIDE,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_NE,
    TOK_COMMA,
    TOK_SEMICOLON,
    TOK_COLON,
    TOK_NEWLINE,
    TOK_EOF
} BasicTokenType;

/* Structure representing a single lexical token */
typedef struct {
    BasicTokenType type;
    char *value;
    int lineNum;
} Token;

/* ===== LEXER FUNCTIONS ===== */

/**
 * Tokenize a BASIC source line into an array of tokens.
 *
 * Performs lexical analysis on a raw BASIC source line and converts it
 * into a sequence of typed tokens.  Recognises keywords, identifiers,
 * numbers, string literals, operators, and punctuation.
 *
 * @param line  NUL-terminated source string to analyse
 * @return Heap-allocated token array terminated by a TOK_EOF sentinel.
 *         Returns NULL on allocation failure.
 *         The array must be freed with freeTokens().
 *
 * Examples:
 *   "PRINT 42"        -> [TOK_PRINT, TOK_NUMBER("42"), TOK_EOF]
 *   "LET A = 10 + 5"  -> [TOK_LET, TOK_IDENTIFIER("A"), TOK_EQUALS, ...]
 *   "IF X > 5 THEN Y = 1" -> [TOK_IF, TOK_IDENTIFIER("X"), TOK_GT, ...]
 */
Token* tokenize(const char *line);

/**
 * Free all memory allocated for a token array produced by tokenize().
 *
 * Releases the string value of every token as well as the array itself.
 *
 * @param tokens  Token array to free (may be NULL)
 *
 * Note: After this call the pointer becomes invalid.
 */
void freeTokens(Token *tokens);

#endif /* LEXER_H */
