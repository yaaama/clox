#include "lexer.h"
#include "list.h"
#include "util.h"
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

typedef struct LinePosition {
  size_t line; // Line number
  size_t x;    // Where in the line
} LinePosition;

typedef struct Token {
  TokenType type;
  LinePosition pos;
  const char *str;
  size_t len;
} Token;

typedef List_t TokenList; // Where we keep our tokens

typedef struct Lexer {
  const char *source;         // The source we are lexing
  size_t source_len;          // Length of the source file
  TokenList *tokens;          // Stores our tokens (alias for List_t)
  char curr_char;             // TODO
  size_t consumed_cursor;     // Tracks the characters we have consumed
  size_t cursor;              // Tracks the position of where we have scanned
  LinePosition line_position; // Which line we are in and where
  bool error;                 // Error flag
  Error errors[256];          // Array of errors (max 256)
} Lexer;

Lexer *lex; // The lexer

/* Function declarations *****************************************************/
void init_lexer(char *source, size_t sourcelen);
FILE *open_file(char *filename); // Opens file
char *file_contents(FILE *file); // Retrieves contents from file
size_t file_size(FILE *file);    // Size of file
void add_token(TokenType type, const char *beg, size_t len); // Adds a token
void insert_token(Token *token); // Inserts a token into our TokenList
void scan_token(void);

/*****************************************************************************/
/*                                   Lexing                                  */
/*****************************************************************************/

#define LEXER_INCREMENT(lex) lex->cursor++

// Macro to check if the source has been fully lexed
#define LEXER_SOURCE_FINISHED(lex)                                             \
  ((lex->cursor >= lex->source_len) || (lex->source[lex->cursor] == '\0')) ? 1 \
                                                                           : 0

#define LEXER_CURR_LINE(lex) lex->line_position.line

void init_lexer(char *source, size_t sourcelen) {

  PRINT_TRACE("%s", "Initialising lexer.");

  lex = malloc(sizeof(Lexer));
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
bool is_identifier_start(char c) { return isalpha(c) || c == '_'; }

/* Checks for whether a symbol is still going on */
bool is_identifier_continuing(char c) {
  return is_identifier_start(c) || isdigit(c);
}

char peek_next(size_t curr_pos) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    PRINT_TRACE("%sPeeking cancelled.", " ");
    return '\0';
  }

  size_t nextPos = curr_pos + 1;

  PRINT_TRACE("Peeking char: %c", lex->source[nextPos]);
  return lex->source[nextPos];
}

bool match_next(char c) {
  if (LEXER_SOURCE_FINISHED(lex)) {
    return false;
  }

  if (peek_next(lex->cursor) != c) {
    return false;
  }

  return true;
}

void handle_identifier(void) {

  const char *start =
      &(lex->source[lex->cursor]); // Pointer to where symbol starts
  const char *end = &(lex->source[lex->cursor]); // Ptr to where symbol ends
  PRINT_TRACE("Start: %c End: %c\n", *start, *end);
  size_t cursPos = lex->cursor;   // Where we are currently
  size_t len = 1;                 // The length of the symbol
  char peek = peek_next(cursPos); // The next symbol
  char c = lex->source[cursPos];

  PRINT_TRACE("Start of identifier: %c", c);
  // Checks if the current character is the start of a symbol
  if (!is_identifier_start(c)) {
    PRINT_ERROR("Not identifier: '%c'", c);
    return;
  }

  while (is_identifier_continuing((peek))) {

    PRINT_TRACE("\tCurr char: %c\n", peek);
    cursPos++;
    len++;
    end++;
    peek = peek_next(cursPos);
    PRINT_TRACE("\tLength: %zu\n", len);
  }
  add_token(IDENTIFIER, start, len);
}

// Tokenises a string literal, removes the quote marks
void handle_string_literal(void) {

  const char *start =
      &(lex->source[lex->cursor]); // Pointer to where symbol starts
  size_t startPos = lex->cursor;
  size_t i = startPos;

  while (peek_next(i) != '"' &&
         !((i >= lex->source_len) || (lex->source[i] == '\0'))) {
    if (peek_next(i) == '\n') {
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

  add_token(STRING, start, len);
}

// BUG If you are scanning a string such as big1234a1244, then it will skip the
// 'a for some reason'.
void handle_number_literal(void) {

  size_t i = lex->cursor;

  char curr = lex->source[i];
  if (!isdigit(curr)) {
    PRINT_ERROR("Not a number...%s", "");
    return;
  }

  // Scanning through the numbers
  while (isdigit(peek_next(i))) {
    printf("i: %zu, lex->cursor: %zu\n", i, lex->cursor);
    i++;
  }

  // If the while loop stopped because of decimal point...
  if (peek_next(i) == '.' && isdigit(peek_next(i + 1))) {
    i++;
    while (isdigit(peek_next(i))) {
      printf("i: %zu, lex->cursor: %zu\n", i, lex->cursor);
      i++;
    }
  }
  i++;
  size_t len = i - lex->cursor;
  add_token(NUMBER, &lex->source[lex->cursor], len);
}

// Creates new token and adds it to the linked list stored in the lexer struct
void add_token(TokenType type, const char *beg, size_t len) {

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

  // Preparing string of the token
  char *tokenStr = malloc(sizeof(char) * (len + 1));
  tokenStr[len] = '\0'; // Terminate token string

  size_t index = 0; // Keeps a track of how many iterations we've done

  // If single char
  if (len == 1) {
    tokenStr[0] = *beg;
    token->str = tokenStr;
    list_node_insert(lex->tokens, token, NULL);

    return;
  }

  // If token is not single char...
  while (index < len) {
    tokenStr[index] = lex->source[lex->cursor];
    index++;
    LEXER_INCREMENT(lex);
  }

  token->str = tokenStr;

  list_node_insert(lex->tokens, token, NULL);
  PRINT_TRACE("token str = %s", token->str);
}

// Skips whitespaces and returns next character
void lex_skip_whitespace(void) {

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

void lex_skip_comment(void) {

  while (!(LEXER_SOURCE_FINISHED(lex)) && peek_next(lex->cursor) != '\n') {
    LEXER_INCREMENT(lex);
  }
}

void scan_token(void) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    return;
  }

  const char *curr = &lex->source[lex->cursor]; // Current char
  PRINT_TRACE("Consumed char: '%c'", *curr);

  switch (*curr) {
  case '"': {
    handle_string_literal();
    break;
  }
  case 0:
    add_token(EOF_T, curr, 1);
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
    /* Single character tokens ***********************************************/
  case '{': {
    add_token(LEFT_BRACE, curr, 1);
    break;
  }
  case '}': {
    add_token(RIGHT_BRACE, curr, 1);
    break;
  }
  case '(': {
    add_token(LEFT_PAREN, curr, 1);
    break;
  }
  case ')': {
    add_token(RIGHT_PAREN, curr, 1);
    break;
  }
  case ',': {
    add_token(COMMA, curr, 1);
    break;
  }
  case '.': {
    add_token(DOT, curr, 1);
    break;
  }
  case '-': {
    add_token(MINUS, curr, 1);
    break;
  }
  case '+': {
    add_token(PLUS, curr, 1);
    break;
  }
  case ';': {
    add_token(SEMICOLON, curr, 1);
    break;
  }
  case '*': {
    add_token(STAR, curr, 1);
    break;
  }
  /* Operators *************************************************************/
  case '!': {
    match_next('=') ? add_token(BANG_EQUAL, curr, 2) : add_token(BANG, curr, 1);
    break;
  }
  case '=': {
    match_next('=') ? add_token(EQUAL_EQUAL, curr, 2)
                    : add_token(EQUAL, curr, 1);
    break;
  }
  case '<': {
    match_next('=') ? add_token(LESS_EQUAL, curr, 2) : add_token(LESS, curr, 1);
    break;
  }
  case '>': {
    match_next('=') ? add_token(GREATER_EQUAL, curr, 2)
                    : add_token(GREATER, curr, 1);
    break;
  }
  // Division OR the start of a comment
  case '/': {
    if (match_next('/')) { // If comment (//)...
      lex_skip_comment();
    } else { // If it is single slash...
      add_token(SLASH, curr, 1);
    }
    break;
  }
  default:
    if (isdigit(*curr)) {
      handle_number_literal();
      break;
    }
    if (is_identifier_start(*curr)) {
      handle_identifier();
      break;
    }
    PRINT_TRACE("Invalid character: %c", *curr);
    break;
  }
}

/* Functions for file handling ***********************************************/

// Opens file
FILE *open_file(char *filename) {

  FILE *fp = fopen(filename, "rb");

  if (fp == NULL) {
    PRINT_ERROR("FILE %s does not exist!", filename);
    print_usage();
    exit(1);
  }
  PRINT_TRACE("File '%s' opened", filename);

  return fp;
}

// Returns string of file contents
char *file_contents(FILE *file) {

  size_t filesize = file_size(file); // Size of file in bytes
  PRINT_TRACE("File size is: %zu", filesize);

  char *contents =
      malloc(sizeof(char) * (filesize + 1)); // String to store the bytes read
  contents[filesize] = '\0';                 // Will terminate the char
  size_t readCount = 0; // Keeps track of the number of bytes read
  size_t loopCount = 0; // Keeps track of iterations
  /* PRINT_TRACE("readCount: %zu", readCount); */

  while (readCount < filesize) {

    // Exit if eof reached
    if (feof(file)) {
      PRINT_TRACE("%s", "Eof reached!");
      break;
    }
    // Exit if error
    if (ferror(file)) {
      PRINT_TRACE("%s",
                  "Some sort of error has occured whilst reading the file.");
      break;
    }

    // Reading from file
    size_t currBytesRead = fread(contents, 1, filesize - readCount, file);
    readCount += currBytesRead;
    PRINT_TRACE("Loop: %zu, readCount: %zu", loopCount, readCount);
    loopCount++;
  }

  // If the number of bytes are not the same as the filesize, then output error
  if (readCount != filesize) {
    PRINT_ERROR("%s", "Not all of the file was read!");
    fclose(file);
    return NULL;
  }

  fclose(file);
  return contents;
}

// Returns file length
size_t file_size(FILE *file) {

  int err = fseek(file, 0L, SEEK_END);
  if (err) {
    PRINT_ERROR("%s", "fseek returned err");
    return 0;
  }
  long size = ftell(file);
  assert(size > 0 && "File is empty!");

  rewind(file);

  return size;
}

void print_token(void *tkn) {
  Token *token = (Token *)tkn;
  printf("Token string: '%s', Line: %zu, pos: %zu, length: %zu\n", token->str,
         token->pos.line, token->pos.x, token->len);
}

void print_tokenlist(void) {

  if (lex->tokens->head == NULL) {
    printf("List of tokens is empty!\n");
    return;
  }
  printf("\n\n-----------------------------------------------");
  printf("\n\t\t-- Token list: --\n");
  list_foreach(lex->tokens, print_token);
  printf("\n-----------------------------------------------\n\n");
}

void test_lexer(void) {

  FILE *file = open_file("tests/lexer-token-position");
  size_t filesize = file_size(file);
  char *contents = file_contents(file);
  PRINT_TRACE("Contents: %s, filesize: %zu", contents, filesize);

  PRINT_ERROR("%s", "Hey there champ!");

  init_lexer(contents, filesize);

  /* lex->tokens; */
  list_create((void *)&lex->tokens);

  while (!LEXER_SOURCE_FINISHED(lex)) {
    PRINT_TRACE("Scanning char: %c", lex->source[lex->cursor]);
    PRINT_TRACE("Cursor val: %zu", lex->cursor);
    scan_token();
    LEXER_INCREMENT(lex);
    lex->line_position.x++;
    print_tokenlist();
  }
  print_tokenlist();
}
