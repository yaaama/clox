#ifndef LEXER_H_
#define LEXER_H_

#include "list.h"
#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* A list of token types */
typedef enum TokenType {

  // Single-character tokens.
  TOKEN_LEFTPAREN,   // (
  TOKEN_RIGHT_PAREN, // )
  TOKEN_LEFT_BRACE,  // {
  TOKEN_RIGHT_BRACE, // }
  TOKEN_COMMA,       // ,
  TOKEN_DOT,         // .
  TOKEN_MINUS,       // -
  TOKEN_PLUS,        // +
  TOKEN_SEMICOLON,   // ;
  TOKEN_SLASH,       // /
  TOKEN_STAR,        // *
  // One or two character tokens.
  TOKEN_BANG,          // !
  TOKEN_BANG_EQUAL,    // !=
  TOKEN_EQUAL,         // =
  TOKEN_EQUAL_EQUAL,   // ==
  TOKEN_GREATER,       // >
  TOKEN_GREATER_EQUAL, // >=
  TOKEN_LESS,          // <
  TOKEN_LESS_EQUAL,    // <=
  // Literals
  TOKEN_IDENTIFIER, // The name of a function, variable, or keyword of some kind
  TOKEN_STRING,     // String literal
  TOKEN_NUMBER,     // A number literal
  // Keywords
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

/* KeyWord struct */
typedef struct KeyWord {
  char *word;
  TokenType token_type;
} KeyWord;
/* Vector to keep track of location info within the source */
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

/* The Lexer struct */
typedef struct Lexer {
  const char *source; // The source we are lexing
  size_t source_len;  // Length of the source file
  List_t *tokens;     // Stores our tokens (alias for List_t)
  /* char curr_char;             // TODO */
  /* size_t consumed_cursor;     // Tracks the characters we have consumed */
  size_t cursor;              // Tracks the position of where we have scanned
  LinePosition line_position; // Which line we are in and where
  bool error;                 // Error flag
  Error errors[256];          // Array of errors (max 256)
} Lexer;

void test_lexer(void);

#endif // LEXER_H_
