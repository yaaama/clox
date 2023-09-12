#include "lexer.h"
#include "list.h"
#include "util.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A list of token types */
typedef enum TokenType {

  // Single-character tokens.
  LEFT_PAREN,  // (
  RIGHT_PAREN, // )
  LEFT_BRACE,  // {
  RIGHT_BRACE, // }
  COMMA,       // ,
  DOT,         // .
  MINUS,       // -
  PLUS,        // +
  SEMICOLON,   // ;
  SLASH,       // /
  STAR,        // *
  // One or two character tokens.
  BANG,          // !
  BANG_EQUAL,    // !=
  EQUAL,         // =
  EQUAL_EQUAL,   // ==
  GREATER,       // >
  GREATER_EQUAL, // >=
  LESS,          // <
  LESS_EQUAL,    // <=
  // Literals
  IDENTIFIER, // The name of a function, variable, or keyword of some kind
  STRING,     // String literal
  NUMBER,     // A number literal
  // Keywords
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
  EOF_T,  // End of file
  INVALID // Invalid
} TokenType;

/* KeyWord struct */
typedef struct KeyWord {
  char *word;
  TokenType token_type;
} KeyWord;

/* Keywords 2d array */
KeyWord kw[16] = {
    {"and", AND},   {"class", CLASS}, {"else", ELSE},     {"false", FALSE},
    {"for", FOR},   {"fun", FUN},     {"if", IF},         {"nil", NIL},
    {"or", OR},     {"print", PRINT}, {"return", RETURN}, {"super", SUPER},
    {"this", THIS}, {"true", TRUE},   {"var", VAR},       {"while", WHILE}};

/* Macro for how many keywords we have in the language  */
#define KW_COUNT 16

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

typedef List_t TokenList; // Where we keep our tokens

/* The Lexer struct */
typedef struct Lexer {
  const char *source; // The source we are lexing
  size_t source_len;  // Length of the source file
  TokenList *tokens;  // Stores our tokens (alias for List_t)
  /* char curr_char;             // TODO */
  /* size_t consumed_cursor;     // Tracks the characters we have consumed */
  size_t cursor;              // Tracks the position of where we have scanned
  LinePosition line_position; // Which line we are in and where
  bool error;                 // Error flag
  Error errors[256];          // Array of errors (max 256)
} Lexer;

Lexer *lex; // The lexer

/* Function declarations *****************************************************/
void lexer_init(char *source, size_t sourcelen); // Init and allocate lexer
void tokenlist_insert(TokenType type, const char *beg,
                      size_t len); // Adds a token
void scan(void);                   // Scans the current character in source

/*****************************************************************************/
/*                                   Lexing                                  */
/*****************************************************************************/

/* Macro to increment the lexer provided */
#define LEXER_INCREMENT(lex) lex->cursor++

/* Macro to check if the source has been fully lexed */
#define LEXER_SOURCE_FINISHED(lex)                                             \
  ((lex->cursor >= lex->source_len) || (lex->source[lex->cursor] == '\0')) ? 1 \
                                                                           : 0

/* Helpful macro to retrive the current line the lexer is on  */
#define LEXER_CURR_LINE(lex) lex->line_position.line

void lexer_init(char *source, size_t sourcelen) {

  PRINT_TRACE("%s", "Initialising lexer.");

  lex = malloc(sizeof(Lexer));       // Create our lexer struct
  list_create((void *)&lex->tokens); // Creating the tokenlist
  lex->source = source;              // Assigning our source
  lex->source_len = sourcelen;       // The source length
  PRINT_TRACE("Source is: '%s', source length is '%zu'", lex->source,
              lex->source_len);

  lex->cursor = 0; // Where we are in the overall source
  lex->line_position.line = 0;
  lex->line_position.x = 0;

  lex->error = false;
}

/* Checks for whether symbol starts */
static bool identifier_starting(char c) { return isalpha(c) || c == '_'; }

/* Checks for whether a symbol is still continuing */
static bool identifier_continuing(char c) {
  return identifier_starting(c) || isdigit(c);
}

/* Returns the next character from the lexer source without consuming it */
static char peek_next_char(size_t curr_pos) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    PRINT_TRACE("%sPeeking cancelled.", " ");
    return '\0';
  }

  size_t nextPos = curr_pos + 1;

  /* PRINT_TRACE("Peeking char: %c", lex->source[nextPos]); */
  return lex->source[nextPos];
}

/* Returns true if the next character is 'c' */
static bool matches_next_char(char c) {
  if (LEXER_SOURCE_FINISHED(lex)) {
    return false;
  }

  if (peek_next_char(lex->cursor) != c) {
    return false;
  }

  return true;
}

/* Handles when an identifier of some kind is encountered.
  An identifier is a string without a quote delimiter. */
void tokenize_identifier(void) {

  const char *start =
      &(lex->source[lex->cursor]); // Pointer to where identifier starts in the
                                   // source
  size_t cursPos = lex->cursor;    // Where we are currently
  size_t len = 1;                  // The length of the identifier
  char peek = peek_next_char(cursPos); // The next identifier
  char c = lex->source[cursPos];

  PRINT_TRACE("Curr char: '%c'", c);
  // Checks if the current character is the start of a identifier
  if (!identifier_starting(c)) {
    PRINT_ERROR("Not start identifier: '%c'", c);
    return;
  }

  while (identifier_continuing((peek))) {

    PRINT_TRACE("Curr char: '%c', id len %zu", peek, len);
    cursPos++;
    len++;
    peek = peek_next_char(cursPos);
  }

  tokenlist_insert(IDENTIFIER, start, len);
}

/* Tokenises a string literal, removes the quote marks */
void tokenize_string_literal(void) {

  const char *start =
      &(lex->source[lex->cursor]); // Pointer to where symbol starts
  size_t startPos = lex->cursor;
  size_t i = startPos;

  while (peek_next_char(i) != '"' &&
         !((i >= lex->source_len) || (lex->source[i] == '\0'))) {
    if (peek_next_char(i) == '\n') {
      lex->line_position.line++;
      lex->line_position.x = 0;
    }
    i++;
  }

  if ((i >= lex->source_len) || (lex->source[i] == '\0')) {
    PRINT_ERROR("%s on line %zu", "Unterminated string!", LEXER_CURR_LINE(lex));
    return;
  }

  LEXER_INCREMENT(lex);   // Trims the leading " from the string
  lex->line_position.x++; // Accounts for us trimming " from the string

  size_t len = i - startPos; // Length of the literal

  tokenlist_insert(STRING, start, len);
}

// BUG If you are scanning a string such as big1234a1244, then it will skip the
// 'a for some reason'.
void tokenize_number_literal(void) {

  size_t i = lex->cursor;

  char curr = lex->source[i];
  if (!isdigit(curr)) {
    PRINT_ERROR("Not a number...%s", "");
    return;
  }

  // Scanning through the numbers
  while (isdigit(peek_next_char(i))) {
    printf("i: %zu, lex->cursor: %zu\n", i, lex->cursor);
    i++;
  }

  // If the while loop stopped because of decimal point...
  if (peek_next_char(i) == '.' && isdigit(peek_next_char(i + 1))) {
    i++;
    while (isdigit(peek_next_char(i))) {
      printf("i: %zu, lex->cursor: %zu\n", i, lex->cursor);
      i++;
    }
  }
  i++;
  size_t len = i - lex->cursor;
  tokenlist_insert(NUMBER, &lex->source[lex->cursor], len);
}

/* Creates new token and adds it to the linked list stored in the lexer struct
   This function takes the beginning position, and will iterate over the source
   and create a fresh string. @len is how long that tokens string should be.
 */
void tokenlist_insert(TokenType type, const char *beg, size_t len) {

  PRINT_TRACE("Adding token->%s", beg);

  if (lex->cursor >= lex->source_len || lex->source[lex->cursor] == '\0') {
    PRINT_TRACE("%s", "End of source. Exiting.");
    return;
  }

  Token *token = malloc(sizeof(Token));
  token->type = type;
  token->len = len;
  token->pos.line = lex->line_position.line;
  token->pos.x = lex->line_position.x;

  assert((token != NULL));

  // Preparing string stored in Token
  char *tokenStr = calloc(1, sizeof(char));
  tokenStr[len] = '\0'; // Terminate token string

  size_t index = 0; // Keeps a track of how many iterations we've done

  // Single char token
  if (len == 1) {
    tokenStr[0] = *beg;
    token->str = tokenStr;
    list_node_insert(lex->tokens, token, NULL);
    PRINT_TRACE("token str = %s", token->str);

    return;
  }

  // Multi char token
  while (index < len) {
    tokenStr[index] = lex->source[lex->cursor];
    index++;
    LEXER_INCREMENT(lex);
  }

  // NOTE: If the next character is not a WS then we want to read it
  // For example: "func foo()"
  // If we do not decrement cursor then the ( will not be read.
  if (!isspace(lex->source[lex->cursor])) {
    lex->cursor--;
  }

  // Checking for whether token is a keyword
  if (type == IDENTIFIER) {
    for (size_t i = 0; i < KW_COUNT; i++) {
      if (strcmp(tokenStr, kw[i].word) == 0) {
        token->type = kw[i].token_type;
      }
    }
  }
  token->str = tokenStr;

  list_node_insert(lex->tokens, token, NULL);
  PRINT_TRACE("token str = %s", token->str);
}

/* NOTE UNUSED
   Skips whitespaces and returns next character */
void lx_src_skip_whitespace(void) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    return;
  }

  char next = lex->source[lex->cursor];

  // Skip whitespace
  while (next == '\n' || next == '\r' || next == '\t' || next == ' ') {
    LEXER_INCREMENT(lex);
  }

  return;
}

/* Skips a comment line */
void lx_src_skip_comment(void) {

  while (!(LEXER_SOURCE_FINISHED(lex)) && peek_next_char(lex->cursor) != '\n') {
    LEXER_INCREMENT(lex);
  }
}

/* Scans the current character in source and lexes it appropiately */
void scan(void) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    return;
  }

  const char *curr = &lex->source[lex->cursor]; // Current char
  PRINT_TRACE("Consumed char: '%c'", *curr);

  switch (*curr) {
  case '"': {
    tokenize_string_literal();
    break;
  }
  case 0: {
    tokenlist_insert(EOF_T, curr, 1);
    break;
  }

    /* Whitespace ************************************************************/
  case ' ': {
    break;
  }
  // FIXME Currently a tab width is only considered to be 1 space.
  case '\t': {
    break;
  }
    // Carriage return
  case '\r': {
    lex->line_position.x = 0;
    lex->line_position.line += 1;
    break;
  }
    // New line
  case '\n': {
    lex->line_position.x = 0;
    lex->line_position.line += 1;
    break;
  }

  // Division OR the start of a comment
  case '/': {
    if (matches_next_char('/')) { // If comment (//)...
      lx_src_skip_comment();
    } else { // If it is single slash...
      tokenlist_insert(SLASH, curr, 1);
    }
    break;
  }
    /* Single character tokens ***********************************************/
  case '{': {
    tokenlist_insert(LEFT_BRACE, curr, 1);
    break;
  }
  case '}': {
    tokenlist_insert(RIGHT_BRACE, curr, 1);
    break;
  }
  case '(': {
    tokenlist_insert(LEFT_PAREN, curr, 1);
    break;
  }
  case ')': {
    tokenlist_insert(RIGHT_PAREN, curr, 1);
    break;
  }
  case ',': {
    tokenlist_insert(COMMA, curr, 1);
    break;
  }
  case '.': {
    tokenlist_insert(DOT, curr, 1);
    break;
  }
  case '-': {
    tokenlist_insert(MINUS, curr, 1);
    break;
  }
  case '+': {
    tokenlist_insert(PLUS, curr, 1);
    break;
  }
  case ';': {
    tokenlist_insert(SEMICOLON, curr, 1);
    break;
  }
  case '*': {
    tokenlist_insert(STAR, curr, 1);
    break;
  }
  /* Operators *************************************************************/
  case '!': {
    matches_next_char('=') ? tokenlist_insert(BANG_EQUAL, curr, 2)
                           : tokenlist_insert(BANG, curr, 1);
    break;
  }
  case '=': {
    matches_next_char('=') ? tokenlist_insert(EQUAL_EQUAL, curr, 2)
                           : tokenlist_insert(EQUAL, curr, 1);
    break;
  }
  case '<': {
    matches_next_char('=') ? tokenlist_insert(LESS_EQUAL, curr, 2)
                           : tokenlist_insert(LESS, curr, 1);
    break;
  }
  case '>': {
    matches_next_char('=') ? tokenlist_insert(GREATER_EQUAL, curr, 2)
                           : tokenlist_insert(GREATER, curr, 1);
    break;
  }
  default:
    // Numbers
    if (isdigit(*curr)) {
      tokenize_number_literal();
      break;
    }
    // Identifiers
    if (identifier_starting(*curr)) {
      tokenize_identifier();
      break;
    }

    // If nothing matches...
    // TODO handle if nothing matches
    /* add_token(INVALID, curr, 1); */
    lex->error = true;
    /* lex->errors[] */
    PRINT_TRACE("Invalid character: %c", *curr);
    break;
  }
}

void token_print(void *tkn) {
  Token *token = (Token *)tkn;
  printf("Token string: '%s', type: %d, Line: %zu, pos: %zu, length: %zu\n",
         token->str, token->type, token->pos.line, token->pos.x, token->len);
}

void tokenlist_print(void) {

  if (lex->tokens->head == NULL) {
    printf("List of tokens is empty!\n");
    return;
  }
  printf("\n\n-----------------------------------------------");
  printf("\n\t\t-- Token list: --\n");
  list_foreach(lex->tokens, token_print);
  printf("\n-----------------------------------------------\n\n");
}

void lexer_destroy(Lexer *lex) {
  PRINT_TRACE("Destroying the lexer! %s", "");
  free((void *)lex->source);
  list_destroy(lex->tokens);
  free(lex);
  PRINT_TRACE("%s", "Lexer destroyed.");
}

void test_lexer(void) {

  char *source = "tests/lexer-keywords";
  size_t filesize = file_size_name(source);
  char *contents = file_open_read(source);

  PRINT_TRACE("Contents: \n%s", contents);

  lexer_init(contents, filesize);

  list_create((void *)&lex->tokens);

  while (!LEXER_SOURCE_FINISHED(lex)) {
    PRINT_TRACE("Scanning char: %c. Cursor val: %zu", lex->source[lex->cursor],
                lex->cursor);
    scan();
    LEXER_INCREMENT(lex);
    lex->line_position.x++;
  }
  tokenlist_print();
  lexer_destroy(lex);
}
