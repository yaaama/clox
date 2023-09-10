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

typedef struct LinePos {
  size_t line; // Line number
  size_t x;    // Where in the line
} Position;

typedef struct Token {
  struct Token *next;
  TokenType type;
  Position pos;
  const char *str;
  size_t len;
} Token;

/* typedef struct TokenList { */
/*   Token *head; */
/*   Token *tail; */
/*   size_t size; */
/* } TokenList; */
typedef List_t TokenList;

typedef struct Lexer {
  const char *source;
  size_t source_len;
  TokenList *tokens;
  char curr_char;
  size_t cursor;
  size_t line_num;
  size_t line_pos;
  bool error;        // error flag
  Error errors[256]; // Array of errors (max 256)
} Lexer;

const char *whitespace = "\t\r\n";

/* Function declarations *****************************************************/
void init_lexer(char *source, size_t sourcelen);
TokenList *init_tokenlist(void);
FILE *open_file(char *filename);
char *file_contents(FILE *file);
size_t file_size(FILE *file);
void add_token(TokenType type, const char *beg, const char *end, size_t len);

// TODO Create procedures that will work on the Token linked list
Lexer *lex;

/*****************************************************************************/
/*                                   Lexing                                  */
/*****************************************************************************/
void init_lexer(char *source, size_t sourcelen) {

  Tput("Initialising lexer.");
  /* Will have to allocate source and tokens later */
  lex = malloc(sizeof(Lexer));

  list_create((void *)&lex->tokens);
  lex->source = source;
  lex->source_len = sourcelen;
  Tprint("Source is: '%s', source length is '%zu'", lex->source,
         lex->source_len);

  lex->line_num = 0;
  lex->cursor = 0;
  lex->line_pos = 0;

  lex->error = false;
}

/* Checks for whether symbol starts */
bool is_identifier_start(char c) { return isalpha(c) || c == '_'; }

/* Checks for whether a symbol is still going on */
bool is_identifier_continuing(char c) {
  return is_identifier_start(c) || isdigit(c);
}

char peek_next(size_t curr_pos) {

  if ((lex->cursor >= lex->source_len)) {
    Tprint("%sPeeking cancelled.", " ");
    return 0;
  }

  size_t nextPos = curr_pos + 1;

  Tprint("Peeking char: %c", lex->source[nextPos]);
  return lex->source[nextPos];
}

void handle_identifier(void) {

  /* bool end = !(lex->cursor <= lex->source_len); */

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
  add_token(IDENTIFIER, start, end, len);
}

// Creates new token and adds it to the linked list stored in the lexer struct
void add_token(TokenType type, const char *beg, const char *end, size_t len) {

  Tprint("Adding token->%s", beg);

  if (lex->cursor >= lex->source_len || lex->source[lex->cursor] == '\0') {
    Tprint("%s", "End of source. Exiting.");
    return;
  }

  Token *token = malloc(sizeof(Token));
  token->type = type;
  token->pos.line = lex->line_num;
  token->pos.x = lex->line_pos;
  token->next = NULL;

  assert((token != NULL));

  // Preparing string of the token
  char *tokenStr = malloc(sizeof(char) * (len + 1));
  tokenStr[len] = '\0'; // Terminate token string

  // If single char
  if (len == 1) {
    tokenStr[0] = *beg;
    token->str = tokenStr;
    list_node_insert(lex->tokens, token, NULL);
    lex->cursor++;
    return;
  }

  // If token is not single char...
  size_t index = 0; // Keeps a track of how many iterations we've done
  while (index < len) {
    tokenStr[index] = lex->source[lex->cursor];
    index++;
    lex->cursor++;
  }

  token->str = tokenStr;

  list_node_insert(lex->tokens, token, NULL);
  Tprint("token str = %s", token->str);
}

// Gets next character and skips whitespace
void next_char(void) {

  if ((lex->cursor >= lex->source_len) || (lex->source[lex->cursor] == '\0')) {
    return;
  }

  char next = lex->source[lex->cursor];
  /* lex->source++; */

  // Skip whitespace
  while (next == '\n' || next == '\r' || next == '\t' || next == ' ') {
    lex->cursor++;
    next = lex->source[lex->cursor];
  }

  return;
}

void scan_token(void) {

  if ((lex->cursor >= lex->source_len) || (lex->source[lex->cursor] == '\0')) {
    return;
  }
  /* const char *curr = &lex->source[lex->cursor]; */
  next_char();
  const char *curr = &lex->source[lex->cursor];
  Tprint("Consumed char: '%c'", *curr);

  switch (*curr) {

    // Whitespace
  case ' ': {
    break;
  }
  case '\t': {
    break;
  }
  case '\r': {
    break;
  }
    // New line
  case '\n': {
    lex->line_num++;
    lex->line_pos = 0;
    break;
  }

    // Single character tokens
  case '{': {
    add_token(LEFT_BRACE, curr, curr + 1, 1);
    break;
  }
  case '}': {
    add_token(RIGHT_BRACE, curr, curr + 1, 1);
    break;
  }
  case '(': {
    add_token(LEFT_PAREN, curr, curr + 1, 1);
    break;
  }
  case ')': {
    add_token(RIGHT_PAREN, curr, curr + 1, 1);
    break;
  }
  case ',': {
    add_token(COMMA, curr, curr + 1, 1);
    break;
  }
  case '.': {
    add_token(DOT, curr, curr + 1, 1);
    break;
  }
  case '-': {
    add_token(MINUS, curr, curr + 1, 1);
    break;
  }
  case '+': {
    add_token(PLUS, curr, curr + 1, 1);
    break;
  }
  case ';': {
    add_token(SEMICOLON, curr, curr + 1, 1);
    break;
  }
  case '*': {
    add_token(STAR, curr, curr + 1, 1);
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

  lex->cursor++;
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
  printf("Token string: '%s'\n", token->str);
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

  FILE *file = open_file("tests/basic-lexing");
  size_t filesize = file_size(file);
  char *contents = file_contents(file);
  Tprint("Contents: %s, filesize: %zu", contents, filesize);

  init_lexer(contents, filesize);

  /* lex->tokens; */
  list_create((void *)&lex->tokens);

  while (lex->cursor < lex->source_len) {
    Tprint("Scanning char: %c", lex->source[lex->cursor]);
    Tprint("Cursor val: %zu", lex->cursor);
    print_tokenlist();
    scan_token();
  }
}
