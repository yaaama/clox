#ifndef PARSER_H_
#define PARSER_H_

#include "lexer.h"
#include "util.h"

// Parsing
/*****************************************************************************/
/*                                  Parsing                                   */
/*****************************************************************************/

typedef struct Expression {
  enum {
    EXPR_UNARY,
    EXPR_BINARY,
    EXPR_LITERAL,
    EXPR_ASSIGN,
    EXPR_CALL,
  } type;
  Token *token;
} Expression_t;

typedef struct AST_Node {
  Expression_t expression; // The type of expression so we can cast data
  void *data; // Will point to a a specific expr struc such as "ast_expr_Binary"
  LinePosition position; // Position of where the expression is.
  struct AST_Node *left_child;
  struct AST_Node *right_child;
  struct AST_Node *root_node; // REVIEW Don't know if we need this...
} AST_Node_t;

typedef struct AST {
  char *filename;
  char *source;
  AST_Node_t *root_node;

} AST_t;

typedef struct Parser {
  Lexer *lexer;
  Token *token;
} Parser_t;

Parser_t *init_parser(Lexer *lex);

#endif // PARSER_H_
