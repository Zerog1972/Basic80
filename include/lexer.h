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
    TOK_READ,
    TOK_DATA,
    TOK_RESTORE,
    TOK_SAVE,
    TOK_LOAD,
    TOK_HELP,
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

/* Structure pour un token */
typedef struct {
    BasicTokenType type;
    char *value;
    int lineNum;
} Token;

/* ===== FONCTIONS DU LEXER ===== */

/**
 * Tokenize une ligne de code BASIC en une liste de tokens.
 * 
 * Analyse lexicale d'une ligne de code source BASIC et la convertit
 * en une séquence de tokens identifiables. Reconnaît les mots-clés,
 * identifiants, nombres, chaînes, opérateurs et symboles.
 * 
 * @param line Chaîne de caractères contenant la ligne de code à analyser
 * @return Tableau de tokens terminé par un token TOK_EOF.
 *         Retourne NULL en cas d'erreur d'allocation mémoire.
 *         Le tableau doit être libéré avec freeTokens().
 * 
 * Exemples:
 *   "PRINT 42"        -> [TOK_PRINT, TOK_NUMBER("42"), TOK_EOF]
 *   "LET A = 10 + 5"  -> [TOK_LET, TOK_IDENTIFIER("A"), TOK_EQUALS, ...]
 *   "IF X > 5 THEN Y = 1" -> [TOK_IF, TOK_IDENTIFIER("X"), TOK_GT, ...]
 */
Token* tokenize(const char *line);

/**
 * Libère la mémoire allouée pour un tableau de tokens.
 * 
 * Désalloue tous les tokens d'un tableau retourné par tokenize(),
 * incluant les valeurs de chaînes associées à chaque token.
 * 
 * @param tokens Tableau de tokens à libérer (peut être NULL)
 * 
 * Note: Après l'appel, le pointeur tokens devient invalide.
 */
void freeTokens(Token *tokens);

#endif /* LEXER_H */
