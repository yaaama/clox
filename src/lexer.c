#include "lexer.h"
#include "util.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

const char *whitespace = "\t\r\n";

/* Function declarations *****************************************************/
void init_lexer(char *source, size_t sourcelen);
TokenList *init_tokenlist(void);
FILE *open_file(char *filename);
char *file_contents(FILE *file);
size_t file_size(FILE *file);

// TODO Create procedures that will work on the Token linked list
Lexer *lex;

void init_lexer(char *source, size_t sourcelen) {

  Tput("Initialising lexer.");
  /* Will have to allocate source and tokens later */
  lex = malloc(sizeof(Lexer));
  lex->tokens = malloc(sizeof(TokenList));
  lex->source = source;
  lex->source_len = sourcelen;
  Tprint("Source is: '%s', source length is '%zu'", lex->source,
         lex->source_len);

  lex->line_num = 0;
  lex->cursor = 0;
  lex->ln_cursor = 0;

  lex->error = false;
}

/* Functions for tokenisation ************************************************/

// Creates new token and adds it to the linked list stored in the lexer struct
void add_token(TokenType type, const char *beg, size_t len) {

  Tprint("Adding token->%s", beg);

  if (lex->cursor >= lex->source_len || lex->source[lex->cursor] == '\0') {

    Tprint("%s", "End of source. Exiting.");
    return;
  }

  Token *token = malloc(sizeof(Token));

  token->type = type;
  token->pos.line = lex->line_num;
  token->pos.x = lex->ln_cursor;
  token->next = NULL;
  token->str = beg;

  assert(token != NULL && token->str != NULL && "NULL problem");

  // If list is empty...
  if (lex->tokens->head == NULL) {
    lex->tokens->head = token;
    lex->tokens->head->next = NULL;
    // Else...
  } else {
    lex->tokens->tail->next = token;
    lex->tokens->tail = token;
  }

  // If token is single char then we can just return
  if (beg == NULL) {
    Tprint("%s", "No token str");
    return;
  }
  if (len < 1) {
    Tprint("%s", "No token str");
    return;
  }

  // If token is not single char...
  char *tokenStr = malloc(
      sizeof(char) * (len + 1)); // Setting up the string stored for the token
  tokenStr[len] = '\0';          // Terminate string

  size_t index = 0; // Keeps a track of how many iterations we've done
  while (lex->source[lex->cursor] && index < len) {
    tokenStr[index] = lex->source[lex->cursor];
    lex->cursor++;
    index++;
  }

  token->str = tokenStr;
  Tprint("token str = %s", token->str);
}

void scan_token(void) {

  if ((lex->cursor >= lex->source_len) || (lex->source[lex->cursor] == EOF)) {
    return;
  }
  const char *curr = &lex->source[lex->cursor];
  Tprint("Consumed char: %s", curr);

  switch (*curr) {

    // Whitespace
  case ' ': {
    Tprint("%s", "Token type whitespace");
    break;
  }
  case '\t': {
    Tprint("%s", "Token type whitespace");
    break;
  }
  case '\r': {
    Tprint("%s", "Token type whitespace");
    break;
  }
    // New line
  case '\n': {
    lex->line_num++;
    lex->ln_cursor = 0;
    Tprint("%s", "Token type whitespace");
    break;
  }

    // Single character tokens
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
  default:
    Tprint("Invalid character: %c", *curr);
    break;
  }
}

TokenList *init_tokenlist(void) {

  TokenList *list = malloc(sizeof(TokenList));
  list->head = NULL;
  list->tail = NULL;

  return list;
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

void test_lexer(void) {

  FILE *file = open_file("tests/basic-lexing");
  size_t filesize = file_size(file);
  char *contents = file_contents(file);
  Tprint("Contents: %s, filesize: %zu", contents, filesize);

  init_lexer(contents, filesize);

  lex->tokens = init_tokenlist();

  while (lex->cursor <= lex->source_len) {
    Tprint("Scanning char: %c", lex->source[lex->cursor]);
    Tprint("Cursor val: %zu", lex->cursor);
    scan_token();
    lex->cursor++;
  }

  Tprint("%s,", "Final output: ");

  while (lex->tokens) {
  }
}
