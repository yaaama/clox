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

void init_lexer(char *source, size_t sourcelen) {

  Tput("Initialising lexer.");

  lex = malloc(sizeof(Lexer));
  list_create((void *)&lex->tokens); // Creating the tokenlist
  lex->source = source;              // Assigning our source
  lex->source_len = sourcelen;       // The source length
  Tprint("Source is: '%s', source length is '%zu'", lex->source,
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
    Tprint("%sPeeking cancelled.", " ");
    return '\0';
  }

  size_t nextPos = curr_pos + 1;

  Tprint("Peeking char: %c", lex->source[nextPos]);
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
  Tprint("Start: %c End: %c\n", *start, *end);
  size_t cursPos = lex->cursor;   // Where we are currently
  size_t len = 1;                 // The length of the symbol
  char peek = peek_next(cursPos); // The next symbol
  char c = lex->source[cursPos];

  Tprint("Start of identifier: %c", c);
  // Checks if the current character is the start of a symbol
  if (!is_identifier_start(c)) {
    Eprintf("Not identifier: '%c'", c);
    return;
  }

  while (is_identifier_continuing((peek))) {

    Tprint("\tCurr char: %c\n", peek);
    cursPos++;
    len++;
    end++;
    peek = peek_next(cursPos);
    Tprint("\tLength: %zu\n", len);
  }
  add_token(IDENTIFIER, start, len);
}

// Creates new token and adds it to the linked list stored in the lexer struct
void add_token(TokenType type, const char *beg, size_t len) {

  Tprint("Adding token->%s", beg);

  if (lex->cursor >= lex->source_len || lex->source[lex->cursor] == '\0') {
    Tprint("%s", "End of source. Exiting.");
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

  // If single char
  if (len == 1) {
    tokenStr[0] = *beg;
    token->str = tokenStr;
    list_node_insert(lex->tokens, token, NULL);

    return;
  }

  // If token is not single char...
  size_t index = 0; // Keeps a track of how many iterations we've done
  while (index < len) {
    tokenStr[index] = lex->source[lex->cursor];
    index++;
    LEXER_INCREMENT(lex);
  }

  token->str = tokenStr;

  list_node_insert(lex->tokens, token, NULL);
  Tprint("token str = %s", token->str);
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

  /* lex_skip_whitespace(); */
  /* scan_token(); */
}

void scanned_whitespace(void) {}

void scan_token(void) {

  if (LEXER_SOURCE_FINISHED(lex)) {
    return;
  }

  const char *curr = &lex->source[lex->cursor]; // Current char
  Tprint("Consumed char: '%c'", *curr);

  switch (*curr) {
    /* Whitespace ************************************************************/
  case ' ': {
    lex->line_position.x++;
    break;
  }
  case '\t': {
    lex->line_position.x++;
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
    if (is_identifier_start(*curr)) {
      handle_identifier();
      break;
    }
    Tprint("Invalid character: %c", *curr);
    break;
  }
}

/* Functions for file handling ***********************************************/

// Opens file
FILE *open_file(char *filename) {

  FILE *fp = fopen(filename, "r");

  if (fp == NULL) {
    Eprintf("FILE %s does not exist!", filename);
    print_usage();
    exit(1);
  }
  Tprint("File '%s' opened", filename);

  return fp;
}

// Returns string of file contents
char *file_contents(FILE *file) {

  size_t filesize = file_size(file); // Size of file in bytes
  Tprint("File size is: %zu", filesize);

  char *contents =
      malloc(sizeof(char) * (filesize + 1)); // String to store the bytes read
  contents[filesize] = '\0';                 // Will terminate the char
  size_t readCount = 0; // Keeps track of the number of bytes read
  size_t loopCount = 0; // Keeps track of iterations
  /* Tprint("readCount: %zu", readCount); */

  while (readCount < filesize) {

    // Exit if eof reached
    if (feof(file)) {
      Tprint("%s", "Eof reached!");
      break;
    }
    // Exit if error
    if (ferror(file)) {
      Tprint("%s", "Some sort of error has occured whilst reading the file.");
      break;
    }

    // Reading from file
    size_t currBytesRead = fread(contents, 1, filesize - readCount, file);
    readCount += currBytesRead;
    Tprint("Loop: %zu, readCount: %zu", loopCount, readCount);
    loopCount++;
  }

  // If the number of bytes are not the same as the filesize, then output error
  if (readCount != filesize) {
    Eprintf("%s", "Not all of the file was read!");
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
    Eprintf("%s", "fseek returned err");
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
  Tprint("Contents: %s, filesize: %zu", contents, filesize);

  init_lexer(contents, filesize);

  /* lex->tokens; */
  list_create((void *)&lex->tokens);

  while (!LEXER_SOURCE_FINISHED(lex)) {
    Tprint("Scanning char: %c", lex->source[lex->cursor]);
    Tprint("Cursor val: %zu", lex->cursor);
    scan_token();
    LEXER_INCREMENT(lex);
    print_tokenlist();
  }
  print_tokenlist();
}
