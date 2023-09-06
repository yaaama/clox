#ifndef LEXER_H_
#define LEXER_H_

#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum TokenType {

  // Single-character tokens.
  LEFT_PAREN,
  RIGHT_PAREN,
  LEFT_BRACE,
  RIGHT_BRACE,
  COMMA,
  DOT,
  MINUS,
  PLUS,
  SEMICOLON,
  SLASH,
  STAR,
  // One or two character tokens.
  BANG,
  BANG_EQUAL,
  EQUAL,
  EQUAL_EQUAL,
  GREATER,
  GREATER_EQUAL,
  LESS,
  LESS_EQUAL,
  // Literals.
  IDENTIFIER,
  STRING,
  NUMBER,
  // Keywords.
  AND,
  CLASS,
  ELSE,
  FALSE,
  FUN,
  FOR,
  IF,
  NIL,
  OR,
  PRINT,
  RETURN,
  SUPER,
  THIS,
  TRUE,
  VAR,
  WHILE,
  EOF_T,

} TokenType;

typedef struct Position {
  size_t line; // Line number
  size_t x;    // Where in the line
} Position;

typedef struct Token {
  const char *str;
  TokenType type;
  Position pos;
  struct Token *next;
} Token;

typedef struct TokenList {
  Token *head;
  Token *tail;
  size_t size;
} TokenList;

typedef struct Lexer {
  const char *source;
  size_t source_len;
  size_t line_num;
  size_t cursor;
  size_t ln_cursor;
  bool error;
  TokenList *tokens;
  Error errors[256];
} Lexer;

FILE *open_file(char *filename);
char *file_contents(FILE *file);
size_t file_size(FILE *file);
void add_token(TokenType type, const char *beg, size_t len);
void test_lexer(void);

#endif // LEXER_H_
