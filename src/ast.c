#include "include/ast.h"

AST_t *ast_create(AST_Type type) {

  AST_t *ast = calloc(1, sizeof(AST_t));

  ast->type = type;

  if (type == AST_COMPOUND) {
    ast->children = array_create(sizeof(AST_t *));
  }

  return ast;
}
