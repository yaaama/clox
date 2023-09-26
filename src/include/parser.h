#ifndef PARSER_H_
#define PARSER_H_

#include "ast.h"
#include "lexer.h"
#include "list.h"
#include "util.h"

// Parsing
/*****************************************************************************/
/*                                  Parsing                                   */
/*****************************************************************************/

typedef struct PARSER_STRUCT {
  Lexer *lexer;
  TokenListNode *tkn_node;
  TokenList *tkn_list;
  Token *token;
} Parser;

Parser *init_parser(Lexer *lex);
AST_t *parse_program(Parser *parser);
void pretty_print_ast(AST_t *ast, int depth);

#endif // PARSER_H_
