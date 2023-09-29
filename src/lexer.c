#include "include/lexer.h"
#include "include/list.h"
#include "include/util.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Keywords 2d array */
KeyWord kw[16] = {
    {"and", TOKEN_AND},     {"class", TOKEN_CLASS},   {"else", TOKEN_ELSE},
    {"false", TOKEN_FALSE}, {"for", TOKEN_FOR},       {"fun", TOKEN_FUNC},
    {"if", TOKEN_IF},       {"nil", TOKEN_NIL},       {"or", TOKEN_OR},
    {"print", TOKEN_PRINT}, {"return", TOKEN_RETURN}, {"super", TOKEN_SUPER},
    {"this", TOKEN_THIS},   {"true", TOKEN_TRUE},     {"var", TOKEN_VAR},
    {"while", TOKEN_WHILE}};

/* Macro for how many keywords we have in the language  */
#define KW_COUNT 16

/* Function declarations *****************************************************/
// Init and allocate lexer
Lexer *lexer_init(char *source, size_t sourcelen);
// Adds a token
void tokenlist_insert(Lexer *lexer, TokenType type, const char *beg,
                      const char *end);
// Scans the current character in source
void scan(Lexer *lexer);

/*****************************************************************************/
/*                                   Lexing                                  */
/*****************************************************************************/

/* Macro to check if the source has been fully lexed */
// #define LEXER_SOURCE_FINISHED(lex) ((lex->cursor > lex->source_len) ||
// (lex->source[lex->cursor] == '\0')) ? 1 : 0

#define LEXER_SOURCE_FINISHED(lex) ((lex->cursor > lex->source_len) ? 1 : 0)

/****************************************************************************/
// NOTE: Unused
static TokenList *create_tokenlist(void) {
  TokenList *list = calloc(1, sizeof(TokenList));
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;

  return list;
}
static void tknlist_insert(TokenList *list, Token *token) {
  assert(list != NULL && token != NULL);

  list->size++;
  TokenListNode *newNode = calloc(1, sizeof(TokenListNode));
  newNode->token = token;

  if (list->head == NULL) {
    list->head = newNode;
    list->tail = list->head;
  } else {
    TokenListNode *prevTail = list->tail;
    list->tail->next = newNode;
    newNode->previous = prevTail;
    list->tail = newNode;
  }
}
/****************************************************************************/

Lexer *lexer_init(char *source, size_t sourcelen) {

  printf("\n\nInitialising lexer.\n\n");

  Lexer *lexer;
  lexer = calloc(1, sizeof(Lexer)); // Create our lexer struct

  /* lexer->tokens = create_tokenlist(); // List structure where we store our *
   * tokens */
  lexer->token_list = array_create(sizeof(Token *));
  lexer->source = source;        // Assigning our source
  lexer->source_len = sourcelen; // The source length

  printf("\n\nSource is: \n\n'%s'\n\nSource length is '%zu'", lexer->source,
         lexer->source_len);

  lexer->cursor = 0; // Where we are in the overall source
  lexer->line_position.line = 0;
  lexer->line_position.x = 0;

  return lexer;
}

char lexer_advance(Lexer *lexer) {

  if (LEXER_SOURCE_FINISHED(lexer)) {
    return 0;
  }

  lexer->cursor++;
  lexer->line_position.x++;
  return lexer->source[lexer->cursor];
}

/* Checks for whether symbol starts */
static bool identifier_starting(char c) { return isalpha(c) || c == '_'; }

// NOTE: Unused
/* Checks for whether a symbol is still continuing */
static bool identifier_continuing(char c) {
  return identifier_starting(c) || isdigit(c);
}

/* Returns the next character from the lexer source without consuming it */
static char peek_next_char(Lexer *lexer, size_t curr_pos) {

  if (LEXER_SOURCE_FINISHED(lexer)) {
    PRINT_TRACE("%sPeeking cancelled.", " ");
    return '\0';
  }

  size_t nextPos = curr_pos + 1;

  /* PRINT_TRACE("Peeking char: %c", lex->source[nextPos]); */
  return lexer->source[nextPos];
}

/* Returns true if the next character is 'c' */
static bool matches_next_char(Lexer *lexer, char c) {
  if (LEXER_SOURCE_FINISHED(lexer)) {
    return false;
  }

  if (peek_next_char(lexer, lexer->cursor) != c) {
    return false;
  }

  return true;
}

/* Handles when an identifier of some kind is encountered.
  An identifier is a string without a quote delimiter. */
void tokenize_identifier(Lexer *lexer) {

  const char *start =
      &(lexer->source[lexer->cursor]); // Where the identifier starts

  const char *end = start;

  size_t cursPos = lexer->cursor;             // Where we are currently
  char peek = peek_next_char(lexer, cursPos); // The next char

  char c = lexer->source[cursPos];

  PRINT_TRACE("Curr char: '%c'", c);

  // identifier = daga42
  // continues until char is neither alphanumerical or `_`
  while (identifier_starting(peek) || isdigit(peek)) {

    printf("[tokenize_identifier] '%c' \n", peek);
    end++;
    cursPos++;
    peek = peek_next_char(lexer, cursPos);
  }

  tokenlist_insert(lexer, TOKEN_IDENTIFIER, start, end);
}

/* Tokenises a string literal, removes the quote marks */
void tokenize_string_literal(Lexer *lexer) {

  lexer_advance(lexer); // Removes quote mark
  const char *start =
      &(lexer->source[lexer->cursor]); // Pointer to where symbol starts

  const char *end = start;

  size_t startPos = lexer->cursor;
  size_t i = startPos;

  while (peek_next_char(lexer, i) != '"' && !LEXER_SOURCE_FINISHED(lexer)) {
    if (peek_next_char(lexer, i) == '\n' || peek_next_char(lexer, i) == '\r') {
      lexer->line_position.line++;
      lexer->line_position.x = 0;
    }
    i++;
    end++;
  }

  if ((i >= lexer->source_len) || (lexer->source[i] == '\0')) {
    PRINT_ERROR("%s on line %zu", "Unterminated string!",
                lexer->line_position.line);
    exit(1);
  }

  lexer_advance(lexer); // Trims leading `"`

  tokenlist_insert(lexer, TOKEN_STRING, start, end);
}

void tokenize_number_literal(Lexer *lexer) {

  const char *curr = &lexer->source[lexer->cursor];
  const char *end = &lexer->source[lexer->cursor];
  size_t i = lexer->cursor;

  if (!isdigit(*curr)) {
    PRINT_ERROR("Not a number: `%c`", *curr);
    return;
  }

  // Scanning through the numbers
  while (isdigit(peek_next_char(lexer, i))) {
    printf("i: %zu, lex->cursor: %zu\n", i, lexer->cursor);
    i++;
    end++;
  }

  // If the while loop stopped because of decimal point...
  if (peek_next_char(lexer, i) == '.' &&
      isdigit(peek_next_char(lexer, (i + 1)))) {
    i++;
    end++;
    while (isdigit(peek_next_char(lexer, i))) {
      printf("i: %zu, lex->cursor: %zu\n", i, lexer->cursor);
      i++;
      end++;
    }
  }
  tokenlist_insert(lexer, TOKEN_NUMBER, curr, end);
}

/* Creates new token and adds it to the linked list stored in the lexer struct
   This function takes the beginning position, and will iterate over the source
   and create a fresh string. @len is how long that tokens string should be.
 */
void tokenlist_insert(Lexer *lexer, TokenType type, const char *beg,
                      const char *end) {

  printf("\n\n---ADDING NEW TOKEN:---\n");
  PRINT_TRACE("TOKEN STRING:-> `%.*s`\nTOKEN LENGTH: `%zu`",
              (int)(end - beg + 1), beg, end - beg + 1);

  // Creating Token
  Token *token = calloc(1, sizeof(Token));
  assert((token != NULL) && "Malloc failed.");
  token->type = type;
  token->pos.line = lexer->line_position.line;
  token->pos.x = lexer->line_position.x;

  // Length of token string
  size_t tokenLength = ((end - beg) + 1);
  // Allocating memory for the string
  char *tokenString = calloc(tokenLength, sizeof(char));
  // Null terminating
  tokenString[tokenLength] = '\0';
  // Copying string
  memcpy(tokenString, beg, tokenLength);
  printf("Mem copied string: `%s`", tokenString);
  token->len = tokenLength;
  token->str = tokenString;

  if (*beg == '\0' || 0) {
    token->type = TOKEN_EOF;
    array_push(lexer->token_list, token);
    PRINT_TRACE("%s", "End of source. Exiting.");
    return;
  }

  // Checking for whether token is a keyword
  if (type == TOKEN_IDENTIFIER) {
    for (size_t i = 0; i < KW_COUNT; i++) {
      if (strcmp(tokenString, kw[i].word) == 0) {
        token->type = kw[i].token_type;
        printf("KEYWORD FOUND: %s\n\n", kw[i].word);
      }
    }
  }

  array_push(lexer->token_list, token);

  lexer->cursor += tokenLength - 1;
}

/* NOTE UNUSED
   Skips whitespaces and returns next character */
void lx_src_skip_whitespace(Lexer *lexer) {

  if (LEXER_SOURCE_FINISHED(lexer)) {
    return;
  }

  char next = lexer->source[lexer->cursor];

  // Skip whitespace
  while (next == '\n' || next == '\r' || next == '\t' || next == ' ') {
    lexer_advance(lexer);
  }

  return;
}

/* Skips a comment line */
void lx_src_skip_comment(Lexer *lexer) {

  size_t i = lexer->cursor;
  while (!(LEXER_SOURCE_FINISHED(lexer)) && peek_next_char(lexer, i) != '\n') {
    ++i;
  }

  size_t offset = (i - lexer->cursor);
  lexer->cursor += offset;
}

/* Scans the current character in source and lexes it appropiately */
void scan(Lexer *lexer) {

  if (LEXER_SOURCE_FINISHED(lexer)) {
    return;
  }

  const char *beg = &lexer->source[lexer->cursor]; // Current char

  PRINT_TRACE("Scanning char: `%c`", *beg);

  switch (*beg) {
  case 0: {
    printf("!!!---End of file reached---!!!");
    tokenlist_insert(lexer, TOKEN_EOF, beg, beg);
    break;
  }

    /* Whitespace ************************************************************/
  case ' ': {
    lexer->line_position.x++;
    break;
  }
  // FIXME Currently a tab width is only considered to be 1 space.
  case '\t': {
    lexer->line_position.x++;
    break;
  }
    // Carriage return
  case '\r': {
    lexer->line_position.x = 0;
    lexer->line_position.line++;
    break;
  }
    // New line
  case '\n': {
    lexer->line_position.x = 0;
    lexer->line_position.line++;
    break;
  }

  // Division OR the start of a comment
  case '/': {
    if (matches_next_char(lexer, '/')) { // If comment (//)...
      lx_src_skip_comment(lexer);
    } else { // If it is single slash...
      tokenlist_insert(lexer, TOKEN_SLASH, beg, beg);
    }
    break;
  }
    /* Single character tokens ***********************************************/
  case '{': {
    tokenlist_insert(lexer, TOKEN_LEFT_BRACE, beg, beg);
    break;
  }
  case '}': {
    tokenlist_insert(lexer, TOKEN_RIGHT_BRACE, beg, beg);
    break;
  }
  case '(': {
    tokenlist_insert(lexer, TOKEN_LEFTPAREN, beg, beg);
    break;
  }
  case ')': {
    tokenlist_insert(lexer, TOKEN_RIGHT_PAREN, beg, beg);
    break;
  }
  case ',': {
    tokenlist_insert(lexer, TOKEN_COMMA, beg, beg);
    break;
  }
  case '.': {
    tokenlist_insert(lexer, TOKEN_DOT, beg, beg);
    break;
  }
  case '-': {
    tokenlist_insert(lexer, TOKEN_MINUS, beg, beg);
    break;
  }
  case '+': {
    tokenlist_insert(lexer, TOKEN_PLUS, beg, beg);
    break;
  }
  case ';': {
    tokenlist_insert(lexer, TOKEN_SEMICOLON, beg, beg);

    break;
  }
  case '*': {
    tokenlist_insert(lexer, TOKEN_STAR, beg, beg);
    break;
  }
  /* Operators *************************************************************/
  case '!': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_BANG_EQUAL, beg, beg + 1)
        : tokenlist_insert(lexer, TOKEN_BANG, beg, beg);
    break;
  }
  case '=': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_EQUAL_EQUAL, beg, beg + 1)
        : tokenlist_insert(lexer, TOKEN_EQUAL, beg, beg);
    break;
  }
  case '<': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_LESS_EQUAL, beg, beg + 1)
        : tokenlist_insert(lexer, TOKEN_LESS, beg, beg);
    break;
  }
  case '>': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_GREATER_EQUAL, beg, beg + 1)
        : tokenlist_insert(lexer, TOKEN_GREATER, beg, beg);
    break;
  }
  case '"': {
    tokenize_string_literal(lexer);
    break;
  }
  default:
    // Numbers
    if (isdigit(*beg)) {
      tokenize_number_literal(lexer);
      break;
    }
    // Identifiers
    if (identifier_starting(*beg)) {
      tokenize_identifier(lexer);
      break;
    }

    // If nothing matches...
    // TODO handle if nothing matches
    PRINT_TRACE("Invalid character: %c", *beg);
    break;
  }
}

void token_print(void *tkn) {
  Token *token = (Token *)tkn;
  if (token->type == TOKEN_EOF) {
    printf("[TOKEN] EOF, type: '%s', Line: %zu, pos: %zu, length: %zu\n",
           tokentype_to_string(token->type), token->pos.line, token->pos.x,
           token->len);
    return;
  }
  printf(
      "[TOKEN] Str `%s`, type: `%s`, Line: `%zu`, pos: `%zu`, length: `%zu`\n",
      token->str, tokentype_to_string(token->type), token->pos.line,
      token->pos.x, token->len);
}

void tokenlist_print(Lexer *lexer) {

  if ((Token *)lexer->token_list->items[0] == NULL) {
    printf("List of tokens is empty!\n");
    return;
  }
  printf("\n\n-----------------------------------------------");
  printf("\n\t\t-- Token list: --\n");

  size_t i = 0;
  Token *curr = lexer->token_list->items[i];

  while (curr) {
    if (curr->type == TOKEN_EOF) {
      printf("[TOKEN] EOF \n");
      break;
    }
    token_print(curr);
    i++;
    curr = lexer->token_list->items[i];
  }

  printf("\t Total number of tokens: `%zu`\n", i);
  printf("-----------------------------------------------\n\n");
}

void tknlist_foreach(TokenList *list, void (*operation)(void *e)) {

  size_t count = 0;
  TokenListNode *curr = list->head;

  while (curr) {
    operation(curr->token);
    curr = curr->next;
    count++;
  }

  printf("list_foreach: %zu number of operations performed.\n", count);
}

void token_destroy(void *tkn) {
  Token *token = (Token *)tkn;
  free((void *)token->str);
}

void lexer_destroy(Lexer *lex) {
  PRINT_TRACE("Destroying the lexer! %s", "");
  free((void *)lex->source);

  // Freeing all mem from tokens
  /* tknlist_foreach(lex->tokens, token_destroy); */
  /* tknlist_foreach(lex->tokens, free); */
  /* free(lex->tokens); */
  free(lex);
  PRINT_TRACE("%s", "Lexer destroyed.");
}

void lexer_lex(Lexer *lexer) {
  /* printf("Contents: \n\n------\n%s\n-------\n\n", lexer->source); */

  while (!LEXER_SOURCE_FINISHED(lexer)) {
    printf("\nScanning char: `%c`\tCursor val: `%zu`\n",
           lexer->source[lexer->cursor], lexer->cursor);
    scan(lexer);
    lexer_advance(lexer);
  }

  tokenlist_print(lexer);
}

// NOTE: Ignore this function
void test_lexer(void) {
  char *source = "test.lox";
  size_t filesize = file_size_name(source);
  char *contents = file_open_read(source);

  PRINT_TRACE("Contents: \n%s", contents);

  Lexer *lex = lexer_init(contents, filesize);

  while (!LEXER_SOURCE_FINISHED(lex)) {
    /* PRINT_TRACE("Scanning char: %c. Cursor val: %zu",
     * lex->source[lex->cursor], */
    /* lex->cursor); */
    /* scan(lex); */
    /* LEXER_INCREMENT(lex); */
    lex->line_position.x++;
  }

  tokenlist_print(lex);
  /* lexer_destroy(lex); */
}
