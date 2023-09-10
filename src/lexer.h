#ifndef LEXER_H_
#define LEXER_H_

#include "util.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

FILE *open_file(char *filename);
char *file_contents(FILE *file);
size_t file_size(FILE *file);
void test_lexer(void);

#endif // LEXER_H_
