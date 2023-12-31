#include "include/lexer.h"
#include "include/parser.h"
#include "include/util.h"
#include <assert.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  char *source = "tests/parsing-class";
  size_t filesize = file_size_name(source);
  char *contents = file_open_read(source);
  Lexer *lexer = lexer_init(contents, filesize);
  lexer_lex(lexer);

  Parser *parser = init_parser(lexer);
  parse_program(parser);

  /* Parser_t *parser = init_parser(lexer); */
}
