#ifndef LEXER_H_
#define LEXER_H_

#include "list.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* The Lexer struct */
typedef struct Lexer {
  const char *source; // The source we are lexing
  size_t source_len;  // Length of the source file
  array_T *token_list;
  size_t cursor;              // Tracks the position of where we have scanned
  LinePosition line_position; // FIXME Which line we are in and where
} Lexer;

void test_lexer(void);
Lexer *lexer_init(char *source, size_t filesize);
void lexer_lex(Lexer *lexer);

#endif // LEXER_H_
