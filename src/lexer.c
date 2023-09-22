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

typedef List_t TokenList; // Where we keep our tokens

/* Lexer *lexer; // The lexer */

/* Function declarations *****************************************************/
Lexer *init_lexer(char *source, size_t sourcelen); // Init and allocate lexer
void tokenlist_insert(Lexer *lexer, TokenType type, const char *beg,
                      size_t len); // Adds a token
void scan(Lexer *lexer);           // Scans the current character in source
char *tokentype_to_string(TokenType type);

/*****************************************************************************/
/*                                   Lexing                                  */
/*****************************************************************************/

/* Macro to increment the lexer provided */
#define LEXER_INCREMENT(lex) lex->cursor++

/* Macro to check if the source has been fully lexed */
/* #define LEXER_SOURCE_FINISHED(lex) \ */
/*   ((lex->cursor > lex->source_len) || (lex->source[lex->cursor] == '\0')) ? 1
                                                                           : 0
                                                                           */

#define LEXER_SOURCE_FINISHED(lex) ((lex->cursor > lex->source_len)) ? 1 : 0
/* Helpful macro to retrive the current line the lexer is on  */
#define LEXER_CURR_LINE(lex) lex->line_position.line

Lexer *init_lexer(char *source, size_t sourcelen) {

  PRINT_TRACE("%s", "Initialising lexer.");
  Lexer *lexer;
  lexer = calloc(1, sizeof(Lexer));    // Create our lexer struct
  list_create((void *)&lexer->tokens); // Creating the tokenlist
  lexer->source = source;              // Assigning our source
  lexer->source_len = sourcelen;       // The source length
  PRINT_TRACE("Source is: '%s', source length is '%zu'", lexer->source,
              lexer->source_len);

  lexer->cursor = 0; // Where we are in the overall source
  lexer->line_position.line = 0;
  lexer->line_position.x = 0;

  lexer->error = false;
  return lexer;
}

/* Checks for whether symbol starts */
static bool identifier_starting(char c) { return isalpha(c) || c == '_'; }

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
      &(lexer->source[lexer->cursor]); // Pointer to where identifier starts in
                                       // the source
  size_t cursPos = lexer->cursor;      // Where we are currently
  size_t len = 1;                      // The length of the identifier
  char peek = peek_next_char(lexer, cursPos); // The next identifier
  char c = lexer->source[cursPos];

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
    peek = peek_next_char(lexer, cursPos);
  }

  tokenlist_insert(lexer, TOKEN_IDENTIFIER, start, len);
}

/* Tokenises a string literal, removes the quote marks */
void tokenize_string_literal(Lexer *lexer) {

  const char *start =
      &(lexer->source[lexer->cursor]); // Pointer to where symbol starts
  size_t startPos = lexer->cursor;
  size_t i = startPos;

  while (peek_next_char(lexer, i) != '"' &&
         !((i >= lexer->source_len) || (lexer->source[i] == '\0'))) {
    if (peek_next_char(lexer, i) == '\n') {
      lexer->line_position.line++;
      lexer->line_position.x = 0;
    }
    i++;
  }

  if ((i >= lexer->source_len) || (lexer->source[i] == '\0')) {
    PRINT_ERROR("%s on line %zu", "Unterminated string!",
                LEXER_CURR_LINE(lexer));
    return;
  }

  LEXER_INCREMENT(lexer);   // Trims the leading " from the string
  lexer->line_position.x++; // Accounts for us trimming " from the string

  size_t len = i - startPos; // Length of the literal

  tokenlist_insert(lexer, TOKEN_STRING, start, len);
}

// BUG If you are scanning a string such as big1234a1244, then it will skip the
// 'a for some reason'.
void tokenize_number_literal(Lexer *lexer) {

  size_t i = lexer->cursor;

  char curr = lexer->source[i];
  if (!isdigit(curr)) {
    PRINT_ERROR("Not a number...%s", "");
    return;
  }

  // Scanning through the numbers
  while (isdigit(peek_next_char(lexer, i))) {
    printf("i: %zu, lex->cursor: %zu\n", i, lexer->cursor);
    i++;
  }

  // If the while loop stopped because of decimal point...
  if (peek_next_char(lexer, i) == '.' &&
      isdigit(peek_next_char(lexer, (i + 1)))) {
    i++;
    while (isdigit(peek_next_char(lexer, i))) {
      printf("i: %zu, lex->cursor: %zu\n", i, lexer->cursor);
      i++;
    }
  }
  i++;
  size_t len = i - lexer->cursor;
  tokenlist_insert(lexer, TOKEN_NUMBER, &lexer->source[lexer->cursor], len);
}

/* Creates new token and adds it to the linked list stored in the lexer struct
   This function takes the beginning position, and will iterate over the source
   and create a fresh string. @len is how long that tokens string should be.
 */
void tokenlist_insert(Lexer *lexer, TokenType type, const char *beg,
                      size_t len) {

  PRINT_TRACE("Adding token->%s", beg);
  Token *token = calloc(1, sizeof(Token));
  token->type = type;
  token->len = len;
  token->pos.line = lexer->line_position.line;
  token->pos.x = lexer->line_position.x;

  if (lexer->cursor >= lexer->source_len ||
      lexer->source[lexer->cursor] == '\0') {
    token->type = TOKEN_EOF;
    list_node_insert(lexer->tokens, token, NULL);
    PRINT_TRACE("%s", "End of source. Exiting.");
    return;
  }

  assert((token != NULL));

  // Preparing string stored in Token
  char *tokenStr = calloc(len + 1, sizeof(char));
  tokenStr[len] = '\0'; // Terminate token string

  size_t index = 0; // Keeps a track of how many iterations we've done

  // Single char token
  if (len == 1) {
    tokenStr[0] = *beg;
    token->str = tokenStr;
    list_node_insert(lexer->tokens, token, NULL);
    PRINT_TRACE("token str = %s", token->str);

    return;
  }

  // Multi char token
  while (index < len) {
    tokenStr[index] = lexer->source[lexer->cursor];
    index++;
    LEXER_INCREMENT(lexer);
  }

  // NOTE: If the next character is not a WS then we want to read it
  // For example: "func foo()"
  // If we do not decrement cursor then the ( will not be read.
  if (!isspace(lexer->source[lexer->cursor])) {
    lexer->cursor--;
  }

  // Checking for whether token is a keyword
  if (type == TOKEN_IDENTIFIER) {
    for (size_t i = 0; i < KW_COUNT; i++) {
      if (strcmp(tokenStr, kw[i].word) == 0) {
        token->type = kw[i].token_type;
      }
    }
  }
  token->str = tokenStr;

  list_node_insert(lexer->tokens, token, NULL);
  PRINT_TRACE("token str = %s", token->str);
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
    LEXER_INCREMENT(lexer);
  }

  return;
}

/* Skips a comment line */
void lx_src_skip_comment(Lexer *lexer) {

  while (!(LEXER_SOURCE_FINISHED(lexer)) &&
         peek_next_char(lexer, lexer->cursor) != '\n') {
    LEXER_INCREMENT(lexer);
  }
}

/* Scans the current character in source and lexes it appropiately */
void scan(Lexer *lexer) {

  if (LEXER_SOURCE_FINISHED(lexer)) {
    return;
  }

  const char *curr = &lexer->source[lexer->cursor]; // Current char
  PRINT_TRACE("Scanning char: `%c`", *curr);

  switch (*curr) {
  case 0: {
    PRINT_TRACE("%s End of file reached!", "\n");
    tokenlist_insert(lexer, TOKEN_EOF, curr, 1);
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
      tokenlist_insert(lexer, TOKEN_SLASH, curr, 1);
    }
    break;
  }
    /* Single character tokens ***********************************************/
  case '{': {
    tokenlist_insert(lexer, TOKEN_LEFT_BRACE, curr, 1);
    break;
  }
  case '}': {
    tokenlist_insert(lexer, TOKEN_RIGHT_BRACE, curr, 1);
    break;
  }
  case '(': {
    tokenlist_insert(lexer, TOKEN_LEFTPAREN, curr, 1);
    break;
  }
  case ')': {
    tokenlist_insert(lexer, TOKEN_RIGHT_PAREN, curr, 1);
    break;
  }
  case ',': {
    tokenlist_insert(lexer, TOKEN_COMMA, curr, 1);
    break;
  }
  case '.': {
    tokenlist_insert(lexer, TOKEN_DOT, curr, 1);
    break;
  }
  case '-': {
    tokenlist_insert(lexer, TOKEN_MINUS, curr, 1);
    break;
  }
  case '+': {
    tokenlist_insert(lexer, TOKEN_PLUS, curr, 1);
    break;
  }
  case ';': {
    tokenlist_insert(lexer, TOKEN_SEMICOLON, curr, 1);

    break;
  }
  case '*': {
    tokenlist_insert(lexer, TOKEN_STAR, curr, 1);
    break;
  }
  /* Operators *************************************************************/
  case '!': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_BANG_EQUAL, curr, 2)
        : tokenlist_insert(lexer, TOKEN_BANG, curr, 1);
    break;
  }
  case '=': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_EQUAL_EQUAL, curr, 2)
        : tokenlist_insert(lexer, TOKEN_EQUAL, curr, 1);
    break;
  }
  case '<': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_LESS_EQUAL, curr, 2)
        : tokenlist_insert(lexer, TOKEN_LESS, curr, 1);
    break;
  }
  case '>': {
    matches_next_char(lexer, '=')
        ? tokenlist_insert(lexer, TOKEN_GREATER_EQUAL, curr, 2)
        : tokenlist_insert(lexer, TOKEN_GREATER, curr, 1);
    break;
  }
  case '"': {
    tokenize_string_literal(lexer);
    break;
  }
  default:
    // Numbers
    if (isdigit(*curr)) {
      tokenize_number_literal(lexer);
      break;
    }
    // Identifiers
    if (identifier_starting(*curr)) {
      tokenize_identifier(lexer);
      break;
    }

    // If nothing matches...
    // TODO handle if nothing matches
    /* add_token(INVALID, curr, 1); */
    lexer->error = true;
    /* lex->errors[] */
    PRINT_TRACE("Invalid character: %c", *curr);
    break;
  }
}

void token_print(void *tkn) {
  Token *token = (Token *)tkn;
  printf("Token string: '%s', type: '%s', Line: %zu, pos: %zu, length: %zu\n",
         token->str, tokentype_to_string(token->type), token->pos.line,
         token->pos.x, token->len);
}

void tokenlist_print(Lexer *lexer) {

  if (lexer->tokens->head == NULL) {
    printf("List of tokens is empty!\n");
    return;
  }
  printf("\n\n-----------------------------------------------");
  printf("\n\t\t-- Token list: --\n");
  list_foreach(lexer->tokens, token_print);
  printf("\n-----------------------------------------------\n\n");
}

void token_destroy(void *tkn) {
  Token *token = (Token *)tkn;
  free((void *)token->str);
}

void lexer_destroy(Lexer *lex) {
  PRINT_TRACE("Destroying the lexer! %s", "");
  free((void *)lex->source);

  // Freeing all mem from tokens
  list_foreach(lex->tokens, token_destroy);

  list_destroy(lex->tokens);
  free(lex);
  PRINT_TRACE("%s", "Lexer destroyed.");
}

void lexer_lex(Lexer *lexer) {
  PRINT_TRACE("Contents: \n%s\n\n", lexer->source);

  while (!LEXER_SOURCE_FINISHED(lexer)) {
    PRINT_TRACE("Scanning char: %c. Cursor val: %zu",
                lexer->source[lexer->cursor], lexer->cursor);
    scan(lexer);
    LEXER_INCREMENT(lexer);
    lexer->line_position.x++;
  }
  tokenlist_print(lexer);
}

void test_lexer(void) {
  char *source = "test.lox";
  size_t filesize = file_size_name(source);
  char *contents = file_open_read(source);

  PRINT_TRACE("Contents: \n%s", contents);

  Lexer *lex = init_lexer(contents, filesize);

  while (!LEXER_SOURCE_FINISHED(lex)) {
    PRINT_TRACE("Scanning char: %c. Cursor val: %zu", lex->source[lex->cursor],
                lex->cursor);
    scan(lex);
    LEXER_INCREMENT(lex);
    lex->line_position.x++;
  }

  tokenlist_print(lex);
  /* lexer_destroy(lex); */
}

char *tokentype_to_string(TokenType type) {
  switch (type) {

  case (TOKEN_LEFTPAREN):
    return "LEFT_PAREN";
  case (TOKEN_RIGHT_PAREN):
    return "RIGHT_PAREN";
  case (TOKEN_LEFT_BRACE):
    return "LEFT_BRACE";
  case (TOKEN_RIGHT_BRACE):
    return "RIGHT_BRACE";
  case (TOKEN_COMMA):
    return "COMMA";
  case (TOKEN_DOT):
    return "DOT";
  case (TOKEN_MINUS):
    return "MINUS";
  case (TOKEN_PLUS):
    return "PLUS";
  case (TOKEN_SEMICOLON):
    return "SEMICOLON";
  case (TOKEN_SLASH):
    return "SLASH";
  case (TOKEN_STAR):
    return "STAR";
  case (TOKEN_BANG):
    return "BANG";
  case (TOKEN_BANG_EQUAL):
    return "BANG_EQUAL";
  case (TOKEN_EQUAL):
    return "EQUAL";
  case (TOKEN_EQUAL_EQUAL):
    return "EQUAL_EQUAL ";
  case (TOKEN_GREATER):
    return "GREATER";
  case (TOKEN_GREATER_EQUAL):
    return "GREATER_EQUAL";
  case (TOKEN_LESS):
    return "LESS";
  case (TOKEN_LESS_EQUAL):
    return "LESS_EQUAL";
  case (TOKEN_IDENTIFIER):
    return "IDENTIFIER";
  case (TOKEN_STRING):
    return "STRING";
  case (TOKEN_NUMBER):
    return "NUMBER";
  case (TOKEN_AND):
    return "AND";
  case (TOKEN_CLASS):
    return "CLASS";
  case (TOKEN_ELSE):
    return "ELSE";
  case (TOKEN_FALSE):
    return "FALSE";
  case (TOKEN_FUNC):
    return "FUN";
  case (TOKEN_FOR):
    return "FOR";
  case (TOKEN_IF):
    return "IF";
  case (TOKEN_NIL):
    return "NIL";
  case (TOKEN_OR):
    return "OR";
  case (TOKEN_PRINT):
    return "PRINT";
  case (TOKEN_RETURN):
    return "RETURN";
  case (TOKEN_SUPER):
    return "SUPER";
  case (TOKEN_THIS):
    return "THIS";
  case (TOKEN_TRUE):
    return "TRUE";
  case (TOKEN_VAR):
    return "VAR";
  case (TOKEN_WHILE):
    return "WHILE";
  case (TOKEN_EOF):
    return "EOF_TOKEN";
  case (TOKEN_INVALID):
    return "INVALID";
  }
}
