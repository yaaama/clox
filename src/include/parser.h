#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "lexer.h"
#include "list.h"
#include "token.h"
#include "util.h"

// Parsing
/*****************************************************************************/
/*                                  Parsing                                   */
/*****************************************************************************/

typedef struct PARSER_STRUCT {
  Lexer *lexer;
  Token *token;
} Parser;

Parser *init_parser(Lexer *lex);
AST_t *parse_program(Parser *parser);
void pretty_print_ast(AST_t *ast, int depth);

#endif // PARSER_H_
