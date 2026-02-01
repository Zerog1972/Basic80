#ifndef LEXER_H
#define LEXER_H

/* Types de tokens */
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
    TOK_SAVE,
    TOK_LOAD,
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
    TOK_LEN,
    TOK_MID,
    TOK_LEFT,
    TOK_RIGHT,
    TOK_CHR,
    TOK_ASC,
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

/* Structure pour un token */
typedef struct {
    BasicTokenType type;
    char *value;
    int lineNum;
} Token;

/* Fonctions du lexer */
Token* tokenize(const char *line);
void freeTokens(Token *tokens);

#endif /* LEXER_H */
