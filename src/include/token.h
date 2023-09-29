#ifndef TOKEN_H_
#define TOKEN_H_

#include <stddef.h>

/* A list of token types */
typedef enum TokenType {

  /* Single char tokens ******************************************************/
  TOKEN_LEFTPAREN,   // (
  TOKEN_RIGHT_PAREN, // )
  TOKEN_LEFT_BRACE,  // {
  TOKEN_RIGHT_BRACE, // }
  TOKEN_COMMA,       // ,
  TOKEN_SEMICOLON,   // ;

  TOKEN_DOT,   // .
  TOKEN_MINUS, // -
  TOKEN_PLUS,  // +
  TOKEN_SLASH, // /
  TOKEN_STAR,  // *
  /* 1 or 2 char tokens ******************************************************/
  TOKEN_BANG,          // !
  TOKEN_BANG_EQUAL,    // !=
  TOKEN_EQUAL,         // =
  TOKEN_EQUAL_EQUAL,   // ==
  TOKEN_GREATER,       // >
  TOKEN_GREATER_EQUAL, // >=
  TOKEN_LESS,          // <
  TOKEN_LESS_EQUAL,    // <=
  /* Literals ****************************************************************/
  TOKEN_IDENTIFIER, // The name of a function, variable, or keyword of some kind
  TOKEN_STRING,     // String literal
  TOKEN_NUMBER,     // A number literal
  /* Keywords ****************************************************************/
  TOKEN_AND,
  TOKEN_CLASS,
  TOKEN_ELSE,
  TOKEN_FALSE,
  TOKEN_FUNC,
  TOKEN_FOR,
  TOKEN_IF,
  TOKEN_NIL,
  TOKEN_OR,
  TOKEN_PRINT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_TRUE,
  TOKEN_VAR,
  TOKEN_WHILE,
  TOKEN_EOF,    // End of file
  TOKEN_INVALID // Invalid
} TokenType;

typedef struct LinePosition {
  size_t line; // Line number
  size_t x;    // Where in the line
} LinePosition;

/* Token structure. */
typedef struct Token {
  TokenType type;   // Token type
  LinePosition pos; // FIXME Its position in source
  const char *str;  // String literal
  size_t len;       // Length of the token
} Token;

/* KeyWord struct */
typedef struct KeyWord {
  char *word;
  TokenType token_type;
} KeyWord;

char *tokentype_to_string(TokenType type);

#endif // TOKEN_H_
