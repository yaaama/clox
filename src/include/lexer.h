#ifndef LEXER_H_
#define LEXER_H_

#include "list.h"
#include "token.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

/* KeyWord struct */
typedef struct KeyWord {
  char *word;
  TokenType token_type;
} KeyWord;
/* Vector to keep track of location info within the source */

typedef struct TOKENLIST_NODE_STRUCT {
  Token *token;
  struct TOKENLIST_NODE_STRUCT *next;
  struct TOKENLIST_NODE_STRUCT *previous;

} TokenListNode;

typedef struct TOKENLIST_STRUCT {
  TokenListNode *head;
  TokenListNode *tail;
  size_t size;

} TokenList;

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
char *tokentype_to_string(TokenType type);

#endif // LEXER_H_
